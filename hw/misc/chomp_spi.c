

#include "qemu/osdep.h"
#include "chardev/char-fe.h"
#include "exec/address-spaces.h"
#include "hw/arm/boot.h"
#include "hw/block/block.h"
#include "hw/block/flash.h"
#include "hw/boards.h"
#include "hw/char/cadence_uart.h"
#include "hw/cpu/a9mpcore.h"
#include "hw/hw.h"
#include "hw/irq.h"
#include "hw/loader.h"
#include "hw/misc/unimp.h"
#include "hw/net/cadence_gem.h"
#include "hw/ptimer.h"
#include "hw/qdev-clock.h"
#include "hw/qdev-properties.h"
#include "hw/registerfields.h"
#include "hw/sd/sdhci.h"
#include "hw/ssi/ssi.h"
#include "hw/sysbus.h"
#include "hw/usb/chipidea.h"
#include "migration/vmstate.h"
#include "net/net.h"
#include "qapi/error.h"
#include "qemu/bitmap.h"
#include "qemu/error-report.h"
#include "qemu/host-utils.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "qemu/timer.h"
#include "qemu/units.h"
#include "sysemu/block-backend.h"
#include "sysemu/reset.h"
#include "sysemu/sysemu.h"
#include "trace.h"

#define CHOMP_SPI_MMIO_SIZE     0x1000
#define CHOMP_SPI_NUM_REGS      (CHOMP_SPI_MMIO_SIZE / 4)

#ifndef CHOMP_SPI_ERR_DEBUG
#define CHOMP_SPI_ERR_DEBUG 1
#endif

#define DB_PRINT(...) do { \
        if (CHOMP_SPI_ERR_DEBUG) { \
            fprintf(stderr,  ": %s: ", __func__); \
            fprintf(stderr, ## __VA_ARGS__); \
        } \
    } while (0)

REG32(SPI_CTRL, 0x000);
REG32(SPI_STA, 0x004);
REG32(SPI_INT_EN, 0x008);
REG32(SPI_DATA_CNT, 0x00c);
REG32(SPI_TX_E, 0x010);
REG32(SPI_RX_E, 0x014);
REG32(SPI_TX_INBUF, 0x018);
REG32(SPI_RX_INBUF, 0x01c);

typedef struct ChompSPIState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    AddressSpace *as;

    uint32_t regs[CHOMP_SPI_NUM_REGS];

    uint32_t current_addr;
    BlockBackend *blk;

} ChompSPIState;


#define TYPE_CHOMP_SPI "chomptech,spi"
#define CHOMP_SPI(obj) OBJECT_CHECK(ChompSPIState, (obj), TYPE_CHOMP_SPI)

static uint64_t chomp_spi_read(void *opaque, hwaddr addr,
                               unsigned size)
{
    ChompSPIState *s = CHOMP_SPI(opaque);
    addr >>= 2;

    uint32_t ret = 0;

    switch(addr) {
        case R_SPI_CTRL:
            DB_PRINT("R_SPI_CTRL read\n");
            break;
        case R_SPI_STA:
            DB_PRINT("R_SPI_STA read\n");
            ret |= 0x100;
            break;
        /*
        case R_SPI_INT_EN:
            DB_PRINT("R_SPI_INT_EN write %08x\n", val);
            break;
        case R_SPI_DATA_CNT:
            DB_PRINT("R_SPI_DATA write %08x\n", val);
            break;
        case R_SPI_TX_E:
            DB_PRINT("R_SPI_TX_E write %08x\n", val);
            break;
        case R_SPI_RX_E:
            DB_PRINT("R_SPI_RX_E write %08x\n", val);
            break;
        case R_SPI_TX_INBUF:
            DB_PRINT("R_SPI_TX_INBUF write %08x\n", val);
            break;
        */
        case R_SPI_RX_INBUF:
            DB_PRINT("R_SPI_RX_INBUF read\n");
            blk_pread(s->blk, s->current_addr * 4, &ret, 0x4);
            s->current_addr++;
            break;
        default:
           DB_PRINT("SPI READ addr: %08" HWADDR_PRIx "\n", addr * 4);
    }

    return ret;
}

