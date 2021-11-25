/****************************************************************************************************************//**
*   @file               msgq.cc
*   @brief              Internal implementation of Messages and Message Queues
*   @author             Adam Clark (hobbyos@eryjus.com)
*   @date               2021-Nov-17
*   @since              v0.0.14
*
*   @copyright          Copyright (c)  2017-2021 -- Adam Clark\n
*                       Licensed under "THE BEER-WARE LICENSE"\n
*                       See \ref LICENSE.md for details.
*
*   This file contains the internal implementation of Messages and MessageQueues.  This is used for the
*   implementation of the POSIX XSI IPC.
*
* ------------------------------------------------------------------------------------------------------------------
*
*   |     Date    | Tracker |  Version | Pgmr | Description
*   |:-----------:|:-------:|:--------:|:----:|:--------------------------------------------------------------------
*   | 2021-Nov-17 | Initial |  v0.0.14 | ADCL | Initial version
*
*///=================================================================================================================



#include "types.h"
#include "heap.h"
#include "kernel-funcs.h"
#include "lists.h"
#include "scheduler.h"
#include "boot-interface.h"

#include <errno.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>



/****************************************************************************************************************//**
*   @typedef            Message_t
*   @brief              Formalization of the internal implementation of a message on a message queue
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             Message_t
*   @brief              The internal implementation of a message on a message queue
*
*   This structure represents a single message on a message queue.  `size` indicates the length of `text` and can
*   be 0.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct Message_t {
    ListHead_t::List_t list;        //!< The structure to insert this Message into the Queue
    size_t size;                    //!< The size of the message text \note Does not include the `type` member
    long type;                      //!< The type of message \note Must be positive
    char text[0];                   //!< The text fo the message
} Message_t;



/****************************************************************************************************************//**
*   @typedef            MessageQueue_t
*   @brief              Formalization of the internal implementation of a message queue
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             MessageQueue_t
*   @brief              The internal implementation of a message queue
*
*   This structure represents a queue of messages.
*///-----------------------------------------------------------------------------------------------------------------
typedef struct MessageQueue_t {
    ListHead_t::List_t list;        //!< The list entry for inserting into the global list
    ListHead_t head;                //!< The actual list of messages \note Holds the lock for this queue members
    ListHead_t sendWait;            //!< The list of processes waiting to send
    ListHead_t recvWait;            //!< The list of processes waiting to receive
    int msgqid;                     //!< The message queue id for this queue
    key_t key;                      //!< The key used to generate this queue
    struct ipc_perm perm;           //!< The current effective IPC permissions
    AtomicInt_t msgCnt;             //!< The current number of message on the queue
    int msgmnb;                     //!< The maximum number of bytes allowed on this queue
    AtomicInt_t currentBytes;       //!< The current number of bytes on this queue
    pid_t lspid;                    //!< The PID that last sent a message
    pid_t lrpid;                    //!< The PID that last received a message
    time_t lstime;                  //!< The time of the last send
    time_t lrtime;                  //!< The time of the last receive
    time_t lctime;                  //!< The time of the last change
    Process_t *owningProc;          //!< The process that owns this message queue
} MessageQueue_t;



/****************************************************************************************************************//**
*   @typedef            MessageMaster_t
*   @brief              Formalization of the internal implementation of a message master control structure
*///-----------------------------------------------------------------------------------------------------------------
/****************************************************************************************************************//**
*   @struct             MessageMaster_t
*   @brief              The internal implementation of a message master control structure
*
*   This structure is a singleton of the master message control structure
*///-----------------------------------------------------------------------------------------------------------------
typedef struct MessageMaster_t {
    int msgmni;                     //!< The maximum number of queues allowed
    int msgmnb;                     //!< The maximum number of bytes per queue
    int msgmax;                     //!< The maximum length of a message
    AtomicInt_t currentQueues;      //!< The current number of message queues
    int nextId;                     //!< The next ID to hand out
    ListHead_t head;                //!< The global list of message queues \note holds the lock for this structure
} MessageMaster_t;



