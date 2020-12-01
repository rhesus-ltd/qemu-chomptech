/*
 * Status and system control registers for CHOMPTECH CHOMP Platform
 * 
 * Author: Sven Huerlimann
 *
 * Based on hw/misc/zynq_sclr.c by Michal Simek, PetaLogix Pty Ltd.
 * Based on hw/arm_sysctl.c, written by Paul Brook
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 */

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

#ifdef CONFIG_FDT
#include "qemu/config-file.h"
#endif

#ifndef CHOMP_SLCR_ERR_DEBUG
#define CHOMP_SLCR_ERR_DEBUG 1
#endif

#define DB_PRINT(...) do { \
        if (CHOMP_SLCR_ERR_DEBUG) { \
            fprintf(stderr,  ": %s: ", __func__); \
            fprintf(stderr, ## __VA_ARGS__); \
        } \
    } while (0)

#define CHOMPTECH_LOCK_KEY 0x767b
#define CHOMPTECH_UNLOCK_KEY 0xdf0d

REG32(CHIP_ID, 0x000)
REG32(CLOCK_DIV1, 0x004)
REG32(CLOCK_DIV2, 0x008)
REG32(CLOCK_RST_EN, 0x00c)

REG32(DACS_HCLK, 0x010)

REG32(IMG_SENSOR_CFG, 0x014)

REG32(TIMER1_CFG, 0x018)
REG32(TIMER2_CFG, 0x01c)

#define TIMER_STATUS_BIT           (1 << 29)  //0: Hasn't generated an interrupt
                                              //1: Has generated an interrupt.
                                              //Note: cleared after being read.
#define TIMER_CLEAR_BIT            (1 << 28)  //1: To clear timer pulse.
#define TIMER_LOAD_BIT             (1 << 27)  //1: To load new count value.
#define TIMER_ENABLE_BIT           (1 << 26)  //0: To disable the Timer
                                              //1: To enable the Timer

#define TIMER_VALUE_MASK            0x3FFFFFF

REG32(PWM0_CTRL, 0x02c)
REG32(PWM1_CTRL, 0x030)

REG32(INT_IRQ, 0x034)
REG32(INT_FIQ, 0x038)
REG32(WGPIO_POL, 0x03c)
REG32(WGPIO_CLR, 0x040)
REG32(WGPIO_EN, 0x044)
REG32(WGPIO_STA, 0x48)
REG32(INT_SYSCTL, 0x4c)

REG32(POWER_CTRL, 0x050)
REG32(BOOTUP_MODE, 0x054)
REG32(MUL_FUNC_CTRL, 0x058)

REG32(ANALOG_CTRL1, 0x05c)
REG32(USB_REG_CFG, 0x060)
REG32(ANALOG_CTRL2, 0x064)
REG32(ANALOG_CTRL3, 0x068)
REG32(ANALOG_CTRL4, 0x06c)

REG32(SHARE_PIN_CTRL, 0x074)

REG32(GPIO_DIR_1, 0x07C)
REG32(GPIO_OUT_1, 0x080)

REG32(GPIO_DIR_2, 0x084)
REG32(GPIO_OUT_2, 0x088)

REG32(GPIO_IN_1, 0x0BC)
REG32(GPIO_IN_2, 0x0C0)

REG32(INT_STA, 0x0CC)
REG32(ANALOG, 0x098)

REG32(GPIO_PULL_UD_1, 0x09C)
REG32(GPIO_PULL_UD_2, 0x0A0)

#define DDRIOB_LENGTH 14

#define CHOMP_SLCR_MMIO_SIZE     0x1000
#define CHOMP_SLCR_NUM_REGS      (CHOMP_SLCR_MMIO_SIZE / 4)

#define CHOMP_SLCR_NUM_CPUS 2

#define FPGA_RST_VALID_BITS 0x01f33F0F
/* The action of the FPGA_RST_CTRL register on the reset pins
 * should be inverted as the resets are active low.
 */
#define FPGA_RST_INVERT_BITS 0x0000000F
#define A9_CPU_RST_CTRL_RST_SHIFT 0

#define TYPE_CHOMP_SLCR "chomptech,chomp_slcr"
#define CHOMP_SLCR(obj) OBJECT_CHECK(ChompSLCRState, (obj), TYPE_CHOMP_SLCR)

typedef struct ChompSLCRState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    qemu_irq cpu_resets[CHOMP_SLCR_NUM_CPUS];

    qemu_irq timer_irq;

    uint32_t regs[CHOMP_SLCR_NUM_REGS];

    Clock *clk;

    Clock *pll_clk;
    Clock *dac_clk;
    Clock *adc_clk;
    
    uint32_t      freq_hz;
    ptimer_state *ptimer;
} ChompSLCRState;


