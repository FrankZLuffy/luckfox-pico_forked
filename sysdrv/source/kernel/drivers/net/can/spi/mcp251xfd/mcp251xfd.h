/* SPDX-License-Identifier: GPL-2.0
 *
 * mcp251xfd - Microchip MCP251xFD Family CAN controller driver
 *
 * Copyright (c) 2019 Pengutronix,
 *                    Marc Kleine-Budde <kernel@pengutronix.de>
 * Copyright (c) 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifndef _MCP251XFD_H
#define _MCP251XFD_H

#include <linux/can/core.h>
#include <linux/can/dev.h>
#include <linux/can/rx-offload.h>
#include <linux/gpio/consumer.h>
#include <linux/kernel.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spi.h>

/* MPC251x registers */

/* CAN FD Controller Module SFR */
// 寄存器3-7： CiCON —— CAN控制寄存器
#define MCP251XFD_REG_CON 0x00
// bit 31-28 TXBWS<3:0>：发送带宽共用位
// 两次连续传输之间的延时（以仲裁位时间为单位）
// 0000 = 无延时
// 0001 = 2
// 0010 = 4
// 0011 = 8
// 0100 = 16
// 0101 = 32
// 0110 = 64
// 0111 = 128
// 1000 = 256
// 1001 = 512
// 1010 = 1024
// 1011 = 2048
// 1111-1100 = 4096
#define MCP251XFD_REG_CON_TXBWS_MASK GENMASK(31, 28)
// bit 27 ABAT：中止所有等待的发送位
// 1 = 通知所有发送FIFO中止发送
// 0 = 模块将在所有发送中止时清零该位
#define MCP251XFD_REG_CON_ABAT BIT(27)
// bit 26-24 REQOP<2:0>：请求工作模式位
// 000 = 设置为正常CAN FD模式；支持混用CAN FD帧和经典CAN 2.0帧
// 001 = 设置为休眠模式
// 010 = 设置为内部环回模式
// 011 = 设置为仅监听模式
// 100 = 设置为配置模式
// 101 = 设置为外部环回模式
// 110 = 设置为正常CAN 2.0模式；接收CAN FD帧时可能生成错误帧
// 111 = 设置为受限工作模式
#define MCP251XFD_REG_CON_REQOP_MASK GENMASK(26, 24)
#define MCP251XFD_REG_CON_MODE_MIXED 0
#define MCP251XFD_REG_CON_MODE_SLEEP 1
#define MCP251XFD_REG_CON_MODE_INT_LOOPBACK 2
#define MCP251XFD_REG_CON_MODE_LISTENONLY 3
#define MCP251XFD_REG_CON_MODE_CONFIG 4
#define MCP251XFD_REG_CON_MODE_EXT_LOOPBACK 5
#define MCP251XFD_REG_CON_MODE_CAN2_0 6
#define MCP251XFD_REG_CON_MODE_RESTRICTED 7
// bit 23-21 OPMOD<2:0>：工作模式状态位
// 000 = 模块处于正常CAN FD模式；支持混用CAN FD帧和经典CAN 2.0帧
// 001 = 模块处于休眠模式
// 010 = 模块处于内部环回模式
// 011 = 模块处于仅监听模式
// 100 = 模块处于配置模式
// 101 = 模块处于外部环回模式
// 110 = 模块处于正常CAN 2.0模式；接收CAN FD帧时可能生成错误帧
// 111 = 模块处于受限工作模式
#define MCP251XFD_REG_CON_OPMOD_MASK GENMASK(23, 21)
// bit 20 TXQEN：使能发送队列位(1)
// 1 = 使能TXQ并在RAM中预留空间
// 0 = 不在RAM中为TXQ预留空间
#define MCP251XFD_REG_CON_TXQEN BIT(20)
// bit 19 STEF：发送事件FIFO存储位(1)
// 1 = 将发送的报文保存到TEF中并在RAM中预留空间
// 0 = 不将发送的报文保存到TEF中
#define MCP251XFD_REG_CON_STEF BIT(19)
// bit 18 SERR2LOM：发生系统错误时切换到仅监听模式位(1)
// 1 = 切换到仅监听模式
// 0 = 切换到受限工作模式
#define MCP251XFD_REG_CON_SERR2LOM BIT(18)
// bit 17 ESIGM：在网关模式下发送ESI位(1)
// 1 = 当报文的ESI为高电平或CAN控制器处于被动错误状态时， ESI隐性发送
// 0 = ESI反映CAN控制器的错误状态
#define MCP251XFD_REG_CON_ESIGM BIT(17)
// bit 16 RTXAT：限制重发尝试位(1)
// 1 = 重发尝试受限，使用CiFIFOCONm.TXAT
// 0 = 重发尝试次数不受限， CiFIFOCONm.TXAT将被忽略
#define MCP251XFD_REG_CON_RTXAT BIT(16)
// bit 12 BRSDIS：比特率切换禁止位
// 1 = 无论发送报文对象中的BRS状态如何，都禁止比特率切换
// 0 = 根据发送报文对象中的BRS进行比特率切换
#define MCP251XFD_REG_CON_BRSDIS BIT(12)
// bit 11 BUSY： CAN模块忙状态位
// 1 = CAN模块正在发送或接收报文
// 0 = CAN模块不工作
#define MCP251XFD_REG_CON_BUSY BIT(11)
// bit 10-9 WFT<1:0>：可选唤醒滤波器时间位
// 00 = T00FILTER
// 01 = T01FILTER
// 10 = T10FILTER
// 11 = T11FILTER
// 注： 请参见表7-5。
#define MCP251XFD_REG_CON_WFT_MASK GENMASK(10, 9)
#define MCP251XFD_REG_CON_WFT_T00FILTER 0x0
#define MCP251XFD_REG_CON_WFT_T01FILTER 0x1
#define MCP251XFD_REG_CON_WFT_T10FILTER 0x2
#define MCP251XFD_REG_CON_WFT_T11FILTER 0x3
// bit 8 WAKFIL：使能CAN总线线路唤醒滤波器位(1)
// 1 = 使用CAN总线线路滤波器来唤醒
// 0 = 不使用CAN总线线路滤波器来唤醒
#define MCP251XFD_REG_CON_WAKFIL BIT(8)
// bit 6 PXEDIS：协议异常事件检测禁止位(1)
// 隐性FDF位后的隐性“保留位”称为“协议异常”。
// 1 = 协议异常被视为格式错误。
// 0 = 如果检测到协议异常， CAN FD控制器模块将进入总线集成状态。
#define MCP251XFD_REG_CON_PXEDIS BIT(6)
// bit 5 ISOCRCEN：使能CAN FD帧中的ISO CRC位(1)
// 1 = CRC字段中包含填充位计数，使用非零CRC初始化向量（符合ISO 11898-1:2015规范）
// 0 = CRC字段中不包含填充位计数，使用全零CRC初始化向量
#define MCP251XFD_REG_CON_ISOCRCEN BIT(5)
// bit 4-0 DNCNT<4:0>： DeviceNet过滤器位编号位
// 10011-11111 = 无效选择（最多可将数据的18位与EID进行比较）
// 10010 = 最多可将数据字节2的bit 6与EID17进行比较
// ...
// 00001 = 最多可将数据字节0的bit 7与EID0进行比较
// 00000 = 不比较数据字节
#define MCP251XFD_REG_CON_DNCNT_MASK GENMASK(4, 0)

// 寄存器3-8： CiNBTCFG ——标称位时间配置寄存器
#define MCP251XFD_REG_NBTCFG 0x04
// bit 31-24 BRP<7:0>：波特率预分频比位
// 1111 1111 = TQ = 256/Fsys
// ...
// 0000 0000 = TQ = 1/Fsys
#define MCP251XFD_REG_NBTCFG_BRP_MASK GENMASK(31, 24)
// bit 23-16 TSEG1<7:0>：时间段1位（传播段 + 相位段1）
// 1111 1111 = 长度为256个TQ
// ...
// 0000 0000 = 长度为1个TQ
#define MCP251XFD_REG_NBTCFG_TSEG1_MASK GENMASK(23, 16)
// bit 14-8 TSEG2<6:0>：时间段2位（相位段2）
// 111 1111 = 长度为128个TQ
// ...
// 000 0000 = 长度为1个TQ
#define MCP251XFD_REG_NBTCFG_TSEG2_MASK GENMASK(14, 8)
// bit 6-0 SJW<6:0>：同步跳转宽度位
// 111 1111 = 长度为128个TQ
// ...
// 000 0000 = 长度为1个TQ
#define MCP251XFD_REG_NBTCFG_SJW_MASK GENMASK(6, 0)

// 寄存器3-9： CiDBTCFG ——数据位时间配置寄存器
#define MCP251XFD_REG_DBTCFG 0x08
// bit 31-24 BRP<7:0>：波特率预分频比位
// 1111 1111 = TQ = 256/Fsys
// ...
// 0000 0000 = TQ = 1/Fsys
#define MCP251XFD_REG_DBTCFG_BRP_MASK GENMASK(31, 24)
// bit 20-16 TSEG1<4:0>：时间段1位（传播段 + 相位段1）
// 1 1111 = 长度为32个TQ
// ...
// 0 0000 = 长度为1个TQ
#define MCP251XFD_REG_DBTCFG_TSEG1_MASK GENMASK(20, 16)
// bit 11-8 TSEG2<3:0>：时间段2位（相位段2）
// 1111 = 长度为16个TQ
// ...
// 0000 = 长度为1个TQ
#define MCP251XFD_REG_DBTCFG_TSEG2_MASK GENMASK(11, 8)
// bit 3-0 SJW<3:0>：同步跳转宽度位
// 1111 = 长度为16个TQ
// ...
// 0000 = 长度为1个TQ
#define MCP251XFD_REG_DBTCFG_SJW_MASK GENMASK(3, 0)