/****************************************************************************************************************//**
*   @var                msgMaster
*   @brief              The message master control structure variable.
*///-----------------------------------------------------------------------------------------------------------------
MessageMaster_t msgMaster;



/****************************************************************************************************************//**
*   @fn                 Return_t MsgqEarlyInit(BootInterface_t *b)
*   @brief              Perform the early initialization for the Messaging module
*
*   Initialize the msgMaster structure to its proper startup values
*
*   @param              b               The interface from the bootloader; \note unused
*
*   @returns            0
*///-----------------------------------------------------------------------------------------------------------------
extern "C" Return_t MsgqEarlyInit(BootInterface_t *b)
{
    ProcessInitTable();

    kMemSetB(&msgMaster, 0, sizeof(msgMaster));
    ListInit(&msgMaster.head.list);
    msgMaster.msgmax = MSGMAX;
    msgMaster.msgmni = MSGMNI;
    msgMaster.msgmnb = MSGMNB;
    msgMaster.nextId = 1;

    return 0;
}



/****************************************************************************************************************//**
*   @fn                 MessageQueue_t *GetQueueByKey(key_t key)
*   @brief              Find a Message Queue by its key
*
*   @param              key             The key of the Message Queue to find
*
*   @returns            NULL when not found, or the Message Queue address
*
*   @retval             NULL            The Message Queue does not exist
*   @retval             non-NULL        The address of the Message Queue in memory
*///-----------------------------------------------------------------------------------------------------------------
static MessageQueue_t *GetQueueByKey(key_t key)
{
    ListHead_t::List_t *wrk = msgMaster.head.list.next;

    while (wrk != &msgMaster.head.list) {
        MessageQueue_t *queue = FIND_PARENT(wrk, MessageQueue_t, list);
        if (queue->key == key) return queue;
        wrk = wrk->next;
    }

    return NULL;
}



/****************************************************************************************************************//**
*   @fn                 MessageQueue_t *GetQueueById(int id)
*   @brief              Find a Message Queue by its id
*
*   @param              id              The id of the Message Queue to find
*
*   @returns            NULL when not found, or the Message Queue address
*
*   @retval             NULL            The Message Queue does not exist
*   @retval             non-NULL        The address of the Message Queue in memory
*///-----------------------------------------------------------------------------------------------------------------
static MessageQueue_t *GetQueueById(int id)
{
    ListHead_t::List_t *wrk = msgMaster.head.list.next;

    while (wrk != &msgMaster.head.list) {
        MessageQueue_t *queue = FIND_PARENT(wrk, MessageQueue_t, list);
        if (queue->msgqid == id) return queue;
        wrk = wrk->next;
    }

    return NULL;
}



/****************************************************************************************************************//**
*   @fn                 int GetNextId(void)
*   @brief              Get the next available Message Queue ID
*
*   @returns            The next available Message Queue ID
*///-----------------------------------------------------------------------------------------------------------------
static int GetNextId(void)
{
    int rv = msgMaster.nextId ++;
    if (rv < 0) rv = 1;
    while (GetQueueById(rv) != NULL) {
        rv = msgMaster.nextId ++;
        if (rv < 0) rv = 1;
    }

    return rv;
}



/****************************************************************************************************************//**
*   @fn                 void CreateNewQueue(key_t key, int id, int msgflg)
*   @brief              Create and initialize a brand new Message Queue
*
*   This function will create and initialize a brand new Message Queue, inserting it into the global list of
*   Message Queues.
*
*   @param              key         The unique key for the message queue
*   @param              id          The unique ID for the nessage queue
*   @param              msgflg      The flags used for creating the queue (used for permissions)
*///-----------------------------------------------------------------------------------------------------------------
static void CreateNewQueue(key_t key, int id, int msgflg)
{
    MessageQueue_t *q = NEW(MessageQueue_t);

    kMemSetB(q, 0, sizeof(MessageQueue_t));
    ListInit(&q->list);
    ListInit(&q->head.list);
    ListInit(&q->sendWait.list);
    ListInit(&q->recvWait.list);
    q->msgqid = id;
    q->key = key;
    q->perm.cgid = q->perm.gid = EffectiveGid();
    q->perm.cuid = q->perm.uid = EffectiveUid();
    q->perm.mode = msgflg & 0777;
    q->msgmnb = msgMaster.msgmnb;
    time(&q->lctime);
    q->owningProc = CurrentThread();

    ListAddTail(&msgMaster.head, &q->list);
    AtomicInc(&msgMaster.currentQueues);
}



