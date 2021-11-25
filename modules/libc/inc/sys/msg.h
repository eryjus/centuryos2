/****************************************************************************************************************//**
*   @file               msg.h
*   @brief              XSI message queue structures
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Nov-15
*   @since              v0.0.14
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
*   This file is the CenturyOS implementation of the POSIX.1 `sys/msg.h` specification.  It is intended for use
*   with the CenturyOS libc and is not intended to work on any generic OS.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Nov-15 | Initial |  v0.0.14 | ADCL | Initial version
*
*///=================================================================================================================



#ifndef __SYS_MSG_H__
#define __SYS_MSG_H__



/****************************************************************************************************************//**
*   @typedef            msgqnum_t
*   @brief              Used for the number of messages in the message queue.
*
*   @posix              The <sys/msg.h> header shall define the following data types through `typedef`:\n\n
*                       `msgqnum_t` -- Used for the number of messages in the message queue.\n\n
*                       These types shall be unsigned integer types that are able to store values at least as
*                       large as a type `unsigned short`.
*
*   `msgqnum_t` will be implemented as an `unsigned int` (32-bit).
*///-----------------------------------------------------------------------------------------------------------------
#ifndef __msgqnum_t_defined
# ifndef __DOXYGEN__
#  define __msgqnum_t_defined
# endif
typedef unsigned int msgqnum_t;
#endif



/****************************************************************************************************************//**
*   @typedef            msglen_t
*   @brief              Used for the number of messages in the message queue.
*
*   @posix              The <sys/msg.h> header shall define the following data types through `typedef`:\n\n
*                       `msglen_t` -- Used for the number of bytes allowed in a message queue.\n\n
*                       These types shall be unsigned integer types that are able to store values at least as
*                       large as a type `unsigned short`.
*
*   `msglen_t` will be implemented as an `unsigned int` (32-bit).
*///-----------------------------------------------------------------------------------------------------------------
#ifndef __msglen_t_defined
# ifndef __DOXYGEN__
#  define __msglen_t_defined
# endif
typedef unsigned int msglen_t;
#endif



/****************************************************************************************************************//**
*   @def                MSG_NOERROR
*   @brief              No error if big message.
*
*   @posix              The <sys/msg.h> header shall define the following symbolic constant as a message
*                       operation flag:\n\n
*                       `MSG_NOERROR` -- No error if big message.
*///-----------------------------------------------------------------------------------------------------------------
#ifdef MSG_NOERROR
# undef MSG_NOERROR
#endif
#define MSG_NOERROR (1<<31)



/****************************************************************************************************************//**
*   @typedef            pid_t
*   @brief              Used for process IDs and process group IDs.
*
*   @posix              The <sys/msg.h> header shall define the `pid_t`, `size_t`, `ssize_t`, and `time_t`
*                       types as described in <sys/types.h>.
*
*   @posix              The <sys/types.h> header shall define at least the following types:\n\n
*                       `pid_t` -- Used for process IDs and process group IDs.\n\n
*                       All of the types shall be defined as arithmetic types of an appropriate length....
*
*   `pid_t` will be implemented as an `unsigned long` (64-bit).
*///-----------------------------------------------------------------------------------------------------------------
#ifndef __pid_t_defined
# ifndef __DOXYGEN__
#  define __pid_t_defined
# endif
typedef unsigned long pid_t;
#endif



/****************************************************************************************************************//**
*   @typedef            size_t
*   @brief              Used for sizes of objects.
*
*   @posix              The <sys/msg.h> header shall define the `pid_t`, `size_t`, `ssize_t`, and `time_t`
*                       types as described in <sys/types.h>.
*
*   @posix              The <sys/types.h> header shall define at least the following types:\n\n
*                       `size_t` -- Used for sizes of objects.\n\n
*                       All of the types shall be defined as arithmetic types of an appropriate length....
*
*   `size_t` will be implemented as an `unsigned int` (32-bit).
*///-----------------------------------------------------------------------------------------------------------------
#ifndef __size_t_defined
# ifndef __DOXYGEN__
#  define __size_t_defined
# endif
typedef unsigned long size_t;
#endif



