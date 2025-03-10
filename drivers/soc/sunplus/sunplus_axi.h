/* SPDX-License-Identifier: GPL-2.0-or-later*/
#ifndef __SP_IOP_H__
#define __SP_IOP_H__
#ifdef CONFIG_SOC_SP7021
#include <mach/io_map.h>
#endif
enum IOP_Status_e {
	IOP_SUCCESS,                /* successful */
	IOP_ERR_IOP_BUSY,           /* IOP is busy */
};

#endif /* __SP_IOP_H__ */