/**
 * Compute and set the ouputs clocks periods.
 */
static void chomp_slcr_compute_clocks(ChompSLCRState *s)
{
    uint64_t clk = clock_get(s->clk);

    /* consider outputs clocks are disabled while in reset */
    if (device_is_in_reset(DEVICE(s))) {
        clk = 0;
    }

    // PLL clock just follows
    //clock_set_source(s->pll_clk, s->clk);
}

/**
 * Propagate the outputs clocks.
 * chomp_slcr_compute_clocks() should have been called before
 * to configure them.
 */
static void chomp_slcr_propagate_clocks(ChompSLCRState *s)
{
    clock_propagate(s->pll_clk);
    clock_propagate(s->dac_clk);
    clock_propagate(s->adc_clk);
}

static void chomp_slcr_clk_callback(void *opaque)
{
    ChompSLCRState *s = (ChompSLCRState *) opaque;
    chomp_slcr_compute_clocks(s);
    chomp_slcr_propagate_clocks(s);
}

static void chomp_slcr_reset_init(Object *obj, ResetType type)
{
    ChompSLCRState *s = CHOMP_SLCR(obj);
    QemuOpts *opts = qemu_find_opts_singleton("boot-opts");
    int boot_mode;

    DB_PRINT("RESET Init\n");

    boot_mode = qemu_opt_get_number(opts, "mode", 0);

    s->regs[R_CHIP_ID] = 0x3900; // FIXME: add as property
    s->regs[R_GPIO_DIR_1] = 0x0000; 
    s->regs[R_GPIO_DIR_2] = 0x0000; 
    s->regs[R_GPIO_IN_1] = 0x1000; // FIXED to UART boot mode
    s->regs[R_TIMER1_CFG] = 0x02000000; 
}

static void chomp_slcr_reset_hold(Object *obj)
{
    ChompSLCRState *s = CHOMP_SLCR(obj);

    DB_PRINT("RESET Hold\n");
    /* will disable all output clocks */
    chomp_slcr_compute_clocks(s);
    chomp_slcr_propagate_clocks(s);
}

static void chomp_slcr_reset_exit(Object *obj)
{
    ChompSLCRState *s = CHOMP_SLCR(obj);

    DB_PRINT("RESET Exit\n");
    /* will compute output clocks according to ps_clk and registers */
    chomp_slcr_compute_clocks(s);
    chomp_slcr_propagate_clocks(s);
}

static bool chomp_slcr_check_offset(hwaddr offset, bool rnw)
{
    switch (offset) {
    case R_GPIO_OUT_1:
    case R_GPIO_OUT_2:
    case R_GPIO_DIR_1:
    case R_GPIO_DIR_2:
    case R_INT_STA:
        return !rnw; // Write only 
    case R_CHIP_ID:
    case R_GPIO_IN_1:
    case R_GPIO_IN_2:
        return rnw;/* read only */
    case R_CLOCK_RST_EN:
    case R_SHARE_PIN_CTRL:
    case R_CLOCK_DIV1:
    case R_CLOCK_DIV2:
    case R_TIMER1_CFG:
        return true;
    default:
        return false;
    }
}

