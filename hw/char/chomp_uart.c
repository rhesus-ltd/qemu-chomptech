#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/timer.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"
#include "hw/sysbus.h"
#include "qemu/module.h"
#include "chardev/char-fe.h"
#include "hw/registerfields.h"
#include "hw/ptimer.h"

#ifndef CHOMP_UART_ERR_DEBUG
#define CHOMP_UART_ERR_DEBUG 1
#endif

extern AddressSpace address_space_memory;

#define DB_PRINT(...) do { \
        if (CHOMP_UART_ERR_DEBUG) { \
            fprintf(stderr,  ": %s: ", __func__); \
            fprintf(stderr, ## __VA_ARGS__); \
        } \
    } while (0)

REG32(UART_CFG1,        0x000)
REG32(UART_CFG2,        0x004)
REG32(UART_DATA_CFG,    0x008)
REG32(UART_BUF_TRSHLD,  0x00C)


//REG_UART_CONFIG_1
#define TX_ADR_CLR                      (1UL << 31)
#define RX_ADR_CLR                      (1 << 30)
#define RX_STA_CLR                      (1 << 29)
#define TX_STA_CLR                      (1 << 28)
#define ENDIAN_SEL_BIG                  (1 << 27)
#define PARITY_EN                       (1 << 26)
#define PARITY_SEL_EVEN                 (1 << 25)
#define RX_TIMEOUT_EN                   (1 << 23)
#define BAUD_RATE_ADJ_EN                (1 << 22)
#define UART_INTERFACE_EN               (1 << 21)
#define URD_SEL_INVERSELY               (1 << 17)
#define UTD_SEL_INVERSELY               (1 << 16)
#define BAUD_RATE_DIV                   (0)         //[15:0]


//REG_UART_CONFIG_2
#define TX_TH_INT_STA                   (1 << 31)
#define RX_TH_INT_STA                   (1 << 30)
#define TX_TH_INT_EN                    (1 << 29)
#define RX_TH_INT_EN                    (1 << 28)
#define TX_END_INT_EN                   (1 << 27)
#define RX_BUF_FULL_INT_EN              (1 << 26)
#define TX_BUF_EMP_INT_EN               (1 << 24)
#define RX_ERROR_INT_EN                 (1 << 23)
#define TIMEOUT_INT_EN                  (1 << 22)
#define RX_READY_INT_EN                 (1 << 21)
#define TX_END                          (1 << 19)
#define RX_TIMEROUT_STA                 (1 << 18)
#define RX_READY                        (1 << 17)   //?????
#define TX_BYT_CNT_VLD                  (1 << 16)
#define TX_BYT_CNT                      (4)         //[15:4]
#define RX_ERROR                        (1 << 3)
#define RX_TIMEROUT                     (1 << 2)
#define RX_BUF_FULL                     (1 << 1)
#define TX_FIFO_EMPTY                   (1 << 0)


//REG_UART_DATA_CONFIG
#define BYT_LEFT                        (23)        //[24:23]
#define TX_ADDR                         (18)        //[22:18]
#define RX_ADDR                         (13)        //[17:13]
#define TX_BYT_SUM                      (0)         //[12:0]


//REG_UART_TXRX_BUF_THRESHOLD
#define TX_TH_CNT                       (17)        //[21:17]
#define RX_TH_CNT                       (12)        //[16:12]
#define TX_TH_CLR                       (1 << 11)
#define TX_TH_CFG                       (6)         //[10:6]
#define RX_TH_CLR                       (1 << 5)
#define RX_TH_CFG                       (0)         //[4:0]

#define CHOMP_UART_MMIO_SIZE     0x1000
#define CHOMP_UART_NUM_REGS      (CHOMP_UART_MMIO_SIZE / 4)

#define TYPE_CHOMP_UART "chomptech,uart"
#define CHOMP_UART(obj) \
    OBJECT_CHECK(ChompUART, (obj), TYPE_CHOMP_UART)

typedef struct ChompUART {
    SysBusDevice parent_obj;

    MemoryRegion mmio;
    AddressSpace *as;
    CharBackend chr;
    qemu_irq irq;

    uint8_t rx_fifo[8];
    unsigned int rx_fifo_pos;
    unsigned int rx_fifo_len;

    uint32_t      freq_hz;
    ptimer_state *ptimer;

    uint32_t regs[CHOMP_UART_NUM_REGS];
} ChompUART;

