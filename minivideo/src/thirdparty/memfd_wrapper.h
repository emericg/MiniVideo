/*!
 * \file      memfd_wrapper.h
 * \license   https://creativecommons.org/publicdomain/zero/1.0/
 *
 * Original wrapper:
 * - Ahmed S. Darwish (https://github.com/a-darwish/memfd-examples)
 * Related resources:
 * - http://man7.org/linux/man-pages/man2/memfd_create.2.html
 * - https://dvdhrm.wordpress.com/2014/06/10/memfd_create2/
 */

#ifndef MEMFD_WRAPPER_H
#define MEMFD_WRAPPER_H

#if defined(__linux__) || defined(__gnu_linux)

#include <stdio.h>
#include <unistd.h>

#include <sys/syscall.h>
#include <linux/memfd.h>

/* ************************************************************************** */

/*!
 * \brief memfd_create() syscall wrapper
 *
 * No glibc wrappers exist for memfd_create(2), so we provide our own.
 *
 * Also define memfd fcntl sealing macros. While they are already
 * defined in the kernel header file <linux/fcntl.h>, that file as
 * a whole conflicts with the original glibc header <fnctl.h>.
 */
static inline int memfd_create(const char *name, unsigned int flags)
{
    return syscall(__NR_memfd_create, name, flags);
}

#ifndef F_LINUX_SPECIFIC_BASE
#define F_LINUX_SPECIFIC_BASE 1024
#endif

#ifndef F_ADD_SEALS
#define F_ADD_SEALS (F_LINUX_SPECIFIC_BASE + 9)
#define F_GET_SEALS (F_LINUX_SPECIFIC_BASE + 10)

#define F_SEAL_SEAL     0x0001  // prevent further seals from being set
#define F_SEAL_SHRINK   0x0002  // prevent file from shrinking
#define F_SEAL_GROW     0x0004  // prevent file from growing
#define F_SEAL_WRITE    0x0008  // prevent writes
#endif // F_ADD_SEALS

/* ************************************************************************** */

/*!
 * \brief fopen() wrapper to get a FILE object from a memfd file descriptor
 */
static inline FILE *memfd_fopen(const char *name, const char *mode)
{
    FILE *file = NULL;

    int fd = memfd_create(name, MFD_CLOEXEC);
    if (fd >= 0)
    {
        file = fdopen(fd, mode);
    }

    return file;
}

/* ************************************************************************** */
#endif // defined(__linux__) || defined(__gnu_linux)
#endif // MEMFD_WRAPPER_H
