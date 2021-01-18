

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

#define NFC_CMD_REG_CMD_CONF        (NFC_CMD_REG_CLE_BIT | NFC_CMD_REG_WE_BIT | NFC_CMD_REG_CMD_BIT)
#define NFC_CMD_REG_ADD_CONF        (NFC_CMD_REG_ALE_BIT | NFC_CMD_REG_WE_BIT | NFC_CMD_REG_CMD_BIT)
#define NFC_CMD_REG_REG_IN_CONF    (NFC_CMD_REG_CNT_EN_BIT | NFC_CMD_REG_RE_BIT | NFC_CMD_REG_CMD_BIT)
#define NFC_CMD_REG_WDATA_CONF      (NFC_CMD_REG_WE_BIT | NFC_CMD_REG_DAT_BIT | NFC_CMD_REG_CNT_EN_BIT)
#define NFC_CMD_REG_RDATA_CONF      (NFC_CMD_REG_RE_BIT | NFC_CMD_REG_DAT_BIT | NFC_CMD_REG_CNT_EN_BIT)

REG32(NFC_DAT_REG1, 0x051)
REG32(NFC_DAT_REG2, 0x054)
REG32(NFC_CTRL, 0x058)
REG32(NFC_CMD_LEN_REG, 0x05C)
REG32(NFC_DAT_LEN_REG, 0x060)
REG32(NFC_RAND_ENC_REG, 0x068)
REG32(NFC_RAND_DEC_REG, 0x06C)

REG32(ECC_CTRL, 0x11000)

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

#define TYPE_CHOMP_NFC "chomp,nfc"

#define CHOMP_NFC_MMIO_SIZE     0x16000
#define CHOMP_NFC_NUM_REGS      (CHOMP_NFC_MMIO_SIZE / 4)

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

    BlockBackend *blk;
    DriveInfo *drv;
    AddressSpace *as;

    uint32_t buf_ptr;
    uint32_t rd_reg_len;

    uint8_t cycle;
    uint16_t cmd;
    uint16_t addr[5];
    uint16_t addr_cnt;
    uint16_t end;
    uint16_t dly;

    uint32_t addr_int;

    uint32_t regs[CHOMP_NFC_NUM_REGS];
} CHOMP_NFCState;


static uint64_t chomp_nfc_read(void *opaque, hwaddr addr,
                         unsigned int size)
{
    CHOMP_NFCState *s = opaque;

    //DB_PRINT("Unimplemented read access reg=" TARGET_FMT_plx "\n", addr * 4);

    addr >>= 2;
    uint32_t ret = s->regs[addr];

    switch (addr) {
        case R_NFC_RAND_DEC:
          //  printf("huga\n");
            break;
        case R_NFC_COMMAND:
        case R_NFC_COMMAND + 1:
        case R_NFC_COMMAND + 2:
        case R_NFC_COMMAND + 3:
        case R_NFC_COMMAND + 4:
        case R_NFC_COMMAND + 5:
        case R_NFC_COMMAND + 6:
        case R_NFC_COMMAND + 7:
            printf("NFC_COMMAND read (cycle %d): %08x xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n", (int)addr, ret);
            break;
        case R_NFC_DAT_REG1:
            printf("nfc reg1 read, %d, %d\n", s->rd_reg_len, s->buf_ptr++);
            break;
        case R_NFC_DAT_REG2:
            printf("nfc reg2 read, %d, %d\n", s->rd_reg_len, s->buf_ptr++);
            break;
        case R_NFC_CTRL:
            printf("nfc status\n");
            //ret |= NFC_CTRL_REG_CMD_DONE_BIT; // this should move to an actual command
            break;
        case R_ECC_CTRL:
            DB_PRINT("ECC CTRL READ!\n");
            // ECC is always happy atm.
            ret |= 0x40;
            ret |= 0x06000000; 
            break;
        default:
            DB_PRINT("Unimplemented read access reg=" TARGET_FMT_plx "\n", addr * 4);
            break;
    }
    return ret;
}

