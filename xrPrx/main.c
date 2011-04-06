/*
 * This file is part of xReader.
 *
 * Copyright (C) 2008 hrimfaxi (outmatch@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <psppower.h>
#include <pspinit.h>
#include <string.h>
#include "config.h"
#include "xrPrx.h"

typedef unsigned short bool;

PSP_MODULE_INFO("xrPrx", 0x1007, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

PspDebugErrorHandler curr_handler;
PspDebugRegBlock *exception_regs;

void _pspDebugExceptionHandler(void);
int sceKernelRegisterDefaultExceptionHandler(void *func);
int sceKernelRegisterDefaultExceptionHandler371(void *func);

int xrKernelInitApitype(void)
{
	u32 k1;
	int apitype;

	k1 = pspSdkSetK1(0);
	apitype = sceKernelInitApitype();
	pspSdkSetK1(k1);

	return apitype;
}

/* Entry point */
int module_start(SceSize args, void *argp)
{
	int ret = 0;

#ifndef _DEBUG
	if (args != 8)
		return -1;
	curr_handler = (PspDebugErrorHandler) ((int *) argp)[0];
	exception_regs = (PspDebugRegBlock *) ((int *) argp)[1];
	if (!curr_handler || !exception_regs)
		return -1;

	if (sceKernelDevkitVersion() < 0x03070110)
		ret = sceKernelRegisterDefaultExceptionHandler((void *)
													   _pspDebugExceptionHandler);
	else
		ret = sceKernelRegisterDefaultExceptionHandler371((void *)
														  _pspDebugExceptionHandler);
#endif

	return ret;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}
