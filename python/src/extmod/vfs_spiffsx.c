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

#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/stream.h"
#include "py/binary.h"
#include "py/objarray.h"
#include "py/mperrno.h"
#include "extmod/vfs.h"

const char *MP_VFS_SPIFFSx(make_path)(MP_OBJ_VFS_SPIFFSx *self, mp_obj_t path_in) {
    const char *path = mp_obj_str_get_str(path_in);
    if (path[0] == '/') {
        return path + 1;
    }
    return path;
}

STATIC mp_obj_t MP_VFS_SPIFFSx(make_new)(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_val_t args[MP_ARRAY_SIZE(spiffs_make_allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(spiffs_make_allowed_args), spiffs_make_allowed_args, args);

    MP_OBJ_VFS_SPIFFSx *self = m_new0(MP_OBJ_VFS_SPIFFSx, 1);
    self->base.type = type;
    return MP_OBJ_FROM_PTR(self);
}

// Implementation of mp_vfs_spiffs_file_open is provided in vfs_spiffsx_file.c
STATIC MP_DEFINE_CONST_FUN_OBJ_3(MP_VFS_SPIFFSx(open_obj), MP_VFS_SPIFFSx(file_open));

typedef struct MP_VFS_SPIFFSx(_ilistdir_it_t) {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    bool is_str;
    MP_OBJ_VFS_SPIFFSx *vfs;
    SPIFFSx_TYPE(DIR) dir;
} MP_VFS_SPIFFSx(ilistdir_it_t);

STATIC mp_obj_t MP_VFS_SPIFFSx(ilistdir_it_iternext)(mp_obj_t self_in) {
    MP_VFS_SPIFFSx(ilistdir_it_t) *self = MP_OBJ_TO_PTR(self_in);

    struct SPIFFSx_TYPE(dirent) info;
    for (;;) {
        int ret = SPIFFSx_API(readdir)(&self->dir, &info);
        if (ret == 0) {
            SPIFFSx_API(closedir)(&self->dir);
            return MP_OBJ_STOP_ITERATION;
        }
        if (!(info.name[0] == '.' && (info.name[1] == '\0'
            || (info.name[1] == '.' && info.name[2] == '\0')))) {
            break;
        }
    }

    // make 4-tuple with info about this entry
    mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(4, NULL));
    if (self->is_str) {
        t->items[0] = mp_obj_new_str(info.name, strlen(info.name));
    } else {
        t->items[0] = mp_obj_new_bytes((const byte*)info.name, strlen(info.name));
    }
    t->items[1] = MP_OBJ_NEW_SMALL_INT(MP_S_IFREG); // SPIFFS doesn't have notions of folders.
    t->items[2] = MP_OBJ_NEW_SMALL_INT(0); // no inode number
    t->items[3] = MP_OBJ_NEW_SMALL_INT(info.size);

    return MP_OBJ_FROM_PTR(t);
}

