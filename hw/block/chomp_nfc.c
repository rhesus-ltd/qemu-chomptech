/*
 * QEMU model of Primcell chomp_nfc family of memory controllers
 *
 * Copyright (c) 2012 Xilinx Inc.
 * Copyright (c) 2012 Peter Crosthwaite <peter.crosthwaite@xilinx.com>.
 * Copyright (c) 2011 Edgar E. Iglesias.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "qemu/osdep.h"
#include "hw/hw.h"
#include "hw/block/flash.h"
#include "qapi/error.h"
#include "qemu/timer.h"
#include "sysemu/block-backend.h"
#include "exec/address-spaces.h"
#include "qemu/host-utils.h"
#include "migration/vmstate.h"
#include "hw/qdev-properties.h"
#include "hw/sysbus.h"

#define CHOMP_NFC_ERR_DEBUG 1

#ifdef CHOMP_NFC_ERR_DEBUG
#define DB_PRINT(...) do { \
    fprintf(stderr,  ": %s: ", __func__); \
    fprintf(stderr, ## __VA_ARGS__); \
    } while (0);
#else
    #define DB_PRINT(...)
#endif

#define TYPE_CHOMP_NFC "chomp.nfc"

#define CHOMP_NFC(obj) \
     OBJECT_CHECK(CHOMP_NFCState, (obj), TYPE_CHOMP_NFC)

typedef struct CHOMP_NFCItf {
    MemoryRegion mm;
    DeviceState *dev;
    uint8_t nand_pending_addr_cycles;
} CHOMP_NFCItf;

typedef struct CHOMP_NFCState {
    SysBusDevice parent_obj;

    MemoryRegion mmio;

    /* FIXME: add support for multiple chip selects/interface */

    CHOMP_NFCItf itf[2];

    /* FIXME: add Interrupt support */

    /* FIXME: add ECC support */

    uint8_t x; /* the "x" in chomp_nfc */
} CHOMP_NFCState;

static uint64_t chomp_nfc_read(void *opaque, hwaddr addr,
                         unsigned int size)
{
    CHOMP_NFCState *s = opaque;
    uint32_t r = 0;
    int rdy;

    addr >>= 2;
    switch (addr) {
    case 0x0:
        if (s->itf[0].dev && object_dynamic_cast(OBJECT(s->itf[0].dev),
                                                      "nand")) {
            nand_getpins(s->itf[0].dev, &rdy);
            r |= (!!rdy) << 5;
        }
        if (s->itf[1].dev && object_dynamic_cast(OBJECT(s->itf[1].dev),
                                                      "nand")) {
            nand_getpins(s->itf[1].dev, &rdy);
            r |= (!!rdy) << 6;
        }
        break;
    case 0x56:
            // FIXME:
            DB_PRINT("Strange read\n");
            r = 0x80000000;
            break;
    default:
        DB_PRINT("Unimplemented SMC read access reg=" TARGET_FMT_plx "\n",
                 addr * 4);
        break;
    }
    return r;
}

static void chomp_nfc_write(void *opaque, hwaddr addr, uint64_t value64,
                      unsigned int size)
{
    DB_PRINT("addr=%x v=%x\n", (unsigned)addr, (unsigned)value64);
    addr >>= 2;
    /* FIXME: implement */
    switch(addr) {
        case 0x56:
            DB_PRINT("Strange read\n");
        default:
            DB_PRINT("Some other read\n");
    }
    DB_PRINT("Unimplemented SMC write access reg=" TARGET_FMT_plx "\n",
                 addr * 4);
}

static const MemoryRegionOps chomp_nfc_ops = {
    .read = chomp_nfc_read,
    .write = chomp_nfc_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4
    }
};

static uint64_t nand_read(void *opaque, hwaddr addr,
                           unsigned int size)
{
    CHOMP_NFCItf *s = opaque;
    unsigned int len = size;
    int shift = 0;
    uint32_t r = 0;

    while (len--) {
        uint8_t r8;

        r8 = nand_getio(s->dev) & 0xff;
        r |= r8 << shift;
        shift += 8;
    }
    DB_PRINT("addr=%x r=%x size=%d\n", (unsigned)addr, r, size);
    return r;
}

