#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qapi/error.h"
#include "cpu.h"
#include "hw/sysbus.h"
#include "hw/arm/boot.h"
#include "net/net.h"
#include "exec/address-spaces.h"
#include "sysemu/sysemu.h"
#include "hw/boards.h"
#include "hw/block/flash.h"
#include "hw/loader.h"
#include "hw/ssi/ssi.h"
#include "hw/usb/chipidea.h"
#include "qemu/error-report.h"
#include "hw/misc/unimp.h"
#include "hw/sd/sdhci.h"
#include "hw/char/cadence_uart.h"
#include "hw/net/cadence_gem.h"
#include "hw/cpu/a9mpcore.h"
#include "hw/qdev-clock.h"
#include "sysemu/reset.h"

#include "qemu/osdep.h"
#include "hw/block/block.h"
#include "hw/block/flash.h"
#include "hw/qdev-properties.h"
#include "qapi/error.h"
#include "qemu/error-report.h"
#include "qemu/bitmap.h"
#include "qemu/timer.h"
#include "sysemu/block-backend.h"
#include "qemu/host-utils.h"
#include "qemu/module.h"
#include "hw/sysbus.h"
#include "migration/vmstate.h"
#include "trace.h"

#define TYPE_CHOMP_MACHINE MACHINE_TYPE_NAME("chomptech")
#define CHOMP_MACHINE(obj) \
    OBJECT_CHECK(ChompMachineState, (obj), TYPE_CHOMP_MACHINE)

/* board base frequency: 33.333333 MHz */
#define CLK_FREQUENCY (100 * 1000 * 1000 / 3)

#define FLASH_SIZE (64 * 1024 * 1024)
#define FLASH_SECTOR_SIZE (128 * 1024)

#define IRQ_OFFSET 32 /* pic interrupts start from index 32 */

#define MPCORE_PERIPHBASE 0xF8F00000
#define CHOMP_BOARD_MIDR 0x413FC090

#define BOARD_SETUP_ADDR        0x100

#define SLCR_LOCK_OFFSET        0x004
#define SLCR_UNLOCK_OFFSET      0x008
#define SLCR_ARM_PLL_OFFSET     0x100

#define SLCR_XILINX_UNLOCK_KEY  0xdf0d
#define SLCR_XILINX_LOCK_KEY    0x767b

#define CHOMP_SDHCI_CAPABILITIES 0x69ec0080  /* Datasheet: UG585 (v1.12.1) */

#define MP_FLASH_SIZE_MAX       32*1024*1024

#define ARMV7_IMM16(x) (extract32((x),  0, 12) | \
                        extract32((x), 12,  4) << 16)


#define CHOMP_FLASHCFG_BASE        0x0404a000
#define CHOMP_FLASHCFG_SIZE        0x00002000

#define TYPE_MV88W8618_FLASHCFG "mv88w8618_flashcfg"
#define MV88W8618_FLASHCFG(obj) \
    OBJECT_CHECK(mv88w8618_flashcfg_state, (obj), TYPE_MV88W8618_FLASHCFG)


/* Write immediate val to address r0 + addr. r0 should contain base offset
 * of the SLCR block. Clobbers r1.
 */

#define SLCR_WRITE(addr, val) \
    0xe3001000 + ARMV7_IMM16(extract32((val),  0, 16)), /* movw r1 ... */ \
    0xe3401000 + ARMV7_IMM16(extract32((val), 16, 16)), /* movt r1 ... */ \
    0xe5801000 + (addr)

typedef struct ChompMachineState {
    MachineState parent;
    Clock *clk;
} ChompMachineState;

static void chomp_write_board_setup(ARMCPU *cpu,
                                   const struct arm_boot_info *info)
{
    int n;
    uint32_t board_setup_blob[] = {
        0xe3a004f8, /* mov r0, #0xf8000000 */
        SLCR_WRITE(SLCR_UNLOCK_OFFSET, SLCR_XILINX_UNLOCK_KEY),
        SLCR_WRITE(SLCR_ARM_PLL_OFFSET, 0x00014008),
        SLCR_WRITE(SLCR_LOCK_OFFSET, SLCR_XILINX_LOCK_KEY),
        0xe12fff1e, /* bx lr */
    };
    for (n = 0; n < ARRAY_SIZE(board_setup_blob); n++) {
        board_setup_blob[n] = tswap32(board_setup_blob[n]);
    }
    rom_add_blob_fixed("board-setup", board_setup_blob,
                       sizeof(board_setup_blob), BOARD_SETUP_ADDR);
}