// 寄存器3-10： CiTDC ——发送器延时补偿寄存器
#define MCP251XFD_REG_TDC 0x0c
// bit 25 EDGFLTEN：使能在总线集成状态下边沿滤波位
// 1 = 根据ISO 11898-1:2015标准使能边沿滤波
// 0 = 禁止边沿滤波
#define MCP251XFD_REG_TDC_EDGFLTEN BIT(25)
// bit 24 SID11EN：使能CAN FD基本格式报文中的12位SID位
// 1 = RRS用作CAN FD基本格式报文中的SID11： SID<11:0> = {SID<10:0>, SID11}
// 0 = 不使用RRS； SID<10:0>符合ISO 11898-1:2015规范
#define MCP251XFD_REG_TDC_SID11EN BIT(24)
// bit 17-16 TDCMOD<1:0>：发送器延时补偿模式位；二次采样点（Secondary Sample Point， SSP）
// 10-11 = 自动；测量延时并添加TDCO。
// 01 = 手动；不测量，使用来自寄存器的TDCV + TDCO
// 00 = 禁止TDC
#define MCP251XFD_REG_TDC_TDCMOD_MASK GENMASK(17, 16)
#define MCP251XFD_REG_TDC_TDCMOD_AUTO 2
#define MCP251XFD_REG_TDC_TDCMOD_MANUAL 1
#define MCP251XFD_REG_TDC_TDCMOD_DISABLED 0
// bit 14-8 TDCO<6:0>：发送器延时补偿偏移位；二次采样点（SSP）
// 二进制补码；偏移可以是正值、零或负值。
// 011 1111 = 63 x TSYSCLK
// ...
// 000 0000 = 0 x TSYSCLK
// ...
// 111 1111 = –64 x TSYSCLK
#define MCP251XFD_REG_TDC_TDCO_MASK GENMASK(14, 8)
// bit 5-0 TDCV<5:0>：发送器延时补偿值位；二次采样点（SSP）
// 11 1111 = 63 x TSYSCLK
// ...
// 00 0000 = 0 x TSYSCLK
#define MCP251XFD_REG_TDC_TDCV_MASK GENMASK(5, 0)

// 寄存器3-11： CiTBC ——时基计数器寄存器
// bit 31-0 TBC<31:0>：时基计数器位
// 这是自由运行的定时器，当TBCEN置1时，每经过一个TBCPRE时钟递增一次。
#define MCP251XFD_REG_TBC 0x10

// 寄存器3-12： CiTSCON ——时间戳控制寄存器
#define MCP251XFD_REG_TSCON 0x14
// bit 18 TSRES：时间戳保留位（仅限FD帧）
// 1 = 在FDF位后的位的采样点
// 0 = 在SOF的采样点
#define MCP251XFD_REG_TSCON_TSRES BIT(18)
// bit 17 TSEOF：时间戳EOF位
// 1 = 在帧生效后添加时间戳：
// - 在EOF的倒数第二位之前RX未产生错误
// - 在EOF结束之前TX未产生错误
// 0 = 在帧“开始”时添加时间戳：
// - 经典帧：在SOF的采样点
// - FD帧：请参见TSRES位。
#define MCP251XFD_REG_TSCON_TSEOF BIT(17)
// bit 16 TBCEN：时基计数器使能位
// 1 = 使能TBC
// 0 = 停止并复位TBC
#define MCP251XFD_REG_TSCON_TBCEN BIT(16)
// bit 9-0 TBCPRE<9:0>：时基计数器预分频比位
// 1023 = 每经过1024个时钟TBC递增一次
// ...
// 0 = 每经过1个时钟TBC递增一次
#define MCP251XFD_REG_TSCON_TBCPRE_MASK GENMASK(9, 0)

#define MCP251XFD_REG_VEC 0x18
#define MCP251XFD_REG_VEC_RXCODE_MASK GENMASK(30, 24)
#define MCP251XFD_REG_VEC_TXCODE_MASK GENMASK(22, 16)
#define MCP251XFD_REG_VEC_FILHIT_MASK GENMASK(12, 8)
#define MCP251XFD_REG_VEC_ICODE_MASK GENMASK(6, 0)

// 寄存器3-14： CiINT ——中断寄存器
#define MCP251XFD_REG_INT 0x1c
// 0-15，状态位
#define MCP251XFD_REG_INT_IF_MASK GENMASK(15, 0)
// 16-31，使能位。
#define MCP251XFD_REG_INT_IE_MASK GENMASK(31, 16)
// bit 31 IVMIE：无效报文中断允许位
#define MCP251XFD_REG_INT_IVMIE BIT(31)
// bit 30 WAKIE：总线唤醒中断允许位
#define MCP251XFD_REG_INT_WAKIE BIT(30)
// bit 29 CERRIE： CAN总线错误中断允许位
#define MCP251XFD_REG_INT_CERRIE BIT(29)
// bit 28 SERRIE：系统错误中断允许位
#define MCP251XFD_REG_INT_SERRIE BIT(28)
// bit 27 RXOVIE：接收FIFO溢出中断允许位
#define MCP251XFD_REG_INT_RXOVIE BIT(27)
// bit 26 TXATIE：发送尝试中断允许位
#define MCP251XFD_REG_INT_TXATIE BIT(26)
// bit 25 SPICRCIE： SPI CRC错误中断允许位
#define MCP251XFD_REG_INT_SPICRCIE BIT(25)
// bit 24 ECCIE： ECC错误中断允许位
#define MCP251XFD_REG_INT_ECCIE BIT(24)
// bit 20 TEFIE：发送事件FIFO中断允许位
#define MCP251XFD_REG_INT_TEFIE BIT(20)
// bit 19 MODIE：模式改变中断允许位
#define MCP251XFD_REG_INT_MODIE BIT(19)
// bit 18 TBCIE：时基计数器中断允许位
#define MCP251XFD_REG_INT_TBCIE BIT(18)
// bit 17 RXIE：接收FIFO中断允许位
#define MCP251XFD_REG_INT_RXIE BIT(17)
// bit 16 TXIE：发送FIFO中断允许位
#define MCP251XFD_REG_INT_TXIE BIT(16)
// bit 15 IVMIF：无效报文中断标志位(1)
#define MCP251XFD_REG_INT_IVMIF BIT(15)
// bit 14 WAKIF：总线唤醒中断标志位(1)
#define MCP251XFD_REG_INT_WAKIF BIT(14)
// bit 13 CERRIF： CAN总线错误中断标志位(1)
#define MCP251XFD_REG_INT_CERRIF BIT(13)
// bit 12 SERRIF：系统错误中断标志位(1)
// 1 = 发生了系统错误
// 0 = 未发生系统错误
#define MCP251XFD_REG_INT_SERRIF BIT(12)
// bit 11 RXOVIF：接收对象溢出中断标志位
// 1 = 发生了接收FIFO溢出
// 0 = 未发生接收FIFO溢出
#define MCP251XFD_REG_INT_RXOVIF BIT(11)
// bit 10 TXATIF：发送尝试中断标志位
#define MCP251XFD_REG_INT_TXATIF BIT(10)
// bit 9 SPICRCIF： SPI CRC错误中断标志位
#define MCP251XFD_REG_INT_SPICRCIF BIT(9)
// bit 8 ECCIF： ECC错误中断标志位
#define MCP251XFD_REG_INT_ECCIF BIT(8)
// bit 4 TEFIF：发送事件FIFO中断标志位
// 1 = 有TEF中断待处理
// 0 = 没有TEF中断待处理
#define MCP251XFD_REG_INT_TEFIF BIT(4)
// bit 3 MODIF：工作模式改变中断标志位(1)
// 1 = 工作模式发生了改变（OPMOD已改变）
// 0 = 模式未发生改变
#define MCP251XFD_REG_INT_MODIF BIT(3)
// bit 2 TBCIF：时基计数器溢出中断标志位(1)
// 1 = TBC已溢出
// 0 = TBC未溢出
#define MCP251XFD_REG_INT_TBCIF BIT(2)
// bit 1 RXIF：接收FIFO中断标志位
// 1 = 有接收FIFO中断待处理
// 0 = 没有接收FIFO中断待处理
#define MCP251XFD_REG_INT_RXIF BIT(1)
// bit 0 TXIF：发送FIFO中断标志位
// 1 = 有发送FIFO中断待处理
// 0 = 没有发送FIFO中断待处理
#define MCP251XFD_REG_INT_TXIF BIT(0)
/* These IRQ flags must be cleared by SW in the CAN_INT register */
#define MCP251XFD_REG_INT_IF_CLEARABLE_MASK                                    \
	(MCP251XFD_REG_INT_IVMIF | MCP251XFD_REG_INT_WAKIF |                   \
	 MCP251XFD_REG_INT_CERRIF | MCP251XFD_REG_INT_SERRIF |                 \
	 MCP251XFD_REG_INT_MODIF)