STATIC mp_obj_t MP_VFS_SPIFFSx(ilistdir_func)(size_t n_args, const mp_obj_t *args) {
    MP_OBJ_VFS_SPIFFSx *self = MP_OBJ_TO_PTR(args[0]);
    bool is_str_type = true;
    const char *path;
    if (n_args == 2) {
        if (mp_obj_get_type(args[1]) == &mp_type_bytes) {
            is_str_type = false;
        }
        path = MP_VFS_SPIFFSx(make_path)(self, args[1]);
    } else {
        path = "/";
    }

    MP_VFS_SPIFFSx(ilistdir_it_t) *iter = m_new_obj(MP_VFS_SPIFFSx(ilistdir_it_t));
    iter->base.type = &mp_type_polymorph_iter;
    iter->iternext = MP_VFS_SPIFFSx(ilistdir_it_iternext);
    iter->is_str = is_str_type;
    iter->vfs = self;
    SPIFFSx_API(opendir)(&global_filesystem, path, &iter->dir);
    return MP_OBJ_FROM_PTR(iter);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(MP_VFS_SPIFFSx(ilistdir_obj), 1, 2, MP_VFS_SPIFFSx(ilistdir_func));

STATIC mp_obj_t MP_VFS_SPIFFSx(remove)(mp_obj_t self_in, mp_obj_t path_in) {
    MP_OBJ_VFS_SPIFFSx *self = MP_OBJ_TO_PTR(self_in);
    const char *path = MP_VFS_SPIFFSx(make_path)(self, path_in);
    int ret = SPIFFSx_API(remove)(&global_filesystem, path);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(MP_VFS_SPIFFSx(remove_obj), MP_VFS_SPIFFSx(remove));

STATIC mp_obj_t MP_VFS_SPIFFSx(rmdir)(mp_obj_t self_in, mp_obj_t path_in) {
    MP_OBJ_VFS_SPIFFSx *self = MP_OBJ_TO_PTR(self_in);
    const char *path = MP_VFS_SPIFFSx(make_path)(self, path_in);
    int ret = SPIFFSx_API(remove)(&global_filesystem, path);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(MP_VFS_SPIFFSx(rmdir_obj), MP_VFS_SPIFFSx(rmdir));

STATIC mp_obj_t MP_VFS_SPIFFSx(rename)(mp_obj_t self_in, mp_obj_t path_old_in, mp_obj_t path_new_in) {
    MP_OBJ_VFS_SPIFFSx *self = MP_OBJ_TO_PTR(self_in);
    const char *path_old = MP_VFS_SPIFFSx(make_path)(self, path_old_in);
    vstr_t path_new;
    vstr_init(&path_new, 0);
    vstr_add_str(&path_new, mp_obj_str_get_str(path_new_in));
    int ret = SPIFFSx_API(rename)(&global_filesystem, path_old, vstr_null_terminated_str(&path_new));
    vstr_clear(&path_new);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(MP_VFS_SPIFFSx(rename_obj), MP_VFS_SPIFFSx(rename));

STATIC mp_obj_t MP_VFS_SPIFFSx(mkdir)(mp_obj_t self_in, mp_obj_t path_o) {
    mp_raise_NotImplementedError("Directories are not supported.");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(MP_VFS_SPIFFSx(mkdir_obj), MP_VFS_SPIFFSx(mkdir));

STATIC mp_obj_t MP_VFS_SPIFFSx(chdir)(mp_obj_t self_in, mp_obj_t path_in) {
    mp_raise_NotImplementedError("Directories are not supported.");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(MP_VFS_SPIFFSx(chdir_obj), MP_VFS_SPIFFSx(chdir));

STATIC mp_obj_t MP_VFS_SPIFFSx(getcwd)(mp_obj_t self_in) {
    return MP_OBJ_NEW_QSTR(MP_QSTR__slash_);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(MP_VFS_SPIFFSx(getcwd_obj), MP_VFS_SPIFFSx(getcwd));

STATIC mp_obj_t MP_VFS_SPIFFSx(stat)(mp_obj_t self_in, mp_obj_t path_in) {
    MP_OBJ_VFS_SPIFFSx *self = MP_OBJ_TO_PTR(self_in);
    const char *path = mp_obj_str_get_str(path_in);
    SPIFFSx_TYPE(stat) info;
    int ret = SPIFFSx_API(stat)(&global_filesystem, path, &info);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }

    mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(10, NULL));
    t->items[0] = MP_OBJ_NEW_SMALL_INT(MP_S_IFREG); // st_mode
    t->items[1] = MP_OBJ_NEW_SMALL_INT(0); // st_ino
    t->items[2] = MP_OBJ_NEW_SMALL_INT(0); // st_dev
    t->items[3] = MP_OBJ_NEW_SMALL_INT(0); // st_nlink
    t->items[4] = MP_OBJ_NEW_SMALL_INT(0); // st_uid
    t->items[5] = MP_OBJ_NEW_SMALL_INT(0); // st_gid
    t->items[6] = mp_obj_new_int_from_uint(info.size); // st_size
    t->items[7] = MP_OBJ_NEW_SMALL_INT(0); // st_atime
    t->items[8] = MP_OBJ_NEW_SMALL_INT(0); // st_mtime
    t->items[9] = MP_OBJ_NEW_SMALL_INT(0); // st_ctime

    return MP_OBJ_FROM_PTR(t);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(MP_VFS_SPIFFSx(stat_obj), MP_VFS_SPIFFSx(stat));

STATIC mp_obj_t MP_VFS_SPIFFSx(statvfs)(mp_obj_t self_in, mp_obj_t path_in) {
    (void)path_in;
    MP_OBJ_VFS_SPIFFSx *self = MP_OBJ_TO_PTR(self_in);
    uint32_t n_used = 0;
    uint32_t n_total = 0;

    int res = SPIFFSx_API(info)(&global_filesystem, &n_total, &n_used);
    if (res < 0)
        mp_raise_OSError(-res);
    
    n_total /= global_filesystem.cfg.log_block_size;
    n_used /= global_filesystem.cfg.log_block_size;

    mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(10, NULL));
    t->items[0] = MP_OBJ_NEW_SMALL_INT(global_filesystem.cfg.log_block_size); // f_bsize
    t->items[1] = t->items[0]; // f_frsize
    t->items[2] = MP_OBJ_NEW_SMALL_INT(n_total); // f_blocks
    t->items[3] = MP_OBJ_NEW_SMALL_INT(n_total - n_used); // f_bfree
    t->items[4] = t->items[3]; // f_bavail
    t->items[5] = MP_OBJ_NEW_SMALL_INT(0); // f_files
    t->items[6] = MP_OBJ_NEW_SMALL_INT(0); // f_ffree
    t->items[7] = MP_OBJ_NEW_SMALL_INT(0); // f_favail
    t->items[8] = MP_OBJ_NEW_SMALL_INT(0); // f_flags
    t->items[9] = MP_OBJ_NEW_SMALL_INT(SPIFFSx_MACRO(_OBJ_NAME_LEN) - 1); // f_namemax

    return MP_OBJ_FROM_PTR(t);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(MP_VFS_SPIFFSx(statvfs_obj), MP_VFS_SPIFFSx(statvfs));

STATIC const mp_rom_map_elem_t MP_VFS_SPIFFSx(locals_dict_table)[] = {
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&MP_VFS_SPIFFSx(open_obj)) },
    { MP_ROM_QSTR(MP_QSTR_ilistdir), MP_ROM_PTR(&MP_VFS_SPIFFSx(ilistdir_obj)) },
    { MP_ROM_QSTR(MP_QSTR_mkdir), MP_ROM_PTR(&MP_VFS_SPIFFSx(mkdir_obj)) },
    { MP_ROM_QSTR(MP_QSTR_rmdir), MP_ROM_PTR(&MP_VFS_SPIFFSx(rmdir_obj)) },
    { MP_ROM_QSTR(MP_QSTR_chdir), MP_ROM_PTR(&MP_VFS_SPIFFSx(chdir_obj)) },
    { MP_ROM_QSTR(MP_QSTR_getcwd), MP_ROM_PTR(&MP_VFS_SPIFFSx(getcwd_obj)) },
    { MP_ROM_QSTR(MP_QSTR_remove), MP_ROM_PTR(&MP_VFS_SPIFFSx(remove_obj)) },
    { MP_ROM_QSTR(MP_QSTR_rename), MP_ROM_PTR(&MP_VFS_SPIFFSx(rename_obj)) },
    { MP_ROM_QSTR(MP_QSTR_stat), MP_ROM_PTR(&MP_VFS_SPIFFSx(stat_obj)) },
    { MP_ROM_QSTR(MP_QSTR_statvfs), MP_ROM_PTR(&MP_VFS_SPIFFSx(statvfs_obj)) },
};
STATIC MP_DEFINE_CONST_DICT(MP_VFS_SPIFFSx(locals_dict), MP_VFS_SPIFFSx(locals_dict_table));

STATIC mp_import_stat_t MP_VFS_SPIFFSx(import_stat)(void *self_in, const char *path) {
    MP_OBJ_VFS_SPIFFSx *self = self_in;
    SPIFFSx_TYPE(stat) info;
    int ret = SPIFFSx_API(stat)(&global_filesystem, path, &info);
    if (ret == 0) {
        return MP_IMPORT_STAT_FILE;
    }
    return MP_IMPORT_STAT_NO_EXIST;
}

STATIC const mp_vfs_proto_t MP_VFS_SPIFFSx(proto) = {
    .import_stat = MP_VFS_SPIFFSx(import_stat),
};

const mp_obj_type_t MP_TYPE_VFS_SPIFFSx = {
    { &mp_type_type },
    .name = MP_QSTR_VfsSpiffs,
    .make_new = MP_VFS_SPIFFSx(make_new),
    .protocol = &MP_VFS_SPIFFSx(proto),
    .locals_dict = (mp_obj_dict_t*)&MP_VFS_SPIFFSx(locals_dict),
};
