/***********************************************************

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/


#ifndef OS_H
#define OS_H

#include "misc.h"
#include <stdarg.h>

#define NullFID ((FID) 0)

#define SCREEN_SAVER_ON   0
#define SCREEN_SAVER_OFF  1
#define SCREEN_SAVER_FORCER 2
#define SCREEN_SAVER_CYCLE  3

#ifndef MAX_REQUEST_SIZE
#define MAX_REQUEST_SIZE 65535
#endif
#ifndef MAX_BIG_REQUEST_SIZE
#define MAX_BIG_REQUEST_SIZE 4194303
#endif

typedef void *	FID;
typedef struct _FontPathRec *FontPathPtr;
typedef struct _NewClientRec *NewClientPtr;

#ifndef xnfalloc
#define xnfalloc(size) XNFalloc((unsigned long)(size))
#define xnfcalloc(_num, _size) XNFcalloc((unsigned long)(_num)*(unsigned long)(_size))
#define xnfrealloc(ptr, size) XNFrealloc((void *)(ptr), (unsigned long)(size))

#define xstrdup(s) Xstrdup(s)
#define xnfstrdup(s) XNFstrdup(s)

#define xallocarray(num, size) reallocarray(NULL, (num), (size))
#endif

#include <stdio.h>
#include <stdarg.h>

#ifdef DDXBEFORERESET
extern void ddxBeforeReset (void);
#endif

#ifdef DDXOSVERRORF
extern _X_EXPORT void (*OsVendorVErrorFProc)(const char *, va_list args);
#endif

extern _X_EXPORT int WaitForSomething(
    int* /*pClientsReady*/
);

extern _X_EXPORT int ReadRequestFromClient(ClientPtr /*client*/);

extern _X_EXPORT Bool InsertFakeRequest(
    ClientPtr /*client*/, 
    char* /*data*/, 
    int /*count*/);

extern _X_EXPORT void ResetCurrentRequest(ClientPtr /*client*/);

extern _X_EXPORT void FlushAllOutput(void);

extern _X_EXPORT void FlushIfCriticalOutputPending(void);

extern _X_EXPORT void SetCriticalOutputPending(void);

extern _X_EXPORT int WriteToClient(ClientPtr /*who*/, int /*count*/, const void* /*buf*/);

extern _X_EXPORT void ResetOsBuffers(void);

extern _X_EXPORT void InitConnectionLimits(void);

extern _X_EXPORT void NotifyParentProcess(void);

extern _X_EXPORT void CreateWellKnownSockets(void);

extern _X_EXPORT void ResetWellKnownSockets(void);

extern _X_EXPORT void CloseWellKnownConnections(void);

extern _X_EXPORT XID AuthorizationIDOfClient(ClientPtr /*client*/);

extern _X_EXPORT char *ClientAuthorized(
    ClientPtr /*client*/,
    unsigned int /*proto_n*/,
    char* /*auth_proto*/,
    unsigned int /*string_n*/,
    char* /*auth_string*/);

extern _X_EXPORT Bool EstablishNewConnections(
    ClientPtr /*clientUnused*/,
    void * /*closure*/);

extern _X_EXPORT void CheckConnections(void);

extern _X_EXPORT void CloseDownConnection(ClientPtr /*client*/);

typedef void (*NotifyFdProcPtr)(int fd, int ready, void *data);
extern _X_EXPORT void AddGeneralSocket(int /*fd*/);

#define X_NOTIFY_NONE   0
#define X_NOTIFY_READ   1
#define X_NOTIFY_WRITE  2
extern _X_EXPORT void RemoveGeneralSocket(int /*fd*/);

extern Bool SetNotifyFd(int fd, NotifyFdProcPtr notify_fd, int mask, void *data);
extern _X_EXPORT void AddEnabledDevice(int /*fd*/);

static inline void RemoveNotifyFd(int fd)
{
    (void) SetNotifyFd(fd, NULL, X_NOTIFY_NONE, NULL);
}
extern _X_EXPORT void RemoveEnabledDevice(int /*fd*/);

extern _X_EXPORT int OnlyListenToOneClient(ClientPtr /*client*/);

extern _X_EXPORT void ListenToAllClients(void);

extern _X_EXPORT void IgnoreClient(ClientPtr /*client*/);

extern _X_EXPORT void AttendClient(ClientPtr /*client*/);

extern _X_EXPORT void MakeClientGrabImpervious(ClientPtr /*client*/);

extern _X_EXPORT void MakeClientGrabPervious(ClientPtr /*client*/);

extern void AvailableClientInput(ClientPtr /* client */);

extern _X_EXPORT CARD32 GetTimeInMillis(void);