#define MCP251XFD_REG_RXIF 0x20
#define MCP251XFD_REG_TXIF 0x24
// 寄存器3-16： CiRXOVIF ——接收溢出中断状态寄存器
// bit 31-1 RFOVIF<31:1>：接收FIFO溢出中断待处理位
// 1 = 中断待处理
// 0 = 中断未处于待处理状态
#define MCP251XFD_REG_RXOVIF 0x28
#define MCP251XFD_REG_TXATIF 0x2c
#define MCP251XFD_REG_TXREQ 0x30

// 寄存器3-20： CiTREC——发送/接收错误计数寄存器
#define MCP251XFD_REG_TREC 0x34
// bit 21 TXBO：发送器处于离线状态位（TEC > 255）
// 在配置模式下，由于模块未处于总线上而将TXBO置1。
#define MCP251XFD_REG_TREC_TXBO BIT(21)
// bit 20 TXBP：发送器处于被动错误状态位（TEC > 127）
#define MCP251XFD_REG_TREC_TXBP BIT(20)
// bit 19 RXBP：接收器处于被动错误状态位（REC > 127）
#define MCP251XFD_REG_TREC_RXBP BIT(19)
// bit 18 TXWARN：发送器处于警告错误状态位（128 > TEC > 95）
#define MCP251XFD_REG_TREC_TXWARN BIT(18)
// bit 17 RXWARN：接收器处于警告错误状态位（128 > REC > 95）
#define MCP251XFD_REG_TREC_RXWARN BIT(17)
// bit 16 EWARN：发送器或接收器处于警告错误状态位
#define MCP251XFD_REG_TREC_EWARN BIT(16)
// bit 15-8 TEC<7:0>：发送错误计数器位
#define MCP251XFD_REG_TREC_TEC_MASK GENMASK(15, 8)
// bit 7-0 REC<7:0>：接收错误计数器位
#define MCP251XFD_REG_TREC_REC_MASK GENMASK(7, 0)

// 寄存器3-21： CiBDIAG0 ——总线诊断寄存器0
#define MCP251XFD_REG_BDIAG0 0x38
// bit 31-24 DTERRCNT<7:0>：数据比特率发送错误计数器位
#define MCP251XFD_REG_BDIAG0_DTERRCNT_MASK GENMASK(31, 24)
// bit 23-16 DRERRCNT<7:0>：数据比特率接收错误计数器位
#define MCP251XFD_REG_BDIAG0_DRERRCNT_MASK GENMASK(23, 16)
// bit 15-8 NTERRCNT<7:0>：标称比特率发送错误计数器位
#define MCP251XFD_REG_BDIAG0_NTERRCNT_MASK GENMASK(15, 8)
// bit 7-0 NRERRCNT<7:0>：标称比特率接收错误计数器位
#define MCP251XFD_REG_BDIAG0_NRERRCNT_MASK GENMASK(7, 0)

// 寄存器3-22： CiBDIAG1 ——总线诊断寄存器1
#define MCP251XFD_REG_BDIAG1 0x3c
// bit 31 DLCMM： DLC不匹配位
// 在发送或接收期间，指定的DLC大于FIFO元素的PLSIZE。
#define MCP251XFD_REG_BDIAG1_DLCMM BIT(31)
// bit 30 ESI：接收的CAN FD报文的ESI标志置1。
#define MCP251XFD_REG_BDIAG1_ESI BIT(30)
// bit 29 DCRCERR：与标称比特率相同（见下文）。
#define MCP251XFD_REG_BDIAG1_DCRCERR BIT(29)
// bit 28 DSTUFERR：与标称比特率相同（见下文）。
#define MCP251XFD_REG_BDIAG1_DSTUFERR BIT(28)
// bit 27 DFORMERR：与标称比特率相同（见下文）。
#define MCP251XFD_REG_BDIAG1_DFORMERR BIT(27)
// bit 25 DBIT1ERR：与标称比特率相同（见下文）。
#define MCP251XFD_REG_BDIAG1_DBIT1ERR BIT(25)
// bit 24 DBIT0ERR：与标称比特率相同（见下文）。
#define MCP251XFD_REG_BDIAG1_DBIT0ERR BIT(24)
// bit 23 TXBOERR：器件进入离线状态（且自动恢复）。
#define MCP251XFD_REG_BDIAG1_TXBOERR BIT(23)
// bit 21 NCRCERR：接收的报文的CRC校验和不正确。
// 输入报文的CRC与通过接收到的数据计算得到的CRC不匹配。
#define MCP251XFD_REG_BDIAG1_NCRCERR BIT(21)
// bit 20 NSTUFERR：在接收报文的一部分中，序列中包含了5个以上相等位，而报文中不允许出现这种序列。
#define MCP251XFD_REG_BDIAG1_NSTUFERR BIT(20)
// bit 19 NFORMERR：接收帧的固定格式部分格式错误。
#define MCP251XFD_REG_BDIAG1_NFORMERR BIT(19)
// bit 18 NACKERR：发送报文未应答。
#define MCP251XFD_REG_BDIAG1_NACKERR BIT(18)
// bit 17 NBIT1ERR：在发送报文（仲裁字段除外）期间，
// 器件要发送隐性电平（逻辑值为1的位），但监视到的总线值为显性。
#define MCP251XFD_REG_BDIAG1_NBIT1ERR BIT(17)
// bit 16 NBIT0ERR： 在发送报文（或应答位、主动错误标志或过载标志）期间，
// 器件要发送显性电平（逻辑值为0的数据或标识符位），但监视的总线值为隐性。
#define MCP251XFD_REG_BDIAG1_NBIT0ERR BIT(16)
#define MCP251XFD_REG_BDIAG1_BERR_MASK                                         \
	(MCP251XFD_REG_BDIAG1_DLCMM | MCP251XFD_REG_BDIAG1_ESI |               \
	 MCP251XFD_REG_BDIAG1_DCRCERR | MCP251XFD_REG_BDIAG1_DSTUFERR |        \
	 MCP251XFD_REG_BDIAG1_DFORMERR | MCP251XFD_REG_BDIAG1_DBIT1ERR |       \
	 MCP251XFD_REG_BDIAG1_DBIT0ERR | MCP251XFD_REG_BDIAG1_TXBOERR |        \
	 MCP251XFD_REG_BDIAG1_NCRCERR | MCP251XFD_REG_BDIAG1_NSTUFERR |        \
	 MCP251XFD_REG_BDIAG1_NFORMERR | MCP251XFD_REG_BDIAG1_NACKERR |        \
	 MCP251XFD_REG_BDIAG1_NBIT1ERR | MCP251XFD_REG_BDIAG1_NBIT0ERR)
// bit 15-0 EFMSGCNT<15:0>：无错误报文计数器位
#define MCP251XFD_REG_BDIAG1_EFMSGCNT_MASK GENMASK(15, 0)

// 寄存器3-23： CiTEFCON——发送事件FIFO控制寄存器
#define MCP251XFD_REG_TEFCON 0x40
// bit 28-24 FSIZE<4:0>： FIFO大小位(1)
// 0_0000 = FIFO深度为1个报文
// 0_0001 = FIFO深度为2个报文
// 0_0010 = FIFO深度为3个报文
// ...
// 1_1111 = FIFO深度为32个报文
#define MCP251XFD_REG_TEFCON_FSIZE_MASK GENMASK(28, 24)
// bit 10 FRESET： FIFO复位位
// 1 = 当该位置1时， FIFO复位；当FIFO复位时，该位由硬件清零。用户在采取任何操作前应等待该位清零。
// 0 = 无影响
#define MCP251XFD_REG_TEFCON_FRESET BIT(10)
// bit 8 UINC：递增尾部位
// 当该位置1时， FIFO尾部递增一个报文。
#define MCP251XFD_REG_TEFCON_UINC BIT(8)
// bit 5 TEFTSEN：发送事件FIFO时间戳使能位(1)
// 1 = 对TEF中的对象加时间戳
// 0 = 不对TEF中的对象加时间戳
#define MCP251XFD_REG_TEFCON_TEFTSEN BIT(5)
// bit 3 TEFOVIE：发送事件FIFO溢出中断允许位
// 1 = 允许在发生溢出事件时产生中断
// 0 = 禁止在发生溢出事件时产生中断
#define MCP251XFD_REG_TEFCON_TEFOVIE BIT(3)
// bit 2 TEFFIE：发送事件FIFO满中断允许位
// 1 = 允许在FIFO满时产生中断
// 0 = 禁止在FIFO满时产生中断
#define MCP251XFD_REG_TEFCON_TEFFIE BIT(2)
// bit 1 TEFHIE：发送事件FIFO半满中断允许位
// 1 = 允许在FIFO半满时产生中断
// 0 = 禁止在FIFO半满时产生中断
#define MCP251XFD_REG_TEFCON_TEFHIE BIT(1)
// bit 0 TEFNEIE：发送事件FIFO非空中断允许位
// 1 = 允许在FIFO非空时产生中断
// 0 = 禁止在FIFO非空时产生中断
#define MCP251XFD_REG_TEFCON_TEFNEIE BIT(0)

// 寄存器3-24： CiTEFSTA ——发送事件FIFO状态寄存器
#define MCP251XFD_REG_TEFSTA 0x44
// bit 3 TEFOVIF：发送事件FIFO溢出中断标志位
// 1 = 发生了溢出事件
// 0 = 未发生溢出事件
#define MCP251XFD_REG_TEFSTA_TEFOVIF BIT(3)
// bit 2 TEFFIF：发送事件FIFO满中断标志位(1)
// 1 = FIFO已满
// 0 = FIFO未满
#define MCP251XFD_REG_TEFSTA_TEFFIF BIT(2)
// bit 1 TEFHIF：发送事件FIFO半满中断标志位(1)
// 1 = FIFO ≥ 半满
// 0 = FIFO < 半满
#define MCP251XFD_REG_TEFSTA_TEFHIF BIT(1)
// bit 0 TEFNEIF：发送事件FIFO非空中断标志位(1)
// 1 = FIFO非空，至少包含一个报文
// 0 = FIFO为空
#define MCP251XFD_REG_TEFSTA_TEFNEIF BIT(0)

