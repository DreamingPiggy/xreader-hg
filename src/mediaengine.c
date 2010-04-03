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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspaudio.h>
#include <pspaudiocodec.h>
#include <kubridge.h>
#include "config.h"
#include "xrhal.h"
#include "mediaengine.h"
#include "pspvaudio.h"
#include "common/datatype.h"

#ifdef ENABLE_MUSIC

static bool b_vaudio_prx_loaded   = false;
static bool b_avcodec_prx_loaded  = false;
static bool b_at3p_prx_loaded     = false;

static SceUID g_modid = -1;

int load_me_prx(int mode)
{
	if (mode & VAUDIO) {
		g_modid = kuKernelLoadModule("flash0:/kd/vaudio.prx", 0, NULL);
		xrKernelStartModule(g_modid, 0, NULL, 0, NULL);
		b_vaudio_prx_loaded = true;
	}

	if (mode & AVCODEC) {
		xrUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
		b_avcodec_prx_loaded = true;
	}

	if (mode & ATRAC3PLUS) {
		xrUtilityLoadAvModule(PSP_AV_MODULE_ATRAC3PLUS);
		b_at3p_prx_loaded = true;
	}

	return 0;
}

int unload_prx(void)
{
	if (b_vaudio_prx_loaded) {
		int result;

		sceKernelStopModule(g_modid, 0, NULL, &result, NULL); 
		sceKernelUnloadModule(g_modid);
		g_modid = -1;
		b_vaudio_prx_loaded = false;
	}

	if (b_avcodec_prx_loaded) {
		sceUtilityUnloadAvModule(PSP_AV_MODULE_AVCODEC);
		b_avcodec_prx_loaded = false;
	}

	if (b_at3p_prx_loaded) {
		sceUtilityUnloadAvModule(PSP_AV_MODULE_ATRAC3PLUS);
		b_at3p_prx_loaded = false;
	}

	return 0;
}

#endif