static uint64_t chomp_slcr_read(void *opaque, hwaddr offset,
    unsigned size)
{
    ChompSLCRState *s = opaque;
    offset /= 4;
    uint32_t ret = s->regs[offset];
    uint32_t count = 0;

    if (!chomp_slcr_check_offset(offset, true)) {
        qemu_log_mask(LOG_GUEST_ERROR, "chomp_slcr: Invalid read access to "
                      " addr %" HWADDR_PRIx "\n", offset * 4);
    }

    switch (offset) {
    case R_CHIP_ID:
        DB_PRINT("Read R_CHIP_ID\n");
        break;
    case R_CLOCK_DIV1:
        DB_PRINT("Read R_CLOCK_DIV1\n");
        break;
    case R_CLOCK_DIV2:
        DB_PRINT("Read R_CLOCK_DIV2\n");
        break;
    case R_CLOCK_RST_EN:
        DB_PRINT("Read R_CHIP_I\n");
        break; 
    case R_TIMER1_CFG:
       // DB_PRINT("Read R_TIMER1_CFG\n");
        //ret |= 0x20000000;
        count = ptimer_get_count(s->ptimer);
        s->regs[R_TIMER1_CFG] &= ~0x3FFFFFF;
        s->regs[R_TIMER1_CFG] |= (count & 0x3FFFFFF);
        ret = s->regs[R_TIMER1_CFG];
        break;   
    case R_TIMER2_CFG:
        DB_PRINT("Read R_TIMER2_CFG\n");
        break;  
    case R_SHARE_PIN_CTRL:
        DB_PRINT("Read R_SHARE_PIN_CTRL\n");
        break; 
    case R_GPIO_DIR_1:
        DB_PRINT("Read R_GPIO_DIR_1\n");
        break;
    case R_GPIO_DIR_2:
        DB_PRINT("Read R_GPIO_DIR_2\n");
        break;
    case R_GPIO_IN_1:
        DB_PRINT("Read R_GPIO_IN_1\n");
        break;
    case R_GPIO_IN_2:
        DB_PRINT("Read R_GPIO_IN_2\n");
        break; 
    case R_GPIO_OUT_1:
        DB_PRINT("Read R_GPIO_OUT_1\n");
        break;
    case R_GPIO_OUT_2:
        DB_PRINT("Read R_GPIO_OUT_2\n");
        break;
    case R_GPIO_PULL_UD_1:
        DB_PRINT("Read R_GPIO_PULL_UD_1\n");
        break;
    case R_GPIO_PULL_UD_2:
        DB_PRINT("Read R_GPIO_PULL_UD_2\n");
        break;
    case R_INT_STA:
        break;
    }

    //DB_PRINT("addr: %08" HWADDR_PRIx " data: %08" PRIx32 "\n", offset * 4, ret);
    return ret;
}

static void chomp_slcr_write(void *opaque, hwaddr offset,
                          uint64_t val, unsigned size)
{
    ChompSLCRState *s = (ChompSLCRState *)opaque;
    offset /= 4;

    if (!chomp_slcr_check_offset(offset, false)) {
        qemu_log_mask(LOG_GUEST_ERROR, "chomp_slcr: Invalid write access to "
                      "addr %" HWADDR_PRIx "\n", offset * 4);
        return;
    }

    switch (offset) {
    case R_CLOCK_DIV1:
        DB_PRINT("Write R_CLOCK_DIV1\n");
        break;
    case R_CLOCK_DIV2:
        DB_PRINT("Write R_CLOCK_DIV2\n");
        break;
    case R_CLOCK_RST_EN:
        DB_PRINT("Write R_CLOCK_RST_EN\n");
        break; 
    case R_TIMER1_CFG:
        ptimer_transaction_begin(s->ptimer);
        if (val & TIMER_CLEAR_BIT) {
            DB_PRINT("Clear timer pulse\n");
            s->regs[R_TIMER1_CFG] &= ~TIMER_STATUS_BIT;
        }
        if (val & TIMER_LOAD_BIT) {
            DB_PRINT("Load new timer value\n");
            // load new timer value
            if (s->regs[R_TIMER1_CFG] & TIMER_ENABLE_BIT) {
                ptimer_stop(s->ptimer);
                s->regs[R_TIMER1_CFG] &= ~TIMER_ENABLE_BIT;
            }
            uint32_t tvalue = val & 0x3FFFFFF;
            ptimer_set_limit(s->ptimer, tvalue + 1, 1);
        }
        if (val & TIMER_ENABLE_BIT) {
            DB_PRINT("Enable timer\n");
            // start timer
            ptimer_run(s->ptimer, 1);
            s->regs[R_TIMER1_CFG] &= ~TIMER_STATUS_BIT;
            s->regs[R_TIMER1_CFG] |= TIMER_ENABLE_BIT;
        } else {
            DB_PRINT("Disable timer\n");
            // stop timer
            ptimer_stop(s->ptimer);
            s->regs[R_TIMER1_CFG] &= ~TIMER_ENABLE_BIT; 
        }
        ptimer_transaction_commit(s->ptimer);

        // Flag is cleared on read
        s->regs[R_TIMER1_CFG] &= ~0x20000000;
        break; 
    case R_TIMER2_CFG:
        DB_PRINT("Write R_TIMER2_CFG\n");
        break;  
    case R_SHARE_PIN_CTRL:
        DB_PRINT("Write R_SHARE_PIN_CTRL\n");
        break; 
    case R_GPIO_DIR_1:
        DB_PRINT("Write R_GPIO_DIR_1\n");
        break;
    case R_GPIO_DIR_2:
        DB_PRINT("Write R_GPIO_DIR_2\n");
        break;
    case R_GPIO_IN_1:
        DB_PRINT("Write R_GPIO_IN_1\n");
        break;
    case R_GPIO_IN_2:
        DB_PRINT("Write R_GPIO_IN_2\n");
        break;
    case R_GPIO_OUT_1:
        DB_PRINT("Write R_GPIO_OUT_1\n");
        break;
    case R_GPIO_OUT_2:
        DB_PRINT("Write R_GPIO_OUT_2\n");
        break;
    case R_GPIO_PULL_UD_1:
        DB_PRINT("Write R_GPIO_PULL_UD_1\n");
        break;
    case R_GPIO_PULL_UD_2:
        DB_PRINT("Write R_GPIO_PULL_UD_2\n");
        break;
    }

    DB_PRINT("addr: %08" HWADDR_PRIx " data: %08" PRIx64 "\n", offset * 4, val);

    s->regs[offset] = val;
}

