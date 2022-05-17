/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Damien P. George
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "py/runtime.h"
#include "extmod/vfs.h"
#include "extmod/vfs_spiffs.h"

#if MICROPY_VFS && (MICROPY_VFS_SPIFFS)

enum { SPIFFS_MAKE_ARG_bdev, SPIFFS_MAKE_ARG_readsize, SPIFFS_MAKE_ARG_progsize, SPIFFS_MAKE_ARG_lookahead };

static const mp_arg_t spiffs_make_allowed_args[] = {
    { MP_QSTR_readsize, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 32} },
    { MP_QSTR_progsize, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 32} },
    { MP_QSTR_lookahead, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 32} },
};

#include "../../../ion/src/external/spiffs/spiffs.h"

spiffs global_filesystem;

#define SPIFFS_BUILD_VERSION (2)
#define SPIFFSx_MACRO(s) SPIFFS ## s
#define SPIFFSx_API(s) SPIFFS_ ## s
#define SPIFFSx_TYPE(s) spiffs_ ## s
#define MP_VFS_SPIFFSx(s) mp_vfs_spiffs_ ## s
#define MP_OBJ_VFS_SPIFFSx mp_obj_vfs_spiffs_t
#define MP_OBJ_VFS_SPIFFSx_FILE mp_obj_vfs_spiffs_file_t
#define MP_TYPE_VFS_SPIFFSx mp_type_vfs_spiffs
#define MP_TYPE_VFS_SPIFFSx_(s) mp_type_vfs_spiffs ## s

typedef struct _mp_obj_vfs_spiffs_t {
    mp_obj_base_t base;
    mp_vfs_blockdev_t blockdev;
    spiffs_config config;
    spiffs spiffs;
} mp_obj_vfs_spiffs_t;

typedef struct _mp_obj_vfs_spiffs_file_t {
    mp_obj_base_t base;
    mp_obj_vfs_spiffs_t *vfs;
    int file;
    uint8_t file_buffer[0];
} mp_obj_vfs_spiffs_file_t;

const char *mp_vfs_spiffs_make_path(mp_obj_vfs_spiffs_t *self, mp_obj_t path_in);
mp_obj_t mp_vfs_spiffs_file_open(mp_obj_t self_in, mp_obj_t path_in, mp_obj_t mode_in);

#include "extmod/vfs_spiffsx.c"
#include "extmod/vfs_spiffsx_file.c"

#endif // MICROPY_VFS && (MICROPY_VFS_SPIFFS)
