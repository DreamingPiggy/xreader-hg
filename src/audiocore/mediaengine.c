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
#include <psputility.h>
#include <kubridge.h>
#include "config.h"
#include "mediaengine.h"
#include "pspvaudio.h"
#include "common/datatype.h"
#include "scene.h"
#include "strsafe.h"
#include "dbg.h"
#ifdef DMALLOC
#include "dmalloc.h"
#endif

#ifdef ENABLE_MUSIC

static bool b_vaudio_prx_loaded = false;
static bool b_avcodec_prx_loaded = false;
static bool b_at3p_prx_loaded = false;
static bool b_cooleye_bridge_prx_loaded = false;
static bool b_asfparser_prx_loaded = false;

static SceUID g_modid = -1;
static SceUID g_modid2 = -1;

int load_me_prx(int mode)
{
	int ret;
	unsigned long prev;

	if (mode & VAUDIO) {
		prev = *((unsigned long *) 0x08fc1000);
		ret = sceUtilityLoadModule(0x0305);

		if (prev != *((unsigned long *) 0x08fc1000)) {
			fprintf(stderr, "vaudio: 0x08fc1000 changed\n");
		}

		*((unsigned long *) 0x08fc1000) = prev;

		if (ret == 0)
			b_vaudio_prx_loaded = true;
	}

	if (mode & AVCODEC) {
		prev = *((unsigned long *) 0x08fc1000);
		ret = sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);

		if (prev != *((unsigned long *) 0x08fc1000)) {
			fprintf(stderr, "avcodec: 0x08fc1000 changed\n");
		}

		*((unsigned long *) 0x08fc1000) = prev;

		if (ret == 0)
			b_avcodec_prx_loaded = true;
	}

	if (mode & ATRAC3PLUS) {
		prev = *((unsigned long *) 0x08fc1000);
		ret = sceUtilityLoadModule(0x0302);

		if (prev != *((unsigned long *) 0x08fc1000)) {
			fprintf(stderr, "atrac3p: 0x08fc1000 changed\n");
		}

		*((unsigned long *) 0x08fc1000) = prev;

		if (ret == 0)
			b_at3p_prx_loaded = true;
	}

	if (mode & COOLEYEBRIDGE) {
		char path[PATH_MAX];
		int status;
		unsigned long prev;

		SPRINTF_S(path, "%scooleyesBridge.prx", scene_appdir());
		g_modid = kuKernelLoadModule(path, 0, NULL);

		if (g_modid >= 0) {
			prev = *((unsigned long *) 0x08fc1000);
			ret = sceKernelStartModule(g_modid, 0, 0, &status, NULL);

			if (prev != *((unsigned long *) 0x08fc1000)) {
				fprintf(stderr, "cooleye_bridge: 0x08fc1000 changed\n");
			}

			*((unsigned long *) 0x08fc1000) = prev;

			if (ret >= 0)
				b_cooleye_bridge_prx_loaded = true;
		}
	}

	if (mode & ASFPARSER) {
		int status;
		unsigned long prev;

		g_modid2 = kuKernelLoadModule("flash0:/kd/libasfparser.prx", 0, NULL);

		if (g_modid2 >= 0) {
			prev = *((unsigned long *) 0x08fc1000);
			ret = sceKernelStartModule(g_modid2, 0, 0, &status, NULL);

			if (prev != *((unsigned long *) 0x08fc1000)) {
				fprintf(stderr, "asfparser: 0x08fc1000 changed\n");
			}

			*((unsigned long *) 0x08fc1000) = prev;

			if (ret >= 0)
				b_asfparser_prx_loaded = true;
		}
	}

	return 0;
}

int unload_prx(void)
{
	if (b_asfparser_prx_loaded) {
		int result;

		sceKernelStopModule(g_modid2, 0, NULL, &result, NULL);
		sceKernelUnloadModule(g_modid2);
		g_modid2 = -1;
		b_asfparser_prx_loaded = false;
	}

	if (b_cooleye_bridge_prx_loaded) {
		int result;

		sceKernelStopModule(g_modid, 0, NULL, &result, NULL);
		sceKernelUnloadModule(g_modid);
		g_modid = -1;
		b_cooleye_bridge_prx_loaded = false;
	}

	if (b_at3p_prx_loaded) {
		sceUtilityUnloadModule(0x0302);
		b_at3p_prx_loaded = false;
	}

	if (b_avcodec_prx_loaded) {
		sceUtilityUnloadAvModule(PSP_AV_MODULE_AVCODEC);
		b_avcodec_prx_loaded = false;
	}

	if (b_vaudio_prx_loaded) {
		sceUtilityUnloadModule(0x0305);
		b_vaudio_prx_loaded = false;
	}

	return 0;
}

#endif
