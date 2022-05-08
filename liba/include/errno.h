#ifndef LIBA_ERRNO_H
#define LIBA_ERRNO_H

/* Operation not permitted */
#define EPERM 1

/* No such file or directory */
#define ENOENT 2

/* I/O error */
#define EIO 5

/* Bad file number */
#define EBADF 9

/* Resource temporarily unavailable */
#define EAGAIN 11

/* Not enough space */
#define ENOMEM 12

/* File exists */
#define EEXIST 17

/* No such device */
#define ENODEV 19

/* Invalid argument */
#define EINVAL 22

/* Too many open files */
#define EMFILE 24

/* File too large */
#define EFBIG 27

/* Read-only file system */
#define EROFS 30

/* File name too long */
#define ENAMETOOLONG 36

/* Quota exceeded */
#define EDQUOT 122



extern int errno;

#endif