/****************************************************************************************************************//**
*   @fn                 int msg_MsgqGet(key_t key, int msgflg)
*   @brief              Internal function to get a Message Queue
*
*   This function is the internal implementation to obtain/create a message queue.  This is the actual function
*   that will back the `msgget` system function.
*
*   @param              key             The key to use to uniquely identify a message queue
*   @param              msgflg          Flags to be used in getting the proper message queue
*
*   @returns            The Message Queue Identifier or an error value
*
*   @retval             -EACCES         When the Message Queue exists but permissions would not allow operations
*   @retval             -EEXIST         When the Message Queue exists and it was not expected to exist already (as
*                                       in: `(msgflg & (IPC_CREAT | IPC_EXCL)) != 0`)
*   @retval             -ENOENT         When the Message Queue does not exist but IPC_CREAT was not specified
*   @retval             -ENOSPC         When the Message Queue would otherwise be created but there is no room
*///-----------------------------------------------------------------------------------------------------------------
extern "C" int msg_MsgqGet(key_t key, int msgflg)
{
    int rv;
    MessageQueue_t *q;

    SpinLock(&msgMaster.head.lock); {
        // -- handle a private key first
        if (key == IPC_PRIVATE) {
            // -- have room?
            if (AtomicRead(&msgMaster.currentQueues) >= msgMaster.msgmni) {
                rv = -ENOSPC;
                goto exit;
            }
            // -- create a new queue
            rv = GetNextId();
            CreateNewQueue(key, rv, msgflg);
            SpinUnlock(&msgMaster.head.lock);
            goto exit;
        }

        // -- is this an existing queue?
        q = GetQueueByKey(key);

        if (q == NULL) {
            // -- no existing queue
            if (!(msgflg & IPC_CREAT)) {
                rv = -ENOENT;
                goto exit;
            }
            // -- have room?
            if (AtomicRead(&msgMaster.currentQueues) >= msgMaster.msgmni) {
                rv = -ENOSPC;
                goto exit;
            }
            // -- create a new queue
            rv = GetNextId();
            CreateNewQueue(key, rv, msgflg);
            SpinUnlock(&msgMaster.head.lock);
            goto exit;
        } else {
            // -- existing queue
            if (msgflg & (IPC_CREAT | IPC_EXCL)) {
                rv = -EEXIST;
                goto exit;
            }
            // -- need uid and gid
            uid_t cuid = EffectiveUid();
            gid_t cgid = EffectiveGid();

            // -- does uid have permission?
            if (cuid == q->perm.cuid || cuid == q->perm.uid) {
                if (q->perm.mode & 0600) {
                    rv = q->msgqid;
                    goto exit;
                }
            }
            // -- does gid have permission?
            if (cgid == q->perm.cgid || cgid == q->perm.gid) {
                if (q->perm.mode & 0060) {
                    rv = q->msgqid;
                    goto exit;
                }
            }
            // -- does anyone have permission?
            if (q->perm.mode & 0006) {
                rv = q->msgqid;
                goto exit;
            }
            // -- no permission
            rv = -EACCES;
            goto exit;
        }

exit:
        SpinUnlock(&msgMaster.head.lock);
    }

    return rv;
}



