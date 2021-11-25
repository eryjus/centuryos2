/****************************************************************************************************************//**
*   @file               ipc.h
*   @brief              XSI interprocess communication access structure
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Nov-15
*   @since              v0.0.14
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
*   This file is the CenturyOS implementation of the POSIX.1 `sys/cip.h` specification.  It is intended for use
*   with the CenturyOS libc and is not intended to work on any generic OS.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Nov-15 | Initial |  v0.0.14 | ADCL | Initial version
*
*///=================================================================================================================



#ifndef __SYS_IPC_H__
#define __SYS_IPC_H__



/****************************************************************************************************************//**
*   @typedef            uid_t
*   @brief              Used for user IDs.
*
*   @posix              The <sys/ipc.h> header shall define the `uid_t`, `gid_t`, `mode_t`, and `key_t` types as
*                       described in <sys/types.h>.
*
*   @posix              The <sys/types.h> header shall define at least the following types:\n\n
*                       `uid_t` -- Used for user IDs.\n\n
*                       All of the types shall be defined as arithmetic types of an appropriate length....
*
*   `uid_t` will be implemented as an `unsigned long` (64-bit).
*///-----------------------------------------------------------------------------------------------------------------
#ifndef __uid_t_defined
# ifndef __DOXYGEN__
#  define __uid_t_defined
# endif
typedef unsigned long uid_t;
#endif



/****************************************************************************************************************//**
*   @typedef            gid_t
*   @brief              Used for group IDs.
*
*   @posix              The <sys/ipc.h> header shall define the `uid_t`, `gid_t`, `mode_t`, and `key_t` types as
*                       described in <sys/types.h>.
*
*   @posix              The <sys/types.h> header shall define at least the following types:\n\n
*                       `gid_t` -- Used for group IDs.\n\n
*                       All of the types shall be defined as arithmetic types of an appropriate length....
*
*   `gid_t` will be implemented as an `unsigned long` (64-bit).
*///-----------------------------------------------------------------------------------------------------------------
#ifndef __gid_t_defined
# ifndef __DOXYGEN__
#  define __gid_t_defined
# endif
typedef unsigned long gid_t;
#endif



/****************************************************************************************************************//**
*   @typedef            mode_t
*   @brief              Used for some file attributes.
*
*   @posix              The <sys/ipc.h> header shall define the `uid_t`, `gid_t`, `mode_t`, and `key_t` types as
*                       described in <sys/types.h>.
*
*   @posix              The <sys/types.h> header shall define at least the following types:\n\n
*                       `mode_t` -- Used for some file attributes.\n\n
*                       All of the types shall be defined as arithmetic types of an appropriate length....
*
*   `mode_t` will be implemented as an `unsigned int` (32-bit).
*///-----------------------------------------------------------------------------------------------------------------
#ifndef __mode_t_defined
# ifndef __DOXYGEN__
#  define __mode_t_defined
# endif
typedef unsigned int mode_t;
#endif



/****************************************************************************************************************//**
*   @typedef            key_t
*   @brief              Used for XSI interprocess communication.
*
*   @posix              The <sys/ipc.h> header shall define the `uid_t`, `gid_t`, `mode_t`, and `key_t` types as
*                       described in <sys/types.h>.
*
*   @posix              The <sys/types.h> header shall define at least the following types:\n\n
*                       `key_t` -- Used for XSI interprocess communication.\n\n
*                       All of the types shall be defined as arithmetic types of an appropriate length....
*
*   `key_t` will be implemented as an `unsigned long` (64-bit).
*///-----------------------------------------------------------------------------------------------------------------
#ifndef __key_t_defined
# ifndef __DOXYGEN__
#  define __key_t_defined
# endif
typedef unsigned long key_t;
#endif



/****************************************************************************************************************//**
*   @struct             ipc_perm
*   @brief              POSIX.1 IPC Permissions Structure
*
*   @posix              The <sys/ipc.h> header shall define the `ipc_perm` structure, which shall include the
*                       following members:
*                       * uid_t    uid    -- Owner's user ID.
*                       * gid_t    gid    -- Owner's group ID.
*                       * uid_t    cuid   -- Creator's user ID.
*                       * gid_t    cgid   -- Creator's group ID.
*                       * mode_t   mode   -- Read/write permission.
*///-----------------------------------------------------------------------------------------------------------------
struct ipc_perm {
    uid_t               uid;            //!< Owner's user ID.
    gid_t               gid;            //!< Owner's group ID.
    uid_t               cuid;           //!< Creator's user ID.
    gid_t               cgid;           //!< Creator's group ID.
    mode_t              mode;           //!< Read/write permission.
};




/****************************************************************************************************************//**
*   @def                IPC_CREAT
*   @brief              Create entry if key does not exist.
*
*   @posix              The <sys/ipc.h> header shall define the following symbolic constants:\n\n
*                       `IPC_CREAT` -- Create entry if key does not exist.
*///-----------------------------------------------------------------------------------------------------------------
#ifdef IPC_CREAT
# undef IPC_CREAT
#endif
#define IPC_CREAT (1<<10)