static void chomp_nfc_write(void *opaque, hwaddr addr, uint64_t value64,
                      unsigned int size)
{
    CHOMP_NFCState *s = opaque; 
    //DB_PRINT("addr=%x v=%x\n", (unsigned)addr, (unsigned)value64);
    char dest[0x800];
    uint32_t value = value64;
    addr >>= 2;
    uint32_t r = s->regs[addr]; 
    /* FIXME: implement */
    switch(addr) {
        case R_NFC_RAND_DEC:
            printf("NFC_RAND_DEC write\n");
            break;
        case R_NFC_COMMAND:
            printf("Start of Command Cyce ///////////////////////////////////////////////\n");
            printf("RES | INF | DLY | WIT | DAT | STF | CMD | WE  | RE  | CNT | CLE | ALE | LST \n");
        case R_NFC_COMMAND + 1:
        case R_NFC_COMMAND + 2:
        case R_NFC_COMMAND + 3:
        case R_NFC_COMMAND + 4:
        case R_NFC_COMMAND + 5:
        case R_NFC_COMMAND + 6:
        case R_NFC_COMMAND + 7:
        case R_NFC_COMMAND + 8:
        case R_NFC_COMMAND + 9:

            s->cycle = addr;
            uint16_t reg_info = (value >> NFC_CMD_REG_INFO_POS) & 0xFFF; 

            printf(" %c  |", (value & NFC_CMD_REG_RES_BIT ? 'X' : ' '));
            printf(" %03x |", reg_info);
            printf("  %c  |", (value & NFC_CMD_REG_DELAY_BIT ? 'X' : ' '));
            printf("  %c  |", (value & NFC_CMD_REG_WAIT_JUMP_BIT ? 'X' : ' '));
            printf("  %c  |", (value & NFC_CMD_REG_DAT_BIT ? 'X' : ' '));
            printf("  %c  |", (value & NFC_CMD_REG_STF_EN_BIT ? 'X' : ' '));
            printf("  %c  |", (value & NFC_CMD_REG_CMD_BIT ? 'X' : ' '));
            printf("  %c  |", (value & NFC_CMD_REG_WE_BIT ? 'X' : ' '));
            printf("  %c  |", (value & NFC_CMD_REG_RE_BIT ? 'X' : ' '));
            printf("  %c  |", (value & NFC_CMD_REG_CNT_EN_BIT ? 'X' : ' '));
            printf("  %c  |", (value & NFC_CMD_REG_CLE_BIT ? 'X' : ' '));
            printf("  %c  |", (value & NFC_CMD_REG_ALE_BIT ? 'X' : ' '));
            printf("  %c  |", (value & NFC_CMD_REG_LAST_BIT ? 'X' : ' '));
           
            printf(" NFC_COMMAND write (cycle %d): %08x\n", (int)addr, (uint32_t)value64);
          
            if(value & NFC_CMD_REG_CNT_EN_BIT) {
//                printf("NFC_CMD_REG_CNT_EN_BIT write\n");
                r |= NFC_CMD_REG_CNT_EN_BIT;

                s->addr_int++;
                
                uint32_t column = s->addr[0] << 8 | s->addr[1];
                uint32_t row = (s->addr[2] << 16) | (s->addr[3] << 8) | s->addr[4];

//                printf("Addr Int: %08x, Column: %d, Row: %d\n", s->addr_int, column, row);
            }

            if(value & NFC_CMD_REG_RE_BIT) {
//                printf("NFC_CMD_REG_RE_BIT write\n");
                r |= NFC_CMD_REG_RE_BIT;
            }

            if(value & NFC_CMD_REG_DAT_BIT) {
                r |= NFC_CMD_REG_DAT_BIT; 
  //              printf("(DAT) NFC_CMD_REG_INFO_POS write: %04x\n", (value >> 11) & 0x3FF);
            }

            if(value & NFC_CMD_REG_RES_BIT) {
            //   printf("NFC_CMD_REG_RES_BIT write\n");
               s->rd_reg_len = value & (0xFFF << NFC_CMD_REG_INFO_POS);
            }
            
            if(value & (0x3FF << NFC_CMD_REG_INFO_POS)) {
            //   printf("NFC_CMD_REG_INFO_POS write: %04x\n", (value >> 11) & 0x3FF);
            }
            
            if(value & NFC_CMD_REG_WAIT_JUMP_BIT) {
            //   printf("NFC_CMD_REG_WAIT_JUMP_BIT write\n");
            }
            
            if(value & NFC_CMD_REG_STF_EN_BIT) {
            //   printf("NFC_CMD_REG_STF_EN_BIT write\n");
            }

            if(value & NFC_CMD_REG_CMD_BIT) {
              // printf("NFC_CMD_REG_CMD_BIT write\n");
               s->buf_ptr = 0;
            }
            
            if(value & NFC_CMD_REG_WE_BIT) {
              // printf("WRITE ENABLE\n");
            } else {
              // printf("WRTIE DISABLE\n");
            }
            
            if(value & NFC_CMD_REG_CLE_BIT) {
              //  printf("COMMAND LATCH ENABLE\n");
            } else {
              //  printf("COMMAND LATCH DISABLE\n");
            }
            
            if(value & NFC_CMD_REG_ALE_BIT) {
               //printf("ADDRESSS LATCH ENABLE\n");
            } else {
              // printf("ADDRESSS LATCH DISABLE\n");
            }
            
            if(value & NFC_CMD_REG_DELAY_BIT) {
            //   printf("DELAY_BIT write\n");
            }

            if(value & NFC_CMD_REG_WE_BIT && value & NFC_CMD_REG_CMD_BIT) {
                if(value & NFC_CMD_REG_ALE_BIT) {
                    s->addr[addr - 1] = reg_info;
                    s->addr_cnt++;
                   // printf("Address latch\n");
                } else if(value & NFC_CMD_REG_CLE_BIT) {
                    if(addr == 0) {
                        s->cmd = reg_info;
                        s->addr_cnt = 0;
                    } else {
                        s->end = reg_info;
                    }
                }
            }

            /*
            if((value & NFC_CMD_REG_CMD_CONF) == NFC_CMD_REG_CMD_CONF) {
                printf("NFC_CMD_REG_CMD_CONF ++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                if(s->cycle == 0) {
                    s->cmd = reg_info;
                    printf("Commmand Cycle\n");
                    s->addr_cnt = 0;
                } else {
                    s->end = reg_info;
                    printf("End Cycle\n");
                    printf("Got NAND command: %02x, ", s->cmd);
                    for(int i = 0; i < s->addr_cnt ; i++) {
                        printf("%02x, ",s->addr[i]);
                    }
                    printf("%02x\n", s->end);
                } 
            }*/

            if((value & NFC_CMD_REG_DELAY_BIT) == NFC_CMD_REG_DELAY_BIT) {
            //    printf("NFC_CMD_REG_DELAY_BIT -------------------------------------------------------\n");
                s->dly = reg_info;
            } 

            /*
            if((value & NFC_CMD_REG_ADD_CONF) == NFC_CMD_REG_ADD_CONF) {
                printf("NFC_CMD_REG_ADD_CONF ========================================================\n");
                printf("%08x\n", value);
                s->addr[addr - 1] = reg_info;
                s->addr_cnt++;
            }
            */

            if((value & NFC_CMD_REG_RDATA_CONF) == NFC_CMD_REG_RDATA_CONF) {
                printf("NFC_CMD_REG_RDATA_CONF ========================================================\n");
                printf("%08x\n", value);
                printf("Offset: %08x, %08x\n", (reg_info >> 4), (reg_info >> 6));

                               uint32_t column = s->addr[0] << 8 | s->addr[1];
                uint32_t row = (s->addr[2] << 16) | (s->addr[3] << 8) | s->addr[4];

                s->addr_int = column << 24 | row;

                blk_pread(s->blk, 0x800 * column, dest, 0x800);

                printf("Addr-Int: %08x, Column: %d, Row: %d\n", s->addr_int, column, row);

                for(int i = 0; i < 0x800/4; i++) {
                    //address_space_stl_le(s->as, 0x0802f800 + i * 4, *((uint32_t*)dest + (reg_info >> 6) + i - 1), MEMTXATTRS_UNSPECIFIED, NULL); 
                    address_space_stl_le(s->as, 0x0802f800 + i * 4, *((uint32_t*)dest + i), MEMTXATTRS_UNSPECIFIED, NULL); 
                }
            }

            if(value & NFC_CMD_REG_LAST_BIT) {
                //printf("EEEENNNNNDDDD of TRANSACTION oooooooooooooooooooooooooooooooooooo\n");
                //printf("NFC_CMD_REG_LAST_BIT write\n");
                //r |= NFC_CMD_REG_LAST_BIT; 
                //printf("End Cycle\n");
                    printf("Got NAND command: %02x, ", s->cmd);
                    if(s->addr_cnt) {
                        printf("Address: ");  
                        for(int i = 0; i < s->addr_cnt; i++) {
                            printf("%02x, ",s->addr[i]);
                        }
                    }
                printf("End: %02x, Delay: %02x\n", s->end, s->dly);  

                /*if(s->cmd == 0x0) {
                    if(s->cmd == 0x0 && s->end == 0x30) {
                        printf("RESET STATUS READ!\n");
                        dest[0] = 0;
                        dest[1] = 0;
                        dest[2] = 0;
                        dest[3] = (3 << 5);
                    }

                    uint32_t column = s->addr[0] << 8 | s->addr[1];
                    uint32_t row = (s->addr[2] << 16) | (s->addr[3] << 8) | s->addr[4];

                    s->addr_int = column << 24 | row;

                    blk_pread(s->blk, 0x800 * column, dest, 0x800);

                    printf("Addr-Int: %08x, Column: %d, Row: %d\n", s->addr_int, column, row);

                    for(int i = 0; i < 0x800/4; i++) {
                        address_space_stl_le(s->as, 0x0802f800 + i * 4, *((uint32_t*)dest + i), MEMTXATTRS_UNSPECIFIED, NULL); 
                    }
                } else 
                */if(s->cmd == 0xFF) {
                    printf("Reset command\n");
                    s->addr_int = 0;
                    s->addr_cnt = 0;
                    s->buf_ptr = 0;
                }
            }

           
            break;
        case R_NFC_CTRL:
            //printf("nfc status write\n");
            if(value & NFC_CTRL_REG_CMD_VALID_BIT) {
                /*blk_pread(s->blk, 0x1C, dest, 0x100);
                printf("First Word: %8p, %08x\n", s->blk, *(unsigned int*)dest);
                for(int i = 0; i < 0x100/4; i++) { // This is wrong.. so wrong.. FIXME:
                    printf("Store: %08x to %08x\n", *((uint32_t*)dest + i), 0x0802f800 + i * 4);
                    address_space_stl_le(s->as, 0x0802f800 + i * 4, *((uint32_t*)dest + i), MEMTXATTRS_UNSPECIFIED, NULL); //copy the shit to the guest memory
                }
                */
                //printf("Command valid bit set\n");
                r |= NFC_CTRL_REG_CMD_DONE_BIT; // We are fast..
                printf("COMMAND DONE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            }

            if(value & NFC_CTRL_REG_STA_CLR_BIT) {
                printf("Clear interface status\n");
                s->regs[R_NFC_CTRL] &= ~NFC_CTRL_REG_STA_CLR_BIT;
            }

            if(value & NFC_CTRL_REG_CE0_SEL_BIT) {
                printf("Chipselect asserted\n");
            }

            if(value & NFC_CTRL_REG_CE_SAVE_BIT) {
                printf("NFC_CTRL_REG_CE_SAVE_BIT asserted\n");
            }
            break;
       

            printf("nfc whatever write %08x, %08x\n", (int)(addr * 4), value);
//            break;
        case R_NFC_DAT_REG1:
            printf("nfc reg1 write %08x, %08x\n", (int)(addr * 4), value);
            break;
        case R_NFC_DAT_REG2:
            printf("nfc reg2 write %08x, %08x\n", (int)(addr * 4), value);
            break;
        case R_NFC_CMD_LEN_REG:
            printf("nfc cmd len write %08x, %08x\n", (int)(addr * 4), value);
            break;
        case R_NFC_DAT_LEN_REG:
            printf("nfc dat len write %08x, %08x\n", (int)(addr * 4), value);
            break;
        default:
            DB_PRINT("Unimplemented write access reg=" TARGET_FMT_plx " Data: %08x\n", addr * 4, (uint32_t)value64);
    }

    s->regs[addr] = r;
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
    /* Register flash - This is wrong - fix me */
     unsigned long flash_size;
     DriveInfo *dinfo;
    dinfo = drive_get(IF_NONE, 0, 0);
    if (dinfo) {
        s->blk = blk_by_legacy_dinfo(dinfo);
        char data[256];
        blk_pread(s->blk, 0, data, 32);
        printf("First Word: %08x\n", *(uint32_t*)data);

        flash_size = blk_getlength(s->blk);
        if (flash_size != 8*1024*1024 && flash_size != 16*1024*1024 &&
            flash_size != 32*1024*1024) {
            error_report("Invalid flash image size");
        //    exit(1);
        }

        //qdev_prop_set_uint32(dev, "drive", blk);
        //object_property_set_link(OBJECT(dev), "drive", OBJECT(blk), &error_abort);
        /*
         * The original U-Boot accesses the flash at 0xFE000000 instead of
         * 0xFF800000 (if there is 8 MB flash). So remap flash access if the
         * image is smaller than 32 MB.
         *
        chomp_nfc_register(0x100000000ULL - MP_FLASH_SIZE_MAX,
                              "chomptech.flash", flash_size,
                              blk, 0x10000,
                              MP_FLASH_SIZE_MAX / flash_size,
                              2, 0x00BF, 0x236D, 0x0000, 0x0000,
                              0x5555, 0x2AAA, 0);
       */ 
    }
}

static void chomp_nfc_init(Object *obj)
{
    CHOMP_NFCState *s = CHOMP_NFC(obj);

    printf("Instance init\n");

    object_property_add_link(obj, "blk", TYPE_DEVICE,
                             (Object **)&s->blk,
                             object_property_allow_set_link,
                             OBJ_PROP_LINK_STRONG);

    s->as = &address_space_memory;
                    
}

static Property chomp_nfc_properties[] = {
    DEFINE_PROP_DRIVE("drive", CHOMP_NFCState, blk),
    DEFINE_PROP_END_OF_LIST(),
};

static const VMStateDescription vmstate_chomp_nfc = {
    .name = "chomp,nfc",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
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
    .instance_init  = chomp_nfc_init,
};

static void chomp_nfc_register_types(void)
{
    type_register_static(&chomp_nfc_info);
}

type_init(chomp_nfc_register_types)