/****************************************************************************************************************//**
*   @fn                 int MsgqStat(int msqid, struct msqid_ds *buf)
*   @brief              Handle getting the message queue status
*
*   This function is the internal implementation to obtain the Message Queue status.
*
*   @param              msqid           The key to use to uniquely identify a message queue
*   @param              buf             The \ref msqid_ds structure to be used with the operation
*
*   @returns            0 on success, non-zero on error
*
*   @retval             -EINVAL         Invalid `cmd` or invalid `msgqid`
*   @retval             -EACCES         Insufficient permissions
*   @retval             -EFAULT         The `buf` is not mapped
*///-----------------------------------------------------------------------------------------------------------------
static int MsgqStat(int msqid, struct msqid_ds *buf)
{
    int rv;
    MessageQueue_t *q;

    SpinLock(&msgMaster.head.lock); {
        q = GetQueueById(msqid);

        if (q == NULL) {
            SpinUnlock(&msgMaster.head.lock);
            return -EINVAL;
        }

        SpinLock(&q->head.lock); {
            SpinUnlock(&msgMaster.head.lock);

            uid_t cuid = EffectiveUid();
            gid_t cgid = EffectiveGid();

            if (cuid == q->perm.cuid || cuid == q->perm.uid) {
                if (q->perm.mode & 0400) goto more;
            }

            if (cgid == q->perm.cgid || cgid == q->perm.gid) {
                if (q->perm.mode & 0x0040) goto more;
            }

            if (q->perm.mode & 0x0004) goto more;

            rv = -EACCES;
            goto exit;

more:
            if (!MmuIsMapped((Addr_t)buf) || !MmuIsMapped((Addr_t)buf + sizeof(struct msqid_ds) - 1)) {
                rv = -EFAULT;
                goto exit;
            }

            buf->msg_ctime = q->lctime;
            buf->msg_lrpid = q->lrtime;
            buf->msg_lspid = q->lstime;
            buf->msg_perm = q->perm;
            buf->msg_qbytes = q->msgmnb;
            buf->msg_qnum = AtomicRead(&q->msgCnt);
            buf->msg_lrpid = q->lrpid;
            buf->msg_lspid = q->lspid;

            rv = 0;

exit:
            SpinUnlock(&q->head.lock);
        }
    }

    return rv;
}



/****************************************************************************************************************//**
*   @fn                 int MsgqSet(int msqid, struct msqid_ds *buf)
*   @brief              Handle setting the message queue status
*
*   This function is the internal implementation to set the Message Queue values.
*
*   @param              msqid           The key to use to uniquely identify a message queue
*   @param              buf             The \ref msqid_ds structure to be used with the operation
*
*   @returns            0 on success, non-zero on error
*
*   @retval             -EINVAL         Invalid `cmd` or invalid `msgqid`
*   @retval             -EFAULT         The `buf` is not mapped
*   @retval             -EPERM          Operation not allowed given the process permissions
*///-----------------------------------------------------------------------------------------------------------------
static int MsgqSet(int msqid, struct msqid_ds *buf)
{
    MessageQueue_t *q;

    // -- first remove the queue from the global list (cannot be found anymore)
    SpinLock(&msgMaster.head.lock); {
        q = GetQueueById(msqid);

        if (q == NULL) {
            SpinUnlock(&msgMaster.head.lock);
            return -EINVAL;
        }

        uid_t cuid = EffectiveUid();

        if (cuid != q->perm.cuid && cuid != q->perm.uid) {
            SpinUnlock(&msgMaster.head.lock);
            return -EPERM;
        }

        if (!MmuIsMapped((Addr_t)buf) || !MmuIsMapped((Addr_t)buf + sizeof(struct msqid_ds) - 1)) {
            SpinUnlock(&msgMaster.head.lock);
            return -EFAULT;
        }

        if (cuid != 0 && buf->msg_qbytes > q->msgmnb) {
            SpinUnlock(&msgMaster.head.lock);
            return -EPERM;
        }

        q->perm.uid = buf->msg_perm.uid;
        q->perm.gid = buf->msg_perm.gid;
        q->perm.mode = buf->msg_perm.mode & 0777;
        q->msgmnb = buf->msg_qbytes;

        time(&q->lctime);
    }

    return 0;
}



