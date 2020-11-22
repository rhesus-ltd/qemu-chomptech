/*
 * Status and system control registers for CHOMPTECH CHOMP Platform
 *
 * Copyright (c) 2011 Michal Simek <monstr@monstr.eu>
 * Copyright (c) 2012 PetaLogix Pty Ltd.
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

REG32(INT_IRQ, 0x034)
REG32(INT_FIQ, 0x038)

REG32(POWER_CTRL, 0x050)

REG32(INT_STA, 0x0CC)

REG32(SYSCTL, 0x04C)

REG32(SHARE_PIN_CRTL, 0x074)

REG32(GPIO_DIR_1, 0x07C)
REG32(GPIO_OUT_1, 0x080)

REG32(GPIO_DIR_2, 0x084)
REG32(GPIO_OUT_2, 0x088)

REG32(GPIO_IN_1, 0x0BC)
REG32(GPIO_IN_2, 0x0C0)

REG32(ANALOG, 0x098)

REG32(GPIO_PULL_UD_1, 0x09C)
REG32(GPIO_PULL_UD_2, 0x0A0)

/* fields for [ARM|DDR|IO]_PLL_CTRL registers 
    FIELD(xxx_PLL_CTRL, PLL_RESET, 0, 1)
    FIELD(xxx_PLL_CTRL, PLL_PWRDWN, 1, 1)
    FIELD(xxx_PLL_CTRL, PLL_BYPASS_QUAL, 3, 1)
    FIELD(xxx_PLL_CTRL, PLL_BYPASS_FORCE, 4, 1)
    FIELD(xxx_PLL_CTRL, PLL_FPDIV, 12, 7)
REG32(PLL_STATUS, 0x10c)
REG32(ARM_PLL_CFG, 0x110)
REG32(DDR_PLL_CFG, 0x114)
REG32(IO_PLL_CFG, 0x118)

REG32(ARM_CLK_CTRL, 0x120)
REG32(DDR_CLK_CTRL, 0x124)
REG32(DCI_CLK_CTRL, 0x128)
REG32(APER_CLK_CTRL, 0x12c)
REG32(USB0_CLK_CTRL, 0x130)
REG32(USB1_CLK_CTRL, 0x134)
REG32(GEM0_RCLK_CTRL, 0x138)
REG32(GEM1_RCLK_CTRL, 0x13c)
REG32(GEM0_CLK_CTRL, 0x140)
REG32(GEM1_CLK_CTRL, 0x144)
REG32(SMC_CLK_CTRL, 0x148)
REG32(LQSPI_CLK_CTRL, 0x14c)
REG32(SDIO_CLK_CTRL, 0x150)
REG32(UART_CLK_CTRL, 0x154)
    FIELD(UART_CLK_CTRL, CLKACT0, 0, 1)
    FIELD(UART_CLK_CTRL, CLKACT1, 1, 1)
    FIELD(UART_CLK_CTRL, SRCSEL,  4, 2)
    FIELD(UART_CLK_CTRL, DIVISOR, 8, 6)
REG32(SPI_CLK_CTRL, 0x158)
REG32(CAN_CLK_CTRL, 0x15c)
REG32(CAN_MIOCLK_CTRL, 0x160)
REG32(DBG_CLK_CTRL, 0x164)
REG32(PCAP_CLK_CTRL, 0x168)
REG32(TOPSW_CLK_CTRL, 0x16c)

#define FPGA_CTRL_REGS(n, start) \
    REG32(FPGA ## n ## _CLK_CTRL, (start)) \
    REG32(FPGA ## n ## _THR_CTRL, (start) + 0x4)\
    REG32(FPGA ## n ## _THR_CNT,  (start) + 0x8)\
    REG32(FPGA ## n ## _THR_STA,  (start) + 0xc)
FPGA_CTRL_REGS(0, 0x170)
FPGA_CTRL_REGS(1, 0x180)
FPGA_CTRL_REGS(2, 0x190)
FPGA_CTRL_REGS(3, 0x1a0)

REG32(BANDGAP_TRIP, 0x1b8)
REG32(PLL_PREDIVISOR, 0x1c0)
REG32(CLK_621_TRUE, 0x1c4)

REG32(PSS_RST_CTRL, 0x200)
    FIELD(PSS_RST_CTRL, SOFT_RST, 0, 1)
REG32(DDR_RST_CTRL, 0x204)
REG32(TOPSW_RESET_CTRL, 0x208)
REG32(DMAC_RST_CTRL, 0x20c)
REG32(USB_RST_CTRL, 0x210)
REG32(GEM_RST_CTRL, 0x214)
REG32(SDIO_RST_CTRL, 0x218)
REG32(SPI_RST_CTRL, 0x21c)
REG32(CAN_RST_CTRL, 0x220)
REG32(I2C_RST_CTRL, 0x224)
REG32(UART_RST_CTRL, 0x228)
REG32(GPIO_RST_CTRL, 0x22c)
REG32(LQSPI_RST_CTRL, 0x230)
REG32(SMC_RST_CTRL, 0x234)
REG32(OCM_RST_CTRL, 0x238)
REG32(FPGA_RST_CTRL, 0x240)
REG32(A9_CPU_RST_CTRL, 0x244)

REG32(RS_AWDT_CTRL, 0x24c)
REG32(RST_REASON, 0x250)

REG32(REBOOT_STATUS, 0x258)
REG32(BOOT_MODE, 0x25c)

REG32(APU_CTRL, 0x300)
REG32(WDT_CLK_SEL, 0x304)

REG32(TZ_DMA_NS, 0x440)
REG32(TZ_DMA_IRQ_NS, 0x444)
REG32(TZ_DMA_PERIPH_NS, 0x448)

REG32(PSS_IDCODE, 0x530)

REG32(DDR_URGENT, 0x600)
REG32(DDR_CAL_START, 0x60c)
REG32(DDR_REF_START, 0x614)
REG32(DDR_CMD_STA, 0x618)
REG32(DDR_URGENT_SEL, 0x61c)
REG32(DDR_DFI_STATUS, 0x620)

REG32(MIO, 0x700)
#define MIO_LENGTH 54

REG32(MIO_LOOPBACK, 0x804)
REG32(MIO_MST_TRI0, 0x808)
REG32(MIO_MST_TRI1, 0x80c)

REG32(SD0_WP_CD_SEL, 0x830)
REG32(SD1_WP_CD_SEL, 0x834)

REG32(LVL_SHFTR_EN, 0x900)
REG32(OCM_CFG, 0x910)

REG32(CPU_RAM, 0xa00)

REG32(IOU, 0xa30)

REG32(DMAC_RAM, 0xa50)

REG32(AFI0, 0xa60)
REG32(AFI1, 0xa6c)
REG32(AFI2, 0xa78)
REG32(AFI3, 0xa84)
#define AFI_LENGTH 3

REG32(OCM, 0xa90)

REG32(DEVCI_RAM, 0xaa0)

REG32(CSG_RAM, 0xab0)

REG32(GPIOB_CTRL, 0xb00)
REG32(GPIOB_CFG_CMOS18, 0xb04)
REG32(GPIOB_CFG_CMOS25, 0xb08)
REG32(GPIOB_CFG_CMOS33, 0xb0c)
REG32(GPIOB_CFG_HSTL, 0xb14)
REG32(GPIOB_DRVR_BIAS_CTRL, 0xb18)

REG32(DDRIOB, 0xb40) */
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

    /* PS to PL reset signals.  */
    qemu_irq fpga_resets[17];

    uint32_t regs[CHOMP_SLCR_NUM_REGS];

    Clock *clk;
    Clock *uart0_ref_clk;
} ChompSLCRState;

