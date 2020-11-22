/*
 * QEMU model of the CRL_APB APB control registers for clock controller. The rstctrl_lpd willbe added to this as well
 *
 * Copyright (c) 2014 Xilinx Inc.
 *
 * Autogenerated by xregqemu.py 2014-03-20.
 * Written by Edgar E. Iglesias
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
#include "hw/sysbus.h"
#include "hw/register.h"
#include "qemu/bitops.h"
#include "qemu/log.h"
#include "qemu/config-file.h"
#include "sysemu/sysemu.h"
#include "qemu/option.h"
#include "migration/vmstate.h"
#include "hw/qdev-properties.h"

#include "hw/fdt_generic_util.h"

#ifndef XILINX_CRL_APB_ERR_DEBUG
#define XILINX_CRL_APB_ERR_DEBUG 0
#endif

#define TYPE_XILINX_CRL_APB "xlnx.zynqmp-crl"

#define XILINX_CRL_APB(obj) \
     OBJECT_CHECK(CRL_APB, (obj), TYPE_XILINX_CRL_APB)

REG32(ERR_CTRL, 0x0)
    FIELD(ERR_CTRL, SLVERR_ENABLE, 0, 1)
REG32(IR_STATUS, 0x4)
    FIELD(IR_STATUS, ADDR_DECODE_ERR, 0, 1)
REG32(IR_MASK, 0x8)
    FIELD(IR_MASK, ADDR_DECODE_ERR, 0, 1)
REG32(IR_ENABLE, 0xc)
    FIELD(IR_ENABLE, ADDR_DECODE_ERR, 0, 1)
REG32(IR_DISABLE, 0x10)
    FIELD(IR_DISABLE, ADDR_DECODE_ERR, 0, 1)
REG32(CRL_WPROT, 0x1c)
    FIELD(CRL_WPROT, ACTIVE, 0, 1)
REG32(IOPLL_CTRL, 0x20)
    FIELD(IOPLL_CTRL, POST_SRC, 24, 3)
    FIELD(IOPLL_CTRL, PRE_SRC, 20, 3)
    FIELD(IOPLL_CTRL, CLKOUTDIV, 17, 1)
    FIELD(IOPLL_CTRL, DIV2, 16, 1)
    FIELD(IOPLL_CTRL, FBDIV, 8, 7)
    FIELD(IOPLL_CTRL, BYPASS, 3, 1)
    FIELD(IOPLL_CTRL, RESET, 0, 1)
REG32(IOPLL_CFG, 0x24)
    FIELD(IOPLL_CFG, LOCK_DLY, 25, 7)
    FIELD(IOPLL_CFG, LOCK_CNT, 13, 10)
    FIELD(IOPLL_CFG, LFHF, 10, 2)
    FIELD(IOPLL_CFG, CP, 5, 4)
    FIELD(IOPLL_CFG, RES, 0, 4)
REG32(IOPLL_FRAC_CFG, 0x28)
    FIELD(IOPLL_FRAC_CFG, ENABLED, 31, 1)
    FIELD(IOPLL_FRAC_CFG, SEED, 22, 3)
    FIELD(IOPLL_FRAC_CFG, ALGRTHM, 19, 1)
    FIELD(IOPLL_FRAC_CFG, ORDER, 18, 1)
    FIELD(IOPLL_FRAC_CFG, DATA, 0, 16)
REG32(RPLL_CTRL, 0x30)
    FIELD(RPLL_CTRL, POST_SRC, 24, 3)
    FIELD(RPLL_CTRL, PRE_SRC, 20, 3)
    FIELD(RPLL_CTRL, CLKOUTDIV, 17, 1)
    FIELD(RPLL_CTRL, DIV2, 16, 1)
    FIELD(RPLL_CTRL, FBDIV, 8, 7)
    FIELD(RPLL_CTRL, BYPASS, 3, 1)
    FIELD(RPLL_CTRL, RESET, 0, 1)
REG32(RPLL_CFG, 0x34)
    FIELD(RPLL_CFG, LOCK_DLY, 25, 7)
    FIELD(RPLL_CFG, LOCK_CNT, 13, 10)
    FIELD(RPLL_CFG, LFHF, 10, 2)
    FIELD(RPLL_CFG, CP, 5, 4)
    FIELD(RPLL_CFG, RES, 0, 4)
REG32(RPLL_FRAC_CFG, 0x38)
    FIELD(RPLL_FRAC_CFG, ENABLED, 31, 1)
    FIELD(RPLL_FRAC_CFG, SEED, 22, 3)
    FIELD(RPLL_FRAC_CFG, ALGRTHM, 19, 1)
    FIELD(RPLL_FRAC_CFG, ORDER, 18, 1)
    FIELD(RPLL_FRAC_CFG, DATA, 0, 16)
REG32(PLL_STATUS, 0x40)
    FIELD(PLL_STATUS, RPLL_STABLE, 4, 1)
    FIELD(PLL_STATUS, IOPLL_STABLE, 3, 1)
    FIELD(PLL_STATUS, RPLL_LOCK, 1, 1)
    FIELD(PLL_STATUS, IOPLL_LOCK, 0, 1)
REG32(IOPLL_TO_FPD_CTRL, 0x44)
    FIELD(IOPLL_TO_FPD_CTRL, DIVISOR0, 8, 6)
REG32(RPLL_TO_FPD_CTRL, 0x48)
    FIELD(RPLL_TO_FPD_CTRL, DIVISOR0, 8, 6)
REG32(USB3_DUAL_REF_CTRL, 0x4c)
    FIELD(USB3_DUAL_REF_CTRL, CLKACT, 25, 1)
    FIELD(USB3_DUAL_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(USB3_DUAL_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(USB3_DUAL_REF_CTRL, SRCSEL, 0, 3)
REG32(GEM0_REF_CTRL, 0x50)
    FIELD(GEM0_REF_CTRL, RX_CLKACT, 26, 1)
    FIELD(GEM0_REF_CTRL, CLKACT, 25, 1)
    FIELD(GEM0_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(GEM0_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(GEM0_REF_CTRL, SRCSEL, 0, 3)
REG32(GEM1_REF_CTRL, 0x54)
    FIELD(GEM1_REF_CTRL, RX_CLKACT, 26, 1)
    FIELD(GEM1_REF_CTRL, CLKACT, 25, 1)
    FIELD(GEM1_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(GEM1_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(GEM1_REF_CTRL, SRCSEL, 0, 3)
REG32(GEM2_REF_CTRL, 0x58)
    FIELD(GEM2_REF_CTRL, RX_CLKACT, 26, 1)
    FIELD(GEM2_REF_CTRL, CLKACT, 25, 1)
    FIELD(GEM2_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(GEM2_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(GEM2_REF_CTRL, SRCSEL, 0, 3)
REG32(GEM3_REF_CTRL, 0x5c)
    FIELD(GEM3_REF_CTRL, RX_CLKACT, 26, 1)
    FIELD(GEM3_REF_CTRL, CLKACT, 25, 1)
    FIELD(GEM3_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(GEM3_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(GEM3_REF_CTRL, SRCSEL, 0, 3)
REG32(USB0_BUS_REF_CTRL, 0x60)
    FIELD(USB0_BUS_REF_CTRL, CLKACT, 25, 1)
    FIELD(USB0_BUS_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(USB0_BUS_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(USB0_BUS_REF_CTRL, SRCSEL, 0, 3)
REG32(USB1_BUS_REF_CTRL, 0x64)
    FIELD(USB1_BUS_REF_CTRL, CLKACT, 25, 1)
    FIELD(USB1_BUS_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(USB1_BUS_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(USB1_BUS_REF_CTRL, SRCSEL, 0, 3)
REG32(QSPI_REF_CTRL, 0x68)
    FIELD(QSPI_REF_CTRL, CLKACT, 24, 1)
    FIELD(QSPI_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(QSPI_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(QSPI_REF_CTRL, SRCSEL, 0, 3)
REG32(SDIO0_REF_CTRL, 0x6c)
    FIELD(SDIO0_REF_CTRL, CLKACT, 24, 1)
    FIELD(SDIO0_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(SDIO0_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(SDIO0_REF_CTRL, SRCSEL, 0, 3)
REG32(SDIO1_REF_CTRL, 0x70)
    FIELD(SDIO1_REF_CTRL, CLKACT, 24, 1)
    FIELD(SDIO1_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(SDIO1_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(SDIO1_REF_CTRL, SRCSEL, 0, 3)
REG32(UART0_REF_CTRL, 0x74)
    FIELD(UART0_REF_CTRL, CLKACT, 24, 1)
    FIELD(UART0_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(UART0_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(UART0_REF_CTRL, SRCSEL, 0, 3)
REG32(UART1_REF_CTRL, 0x78)
    FIELD(UART1_REF_CTRL, CLKACT, 24, 1)
    FIELD(UART1_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(UART1_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(UART1_REF_CTRL, SRCSEL, 0, 3)
REG32(SPI0_REF_CTRL, 0x7c)
    FIELD(SPI0_REF_CTRL, CLKACT, 24, 1)
    FIELD(SPI0_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(SPI0_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(SPI0_REF_CTRL, SRCSEL, 0, 3)
REG32(SPI1_REF_CTRL, 0x80)
    FIELD(SPI1_REF_CTRL, CLKACT, 24, 1)
    FIELD(SPI1_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(SPI1_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(SPI1_REF_CTRL, SRCSEL, 0, 3)
REG32(CAN0_REF_CTRL, 0x84)
    FIELD(CAN0_REF_CTRL, CLKACT, 24, 1)
    FIELD(CAN0_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(CAN0_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(CAN0_REF_CTRL, SRCSEL, 0, 3)
REG32(CAN1_REF_CTRL, 0x88)
    FIELD(CAN1_REF_CTRL, CLKACT, 24, 1)
    FIELD(CAN1_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(CAN1_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(CAN1_REF_CTRL, SRCSEL, 0, 3)
REG32(CPU_R5_CTRL, 0x90)
    FIELD(CPU_R5_CTRL, CLKACT_CORE, 25, 1)
    FIELD(CPU_R5_CTRL, CLKACT, 24, 1)
    FIELD(CPU_R5_CTRL, DIVISOR0, 8, 6)
    FIELD(CPU_R5_CTRL, SRCSEL, 0, 3)
REG32(IOU_SWITCH_CTRL, 0x9c)
    FIELD(IOU_SWITCH_CTRL, CLKACT, 24, 1)
    FIELD(IOU_SWITCH_CTRL, DIVISOR0, 8, 6)
    FIELD(IOU_SWITCH_CTRL, SRCSEL, 0, 3)
REG32(CSU_PLL_CTRL, 0xa0)
    FIELD(CSU_PLL_CTRL, CLKACT, 24, 1)
    FIELD(CSU_PLL_CTRL, DIVISOR0, 8, 6)
    FIELD(CSU_PLL_CTRL, SRCSEL, 0, 3)
REG32(PCAP_CTRL, 0xa4)
    FIELD(PCAP_CTRL, CLKACT, 24, 1)
    FIELD(PCAP_CTRL, DIVISOR0, 8, 6)
    FIELD(PCAP_CTRL, SRCSEL, 0, 3)
REG32(LPD_SWITCH_CTRL, 0xa8)
    FIELD(LPD_SWITCH_CTRL, CLKACT, 24, 1)
    FIELD(LPD_SWITCH_CTRL, DIVISOR0, 8, 6)
    FIELD(LPD_SWITCH_CTRL, SRCSEL, 0, 3)
REG32(LPD_LSBUS_CTRL, 0xac)
    FIELD(LPD_LSBUS_CTRL, CLKACT, 24, 1)
    FIELD(LPD_LSBUS_CTRL, DIVISOR0, 8, 6)
    FIELD(LPD_LSBUS_CTRL, SRCSEL, 0, 3)
REG32(DBG_LPD_CTRL, 0xb0)
    FIELD(DBG_LPD_CTRL, CLKACT, 24, 1)
    FIELD(DBG_LPD_CTRL, DIVISOR0, 8, 6)
    FIELD(DBG_LPD_CTRL, SRCSEL, 0, 3)
REG32(NAND_REF_CTRL, 0xb4)
    FIELD(NAND_REF_CTRL, CLKACT, 24, 1)
    FIELD(NAND_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(NAND_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(NAND_REF_CTRL, SRCSEL, 0, 3)
REG32(ADMA_REF_CTRL, 0xb8)
    FIELD(ADMA_REF_CTRL, CLKACT, 24, 1)
    FIELD(ADMA_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(ADMA_REF_CTRL, SRCSEL, 0, 3)
REG32(PL0_REF_CTRL, 0xc0)
    FIELD(PL0_REF_CTRL, CLKACT, 24, 1)
    FIELD(PL0_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(PL0_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(PL0_REF_CTRL, SRCSEL, 0, 3)
REG32(PL1_REF_CTRL, 0xc4)
    FIELD(PL1_REF_CTRL, CLKACT, 24, 1)
    FIELD(PL1_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(PL1_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(PL1_REF_CTRL, SRCSEL, 0, 3)
REG32(PL2_REF_CTRL, 0xc8)
    FIELD(PL2_REF_CTRL, CLKACT, 24, 1)
    FIELD(PL2_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(PL2_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(PL2_REF_CTRL, SRCSEL, 0, 3)
REG32(PL3_REF_CTRL, 0xcc)
    FIELD(PL3_REF_CTRL, CLKACT, 24, 1)
    FIELD(PL3_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(PL3_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(PL3_REF_CTRL, SRCSEL, 0, 3)
REG32(PL0_THR_CTRL, 0xd0)
    FIELD(PL0_THR_CTRL, CURR_VAL, 16, 16)
    FIELD(PL0_THR_CTRL, RUNNING, 15, 1)
    FIELD(PL0_THR_CTRL, CPU_START, 1, 1)
    FIELD(PL0_THR_CTRL, CNT_RST, 0, 1)
REG32(PL0_THR_CNT, 0xd4)
    FIELD(PL0_THR_CNT, LAST_CNT, 0, 16)
REG32(PL1_THR_CTRL, 0xd8)
    FIELD(PL1_THR_CTRL, CURR_VAL, 16, 16)
    FIELD(PL1_THR_CTRL, RUNNING, 15, 1)
    FIELD(PL1_THR_CTRL, CPU_START, 1, 1)
    FIELD(PL1_THR_CTRL, CNT_RST, 0, 1)
REG32(PL1_THR_CNT, 0xdc)
    FIELD(PL1_THR_CNT, LAST_CNT, 0, 16)
REG32(PL2_THR_CTRL, 0xe0)
    FIELD(PL2_THR_CTRL, CURR_VAL, 16, 16)
    FIELD(PL2_THR_CTRL, RUNNING, 15, 1)
    FIELD(PL2_THR_CTRL, CPU_START, 1, 1)
    FIELD(PL2_THR_CTRL, CNT_RST, 0, 1)
REG32(PL2_THR_CNT, 0xe4)
    FIELD(PL2_THR_CNT, LAST_CNT, 0, 16)
REG32(PL3_THR_CTRL, 0xe8)
    FIELD(PL3_THR_CTRL, CURR_VAL, 16, 16)
    FIELD(PL3_THR_CTRL, RUNNING, 15, 1)
    FIELD(PL3_THR_CTRL, CPU_START, 1, 1)
    FIELD(PL3_THR_CTRL, CNT_RST, 0, 1)
REG32(PL3_THR_CNT, 0xfc)
    FIELD(PL3_THR_CNT, LAST_CNT, 0, 16)
REG32(GEM_TSU_REF_CTRL, 0x100)
    FIELD(GEM_TSU_REF_CTRL, CLKACT, 24, 1)
    FIELD(GEM_TSU_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(GEM_TSU_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(GEM_TSU_REF_CTRL, SRCSEL, 0, 3)
REG32(DLL_REF_CTRL, 0x104)
    FIELD(DLL_REF_CTRL, SRCSEL, 0, 3)
REG32(AMS_REF_CTRL, 0x108)
    FIELD(AMS_REF_CTRL, CLKACT, 24, 1)
    FIELD(AMS_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(AMS_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(AMS_REF_CTRL, SRCSEL, 0, 3)
REG32(I2C0_REF_CTRL, 0x120)
    FIELD(I2C0_REF_CTRL, CLKACT, 24, 1)
    FIELD(I2C0_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(I2C0_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(I2C0_REF_CTRL, SRCSEL, 0, 3)
REG32(I2C1_REF_CTRL, 0x124)
    FIELD(I2C1_REF_CTRL, CLKACT, 24, 1)
    FIELD(I2C1_REF_CTRL, DIVISOR1, 16, 6)
    FIELD(I2C1_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(I2C1_REF_CTRL, SRCSEL, 0, 3)
REG32(TIMESTAMP_REF_CTRL, 0x128)
    FIELD(TIMESTAMP_REF_CTRL, CLKACT, 24, 1)
    FIELD(TIMESTAMP_REF_CTRL, DIVISOR0, 8, 6)
    FIELD(TIMESTAMP_REF_CTRL, SRCSEL, 0, 3)
REG32(SAFTEY_CHK, 0x130)
REG32(CLKMON_STATUS, 0x140)
    FIELD(CLKMON_STATUS, CNTA7_OVER_ERR, 15, 1)
    FIELD(CLKMON_STATUS, MON7_ERR, 14, 1)
    FIELD(CLKMON_STATUS, CNTA6_OVER_ERR, 13, 1)
    FIELD(CLKMON_STATUS, MON6_ERR, 12, 1)
    FIELD(CLKMON_STATUS, CNTA5_OVER_ERR, 11, 1)
    FIELD(CLKMON_STATUS, MON5_ERR, 10, 1)
    FIELD(CLKMON_STATUS, CNTA4_OVER_ERR, 9, 1)
    FIELD(CLKMON_STATUS, MON4_ERR, 8, 1)
    FIELD(CLKMON_STATUS, CNTA3_OVER_ERR, 7, 1)
    FIELD(CLKMON_STATUS, MON3_ERR, 6, 1)
    FIELD(CLKMON_STATUS, CNTA2_OVER_ERR, 5, 1)
    FIELD(CLKMON_STATUS, MON2_ERR, 4, 1)
    FIELD(CLKMON_STATUS, CNTA1_OVER_ERR, 3, 1)
    FIELD(CLKMON_STATUS, MON1_ERR, 2, 1)
    FIELD(CLKMON_STATUS, CNTA0_OVER_ERR, 1, 1)
    FIELD(CLKMON_STATUS, MON0_ERR, 0, 1)
REG32(CLKMON_MASK, 0x144)
    FIELD(CLKMON_MASK, CNTA7_OVER_ERR, 15, 1)
    FIELD(CLKMON_MASK, MON7_ERR, 14, 1)
    FIELD(CLKMON_MASK, CNTA6_OVER_ERR, 13, 1)
    FIELD(CLKMON_MASK, MON6_ERR, 12, 1)
    FIELD(CLKMON_MASK, CNTA5_OVER_ERR, 11, 1)
    FIELD(CLKMON_MASK, MON5_ERR, 10, 1)
    FIELD(CLKMON_MASK, CNTA4_OVER_ERR, 9, 1)
    FIELD(CLKMON_MASK, MON4_ERR, 8, 1)
    FIELD(CLKMON_MASK, CNTA3_OVER_ERR, 7, 1)
    FIELD(CLKMON_MASK, MON3_ERR, 6, 1)
    FIELD(CLKMON_MASK, CNTA2_OVER_ERR, 5, 1)
    FIELD(CLKMON_MASK, MON2_ERR, 4, 1)
    FIELD(CLKMON_MASK, CNTA1_OVER_ERR, 3, 1)
    FIELD(CLKMON_MASK, MON1_ERR, 2, 1)
    FIELD(CLKMON_MASK, CNTA0_OVER_ERR, 1, 1)
    FIELD(CLKMON_MASK, MON0_ERR, 0, 1)
REG32(CLKMON_ENABLE, 0x148)
    FIELD(CLKMON_ENABLE, CNTA7_OVER_ERR, 15, 1)
    FIELD(CLKMON_ENABLE, MON7_ERR, 14, 1)
    FIELD(CLKMON_ENABLE, CNTA6_OVER_ERR, 13, 1)
    FIELD(CLKMON_ENABLE, MON6_ERR, 12, 1)
    FIELD(CLKMON_ENABLE, CNTA5_OVER_ERR, 11, 1)
    FIELD(CLKMON_ENABLE, MON5_ERR, 10, 1)
    FIELD(CLKMON_ENABLE, CNTA4_OVER_ERR, 9, 1)
    FIELD(CLKMON_ENABLE, MON4_ERR, 8, 1)
    FIELD(CLKMON_ENABLE, CNTA3_OVER_ERR, 7, 1)
    FIELD(CLKMON_ENABLE, MON3_ERR, 6, 1)
    FIELD(CLKMON_ENABLE, CNTA2_OVER_ERR, 5, 1)
    FIELD(CLKMON_ENABLE, MON2_ERR, 4, 1)
    FIELD(CLKMON_ENABLE, CNTA1_OVER_ERR, 3, 1)
    FIELD(CLKMON_ENABLE, MON1_ERR, 2, 1)
    FIELD(CLKMON_ENABLE, CNTA0_OVER_ERR, 1, 1)
    FIELD(CLKMON_ENABLE, MON0_ERR, 0, 1)
REG32(CLKMON_DISABLE, 0x14c)
    FIELD(CLKMON_DISABLE, CNTA7_OVER_ERR, 15, 1)
    FIELD(CLKMON_DISABLE, MON7_ERR, 14, 1)
    FIELD(CLKMON_DISABLE, CNTA6_OVER_ERR, 13, 1)
    FIELD(CLKMON_DISABLE, MON6_ERR, 12, 1)
    FIELD(CLKMON_DISABLE, CNTA5_OVER_ERR, 11, 1)
    FIELD(CLKMON_DISABLE, MON5_ERR, 10, 1)
    FIELD(CLKMON_DISABLE, CNTA4_OVER_ERR, 9, 1)
    FIELD(CLKMON_DISABLE, MON4_ERR, 8, 1)
    FIELD(CLKMON_DISABLE, CNTA3_OVER_ERR, 7, 1)
    FIELD(CLKMON_DISABLE, MON3_ERR, 6, 1)
    FIELD(CLKMON_DISABLE, CNTA2_OVER_ERR, 5, 1)
    FIELD(CLKMON_DISABLE, MON2_ERR, 4, 1)
    FIELD(CLKMON_DISABLE, CNTA1_OVER_ERR, 3, 1)
    FIELD(CLKMON_DISABLE, MON1_ERR, 2, 1)
    FIELD(CLKMON_DISABLE, CNTA0_OVER_ERR, 1, 1)
    FIELD(CLKMON_DISABLE, MON0_ERR, 0, 1)
REG32(CLKMON_TRIGGER, 0x150)
    FIELD(CLKMON_TRIGGER, CNTA7_OVER_ERR, 15, 1)
    FIELD(CLKMON_TRIGGER, MON7_ERR, 14, 1)
    FIELD(CLKMON_TRIGGER, CNTA6_OVER_ERR, 13, 1)
    FIELD(CLKMON_TRIGGER, MON6_ERR, 12, 1)
    FIELD(CLKMON_TRIGGER, CNTA5_OVER_ERR, 11, 1)
    FIELD(CLKMON_TRIGGER, MON5_ERR, 10, 1)
    FIELD(CLKMON_TRIGGER, CNTA4_OVER_ERR, 9, 1)
    FIELD(CLKMON_TRIGGER, MON4_ERR, 8, 1)
    FIELD(CLKMON_TRIGGER, CNTA3_OVER_ERR, 7, 1)
    FIELD(CLKMON_TRIGGER, MON3_ERR, 6, 1)
    FIELD(CLKMON_TRIGGER, CNTA2_OVER_ERR, 5, 1)
    FIELD(CLKMON_TRIGGER, MON2_ERR, 4, 1)
    FIELD(CLKMON_TRIGGER, CNTA1_OVER_ERR, 3, 1)
    FIELD(CLKMON_TRIGGER, MON1_ERR, 2, 1)
    FIELD(CLKMON_TRIGGER, CNTA0_OVER_ERR, 1, 1)
    FIELD(CLKMON_TRIGGER, MON0_ERR, 0, 1)
REG32(CHKR0_CLKA_UPPER, 0x160)
REG32(CHKR0_CLKA_LOWER, 0x164)
REG32(CHKR0_CLKB_CNT, 0x168)
REG32(CHKR0_CTRL, 0x16c)
    FIELD(CHKR0_CTRL, START_SINGLE, 8, 1)
    FIELD(CHKR0_CTRL, START_CONTINUOUS, 7, 1)
    FIELD(CHKR0_CTRL, CLKB_MUX_CTRL, 5, 1)
    FIELD(CHKR0_CTRL, CLKA_MUX_CTRL, 1, 3)
    FIELD(CHKR0_CTRL, ENABLE, 0, 1)
REG32(CHKR1_CLKA_UPPER, 0x170)
REG32(CHKR1_CLKA_LOWER, 0x174)
REG32(CHKR1_CLKB_CNT, 0x178)
REG32(CHKR1_CTRL, 0x17c)
    FIELD(CHKR1_CTRL, START_SINGLE, 8, 1)
    FIELD(CHKR1_CTRL, START_CONTINUOUS, 7, 1)
    FIELD(CHKR1_CTRL, CLKB_MUX_CTRL, 5, 1)
    FIELD(CHKR1_CTRL, CLKA_MUX_CTRL, 1, 3)
    FIELD(CHKR1_CTRL, ENABLE, 0, 1)
REG32(CHKR2_CLKA_UPPER, 0x180)
REG32(CHKR2_CLKA_LOWER, 0x184)
REG32(CHKR2_CLKB_CNT, 0x188)
REG32(CHKR2_CTRL, 0x18c)
    FIELD(CHKR2_CTRL, START_SINGLE, 8, 1)
    FIELD(CHKR2_CTRL, START_CONTINUOUS, 7, 1)
    FIELD(CHKR2_CTRL, CLKB_MUX_CTRL, 5, 1)
    FIELD(CHKR2_CTRL, CLKA_MUX_CTRL, 1, 3)
    FIELD(CHKR2_CTRL, ENABLE, 0, 1)
REG32(CHKR3_CLKA_UPPER, 0x190)
REG32(CHKR3_CLKA_LOWER, 0x194)
REG32(CHKR3_CLKB_CNT, 0x198)
REG32(CHKR3_CTRL, 0x19c)
    FIELD(CHKR3_CTRL, START_SINGLE, 8, 1)
    FIELD(CHKR3_CTRL, START_CONTINUOUS, 7, 1)
    FIELD(CHKR3_CTRL, CLKB_MUX_CTRL, 5, 1)
    FIELD(CHKR3_CTRL, CLKA_MUX_CTRL, 1, 3)
    FIELD(CHKR3_CTRL, ENABLE, 0, 1)
REG32(CHKR4_CLKA_UPPER, 0x1a0)
REG32(CHKR4_CLKA_LOWER, 0x1a4)
REG32(CHKR4_CLKB_CNT, 0x1a8)
REG32(CHKR4_CTRL, 0x1ac)
    FIELD(CHKR4_CTRL, START_SINGLE, 8, 1)
    FIELD(CHKR4_CTRL, START_CONTINUOUS, 7, 1)
    FIELD(CHKR4_CTRL, CLKB_MUX_CTRL, 5, 1)
    FIELD(CHKR4_CTRL, CLKA_MUX_CTRL, 1, 3)
    FIELD(CHKR4_CTRL, ENABLE, 0, 1)
REG32(CHKR5_CLKA_UPPER, 0x1b0)
REG32(CHKR5_CLKA_LOWER, 0x1b4)
REG32(CHKR5_CLKB_CNT, 0x1b8)
REG32(CHKR5_CTRL, 0x1bc)
    FIELD(CHKR5_CTRL, START_SINGLE, 8, 1)
    FIELD(CHKR5_CTRL, START_CONTINUOUS, 7, 1)
    FIELD(CHKR5_CTRL, CLKB_MUX_CTRL, 5, 1)
    FIELD(CHKR5_CTRL, CLKA_MUX_CTRL, 1, 3)
    FIELD(CHKR5_CTRL, ENABLE, 0, 1)
REG32(CHKR6_CLKA_UPPER, 0x1c0)
REG32(CHKR6_CLKA_LOWER, 0x1c4)
REG32(CHKR6_CLKB_CNT, 0x1c8)
REG32(CHKR6_CTRL, 0x1cc)
    FIELD(CHKR6_CTRL, START_SINGLE, 8, 1)
    FIELD(CHKR6_CTRL, START_CONTINUOUS, 7, 1)
    FIELD(CHKR6_CTRL, CLKB_MUX_CTRL, 5, 1)
    FIELD(CHKR6_CTRL, CLKA_MUX_CTRL, 1, 3)
    FIELD(CHKR6_CTRL, ENABLE, 0, 1)
REG32(CHKR7_CLKA_UPPER, 0x1d0)
REG32(CHKR7_CLKA_LOWER, 0x1d4)
REG32(CHKR7_CLKB_CNT, 0x1d8)
REG32(CHKR7_CTRL, 0x1dc)
    FIELD(CHKR7_CTRL, START_SINGLE, 8, 1)
    FIELD(CHKR7_CTRL, START_CONTINUOUS, 7, 1)
    FIELD(CHKR7_CTRL, CLKB_MUX_CTRL, 5, 1)
    FIELD(CHKR7_CTRL, CLKA_MUX_CTRL, 1, 3)
    FIELD(CHKR7_CTRL, ENABLE, 0, 1)
REG32(BOOT_MODE_USER, 0x200)
    FIELD(BOOT_MODE_USER, ALT_BOOT_MODE, 12, 4)
    FIELD(BOOT_MODE_USER, USE_ALT, 8, 1)
    FIELD(BOOT_MODE_USER, BOOT_MODE, 0, 4)
REG32(BOOT_MODE_POR, 0x204)
    FIELD(BOOT_MODE_POR, BOOT_MODE2, 8, 4)
    FIELD(BOOT_MODE_POR, BOOT_MODE1, 4, 4)
    FIELD(BOOT_MODE_POR, BOOT_MODE0, 0, 4)
REG32(RESET_CTRL, 0x218)
    FIELD(RESET_CTRL, SOFT_RESET, 4, 1)
    FIELD(RESET_CTRL, SRST_DIS, 0, 1)
REG32(BLOCKONLY_RST, 0x21c)
    FIELD(BLOCKONLY_RST, MIMIC, 3, 1)
    FIELD(BLOCKONLY_RST, DEBUG_ONLY, 0, 1)
REG32(RESET_REASON, 0x220)
    FIELD(RESET_REASON, MIMIC, 15, 1)
    FIELD(RESET_REASON, DEBUG_SYS, 6, 1)
    FIELD(RESET_REASON, SOFT, 5, 1)
    FIELD(RESET_REASON, SRST, 4, 1)
    FIELD(RESET_REASON, PSONLY_RESET_REQ, 3, 1)
    FIELD(RESET_REASON, PMU_SYS_RESET, 2, 1)
    FIELD(RESET_REASON, INTERNAL_POR, 1, 1)
    FIELD(RESET_REASON, EXTERNAL_POR, 0, 1)
REG32(RST_LPD_IOU0, 0x230)
    FIELD(RST_LPD_IOU0, GEM3_RESET, 3, 1)
    FIELD(RST_LPD_IOU0, GEM2_RESET, 2, 1)
    FIELD(RST_LPD_IOU0, GEM1_RESET, 1, 1)
    FIELD(RST_LPD_IOU0, GEM0_RESET, 0, 1)
REG32(RST_LPD_IOU2, 0x238)
    FIELD(RST_LPD_IOU2, TIMESTAMP_RESET, 20, 1)
    FIELD(RST_LPD_IOU2, IOU_CC_RESET, 19, 1)
    FIELD(RST_LPD_IOU2, GPIO_RESET, 18, 1)
    FIELD(RST_LPD_IOU2, ADMA_RESET, 17, 1)
    FIELD(RST_LPD_IOU2, NAND_RESET, 16, 1)
    FIELD(RST_LPD_IOU2, SWDT_RESET, 15, 1)
    FIELD(RST_LPD_IOU2, TTC3_RESET, 14, 1)
    FIELD(RST_LPD_IOU2, TTC2_RESET, 13, 1)
    FIELD(RST_LPD_IOU2, TTC1_RESET, 12, 1)
    FIELD(RST_LPD_IOU2, TTC0_RESET, 11, 1)
    FIELD(RST_LPD_IOU2, I2C1_RESET, 10, 1)
    FIELD(RST_LPD_IOU2, I2C0_RESET, 9, 1)
    FIELD(RST_LPD_IOU2, CAN1_RESET, 8, 1)
    FIELD(RST_LPD_IOU2, CAN0_RESET, 7, 1)
    FIELD(RST_LPD_IOU2, SDIO1_RESET, 6, 1)
    FIELD(RST_LPD_IOU2, SDIO0_RESET, 5, 1)
    FIELD(RST_LPD_IOU2, SPI1_RESET, 4, 1)
    FIELD(RST_LPD_IOU2, SPI0_RESET, 3, 1)
    FIELD(RST_LPD_IOU2, UART1_RESET, 2, 1)
    FIELD(RST_LPD_IOU2, UART0_RESET, 1, 1)
    FIELD(RST_LPD_IOU2, QSPI_RESET, 0, 1)
REG32(RST_LPD_TOP, 0x23c)
    FIELD(RST_LPD_TOP, FPD_RESET, 23, 1)
    FIELD(RST_LPD_TOP, LPD_SWDT_RESET, 20, 1)
    FIELD(RST_LPD_TOP, AFI_FM6_RESET, 19, 1)
    FIELD(RST_LPD_TOP, SYSMON_RESET, 17, 1)
    FIELD(RST_LPD_TOP, RTC_RESET, 16, 1)
    FIELD(RST_LPD_TOP, APM_RESET, 15, 1)
    FIELD(RST_LPD_TOP, IPI_RESET, 14, 1)
    FIELD(RST_LPD_TOP, USB1_APB_RESET, 11, 1)
    FIELD(RST_LPD_TOP, USB0_APB_RESET, 10, 1)
    FIELD(RST_LPD_TOP, USB1_HIBERRESET, 9, 1)
    FIELD(RST_LPD_TOP, USB0_HIBERRESET, 8, 1)
    FIELD(RST_LPD_TOP, USB1_CORERESET, 7, 1)
    FIELD(RST_LPD_TOP, USB0_CORERESET, 6, 1)
    FIELD(RST_LPD_TOP, RPU_PGE_RESET, 4, 1)
    FIELD(RST_LPD_TOP, OCM_RESET, 3, 1)
    FIELD(RST_LPD_TOP, RPU_AMBA_RESET, 2, 1)
    FIELD(RST_LPD_TOP, RPU_R51_RESET, 1, 1)
    FIELD(RST_LPD_TOP, RPU_R50_RESET, 0, 1)
REG32(RST_LPD_DBG, 0x240)
    FIELD(RST_LPD_DBG, DBG_ACK, 15, 1)
    FIELD(RST_LPD_DBG, RPU_DBG1_RESET, 5, 1)
    FIELD(RST_LPD_DBG, RPU_DBG0_RESET, 4, 1)
    FIELD(RST_LPD_DBG, DBG_LPD_RESET, 1, 1)
    FIELD(RST_LPD_DBG, DBG_FPD_RESET, 0, 1)
REG32(BANK3_CTRL0, 0x270)
    FIELD(BANK3_CTRL0, DRIVE0, 0, 10)
REG32(BANK3_CTRL1, 0x274)
    FIELD(BANK3_CTRL1, DRIVE1, 0, 10)
REG32(BANK3_CTRL2, 0x278)
    FIELD(BANK3_CTRL2, SCHMITT_CMOS_N, 0, 10)
REG32(BANK3_CTRL3, 0x27c)
    FIELD(BANK3_CTRL3, PULL_HIGH_LOW_N, 0, 10)
REG32(BANK3_CTRL4, 0x280)
    FIELD(BANK3_CTRL4, PULL_ENABLE, 0, 10)
REG32(BANK3_CTRL5, 0x284)
    FIELD(BANK3_CTRL5, SLOW_FAST_SLEW_N, 0, 10)
REG32(BANK3_STATUS, 0x288)
    FIELD(BANK3_STATUS, VMODE_1P8_3P3_N, 0, 1)

#define R_MAX (R_BANK3_STATUS + 1)

#define RPU_MAX_CPU 2

typedef struct CRL_APB {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    qemu_irq irq_ir;
    qemu_irq mon_irq_ir;
    qemu_irq rst_r5[RPU_MAX_CPU];
    qemu_irq srst_b;

    uint32_t regs[R_MAX];
    RegisterInfo regs_info[R_MAX];
} CRL_APB;

static void ir_update_irq(CRL_APB *s)
{
    bool pending = s->regs[R_IR_STATUS] & ~s->regs[R_IR_MASK];
    qemu_set_irq(s->irq_ir, pending);
}

static void ir_status_postw(RegisterInfo *reg, uint64_t val64)
{
    CRL_APB *s = XILINX_CRL_APB(reg->opaque);
    ir_update_irq(s);
}

static uint64_t ir_enable_prew(RegisterInfo *reg, uint64_t val64)
{
    CRL_APB *s = XILINX_CRL_APB(reg->opaque);
    uint32_t val = val64;

    s->regs[R_IR_MASK] &= ~val;
    ir_update_irq(s);
    return 0;
}

static uint64_t ir_disable_prew(RegisterInfo *reg, uint64_t val64)
{
    CRL_APB *s = XILINX_CRL_APB(reg->opaque);
    uint32_t val = val64;

    s->regs[R_IR_MASK] |= val;
    ir_update_irq(s);
    return 0;
}

static void mon_ir_update_irq(CRL_APB *s)
{
    bool pending = s->regs[R_CLKMON_STATUS] & ~s->regs[R_CLKMON_MASK];
    qemu_set_irq(s->mon_irq_ir, pending);
}

static void mon_ir_status_postw(RegisterInfo *reg, uint64_t val64)
{
    CRL_APB *s = XILINX_CRL_APB(reg->opaque);

    mon_ir_update_irq(s);
}

static uint64_t mon_ir_enable_prew(RegisterInfo *reg, uint64_t val64)
{
    CRL_APB *s = XILINX_CRL_APB(reg->opaque);
    uint32_t val = val64;

    s->regs[R_CLKMON_MASK] &= ~val;
    mon_ir_update_irq(s);
    return 0;
}

static uint64_t mon_ir_disable_prew(RegisterInfo *reg, uint64_t val64)
{
    CRL_APB *s = XILINX_CRL_APB(reg->opaque);
    uint32_t val = val64;

    s->regs[R_CLKMON_MASK] |= val;
    mon_ir_update_irq(s);
    return 0;
}

static uint64_t mon_ir_trigger_prew(RegisterInfo *reg, uint64_t val64)
{
    CRL_APB *s = XILINX_CRL_APB(reg->opaque);
    uint32_t val = val64;

    s->regs[R_CLKMON_STATUS] = s->regs[R_CLKMON_STATUS] | val;
    mon_ir_update_irq(s);
    return 0;
}

static void update_reset_reason_postw(RegisterInfo *reg, uint64_t val64)
{
    CRL_APB *s = XILINX_CRL_APB(reg->opaque);

    ARRAY_FIELD_DP32(s->regs, RESET_REASON, SOFT,
                ARRAY_FIELD_EX32(s->regs, RESET_CTRL, SOFT_RESET));
    ARRAY_FIELD_DP32(s->regs, RESET_REASON, DEBUG_SYS,
                ARRAY_FIELD_EX32(s->regs, BLOCKONLY_RST, DEBUG_ONLY));
}

static uint64_t rst_lpd_top_prew(RegisterInfo *reg, uint64_t val64)
{
    CRL_APB *s = XILINX_CRL_APB(reg->opaque);
    uint32_t val = val64;
    uint32_t val_old = s->regs[R_RST_LPD_TOP];
    unsigned int i;

    for (i = 0; i < RPU_MAX_CPU; i++) {
        /* Update irq's only when changed */
        uint32_t mask = (1 << (R_RST_LPD_TOP_RPU_R50_RESET_SHIFT + i));
        if ((val ^ val_old) & mask) {
            qemu_set_irq(s->rst_r5[i], !!(val & mask));
        }
    }
    return val64;
}

