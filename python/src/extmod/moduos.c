/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Josef Gajdusek
 * Copyright (c) 2016 Damien P. George
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

#include <string.h>

#include "py/objstr.h"
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "extmod/misc.h"
#include "extmod/vfs.h"
#include "extmod/vfs_spiffs.h"
#include "py/mpconfig.h"

extern const mp_obj_type_t mp_fat_vfs_type;

STATIC const qstr os_uname_info_fields[] = {
    MP_QSTR_sysname, MP_QSTR_nodename,
    MP_QSTR_release
};
STATIC const MP_DEFINE_STR_OBJ(os_uname_info_sysname_obj, "Numworks");
STATIC const MP_DEFINE_STR_OBJ(os_uname_info_nodename_obj, "Numworks");
STATIC const MP_DEFINE_STR_OBJ(os_uname_info_release_obj, MICROPY_VERSION_STRING);

STATIC MP_DEFINE_ATTRTUPLE(
    os_uname_info_obj,
    os_uname_info_fields,
    5,
    (mp_obj_t)&os_uname_info_sysname_obj,
    (mp_obj_t)&os_uname_info_nodename_obj,
    (mp_obj_t)&os_uname_info_release_obj
);

STATIC mp_obj_t os_uname(void) {
    return (mp_obj_t)&os_uname_info_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(os_uname_obj, os_uname);

extern unsigned int _ZN3Ion6randomEv();

STATIC mp_obj_t os_urandom(mp_obj_t num) {
    mp_int_t n = mp_obj_get_int(num);
    vstr_t vstr;
    vstr_init_len(&vstr, n);
    uint32_t r = 0;
    for (int i = 0; i < n; i++) {
        if ((i & 3) == 0) {
            r = _ZN3Ion6randomEv(); // returns 32-bit hardware random number
        }
        vstr.buf[i] = r;
        r >>= 8;
    }
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(os_urandom_obj, os_urandom);

#if MICROPY_PY_OS_DUPTERM
STATIC mp_obj_t os_dupterm_notify(mp_obj_t obj_in) {
    (void)obj_in;
    for (;;) {
        int c = mp_uos_dupterm_rx_chr();
        if (c < 0) {
            break;
        }
        ringbuf_put(&stdin_ringbuf, c);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(os_dupterm_notify_obj, os_dupterm_notify);
#endif

STATIC const mp_rom_map_elem_t os_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_uos) },
    { MP_ROM_QSTR(MP_QSTR_uname), MP_ROM_PTR(&os_uname_obj) },
    { MP_ROM_QSTR(MP_QSTR_urandom), MP_ROM_PTR(&os_urandom_obj) },
    #if MICROPY_PY_OS_DUPTERM
    { MP_ROM_QSTR(MP_QSTR_dupterm), MP_ROM_PTR(&mp_uos_dupterm_obj) },
    { MP_ROM_QSTR(MP_QSTR_dupterm_notify), MP_ROM_PTR(&os_dupterm_notify_obj) },
    #endif
    #if MICROPY_VFS
    { MP_ROM_QSTR(MP_QSTR_ilistdir), MP_ROM_PTR(&mp_vfs_ilistdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_listdir), MP_ROM_PTR(&mp_vfs_listdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_mkdir), MP_ROM_PTR(&mp_vfs_mkdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_rmdir), MP_ROM_PTR(&mp_vfs_rmdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_chdir), MP_ROM_PTR(&mp_vfs_chdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_getcwd), MP_ROM_PTR(&mp_vfs_getcwd_obj) },
    { MP_ROM_QSTR(MP_QSTR_remove), MP_ROM_PTR(&mp_vfs_remove_obj) },
    { MP_ROM_QSTR(MP_QSTR_rename), MP_ROM_PTR(&mp_vfs_rename_obj) },
    { MP_ROM_QSTR(MP_QSTR_stat), MP_ROM_PTR(&mp_vfs_stat_obj) },
    { MP_ROM_QSTR(MP_QSTR_statvfs), MP_ROM_PTR(&mp_vfs_statvfs_obj) },
    #if MICROPY_VFS_FAT
    { MP_ROM_QSTR(MP_QSTR_VfsFat), MP_ROM_PTR(&mp_fat_vfs_type) },
    #endif
    #if MICROPY_VFS_LFS1
    { MP_ROM_QSTR(MP_QSTR_VfsLfs1), MP_ROM_PTR(&mp_type_vfs_lfs1) },
    #endif
    #if MICROPY_VFS_LFS2
    { MP_ROM_QSTR(MP_QSTR_VfsLfs2), MP_ROM_PTR(&mp_type_vfs_lfs) },
    #endif
    #endif
};

STATIC MP_DEFINE_CONST_DICT(os_module_globals, os_module_globals_table);

const mp_obj_module_t uos_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&os_module_globals,
};