static void chomp_slcr_fdt_config(ChompSLCRState *s)
{
#ifdef CONFIG_FDT
    QemuOpts *machine_opts;
    const char *dtb_filename;
    int fdt_size;
    void *fdt = NULL;

    /* identify dtb file name from qemu opts */
    machine_opts = qemu_opts_find(qemu_find_opts("machine"), 0);
    if (machine_opts) {
        dtb_filename = qemu_opt_get(machine_opts, "dtb");
    } else {
        dtb_filename = NULL;
    }

    if (dtb_filename) {
        fdt = load_device_tree(dtb_filename, &fdt_size);
    }

    if (!fdt) {
        return;
    }
#endif

    return;
}

/*
 * return the output frequency of ARM/DDR/IO pll
 * using input frequency and PLL_CTRL register
 */
static uint64_t chomp_slcr_compute_pll(uint64_t input, uint32_t ctrl_reg)
{
    /*
    uint32_t mult = ((ctrl_reg & R_xxx_PLL_CTRL_PLL_FPDIV_MASK) >>
            R_xxx_PLL_CTRL_PLL_FPDIV_SHIFT);
    */
    /* first, check if pll is bypassed */
    /*
    if (ctrl_reg & R_xxx_PLL_CTRL_PLL_BYPASS_FORCE_MASK) {
        return input;
    }

    if (ctrl_reg & (R_xxx_PLL_CTRL_PLL_RESET_MASK |
                    R_xxx_PLL_CTRL_PLL_PWRDWN_MASK)) {
        return 0;
    }
    */
    /* frequency multiplier -> period division */
    return 0; // input / mult;
}

