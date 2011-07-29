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
#include <psputilsforkernel.h>
#include <psppower.h>
#include <pspinit.h>
#include <string.h>
#include "config.h"
#include "systemctrl.h"
#include "xrPrx.h"

typedef unsigned short bool;

PSP_MODULE_INFO("xrPrx", 0x1007, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

PspDebugErrorHandler curr_handler;
PspDebugRegBlock *exception_regs;

void _pspDebugExceptionHandler(void);
int sceKernelRegisterDefaultExceptionHandler(void *func);

int xrKernelInitApitype(void)
{
	u32 k1;
	int apitype;

	k1 = pspSdkSetK1(0);
	apitype = sceKernelInitApitype();
	pspSdkSetK1(k1);

	return apitype;
}

SceUID xrIoOpen(const char *file, int flags, SceMode mode)
{
	SceUID fd;
	u32 k1, level;

	k1 = pspSdkSetK1(0);
	level = sctrlKernelSetUserLevel(8);
	fd = sceIoOpen(file, flags, mode);
	sctrlKernelSetUserLevel(level);
	pspSdkSetK1(k1);

	return fd;
}

SceOff xrIoLseek(SceUID fd, SceOff offset, int whence)
{
	SceOff ret;
	u32 k1, level;

	k1 = pspSdkSetK1(0);
	level = sctrlKernelSetUserLevel(8);
	ret = sceIoLseek(fd, offset, whence);
	sctrlKernelSetUserLevel(level);
	pspSdkSetK1(k1);

	return ret;
}

int xrIoRead(SceUID fd, void *data, SceSize size)
{
	int ret;
	u32 k1, level;

	k1 = pspSdkSetK1(0);
	level = sctrlKernelSetUserLevel(8);
	ret = sceIoRead(fd, data, size);
	sctrlKernelSetUserLevel(level);
	pspSdkSetK1(k1);

	return ret;
}

int xrIoClose(SceUID fd)
{
	SceUID ret;
	u32 k1, level;

	k1 = pspSdkSetK1(0);
	level = sctrlKernelSetUserLevel(8);
	ret = sceIoClose(fd);
	sctrlKernelSetUserLevel(level);
	pspSdkSetK1(k1);

	return ret;
}

void sync_cache(void)
{
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
}

/* Entry point */
int module_start(SceSize args, void *argp)
{
#ifndef _DEBUG
	int ret = 0;

	if (args != 8)
		return 0;
	curr_handler = (PspDebugErrorHandler) ((int *) argp)[0];
	exception_regs = (PspDebugRegBlock *) ((int *) argp)[1];
	if (!curr_handler || !exception_regs)
		return -1;

	ret = sceKernelRegisterDefaultExceptionHandler((void *) _pspDebugExceptionHandler);
#endif

	return 0;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}