/****************************************************************************************************************//**
*   @fn                 int MsgqRmid(int msqid, struct msqid_ds *buf)
*   @brief              Handle removing the message queue status
*
*   This function is the internal implementation to remove a Message Queue.
*
*   @param              msqid           The key to use to uniquely identify a message queue
*   @param              buf             The \ref msqid_ds structure to be used with the operation
*
*   @returns            0 on success, non-zero on error
*
*   @retval             -EINVAL         Invalid `cmd` or invalid `msgqid`
*   @retval             -EPERM          Operation not allowed given the process permissions
*///-----------------------------------------------------------------------------------------------------------------
static int MsgqRmid(int msqid, struct msqid_ds *buf)
{
    MessageQueue_t *q;

    // -- first remove the queue from the global list (cannot be found anymore)
    SpinLock(&msgMaster.head.lock); {
        q = GetQueueById(msqid);

        if (q == NULL) {
            SpinUnlock(&msgMaster.head.lock);
            return -EINVAL;
        }

        uid_t cuid = EffectiveUid();

        if (cuid != q->perm.cuid && cuid != q->perm.uid) {
            SpinUnlock(&msgMaster.head.lock);
            return -EPERM;
        }

        //
        // -- this sequence then eliminates the race condition: anything working on the queue can
        //    complete before this is removed from the global list.  Stack of locks.
        //    -----------------------------------------------------------------------------------
        SpinLock(&q->head.lock); {
            ListRemoveInit(&q->list);
            SpinUnlock(&q->head.lock);
        }

        SpinUnlock(&msgMaster.head.lock);
    }

    // -- now we need to release all waiting processes (each will return -EIDRM)
    SchReadyList(&q->sendWait);
    SchReadyList(&q->recvWait);

    // -- Now we need to clean up all the pending messages
    ListHead_t::List_t *wrk = q->head.list.next;
    while (wrk != &q->head.list) {
        Message_t *m = FIND_PARENT(wrk, Message_t, list);
        ListRemoveInit(wrk);
        FREE(m);
        wrk = wrk->next;
    }

    // -- Finally, clean up the queue itself
    FREE(q);

    return 0;
}



/****************************************************************************************************************//**
*   @fn                 int msg_MsgqCtl(int msqid, int cmd, struct msqid_ds *buf)
*   @brief              Internal function to control a Message Queue
*
*   This function is the internal implementation to control a message queue.  This is the actual function
*   that will back the `msgctl` system function.
*
*   @param              msqid           The key to use to uniquely identify a message queue
*   @param              cmd             Flags to be used in getting the proper message queue
*   @param              buf             The \ref msqid_ds structure to be used with the operation
*
*   @returns            0 on success, non-zero on error
*
*   @retval             -EINVAL         Invalid `cmd` or invalid `msgqid`
*   @retval             -EACCES         Insufficient permissions
*   @retval             -EFAULT         The `buf` is not mapped
*   @retval             -EPERM          Operation not allowed given the process permissions
*///-----------------------------------------------------------------------------------------------------------------
extern "C" int msg_MsgqCtl(int msqid, int cmd, struct msqid_ds *buf)
{
    switch (cmd) {
    case IPC_STAT:
        return MsgqStat(msqid, buf);

    case IPC_SET:
        return MsgqSet(msqid, buf);

    case IPC_RMID:
        return MsgqRmid(msqid, buf);

    default:
        return -EINVAL;
    }
}