extern _X_EXPORT void AdjustWaitForDelay(
    void * /*waitTime*/,
    unsigned long /*newdelay*/);

typedef	struct _OsTimerRec *OsTimerPtr;

typedef CARD32 (*OsTimerCallback)(
    OsTimerPtr /* timer */,
    CARD32 /* time */,
    void * /* arg */);

extern _X_EXPORT void TimerInit(void);

extern _X_EXPORT Bool TimerForce(OsTimerPtr /* timer */);

#define TimerAbsolute (1<<0)
#define TimerForceOld (1<<1)

extern _X_EXPORT OsTimerPtr TimerSet(
    OsTimerPtr /* timer */,
    int /* flags */,
    CARD32 /* millis */,
    OsTimerCallback /* func */,
    void * /* arg */);

extern _X_EXPORT void TimerCheck(void);
extern _X_EXPORT void TimerCancel(OsTimerPtr /* pTimer */);
extern _X_EXPORT void TimerFree(OsTimerPtr /* pTimer */);

extern _X_EXPORT void SetScreenSaverTimer(void);
extern _X_EXPORT void FreeScreenSaverTimer(void);

extern _X_EXPORT void AutoResetServer(int /*sig*/);

extern _X_EXPORT void GiveUp(int /*sig*/);

extern _X_EXPORT void UseMsg(void);

extern _X_EXPORT void ProcessCommandLine(int /*argc*/, char* /*argv*/[]);

extern _X_EXPORT int set_font_authorizations(
    char ** /* authorizations */, 
    int * /*authlen */, 
    void * /* client */);

/*
 * This function malloc(3)s buffer, terminating the server if there is not
 * enough memory.
 */
extern _X_EXPORT void *XNFalloc(unsigned long /*amount*/);
/*
 * This function calloc(3)s buffer, terminating the server if there is not
 * enough memory.
 */
extern _X_EXPORT void *XNFcalloc(unsigned long /*amount*/);
/*
 * This function realloc(3)s passed buffer, terminating the server if there is
 * not enough memory.
 */
extern _X_EXPORT void *XNFrealloc(void * /*ptr*/, unsigned long /*amount*/);

/*
 * This function strdup(3)s passed string. The only difference from the library
 * function that it is safe to pass NULL, as NULL will be returned.
 */
extern _X_EXPORT char *Xstrdup(const char *s);

/*
 * This function strdup(3)s passed string, terminating the server if there is
 * not enough memory. If NULL is passed to this function, NULL is returned.
 */
extern _X_EXPORT char *XNFstrdup(const char *s);

extern _X_EXPORT char *Xprintf(const char *fmt, ...) _X_ATTRIBUTE_PRINTF(1,2);
extern _X_EXPORT char *Xvprintf(const char *fmt, va_list va);
extern _X_EXPORT char *XNFprintf(const char *fmt, ...) _X_ATTRIBUTE_PRINTF(1,2);
extern _X_EXPORT char *XNFvprintf(const char *fmt, va_list va);

typedef void (*OsSigHandlerPtr)(int /* sig */);
typedef int (*OsSigWrapperPtr)(int /* sig */);

extern _X_EXPORT OsSigHandlerPtr OsSignal(int /* sig */, OsSigHandlerPtr /* handler */);
extern _X_EXPORT OsSigWrapperPtr OsRegisterSigWrapper(OsSigWrapperPtr newWrap);

extern _X_EXPORT int auditTrailLevel;

extern _X_EXPORT void LockServer(void);
extern _X_EXPORT void UnlockServer(void);

extern _X_EXPORT int OsLookupColor(
    int	/*screen*/,
    char * /*name*/,
    unsigned /*len*/,
    unsigned short * /*pred*/,
    unsigned short * /*pgreen*/,
    unsigned short * /*pblue*/);

extern _X_EXPORT void OsInit(void);

extern _X_EXPORT void OsCleanup(Bool);

extern _X_EXPORT void OsVendorFatalError(void);

extern _X_EXPORT void OsVendorInit(void);

extern _X_EXPORT void OsBlockSignals (void);

extern _X_EXPORT void OsReleaseSignals (void);

extern _X_EXPORT void OsAbort (void) _X_NORETURN;

extern _X_EXPORT int System(char *);
extern _X_EXPORT void * Popen(char *, char *);
extern _X_EXPORT int Pclose(void *);
extern _X_EXPORT void * Fopen(char *, char *);
extern _X_EXPORT int Fclose(void *);

extern _X_EXPORT void CheckUserParameters(int argc, char **argv, char **envp);
extern _X_EXPORT void CheckUserAuthorization(void);

extern _X_EXPORT int AddHost(
    ClientPtr	/*client*/,
    int         /*family*/,
    unsigned    /*length*/,
    const void */*pAddr*/);

