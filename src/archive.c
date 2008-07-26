#include <psptypes.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <gif_lib.h>
#include <jpeglib.h>
#include <bmplib.h>
#include <chm_lib.h>
#include <unzip.h>
#include <unrar.h>
#include "common/datatype.h"
#include "buffer.h"
#include "passwdmgr.h"
#include "dbg.h"
#include "bg.h"
#include "osk.h"
#include "archive.h"

static void extract_zip_file_into_buffer_with_password(buffer * buf,
													   const char *archname,
													   const char *archpath,
													   const char *password)
{
	unzFile unzf = unzOpen(archname);
	int ret;

	if (unzf == NULL)
		return;
	if (unzLocateFile(unzf, archpath, 0) != UNZ_OK
		|| unzOpenCurrentFilePassword(unzf, password) != UNZ_OK) {
		unzClose(unzf);
		return;
	}

	unz_file_info info;

	if (unzGetCurrentFileInfo(unzf, &info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK) {
		unzCloseCurrentFile(unzf);
		unzClose(unzf);
		return;
	}
	buffer_prepare_copy(buf, info.uncompressed_size);
	if (buf->ptr == NULL) {
		unzCloseCurrentFile(unzf);
		unzClose(unzf);
		return;
	}

	ret = unzReadCurrentFile(unzf, buf->ptr, info.uncompressed_size);
	buf->used = info.uncompressed_size;
	if (ret < 0) {
		free(buf->ptr);
		buf->ptr = NULL;
		buf->size = buf->used = 0;
		unzCloseCurrentFile(unzf);
		unzClose(unzf);
		return;
	}

	buf->used = info.uncompressed_size;
	unzCloseCurrentFile(unzf);
	unzClose(unzf);
}

static void extract_zip_file_into_buffer(buffer * buf, const char *archname,
										 const char *archpath)
{
	unzFile unzf = unzOpen(archname);

	if (unzf == NULL)
		return;
	if (unzLocateFile(unzf, archpath, 0) != UNZ_OK
		|| unzOpenCurrentFile(unzf) != UNZ_OK) {
		unzClose(unzf);
		return;
	}

	unz_file_info info;

	if (unzGetCurrentFileInfo(unzf, &info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK) {
		unzCloseCurrentFile(unzf);
		unzClose(unzf);
		return;
	}

	if (info.flag == 9) {
		unzCloseCurrentFile(unzf);
		unzClose(unzf);
		dbg_printf(d, "%s: crc error, wrong password?", __func__);
		// retry with loaded passwords
		if (get_password_count()) {
			int i, n;

			for (n = get_password_count(), i = 0; i < n; ++i) {
				buffer *b = get_password(i);

				if (b == NULL || b->ptr == NULL) {
					continue;
				}
				dbg_printf(d, "%s: trying list password: %s", __func__, b->ptr);
				extract_zip_file_into_buffer_with_password(buf, archname,
														   archpath, b->ptr);
				if (buf->ptr != NULL) {
					// ok
					add_password(b->ptr);
					return;
				}
			}
		}
		// if all passwords failed, ask user input password
		char pass[128];

		if (get_osk_input_password(pass, 128) == 1 && strcmp(pass, "") != 0) {
			dbg_printf(d, "%s: input %s", __func__, pass);
			extract_zip_file_into_buffer_with_password(buf, archname,
													   archpath, pass);
			if (buf->ptr != NULL) {
				// ok
				add_password(pass);
			}
		}
#ifdef ENABLE_BG
		bg_display();
		disp_flip();
		bg_display();
#endif
		disp_duptocache();
		disp_waitv();
		return;
	}

	buffer_prepare_copy(buf, info.uncompressed_size);
	if (buf->ptr == NULL) {
		unzCloseCurrentFile(unzf);
		unzClose(unzf);
		return;
	}
	int ret = unzReadCurrentFile(unzf, buf->ptr, info.uncompressed_size);

	if (ret < 0) {
		free(buf->ptr);
		buf->ptr = NULL;
		buf->size = buf->used = 0;
		unzCloseCurrentFile(unzf);
		unzClose(unzf);
		return;
	}
	buf->used = info.uncompressed_size;
	unzCloseCurrentFile(unzf);
	unzClose(unzf);
}

static int rarcbproc(UINT msg, LONG UserData, LONG P1, LONG P2)
{
	if (msg == UCM_PROCESSDATA) {
		buffer *buf = (buffer *) UserData;

		if (buffer_append_memory(buf, (void *) P1, P2) == -1) {
			return -1;
		}
	}
	return 0;
}

static void extract_rar_file_into_buffer_with_password(buffer * buf,
													   const char *archname,
													   const char *archpath,
													   char *password)
{
	struct RAROpenArchiveData arcdata;
	int code = 0;

	arcdata.ArcName = (char *) archname;
	arcdata.OpenMode = RAR_OM_EXTRACT;
	arcdata.CmtBuf = NULL;
	arcdata.CmtBufSize = 0;
	HANDLE hrar = RAROpenArchive(&arcdata);

	if (hrar == NULL)
		return;
	RARSetCallback(hrar, rarcbproc, (LONG) buf);
	RARSetPassword(hrar, password);
	do {
		struct RARHeaderData header;

		if (RARReadHeader(hrar, &header) != 0)
			break;
		if (stricmp(header.FileName, archpath) == 0) {
			dbg_printf(d, "%s: setting password %s", __func__, password);
			code = RARProcessFile(hrar, RAR_TEST, NULL, NULL);
			break;
		}
	} while (RARProcessFile(hrar, RAR_SKIP, NULL, NULL) == 0);
	RARCloseArchive(hrar);

	if (code != 0) {
		free(buf->ptr);
		buf->ptr = NULL;
		buf->size = buf->used = 0;
	}
}

static bool test_rar_file_password(buffer * buf,
								   const char *archname, const char *archpath)
{
	dbg_printf(d, "%s: bad data, wrong password?", __func__);
	// retry with loaded passwords
	if (get_password_count()) {
		int i, n;

		for (n = get_password_count(), i = 0; i < n; ++i) {
			buffer *b = get_password(i);

			if (b == NULL || b->ptr == NULL) {
				continue;
			}
			dbg_printf(d, "%s: trying list password: %s", __func__, b->ptr);
			extract_rar_file_into_buffer_with_password(buf, archname,
													   archpath, b->ptr);
			if (buf->ptr != NULL) {
				// ok
				add_password(b->ptr);
				return true;
			}
		}
	}
	// if all passwords failed, ask user input password
	char pass[128];
	bool result = false;

	if (get_osk_input_password(pass, 128) == 1 && strcmp(pass, "") != 0) {
		dbg_printf(d, "%s: input %s", __func__, pass);
		extract_rar_file_into_buffer_with_password(buf, archname,
												   archpath, pass);
		if (buf->ptr != NULL) {
			// ok
			add_password(pass);
			result = true;
		}
	}
#ifdef ENABLE_BG
	bg_display();
	disp_flip();
	bg_display();
#endif
	disp_duptocache();
	disp_waitv();
	return result;
}

static void extract_rar_file_into_buffer(buffer * buf, const char *archname,
										 const char *archpath)
{
	struct RAROpenArchiveData arcdata;
	int code = 0;
	int ret;

	arcdata.ArcName = (char *) archname;
	arcdata.OpenMode = RAR_OM_EXTRACT;
	arcdata.CmtBuf = NULL;
	arcdata.CmtBufSize = 0;
	HANDLE hrar = RAROpenArchive(&arcdata);

	if (hrar == NULL)
		return;
	RARSetCallback(hrar, rarcbproc, (LONG) buf);
	do {
		struct RARHeaderData header;

		if ((ret = RARReadHeader(hrar, &header)) != 0) {
			if (ret != ERAR_UNKNOWN && ret != ERAR_BAD_DATA)
				break;
			RARCloseArchive(hrar);
			test_rar_file_password(buf, archname, archpath);
			return;
		}
		if (stricmp(header.FileName, archpath) == 0) {
			code = RARProcessFile(hrar, RAR_TEST, NULL, NULL);
			break;
		}
	} while (RARProcessFile(hrar, RAR_SKIP, NULL, NULL) == 0);
	RARCloseArchive(hrar);

	if (code == 22) {
		test_rar_file_password(buf, archname, archpath);
		return;
	}

	if (code != 0) {
		free(buf->ptr);
		buf->ptr = NULL;
		buf->size = buf->used = 0;
	}
}

static void extract_chm_file_into_buffer(buffer * buf, const char *archname,
										 const char *archpath)
{
	struct chmFile *chm = chm_open(archname);
	struct chmUnitInfo ui;

	if (chm == NULL) {
		return;
	}

	if (chm_resolve_object(chm, archpath, &ui) != CHM_RESOLVE_SUCCESS) {
		chm_close(chm);
		return;
	}

	buffer_prepare_copy(buf, ui.length);

	if (buf->ptr == NULL) {
		chm_close(chm);
		return;
	}

	buf->used = chm_retrieve_object(chm, &ui, (byte *) buf->ptr, 0, ui.length);
	chm_close(chm);
}

/**
 * 解压档案中文件到内存缓冲
 * @param buffer 缓冲指针指针
 * @param archname 档案文件路径
 * @param archpath 解压文件路径
 * @param filetype 档案文件类型
 * @note 如果解压失败, *buf = NULL
 */
extern void extract_archive_file_into_buffer(buffer ** buf,
											 const char *archname,
											 const char *archpath,
											 t_fs_filetype filetype)
{
	if (buf == NULL)
		return;

	if (archname == NULL || archpath == NULL) {
		*buf = NULL;
		return;
	}

	buffer *b;

	b = buffer_init();

	switch (filetype) {
		case fs_filetype_zip:
			extract_zip_file_into_buffer(b, archname, archpath);
			break;
		case fs_filetype_rar:
			extract_rar_file_into_buffer(b, archname, archpath);
			break;
		case fs_filetype_chm:
			extract_chm_file_into_buffer(b, archname, archpath);
			break;
		default:
			*buf = NULL;
			return;
			break;
	}

	if (b != NULL && b->ptr != NULL)
		*buf = b;
	else {
		*buf = NULL;
		buffer_free(b);
	}
}

static int imagerarcbproc(UINT msg, LONG UserData, LONG P1, LONG P2)
{
	if (msg == UCM_PROCESSDATA) {
		p_image_rar irar = (p_image_rar) UserData;

		if (P2 > irar->size - irar->idx) {
			memcpy(&irar->buf[irar->idx], (void *) P1, irar->size - irar->idx);
			return -1;
		}
		memcpy(&irar->buf[irar->idx], (void *) P1, P2);
		irar->idx += P2;
	}
	return 0;
}

static void extract_rar_file_into_image_with_password(t_image_rar * image,
													  const char *archname,
													  const char *archpath,
													  char *password)
{
	struct RAROpenArchiveData arcdata;
	int code = 0;

	arcdata.ArcName = (char *) archname;
	arcdata.OpenMode = RAR_OM_EXTRACT;
	arcdata.CmtBuf = NULL;
	arcdata.CmtBufSize = 0;
	HANDLE hrar = RAROpenArchive(&arcdata);

	if (hrar == NULL)
		return;
	RARSetCallback(hrar, imagerarcbproc, (LONG) image);
	RARSetPassword(hrar, password);
	do {
		struct RARHeaderData header;

		if (RARReadHeader(hrar, &header) != 0)
			break;
		if (stricmp(header.FileName, archpath) == 0) {
			image->size = header.UnpSize;
			image->idx = 0;
			if ((image->buf = calloc(1, image->size)) == NULL) {
				RARCloseArchive(hrar);
				if (image->buf != NULL) {
					free(image->buf);
					image->buf = NULL;
					image->size = image->idx = 0;
				}
				return;
			}
			code = RARProcessFile(hrar, RAR_TEST, NULL, NULL);
			break;
		}
	} while (RARProcessFile(hrar, RAR_SKIP, NULL, NULL) == 0);
	RARCloseArchive(hrar);

	if (code != 0) {
		free(image->buf);
		image->buf = NULL;
		image->size = image->idx = 0;
	}
}

static bool test_rar_image_password(t_image_rar * image,
									const char *archname, const char *archpath)
{
	dbg_printf(d, "%s: bad data, wrong password?", __func__);
	// retry with loaded passwords
	if (get_password_count()) {
		int i, n;

		for (n = get_password_count(), i = 0; i < n; ++i) {
			buffer *b = get_password(i);

			if (b == NULL || b->ptr == NULL) {
				continue;
			}
			dbg_printf(d, "%s: trying list password: %s", __func__, b->ptr);
			extract_rar_file_into_image_with_password(image, archname,
													  archpath, b->ptr);
			if (image->buf != NULL) {
				// ok
				add_password(b->ptr);
				return true;
			}
		}
	}
	// if all passwords failed, ask user input password
	char pass[128];
	bool result = false;

	if (get_osk_input_password(pass, 128) == 1 && strcmp(pass, "") != 0) {
		dbg_printf(d, "%s: input %s", __func__, pass);
		extract_rar_file_into_image_with_password(image, archname,
												  archpath, pass);
		if (image->buf != NULL) {
			// ok
			add_password(pass);
			result = true;
		}
	}
#ifdef ENABLE_BG
	bg_display();
	disp_flip();
	bg_display();
#endif
	disp_duptocache();
	disp_waitv();
	return result;
}

/**
 * 解压档案中文件到内存缓冲
 * @param buffer 缓冲指针指针
 * @param archname 档案文件路径
 * @param archpath 解压文件路径
 * @param filetype 档案文件类型
 * @note 如果解压失败, *buf = NULL
 */
extern void extract_rar_file_into_image(t_image_rar * image,
										const char *archname,
										const char *archpath)
{
	struct RAROpenArchiveData arcdata;
	int code = 0, ret;

	arcdata.ArcName = (char *) archname;
	arcdata.OpenMode = RAR_OM_EXTRACT;
	arcdata.CmtBuf = NULL;
	arcdata.CmtBufSize = 0;
	HANDLE hrar = RAROpenArchive(&arcdata);

	if (hrar == NULL)
		return;
	RARSetCallback(hrar, imagerarcbproc, (LONG) image);
	do {
		struct RARHeaderData header;

		if ((ret = RARReadHeader(hrar, &header)) != 0) {
			if (ret != ERAR_UNKNOWN && ret != ERAR_BAD_DATA)
				break;
			RARCloseArchive(hrar);
			test_rar_image_password(image, archname, archpath);
			return;
		}
		if (stricmp(header.FileName, archpath) == 0) {
			image->size = header.UnpSize;
			image->idx = 0;
			if ((image->buf = calloc(1, image->size)) == NULL) {
				RARCloseArchive(hrar);
				if (image->buf != NULL) {
					free(image->buf);
					image->buf = NULL;
					image->size = image->idx = 0;
				}
				return;
			}
			code = RARProcessFile(hrar, RAR_TEST, NULL, NULL);
			break;
		}
	} while (RARProcessFile(hrar, RAR_SKIP, NULL, NULL) == 0);
	RARCloseArchive(hrar);

	if (code == 22) {
		test_rar_image_password(image, archname, archpath);
		return;
	}

	if (code != 0) {
		free(image->buf);
		image->buf = NULL;
		image->size = image->idx = 0;
	}
}

extern HANDLE reopen_rar_with_passwords(struct RAROpenArchiveData *arcdata)
{
	struct RARHeaderData header;

	HANDLE hrar = 0;

	hrar = RAROpenArchive(arcdata);
	if (hrar == 0)
		return hrar;

	int ret, n, i;
	buffer *b;

	for (n = get_password_count(), i = 0; i < n; ++i) {
		b = get_password(i);

		if (b == NULL || b->ptr == NULL) {
			continue;
		}
		dbg_printf(d, "%s: trying list password: %s", __func__, b->ptr);
		RARSetPassword(hrar, b->ptr);
		if ((ret = RARReadHeader(hrar, &header)) == 0)
			break;
		else {
			RARCloseArchive(hrar);
			hrar = RAROpenArchive(arcdata);
		}
	}

	if (ret == 0) {
		RARCloseArchive(hrar);
		hrar = RAROpenArchive(arcdata);
		RARSetPassword(hrar, b->ptr);
		return hrar;
	}
	// if all passwords failed, ask user input password
	char pass[128];

	if (get_osk_input_password(pass, 128) == 1 && strcmp(pass, "") != 0) {
		dbg_printf(d, "%s: input %s", __func__, pass);
		RARSetPassword(hrar, pass);
		if ((ret = RARReadHeader(hrar, &header)) == 0) {
			// ok
			add_password(pass);
			RARCloseArchive(hrar);
			hrar = RAROpenArchive(arcdata);
			RARSetPassword(hrar, pass);
			return hrar;
		}
#ifdef ENABLE_BG
		bg_display();
		disp_flip();
		bg_display();
#endif
		disp_duptocache();
		disp_waitv();
	}

	RARCloseArchive(hrar);
	hrar = 0;

	return hrar;
}