/****************************************************************************************************************//**
*   @def                IPC_EXCL
*   @brief              Fail if key exists.
*
*   @posix              The <sys/ipc.h> header shall define the following symbolic constants:\n\n
*                       `IPC_EXCL` -- Fail if key exists.
*///-----------------------------------------------------------------------------------------------------------------
#ifdef IPC_EXCL
# undef IPC_EXCL
#endif
#define IPC_EXCL (1<<11)




/****************************************************************************************************************//**
*   @def                IPC_NOWAIT
*   @brief              Error if request must wait.
*
*   @posix              The <sys/ipc.h> header shall define the following symbolic constants:\n\n
*                       `IPC_NOWAIT` -- Error if request must wait.
*///-----------------------------------------------------------------------------------------------------------------
#ifdef IPC_NOWAIT
# undef IPC_NOWAIT
#endif
#define IPC_NOWAIT (1<<12)




/****************************************************************************************************************//**
*   @def                IPC_PRIVATE
*   @brief              Private key.
*
*   @posix              The <sys/ipc.h> header shall define the following symbolic constants:\n\n
*                       `IPC_PRIVATE` -- Private key.
*///-----------------------------------------------------------------------------------------------------------------
#ifdef IPC_PRIVATE
# undef IPC_PRIVATE
#endif
#define IPC_PRIVATE ((key_t)0)




/****************************************************************************************************************//**
*   @def                IPC_RMID
*   @brief              Remove identifier.
*
*   @posix              The <sys/ipc.h> header shall define the following symbolic constants:\n\n
*                       `IPC_RMID` -- Remove identifier.
*///-----------------------------------------------------------------------------------------------------------------
#ifdef IPC_RMID
# undef IPC_RMID
#endif
#define IPC_RMID 0



/****************************************************************************************************************//**
*   @def                IPC_SET
*   @brief              Set options.
*
*   @posix              The <sys/ipc.h> header shall define the following symbolic constants:\n\n
*                       `IPC_SET` -- Set options.
*///-----------------------------------------------------------------------------------------------------------------
#ifdef IPC_SET
# undef IPC_SET
#endif
#define IPC_SET 1



/****************************************************************************************************************//**
*   @def                IPC_STAT
*   @brief              Get options.
*
*   @posix              The <sys/ipc.h> header shall define the following symbolic constants:\n\n
*                       `IPC_STAT` -- Get options.
*///-----------------------------------------------------------------------------------------------------------------
#ifdef IPC_STAT
# undef IPC_STAT
#endif
#define IPC_STAT 2



/****************************************************************************************************************//**
*   @fn                 key_t ftok(const char *path, int id)
*   @brief              generate an IPC key
*
*   The `ftok()` function shall return a key based on path and id that is usable in subsequent calls to `msgget()`,
*   `semget()`, and `shmget()`. The application shall ensure that the path argument is the pathname of an
*   existing file that the process is able to `stat()`, with the exception that if stat() would fail with [EOVERFLOW]
*   due to file size, `ftok()` shall still succeed.
*
*   The `ftok()` function shall return the same key value for all paths that name the same file, when called with
*   the same id value, and should return different key values when called with different id values or with paths
*   that name different files existing on the same file system at the same time. It is unspecified whether `ftok()`
*   shall return the same key value when called again after the file named by path is removed and recreated with
*   the same name.
*
*   Only the low-order 8-bits of id are significant. The behavior of ftok() is unspecified if these bits are 0.
*
*   @param              path            path to a file or directory
*   @param              id              project ID (only the lower 8 bits are used)
*
*   @return             Upon successful completion, `ftok()` shall return a key. Otherwise, `ftok()` shall
*                       return `(key_t)-1` and set errno to indicate the error.
*
*   `ftok()` *shall* fail if:
*
*   @errno \ref EACCES - Search permission is denied for a component of the path prefix.
*   @errno \ref EIO - An error occurred while reading from the file system.
*   @errno \ref ELOOP - A loop exists in symbolic links encountered during resolution of the *path* argument.
*   @errno \ref ENAMETOOLONG - The length of a component of a pathname is longer than {NAME_MAX}.
*   @errno \ref ENOENT - A component of *path* does not name an existing file or path is an empty string.
*   @errno \ref ENOTDIR - A component of the path prefix names an existing file that is neither a directory
                nor a symbolic link to a directory, or the *path* argument contains at least one non-'/'
                character and ends with one or more trailing '/' characters and the last pathname
                component names an existing file that is neither a directory nor a symbolic link to a directory.
*
*   `ftok()` *may* fail if:
*
*   @errno \ref ELOOP - More than {SYMLOOP_MAX} symbolic links were encountered during resolution of
                the *path* argument.
*   @errno \ref ENAMETOOLONG - The length of a pathname exceeds {PATH_MAX}, or pathname resolution of a symbolic
                link produced an intermediate result with a length that exceeds {PATH_MAX}.
*
*///-----------------------------------------------------------------------------------------------------------------
extern key_t ftok(const char *path, int id);



#endif