/****************************************************************************************************************//**
*   @typedef            ssize_t
*   @brief              Used for a count of bytes or an error indication.
*
*   @posix              The <sys/msg.h> header shall define the `pid_t`, `size_t`, `ssize_t`, and `time_t`
*                       types as described in <sys/types.h>.
*
*   @posix              The <sys/types.h> header shall define at least the following types:\n\n
*                       `ssize_t` -- Used for a count of bytes or an error indication.\n\n
*                       All of the types shall be defined as arithmetic types of an appropriate length....
*
*   `ssize_t` will be implemented as an `int` (32-bit).
*///-----------------------------------------------------------------------------------------------------------------
#ifndef __ssize_t_defined
# ifndef __DOXYGEN__
#  define __ssize_t_defined
# endif
typedef int ssize_t;
#endif



/****************************************************************************************************************//**
*   @typedef            time_t
*   @brief              Used for time in seconds.
*
*   @posix              The <sys/msg.h> header shall define the `pid_t`, `size_t`, `ssize_t`, and `time_t`
*                       types as described in <sys/types.h>.
*
*   @posix              The <sys/types.h> header shall define at least the following types:\n\n
*                       `time_t` -- Used for time in seconds.\n\n
*                       All of the types shall be defined as arithmetic types of an appropriate length....
*
*   `time_t` will be implemented as a `long` (64-bit).
*///-----------------------------------------------------------------------------------------------------------------
#ifndef __time_t_defined
# ifndef __DOXYGEN__
#  define __time_t_defined
# endif
typedef long time_t;
#endif



#include <sys/ipc.h>



/****************************************************************************************************************//**
*   @struct             msqid_ds
*   @brief              POSIX.1 Message Queue ID Structure
*
*   @posix              The <sys/msg.h> header shall define the msqid_ds structure, which shall include the
*                       following members:\n\n
*                       * struct ipc_perm msg_perm   -- Operation permission structure.
*                       * msgqnum_t       msg_qnum   -- Number of messages currently on queue.
*                       * msglen_t        msg_qbytes -- Maximum number of bytes allowed on queue.
*                       * pid_t           msg_lspid  -- Process ID of last msgsnd().
*                       * pid_t           msg_lrpid  -- Process ID of last msgrcv().
*                       * time_t          msg_stime  -- Time of last msgsnd().
*                       * time_t          msg_rtime  -- Time of last msgrcv().
*                       * time_t          msg_ctime  -- Time of last change.
*///-----------------------------------------------------------------------------------------------------------------
struct msqid_ds {
    struct ipc_perm     msg_perm;       //!< Operation permission structure.
    msgqnum_t           msg_qnum;       //!< Number of messages currently on queue.
    msglen_t            msg_qbytes;     //!< Maximum number of bytes allowed on queue.
    pid_t               msg_lspid;      //!< Process ID of last msgsnd().
    pid_t               msg_lrpid;      //!< Process ID of last msgrcv().
    time_t              msg_stime;      //!< Time of last msgsnd().
    time_t              msg_rtime;      //!< Time of last msgrcv().
    time_t              msg_ctime;      //!< Time of last change.
};