static void uart_update_irq(ChompUART *s)
{
    unsigned int irq;
/*
    if (s->rx_fifo_len)
        s->regs[R_STATUS] |= STATUS_IE;
*/
    irq = (s->regs[R_UART_CFG2] & RX_TH_INT_STA) && (s->regs[R_UART_CFG2] & RX_TH_INT_EN);
    qemu_set_irq(s->irq, irq);
}

static void uart_update_status(ChompUART *s)
{
    uint32_t r;

    r = s->regs[R_UART_CFG2];
    //r &= ~7;
    r |= TX_FIFO_EMPTY | TX_END; // Tx fifo is always empty.
    //r |= (s->rx_fifo_len == sizeof (s->rx_fifo)) << 1;
    //r |= (!!s->rx_fifo_len);
    //s->regs[R_UART_CFG2] = r;
}

static void chomp_uart_reset(DeviceState *dev)
{
    uart_update_status(CHOMP_UART(dev));
}

static void timer_hit(void *opaque)
{
    ChompUART *s = opaque;
    //DB_PRINT("Indiaaaaaaaaner!!!!! timer timer timer %08x\n", s->regs[R_UART_DATA_CFG]);
    if(s->regs[R_UART_CFG2] & TIMEOUT_INT_EN) {
        s->regs[R_UART_CFG2] = RX_TIMEROUT_STA;
    }
    s->regs[R_UART_CFG2] |= RX_TIMEROUT;
}

static uint64_t
uart_read(void *opaque, hwaddr addr, unsigned int size)
{
    ChompUART *s = opaque;

    uint32_t r = 0;
    addr >>= 2;

//    uint32_t txaddr = 0;
//    uint32_t rxaddr = 0;

    switch (addr)
    {
        case R_UART_CFG1:
            //r = s->regs[R_UART_CFG1] &= ~RX_TIMEOUT_EN;
            break;

        case R_UART_CFG2:
  //        DB_PRINT("R_UART_CFG2\n");
            r = s->regs[R_UART_CFG2];
            break;
        case R_UART_DATA_CFG:
           // DB_PRINT("R_UART_DATA_CFG\n");
            r = s->regs[R_UART_DATA_CFG];
        //    txaddr = (r >> TX_ADDR) & 0x1F;
        //    rxaddr = (r >> RX_ADDR) & 0x1F;
         //   bytes_left = (r >> BYT_LEFT) & 0x3;
         //   DB_PRINT("Read R_UART_DATA_CFG TX: %08x, RX: %08x, BL: %02x\n", txaddr, rxaddr, bytes_left); 
            break;
        case R_UART_BUF_TRSHLD:
            DB_PRINT("R_UART_BUF_TRSHLD\n");
            r = s->regs[R_UART_BUF_TRSHLD];
            break;
        default:
            break;
    }

    return r;
}