static struct arm_boot_info chomp_binfo = {};

static void chomp_init(MachineState *machine)
{
    ChompMachineState *chomp_machine = CHOMP_MACHINE(machine);
    ARMCPU *cpu;
    MemoryRegion *address_space_mem = get_system_memory();
    MemoryRegion *ocm_ram = g_new(MemoryRegion, 1);
    MemoryRegion *ram = g_new(MemoryRegion, 1);
    DeviceState *dev, *slcr;
    //qemu_irq pic[64];

    cpu = ARM_CPU(object_new(machine->cpu_type));

    object_property_set_int(OBJECT(cpu), "midr", CHOMP_BOARD_MIDR,
                            &error_fatal);

//    object_property_set_int(OBJECT(cpu), "reset-cbar", 0x00000000, &error_fatal);
    qdev_realize(DEVICE(cpu), NULL, &error_fatal);

    // 64k of mask ROM
    memory_region_init_rom(ocm_ram, NULL, "chomp.rom", 64 * KiB, &error_fatal);
    memory_region_add_subregion(address_space_mem, 0x00000000, ocm_ram);

    // 192k of OCM
    memory_region_init_ram(ram, NULL, "chomp.ocmem", 192 * KiB,
                           &error_fatal);
    memory_region_add_subregion(address_space_mem, 0x08000000, ram);

    /* Create slcr, keep a pointer to connect clocks */
    slcr = qdev_new("chomptech,chomp_slcr");
    sysbus_realize_and_unref(SYS_BUS_DEVICE(slcr), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(slcr), 0, 0x04000000);

    /* Create the main clock source, and feed slcr with it */
    chomp_machine->clk = CLOCK(object_new(TYPE_CLOCK));
    object_property_add_child(OBJECT(chomp_machine), "clk",
                              OBJECT(chomp_machine->clk));
    object_unref(OBJECT(chomp_machine->clk));
    clock_set_hz(chomp_machine->clk, CLK_FREQUENCY);
    qdev_connect_clock_in(slcr, "clk", chomp_machine->clk);

    // ---- IRQ Controller -----------------------------------------------------------------
    // ...

    // ---- UART Controller -----------------------------------------------------------------
    dev = qdev_new("chomptech,uart");
    qdev_prop_set_chr(dev, "chardev", serial_hd(0));
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x04036000);
    //qdev_connect_clock_in(dev, "clk", chomp_machine->clk);

    // ---- SPI Controller ------------------------------------------------------------------
    dev = qdev_new("chomptech,spi");
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x040a0000);

    // ---- L2 Controller ------------------------------------------------------------------
    dev = qdev_new("chomptech,l2");
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x04010000);

    // ---- NAND Controller -----------------------------------------------------------------
    dev = qdev_new("chomp,nfc");
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x0404a000);

    chomp_binfo.ram_size = machine->ram_size;
    chomp_binfo.nb_cpus = 1;
    chomp_binfo.board_id = 0xd32;
    chomp_binfo.loader_start = 0;
    chomp_binfo.board_setup_addr = BOARD_SETUP_ADDR;
    chomp_binfo.write_board_setup = chomp_write_board_setup;

//    create_unimplemented_device("USB", 0x04070000, 0x1000);
        
    arm_load_kernel(ARM_CPU(first_cpu), machine, &chomp_binfo);
}

static void chomp_machine_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    mc->desc = "Rudamental Chomptech TipToi Emulation";
    mc->init = chomp_init;
    mc->max_cpus = 1;
    mc->no_sdcard = 1;
    mc->ignore_memory_transaction_failures = true;
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("arm926");
    mc->default_ram_id = "chomp.ext_ram";
}

static const TypeInfo chomp_machine_type = {
    .name = TYPE_CHOMP_MACHINE,
    .parent = TYPE_MACHINE,
    .class_init = chomp_machine_class_init,
    .instance_size = sizeof(ChompMachineState),
};

static void chomp_machine_register_types(void)
{
    type_register_static(&chomp_machine_type);
}

type_init(chomp_machine_register_types)