/****************************************************************************************************************//**
*   @fn                 int msgctl(int msqid, int cmd, struct msqid_ds *buf)
*   @brief              XSI message control operations
*
*   The `msgctl()` function operates on XSI message queues (see XBD Message Queue). It is unspecified whether
*   this function interoperates with the realtime interprocess communication facilities defined in Realtime.
*
*   The `msgctl()` function shall provide message control operations as specified by `cmd`. The following values
*   for `cmd`, and the message control operations they specify, are:
*
*   \ref IPC_STAT - Place the current value of each member of the msqid_ds data structure associated with `msqid`
*                   into the structure pointed to by `buf`. The contents of this structure are defined in
*                   `<sys/msg.h>`.
*
*   \ref IPC_SET -  Set the value of the following members of the msqid_ds data structure associated with `msqid`
*                   to the corresponding value found in the structure pointed to by `buf`:
*
*     * `msg_perm.uid`
*     * `msg_perm.gid`
*     * `msg_perm.mode`
*     * `msg_qbytes`
*
*   Also, the msg_ctime timestamp shall be set to the current time, as described in IPC General Description.
*
*   \ref IPC_SET can only be executed by a process with appropriate privileges or that has an effective user ID
*   equal to the value of msg_perm.cuid or msg_perm.uid in the \ref msqid_ds data structure associated with `msqid`.
*   Only a process with appropriate privileges can raise the value of `msg_qbytes`.
*
*   \ref IPC_RMID - Remove the message queue identifier specified by `msqid` from the system and destroy the
*                   message queue and msqid_ds data structure associated with it. `IPC_RMD` can only be executed
*                   by a process with appropriate privileges or one that has an effective user ID equal to the
*                   value of `msg_perm.cuid` or `msg_perm.uid` in the msqid_ds data structure associated with
*                   `msqid`.
*
*   @param          msqid       message queue id
*   @param          cmd         message queue command
*   @param          buf         buffer on which to operate
*
*   @returns        Upon successful completion, `msgctl()` shall return 0; otherwise, it shall return -1
*                   and set `errno` to indicate the error.
*
*   The msgctl() function shall fail if:
*
*   @errno \ref EACCES - The argument `cmd` is `IPC_STAT` and the calling process does not have read permission;
*               see XSI Interprocess Communication.
*   @errno \ref EINVAL - The value of `msqid` is not a valid message queue identifier; or the value of `cmd` is
*               not a valid command.
*   @errno \ref EPERM - The argument `cmd` is `IPC_RMID` or `IPC_SET` and the effective user ID of the calling
*               process is not equal to that of a process with appropriate privileges and it is not equal
*               to the value of `msg_perm.cuid` or `msg_perm.uid` in the data structure associated with msqid.
*   @errno \ref EPERM - The argument `cmd` is `IPC_SET`, an attempt is being made to increase to the value of
*               `msg_qbytes`, and the effective user ID of the calling process does not have appropriate
*               privileges.
*///-----------------------------------------------------------------------------------------------------------------
int       msgctl(int msqid, int cmd, struct msqid_ds *buf);



/****************************************************************************************************************//**
*   @fn                 int msgget(key_t key, int msgflg)
*   @brief              get the XSI message queue identifier
*
*   The `msgget()` function operates on XSI message queues (see XBD Message Queue). It is unspecified whether
*   this function interoperates with the realtime interprocess communication facilities defined in Realtime.
*
*   The `msgget()` function shall return the message queue identifier associated with the argument `key`.
*
*   A message queue identifier, associated message queue, and data structure (see `<sys/msg.h>`), shall be
*   created for the argument `key` if one of the following is true:
*   * The argument `key` is equal to \ref IPC_PRIVATE.
*   * The argument `key` does not already have a message queue identifier associated with it, and
*     (`msgflg` & \ref IPC_CREAT) is non-zero.
*
*   Upon creation, the data structure associated with the new message queue identifier shall be initialized
*   as follows:
*   * `msg_perm.cuid`, `msg_perm.uid`, `msg_perm.cgid`, and `msg_perm.gid` shall be set to the effective
*     user ID and effective group ID, respectively, of the calling process.
*   * The low-order 9 bits of `msg_perm.mode` shall be set to the low-order 9 bits of `msgflg`.
*   * `msg_qnum`, `msg_lspid`, `msg_lrpid`, `msg_stime`, and `msg_rtime` shall be set to 0.
*   * `msg_ctime` shall be set to the current time, as described in IPC General Description.
*   * `msg_qbytes` shall be set to the system limit.
*
*   @param              key             The key used to uniquely identify the message queue
*   @param              msgflg          Flags for the operation
*
*   @returns            Upon successful completion, `msgget()` shall return a non-negative integer,
*                       namely a message queue identifier. Otherwise, it shall return -1 and set `errno`
*                       to indicate the error.
*
*   The msgget() function shall fail if:
*
*   @errno \ref EACCES - A message queue identifier exists for the argument `key`, but operation permission as
*               specified by the low-order 9 bits of `msgflg` would not be granted; see XSI Interprocess
*               Communication.
*   @errno \ref EEXIST - A message queue identifier exists for the argument `key` but
*               ((`msgflg` & \ref IPC_CREAT) && (`msgflg` & \ref IPC_EXCL)) is non-zero.
*   @errno \ref ENOENT - A message queue identifier does not exist for the argument `key` and
*               (`msgflg` & \ref IPC_CREAT) is 0.
*   @errno \ref ENOSPC - A message queue identifier is to be created but the system-imposed limit on the
*               maximum number of allowed message queue identifiers system-wide would be exceeded.
*///-----------------------------------------------------------------------------------------------------------------
int       msgget(key_t key, int msgflg);