// 寄存器3-25： CiTEFUA ——发送事件FIFO用户地址寄存器
#define MCP251XFD_REG_TEFUA 0x48

// 寄存器3-26： CiTXQCON ——发送队列控制寄存器
#define MCP251XFD_REG_TXQCON 0x50
#define MCP251XFD_REG_TXQCON_PLSIZE_MASK GENMASK(31, 29)
#define MCP251XFD_REG_TXQCON_PLSIZE_8 0
#define MCP251XFD_REG_TXQCON_PLSIZE_12 1
#define MCP251XFD_REG_TXQCON_PLSIZE_16 2
#define MCP251XFD_REG_TXQCON_PLSIZE_20 3
#define MCP251XFD_REG_TXQCON_PLSIZE_24 4
#define MCP251XFD_REG_TXQCON_PLSIZE_32 5
#define MCP251XFD_REG_TXQCON_PLSIZE_48 6
#define MCP251XFD_REG_TXQCON_PLSIZE_64 7
#define MCP251XFD_REG_TXQCON_FSIZE_MASK GENMASK(28, 24)
#define MCP251XFD_REG_TXQCON_TXAT_UNLIMITED 3
#define MCP251XFD_REG_TXQCON_TXAT_THREE_SHOT 1
#define MCP251XFD_REG_TXQCON_TXAT_ONE_SHOT 0
#define MCP251XFD_REG_TXQCON_TXAT_MASK GENMASK(22, 21)
#define MCP251XFD_REG_TXQCON_TXPRI_MASK GENMASK(20, 16)
#define MCP251XFD_REG_TXQCON_FRESET BIT(10)
#define MCP251XFD_REG_TXQCON_TXREQ BIT(9)
#define MCP251XFD_REG_TXQCON_UINC BIT(8)
#define MCP251XFD_REG_TXQCON_TXEN BIT(7)
#define MCP251XFD_REG_TXQCON_TXATIE BIT(4)
#define MCP251XFD_REG_TXQCON_TXQEIE BIT(2)
#define MCP251XFD_REG_TXQCON_TXQNIE BIT(0)

#define MCP251XFD_REG_TXQSTA 0x54
#define MCP251XFD_REG_TXQSTA_TXQCI_MASK GENMASK(12, 8)
#define MCP251XFD_REG_TXQSTA_TXABT BIT(7)
#define MCP251XFD_REG_TXQSTA_TXLARB BIT(6)
#define MCP251XFD_REG_TXQSTA_TXERR BIT(5)
#define MCP251XFD_REG_TXQSTA_TXATIF BIT(4)
#define MCP251XFD_REG_TXQSTA_TXQEIF BIT(2)
#define MCP251XFD_REG_TXQSTA_TXQNIF BIT(0)

#define MCP251XFD_REG_TXQUA 0x58

// 寄存器3-29： CiFIFOCONm —— FIFO控制寄存器m（m = 1至31）
#define MCP251XFD_REG_FIFOCON(x) (0x50 + 0xc * (x))
// bit 31-29 PLSIZE<2:0>： 有效负载大小位;
// 000 = 8个数据字节
// 001 = 12个数据字节
// 010 = 16个数据字节
// 011 = 20个数据字节
// 100 = 24个数据字节
// 101 = 32个数据字节
// 110 = 48个数据字节
// 111 = 64个数据字节
#define MCP251XFD_REG_FIFOCON_PLSIZE_MASK GENMASK(31, 29)
#define MCP251XFD_REG_FIFOCON_PLSIZE_8 0
#define MCP251XFD_REG_FIFOCON_PLSIZE_12 1
#define MCP251XFD_REG_FIFOCON_PLSIZE_16 2
#define MCP251XFD_REG_FIFOCON_PLSIZE_20 3
#define MCP251XFD_REG_FIFOCON_PLSIZE_24 4
#define MCP251XFD_REG_FIFOCON_PLSIZE_32 5
#define MCP251XFD_REG_FIFOCON_PLSIZE_48 6
#define MCP251XFD_REG_FIFOCON_PLSIZE_64 7
// bit 28-24 FSIZE<4:0>： FIFO大小位
// 0_0000 = FIFO深度为1个报文
// 0_0001 = FIFO深度为2个报文
// 0_0010 = FIFO深度为3个报文
// ...
// 1_1111 = FIFO深度为32个报文
#define MCP251XFD_REG_FIFOCON_FSIZE_MASK GENMASK(28, 24)
// bit 22-21 TXAT<1:0>： 重发尝试位
// CiCON.RTXAT置1时使能该功能。
// 00 = 禁止重发尝试
// 01 = 3次重发尝试
// 10 = 重发尝试次数不受限制
// 11 = 重发尝试次数不受限制
#define MCP251XFD_REG_FIFOCON_TXAT_MASK GENMASK(22, 21)
#define MCP251XFD_REG_FIFOCON_TXAT_ONE_SHOT 0
#define MCP251XFD_REG_FIFOCON_TXAT_THREE_SHOT 1
#define MCP251XFD_REG_FIFOCON_TXAT_UNLIMITED 3
// bit 20-16 TXPRI<4:0>： 报文发送优先级位
// 00000 = 最低报文优先级
// ...
// 11111 = 最高报文优先级
#define MCP251XFD_REG_FIFOCON_TXPRI_MASK GENMASK(20, 16)
// bit 10 FRESET： FIFO复位位(3)
// 1 = 当该位置1时， FIFO复位；当FIFO复位时，该位由硬件清零。用户在采取任何操作前应等待该位清零。
// 0 = 无影响
#define MCP251XFD_REG_FIFOCON_FRESET BIT(10)
// bit 9 TXREQ： 报文发送请求位(2)
// TXEN = 1（FIFO配置为发送FIFO）
// 1 = 请求发送报文；在成功发送FIFO中排队的所有报文之后，该位会自动清零。
// 0 = 在该位置1的情况下清零该位将请求中止报文。
// TXEN = 0（FIFO配置为接收FIFO）该位无影响。
#define MCP251XFD_REG_FIFOCON_TXREQ BIT(9)
// bit 8 UINC： 递增头部/尾部位
// TXEN = 1（FIFO配置为发送FIFO）
// 当该位置1时， FIFO头部递增一个报文。
// TXEN = 0（FIFO配置为接收FIFO）
// 当该位置1时， FIFO尾部递增一个报文。
#define MCP251XFD_REG_FIFOCON_UINC BIT(8)
// bit 7 TXEN： TX/RX FIFO选择位(1)
// 1 = 发送FIFO
// 0 = 接收FIFO
#define MCP251XFD_REG_FIFOCON_TXEN BIT(7)
// bit 6 RTREN：自动RTR使能位
// 1 = 接收到远程发送时， TXREQ置1。
// 0 = 接收到远程发送时， TXREQ不受影响。
#define MCP251XFD_REG_FIFOCON_RTREN BIT(6)
// bit 5 RXTSEN：接收的报文时间戳使能位(1)
// 1 = 捕捉RAM中接收到的报文对象的时间戳。
// 0 = 不捕捉时间戳。
#define MCP251XFD_REG_FIFOCON_RXTSEN BIT(5)
// bit 4 TXATIE：超过发送尝试次数中断允许位
// 1 = 允许中断
// 0 = 禁止中断
#define MCP251XFD_REG_FIFOCON_TXATIE BIT(4)
// bit 3 RXOVIE：溢出中断允许位
// 1 = 允许在发生溢出事件时产生中断
// 0 = 禁止在发生溢出事件时产生中断
#define MCP251XFD_REG_FIFOCON_RXOVIE BIT(3)
// bit 2 TFERFFIE：发送/接收FIFO空/满中断允许位
// TXEN = 1（FIFO配置为发送FIFO）
// 发送FIFO空中断允许
// 1 = 允许在FIFO为空时产生中断
// 0 = 禁止在FIFO为空时产生中断
// TXEN = 0（FIFO配置为接收FIFO）
// 接收FIFO满中断允许
// 1 = 允许在FIFO满时产生中断
// 0 = 禁止在FIFO满时产生中断
#define MCP251XFD_REG_FIFOCON_TFERFFIE BIT(2)
// bit 1 TFHRFHIE：发送/接收FIFO半空/半满中断允许位
// TXEN = 1（FIFO配置为发送FIFO）
// 发送FIFO半空中断允许
// 1 = 允许在FIFO半空时产生中断
// 0 = 禁止在FIFO半空时产生中断
// TXEN = 0（FIFO配置为接收FIFO）
// 接收FIFO半满中断允许
// 1 = 允许在FIFO半满时产生中断
// 0 = 禁止在FIFO半满时产生中断
#define MCP251XFD_REG_FIFOCON_TFHRFHIE BIT(1)
// bit 0 TFNRFNIE：发送/接收FIFO未满/非空中断允许位
// TXEN = 1（FIFO配置为发送FIFO）
// 发送FIFO未满中断允许
// 1 = 允许在FIFO未满时产生中断
// 0 = 禁止在FIFO未满时产生中断
// TXEN = 0（FIFO配置为接收FIFO）
// 接收FIFO非空中断允许
// 1 = 允许在FIFO非空时产生中断
// 0 = 禁止在FIFO非空时产生中断
#define MCP251XFD_REG_FIFOCON_TFNRFNIE BIT(0)

