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



#ifndef CHOMP_L2_ERR_DEBUG
#define CHOMP_L2_ERR_DEBUG 1
#endif

#define DB_PRINT(...) do { \
        if (CHOMP_L2_ERR_DEBUG) { \
            fprintf(stderr,  ": %s: ", __func__); \
            fprintf(stderr, ## __VA_ARGS__); \
        } \
    } while (0)

#define CHOMP_L2_MMIO_SIZE     0x1000
#define CHOMP_L2_NUM_REGS      (CHOMP_L2_MMIO_SIZE / 4)

typedef struct ChompL2State {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    hwaddr base;

    uint32_t regs[CHOMP_L2_NUM_REGS];
} ChompL2State;


#define TYPE_CHOMP_L2 "chomptech,l2"
#define CHOMP_L2(obj) OBJECT_CHECK(ChompL2State, (obj), TYPE_CHOMP_L2)


static uint64_t chomp_l2_read(void *opaque, hwaddr addr,
                               unsigned size)
{
    addr >>= 2;

    switch (addr) {
    }

    DB_PRINT("addr: %08" HWADDR_PRIx "\n", addr * 4);

    return 0;
}

static void chomp_l2_write(void *opaque, hwaddr addr,
                            uint64_t value, unsigned size)
{    
    addr >>= 2;

    switch (addr) {
    default:
        break;
    }

    DB_PRINT("addr: %08" HWADDR_PRIx " data: %08" PRIx32 "\n", addr * 4, value);
}


static void chomp_l2_realize(DeviceState *dev, Error **errp)
{
    ChompL2State *s = CHOMP_L2(dev);
}

static const VMStateDescription vmstate_chomp_l2 = {
    .name = "chomp_l2",
    .version_id = 3,
    .minimum_version_id = 2,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(regs, ChompL2State, CHOMP_L2_NUM_REGS),
        VMSTATE_END_OF_LIST()
    }
};

static void chomp_l2_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->vmsd = &vmstate_chomp_l2;
    dc->realize = chomp_l2_realize;
}

static const MemoryRegionOps chomp_l2_ops = {
    .read = chomp_l2_read,
    .write = chomp_l2_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void chomp_l2_init(Object *obj)
{
    ChompL2State *s = CHOMP_L2(obj);
    memory_region_init_io(&s->iomem, obj, &chomp_l2_ops, s, "chomp.l2",
                          CHOMP_L2_MMIO_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);
}



static const TypeInfo chomp_l2_info = {
    .name          = TYPE_CHOMP_L2,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(ChompL2State),
    .instance_init = chomp_l2_init,
    .class_init    = chomp_l2_class_init,
};

static void chomp_l2_register_types(void)
{
    type_register_static(&chomp_l2_info);
}

type_init(chomp_l2_register_types)