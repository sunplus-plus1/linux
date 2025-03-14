/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Sunplus Technology Co., Ltd.
 *       All rights reserved.
 */
#ifndef __MACH_MISC_H
#define __MACH_MISC_H

#include <linux/io.h>
#include <mach/io_map.h>

void sc_smp_cpu_die(unsigned int cpu);

/* Latch and print IC uptime */
static inline void sp_prn_uptime(void)
{
	unsigned int base = VA_IOB_ADDR(0);

	writel(0x1234, (void __iomem *)base + 0x30e8);	/* stcl_2 in AV1_STC */

	pr_info("av1_stc: 0x%04x%04x\n",
		readl((void __iomem *)base + 0x30e4),	/* stcl_1 */
		readl((void __iomem *)base + 0x30e0));	/* stcl_0 */
}

#endif /* __MACH_MISC_H */