// 寄存器3-30： CiFIFOSTAm ——FIFO状态寄存器m（m = 1至31）
#define MCP251XFD_REG_FIFOSTA(x) (0x54 + 0xc * (x))
// bit 12-8 FIFOCI<4:0>： FIFO报文索引位(1)
// TXEN = 1（FIFO配置为发送FIFO）
// 读取该位域将返回一个索引，该索引指向FIFO下一次尝试发送的报文。
// TXEN = 0（FIFO配置为接收FIFO）
// 读取该位域将返回一个索引， FIFO使用该索引保存下一个报文。
#define MCP251XFD_REG_FIFOSTA_FIFOCI_MASK GENMASK(12, 8)
// bit 7 TXABT： 报文中止状态位(2)(3)
// 1 = 报文中止
// 0 = 报文成功完成
#define MCP251XFD_REG_FIFOSTA_TXABT BIT(7)
// bit 6 TXLARB： 报文仲裁失败状态位(2)(3)
// 1 = 报文在发送过程中仲裁失败
// 0 = 报文在发送过程中仲裁未失败
#define MCP251XFD_REG_FIFOSTA_TXLARB BIT(6)
// bit 5 TXERR： 在发送过程中检测到错误位(2)(3)
// 1 = 发送报文时发生总线错误
// 0 = 发送报文时未发生总线错误
#define MCP251XFD_REG_FIFOSTA_TXERR BIT(5)
// bit 4 TXATIF： 超过发送尝试次数中断待处理位
// TXEN = 1（FIFO配置为发送FIFO）
// 1 = 中断待处理
// 0 = 中断未处于待处理状态
// TXEN = 0（FIFO配置为接收FIFO）
// 读为0
#define MCP251XFD_REG_FIFOSTA_TXATIF BIT(4)
// bit 3 RXOVIF： 接收FIFO溢出中断标志位
// TXEN = 1（FIFO配置为发送FIFO）
// 未使用，读为0
// TXEN = 0（FIFO配置为接收FIFO）
// 1 = 发生了溢出事件
// 0 = 未发生溢出事件
#define MCP251XFD_REG_FIFOSTA_RXOVIF BIT(3)
// bit 2 TFERFFIF： 发送/接收FIFO空/满中断标志位
// TXEN = 1（FIFO配置为发送FIFO）
// 发送FIFO空中断标志
// 1 = FIFO为空
// 0 = FIFO非空；至少有一个报文在排队等待发送
// TXEN = 0（FIFO配置为接收FIFO）
// 接收FIFO满中断标志
// 1 = FIFO已满
// 0 = FIFO未满
#define MCP251XFD_REG_FIFOSTA_TFERFFIF BIT(2)
// bit 1 TFHRFHIF： 发送/接收FIFO半空/半满中断标志位
// TXEN = 1（FIFO配置为发送FIFO）
// 发送FIFO半空中断标志
// 1 = FIFO ≤ 半满
// 0 = FIFO > 半满
// TXEN = 0（FIFO配置为接收FIFO）
// 接收FIFO半满中断标志
// 1 = FIFO ≥ 半满
// 0 = FIFO < 半满
#define MCP251XFD_REG_FIFOSTA_TFHRFHIF BIT(1)
// bit 0 TFNRFNIF： 发送/接收FIFO未满/非空中断标志位
// TXEN = 1（FIFO配置为发送FIFO）
// 发送FIFO未满中断标志
// 1 = FIFO未满
// 0 = FIFO已满
// TXEN = 0（FIFO配置为接收FIFO）
// 接收FIFO非空中断标志
// 1 = FIFO非空，至少包含一个报文
// 0 = FIFO为空
#define MCP251XFD_REG_FIFOSTA_TFNRFNIF BIT(0)

// 寄存器3-31： CiFIFOUAm —— FIFO用户地址寄存器m（m = 1至31）
// bit 31-0 FIFOUA<31:0>： FIFO用户地址位
// TXEN = 1（FIFO配置为发送FIFO）
// 读取该寄存器将返回用于写入下一个报文的地址（FIFO头部）。
// TXEN = 0（FIFO配置为接收FIFO）
// 读取该寄存器将返回用于读取下一个报文的地址（FIFO尾部）。
#define MCP251XFD_REG_FIFOUA(x) (0x58 + 0xc * (x))

// 寄存器3-32： CiFLTCONm ——过滤器控制寄存器m（m = 0至7）
#define MCP251XFD_REG_FLTCON(x) (0x1d0 + 0x4 * (x))
// bit 31 FLTEN3： 使能过滤器3接收报文位
// 1 = 使能过滤器
// 0 = 禁止过滤器
#define MCP251XFD_REG_FLTCON_FLTEN3 BIT(31)
// bit 28-24 F3BP<4:0>： 过滤器3命中时指向FIFO的指针位(1)
// 1_1111 = 与过滤器匹配的报文存储在FIFO 31中
// 1_1110 = 与过滤器匹配的报文存储在FIFO 30中
// ........
// 0_0010 = 与过滤器匹配的报文存储在FIFO 2中
// 0_0001 = 与过滤器匹配的报文存储在FIFO 1中
// 0_0000 = 保留； FIFO 0为TX队列，不能接收报文
#define MCP251XFD_REG_FLTCON_F3BP_MASK GENMASK(28, 24)
// bit 23 FLTEN2： 使能过滤器2接收报文位
// 1 = 使能过滤器
// 0 = 禁止过滤器
#define MCP251XFD_REG_FLTCON_FLTEN2 BIT(23)
// bit 20-16 F2BP<4:0>： 过滤器2命中时指向FIFO的指针位(1)
// 1_1111 = 与过滤器匹配的报文存储在FIFO 31中
// 1_1110 = 与过滤器匹配的报文存储在FIFO 30中
// ........
// 0_0010 = 与过滤器匹配的报文存储在FIFO 2中
// 0_0001 = 与过滤器匹配的报文存储在FIFO 1中
// 0_0000 = 保留； FIFO 0为TX队列，不能接收报文
#define MCP251XFD_REG_FLTCON_F2BP_MASK GENMASK(20, 16)
// bit 15 FLTEN1： 使能过滤器1接收报文位
// 1 = 使能过滤器
// 0 = 禁止过滤器
#define MCP251XFD_REG_FLTCON_FLTEN1 BIT(15)
// bit 12-8 F1BP<4:0>： 过滤器1命中时指向FIFO的指针位(1)
// 1_1111 = 与过滤器匹配的报文存储在FIFO 31中
// 1_1110 = 与过滤器匹配的报文存储在FIFO 30中
// ........
// 0_0010 = 与过滤器匹配的报文存储在FIFO 2中
// 0_0001 = 与过滤器匹配的报文存储在FIFO 1中
// 0_0000 = 保留； FIFO 0为TX队列，不能接收报文
#define MCP251XFD_REG_FLTCON_F1BP_MASK GENMASK(12, 8)
// bit 7 FLTEN0： 使能过滤器0接收报文位
// 1 = 使能过滤器
// 0 = 禁止过滤器
#define MCP251XFD_REG_FLTCON_FLTEN0 BIT(7)
// bit 4-0 F0BP<4:0>： 过滤器0命中时指向FIFO的指针位(1)
// 1_1111 = 与过滤器匹配的报文存储在FIFO 31中
// 1_1110 = 与过滤器匹配的报文存储在FIFO 30中
// ........
// 0_0010 = 与过滤器匹配的报文存储在FIFO 2中
// 0_0001 = 与过滤器匹配的报文存储在FIFO 1中
// 0_0000 = 保留； FIFO 0为TX队列，不能接收报文
#define MCP251XFD_REG_FLTCON_F0BP_MASK GENMASK(4, 0)
#define MCP251XFD_REG_FLTCON_FLTEN(x) (BIT(7) << 8 * ((x) & 0x3))
#define MCP251XFD_REG_FLTCON_FLT_MASK(x) (GENMASK(7, 0) << (8 * ((x) & 0x3)))
#define MCP251XFD_REG_FLTCON_FBP(x, fifo) ((fifo) << 8 * ((x) & 0x3))

#define MCP251XFD_REG_FLTOBJ(x) (0x1f0 + 0x8 * (x))
#define MCP251XFD_REG_FLTOBJ_EXIDE BIT(30)
#define MCP251XFD_REG_FLTOBJ_SID11 BIT(29)
#define MCP251XFD_REG_FLTOBJ_EID_MASK GENMASK(28, 11)
#define MCP251XFD_REG_FLTOBJ_SID_MASK GENMASK(10, 0)

#define MCP251XFD_REG_FLTMASK(x) (0x1f4 + 0x8 * (x))
#define MCP251XFD_REG_MASK_MIDE BIT(30)
#define MCP251XFD_REG_MASK_MSID11 BIT(29)
#define MCP251XFD_REG_MASK_MEID_MASK GENMASK(28, 11)
#define MCP251XFD_REG_MASK_MSID_MASK GENMASK(10, 0)

/* RAM */
#define MCP251XFD_RAM_START 0x400
#define MCP251XFD_RAM_SIZE SZ_2K