static const MemoryRegionOps slcr_ops = {
    .read = chomp_slcr_read,
    .write = chomp_slcr_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static int timer_irq_state(ChompSLCRState *t)
{
    bool irq = t->regs[R_TIMER1_CFG] & TIMER_STATUS_BIT;
    return irq;
}

static void timer_hit(void *opaque)
{
    ChompSLCRState *s = opaque;
//    const uint64_t tvalue = s->regs[R_TIMER1_CFG] & TIMER_VALUE_MASK;
    s->regs[R_TIMER1_CFG] |= TIMER_STATUS_BIT;
    qemu_set_irq(s->timer_irq, timer_irq_state(s));
}

static void chomp_slcr_realize(DeviceState *dev, Error **errp)
{
    ChompSLCRState *s = CHOMP_SLCR(dev);

    s->freq_hz = 26000;

    if (s->freq_hz == 0) {
        error_setg(errp, "\"clock-frequency\" property must be provided.");
        return;
    }

    s->ptimer = ptimer_init(timer_hit, s, PTIMER_POLICY_DEFAULT);
    ptimer_transaction_begin(s->ptimer);
    ptimer_set_freq(s->ptimer, s->freq_hz);
    ptimer_transaction_commit(s->ptimer);
}

static const ClockPortInitArray chomp_slcr_clocks = {
    QDEV_CLOCK_IN(ChompSLCRState, clk, chomp_slcr_clk_callback),
    QDEV_CLOCK_OUT(ChompSLCRState, pll_clk),
    QDEV_CLOCK_OUT(ChompSLCRState, dac_clk),
    QDEV_CLOCK_OUT(ChompSLCRState, adc_clk),
    QDEV_CLOCK_END
};

static void chomp_slcr_init(Object *obj)
{
    ChompSLCRState *s = CHOMP_SLCR(obj);

    memory_region_init_io(&s->iomem, obj, &slcr_ops, s, "chomp.slcr",
                          CHOMP_SLCR_MMIO_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

    qdev_init_gpio_out(DEVICE(obj), s->cpu_resets, CHOMP_SLCR_NUM_CPUS);
    qdev_init_clocks(DEVICE(obj), chomp_slcr_clocks);
}

static const VMStateDescription vmstate_chomp_slcr = {
    .name = "chomp_slcr",
    .version_id = 3,
    .minimum_version_id = 2,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(regs, ChompSLCRState, CHOMP_SLCR_NUM_REGS),
        VMSTATE_CLOCK_V(clk, ChompSLCRState, 3),
        VMSTATE_CLOCK_V(pll_clk, ChompSLCRState, 3),
        VMSTATE_CLOCK_V(dac_clk, ChompSLCRState, 3),
        VMSTATE_CLOCK_V(adc_clk, ChompSLCRState, 3),
        VMSTATE_END_OF_LIST()
    }
};

static void chomp_slcr_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    ResettableClass *rc = RESETTABLE_CLASS(klass);

    dc->vmsd = &vmstate_chomp_slcr;
    dc->realize = chomp_slcr_realize;
    rc->phases.enter = chomp_slcr_reset_init;
    rc->phases.hold  = chomp_slcr_reset_hold;
    rc->phases.exit  = chomp_slcr_reset_exit;
}

static const TypeInfo chomp_slcr_info = {
    .class_init = chomp_slcr_class_init,
    .name  = TYPE_CHOMP_SLCR,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size  = sizeof(ChompSLCRState),
    .instance_init = chomp_slcr_init,
};

static void chomp_slcr_register_types(void)
{
    type_register_static(&chomp_slcr_info);
}

type_init(chomp_slcr_register_types)