static uint64_t reset_ctrl_prew(RegisterInfo *reg, uint64_t val64)
{
    CRL_APB *s = XILINX_CRL_APB(reg->opaque);
    uint32_t val = val64;
    uint32_t val_old = s->regs[R_RESET_CTRL];

    if (!FIELD_EX32(val, RESET_CTRL, SRST_DIS)) {
        if ((val ^ val_old) & R_RESET_CTRL_SOFT_RESET_MASK) {
            qemu_set_irq(s->srst_b, !!(val & R_RESET_CTRL_SOFT_RESET_MASK));
        }
    }
    return val;
}

static const RegisterAccessInfo crl_apb_regs_info[] = {
    {   .name = "ERR_CTRL",  .addr = A_ERR_CTRL,
    },{ .name = "IR_STATUS",  .addr = A_IR_STATUS,
        .w1c = 0x1,
        .post_write = ir_status_postw,
    },{ .name = "IR_MASK",  .addr = A_IR_MASK,
        .reset = 0x1,
        .ro = 0x1,
    },{ .name = "IR_ENABLE",  .addr = A_IR_ENABLE,
        .pre_write = ir_enable_prew,
    },{ .name = "IR_DISABLE",  .addr = A_IR_DISABLE,
        .pre_write = ir_disable_prew,
    },{ .name = "CRL_WPROT",  .addr = A_CRL_WPROT,
    },{ .name = "IOPLL_CTRL",  .addr = A_IOPLL_CTRL,
        .reset = 0x12c09,
        .rsvd = 0xf88c80f6,
    },{ .name = "IOPLL_CFG",  .addr = A_IOPLL_CFG,
        .rsvd = 0x1801210,
    },{ .name = "IOPLL_FRAC_CFG",  .addr = A_IOPLL_FRAC_CFG,
        .rsvd = 0x7e330000,
    },{ .name = "RPLL_CTRL",  .addr = A_RPLL_CTRL,
        .reset = 0x12c09,
        .rsvd = 0xf88c80f6,
    },{ .name = "RPLL_CFG",  .addr = A_RPLL_CFG,
        .rsvd = 0x1801210,
    },{ .name = "RPLL_FRAC_CFG",  .addr = A_RPLL_FRAC_CFG,
        .rsvd = 0x7e330000,
    },{ .name = "PLL_STATUS",  .addr = A_PLL_STATUS,
        .reset = 0x18 | R_PLL_STATUS_RPLL_LOCK_MASK |
                 R_PLL_STATUS_IOPLL_LOCK_MASK,
        .rsvd = 0xffffffc4,
        .ro = 0x1b,
    },{ .name = "IOPLL_TO_FPD_CTRL",  .addr = A_IOPLL_TO_FPD_CTRL,
        .reset = 0x400,
        .rsvd = 0xc0ff,
    },{ .name = "RPLL_TO_FPD_CTRL",  .addr = A_RPLL_TO_FPD_CTRL,
        .reset = 0x400,
        .rsvd = 0xc0ff,
    },{ .name = "USB3_DUAL_REF_CTRL",  .addr = A_USB3_DUAL_REF_CTRL,
        .reset = 0x52000,
        .rsvd = 0xfdc0c0f8,
    },{ .name = "GEM0_REF_CTRL",  .addr = A_GEM0_REF_CTRL,
        .reset = 0x2500,
        .rsvd = 0xf9c0c0f8,
    },{ .name = "GEM1_REF_CTRL",  .addr = A_GEM1_REF_CTRL,
        .reset = 0x2500,
        .rsvd = 0xf9c0c0f8,
    },{ .name = "GEM2_REF_CTRL",  .addr = A_GEM2_REF_CTRL,
        .reset = 0x2500,
        .rsvd = 0xf9c0c0f8,
    },{ .name = "GEM3_REF_CTRL",  .addr = A_GEM3_REF_CTRL,
        .reset = 0x2500,
        .rsvd = 0xf9c0c0f8,
    },{ .name = "USB0_BUS_REF_CTRL",  .addr = A_USB0_BUS_REF_CTRL,
        .reset = 0x52000,
        .rsvd = 0xfdc0c0f8,
    },{ .name = "USB1_BUS_REF_CTRL",  .addr = A_USB1_BUS_REF_CTRL,
        .reset = 0x52000,
        .rsvd = 0xfdc0c0f8,
    },{ .name = "QSPI_REF_CTRL",  .addr = A_QSPI_REF_CTRL,
        .reset = 0x1000800,
        .rsvd = 0xfec0c0f8,
    },{ .name = "SDIO0_REF_CTRL",  .addr = A_SDIO0_REF_CTRL,
        .reset = 0x1000f00,
        .rsvd = 0xfec0c0f8,
    },{ .name = "SDIO1_REF_CTRL",  .addr = A_SDIO1_REF_CTRL,
        .reset = 0x1000f00,
        .rsvd = 0xfec0c0f8,
    },{ .name = "UART0_REF_CTRL",  .addr = A_UART0_REF_CTRL,
        .reset = 0x1001800,
        .rsvd = 0xfec0c0f8,
    },{ .name = "UART1_REF_CTRL",  .addr = A_UART1_REF_CTRL,
        .reset = 0x1001800,
        .rsvd = 0xfec0c0f8,
    },{ .name = "SPI0_REF_CTRL",  .addr = A_SPI0_REF_CTRL,
        .reset = 0x1001800,
        .rsvd = 0xfec0c0f8,
    },{ .name = "SPI1_REF_CTRL",  .addr = A_SPI1_REF_CTRL,
        .reset = 0x1001800,
        .rsvd = 0xfec0c0f8,
    },{ .name = "CAN0_REF_CTRL",  .addr = A_CAN0_REF_CTRL,
        .reset = 0x1001800,
        .rsvd = 0xfec0c0f8,
    },{ .name = "CAN1_REF_CTRL",  .addr = A_CAN1_REF_CTRL,
        .reset = 0x1001800,
        .rsvd = 0xfec0c0f8,
    },{ .name = "CPU_R5_CTRL",  .addr = A_CPU_R5_CTRL,
        .reset = 0x3000600,
        .rsvd = 0xfcffc0f8,
    },{ .name = "IOU_SWITCH_CTRL",  .addr = A_IOU_SWITCH_CTRL,
        .reset = 0x1500,
        .rsvd = 0xfeffc0f8,
    },{ .name = "CSU_PLL_CTRL",  .addr = A_CSU_PLL_CTRL,
        .reset = 0x1001500,
        .rsvd = 0xfeffc0f8,
    },{ .name = "PCAP_CTRL",  .addr = A_PCAP_CTRL,
        .reset = 0x1500,
        .rsvd = 0xfeffc0f8,
    },{ .name = "LPD_SWITCH_CTRL",  .addr = A_LPD_SWITCH_CTRL,
        .reset = 0x1000500,
        .rsvd = 0xfeffc0f8,
    },{ .name = "LPD_LSBUS_CTRL",  .addr = A_LPD_LSBUS_CTRL,
        .reset = 0x1001800,
        .rsvd = 0xfeffc0f8,
    },{ .name = "DBG_LPD_CTRL",  .addr = A_DBG_LPD_CTRL,
        .reset = 0x1002000,
        .rsvd = 0xfeffc0f8,
    },{ .name = "NAND_REF_CTRL",  .addr = A_NAND_REF_CTRL,
        .reset = 0x52000,
        .rsvd = 0xfec0c0f8,
    },{ .name = "ADMA_REF_CTRL",  .addr = A_ADMA_REF_CTRL,
        .reset = 0x2000,
        .rsvd = 0xfeffc0f8,
    },{ .name = "PL0_REF_CTRL",  .addr = A_PL0_REF_CTRL,
        .reset = 0x52000,
        .rsvd = 0xfec0c0f8,
    },{ .name = "PL1_REF_CTRL",  .addr = A_PL1_REF_CTRL,
        .reset = 0x52000,
        .rsvd = 0xfec0c0f8,
    },{ .name = "PL2_REF_CTRL",  .addr = A_PL2_REF_CTRL,
        .reset = 0x52000,
        .rsvd = 0xfec0c0f8,
    },{ .name = "PL3_REF_CTRL",  .addr = A_PL3_REF_CTRL,
        .reset = 0x52000,
        .rsvd = 0xfec0c0f8,
    },{ .name = "PL0_THR_CTRL",  .addr = A_PL0_THR_CTRL,
        .reset = 0x1,
        .rsvd = 0x7ffc,
        .ro = 0xffff8000,
    },{ .name = "PL0_THR_CNT",  .addr = A_PL0_THR_CNT,
    },{ .name = "PL1_THR_CTRL",  .addr = A_PL1_THR_CTRL,
        .reset = 0x1,
        .rsvd = 0x7ffc,
        .ro = 0xffff8000,
    },{ .name = "PL1_THR_CNT",  .addr = A_PL1_THR_CNT,
    },{ .name = "PL2_THR_CTRL",  .addr = A_PL2_THR_CTRL,
        .reset = 0x1,
        .rsvd = 0x7ffc,
        .ro = 0xffff8000,
    },{ .name = "PL2_THR_CNT",  .addr = A_PL2_THR_CNT,
    },{ .name = "PL3_THR_CTRL",  .addr = A_PL3_THR_CTRL,
        .reset = 0x1,
        .rsvd = 0x7ffc,
        .ro = 0xffff8000,
    },{ .name = "PL3_THR_CNT",  .addr = A_PL3_THR_CNT,
    },{ .name = "GEM_TSU_REF_CTRL",  .addr = A_GEM_TSU_REF_CTRL,
        .reset = 0x51000,
        .rsvd = 0xfec0c0f8,
    },{ .name = "DLL_REF_CTRL",  .addr = A_DLL_REF_CTRL,
        .rsvd = 0xf8,
    },{ .name = "AMS_REF_CTRL",  .addr = A_AMS_REF_CTRL,
        .reset = 0x1001800,
        .rsvd = 0xfec0c0f8,
    },{ .name = "I2C0_REF_CTRL",  .addr = A_I2C0_REF_CTRL,
        .reset = 0x1000500,
        .rsvd = 0xfec0c0f8,
    },{ .name = "I2C1_REF_CTRL",  .addr = A_I2C1_REF_CTRL,
        .reset = 0x1000500,
        .rsvd = 0xfec0c0f8,
    },{ .name = "TIMESTAMP_REF_CTRL",  .addr = A_TIMESTAMP_REF_CTRL,
        .reset = 0x1800,
        .rsvd = 0xfeffc0f8,
    },{ .name = "SAFTEY_CHK",  .addr = A_SAFTEY_CHK,
    },{ .name = "CLKMON_STATUS",  .addr = A_CLKMON_STATUS,
        .w1c = 0xFFFF,
        .post_write = mon_ir_status_postw,
    },{ .name = "CLKMON_MASK",  .addr = A_CLKMON_MASK,
        .reset = 0x0000FFFF,
        .ro = 0xFFFF,
    },{ .name = "CLKMON_ENABLE",  .addr = A_CLKMON_ENABLE,
        .pre_write = mon_ir_enable_prew,
    },{ .name = "CLKMON_DISABLE",  .addr = A_CLKMON_DISABLE,
        .pre_write = mon_ir_disable_prew,
    },{ .name = "CLKMON_TRIGGER",  .addr = A_CLKMON_TRIGGER,
        .pre_write = mon_ir_trigger_prew,
    },{ .name = "CHKR0_CLKA_UPPER",  .addr = A_CHKR0_CLKA_UPPER,
    },{ .name = "CHKR0_CLKA_LOWER",  .addr = A_CHKR0_CLKA_LOWER,
    },{ .name = "CHKR0_CLKB_CNT",  .addr = A_CHKR0_CLKB_CNT,
    },{ .name = "CHKR0_CTRL",  .addr = A_CHKR0_CTRL,
        .rsvd = 0x50,
    },{ .name = "CHKR1_CLKA_UPPER",  .addr = A_CHKR1_CLKA_UPPER,
    },{ .name = "CHKR1_CLKA_LOWER",  .addr = A_CHKR1_CLKA_LOWER,
    },{ .name = "CHKR1_CLKB_CNT",  .addr = A_CHKR1_CLKB_CNT,
    },{ .name = "CHKR1_CTRL",  .addr = A_CHKR1_CTRL,
    },{ .name = "CHKR2_CLKA_UPPER",  .addr = A_CHKR2_CLKA_UPPER,
    },{ .name = "CHKR2_CLKA_LOWER",  .addr = A_CHKR2_CLKA_LOWER,
    },{ .name = "CHKR2_CLKB_CNT",  .addr = A_CHKR2_CLKB_CNT,
    },{ .name = "CHKR2_CTRL",  .addr = A_CHKR2_CTRL,
        .rsvd = 0x50,
    },{ .name = "CHKR3_CLKA_UPPER",  .addr = A_CHKR3_CLKA_UPPER,
    },{ .name = "CHKR3_CLKA_LOWER",  .addr = A_CHKR3_CLKA_LOWER,
    },{ .name = "CHKR3_CLKB_CNT",  .addr = A_CHKR3_CLKB_CNT,
    },{ .name = "CHKR3_CTRL",  .addr = A_CHKR3_CTRL,
        .rsvd = 0x50,
    },{ .name = "CHKR4_CLKA_UPPER",  .addr = A_CHKR4_CLKA_UPPER,
    },{ .name = "CHKR4_CLKA_LOWER",  .addr = A_CHKR4_CLKA_LOWER,
    },{ .name = "CHKR4_CLKB_CNT",  .addr = A_CHKR4_CLKB_CNT,
    },{ .name = "CHKR4_CTRL",  .addr = A_CHKR4_CTRL,
        .rsvd = 0x50,
    },{ .name = "CHKR5_CLKA_UPPER",  .addr = A_CHKR5_CLKA_UPPER,
    },{ .name = "CHKR5_CLKA_LOWER",  .addr = A_CHKR5_CLKA_LOWER,
    },{ .name = "CHKR5_CLKB_CNT",  .addr = A_CHKR5_CLKB_CNT,
    },{ .name = "CHKR5_CTRL",  .addr = A_CHKR5_CTRL,
        .rsvd = 0x50,
    },{ .name = "CHKR6_CLKA_UPPER",  .addr = A_CHKR6_CLKA_UPPER,
    },{ .name = "CHKR6_CLKA_LOWER",  .addr = A_CHKR6_CLKA_LOWER,
    },{ .name = "CHKR6_CLKB_CNT",  .addr = A_CHKR6_CLKB_CNT,
    },{ .name = "CHKR6_CTRL",  .addr = A_CHKR6_CTRL,
        .rsvd = 0x50,
    },{ .name = "CHKR7_CLKA_UPPER",  .addr = A_CHKR7_CLKA_UPPER,
    },{ .name = "CHKR7_CLKA_LOWER",  .addr = A_CHKR7_CLKA_LOWER,
    },{ .name = "CHKR7_CLKB_CNT",  .addr = A_CHKR7_CLKB_CNT,
    },{ .name = "CHKR7_CTRL",  .addr = A_CHKR7_CTRL,
        .rsvd = 0x50,
    },
    {   .name = "BOOT_MODE_USER",  .addr = A_BOOT_MODE_USER,
        .reset = 0x4,
        .rsvd = 0xf0,
    },{ .name = "BOOT_MODE_POR",  .addr = A_BOOT_MODE_POR,
        .rsvd = 0xf000,
        .ro = 0xfff,
    },{ .name = "RESET_CTRL",  .addr = A_RESET_CTRL,
        .rsvd = 0xff8c,
        .pre_write = reset_ctrl_prew,
        .post_write = update_reset_reason_postw,
    },{ .name = "BLOCKONLY_RST",  .addr = A_BLOCKONLY_RST,
        .rsvd = 0x7dcc,
        .ro = 0x7dcc,
        .w1c = 0x8233,
        .post_write = update_reset_reason_postw,
    },{ .name = "RESET_REASON",  .addr = A_RESET_REASON,
        .rsvd = 0x7e0c,
        .ro = 0x7e0c,
        .w1c = 0x81f3,
        .reset = 0x1,
    },{ .name = "RST_LPD_IOU0",  .addr = A_RST_LPD_IOU0,
        .reset = 0xf,
        .rsvd = 0xfff0,
    },{ .name = "RST_LPD_IOU2",  .addr = A_RST_LPD_IOU2,
        .reset = 0x17ffff,
        .rsvd = 0xffe00000,
    },{ .name = "RST_LPD_TOP",  .addr = A_RST_LPD_TOP,
        .reset = 0x188fdf,
        .rsvd = 0x643020,
        .pre_write = rst_lpd_top_prew,
    },{ .name = "RST_LPD_DBG",  .addr = A_RST_LPD_DBG,
        .reset = 0x33,
        .rsvd = 0xffcc,
    },{ .name = "BANK3_CTRL0",  .addr = A_BANK3_CTRL0,
        .reset = 0x3ff,
    },{ .name = "BANK3_CTRL1",  .addr = A_BANK3_CTRL1,
        .reset = 0x3ff,
    },{ .name = "BANK3_CTRL2",  .addr = A_BANK3_CTRL2,
        .reset = 0x3ff,
    },{ .name = "BANK3_CTRL3",  .addr = A_BANK3_CTRL3,
        .reset = 0x3ff,
    },{ .name = "BANK3_CTRL4",  .addr = A_BANK3_CTRL4,
        .reset = 0x3ff,
    },{ .name = "BANK3_CTRL5",  .addr = A_BANK3_CTRL5,
    },{ .name = "BANK3_STATUS",  .addr = A_BANK3_STATUS,
        .ro = 0x1,
    }
};

