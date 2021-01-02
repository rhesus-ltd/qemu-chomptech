#include "qemu/osdep.h"
#include "qemu/timer.h"
#include "sysemu/runstate.h"
#include "hw/sysbus.h"
#include "hw/fdt_generic_devices.h"
#include "sysemu/sysemu.h"
#include "hw/core/cpu.h"
#include "qapi/error.h"
#include "qemu/option.h"
#include "migration/vmstate.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/registerfields.h"
#include "hw/qdev-clock.h"
#include "hw/ptimer.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"
#include "migration/vmstate.h"
#include "qemu/bitops.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "qapi/error.h"

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


typedef struct ChompSPIState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    AddressSpace *as;

    uint32_t regs[CHOMP_SPI_NUM_REGS];

} ChompSPIState;


#define TYPE_CHOMP_SPI "chomptech,spi"
#define CHOMP_SPI(obj) OBJECT_CHECK(ChompSPIState, (obj), TYPE_CHOMP_SPI)

static uint64_t chomp_spi_read(void *opaque, hwaddr addr,
                               unsigned size)
{
//    ChompSPIState *s = CHOMP_SPI(opaque);
    addr >>= 2;

    uint32_t ret = 0;

    switch(addr) {
        case 1: 
            ret |= 0x100; // FIX ME BIG TIME
    }

    DB_PRINT("SPI READ addr: %08" HWADDR_PRIx "\n", addr * 4);

    return ret;
}

static void chomp_spi_write(void *opaque, hwaddr addr,
                            uint64_t value, unsigned size)
{
    //ChompSPIState *s = CHOMP_SPI(opaque);
    addr >>= 2;

    switch(addr) {
    }
 
    DB_PRINT("SPI WRITE addr: %08" HWADDR_PRIx ", Value: %08x\n", addr * 4, (uint32_t)value);
}

static void chomp_spi_realize(DeviceState *dev, Error **errp)
{
  //  ChompSPIState *s = CHOMP_SPI(dev);
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