/****************************************************************************************************************//**
*   @fn                 int msg_MsgqSnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
*   @brief              Internal function to send a Message
*
*   This function is the internal implementation to send a message.  This is the actual function that will back
*   the `msgsnd` system function.
*
*   @param              msqid           The key to use to uniquely identify a message queue
*   @param              msgp            Pointer to the message to be sent; this parameter is in the general format
*                                       \code
*                                       struct {
*                                           long msgtyp;
*                                           char text[0];
*                                       }
*                                       \endcode
*   @param              msgsz           The length of text in the structure above
*   @param              msgflg          Flags to be used in getting the proper message queue
*
*   @returns            0 on success, non-zero on error
*
*   @retval             -EINVAL         `msgtyp` < 0 or `msgsz` > `msgmax`
*   @retval             -EINTR          The process was interrupted with a signal
*   @retval             -EIDRM          The queue was removed
*   @retval             -EACCES         Insufficient permissions
*   @retval             -EAGAIN         The process would have blocked but `IPC_NOWAIT` was specified
*   @retval             -ENOMEM         The internal message structures could not be allocated
*///-----------------------------------------------------------------------------------------------------------------
extern "C" int msg_MsgqSnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
{
    struct Msg {
        long type;
        char payload[0];
    };

    // -- first check the static components
    struct Msg *p = (struct Msg *)msgp;
    if (p->type < 0) return -EINVAL;
    if (msgsz > msgMaster.msgmax) {
        if (msgflg & MSG_NOERROR) {
            msgsz = msgMaster.msgmax;
        } else {
            return -EINVAL;
        }
    }

    // -- will loop indefinitely as any position can change while blocked
    while (true) {
        // -- here we check if we caught a signal
        if (false) return -EINTR;       //! @todo update the condition for a real signal check

        MessageQueue_t *q;

        SpinLock(&msgMaster.head.lock); {
            q = GetQueueById(msqid);

            if (q == NULL) {
                SpinUnlock(&msgMaster.head.lock);
                return -EIDRM;
            }

            uid_t cuid = EffectiveUid();
            gid_t cgid = EffectiveGid();

            if (cuid == q->perm.cuid || cuid == q->perm.uid) {
                if (q->perm.mode & 0200) goto more;
            }
            if (cgid == q->perm.cgid || cuid == q->perm.gid) {
                if (q->perm.mode & 0020) goto more;
            }
            if (q->perm.mode & 0002) goto more;

            SpinUnlock(&msgMaster.head.lock);
            return -EACCES;

more:
            // -- POSIX implies a system imposed limit for all messages in all queues; we have no such limit
            SpinLock(&q->head.lock); {
                SpinUnlock(&msgMaster.head.lock);

                if (msgsz > q->msgmnb - AtomicRead(&q->currentBytes) || AtomicRead(&q->msgCnt) > q->msgmnb) {
                    SpinUnlock(&q->head.lock);

                    if (msgflg & IPC_NOWAIT) {
                        return -EAGAIN;
                    } else {
                        // -- Here we block until we can continue
                        SchProcessBlock(PRC_MSGW, &q->sendWait);
                        continue;
                    }
                }

                Message_t *msg = (Message_t *)HeapAlloc(sizeof(Message_t) + msgsz, false);
                if (!msg) {
                    SpinUnlock(&q->head.lock);
                    return -ENOMEM;
                }

                kMemSetB(msg, 0, sizeof(Message_t));
                ListInit(&msg->list);

                msg->size = msgsz;
                msg->type = p->type;
                kMemMoveB(msg->text, p->payload, msgsz);
                ListAddTail(&q->head, &msg->list);

                q->lspid = CurrentThread()->pid;
                time(&q->lstime);
                AtomicAdd(&q->currentBytes, msgsz);
                AtomicInc(&q->msgCnt);

                SchReadyList(&q->sendWait);

                SpinUnlock(&q->head.lock);

                return 0;
            }
        }
    }
}



/****************************************************************************************************************//**
*   @fn                 Message_t *GetNextMessage(MessageQueue_t *q, long msgtyp)
*   @brief              Get the next message on the queue for the `msgtyp` requested
*
*   Reads through the queue of messages and returns the first message which meets the qualifications as indicated
*   by the `msgtyp` parameter:
*   * when == 0: get the first message on the queue
*   * when > 0: get the first message on the queue with the same `msgtyp` specified
*   * when < 0: get the first message on teh queue with a `msgtyp` <= the absolute value of `msgtyp`
*
*   @note               `q->lock` must be held by the calling function
*
*   @param              q               The queue to search
*   @param              msgtyp          The type of message to find
*
*   @returns            A pointer to the appropriate Message_t, or NULL if none is available
*///-----------------------------------------------------------------------------------------------------------------
Message_t *GetNextMessage(MessageQueue_t *q, long msgtyp)
{
    if (!AtomicRead(&q->msgCnt)) return NULL;

    ListHead_t::List_t *wrk = q->head.list.next;
    Message_t *rv;

    while (wrk != &q->head.list) {
        rv = FIND_PARENT(wrk, Message_t, list);
        if (msgtyp == 0) return rv;
        if (msgtyp > 0 && rv->type == msgtyp) return rv;
        if (msgtyp < 0 && rv->type <= -msgtyp) return rv;
        wrk = wrk->next;
    }

    return NULL;
}