static void crl_apb_reset(DeviceState *dev)
{
    CRL_APB *s = XILINX_CRL_APB(dev);
    unsigned int i;
    QemuOpts *opts = qemu_find_opts_singleton("boot-opts");
    uint32_t boot_mode = qemu_opt_get_number(opts, "mode", 0);
    uint32_t reset_reason_value = s->regs[R_RESET_REASON];

    assert(boot_mode < (1 << R_BOOT_MODE_USER_BOOT_MODE_LENGTH));

    for (i = 0; i < ARRAY_SIZE(s->regs_info); ++i) {
        register_reset(&s->regs_info[i]);
    }

    s->regs[R_RESET_REASON] = reset_reason_value;

    s->regs[R_BOOT_MODE_POR] = deposit32(s->regs[R_BOOT_MODE_POR],
                                         R_BOOT_MODE_POR_BOOT_MODE0_SHIFT,
                                         R_BOOT_MODE_POR_BOOT_MODE0_LENGTH,
                                         boot_mode);

    s->regs[R_BOOT_MODE_USER] =     deposit32(s->regs[R_BOOT_MODE_USER],
                                         R_BOOT_MODE_USER_BOOT_MODE_SHIFT,
                                         R_BOOT_MODE_USER_BOOT_MODE_LENGTH,
                                         boot_mode);
    ir_update_irq(s);

    qemu_set_irq(s->rst_r5[0], 1);
    qemu_set_irq(s->rst_r5[1], 1);
    qemu_set_irq(s->srst_b, 0);
}