static void nand_write(void *opaque, hwaddr addr, uint64_t value64,
                       unsigned int size)
{
    struct CHOMP_NFCItf *s = opaque;
    bool data_phase, ecmd_valid;
    unsigned int addr_cycles = 0;
    uint16_t start_cmd, end_cmd;
    uint32_t value = value64;
    uint32_t nandaddr = value;

    DB_PRINT("addr=%x v=%x size=%d\n", (unsigned)addr, (unsigned)value, size);

    /* Decode the various signals.  */
    data_phase = (addr >> 19) & 1;
    ecmd_valid = (addr >> 20) & 1;
    start_cmd = (addr >> 3) & 0xff;
    end_cmd = (addr >> 11) & 0xff;
    if (!data_phase) {
        addr_cycles = (addr >> 21) & 7;
    }

    if (!data_phase) {
        DB_PRINT("start_cmd=%x end_cmd=%x (valid=%d) acycl=%d\n",
                start_cmd, end_cmd, ecmd_valid, addr_cycles);
    }

    /* Writing data to the NAND.  */
    if (data_phase) {
        nand_setpins(s->dev, 0, 0, 0, 1, 0);
        while (size--) {
            nand_setio(s->dev, value & 0xff);
            value >>= 8;
        }
    }

    /* Writing Start cmd.  */
    if (!data_phase && !s->nand_pending_addr_cycles) {
        nand_setpins(s->dev, 1, 0, 0, 1, 0);
        nand_setio(s->dev, start_cmd);
    }

    if (!addr_cycles) {
        s->nand_pending_addr_cycles = 0;
    }
    if (s->nand_pending_addr_cycles) {
        addr_cycles = s->nand_pending_addr_cycles;
        s->nand_pending_addr_cycles = 0;
    }
    if (addr_cycles > 4) {
        s->nand_pending_addr_cycles = addr_cycles - 4;
        addr_cycles = 4;
    }
    while (addr_cycles) {
        nand_setpins(s->dev, 0, 1, 0, 1, 0);
        DB_PRINT("nand cycl=%d addr=%x\n", addr_cycles, nandaddr & 0xff);
        nand_setio(s->dev, nandaddr & 0xff);
        nandaddr >>= 8;
        addr_cycles--;
    }

    /* Writing commands. One or two (Start and End).  */
    if (ecmd_valid && !s->nand_pending_addr_cycles) {
        nand_setpins(s->dev, 1, 0, 0, 1, 0);
        nand_setio(s->dev, end_cmd);
    }
}

static const MemoryRegionOps nand_ops = {
    .read = nand_read,
    .write = nand_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 4
    }
};

static void chomp_nfc_init_sram(SysBusDevice *dev, CHOMP_NFCItf *itf)
{
    /* d Just needs to be a valid sysbus device with at least one memory
     * region
     */
    SysBusDevice *sbd = SYS_BUS_DEVICE(itf->dev);

    memory_region_init(&itf->mm, OBJECT(dev), "chomp_nfc.sram", 1 << 24);
    if (sbd) {
        memory_region_add_subregion(&itf->mm, 0,
                                    sysbus_mmio_get_region(sbd, 0));
    }
    sysbus_init_mmio(dev, &itf->mm);
}

static void chomp_nfc_init_nand(SysBusDevice *dev, CHOMP_NFCItf *itf)
{
    /* d Must be a NAND flash */
    assert(object_dynamic_cast(OBJECT(itf->dev), "nand"));

    memory_region_init_io(&itf->mm, OBJECT(dev), &nand_ops, itf, "chomp.nfc_nand",
                          1 << 24);
    sysbus_init_mmio(dev, &itf->mm);
}

static void chomp_nfc_realize(DeviceState *dev, Error **errp)
{
    CHOMP_NFCState *s = CHOMP_NFC(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
    int itfn = 0;

    memory_region_init_io(&s->mmio, OBJECT(dev), &chomp_nfc_ops, s, "chomp.nfc_io",
                          0x1000);
    sysbus_init_mmio(sbd, &s->mmio);
    if (s->x != 1) { /* everything cept PL351 has at least one SRAM */
        chomp_nfc_init_sram(sbd, &s->itf[itfn]);
        itfn++;
    }

    if (s->x & 0x1) {  
        chomp_nfc_init_nand(sbd, &s->itf[itfn]);
    } else if (s->x == 4) { /* PL354 has a second SRAM */
        chomp_nfc_init_sram(sbd, &s->itf[itfn]);
    }
}

static void chomp_nfc_initfn(Object *obj)
{
    CHOMP_NFCState *s = CHOMP_NFC(obj);

    object_property_add_link(obj, "dev0", TYPE_DEVICE,
                             (Object **)&s->itf[0].dev,
                             object_property_allow_set_link,
                             OBJ_PROP_LINK_STRONG);
    object_property_add_link(obj, "dev1", TYPE_DEVICE,
                             (Object **)&s->itf[1].dev,
                             object_property_allow_set_link,
                             OBJ_PROP_LINK_STRONG);
}

static Property chomp_nfc_properties[] = {
    DEFINE_PROP_UINT8("x", CHOMP_NFCState, x, 3),
    DEFINE_PROP_END_OF_LIST(),
};

static const VMStateDescription vmstate_chomp_nfc = {
    .name = "chomp_nfc",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT8(itf[0].nand_pending_addr_cycles, CHOMP_NFCState),
        VMSTATE_UINT8(itf[1].nand_pending_addr_cycles, CHOMP_NFCState),
        VMSTATE_END_OF_LIST()
    }
};

static void chomp_nfc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = chomp_nfc_realize;
    device_class_set_props(dc, chomp_nfc_properties);
    dc->vmsd = &vmstate_chomp_nfc;
}

static TypeInfo chomp_nfc_info = {
    .name           = TYPE_CHOMP_NFC,
    .parent         = TYPE_SYS_BUS_DEVICE,
    .instance_size  = sizeof(CHOMP_NFCState),
    .class_init     = chomp_nfc_class_init,
    .instance_init  = chomp_nfc_initfn,
};

static void chomp_nfc_register_types(void)
{
    type_register_static(&chomp_nfc_info);
}

type_init(chomp_nfc_register_types)