/* Message Object */
// bit T0.29 SID11： 在FD模式下，标准ID可通过r1扩展为12位
#define MCP251XFD_OBJ_ID_SID11 BIT(29)
// bit T0.28-11 EID<17:0>： 扩展标识符
#define MCP251XFD_OBJ_ID_EID_MASK GENMASK(28, 11)
// bit T0.10-0 SID<10:0>： 标准标识符
#define MCP251XFD_OBJ_ID_SID_MASK GENMASK(10, 0)
// bit T1.31-9 SEQ<22:0>： 用于跟踪发送事件FIFO中已发送报文的序列
#define MCP251XFD_OBJ_FLAGS_SEQ_MCP2518FD_MASK GENMASK(31, 9)
#define MCP251XFD_OBJ_FLAGS_SEQ_MCP2517FD_MASK GENMASK(15, 9)
#define MCP251XFD_OBJ_FLAGS_SEQ_MASK MCP251XFD_OBJ_FLAGS_SEQ_MCP2518FD_MASK
// bit T1.8 ESI： 错误状态指示符
// 在CAN-CAN网关模式（CiCON.ESIGM=1）下，发送的ESI标志为T1.ESI与CAN控制器被动错误状态
// 的“逻辑或”结果。
// 在正常模式下， ESI指示错误状态
// 1 = 发送节点处于被动错误状态
// 0 = 发送节点处于主动错误状态
#define MCP251XFD_OBJ_FLAGS_ESI BIT(8)
// bit T1.7 FDF： FD帧；用于区分CAN和CAN FD格式
#define MCP251XFD_OBJ_FLAGS_FDF BIT(7)
// bit T1.6 BRS： 比特率切换；选择是否切换数据比特率
#define MCP251XFD_OBJ_FLAGS_BRS BIT(6)
// bit T1.5 RTR： 远程发送请求；不适用于CAN FD
#define MCP251XFD_OBJ_FLAGS_RTR BIT(5)
// bit T1.4 IDE： 标识符扩展标志；用于区分基本格式和扩展格式
#define MCP251XFD_OBJ_FLAGS_IDE BIT(4)
// bit T1.3-0 DLC<3:0>： 数据长度码
#define MCP251XFD_OBJ_FLAGS_DLC GENMASK(3, 0)

#define MCP251XFD_REG_FRAME_EFF_SID_MASK GENMASK(28, 18)
#define MCP251XFD_REG_FRAME_EFF_EID_MASK GENMASK(17, 0)

/* MCP2517/18FD SFR */
// 寄存器3-1： OSC——MCP2518FD振荡器控制寄存器
#define MCP251XFD_REG_OSC 0xe00
// bit 12 SCLKRDY： 同步SCLKDIV位
// 1 = SCLKDIV 1
// 0 = SCLKDIV 0
#define MCP251XFD_REG_OSC_SCLKRDY BIT(12)
// bit 10 OSCRDY： 时钟就绪
// 1 = 时钟正在运行且保持稳定
// 0 = 时钟未就绪或已关闭
#define MCP251XFD_REG_OSC_OSCRDY BIT(10)
// bit 8 PLLRDY： PLL就绪
// 1 = PLL锁定
// 0 = PLL未就绪
#define MCP251XFD_REG_OSC_PLLRDY BIT(8)
// 3为11 = CLKO 10分频;与bit 6-5 CLKODIV<1:0>： 时钟输出分频比
#define MCP251XFD_REG_OSC_CLKODIV_10 3
#define MCP251XFD_REG_OSC_CLKODIV_4 2
#define MCP251XFD_REG_OSC_CLKODIV_2 1
#define MCP251XFD_REG_OSC_CLKODIV_1 0
// bit 6-5 CLKODIV<1:0>： 时钟输出分频比
// 11 = CLKO 10分频
// 10 = CLKO 4分频
// 01 = CLKO 2分频
// 00 = CLKO 1分频
#define MCP251XFD_REG_OSC_CLKODIV_MASK GENMASK(6, 5)
// bit 4 SCLKDIV： 系统时钟分频比(1)
// 1 = SCLK 2分频
// 0 = SCLK 1分频
#define MCP251XFD_REG_OSC_SCLKDIV BIT(4)
// bit 3 LPMEN： 低功耗模式（LPM）使能(3)
// 1 = 在LPM下，器件将停止时钟并使芯片的大部分电路进入掉电模式。寄存器值和RAM值将丢失。
// 器件将在nCS置为有效或RXCAN活动时唤醒。
// 0 = 在休眠模式下，器件将停止时钟并保留其寄存器值和RAM值。
// 器件将在OSCDIS位清零或RXCAN活动时唤醒。
#define MCP251XFD_REG_OSC_LPMEN BIT(3)
// bit 2 OSCDIS： 时钟（振荡器）禁止(2)
// 1 = 禁止时钟，器件处于休眠模式。
// 0 = 使能时钟
#define MCP251XFD_REG_OSC_OSCDIS BIT(2)
// bit 0 PLLEN： PLL使能(1)
// 1 = 系统时钟来自10x PLL
// 0 = 系统时钟直接来自XTAL振荡器
#define MCP251XFD_REG_OSC_PLLEN BIT(0)

// 寄存器3-2： IOCON——输入/输出控制寄存器
#define MCP251XFD_REG_IOCON 0xe04
// bit 30 INTOD： 中断引脚漏极开路模式
// 1 = 漏极开路输出
// 0 = 推/挽输出
#define MCP251XFD_REG_IOCON_INTOD BIT(30)
// bit 29 SOF： 帧起始信号
// 1 = CLKO引脚上出现SOF信号
// 0 = CLKO引脚上出现时钟
#define MCP251XFD_REG_IOCON_SOF BIT(29)
// bit 28 TXCANOD： TXCAN漏极开路模式
// 1 = 漏极开路输出
// 0 = 推/挽输出
#define MCP251XFD_REG_IOCON_TXCANOD BIT(28)
// bit 25 PM1： GPIO引脚模式
// 1 = 引脚用作GPIO1
// 0 = 中断引脚INT1，在CiINT.RXIF和RXIE置1时置为有效
#define MCP251XFD_REG_IOCON_PM1 BIT(25)
// bit 24 PM0： GPIO引脚模式
// 1 = 引脚用作GPIO0
// 0 = 中断引脚INT0，在CiINT.TXIF和TXIE置1时置为有效
#define MCP251XFD_REG_IOCON_PM0 BIT(24)
// bit 17 GPIO1： GPIO1状态
// 1 = VGPIO1 > VIH
// 0 = VGPIO1 < VIL
#define MCP251XFD_REG_IOCON_GPIO1 BIT(17)
// bit 16 GPIO0： GPIO0状态
// 1 = VGPIO0 > VIH
// 0 = VGPIO0 < VIL
#define MCP251XFD_REG_IOCON_GPIO0 BIT(16)
// bit 9 LAT1： GPIO1锁存器
// 1 = 将引脚驱动为高电平
// 0 = 将引脚驱动为低电平
#define MCP251XFD_REG_IOCON_LAT1 BIT(9)
// bit 8 LAT0： GPIO0锁存器
// 1 = 将引脚驱动为高电平
// 0 = 将引脚驱动为低电平
#define MCP251XFD_REG_IOCON_LAT0 BIT(8)
// bit 6 XSTBYEN： 使能收发器待机引脚控制
// 1 = 使能XSTBY控制
// 0 = 禁止XSTBY控制
#define MCP251XFD_REG_IOCON_XSTBYEN BIT(6)
// bit 1 TRIS1： GPIO1数据方向(1)
// 1 = 输入引脚
// 0 = 输出引脚
#define MCP251XFD_REG_IOCON_TRIS1 BIT(1)
// bit 0 TRIS0： GPIO0数据方向(1)
// 1 = 输入引脚
// 0 = 输出引脚
#define MCP251XFD_REG_IOCON_TRIS0 BIT(0)

// 寄存器3-3： CRC —— CRC寄存器
#define MCP251XFD_REG_CRC 0xe08
// bit 25 FERRIE： CRC命令格式错误中断允许
#define MCP251XFD_REG_CRC_FERRIE BIT(25)
// bit 24 CRCERRIE： CRC错误中断允许
#define MCP251XFD_REG_CRC_CRCERRIE BIT(24)
// bit 17 FERRIF： CRC命令格式错误中断标志
// 1 = “SPI + CRC”命令发生期间字节数不匹配
// 0 = 未发生SPI CRC命令格式错误
#define MCP251XFD_REG_CRC_FERRIF BIT(17)
// bit 16 CRCERRIF： CRC错误中断标志
// 1 = 发生CRC不匹配
// 0 = 未发生CRC错误
#define MCP251XFD_REG_CRC_CRCERRIF BIT(16)
#define MCP251XFD_REG_CRC_IF_MASK GENMASK(17, 16)
// bit 15-0 CRC<15:0>： 自上一次CRC不匹配起的循环冗余校验
#define MCP251XFD_REG_CRC_MASK GENMASK(15, 0)

// 寄存器3-4： ECCCON —— ECC控制寄存器
#define MCP251XFD_REG_ECCCON 0xe0c
// bit 14-8 PARITY<6:0>： 禁止ECC时，在写入RAM期间使用的奇偶校验位
#define MCP251XFD_REG_ECCCON_PARITY_MASK GENMASK(14, 8)
// bit 2 DEDIE： 双位错误检测中断允许标志
#define MCP251XFD_REG_ECCCON_DEDIE BIT(2)
// bit 1 SECIE： 单个位错误纠正中断允许标志
#define MCP251XFD_REG_ECCCON_SECIE BIT(1)
// bit 0 ECCEN： ECC使能
// 1 = 使能ECC
// 0 = 禁止ECC
#define MCP251XFD_REG_ECCCON_ECCEN BIT(0)