/****************************************************************************************************************//**
*   @fn                 ssize_t msg_MsgqRcv(int msqid, const void *msgp, size_t msgsz, long msgtyp, int msgflg)
*   @brief              Internal function to receive a Message
*
*   This function is the internal implementation to receive a message.  This is the actual function that will back
*   the `msgrcv` system function.
*
*   @param              msqid           The key to use to uniquely identify a message queue
*   @param              msgp            Pointer to the message to be sent; this parameter is in the general format
*                                       \code
*                                       struct {
*                                           long msgtyp;
*                                           char text[0];
*                                       }
*                                       \endcode
*   @param              msgsz           The length of text in the structure above
*   @param              msgtyp          The type of message to retrieve.  When 0, retrieve the fist message; when
*                                       > 0, get the first message of that type; when < 0, get the first message
*                                       of type less than or equal to the absolute value of `msgtyp`.
*   @param              msgflg          Flags to be used in getting the proper message queue
*
*   @returns            The number of bytes of the message text, < 0 on error
*
*   @retval             -EINVAL         `msgtyp` < 0 or `msgsz` > `msgmax`
*   @retval             -EINTR          The process was interrupted with a signal
*   @retval             -EIDRM          The queue was removed
*   @retval             -EACCES         Insufficient permissions
*   @retval             -ENOMSG         The process would have blocked but `IPC_NOWAIT` was specified
*///-----------------------------------------------------------------------------------------------------------------
extern "C" ssize_t msg_MsgqRcv(int msqid, const void *msgp, size_t msgsz, long msgtyp, int msgflg)
{
    struct Msg {
        long type;
        char payload[0];
    };
    struct Msg *p = (struct Msg *)msgp;

    // -- will loop indefinitely
    while (true) {
        if (false) return -EINTR;       //! @todo update the condition for a real signal check

        MessageQueue_t *q;

        SpinLock(&msgMaster.head.lock); {
            q = GetQueueById(msqid);

            if (q == NULL) {
                SpinUnlock(&msgMaster.head.lock);
                return -EIDRM;
            }

            uid_t cuid = EffectiveUid();
            gid_t cgid = EffectiveGid();

            if (cuid == q->perm.cuid || cuid == q->perm.uid) {
                if (q->perm.mode & 0400) goto more;
            }
            if (cgid == q->perm.cgid || cuid == q->perm.gid) {
                if (q->perm.mode & 0040) goto more;
            }
            if (q->perm.mode & 0004) goto more;

            SpinUnlock(&msgMaster.head.lock);
            return -EACCES;

more:
            SpinLock(&q->head.lock); {
                SpinUnlock(&msgMaster.head.lock);

                Message_t *m = GetNextMessage(q, msgtyp);

                // -- if there are no messages, block or return
                if (!m) {
                    SpinUnlock(&q->head.lock);

                    if (msgflg & IPC_NOWAIT) {
                        return -ENOMSG;
                    } else {
                        SchProcessBlock(PRC_MSGW, &q->recvWait);
                        continue;
                    }
                }

                if (m->size > msgsz && !(msgflg & MSG_NOERROR)) {
                    SpinUnlock(&q->head.lock);
                    return -E2BIG;
                }

                if (m->size > msgsz) m->size = msgsz;

                p->type = m->type;
                kMemMoveB(p->payload, m->text, m->size);
                FREE(p);

                q->lrpid = CurrentThread()->pid;
                time(&q->lrtime);
                AtomicSub(&q->currentBytes, m->size);
                AtomicDec(&q->msgCnt);

                SchReadyList(&q->sendWait);

                SpinUnlock(&q->head.lock);

                return m->size;
            }
        }
    }
}