static void
uart_write(void *opaque, hwaddr addr,
           uint64_t val64, unsigned int size)
{
    ChompUART *s = opaque;
    uint32_t value = val64;

//    uint32_t txaddr = 0;
//    uint32_t rxaddr = 0; 

    unsigned char ch =  address_space_ldq_le(s->as, /* FIXME: */ 0x0802fa80, MEMTXATTRS_UNSPECIFIED, NULL);

    addr >>= 2;
    switch (addr)
    {
        case R_UART_CFG1:
            //DB_PRINT("\nR_UART_CFG1: %08x\n", value);
            if(value & RX_STA_CLR)  {
                //DB_PRINT("Clear RX Status\n");
                //s->rx_fifo_pos = 0;
            }
            if(value & RX_ADR_CLR) {
                //DB_PRINT("RX_ADDR_CLR\n");
                s->regs[R_UART_DATA_CFG] &= ~(0x7 << BYT_LEFT);
                s->rx_fifo_pos = 0;
            }
            if(value & RX_TIMEOUT_EN) {
                //DB_PRINT("Timeout enable ////////////////////////////////////////////////////////////////////////////////\n");
                qemu_chr_fe_accept_input(&s->chr);
                s->regs[R_UART_CFG1] |= RX_TIMEOUT_EN;

                s->regs[R_UART_CFG2] &= RX_TH_INT_EN;
                s->regs[R_UART_CFG2] &= ~RX_TIMEROUT;

                ptimer_transaction_begin(s->ptimer);
                ptimer_set_limit(s->ptimer, 1000, 1);
                ptimer_run(s->ptimer, 1);
                ptimer_transaction_commit(s->ptimer);
                //s->rx_fifo_pos = 0;
            } else {
                //DB_PRINT("Timeout disable ////////////////////////////////////////////////////////////////////////////////\n");
                //s->regs[R_UART_CFG1] &= ~RX_TIMEOUT_EN;
            }
            if(value & UART_INTERFACE_EN) {
            //    DB_PRINT("Enable UART Interface **********************************\n");
            }
            break;
        case R_UART_CFG2:
            //if(value == 0x00010010) {
            //    return;
            //}
            //DB_PRINT("\nR_UART_CFG2: %08x\n", value);
            //DB_PRINT("PRE: REG_UART_CFG2: %08" PRIx32 ", new: %08x\n", s->regs[R_UART_CFG2], value);
            if(value & TX_BYT_CNT_VLD) {
               // DB_PRINT("\nR_UART_CFG2: %08x\n", value);
                qemu_chr_fe_write_all(&s->chr, &ch, 1);
            }

            if(value & RX_TH_INT_EN) {
               //DB_PRINT("Write RX_TH_INT_EN !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
               s->regs[R_UART_CFG2] |= RX_TH_INT_EN;
            } else {
               //DB_PRINT("NO RX_TH_INT_EN !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            }

            if(value & ~(RX_BUF_FULL | RX_TIMEROUT | TIMEOUT_INT_EN | RX_ERROR | RX_TH_INT_STA | TX_TH_INT_STA)) {
                //DB_PRINT("addr: %08" HWADDR_PRIx " data: %08" PRIx32 "\n", addr * 4, value); 
            }

            if(value & RX_BUF_FULL) { // Write clear
                //DB_PRINT("Write RX_BUF_FULL");
                s->regs[R_UART_CFG2] &= ~RX_BUF_FULL;
            }

            if(value & RX_TIMEROUT) {
                //DB_PRINT("Write RX_TIMEOUT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                s->regs[R_UART_CFG2] &= ~RX_TIMEROUT;
                ptimer_transaction_begin(s->ptimer);
                ptimer_stop(s->ptimer);
                ptimer_transaction_commit(s->ptimer);
                //s->rx_fifo_pos = 0;
            }

            if(value & TIMEOUT_INT_EN) {
                //DB_PRINT("Write TIMEOUT_INT_EN !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                s->regs[R_UART_CFG2] |= TIMEOUT_INT_EN; 
            }

            if(value & RX_ERROR) {
                s->regs[R_UART_CFG2] &= ~RX_ERROR;
            }

            if(value & RX_TH_INT_STA) {
                //DB_PRINT("Write RX_TH_INT_STA +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"); 
                s->regs[R_UART_CFG2] &= ~RX_TH_INT_STA;
                s->rx_fifo_pos = 0; 
            }

            if(value & TX_TH_INT_STA) {
                s->regs[R_UART_CFG2] &= ~TX_TH_INT_STA;
            }

            //DB_PRINT("POST: UART_CFG2: %08" PRIx32 "\n", s->regs[R_UART_CFG2]);
            break;


        case R_UART_DATA_CFG:
            bytes_left = (value >> BYT_LEFT) & 0x3;
//            txaddr = (value >> TX_ADDR) & 0x1F;
//            rxaddr = (value >> RX_ADDR) & 0x1F;
            //DB_PRINT("Write R_UART_DATA_CFG: TX: %08x, RX: %08x, BL: %02x\n", txaddr, rxaddr, bytes_left);
            s->regs[R_UART_DATA_CFG] = value;
            break;
        case R_UART_BUF_TRSHLD:
            DB_PRINT("************************************************* UART Buff thres: %08x\n", value);
            break;
        default:
            //DB_PRINT("%s addr=%x v=%x\n", __func__, addr, value);
            if (addr < ARRAY_SIZE(s->regs))
                s->regs[addr] = value;
            break;
    }
   // uart_update_status(s);
   // uart_update_irq(s);
   
   //DB_PRINT("addr: %08" HWADDR_PRIx " data: %08" PRIx32 "\n", addr * 4, value);
}

static const MemoryRegionOps uart_ops = {
    .read = uart_read,
    .write = uart_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 4
    }
};