// 寄存器3-5： ECCSTAT —— ECC状态寄存器
#define MCP251XFD_REG_ECCSTAT 0xe10
// bit 27-16 ERRADDR<11:0>： 发生上一个ECC错误的地址
#define MCP251XFD_REG_ECCSTAT_ERRADDR_MASK GENMASK(27, 16)
#define MCP251XFD_REG_ECCSTAT_IF_MASK GENMASK(2, 1)
// bit 2 DEDIF： 双位错误检测中断标志
// 1 = 检测到双位错误
// 0 = 未检测到双位错误
#define MCP251XFD_REG_ECCSTAT_DEDIF BIT(2)
// bit 1 SECIF： 单个位错误纠正中断标志
// 1 = 纠正了单个位错误
// 0 = 未发生单个位错误
#define MCP251XFD_REG_ECCSTAT_SECIF BIT(1)

// 寄存器3-6： DEVID——器件ID寄存器
#define MCP251XFD_REG_DEVID 0xe14 /* MCP2518FD only */
// bit 7-4 ID[3:0]： 器件ID
#define MCP251XFD_REG_DEVID_ID_MASK GENMASK(7, 4)
// bit 3-0 REV[3:0]： 芯片版本
#define MCP251XFD_REG_DEVID_REV_MASK GENMASK(3, 0)

/* number of TX FIFO objects, depending on CAN mode
 *
 * FIFO setup: tef: 8*12 bytes = 96 bytes, tx: 8*16 bytes = 128 bytes
 * FIFO setup: tef: 4*12 bytes = 48 bytes, tx: 4*72 bytes = 288 bytes
 */
#define MCP251XFD_TX_OBJ_NUM_CAN 8
#define MCP251XFD_TX_OBJ_NUM_CANFD 4

#if MCP251XFD_TX_OBJ_NUM_CAN > MCP251XFD_TX_OBJ_NUM_CANFD
#define MCP251XFD_TX_OBJ_NUM_MAX MCP251XFD_TX_OBJ_NUM_CAN
#else
#define MCP251XFD_TX_OBJ_NUM_MAX MCP251XFD_TX_OBJ_NUM_CANFD
#endif

#define MCP251XFD_NAPI_WEIGHT 32
// 分配了一个发送FIFO通道
#define MCP251XFD_TX_FIFO 1
#define MCP251XFD_RX_FIFO(x) (MCP251XFD_TX_FIFO + 1 + (x))

/* SPI commands */
#define MCP251XFD_SPI_INSTRUCTION_RESET 0x0000
#define MCP251XFD_SPI_INSTRUCTION_WRITE 0x2000
#define MCP251XFD_SPI_INSTRUCTION_READ 0x3000
#define MCP251XFD_SPI_INSTRUCTION_WRITE_CRC 0xa000
#define MCP251XFD_SPI_INSTRUCTION_READ_CRC 0xb000
#define MCP251XFD_SPI_INSTRUCTION_WRITE_CRC_SAFE 0xc000
#define MCP251XFD_SPI_ADDRESS_MASK GENMASK(11, 0)

#define MCP251XFD_SYSCLOCK_HZ_MAX 40000000
#define MCP251XFD_SYSCLOCK_HZ_MIN 1000000
#define MCP251XFD_SPICLOCK_HZ_MAX 20000000
#define MCP251XFD_OSC_PLL_MULTIPLIER 10
#define MCP251XFD_OSC_STAB_SLEEP_US (3 * USEC_PER_MSEC)
#define MCP251XFD_OSC_STAB_TIMEOUT_US (10 * MCP251XFD_OSC_STAB_SLEEP_US)
#define MCP251XFD_POLL_SLEEP_US (10)
#define MCP251XFD_POLL_TIMEOUT_US (USEC_PER_MSEC)
#define MCP251XFD_SOFTRESET_RETRIES_MAX 3
#define MCP251XFD_READ_CRC_RETRIES_MAX 3
#define MCP251XFD_ECC_CNT_MAX 2
#define MCP251XFD_SANITIZE_SPI 1
#define MCP251XFD_SANITIZE_CAN 1

/* Silence TX MAB overflow warnings */
#define MCP251XFD_QUIRK_MAB_NO_WARN BIT(0)
/* Use CRC to access registers */
#define MCP251XFD_QUIRK_CRC_REG BIT(1)
/* Use CRC to access RX/TEF-RAM */
#define MCP251XFD_QUIRK_CRC_RX BIT(2)
/* Use CRC to access TX-RAM */
#define MCP251XFD_QUIRK_CRC_TX BIT(3)
/* Enable ECC for RAM */
#define MCP251XFD_QUIRK_ECC BIT(4)
/* Use Half Duplex SPI transfers */
#define MCP251XFD_QUIRK_HALF_DUPLEX BIT(5)

struct mcp251xfd_hw_tef_obj {
	u32 id;
	u32 flags;
	u32 ts;
};

/* The tx_obj_raw version is used in spi async, i.e. without
 * regmap. We have to take care of endianness ourselves.
 */
struct __packed mcp251xfd_hw_tx_obj_raw {
	__le32 id;
	__le32 flags;
	u8 data[sizeof_field(struct canfd_frame, data)];
};

struct mcp251xfd_hw_tx_obj_can {
	u32 id;
	u32 flags;
	u8 data[sizeof_field(struct can_frame, data)];
};

struct mcp251xfd_hw_tx_obj_canfd {
	u32 id;
	u32 flags;
	u8 data[sizeof_field(struct canfd_frame, data)];
};

struct mcp251xfd_hw_rx_obj_can {
	u32 id;
	u32 flags;
	u32 ts;
	u8 data[sizeof_field(struct can_frame, data)];
};

struct mcp251xfd_hw_rx_obj_canfd {
	u32 id;
	u32 flags;
	u32 ts;
	u8 data[sizeof_field(struct canfd_frame, data)];
};

struct mcp251xfd_tef_ring {
	unsigned int head;
	unsigned int tail;

	/* u8 obj_num equals tx_ring->obj_num */
	/* u8 obj_size equals sizeof(struct mcp251xfd_hw_tef_obj) */
};

struct __packed mcp251xfd_buf_cmd {
	__be16 cmd;
};

struct __packed mcp251xfd_buf_cmd_crc {
	__be16 cmd;
	u8 len;
};

union mcp251xfd_tx_obj_load_buf {
	struct __packed {
		struct mcp251xfd_buf_cmd cmd;
		struct mcp251xfd_hw_tx_obj_raw hw_tx_obj;
	} nocrc;
	struct __packed {
		struct mcp251xfd_buf_cmd_crc cmd;
		struct mcp251xfd_hw_tx_obj_raw hw_tx_obj;
		__be16 crc;
	} crc;
} ____cacheline_aligned;

union mcp251xfd_write_reg_buf {
	struct __packed {
		struct mcp251xfd_buf_cmd cmd;
		u8 data[4];
	} nocrc;
	struct __packed {
		struct mcp251xfd_buf_cmd_crc cmd;
		u8 data[4];
		__be16 crc;
	} crc;
} ____cacheline_aligned;

struct mcp251xfd_tx_obj {
	struct spi_message msg;
	struct spi_transfer xfer[2];
	union mcp251xfd_tx_obj_load_buf buf;
};

struct mcp251xfd_tx_ring {
	unsigned int head;
	unsigned int tail;

	u16 base;
	u8 obj_num;
	u8 obj_size;

	struct mcp251xfd_tx_obj obj[MCP251XFD_TX_OBJ_NUM_MAX];
	union mcp251xfd_write_reg_buf rts_buf;
};

struct mcp251xfd_rx_ring {
	unsigned int head;
	unsigned int tail;

	u16 base;
	u8 nr;
	u8 fifo_nr;
	u8 obj_num;
	u8 obj_size;

	struct mcp251xfd_hw_rx_obj_canfd obj[];
};

struct __packed mcp251xfd_map_buf_nocrc {
	struct mcp251xfd_buf_cmd cmd;
	u8 data[256];
} ____cacheline_aligned;

struct __packed mcp251xfd_map_buf_crc {
	struct mcp251xfd_buf_cmd_crc cmd;
	u8 data[256 - 4];
	__be16 crc;
} ____cacheline_aligned;

struct mcp251xfd_ecc {
	u32 ecc_stat;
	int cnt;
};

struct mcp251xfd_regs_status {
	u32 intf;
};

enum mcp251xfd_model {
	MCP251XFD_MODEL_MCP2517FD = 0x2517,
	MCP251XFD_MODEL_MCP2518FD = 0x2518,
	MCP251XFD_MODEL_MCP251XFD = 0xffff, /* autodetect model */
};

struct mcp251xfd_devtype_data {
	enum mcp251xfd_model model;
	u32 quirks;
};

struct mcp251xfd_priv {
	struct can_priv can;
	struct can_rx_offload offload;
	struct net_device *ndev;

	struct regmap *map_reg; /* register access */
	struct regmap *map_rx; /* RX/TEF RAM access */

	struct regmap *map_nocrc;
	struct mcp251xfd_map_buf_nocrc *map_buf_nocrc_rx;
	struct mcp251xfd_map_buf_nocrc *map_buf_nocrc_tx;