/****************************************************************************************************************//**
*   @fn                 ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
*   @brief              XSI message receive operation
*
*   The `msgrcv()` function operates on XSI message queues (see XBD Message Queue). It is unspecified whether
*   this function interoperates with the realtime interprocess communication facilities defined in Realtime.
*
*   The msgrcv() function shall read a message from the queue associated with the message queue identifier
*   specified by msqid and place it in the user-defined buffer pointed to by `msgp`.
*
*   The application shall ensure that the argument `msgp` points to a user-defined buffer that contains first
*   a field of type long specifying the type of the message, and then a data portion that holds the data bytes
*   of the message. The structure below is an example of what this user-defined buffer might look like:
*
*   ~~~
*   struct mymsg {
*       long    mtype;     // Message type.
*       char    mtext[1];  // Message text.
*   }
*   ~~~
*
*   The structure member `mtype` is the received message's type as specified by the sending process.
*
*   The structure member `mtext` is the text of the message.
*
*   The argument `msgsz` specifies the size in bytes of mtext. The received message shall be truncated to
*   `msgsz` bytes if it is larger than `msgsz` and (`msgflg` & \ref MSG_NOERROR) is non-zero. The truncated
*   part of the message shall be lost and no indication of the truncation shall be given to the calling process.
*
*   If the value of `msgsz` is greater than \ref SSIZE_MAX, the result is implementation-defined.
*
*   The argument `msgtyp` specifies the type of message requested as follows:
*   * If `msgtyp` is 0, the first message on the queue shall be received.
*   * If `msgtyp` is greater than 0, the first message of type `msgtyp` shall be received.
*   * If `msgtyp` is less than 0, the first message of the lowest type that is less than or equal to the
*     absolute value of `msgtyp` shall be received.
*
*   The argument `msgflg` specifies the action to be taken if a message of the desired type is not on the
*   queue. These are as follows:
*   * If (`msgflg` & \ref IPC_NOWAIT) is non-zero, the calling thread shall return immediately with a
*     return value of -1 and `errno` set to \ref ENOMSG.
*   * If (`msgflg` & \ref IPC_NOWAIT) is 0, the calling thread shall suspend execution until one of
*     the following occurs:
*     * A message of the desired type is placed on the queue.
*     * The message queue identifier `msqid` is removed from the system; when this occurs, errno shall be
*       set to \ref EIDRM and -1 shall be returned.
*     * The calling thread receives a signal that is to be caught; in this case a message is not received
*       and the calling thread resumes execution in the manner prescribed in sigaction.
*
*   Upon successful completion, the following actions are taken with respect to the data structure associated
*   with `msqid`:
*   * `msg_qnum` shall be decremented by 1.
*   * `msg_lrpid` shall be set to the process ID of the calling process.
*   * `msg_rtime` shall be set to the current time, as described in IPC General Description.
*
*   @param              msqid       The message queue identifier
*   @param              msgp        Pointer to the buffer to receive the message
*   @param              msgsz       The size of the buffer to receive the message
*   @param              msgtyp      The type of message so retrieve; see detailed description
*   @param              msgflg      The flags for the operation
*
*   @returns            Upon successful completion, `msgrcv()` shall return a value equal to the number of
*                       bytes actually placed into the buffer `mtext`. Otherwise, no message shall be received,
*                       `msgrcv()` shall return -1, and `errno` shall be set to indicate the error.
*
*   The `msgrcv()` function shall fail if:
*
*   @errno \ref E2BIG - The value of mtext is greater than msgsz and (`msgflg` & \ref MSG_NOERROR) is 0.
*   @errno \ref EACCES - Operation permission is denied to the calling process; see XSI Interprocess Communication.
*   @errno \ref EIDRM - The message queue identifier msqid is removed from the system.
*   @errno \ref EINTR - The `msgrcv()` function was interrupted by a signal.
*   @errno \ref EINVAL - `msqid` is not a valid message queue identifier.
*   @errno \ref ENOMSG - The queue does not contain a message of the desired type and (`msgflg` & \ref IPC_NOWAIT)
*               is non-zero.
*///-----------------------------------------------------------------------------------------------------------------
ssize_t   msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);