extern _X_EXPORT Bool ForEachHostInFamily (
    int	    /*family*/,
    Bool    (* /*func*/ )(
            unsigned char * /* addr */,
            short           /* len */,
            void *         /* closure */),
    void * /*closure*/);

extern _X_EXPORT int RemoveHost(
    ClientPtr	/*client*/,
    int         /*family*/,
    unsigned    /*length*/,
    void *     /*pAddr*/);

extern _X_EXPORT int GetHosts(
    void ** /*data*/,
    int	    * /*pnHosts*/,
    int	    * /*pLen*/,
    BOOL    * /*pEnabled*/);

typedef struct sockaddr * sockaddrPtr;

extern _X_EXPORT int InvalidHost(sockaddrPtr /*saddr*/, int /*len*/, ClientPtr client);

extern _X_EXPORT int LocalClient(ClientPtr /* client */);

extern _X_EXPORT int LocalClientCred(ClientPtr, int *, int *);

#define LCC_UID_SET    (1 << 0)
#define LCC_GID_SET    (1 << 1)
#define LCC_PID_SET    (1 << 2)
#define LCC_ZID_SET    (1 << 3)

typedef struct {
    int fieldsSet;     /* Bit mask of fields set */
    int        euid;           /* Effective uid */
    int egid;          /* Primary effective group id */
    int nSuppGids;     /* Number of supplementary group ids */
    int *pSuppGids;    /* Array of supplementary group ids */
    int pid;           /* Process id */
    int zoneid;                /* Only set on Solaris 10 & later */
} LocalClientCredRec;

extern _X_EXPORT int GetLocalClientCreds(ClientPtr, LocalClientCredRec **);
extern _X_EXPORT void FreeLocalClientCreds(LocalClientCredRec *);

extern _X_EXPORT int ChangeAccessControl(ClientPtr /*client*/, int /*fEnabled*/);

extern _X_EXPORT int GetAccessControl(void);


extern _X_EXPORT void AddLocalHosts(void);

extern _X_EXPORT void ResetHosts(char *display);

extern _X_EXPORT void EnableLocalHost(void);

extern _X_EXPORT void DisableLocalHost(void);

extern _X_EXPORT void AccessUsingXdmcp(void);

extern _X_EXPORT void DefineSelf(int /*fd*/);

extern _X_EXPORT void AugmentSelf(void * /*from*/, int /*len*/);

extern _X_EXPORT void RegisterAuthorizations(void);

extern _X_EXPORT void InitAuthorization(char * /*filename*/);

/* extern int LoadAuthorization(void); */

extern _X_EXPORT int AuthorizationFromID (
	XID 		id,
	unsigned short	*name_lenp,
	char		**namep,
	unsigned short	*data_lenp,
	char		**datap);

extern _X_EXPORT XID CheckAuthorization(
    unsigned int /*namelength*/,
    const char * /*name*/,
    unsigned int /*datalength*/,
    const char * /*data*/,
    ClientPtr /*client*/,
    char ** /*reason*/
);

extern _X_EXPORT void ResetAuthorization(void);

extern _X_EXPORT int RemoveAuthorization (
    unsigned short	name_length,
    const char		*name,
    unsigned short	data_length,
    const char		*data);

extern _X_EXPORT int AddAuthorization(
    unsigned int	/*name_length*/,
    const char *	/*name*/,
    unsigned int	/*data_length*/,
    char *		/*data*/);

extern _X_EXPORT XID GenerateAuthorization(
    unsigned int   /* name_length */,
    const char	*  /* name */,
    unsigned int   /* data_length */,
    const char	*  /* data */,
    unsigned int * /* data_length_return */,
    char	** /* data_return */);

extern void ddxInitGlobals(void);

extern _X_EXPORT int ddxProcessArgument(int /*argc*/, char * /*argv*/ [], int /*i*/);

extern _X_EXPORT void ddxUseMsg(void);

/* int ReqLen(xReq *req, ClientPtr client)
 * Given a pointer to a *complete* request, return its length in bytes.
 * Note that if the request is a big request (as defined in the Big
 * Requests extension), the macro lies by returning 4 less than the
 * length that it actually occupies in the request buffer.  This is so you
 * can blindly compare the length with the various sz_<request> constants
 * in Xproto.h without having to know/care about big requests.
 */
#define ReqLen(_pxReq, _client) \
 ((_pxReq->length ? \
     (_client->swapped ? lswaps(_pxReq->length) : _pxReq->length) \
  : ((_client->swapped ? \
	lswapl(((CARD32*)_pxReq)[1]) : ((CARD32*)_pxReq)[1])-1) \
  ) << 2)