static Property chomp_uart_properties[] = {
    DEFINE_PROP_CHR("chardev", ChompUART, chr),
    DEFINE_PROP_END_OF_LIST(),
};

static void uart_rx(void *opaque, const uint8_t *buf, int size)
{
    ChompUART *s = opaque;
    uint32_t rx_word_count = 0;
    uint32_t rx_byte_count = 0;

    //address_space_stl_le(s->as, 0x0802fabc + s->rx_fifo_pos, *buf, MEMTXATTRS_UNSPECIFIED, NULL); 
    address_space_stl_le(s->as, 0x0802fafc + s->rx_fifo_pos, *buf, MEMTXATTRS_UNSPECIFIED, NULL); 
  //  DB_PRINT("Set fifo (pos: %08x) at: %08x\n", s->rx_fifo_pos, 0x0802fafc + s->rx_fifo_pos);
 
   

    s->rx_fifo_pos++;
    s->rx_fifo_pos &= 0x3F;
    s->rx_fifo_len++;
 rx_word_count = s->rx_fifo_pos >> 2;
    rx_byte_count = s->rx_fifo_pos & 0x3;
    //DB_PRINT("Register fifo (pos: %d) byte cnt: %02x, word cnt: %02x\n", s->rx_fifo_pos, rx_byte_count, rx_word_count);
    
    s->regs[R_UART_DATA_CFG] = (rx_word_count << RX_ADDR | rx_byte_count << BYT_LEFT);

    //DB_PRINT("Got char\n");

//    if(size == 1) {
//        address_space_stl_le(s->as, 0x0802fafc, *buf, MEMTXATTRS_UNSPECIFIED, NULL);
//    }

    //if(s->rx_fifo_pos == 4) {
        s->regs[R_UART_CFG2] |= RX_TH_INT_STA;
    //}
   

   
    uart_update_status(s);
    uart_update_irq(s);
}

static int uart_can_rx(void *opaque)
{
  //  ChompUART *s = opaque;

    return 1; //s->rx_fifo_len < sizeof(s->rx_fifo);
}

static void uart_event(void *opaque, QEMUChrEvent event)
{

}

static void chomp_uart_realize(DeviceState *dev, Error **errp)
{
    ChompUART *s = CHOMP_UART(dev);

    qemu_chr_fe_set_handlers(&s->chr, uart_can_rx, uart_rx,
                             uart_event, NULL, s, NULL, true);
    s->freq_hz = 26000;

    if (s->freq_hz == 0) {
        DB_PRINT("clock-frequency property must be provided.\n");
        return;
    }

    s->ptimer = ptimer_init(timer_hit, s, PTIMER_POLICY_DEFAULT);
    ptimer_transaction_begin(s->ptimer);
    ptimer_set_freq(s->ptimer, s->freq_hz);
    ptimer_transaction_commit(s->ptimer);
}



static void chomp_uart_init(Object *obj)
{
    ChompUART *s = CHOMP_UART(obj);

    sysbus_init_irq(SYS_BUS_DEVICE(obj), &s->irq);

    memory_region_init_io(&s->mmio, obj, &uart_ops, s,
                          "chomp.uart", CHOMP_UART_NUM_REGS * 4);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);

    s->rx_fifo_pos = 0;//0x3C; 
    s->as = &address_space_memory;
}


static void chomp_uart_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = chomp_uart_reset;
    dc->realize = chomp_uart_realize;
    device_class_set_props(dc, chomp_uart_properties);
}

static const TypeInfo chomp_uart_info = {
    .name          = TYPE_CHOMP_UART,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(ChompUART),
    .instance_init = chomp_uart_init,
    .class_init    = chomp_uart_class_init,
};

static void chomp_uart_register_types(void)
{
    type_register_static(&chomp_uart_info);
}

type_init(chomp_uart_register_types)