static const MemoryRegionOps crl_apb_ops = {
    .read = register_read_memory,
    .write_with_attrs = register_write_memory_with_attrs,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void crl_apb_realize(DeviceState *dev, Error **errp)
{
    /* Delete this if you don't need it */
}

static void crl_apb_init(Object *obj)
{
    CRL_APB *s = XILINX_CRL_APB(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    RegisterInfoArray *reg_array;

    memory_region_init(&s->iomem, obj, TYPE_XILINX_CRL_APB, R_MAX * 4);
    reg_array =
        register_init_block32(DEVICE(obj), crl_apb_regs_info,
                              ARRAY_SIZE(crl_apb_regs_info),
                              s->regs_info, s->regs,
                              &crl_apb_ops,
                              XILINX_CRL_APB_ERR_DEBUG,
                              R_MAX * 4);
    memory_region_add_subregion(&s->iomem,
                                0x0,
                                &reg_array->mem);
    sysbus_init_mmio(sbd, &s->iomem);
    sysbus_init_irq(sbd, &s->irq_ir);
    qdev_init_gpio_out_named(DEVICE(obj), &s->mon_irq_ir, "clkmon_error_out",
                             1);
    qdev_init_gpio_out_named(DEVICE(obj), s->rst_r5, "RST_R5", 2);
    qdev_init_gpio_out_named(DEVICE(obj), &s->srst_b, "SRST_B", 2);
}

static const VMStateDescription vmstate_crl_apb = {
    .name = TYPE_XILINX_CRL_APB,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(regs, CRL_APB, R_MAX),
        VMSTATE_END_OF_LIST(),
    }
};

static const FDTGenericGPIOSet crl_gpios[] = {
    {
        .names = &fdt_generic_gpio_name_set_gpio,
        .gpios = (FDTGenericGPIOConnection[]) {
            { .name = "RST_R5",     .fdt_index = 0,     .range = 2 },
            { .name = "SRST_B",     .fdt_index = 2  },
            { },
        }
    },
    { },
};

static const FDTGenericGPIOSet crl_client_gpios [] = {
    {
        .names = &fdt_generic_gpio_name_set_gpio,
        .gpios = (FDTGenericGPIOConnection [])  {
            { .name = "clkmon_error_out", .fdt_index = 0, .range = 1 },
            { },
        },
    },
    { },
};

static void crl_apb_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    FDTGenericGPIOClass *fggc = FDT_GENERIC_GPIO_CLASS(klass);

    dc->reset = crl_apb_reset;
    dc->realize = crl_apb_realize;
    dc->vmsd = &vmstate_crl_apb;
    fggc->controller_gpios = crl_gpios;
    fggc->client_gpios = crl_client_gpios;
}

static const TypeInfo crl_apb_info = {
    .name          = TYPE_XILINX_CRL_APB,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(CRL_APB),
    .class_init    = crl_apb_class_init,
    .instance_init = crl_apb_init,
    .interfaces    = (InterfaceInfo[]) {
        { TYPE_FDT_GENERIC_GPIO },
        { }
    },
};

static void crl_apb_register_types(void)
{
    type_register_static(&crl_apb_info);
}

type_init(crl_apb_register_types)