/* otherReqTypePtr CastxReq(xReq *req, otherReqTypePtr)
 * Cast the given request to one of type otherReqTypePtr to access
 * fields beyond the length field.
 */
#define CastxReq(_pxReq, otherReqTypePtr) \
    (_pxReq->length ? (otherReqTypePtr)_pxReq \
		    : (otherReqTypePtr)(((CARD32*)_pxReq)+1))

/* stuff for SkippedRequestsCallback */
extern CallbackListPtr SkippedRequestsCallback;
typedef struct {
    xReqPtr req;
    ClientPtr client;
    int numskipped;
} SkippedRequestInfoRec;

/* stuff for ReplyCallback */
extern _X_EXPORT CallbackListPtr ReplyCallback;
typedef struct {
    ClientPtr client;
    const void * replyData;
    unsigned long dataLenBytes;
    unsigned long bytesRemaining;
    Bool startOfReply;
} ReplyInfoRec;

/* stuff for FlushCallback */
extern _X_EXPORT CallbackListPtr FlushCallback;

extern _X_EXPORT void AbortDDX(void);
extern _X_EXPORT void ddxGiveUp(void);
extern _X_EXPORT int TimeSinceLastInputEvent(void);

#ifndef HAVE_REALLOCARRAY
#define reallocarray xreallocarray
extern _X_EXPORT void *
reallocarray(void *optr, size_t nmemb, size_t size);
#endif

#ifndef HAVE_STRLCPY
extern _X_EXPORT size_t
strlcpy(char *dst, const char *src, size_t siz);
extern _X_EXPORT size_t
strlcat(char *dst, const char *src, size_t siz);
#endif

#ifndef HAVE_TIMINGSAFE_MEMCMP
extern _X_EXPORT int
timingsafe_memcmp(const void *b1, const void *b2, size_t len);
#endif

/* Logging. */
typedef enum _LogParameter {
    XLOG_FLUSH,
    XLOG_SYNC,
    XLOG_VERBOSITY,
    XLOG_FILE_VERBOSITY
} LogParameter;

/* Flags for log messages. */
typedef enum {
    X_PROBED,			/* Value was probed */
    X_CONFIG,			/* Value was given in the config file */
    X_DEFAULT,			/* Value is a default */
    X_CMDLINE,			/* Value was given on the command line */
    X_NOTICE,			/* Notice */
    X_ERROR,			/* Error message */
    X_WARNING,			/* Warning message */
    X_INFO,			/* Informational message */
    X_NONE,			/* No prefix */
    X_NOT_IMPLEMENTED,		/* Not implemented */
    X_UNKNOWN = -1		/* unknown -- this must always be last */
} MessageType;

/* XXX Need to check which GCC versions have the format(printf) attribute. */
#if defined(__GNUC__) && \
    ((__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ > 4)))
#define _printf_attribute(a,b) __attribute((format(__printf__,a,b)))
#else
#define _printf_attribute(a,b) /**/
#endif

extern _X_EXPORT const char *LogInit(const char *fname, const char *backup);
extern _X_EXPORT void LogSetDisplay(void);
extern _X_EXPORT const char *LogInit(const char *fname, const char *backup);
extern _X_EXPORT void LogClose(void);
extern _X_EXPORT Bool LogSetParameter(LogParameter param, int value);
extern _X_EXPORT void LogVWrite(int verb, const char *f, va_list args);
extern _X_EXPORT void LogWrite(int verb, const char *f, ...) _printf_attribute(2,3);
extern _X_EXPORT void LogVMessageVerb(MessageType type, int verb, const char *format,
			    va_list args);
extern _X_EXPORT void LogMessageVerb(MessageType type, int verb, const char *format,
			             ...) _printf_attribute(3,4);
extern _X_EXPORT void LogMessage(MessageType type, const char *format, ...)
			         _printf_attribute(2,3);
extern _X_EXPORT void FreeAuditTimer(void);
extern _X_EXPORT void AuditF(const char *f, ...) _printf_attribute(1,2);
extern _X_EXPORT void VAuditF(const char *f, va_list args);
extern _X_EXPORT void FatalError(const char *f, ...) _printf_attribute(1,2)
#if defined(__GNUC__) && \
    ((__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ > 4)))
__attribute((noreturn))
#endif
;

#ifdef DEBUG
#define DebugF ErrorF
#else
#define DebugF(...)             /* */
#endif

extern _X_EXPORT void VErrorF(const char *f, va_list args);
extern _X_EXPORT void ErrorF(const char *f, ...) _printf_attribute(1,2);
extern _X_EXPORT void Error(char *str);
extern _X_EXPORT void LogPrintMarkers(void);

extern void xorg_backtrace(void);

#endif /* OS_H */