/*
 * return the output period of a clock given:
 * + the periods in an array corresponding to input mux selector
 * + the register xxx_CLK_CTRL value
 * + enable bit index in ctrl register
 *
 * This function makes the assumption that the ctrl_reg value is organized as
 * follows:
 * + bits[13:8]  clock frequency divisor
 * + bits[5:4]   clock mux selector (index in array)
 * + bits[index] clock enable
 */
static uint64_t chomp_slcr_compute_clock(const uint64_t periods[],
                                        uint32_t ctrl_reg,
                                        unsigned index)
{
    uint32_t srcsel = extract32(ctrl_reg, 4, 2); /* bits [5:4] */
    uint32_t divisor = extract32(ctrl_reg, 8, 6); /* bits [13:8] */

    /* first, check if clock is disabled */
    if (((ctrl_reg >> index) & 1u) == 0) {
        return 0;
    }

    /*
     * according to the Chomp technical ref. manual UG585 v1.12.2 in
     * Clocks chapter, section 25.10.1 page 705:
     * "The 6-bit divider provides a divide range of 1 to 63"
     * We follow here what is implemented in linux kernel and consider
     * the 0 value as a bypass (no division).
     */
    /* frequency divisor -> period multiplication */
    return periods[srcsel] * (divisor ? divisor : 1u);
}

/*
 * macro helper around chomp_slcr_compute_clock to avoid repeating
 * the register name.
 */
