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

#include <pspsdk.h>
#include <string.h>
#include "config.h"
#include "systemctrl.h"
#include "musicdrv.h"
#include "musicmgr.h"

PSP_MODULE_INFO("dummy", 0x0000, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

static int dummy_set_opt(const char *unused, const char *values)
{
	return 0;
}

static int dummy_load(const char *spath, const char *lpath)
{
	return 0;
}

static int dummy_end(void)
{
	return 0;
}

static int dummy_suspend(void)
{
	return 0;
}

static int dummy_resume(const char *spath, const char *lpath)
{
	return 0;
}

static int dummy_get_info(struct music_info *info)
{
	return 0;
}

static int dummy_probe(const char *spath)
{
	return 0;
}

static struct music_ops dummy_ops = {
	.name = "dummy",
	.set_opt = dummy_set_opt,
	.load = dummy_load,
	.end = dummy_end,
	.suspend = dummy_suspend,
	.resume = dummy_resume,
	.get_info = dummy_get_info,
	.probe = dummy_probe,
	.next = NULL,
};

/* Entry point */
int module_start(SceSize args, void *argp)
{
	register_musicdrv(&dummy_ops);

	return 0;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}
