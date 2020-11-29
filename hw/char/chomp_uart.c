#include "qemu/osdep.h"
#include "qemu/log.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"
#include "hw/sysbus.h"
#include "qemu/module.h"
#include "chardev/char-fe.h"

#define DUART(x)

#define R_RX            0
#define R_TX            1
#define R_STATUS        2
#define R_CTRL          3
#define R_MAX           4

#define STATUS_RXVALID    0x01
#define STATUS_RXFULL     0x02
#define STATUS_TXEMPTY    0x04
#define STATUS_TXFULL     0x08
#define STATUS_IE         0x10
#define STATUS_OVERRUN    0x20
#define STATUS_FRAME      0x40
#define STATUS_PARITY     0x80

#define CONTROL_RST_TX    0x01
#define CONTROL_RST_RX    0x02
#define CONTROL_IE        0x10

#define TYPE_CHOMP_UART "chomptech,uart"
#define CHOMP_UART(obj) \
    OBJECT_CHECK(ChompUART, (obj), TYPE_CHOMP_UART)

typedef struct ChompUART {
    SysBusDevice parent_obj;

    MemoryRegion mmio;
    CharBackend chr;
    qemu_irq irq;

    uint8_t rx_fifo[8];
    unsigned int rx_fifo_pos;
    unsigned int rx_fifo_len;

    uint32_t regs[R_MAX];
} ChompUART;

static void uart_update_irq(ChompUART *s)
{
    unsigned int irq;

    if (s->rx_fifo_len)
        s->regs[R_STATUS] |= STATUS_IE;

    irq = (s->regs[R_STATUS] & STATUS_IE) && (s->regs[R_CTRL] & CONTROL_IE);
    qemu_set_irq(s->irq, irq);
}

static void uart_update_status(ChompUART *s)
{
    uint32_t r;

    r = s->regs[R_STATUS];
    r &= ~7;
    r |= 1 << 2; /* Tx fifo is always empty. We are fast :) */
    r |= (s->rx_fifo_len == sizeof (s->rx_fifo)) << 1;
    r |= (!!s->rx_fifo_len);
    s->regs[R_STATUS] = r;
}

static void chomp_uart_reset(DeviceState *dev)
{
    uart_update_status(CHOMP_UART(dev));
}

static uint64_t
uart_read(void *opaque, hwaddr addr, unsigned int size)
{
    ChompUART *s = opaque;
    uint32_t r = 0;
    addr >>= 2;
    switch (addr)
    {
        case R_RX:
            r = s->rx_fifo[(s->rx_fifo_pos - s->rx_fifo_len) & 7];
            if (s->rx_fifo_len)
                s->rx_fifo_len--;
            uart_update_status(s);
            uart_update_irq(s);
            qemu_chr_fe_accept_input(&s->chr);
            break;

        default:
            if (addr < ARRAY_SIZE(s->regs))
                r = s->regs[addr];
            DUART(qemu_log("%s addr=%x v=%x\n", __func__, addr, r));
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
    unsigned char ch = value;

    addr >>= 2;
    switch (addr)
    {
        case R_STATUS:
            qemu_log_mask(LOG_GUEST_ERROR, "%s: write to UART STATUS\n",
                          __func__);
            break;

        case R_CTRL:
            if (value & CONTROL_RST_RX) {
                s->rx_fifo_pos = 0;
                s->rx_fifo_len = 0;
            }
            s->regs[addr] = value;
            break;

        case R_TX:
            /* XXX this blocks entire thread. Rewrite to use
             * qemu_chr_fe_write and background I/O callbacks */
            qemu_chr_fe_write_all(&s->chr, &ch, 1);
            s->regs[addr] = value;

            /* hax.  */
            s->regs[R_STATUS] |= STATUS_IE;
            break;

        default:
            DUART(printf("%s addr=%x v=%x\n", __func__, addr, value));
            if (addr < ARRAY_SIZE(s->regs))
                s->regs[addr] = value;
            break;
    }
    uart_update_status(s);
    uart_update_irq(s);
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

    /* Got a byte.  */
    if (s->rx_fifo_len >= 8) {
        printf("WARNING: UART dropped char.\n");
        return;
    }
    s->rx_fifo[s->rx_fifo_pos] = *buf;
    s->rx_fifo_pos++;
    s->rx_fifo_pos &= 0x7;
    s->rx_fifo_len++;

    uart_update_status(s);
    uart_update_irq(s);
}

static int uart_can_rx(void *opaque)
{
    ChompUART *s = opaque;

    return s->rx_fifo_len < sizeof(s->rx_fifo);
}

static void uart_event(void *opaque, QEMUChrEvent event)
{

}

static void chomp_uart_realize(DeviceState *dev, Error **errp)
{
    ChompUART *s = CHOMP_UART(dev);

    qemu_chr_fe_set_handlers(&s->chr, uart_can_rx, uart_rx,
                             uart_event, NULL, s, NULL, true);
}

static void chomp_uart_init(Object *obj)
{
    ChompUART *s = CHOMP_UART(obj);

    sysbus_init_irq(SYS_BUS_DEVICE(obj), &s->irq);

    memory_region_init_io(&s->mmio, obj, &uart_ops, s,
                          "chomp.uart", R_MAX * 4);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);
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