#define CHOMP_COMPUTE_CLK(state, plls, reg, enable_field) \
    chomp_slcr_compute_clock((plls), (state)->regs[reg], \
                            reg ## _ ## enable_field ## _SHIFT)

/**
 * Compute and set the ouputs clocks periods.
 * But do not propagate them further. Connected clocks
 * will not receive any updates (See chomp_slcr_compute_clocks())
 */
static void chomp_slcr_compute_clocks(ChompSLCRState *s)
{
    uint64_t clk = clock_get(s->clk);

    /* consider outputs clocks are disabled while in reset */
    if (device_is_in_reset(DEVICE(s))) {
        clk = 0;
    }

    // TODO:
    /*
    uint64_t io_pll = chomp_slcr_compute_pll(ps_clk, s->regs[R_IO_PLL_CTRL]);
    uint64_t arm_pll = chomp_slcr_compute_pll(ps_clk, s->regs[R_ARM_PLL_CTRL]);
    uint64_t ddr_pll = chomp_slcr_compute_pll(ps_clk, s->regs[R_DDR_PLL_CTRL]);

    uint64_t uart_mux[4] = {io_pll, io_pll, arm_pll, ddr_pll};

    // compute uartX reference clocks 
    clock_set(s->uart0_ref_clk,
              CHOMP_COMPUTE_CLK(s, uart_mux, R_UART_CLK_CTRL, CLKACT0));
    clock_set(s->uart1_ref_clk,
              CHOMP_COMPUTE_CLK(s, uart_mux, R_UART_CLK_CTRL, CLKACT1));
    */
}

/**
 * Propagate the outputs clocks.
 * chomp_slcr_compute_clocks() should have been called before
 * to configure them.
 */
static void chomp_slcr_propagate_clocks(ChompSLCRState *s)
{
    clock_propagate(s->uart0_ref_clk);
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
    int i;
    QemuOpts *opts = qemu_find_opts_singleton("boot-opts");
    int boot_mode;

    DB_PRINT("RESET\n");

    boot_mode = qemu_opt_get_number(opts, "mode", 0);

    s->regs[R_CHIP_ID] = 0x3900;

    s->regs[R_GPIO_IN_1] = 0x8000; // FIXED to UART boot mode

    /*
    s->regs[R_LOCKSTA] = 1;
    s->regs[R_ARM_PLL_CTRL]   = 0x0001A008;
    s->regs[R_DDR_PLL_CTRL]   = 0x0001A008;
    s->regs[R_IO_PLL_CTRL]    = 0x0001A008;
    s->regs[R_PLL_STATUS]     = 0x0000003F;
    s->regs[R_ARM_PLL_CFG]    = 0x00014000;
    s->regs[R_DDR_PLL_CFG]    = 0x00014000;
    s->regs[R_IO_PLL_CFG]     = 0x00014000;

    s->regs[R_ARM_CLK_CTRL]   = 0x1F000200;
    s->regs[R_DDR_CLK_CTRL]   = 0x18400003;
    s->regs[R_DCI_CLK_CTRL]   = 0x01E03201;
    s->regs[R_APER_CLK_CTRL]  = 0x01FFCCCD;
    s->regs[R_USB0_CLK_CTRL]  = s->regs[R_USB1_CLK_CTRL]  = 0x00101941;
    s->regs[R_GEM0_RCLK_CTRL] = s->regs[R_GEM1_RCLK_CTRL] = 0x00000001;
    s->regs[R_GEM0_CLK_CTRL]  = s->regs[R_GEM1_CLK_CTRL]  = 0x00003C01;
    s->regs[R_SMC_CLK_CTRL]   = 0x00003C01;
    s->regs[R_LQSPI_CLK_CTRL] = 0x00002821;
    s->regs[R_SDIO_CLK_CTRL]  = 0x00001E03;
    s->regs[R_UART_CLK_CTRL]  = 0x00003F03;
    s->regs[R_SPI_CLK_CTRL]   = 0x00003F03;
    s->regs[R_CAN_CLK_CTRL]   = 0x00501903;
    s->regs[R_DBG_CLK_CTRL]   = 0x00000F03;
    s->regs[R_PCAP_CLK_CTRL]  = 0x00000F01;

    s->regs[R_FPGA0_CLK_CTRL] = s->regs[R_FPGA1_CLK_CTRL]
                              = s->regs[R_FPGA2_CLK_CTRL]
                              = s->regs[R_FPGA3_CLK_CTRL] = 0x00101800;
    s->regs[R_FPGA0_THR_STA] = s->regs[R_FPGA1_THR_STA]
                             = s->regs[R_FPGA2_THR_STA]
                             = s->regs[R_FPGA3_THR_STA] = 0x00010000;

    s->regs[R_BANDGAP_TRIP]   = 0x0000001F;
    s->regs[R_PLL_PREDIVISOR] = 0x00000001;
    s->regs[R_CLK_621_TRUE]   = 0x00000001;

    s->regs[R_FPGA_RST_CTRL]  = 0x01F33F0F;
    s->regs[R_RST_REASON]     = 0x00000040;

    s->regs[R_BOOT_MODE]      = boot_mode;

    for (i = 0; i < 54; i++) {
        s->regs[R_MIO + i] = 0x00001601;
    }
    for (i = 2; i <= 8; i++) {
        s->regs[R_MIO + i] = 0x00000601;
    }

    s->regs[R_MIO_MST_TRI0] = s->regs[R_MIO_MST_TRI1] = 0xFFFFFFFF;

    s->regs[R_CPU_RAM + 0] = s->regs[R_CPU_RAM + 1] = s->regs[R_CPU_RAM + 3]
                           = s->regs[R_CPU_RAM + 4] = s->regs[R_CPU_RAM + 7]
                           = 0x00010101;
    s->regs[R_CPU_RAM + 2] = s->regs[R_CPU_RAM + 5] = 0x01010101;
    s->regs[R_CPU_RAM + 6] = 0x00000001;

    s->regs[R_IOU + 0] = s->regs[R_IOU + 1] = s->regs[R_IOU + 2]
                       = s->regs[R_IOU + 3] = 0x09090909;
    s->regs[R_IOU + 4] = s->regs[R_IOU + 5] = 0x00090909;
    s->regs[R_IOU + 6] = 0x00000909;

    s->regs[R_DMAC_RAM] = 0x00000009;

    s->regs[R_AFI0 + 0] = s->regs[R_AFI0 + 1] = 0x09090909;
    s->regs[R_AFI1 + 0] = s->regs[R_AFI1 + 1] = 0x09090909;
    s->regs[R_AFI2 + 0] = s->regs[R_AFI2 + 1] = 0x09090909;
    s->regs[R_AFI3 + 0] = s->regs[R_AFI3 + 1] = 0x09090909;
    s->regs[R_AFI0 + 2] = s->regs[R_AFI1 + 2] = s->regs[R_AFI2 + 2]
                        = s->regs[R_AFI3 + 2] = 0x00000909;

    s->regs[R_OCM + 0] = 0x01010101;
    s->regs[R_OCM + 1] = s->regs[R_OCM + 2] = 0x09090909;

    s->regs[R_DEVCI_RAM] = 0x00000909;
    s->regs[R_CSG_RAM]   = 0x00000001;

    s->regs[R_DDRIOB + 0] = s->regs[R_DDRIOB + 1] = s->regs[R_DDRIOB + 2]
                          = s->regs[R_DDRIOB + 3] = 0x00000e00;
    s->regs[R_DDRIOB + 4] = s->regs[R_DDRIOB + 5] = s->regs[R_DDRIOB + 6]
                          = 0x00000e00;
    s->regs[R_DDRIOB + 12] = 0x00000021;
    */

    chomp_slcr_fdt_config(s);
}

static void chomp_slcr_reset_hold(Object *obj)
{
    ChompSLCRState *s = CHOMP_SLCR(obj);

    /* will disable all output clocks */
    chomp_slcr_compute_clocks(s);
    chomp_slcr_propagate_clocks(s);
}

static void chomp_slcr_reset_exit(Object *obj)
{
    ChompSLCRState *s = CHOMP_SLCR(obj);

    /* will compute output clocks according to ps_clk and registers */
    chomp_slcr_compute_clocks(s);
    chomp_slcr_propagate_clocks(s);
}

static bool chomp_slcr_check_offset(hwaddr offset, bool rnw)
{
    switch (offset) {
        /*
    case R_LOCK:
    case R_UNLOCK:
    case R_DDR_CAL_START:
    case R_DDR_REF_START:
        return !rnw; // Write only 
    */
    case R_CHIP_ID:
    /*
    case R_LOCKSTA:
    case R_FPGA0_THR_STA:
    case R_FPGA1_THR_STA:
    case R_FPGA2_THR_STA:
    case R_FPGA3_THR_STA:
    case R_BOOT_MODE:
    case R_PSS_IDCODE:
    case R_DDR_CMD_STA:
    case R_DDR_DFI_STATUS:
    case R_PLL_STATUS:
    */
        return rnw;/* read only */
    /*
    case R_ARM_PLL_CTRL ... R_IO_PLL_CTRL:
    case R_ARM_PLL_CFG ... R_IO_PLL_CFG:
    case R_ARM_CLK_CTRL ... R_TOPSW_CLK_CTRL:
    case R_FPGA0_CLK_CTRL ... R_FPGA0_THR_CNT:
    case R_FPGA1_CLK_CTRL ... R_FPGA1_THR_CNT:
    case R_FPGA2_CLK_CTRL ... R_FPGA2_THR_CNT:
    case R_FPGA3_CLK_CTRL ... R_FPGA3_THR_CNT:
    case R_BANDGAP_TRIP:
    case R_PLL_PREDIVISOR:
    case R_CLK_621_TRUE:
    case R_PSS_RST_CTRL ... R_A9_CPU_RST_CTRL:
    case R_RS_AWDT_CTRL:
    case R_RST_REASON:
    case R_REBOOT_STATUS:
    case R_APU_CTRL:
    case R_WDT_CLK_SEL:
    case R_TZ_DMA_NS ... R_TZ_DMA_PERIPH_NS:
    case R_DDR_URGENT:
    case R_DDR_URGENT_SEL:
    case R_MIO ... R_MIO + MIO_LENGTH - 1:
    case R_MIO_LOOPBACK ... R_MIO_MST_TRI1:
    case R_SD0_WP_CD_SEL:
    case R_SD1_WP_CD_SEL:
    case R_LVL_SHFTR_EN:
    case R_OCM_CFG:
    case R_CPU_RAM:
    case R_IOU:
    case R_DMAC_RAM:
    case R_AFI0 ... R_AFI3 + AFI_LENGTH - 1:
    case R_OCM:
    case R_DEVCI_RAM:
    case R_CSG_RAM:
    case R_GPIOB_CTRL ... R_GPIOB_CFG_CMOS33:
    case R_GPIOB_CFG_HSTL:
    case R_GPIOB_DRVR_BIAS_CTRL:
    case R_DDRIOB ... R_DDRIOB + DDRIOB_LENGTH - 1:
    */
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

    if (!chomp_slcr_check_offset(offset, true)) {
        qemu_log_mask(LOG_GUEST_ERROR, "chomp_slcr: Invalid read access to "
                      " addr %" HWADDR_PRIx "\n", offset * 4);
    }

    DB_PRINT("addr: %08" HWADDR_PRIx " data: %08" PRIx32 "\n", offset * 4, ret);
    return ret;
}

static void chomp_slcr_write(void *opaque, hwaddr offset,
                          uint64_t val, unsigned size)
{
    ChompSLCRState *s = (ChompSLCRState *)opaque;
    offset /= 4;
    int i;

    DB_PRINT("addr: %08" HWADDR_PRIx " data: %08" PRIx64 "\n", offset * 4, val);

    if (!chomp_slcr_check_offset(offset, false)) {
        qemu_log_mask(LOG_GUEST_ERROR, "chomp_slcr: Invalid write access to "
                      "addr %" HWADDR_PRIx "\n", offset * 4);
        return;
    }

    switch (offset) {
        /*
    //case R_SCL:
    //    s->regs[R_SCL] = val & 0x1;
    //    return;
    case R_LOCK:
        if ((val & 0xFFFF) == CHOMPTECH_LOCK_KEY) {
            DB_PRINT("CHOMPTECH LOCK 0xF8000000 + 0x%x <= 0x%x\n", (int)offset,
                (unsigned)val & 0xFFFF);
            s->regs[R_LOCKSTA] = 1;
        } else {
            DB_PRINT("WRONG CHOMPTECH LOCK KEY 0xF8000000 + 0x%x <= 0x%x\n",
                (int)offset, (unsigned)val & 0xFFFF);
        }
        return;
    case R_UNLOCK:
        if ((val & 0xFFFF) == CHOMPTECH_UNLOCK_KEY) {
            DB_PRINT("CHOMPTECH UNLOCK 0xF8000000 + 0x%x <= 0x%x\n", (int)offset,
                (unsigned)val & 0xFFFF);
            s->regs[R_LOCKSTA] = 0;
        } else {
            DB_PRINT("WRONG CHOMPTECH UNLOCK KEY 0xF8000000 + 0x%x <= 0x%x\n",
                (int)offset, (unsigned)val & 0xFFFF);
        }
        */
        return;
    }

    /*
    if (s->regs[R_LOCKSTA]) {
        qemu_log_mask(LOG_GUEST_ERROR,
                      "SCLR registers are locked. Unlock them first\n");
        return;
    }
    */
    s->regs[offset] = val;

    /*
    switch (offset) {
    case R_PSS_RST_CTRL:
        if (FIELD_EX32(val, PSS_RST_CTRL, SOFT_RST)) {
            qemu_system_reset_request(SHUTDOWN_CAUSE_GUEST_RESET);
        }
        break;
    case R_A9_CPU_RST_CTRL:
        for (i = 0; i < CHOMP_SLCR_NUM_CPUS; ++i) {
            bool rst = extract32(val, A9_CPU_RST_CTRL_RST_SHIFT + i, 1);

            qemu_set_irq(s->cpu_resets[i], rst);
            DB_PRINT("%sresetting cpu %d\n", rst ? "" : "un-", i);
        }
        break;
    case R_FPGA_RST_CTRL:
        s->regs[offset] &= FPGA_RST_VALID_BITS;
        chomp_slcr_update_fpga_resets(s);
        break;
    case R_IO_PLL_CTRL:
    case R_ARM_PLL_CTRL:
    case R_DDR_PLL_CTRL:
    case R_UART_CLK_CTRL:
        chomp_slcr_compute_clocks(s);
        chomp_slcr_propagate_clocks(s);
        break;
    }
    */
}

static const MemoryRegionOps slcr_ops = {
    .read = chomp_slcr_read,
    .write = chomp_slcr_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void chomp_slcr_realize(DeviceState *dev, Error **errp)
{
    int i;
    CPUState *env = first_cpu;

    /* FIXME: Make this not suck */
    for (i  = 0; i < fdt_generic_num_cpus && i < CHOMP_SLCR_NUM_CPUS; ++i) {
        Object *cpu_obj = OBJECT(env);
        if (!cpu_obj->parent) {
            char *cpu_child_name = g_strdup_printf("cpu-%d\n", i);
            object_property_add_child(qdev_get_machine(), cpu_child_name,
                                      cpu_obj);
        }
        qdev_connect_gpio_out(dev, i,
                              qdev_get_gpio_in_named(DEVICE(env), "reset", 0));
        env = CPU_NEXT(env);
    }
}

static const ClockPortInitArray chomp_slcr_clocks = {
    QDEV_CLOCK_IN(ChompSLCRState, clk, chomp_slcr_clk_callback),
    QDEV_CLOCK_OUT(ChompSLCRState, uart0_ref_clk),
    QDEV_CLOCK_END
};

static void chomp_slcr_init(Object *obj)
{
    ChompSLCRState *s = CHOMP_SLCR(obj);

    memory_region_init_io(&s->iomem, obj, &slcr_ops, s, "slcr",
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
