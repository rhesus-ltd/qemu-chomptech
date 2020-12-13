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
#include "qemu/log.h"
#include "qemu/timer.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"
#include "hw/sysbus.h"
#include "qemu/module.h"
#include "chardev/char-fe.h"
#include "hw/registerfields.h"
#include "hw/ptimer.h"

#define CHOMP_NFC_ERR_DEBUG 1

#ifdef CHOMP_NFC_ERR_DEBUG
#define DB_PRINT(...) do { \
    fprintf(stderr,  ": %s: ", __func__); \
    fprintf(stderr, ## __VA_ARGS__); \
    } while (0);
#else
    #define DB_PRINT(...)
#endif

REG32(NFC_COMMAND, 0x000)

/* command register*/
#define NFC_CMD_REG_RES_BIT         (1UL << 31)   //this bit is reserved in cmd register
#define NFC_CMD_REG_INFO_POS        11          //bit 11 - bit 22
#define NFC_CMD_REG_DELAY_BIT       (1 << 10)   //wait delay time enable bit
#define NFC_CMD_REG_WAIT_JUMP_BIT   (1 << 9)    //wait R/b enable bit
#define NFC_CMD_REG_DAT_BIT         (1 << 8)
#define NFC_CMD_REG_STF_EN_BIT      (1 << 7)
#define NFC_CMD_REG_CMD_BIT         (1 << 6)
#define NFC_CMD_REG_WE_BIT          (1 << 5)
#define NFC_CMD_REG_RE_BIT          (1 << 4)
#define NFC_CMD_REG_CNT_EN_BIT      (1 << 3)
#define NFC_CMD_REG_CLE_BIT         (1 << 2)
#define NFC_CMD_REG_ALE_BIT         (1 << 1)
#define NFC_CMD_REG_LAST_BIT        (1 << 0)    // last command's bit0 set to 1

REG32(NFC_DAT_REG1, 0x051)
REG32(NFC_DAT_REG2, 0x054)
REG32(NFC_CTRL, 0x058)
REG32(NFC_CMD_LEN_REG, 0x05C)
REG32(NFC_DAT_LEN_REG, 0x060)
REG32(NFC_RAND_ENC_REG, 0x068)
REG32(NFC_RAND_DEC_REG, 0x06C)

#define NFC_CTRL_REG_CMD_DONE_BIT    (1UL << 31)
#define NFC_CTRL_REG_CMD_VALID_BIT   (1 << 30)
#define NFC_CTRL_REG_STA_CLR_BIT     (1 << 14)
#define NFC_CTRL_REG_CE_SEL_POS     (10)
#define NFC_CTRL_REG_CE3_SEL_BIT     (1 << (NFC_CTRL_REG_CE_SEL_POS + 3))
#define NFC_CTRL_REG_CE2_SEL_BIT     (1 << (NFC_CTRL_REG_CE_SEL_POS + 2))
#define NFC_CTRL_REG_CE1_SEL_BIT     (1 << (NFC_CTRL_REG_CE_SEL_POS + 1))
#define NFC_CTRL_REG_CE0_SEL_BIT     (1 << (NFC_CTRL_REG_CE_SEL_POS))
#define NFC_CTRL_REG_CE_SAVE_BIT     (1 << 9)
#define NFC_CTRL_REG_STF_INFO_POS    (1)         //bit1 -bit 8
#define NFC_CTRL_REG_POWER_SAVE_BIT  (1 << 0)
#define NFC_CTRL_REG_CHIP_CONF(chip) ((0x01 << (chip)) << NFC_CTRL_REG_CE_SEL_POS)

REG32(NFC_CMD_LEN, 0x05C)
REG32(NFC_DATA_LEN, 0x060)
REG32(NFC_RAND_ENC, 0x068)
REG32(NFC_RAND_DEC, 0x06C)

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
    //CHOMP_NFCState *s = opaque;
    uint32_t ret = 0;

    //DB_PRINT("Unimplemented read access reg=" TARGET_FMT_plx "\n", addr * 4);

    addr >>= 2;
    switch (addr) {
        case R_NFC_RAND_DEC:
          //  printf("huga\n");
            break;
        case R_NFC_COMMAND:
          //  printf("haga\n");
            break;
        case R_NFC_CTRL:
          //  printf("nfc status\n");
            ret |= NFC_CTRL_REG_CMD_DONE_BIT;
            break;
        default:
            //DB_PRINT("Unimplemented read access reg=" TARGET_FMT_plx "\n", addr * 4);
            break;
    }
    return ret;
}

static void chomp_nfc_write(void *opaque, hwaddr addr, uint64_t value64,
                      unsigned int size)
{
    //CHOMP_NFCState *s = opaque; 
    //DB_PRINT("addr=%x v=%x\n", (unsigned)addr, (unsigned)value64);
    addr >>= 2;
    /* FIXME: implement */
    switch(addr) {
        case R_NFC_RAND_DEC:
            printf("NFC_RAND_DEC write\n");
            break;
        case R_NFC_COMMAND:
            printf("NFC_COMMAND write\n");
            break;
        case R_NFC_CTRL:
            printf("nfc status write\n");
            break;
        case R_NFC_DAT_REG1:
            printf("nfc reg1 write\n");
            break;
        case R_NFC_DAT_REG2:
            printf("nfc reg2 write\n");
            break;
        case R_NFC_CMD_LEN_REG:
            printf("nfc cmd len write\n");
            break;
        case R_NFC_DAT_LEN_REG:
            printf("nfc dat len write\n");
            break;
        default:
            DB_PRINT("Some other write\n");
    }
    DB_PRINT("Unimplemented write access reg=" TARGET_FMT_plx " Data: %08x\n", addr * 4, (uint32_t)value64);
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


static void chomp_nfc_realize(DeviceState *dev, Error **errp)
{
    CHOMP_NFCState *s = CHOMP_NFC(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);

    memory_region_init_io(&s->mmio, OBJECT(dev), &chomp_nfc_ops, s, "chomp.nfc_io",
                          0x16000);
    sysbus_init_mmio(sbd, &s->mmio);
    /*if (s->x != 1) { 
        chomp_nfc_init_sram(sbd, &s->itf[itfn]);
        itfn++;
    }

    if (s->x & 0x1) {  
        chomp_nfc_init_nand(sbd, &s->itf[itfn]);
    } else if (s->x == 4) {
        chomp_nfc_init_sram(sbd, &s->itf[itfn]);
    }
    */
}

static void chomp_nfc_initfn(Object *obj)
{
    CHOMP_NFCState *s = CHOMP_NFC(obj);

    printf("Instance init\n");

    object_property_add_link(obj, "dev0", TYPE_DEVICE,
                             (Object **)&s->itf[0].dev,
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