	struct regmap *map_crc;
	struct mcp251xfd_map_buf_crc *map_buf_crc_rx;
	struct mcp251xfd_map_buf_crc *map_buf_crc_tx;

	struct spi_device *spi;
	u32 spi_max_speed_hz_orig;

	struct mcp251xfd_tef_ring tef;
	struct mcp251xfd_tx_ring tx[1];
	struct mcp251xfd_rx_ring *rx[1];

	u8 rx_ring_num;

	struct mcp251xfd_ecc ecc;
	struct mcp251xfd_regs_status regs_status;

	struct gpio_desc *rx_int;
	// struct clk *clk;
	// struct regulator *reg_vdd;
	// struct regulator *reg_xceiver;

	struct mcp251xfd_devtype_data devtype_data;
	struct can_berr_counter bec;
};

#define MCP251XFD_IS(_model)                                                   \
	static inline bool mcp251xfd_is_##_model(                              \
		const struct mcp251xfd_priv *priv)                             \
	{                                                                      \
		return priv->devtype_data.model ==                             \
		       MCP251XFD_MODEL_MCP##_model##FD;                        \
	}

MCP251XFD_IS(2517);
MCP251XFD_IS(2518);
MCP251XFD_IS(251X);

static inline u8 mcp251xfd_first_byte_set(u32 mask)
{
	return (mask & 0x0000ffff) ? ((mask & 0x000000ff) ? 0 : 1) :
				     ((mask & 0x00ff0000) ? 2 : 3);
}

static inline u8 mcp251xfd_last_byte_set(u32 mask)
{
	return (mask & 0xffff0000) ? ((mask & 0xff000000) ? 3 : 2) :
				     ((mask & 0x0000ff00) ? 1 : 0);
}

static inline __be16 mcp251xfd_cmd_reset(void)
{
	return cpu_to_be16(MCP251XFD_SPI_INSTRUCTION_RESET);
}

static inline void mcp251xfd_spi_cmd_read_nocrc(struct mcp251xfd_buf_cmd *cmd,
						u16 addr)
{
	cmd->cmd = cpu_to_be16(MCP251XFD_SPI_INSTRUCTION_READ | addr);
}

static inline void mcp251xfd_spi_cmd_write_nocrc(struct mcp251xfd_buf_cmd *cmd,
						 u16 addr)
{
	cmd->cmd = cpu_to_be16(MCP251XFD_SPI_INSTRUCTION_WRITE | addr);
}

static inline bool mcp251xfd_reg_in_ram(unsigned int reg)
{
	static const struct regmap_range range =
		regmap_reg_range(MCP251XFD_RAM_START,
				 MCP251XFD_RAM_START + MCP251XFD_RAM_SIZE - 4);

	return regmap_reg_in_range(reg, &range);
}

static inline void
__mcp251xfd_spi_cmd_crc_set_len(struct mcp251xfd_buf_cmd_crc *cmd, u16 len,
				bool in_ram)
{
	/* Number of u32 for RAM access, number of u8 otherwise. */
	if (in_ram)
		cmd->len = len >> 2;
	else
		cmd->len = len;
}

static inline void
mcp251xfd_spi_cmd_crc_set_len_in_ram(struct mcp251xfd_buf_cmd_crc *cmd, u16 len)
{
	__mcp251xfd_spi_cmd_crc_set_len(cmd, len, true);
}

static inline void
mcp251xfd_spi_cmd_crc_set_len_in_reg(struct mcp251xfd_buf_cmd_crc *cmd, u16 len)
{
	__mcp251xfd_spi_cmd_crc_set_len(cmd, len, false);
}

static inline void
mcp251xfd_spi_cmd_read_crc_set_addr(struct mcp251xfd_buf_cmd_crc *cmd, u16 addr)
{
	cmd->cmd = cpu_to_be16(MCP251XFD_SPI_INSTRUCTION_READ_CRC | addr);
}

static inline void mcp251xfd_spi_cmd_read_crc(struct mcp251xfd_buf_cmd_crc *cmd,
					      u16 addr, u16 len)
{
	mcp251xfd_spi_cmd_read_crc_set_addr(cmd, addr);
	__mcp251xfd_spi_cmd_crc_set_len(cmd, len, mcp251xfd_reg_in_ram(addr));
}

static inline void
mcp251xfd_spi_cmd_write_crc_set_addr(struct mcp251xfd_buf_cmd_crc *cmd,
				     u16 addr)
{
	cmd->cmd = cpu_to_be16(MCP251XFD_SPI_INSTRUCTION_WRITE_CRC | addr);
}

static inline void
mcp251xfd_spi_cmd_write_crc(struct mcp251xfd_buf_cmd_crc *cmd, u16 addr,
			    u16 len)
{
	mcp251xfd_spi_cmd_write_crc_set_addr(cmd, addr);
	__mcp251xfd_spi_cmd_crc_set_len(cmd, len, mcp251xfd_reg_in_ram(addr));
}

static inline u8 *
mcp251xfd_spi_cmd_write(const struct mcp251xfd_priv *priv,
			union mcp251xfd_write_reg_buf *write_reg_buf, u16 addr)
{
	u8 *data;

	if (priv->devtype_data.quirks & MCP251XFD_QUIRK_CRC_REG) {
		mcp251xfd_spi_cmd_write_crc_set_addr(&write_reg_buf->crc.cmd,
						     addr);
		data = write_reg_buf->crc.data;
	} else {
		mcp251xfd_spi_cmd_write_nocrc(&write_reg_buf->nocrc.cmd, addr);
		data = write_reg_buf->nocrc.data;
	}

	return data;
}

static inline u16 mcp251xfd_get_tef_obj_addr(u8 n)
{
	return MCP251XFD_RAM_START + sizeof(struct mcp251xfd_hw_tef_obj) * n;
}

static inline u16
mcp251xfd_get_tx_obj_addr(const struct mcp251xfd_tx_ring *ring, u8 n)
{
	return ring->base + ring->obj_size * n;
}

static inline u16
mcp251xfd_get_rx_obj_addr(const struct mcp251xfd_rx_ring *ring, u8 n)
{
	return ring->base + ring->obj_size * n;
}

static inline u8 mcp251xfd_get_tef_head(const struct mcp251xfd_priv *priv)
{
	return priv->tef.head & (priv->tx->obj_num - 1);
}

static inline u8 mcp251xfd_get_tef_tail(const struct mcp251xfd_priv *priv)
{
	return priv->tef.tail & (priv->tx->obj_num - 1);
}

static inline u8 mcp251xfd_get_tef_len(const struct mcp251xfd_priv *priv)
{
	return priv->tef.head - priv->tef.tail;
}

static inline u8 mcp251xfd_get_tef_linear_len(const struct mcp251xfd_priv *priv)
{
	u8 len;

	len = mcp251xfd_get_tef_len(priv);

	return min_t(u8, len, priv->tx->obj_num - mcp251xfd_get_tef_tail(priv));
}

static inline u8 mcp251xfd_get_tx_head(const struct mcp251xfd_tx_ring *ring)
{
	return ring->head & (ring->obj_num - 1);
}

static inline u8 mcp251xfd_get_tx_tail(const struct mcp251xfd_tx_ring *ring)
{
	return ring->tail & (ring->obj_num - 1);
}

static inline u8 mcp251xfd_get_tx_free(const struct mcp251xfd_tx_ring *ring)
{
	return ring->obj_num - (ring->head - ring->tail);
}

static inline int
mcp251xfd_get_tx_nr_by_addr(const struct mcp251xfd_tx_ring *tx_ring, u8 *nr,
			    u16 addr)
{
	if (addr < mcp251xfd_get_tx_obj_addr(tx_ring, 0) ||
	    addr >= mcp251xfd_get_tx_obj_addr(tx_ring, tx_ring->obj_num))
		return -ENOENT;

	*nr = (addr - mcp251xfd_get_tx_obj_addr(tx_ring, 0)) /
	      tx_ring->obj_size;

	return 0;
}

static inline u8 mcp251xfd_get_rx_head(const struct mcp251xfd_rx_ring *ring)
{
	return ring->head & (ring->obj_num - 1);
}

static inline u8 mcp251xfd_get_rx_tail(const struct mcp251xfd_rx_ring *ring)
{
	return ring->tail & (ring->obj_num - 1);
}

static inline u8 mcp251xfd_get_rx_len(const struct mcp251xfd_rx_ring *ring)
{
	return ring->head - ring->tail;
}

static inline u8
mcp251xfd_get_rx_linear_len(const struct mcp251xfd_rx_ring *ring)
{
	u8 len;

	len = mcp251xfd_get_rx_len(ring);

	return min_t(u8, len, ring->obj_num - mcp251xfd_get_rx_tail(ring));
}

#define mcp251xfd_for_each_tx_obj(ring, _obj, n)                               \
	for ((n) = 0, (_obj) = &(ring)->obj[(n)]; (n) < (ring)->obj_num;       \
	     (n)++, (_obj) = &(ring)->obj[(n)])

#define mcp251xfd_for_each_rx_ring(priv, ring, n)                              \
	for ((n) = 0, (ring) = *((priv)->rx + (n)); (n) < (priv)->rx_ring_num; \
	     (n)++, (ring) = *((priv)->rx + (n)))

int mcp251xfd_regmap_init(struct mcp251xfd_priv *priv);
u16 mcp251xfd_crc16_compute2(const void *cmd, size_t cmd_size, const void *data,
			     size_t data_size);
u16 mcp251xfd_crc16_compute(const void *data, size_t data_size);

#endif