static void chomp_spi_write(void *opaque, hwaddr addr,
                            uint64_t value, unsigned size)
{
    ChompSPIState *s = CHOMP_SPI(opaque);
    uint32_t val = (uint32_t)value;
    addr >>= 2;

    switch(addr) {
        case R_SPI_CTRL:
            DB_PRINT("R_SPI_CTRL write %08x\n", val);
            break;
        case R_SPI_STA:
            DB_PRINT("R_SPI_STA write %08x\n", val);
            break;
        case R_SPI_INT_EN:
            DB_PRINT("R_SPI_INT_EN write %08x\n", val);
            break;
        case R_SPI_DATA_CNT:
            DB_PRINT("R_SPI_DATA write %08x\n", val);
            break;
        case R_SPI_TX_E:
            DB_PRINT("R_SPI_TX_E write %08x\n", val);
            break;
        case R_SPI_RX_E:
            DB_PRINT("R_SPI_RX_E write %08x\n", val);
            break;
        case R_SPI_TX_INBUF:
            DB_PRINT("R_SPI_TX_INBUF write %08x\n", val);
            // FIXME:
            s->current_addr = 0;
            break;
        case R_SPI_RX_INBUF:
            DB_PRINT("R_SPI_RX_INBUF write %08x\n", val);
            break;
        default:
            DB_PRINT("SPI WRITE addr: %08" HWADDR_PRIx ", Value: %08x\n", addr * 4, (uint32_t)value);
    }
}

static void chomp_spi_realize(DeviceState *dev, Error **errp)
{
    ChompSPIState *s = CHOMP_SPI(dev);
    //    unsigned long flash_size;
    DriveInfo *dinfo;
    dinfo = drive_get(IF_NONE, 0, 0);
    if (dinfo) {
        s->blk = blk_by_legacy_dinfo(dinfo);
        char data[256];
        blk_pread(s->blk, 0, data, 32);
        printf("First Word: %08x\n", *(uint32_t*)data);

//        flash_size = blk_getlength(s->blk);
    }
}

static const MemoryRegionOps chomp_spi_ops = {
    .read = chomp_spi_read,
    .write = chomp_spi_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 4
    }
};


static const VMStateDescription vmstate_chomp_spi = {
    .name = "chomp_spi",
    .version_id = 3,
    .minimum_version_id = 2,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(regs, ChompSPIState, CHOMP_SPI_NUM_REGS),
        VMSTATE_END_OF_LIST()
    }
};

static void chomp_spi_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
//    ResettableClass *rc = RESETTABLE_CLASS(klass);

    dc->vmsd = &vmstate_chomp_spi;
    dc->realize = chomp_spi_realize;
    /*
    rc->phases.enter = chomp_spi;
    rc->phases.hold  = chomp_slcr_reset_hold;
    rc->phases.exit  = chomp_slcr_reset_exit;*/
}

static void chomp_spi_init(Object *obj)
{
    ChompSPIState *s = CHOMP_SPI(obj);
    memory_region_init_io(&s->iomem, obj, &chomp_spi_ops, s, "chomp.spi",
                          CHOMP_SPI_MMIO_SIZE);


    object_property_add_link(obj, "blk", TYPE_DEVICE,
                             (Object **)&s->blk,
                             object_property_allow_set_link,
                             OBJ_PROP_LINK_STRONG);

    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);
}

static const TypeInfo chomp_spi_info = {
    .name          = TYPE_CHOMP_SPI,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(ChompSPIState),
    .instance_init = chomp_spi_init,
    .class_init    = chomp_spi_class_init,
};

static void chomp_spi_register_types(void)
{
    type_register_static(&chomp_spi_info);
}

type_init(chomp_spi_register_types)