/****************************************************************************************************************//**
*   @fn                 int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
*   @brief              XSI message send operation
*
*   The `msgsnd()` function operates on XSI message queues (see XBD Message Queue). It is unspecified whether
*   this function interoperates with the realtime interprocess communication facilities defined in Realtime.
*
*   The `msgsnd()` function shall send a message to the queue associated with the message queue identifier
*   specified by `msqid`.
*
*   The application shall ensure that the argument `msgp` points to a user-defined buffer that contains first
*   a field of type long specifying the type of the message, and then a data portion that holds the data bytes
*   of the message. The structure below is an example of what this user-defined buffer might look like:
*
*   ~~~
*   struct mymsg {
*       long   mtype;       // Message type.
*       char   mtext[1];    // Message text.
*   }
*   ~~~
*
*   The structure member `mtype` is a non-zero positive type long that can be used by the receiving process
*   for message selection.
*
*   The structure member `mtext` is any text of length `msgsz` bytes. The argument `msgsz` can range from 0
*   to a system-imposed maximum.
*
*   The argument `msgflg` specifies the action to be taken if one or more of the following is true:
*   * The number of bytes already on the queue is equal to `msg_qbytes`; see `<sys/msg.h>`.
*   * The total number of messages on all queues system-wide is equal to the system-imposed limit.
*
*   These actions are as follows:
*   * If (`msgflg` & \ref IPC_NOWAIT) is non-zero, the message shall not be sent and the calling thread
*     shall return immediately.
*   * If (`msgflg` & \ref IPC_NOWAIT) is 0, the calling thread shall suspend execution until one of the
*     following occurs:
*     * The condition responsible for the suspension no longer exists, in which case the message is sent.
*     * The message queue identifier `msqid` is removed from the system; when this occurs, errno shall
*       be set to \ref EIDRM and -1 shall be returned.
*     * The calling thread receives a signal that is to be caught; in this case the message is not sent
*       and the calling thread resumes execution in the manner prescribed in sigaction.
*
*   Upon successful completion, the following actions are taken with respect to the data structure
*   associated with `msqid`; see `<sys/msg.h>`:
*   * `msg_qnum` shall be incremented by 1.
*   * `msg_lspid` shall be set to the process ID of the calling process.
*   * `msg_stime` shall be set to the current time, as described in IPC General Description.
*
*   @param              msqid           The message queue id
*   @param              msgp            Pointer to a message buffer
*   @param              msgsz           The size of the message
*   @param              msgflg          The flags for the operation
*
*   @returns            Upon successful completion, `msgsnd()` shall return 0; otherwise, no message
*                       shall be sent, msgsnd() shall return -1, and errno shall be set to indicate
*                       the error.
*
*   The `msgsnd()` function shall fail if:
*
*   @errno \ref EACCES - Operation permission is denied to the calling process; see XSI Interprocess
*               Communication.
*   @errno \ref EAGAIN - The message cannot be sent for one of the reasons cited above and
*               (`msgflg` & \ref IPC_NOWAIT) is non-zero.
*   @errno \ref EIDRM - The message queue identifier `msqid` is removed from the system.
*   @errno \ref EINTR - The `msgsnd()` function was interrupted by a signal.
*   @errno \ref EINVAL - The value of `msqid` is not a valid message queue identifier, or the value
*               of `mtype` is less than 1; or the value of `msgsz` is greater than the system-imposed limit.
*///-----------------------------------------------------------------------------------------------------------------
int       msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);



#endif


