/**************************************************************************/
/*                                                                        */
/* Copyright (c) 2001, 2011 NoMachine (http://www.nomachine.com)          */
/* Copyright (c) 2008-2014 Oleksandr Shneyder <o.shneyder@phoca-gmbh.de>  */
/* Copyright (c) 2011-2016 Mike Gabriel <mike.gabriel@das-netzwerkteam.de>*/
/* Copyright (c) 2014-2016 Mihai Moldovan <ionic@ionic.de>                */
/* Copyright (c) 2014-2016 Ulrich Sibiller <uli42@gmx.de>                 */
/* Copyright (c) 2015-2016 Qindel Group (http://www.qindel.com)           */
/*                                                                        */
/* NXAGENT, NX protocol compression and NX extensions to this software    */
/* are copyright of the aforementioned persons and companies.             */
/*                                                                        */
/* Redistribution and use of the present software is allowed according    */
/* to terms specified in the file LICENSE which comes in the source       */
/* distribution.                                                          */
/*                                                                        */
/* All rights reserved.                                                   */
/*                                                                        */
/* NOTE: This software has received contributions from various other      */
/* contributors, only the core maintainers and supporters are listed as   */
/* copyright holders. Please contact us, if you feel you should be listed */
/* as copyright holder, as well.                                          */
/*                                                                        */
/**************************************************************************/

#include "X.h"
#include "Xproto.h"
#include "Xatom.h"
#include "selection.h"
#include "windowstr.h"
#include "scrnintstr.h"

#include "Windows.h"
#include "Atoms.h"
#include "Agent.h"
#include "Args.h"
#include "Trap.h"
#include "Rootless.h"
#include "Clipboard.h"
#include "Utils.h"

#include "gcstruct.h"
#include "xfixeswire.h"
#include "X11/include/Xfixes_nxagent.h"

/*
 * Use asynchronous get property replies.
 */

#include "compext/Compext.h"

/*
 * Set here the required log level.
 */

#define PANIC
#define WARNING
#undef  TEST
#undef  DEBUG

/*
 * These are defined in the dispatcher.
 */

extern int NumCurrentSelections;
extern Selection *CurrentSelections;

int nxagentLastClipboardClient = -1;

static int agentClipboardStatus;
static int clientAccum;

Atom serverCutProperty;
Atom clientCutProperty;
static Window serverWindow;

const int nxagentPrimarySelection = 0;
const int nxagentClipboardSelection = 1;
const int nxagentMaxSelections = 2;

typedef struct _SelectionOwner
{
    Atom selection;
    ClientPtr client;
    Window window;
    WindowPtr windowPtr;
    Time lastTimeChanged;

} SelectionOwner;

static SelectionOwner *lastSelectionOwner;
static Atom nxagentLastRequestedSelection;
static Atom nxagentClipboardAtom;
static Atom nxagentTimestampAtom;

/*
 * Needed to handle the notify selection event to
 * be sent to client once the selection property
 * has been retrieved from the real X server.
 */

typedef enum
{
  SelectionStageNone,
  SelectionStageQuerySize,
  SelectionStageWaitSize,
  SelectionStageQueryData,
  SelectionStageWaitData
} ClientSelectionStage;

static WindowPtr     lastClientWindowPtr;
static ClientPtr     lastClientClientPtr;
static Window        lastClientRequestor;
static Atom          lastClientProperty;
static Atom          lastClientSelection;
static Atom          lastClientTarget;
static Time          lastClientTime;
static Time          lastClientReqTime;
static unsigned long lastClientPropertySize;

static ClientSelectionStage lastClientStage;

static Window lastServerRequestor;
static Atom   lastServerProperty;
static Atom   lastServerTarget;
static Time   lastServerTime;

static Atom serverTARGETS;
static Atom serverTEXT;
static Atom serverUTF8_STRING;
static Atom clientTARGETS;
static Atom clientTEXT;
static Atom clientCOMPOUND_TEXT;
static Atom clientUTF8_STRING;

static char szAgentTARGETS[] = "TARGETS";
static char szAgentTEXT[] = "TEXT";
static char szAgentCOMPOUND_TEXT[] = "COMPOUND_TEXT";
static char szAgentUTF8_STRING[] = "UTF8_STRING";
static char szAgentNX_CUT_BUFFER_CLIENT[] = "NX_CUT_BUFFER_CLIENT";

/*
 * some helpers for debugging output
 */

#ifdef DEBUG
const char * GetClientSelectionStageString(int stage)
{
  switch(stage)
  {
    case SelectionStageNone:      return("None"); break;;
    case SelectionStageQuerySize: return("QuerySize"); break;;
    case SelectionStageWaitSize:  return("WaitSize"); break;;
    case SelectionStageQueryData: return("QueryData"); break;;
    case SelectionStageWaitData:  return("WaitData"); break;;
    default:                      return("UNKNOWN!"); break;;
  }
}
#define SetClientSelectionStage(stage) do {fprintf(stderr, "%s: Changing selection stage from [%s] to [%s]\n", __func__, GetClientSelectionStageString(lastClientStage), GetClientSelectionStageString(SelectionStage##stage)); lastClientStage = SelectionStage##stage;} while (0)
#define PrintClientSelectionStage() do {fprintf(stderr, "%s: Current selection stage [%s]\n", __func__, GetClientSelectionStageString(lastClientStage));} while (0)
#else
#define SetClientSelectionStage(stage) do {lastClientStage = SelectionStage##stage;} while (0)
#define PrintClientSelectionStage()
#endif

/*
 * see also nx-X11/lib/src/ErrDes.c
 */
const char * GetXErrorString(int code)
{
  switch(code)
  {
    case Success:           return("Success"); break;;
    case BadRequest:        return("BadRequest"); break;;
    case BadValue:          return("BadValue"); break;;
    case BadWindow:         return("BadWindow"); break;;
    case BadPixmap:         return("BadPixmap"); break;;
    case BadAtom:           return("BadAtom"); break;;
    case BadCursor:         return("BadCursor"); break;;
    case BadFont:           return("BadFont"); break;;
    case BadMatch:          return("BadMatch"); break;;
    case BadDrawable:       return("BadDrawable"); break;;
    case BadAccess:         return("BadAccess"); break;;
    case BadAlloc:          return("BadAlloc"); break;;
    case BadColor:          return("BadColor"); break;;
    case BadGC:             return("BadGC"); break;;
    case BadIDChoice:       return("BadIDChoice"); break;;
    case BadName:           return("BadName"); break;;
    case BadLength:         return("BadLength"); break;;
    case BadImplementation: return("BadImplementation"); break;;
    default:                return("UNKNOWN!"); break;;
  }
}

/*
 * Save the values queried from X server.
 */

XFixesAgentInfoRec nxagentXFixesInfo = { -1, -1, -1, 0 };

extern Display *nxagentDisplay;

Bool nxagentValidServerTargets(Atom target);
void nxagentSendSelectionNotify(Atom property);
void nxagentTransferSelection(int resource);
void nxagentCollectPropertyEvent(int resource);
void nxagentResetSelectionOwner(void);
WindowPtr nxagentGetClipboardWindow(Atom property, WindowPtr pWin);
void nxagentNotifyConvertFailure(ClientPtr client, Window requestor,
                                     Atom selection, Atom target, Time time);
int nxagentSendNotify(xEvent *event);

void nxagentPrintClipboardStat(char *);

#ifdef DEBUG
void nxagentPrintSelectionStat(int sel)
{
  SelectionOwner lOwner = lastSelectionOwner[sel];
  Selection curSel = CurrentSelections[sel];
  char *s = NULL;

#ifdef CLIENTIDS
  fprintf(stderr, "  lastSelectionOwner[%d].client           [%p] index [%d] PID [%d] Cmd [%s %s]\n",
          sel,
          (void *)lOwner.client,
          lOwner.client ? lOwner.client->index : -1,
          GetClientPid(lOwner.client),
          GetClientCmdName(lOwner.client),
          GetClientCmdArgs(lOwner.client));
#else
  fprintf(stderr, "  lastSelectionOwner[%d].client           [%p] index [%d]\n",
          sel,
          (void *)lOwner.client,
          lOwner.client ? lOwner.client->index : -1);
#endif
  fprintf(stderr, "  lastSelectionOwner[%d].window           [0x%x]\n", sel, lOwner.window);
  fprintf(stderr, "  lastSelectionOwner[%d].windowPtr        [%p]\n", sel, (void *)lOwner.windowPtr);
  fprintf(stderr, "  lastSelectionOwner[%d].lastTimeChanged  [%u]\n", sel, lOwner.lastTimeChanged);

  /*
    print the selection name.
  */
  if (lOwner.client)
  {
    fprintf(stderr, "  lastSelectionOwner[%d].selection        [% 4d][%s] (local)\n", sel, lOwner.selection, NameForAtom(lOwner.selection));
  }
  else
  {
    SAFE_XFree(s); s = XGetAtomName(nxagentDisplay, lOwner.selection);
    fprintf(stderr, "  lastSelectionOwner[%d].selection        [% 4d][%s] (remote)\n", sel, lOwner.selection, validateString(s));
    SAFE_XFree(s);
  }
#ifdef CLIENTIDS
  fprintf(stderr, "  CurrentSelections[%s].client            [%p] index [%d] PID [%d] Cmd [%s %s]\n",
          sel,
          (void *)curSel.client,
          curSel.client ? curSel.client->index : -1,
          GetClientPid(curSel.client),
          GetClientCmdName(curSel.client),
          GetClientCmdArgs(curSel.client));
#else
  fprintf(stderr, "  CurrentSelections[%d].client            [%p] index [%d]\n",
          sel,
          (void *)curSel.client,
          curSel.client ? curSel.client->index : -1);
#endif
  fprintf(stderr, "  CurrentSelections[%d].window            [0x%x]\n", sel, curSel.window);
  return;
}
#endif

void nxagentPrintClipboardStat(char *header)
{
  #ifdef DEBUG
  char *s = NULL;

  fprintf(stderr, "/----- Clipboard internal status - %s -----\n", header);

  fprintf(stderr, "  current time                    (Time) [%u]\n", GetTimeInMillis());
  fprintf(stderr, "  agentClipboardStatus             (int) [%d]\n", agentClipboardStatus);
  fprintf(stderr, "  clientAccum                      (int) [%d]\n", clientAccum);
  fprintf(stderr, "  nxagentMaxSelections             (int) [%d]\n", nxagentMaxSelections);
  fprintf(stderr, "  NumCurrentSelections             (int) [%d]\n", NumCurrentSelections);
  fprintf(stderr, "  serverWindow                  (Window) [0x%x]\n", serverWindow);
  fprintf(stderr, "  nxagentLastClipboardClient       (int) [%d]\n", nxagentLastClipboardClient);

  fprintf(stderr, "  ClipboardMode                          ");
  switch(nxagentOption(Clipboard))
  {
    case ClipboardBoth:    fprintf(stderr, "[Both]"); break;;
    case ClipboardClient:  fprintf(stderr, "[Client]"); break;;
    case ClipboardServer:  fprintf(stderr, "[Server]"); break;;
    case ClipboardNone:    fprintf(stderr, "[None]"); break;;
    default:               fprintf(stderr, "[UNKNOWN] (FAIL!)"); break;;
  }
  fprintf(stderr,"\n");

  fprintf(stderr, "lastServer\n");
  fprintf(stderr, "  lastServerRequestor           (Window) [0x%x]\n", lastServerRequestor);
  SAFE_XFree(s); s = XGetAtomName(nxagentDisplay, lastServerProperty);
  fprintf(stderr, "  lastServerProperty              (Atom) [% 4d][%s]\n", lastServerProperty, s);
  SAFE_XFree(s); s = XGetAtomName(nxagentDisplay, lastServerTarget);
  fprintf(stderr, "  lastServerTarget                (Atom) [% 4d][%s]\n", lastServerTarget, s);
  fprintf(stderr, "  lastServerTime                  (Time) [%u]\n", lastServerTime);

  fprintf(stderr, "lastClient\n");
  fprintf(stderr, "  lastClientWindowPtr        (WindowPtr) [%p]\n", (void *)lastClientWindowPtr);
  fprintf(stderr, "  lastClientClientPtr        (ClientPtr) [%p]\n", (void *)lastClientClientPtr);
  fprintf(stderr, "  lastClientRequestor           (Window) [0x%x]\n", lastClientRequestor);
  fprintf(stderr, "  lastClientProperty              (Atom) [% 4d][%s]\n", lastClientProperty, NameForAtom(lastClientProperty));
  fprintf(stderr, "  lastClientSelection             (Atom) [% 4d][%s]\n", lastClientSelection, NameForAtom(lastClientSelection));
  fprintf(stderr, "  lastClientTarget                (Atom) [% 4d][%s]\n", lastClientTarget, NameForAtom(lastClientTarget));
  fprintf(stderr, "  lastClientTime                  (Time) [%u]\n", lastClientTime);
  fprintf(stderr, "  lastClientReqTime               (Time) [%u]\n", lastClientReqTime);
  fprintf(stderr, "  lastClientPropertySize (unsigned long) [%lu]\n", lastClientPropertySize);
  fprintf(stderr, "  lastClientStage (ClientSelectionStage) [%d][%s]\n", lastClientStage, GetClientSelectionStageString(lastClientStage));

  fprintf(stderr, "PRIMARY\n");
  nxagentPrintSelectionStat(nxagentPrimarySelection);
  fprintf(stderr, "CLIPBOARD\n");
  nxagentPrintSelectionStat(nxagentClipboardSelection);

  fprintf(stderr, "Atoms (server side)\n");
  SAFE_XFree(s); s = XGetAtomName(nxagentDisplay, serverTARGETS);
  fprintf(stderr, "  serverTARGETS                          [% 4d][%s]\n", serverTARGETS, validateString(s));
  SAFE_XFree(s); s = XGetAtomName(nxagentDisplay, serverTEXT);
  fprintf(stderr, "  serverTEXT                             [% d][%s]\n", serverTEXT, s);
  SAFE_XFree(s); s = XGetAtomName(nxagentDisplay, serverUTF8_STRING);
  fprintf(stderr, "  serverUTF8_STRING                      [% 4d][%s]\n", serverUTF8_STRING, s);
  SAFE_XFree(s); s = XGetAtomName(nxagentDisplay, serverCutProperty);
  fprintf(stderr, "  serverCutProperty                      [% 4d][%s]\n", serverCutProperty, s);

  SAFE_XFree(s); s = XGetAtomName(nxagentDisplay, nxagentClipboardAtom);
  fprintf(stderr, "  nxagentClipboardAtom                   [% 4d][%s]\n", nxagentClipboardAtom, s);
  SAFE_XFree(s); s = XGetAtomName(nxagentDisplay, nxagentTimestampAtom);
  fprintf(stderr, "  nxagentTimestampAtom                   [% 4d][%s]\n", nxagentTimestampAtom, s);

  fprintf(stderr, "Atoms (inside nxagent)\n");
  fprintf(stderr, "  clientTARGETS                          [% 4d][%s]\n", clientTARGETS, NameForAtom(clientTARGETS));
  fprintf(stderr, "  clientTEXT                             [% 4d][%s]\n", clientTEXT, NameForAtom(clientTEXT));
  fprintf(stderr, "  clientCOMPOUND_TEXT                    [% 4d][%s]\n", clientCOMPOUND_TEXT, NameForAtom(clientCOMPOUND_TEXT));
  fprintf(stderr, "  clientUTF8_STRING                      [% 4d][%s]\n", clientUTF8_STRING, NameForAtom(clientUTF8_STRING));
  fprintf(stderr, "  clientCutProperty                      [% 4d][%s]\n", clientCutProperty, NameForAtom(clientCutProperty));
  fprintf(stderr, "  nxagentLastRequestedSelection          [% 4d][%s]\n", nxagentLastRequestedSelection, NameForAtom(nxagentLastRequestedSelection));

  fprintf(stderr, "\\------------------------------------------------------------------------------\n");

  SAFE_XFree(s);
#endif
}

/*
 * This is from NXproperty.c.
 */

int GetWindowProperty(WindowPtr pWin, Atom property, long longOffset, long longLength,
                          Bool delete, Atom type, Atom *actualType, int *format,
                              unsigned long *nItems, unsigned long *bytesAfter,
                                  unsigned char **propData);

Bool nxagentValidServerTargets(Atom target)
{
  if (target == XA_STRING)
  {
    #ifdef DEBUG
    fprintf(stderr, "%s: valid target [XA_STRING].\n", __func__);
    #endif
    return True;
  }
  else if (target == serverTEXT)
  {
    #ifdef DEBUG
    fprintf(stderr, "%s: valid target [TEXT].\n", __func__);
    #endif
    return True;
  }
  /* by dimbor */
  else if (target == serverUTF8_STRING)
  {
    #ifdef DEBUG
    fprintf(stderr, "%s: valid target [UTF8_STRING].\n", __func__);
    #endif
    return True;
  }
  else if (target == serverTARGETS)
  {
    #ifdef DEBUG
    fprintf(stderr, "%s: special target [TARGETS].\n", __func__);
    #endif
    return False;
  }

  #ifdef DEBUG
  fprintf(stderr, "%s: invalid target [%u].\n", __func__, target);
  #endif
  return False;
}

void nxagentClearClipboard(ClientPtr pClient, WindowPtr pWindow)
{
  #ifdef DEBUG
  fprintf(stderr, "%s: Called with client [%p] window [%p].\n", __func__,
              (void *) pClient, (void *) pWindow);
  #endif

  nxagentPrintClipboardStat("before nxagentClearClipboard");

  /*
   * Only for PRIMARY and CLIPBOARD selections.
   */

  for (int i = 0; i < nxagentMaxSelections; i++)
  {
    if ((pClient != NULL && lastSelectionOwner[i].client == pClient) ||
            (pWindow != NULL && lastSelectionOwner[i].windowPtr == pWindow))
    {
      #ifdef TEST
      fprintf(stderr, "%s: Resetting state with client [%p] window [%p].\n", __func__,
                  (void *) pClient, (void *) pWindow);
      #endif

      lastSelectionOwner[i].client = NULL;
      lastSelectionOwner[i].window = None;
      lastSelectionOwner[i].windowPtr = NULL;
      lastSelectionOwner[i].lastTimeChanged = GetTimeInMillis();

      lastClientWindowPtr = NULL;
      SetClientSelectionStage(None);

      lastServerRequestor = None;
    }
  }

  if (pWindow == lastClientWindowPtr)
  {
    lastClientWindowPtr = NULL;
    SetClientSelectionStage(None);
  }

  nxagentPrintClipboardStat("after nxagentClearClipboard");
}

void nxagentClearSelection(XEvent *X)
{
  int i = 0;

  #ifdef DEBUG
  fprintf(stderr, "%s: SelectionClear event for selection [%lu].\n", __func__, X->xselectionclear.selection);
  #endif

  nxagentPrintClipboardStat("before nxagentClearSelection");

  if (agentClipboardStatus != 1 ||
          nxagentOption(Clipboard) == ClipboardServer)
  {
    return;
  }

  while ((i < nxagentMaxSelections) &&
            (lastSelectionOwner[i].selection != X->xselectionclear.selection))
  {
    i++;
  }

  if (i < nxagentMaxSelections)
  {
    if (lastSelectionOwner[i].client != NULL)
    {
      xEvent x;
      memset(&x, 0, sizeof(xEvent));
      x.u.u.type = SelectionClear;
      x.u.selectionClear.time = GetTimeInMillis();
      x.u.selectionClear.window = lastSelectionOwner[i].window;
      x.u.selectionClear.atom = CurrentSelections[i].selection;

      (void) TryClientEvents(lastSelectionOwner[i].client, &x, 1,
                             NoEventMask, NoEventMask,
                             NullGrab);
    }

    CurrentSelections[i].window = screenInfo.screens[0]->root->drawable.id;
    CurrentSelections[i].client = NullClient;

    lastSelectionOwner[i].client = NULL;
    lastSelectionOwner[i].window = None;
    lastSelectionOwner[i].lastTimeChanged = GetTimeInMillis();
  }

  lastClientWindowPtr = NULL;
  SetClientSelectionStage(None);
  nxagentPrintClipboardStat("after nxagentClearSelection");
}

void nxagentRequestSelection(XEvent *X)
{
  int i = 0;
  XSelectionEvent eventSelection = {0};

  #ifdef DEBUG
  fprintf(stderr, "%s: Got called.\n", __func__);
  #endif

  nxagentPrintClipboardStat("before nxagentRequestSelection");

  if (agentClipboardStatus != 1)
  {
    return;
  }

  if (!nxagentValidServerTargets(X->xselectionrequest.target) ||
         (lastServerRequestor != None) ||
             ((X->xselectionrequest.selection != lastSelectionOwner[nxagentPrimarySelection].selection) &&
                 (X->xselectionrequest.selection != lastSelectionOwner[nxagentClipboardSelection].selection)))
  {
/*
FIXME: Do we need this?

    char *strTarget;

    strTarget = XGetAtomName(nxagentDisplay, X->xselectionrequest.target);

    fprintf(stderr, "SelectionRequest event aborting sele=[%s] target=[%s]\n",
                validateString(NameForAtom(X->xselectionrequest.selection)),
                    validateString(NameForAtom(X->xselectionrequest.target)));

    fprintf(stderr, "SelectionRequest event aborting sele=[%s] ext target=[%s] Atom size is [%d]\n",
                validateString(NameForAtom(X->xselectionrequest.selection)), strTarget, sizeof(Atom));

    SAFE_XFree(strTarget);
*/
    memset(&eventSelection, 0, sizeof(XSelectionEvent));
    eventSelection.property = None;

    if (X->xselectionrequest.target == serverTARGETS)
    {
      Atom xa_STRING = XA_STRING;
      XChangeProperty (nxagentDisplay,
                                X->xselectionrequest.requestor,
                                X->xselectionrequest.property,
                                XInternAtom(nxagentDisplay, "ATOM", 0),
                                sizeof(Atom)*8,
                                PropModeReplace,
                                (unsigned char*)&xa_STRING,
                                1);
      eventSelection.property = X->xselectionrequest.property;
    }
    else if (X->xselectionrequest.target == nxagentTimestampAtom)
    {
      while ((i < NumCurrentSelections) &&
                lastSelectionOwner[i].selection != X->xselectionrequest.selection) i++;

      if (i < NumCurrentSelections)
      {
        XChangeProperty(nxagentDisplay,
                                 X->xselectionrequest.requestor,
                                 X->xselectionrequest.property,
                                 X->xselectionrequest.target,
                                 32,
                                 PropModeReplace,
                                 (unsigned char *) &lastSelectionOwner[i].lastTimeChanged,
                                 1);
        eventSelection.property = X->xselectionrequest.property;
      }
    }

    eventSelection.type = SelectionNotify;
    eventSelection.send_event = True;
    eventSelection.display = nxagentDisplay;
    eventSelection.requestor = X->xselectionrequest.requestor;
    eventSelection.selection = X->xselectionrequest.selection;
    eventSelection.target = X->xselectionrequest.target;
    eventSelection.time = X->xselectionrequest.time;

    #ifdef DEBUG
    int result =
    #endif
    XSendEvent(nxagentDisplay,
                        eventSelection.requestor,
                        False,
                        0L,
                        (XEvent *) &eventSelection);

    #ifdef DEBUG
    fprintf(stderr, "%s: XSendEvent() returned [%s]\n", __func__, GetXErrorString(result));
    if (result == BadValue || result == BadWindow)
    {
      fprintf(stderr, "%s: WARNING! XSendEvent failed.\n", __func__);
    }
    else
    {
      fprintf(stderr, "%s: XSendEvent sent to window [0x%lx].\n", __func__,
                  eventSelection.requestor);
    }
    #endif

    return;
  }

  /*
   * This is necessary in nxagentGetClipboardWindow.
   */

  nxagentLastRequestedSelection = X->xselectionrequest.selection;

  /* FIXME: shouldn't we reset i to 0 here first? */
  while ((i < nxagentMaxSelections) &&
            (lastSelectionOwner[i].selection != X->xselectionrequest.selection))
  {
    i++;
  }

  if (i < nxagentMaxSelections)
  {
    if ((lastClientWindowPtr != NULL) && (lastSelectionOwner[i].client != NULL))
    {
      XConvertSelection(nxagentDisplay, CurrentSelections[i].selection,
                            X->xselectionrequest.target, serverCutProperty,
                                serverWindow, lastClientTime);

      #ifdef DEBUG
      fprintf(stderr, "%s: Sent XConvertSelection.\n", __func__);
      #endif
    }
    else
    {
      if (lastSelectionOwner[i].client != NULL && 
             nxagentOption(Clipboard) != ClipboardClient)
      {
        xEvent x;

        lastServerProperty = X->xselectionrequest.property;
        lastServerRequestor = X->xselectionrequest.requestor;
        lastServerTarget = X->xselectionrequest.target;

        /* by dimbor */
        if (lastServerTarget != XA_STRING)
            lastServerTarget = serverUTF8_STRING;

        lastServerTime = X->xselectionrequest.time;

        memset(&x, 0, sizeof(xEvent));
        x.u.u.type = SelectionRequest;
        x.u.selectionRequest.time = GetTimeInMillis();
        x.u.selectionRequest.owner = lastSelectionOwner[i].window;

        /*
         * Fictitious window.
         */

        x.u.selectionRequest.requestor = screenInfo.screens[0]->root->drawable.id;

        /*
         * Don't send the same window, some programs are
         * clever and verify cut and paste operations
         * inside the same window and don't Notify at all.
         *
         * x.u.selectionRequest.requestor = lastSelectionOwnerWindow;
         */

        x.u.selectionRequest.selection = CurrentSelections[i].selection;

        /* by dimbor (idea from zahvatov) */
        if (X->xselectionrequest.target != XA_STRING)
          x.u.selectionRequest.target = clientUTF8_STRING;
        else
          x.u.selectionRequest.target = XA_STRING;

        x.u.selectionRequest.property = clientCutProperty;

        (void) TryClientEvents(lastSelectionOwner[i].client, &x, 1,
                               NoEventMask, NoEventMask /* CantBeFiltered */,
                               NullGrab);

        #ifdef DEBUG
        fprintf(stderr, "%s: Executed TryClientEvents with clientCutProperty.\n", __func__);
        #endif
      }
      else
      {
        /*
         * Probably we must send a Notify
         * to requestor with property None.
         */

        eventSelection.type = SelectionNotify;
        eventSelection.send_event = True;
        eventSelection.display = nxagentDisplay;
        eventSelection.requestor = X->xselectionrequest.requestor;
        eventSelection.selection = X->xselectionrequest.selection;
        eventSelection.target = X->xselectionrequest.target;
        eventSelection.property = None;
        eventSelection.time = X->xselectionrequest.time;

        #ifdef DEBUG
        int result =
        #endif
        XSendEvent(nxagentDisplay,
                            eventSelection.requestor,
                            False,
                            0L,
                            (XEvent *) &eventSelection);

        #ifdef DEBUG
        fprintf(stderr, "%s: XSendEvent() returned [%s]\n", __func__, GetXErrorString(result));
        if (result == BadValue || result == BadWindow)
        {
          fprintf(stderr, "%s: WARNING! XSendEvent failed.\n", __func__);
        }
        else
        {
          fprintf(stderr, "%s: XSendEvent with property None sent to window [0x%lx].\n", __func__,
                      eventSelection.requestor);
        }
        #endif
      }
    }
  }
  nxagentPrintClipboardStat("after nxagentRequestSelection");
}

void nxagentSendSelectionNotify(Atom property)
{
  xEvent x;

  #ifdef DEBUG
  fprintf (stderr, "%s: Sending event to client [%d].\n", __func__,
               lastClientClientPtr -> index);
  #endif

  memset(&x, 0, sizeof(xEvent));
  x.u.u.type = SelectionNotify;

  x.u.selectionNotify.time = lastClientTime;
  x.u.selectionNotify.requestor = lastClientRequestor;
  x.u.selectionNotify.selection = lastClientSelection;
  x.u.selectionNotify.target = lastClientTarget;

  x.u.selectionNotify.property = property;

  TryClientEvents(lastClientClientPtr, &x, 1, NoEventMask,
                      NoEventMask , NullGrab);

  return;
}

void nxagentTransferSelection(int resource)
{
  int result;

  if (lastClientClientPtr -> index != resource)
  {
    #ifdef DEBUG
    fprintf (stderr, "%s: WARNING! Inconsistent resource [%d] with current client [%d].\n", __func__,
                 resource, lastClientClientPtr -> index);
    #endif

    nxagentSendSelectionNotify(None);

    lastClientWindowPtr = NULL;
    SetClientSelectionStage(None);

    return;
  }

  switch (lastClientStage)
  {
    case SelectionStageQuerySize:
    {
      PrintClientSelectionStage();
      /*
       * Don't get data yet, just get size. We skip
       * this stage in current implementation and
       * go straight to the data.
       */

      nxagentLastClipboardClient = NXGetCollectPropertyResource(nxagentDisplay);

      if (nxagentLastClipboardClient == -1)
      {
        #ifdef WARNING
        fprintf(stderr, "%s: WARNING! Asynchronous GetProperty queue full.\n", __func__);
        #endif

        result = -1;
      }
      else
      {
        result = NXCollectProperty(nxagentDisplay,
                                   nxagentLastClipboardClient,
                                   serverWindow,
                                   serverCutProperty,
                                   0,
                                   0,
                                   False,
                                   AnyPropertyType);
      }

      if (result == -1)
      {
        #ifdef DEBUG
        fprintf (stderr, "%s: Aborting selection notify procedure for client [%d].\n", __func__,
                     lastClientClientPtr -> index);
        #endif

        nxagentSendSelectionNotify(None);

        lastClientWindowPtr = NULL;
        SetClientSelectionStage(None);

        return;
      }

      SetClientSelectionStage(WaitSize);

      break;
    }
    case SelectionStageQueryData:
    {
      PrintClientSelectionStage();

      /*
       * Request the selection data now.
       */

      #ifdef DEBUG
      fprintf(stderr, "%s: Getting property content from remote server.\n", __func__);
      #endif

      nxagentLastClipboardClient = NXGetCollectPropertyResource(nxagentDisplay);

      if (nxagentLastClipboardClient == -1)
      {
        #ifdef WARNING
        fprintf(stderr, "%s: WARNING! Asynchronous GetProperty queue full.\n", __func__);
        #endif

        result = -1;
      }
      else
      {
        result = NXCollectProperty(nxagentDisplay,
                                   nxagentLastClipboardClient,
                                   serverWindow,
                                   serverCutProperty,
                                   0,
                                   lastClientPropertySize,
                                   False,
                                   AnyPropertyType);
      }

      if (result == -1)
      {
        #ifdef DEBUG
        fprintf (stderr, "%s: Aborting selection notify procedure for client [%d].\n", __func__,
                     lastClientClientPtr -> index);
        #endif

        nxagentSendSelectionNotify(None);

        lastClientWindowPtr = NULL;
        SetClientSelectionStage(None);

        return;
      }

      SetClientSelectionStage(WaitData);

      break;
    }
    default:
    {
      #ifdef DEBUG
      fprintf (stderr, "%s: WARNING! Inconsistent state [%s] for client [%d].\n", __func__,
                   GetClientSelectionStageString(lastClientStage), lastClientClientPtr -> index);
      #endif

      break;
    }
  }
}

void nxagentCollectPropertyEvent(int resource)
{
  Atom                  atomReturnType;
  int                   resultFormat;
  unsigned long         ulReturnItems;
  unsigned long         ulReturnBytesLeft;
  unsigned char         *pszReturnData = NULL;
  int                   result;

  /*
   * We have received the notification so
   * we can safely retrieve data from the
   * client structure.
   */

  result = NXGetCollectedProperty(nxagentDisplay,
                                  resource,
                                  &atomReturnType,
                                  &resultFormat,
                                  &ulReturnItems,
                                  &ulReturnBytesLeft,
                                  &pszReturnData);

  nxagentLastClipboardClient = -1;

  if (result == 0)
  {
    #ifdef DEBUG
    fprintf (stderr, "%s: Failed to get reply data for client [%d].\n", __func__,
                 lastClientClientPtr -> index);
    #endif

    nxagentSendSelectionNotify(None);

    lastClientWindowPtr = NULL;
    SetClientSelectionStage(None);

    SAFE_XFree(pszReturnData);
    return;
  }
 
  if (resultFormat != 8 && resultFormat != 16 && resultFormat != 32)
  {
    #ifdef DEBUG
    fprintf (stderr, "%s: WARNING! Invalid property value.\n", __func__);
    #endif

    if (lastClientClientPtr != NULL)
    {
      nxagentSendSelectionNotify(None);
    }

    lastClientWindowPtr = NULL;
    SetClientSelectionStage(None);

    SAFE_XFree(pszReturnData);
    return;
  }

  switch (lastClientStage)
  {
    case SelectionStageWaitSize:
    {
      PrintClientSelectionStage();
      #ifdef DEBUG
      fprintf (stderr, "%s: Got size notify event for client [%d].\n", __func__,
                   lastClientClientPtr -> index);
      #endif

      if (ulReturnBytesLeft == 0)
      {
        #ifdef DEBUG
        fprintf (stderr, "%s: Aborting selection notify procedure for client [%d].\n", __func__,
                     lastClientClientPtr -> index);
        #endif

        nxagentSendSelectionNotify(None);

        lastClientWindowPtr = NULL;
        SetClientSelectionStage(None);

        SAFE_XFree(pszReturnData);
        return;
      }

      #ifdef DEBUG
      fprintf(stderr, "%s: Got property size from remote server.\n", __func__);
      #endif

      /*
       * Request the selection data now.
       */

      lastClientPropertySize = ulReturnBytesLeft;
      SetClientSelectionStage(QueryData);

      nxagentTransferSelection(resource);

      break;
    }
    case SelectionStageWaitData:
    {
      PrintClientSelectionStage();
      #ifdef DEBUG
      fprintf (stderr, "%s: Got data notify event for client [%d].\n", __func__,
                   lastClientClientPtr -> index);
      #endif

      if (ulReturnBytesLeft != 0)
      {
        #ifdef DEBUG
        fprintf (stderr, "%s: Aborting selection notify procedure for client [%d].\n", __func__,
                     lastClientClientPtr -> index);
        #endif

        nxagentSendSelectionNotify(None);

        lastClientWindowPtr = NULL;
        SetClientSelectionStage(None);

        SAFE_XFree(pszReturnData);
        return;
      }

      #ifdef DEBUG
      fprintf(stderr, "%s: Got property content from remote server.\n", __func__);
      #endif

      ChangeWindowProperty(lastClientWindowPtr,
                           lastClientProperty,
                           lastClientTarget,
                           resultFormat, PropModeReplace,
                           ulReturnItems, pszReturnData, 1);

      #ifdef DEBUG
      {
        int num = min(20, ulReturnItems);
        fprintf(stderr, "%s: Selection property [%s] changed to [\"%*.*s\"...] \n", __func__,
                validateString(NameForAtom(lastClientProperty)), num, num, pszReturnData);
      }
      #endif

      nxagentSendSelectionNotify(lastClientProperty);

      /*
       * Enable further requests from clients.
       */

      lastClientWindowPtr = NULL;
      SetClientSelectionStage(None);

      break;
    }
    default:
    {
      #ifdef DEBUG
      fprintf (stderr, "%s: WARNING! Inconsistent state [%s] for client [%d].\n", __func__,
                   GetClientSelectionStageString(lastClientStage), lastClientClientPtr -> index);
      #endif

      break;
    }
  }

  SAFE_XFree(pszReturnData);
}

void nxagentNotifySelection(XEvent *X)
{
  XSelectionEvent eventSelection;

  #ifdef DEBUG
  fprintf(stderr, "%s: Got called.\n", __func__);
  #endif

  if (agentClipboardStatus != 1)
  {
    return;
  }

  #ifdef DEBUG
  fprintf(stderr, "%s: SelectionNotify event.\n", __func__);
  #endif

  PrintClientSelectionStage();

  if (lastClientWindowPtr != NULL)
  {
    if ((lastClientStage == SelectionStageNone) && (X->xselection.property == serverCutProperty))
    {
      #ifdef DEBUG
      fprintf(stderr, "%s: Starting selection transferral for client [%d].\n", __func__,
                  lastClientClientPtr -> index);
      #endif

      /*
       * The state machine is able to work in two phases. In the first
       * phase we get the size of property data, in the second we get
       * the actual data. We save a round-trip by requesting a prede-
       * termined amount of data in a single GetProperty and by discar-
       * ding the remaining part. This is not the optimal solution (we
       * could get the remaining part if it doesn't fit in a single
       * reply) but, at least with text, it should work in most situa-
       * tions.
       */

      SetClientSelectionStage(QueryData);
      lastClientPropertySize = 262144;

      nxagentTransferSelection(lastClientClientPtr -> index);
    }
    else
    {
      #ifdef DEBUG
      fprintf(stderr, "%s: WARNING! Resetting selection transferral for client [%d].\n", __func__,
                  lastClientClientPtr -> index);
      #endif

      nxagentSendSelectionNotify(None);

      lastClientWindowPtr = NULL;
      SetClientSelectionStage(None);
    }

    return;
  }
  else
  {
    int i = 0;

    while ((i < nxagentMaxSelections) && (lastSelectionOwner[i].selection != X->xselection.selection))
    {
      i++;
    }

    if (i < nxagentMaxSelections)
    {
      if ((lastSelectionOwner[i].client != NULL) &&
             (lastSelectionOwner[i].windowPtr != NULL) &&
                 (X->xselection.property == clientCutProperty))
      {
        Atom            atomReturnType;
        int             resultFormat;
        unsigned long   ulReturnItems;
        unsigned long   ulReturnBytesLeft;
        unsigned char   *pszReturnData = NULL;

        int result = GetWindowProperty(lastSelectionOwner[i].windowPtr, clientCutProperty, 0, 0, False,
                                           AnyPropertyType, &atomReturnType, &resultFormat,
                                               &ulReturnItems, &ulReturnBytesLeft, &pszReturnData);

        #ifdef DEBUG
        fprintf(stderr, "%s: GetWindowProperty() returned [%s]\n", __func__, GetXErrorString(result));
        #endif
        if (result == BadAlloc || result == BadAtom ||
                result == BadWindow || result == BadValue)
        {
          fprintf (stderr, "Client GetProperty failed. Error = %s", GetXErrorString(result));
          lastServerProperty = None;
        }
        else
        {
          result = GetWindowProperty(lastSelectionOwner[i].windowPtr, clientCutProperty, 0,
                                         ulReturnBytesLeft, False, AnyPropertyType, &atomReturnType,
                                             &resultFormat, &ulReturnItems, &ulReturnBytesLeft,
                                                 &pszReturnData);
          #ifdef DEBUG
          fprintf(stderr, "%s: GetWindowProperty() returned [%s]\n", __func__, GetXErrorString(result));
          #endif

          if (result == BadAlloc || result == BadAtom ||
                  result == BadWindow || result == BadValue)
          {
            fprintf (stderr, "SelectionNotify - XGetWindowProperty failed. Error = %s\n", GetXErrorString(result));
            lastServerProperty = None;
          }
          else
          {
            result = XChangeProperty(nxagentDisplay,
                                     lastServerRequestor,
                                     lastServerProperty,
                                     lastServerTarget,
                                     8,
                                     PropModeReplace,
                                     pszReturnData,
                                     ulReturnItems);
          }
          #ifdef DEBUG
          fprintf(stderr, "%s: XChangeProperty() returned [%s]\n", __func__, GetXErrorString(result));
          #endif

          /*
           * SAFE_XFree(pszReturnData);
           */

        }

        memset(&eventSelection, 0, sizeof(XSelectionEvent));
        eventSelection.type = SelectionNotify;
        eventSelection.send_event = True;
        eventSelection.display = nxagentDisplay;
        eventSelection.requestor = lastServerRequestor;

        eventSelection.selection = X->xselection.selection;

        /*
         * eventSelection.target = X->xselection.target;
         */

        eventSelection.target = lastServerTarget;
        eventSelection.property = lastServerProperty;
        eventSelection.time = lastServerTime;

        /*
         * eventSelection.time = CurrentTime;
         * eventSelection.time = lastServerTime;
         */

        #ifdef DEBUG
        fprintf(stderr, "%s: Sending event to requestor [%p].\n", __func__, (void *)eventSelection.requestor);
        #endif

        result = XSendEvent(nxagentDisplay,
                             eventSelection.requestor,
                             False,
                             0L,
                             (XEvent *) &eventSelection);

        #ifdef DEBUG
        fprintf(stderr, "%s: XSendEvent() returned [%s]\n", __func__, GetXErrorString(result));
        #endif
        if (result == BadValue || result == BadWindow)
        {
          fprintf (stderr, "SelectionRequest - XSendEvent failed\n");
        }

        lastServerRequestor = None; /* allow further request */
      }
    }
  }
}

/*
 * Acquire selection so we don't get selection
 * requests from real X clients.
 */

void nxagentResetSelectionOwner(void)
{
  int i;

  if (lastServerRequestor != None)
  {
    #ifdef TEST
    fprintf(stderr, "%s: WARNING! Requestor window [0x%x] already found.\n", __func__,
                lastServerRequestor);
    #endif

    return;
  }

  /*
   * Only for PRIMARY and CLIPBOARD selections.
   */

  for (i = 0; i < nxagentMaxSelections; i++)
  {
    XSetSelectionOwner(nxagentDisplay, lastSelectionOwner[i].selection, serverWindow, CurrentTime);

    #ifdef DEBUG
    fprintf(stderr, "%s: Reset clipboard state.\n", __func__);
    #endif

    lastSelectionOwner[i].client = NULL;
    lastSelectionOwner[i].window = None;
    lastSelectionOwner[i].windowPtr = NULL;
    lastSelectionOwner[i].lastTimeChanged = GetTimeInMillis();
  }

  lastClientWindowPtr = NULL;
  SetClientSelectionStage(None);

  lastServerRequestor = None;

  return;
}

#ifdef NXAGENT_CLIPBOARD
void nxagentSetSelectionCallback(CallbackListPtr *callbacks, void *data,
                                   void *args)
{
  /*
   * Only act if the Trap is unset. The trap indicates that we are
   * triggered by a clipboard event originating from the real X
   * server. In that case we do not want to propagate back changes to
   * the real X server, because it already knows about them and we
   * would end up in an infinite loop of events. If there was a better
   * way to identify that situation during Callback processing we
   * could get rid of the Trap...
  */
  if (nxagentExternalClipboardEventTrap != 0)
  {
    #ifdef DEBUG
    fprintf(stderr, "%s: Trap is set, doing nothing\n", __func__);
    #endif
    return;
  }

  SelectionInfoRec *info = (SelectionInfoRec *)args;

  Selection * pCurSel = (Selection *)info->selection;

  #ifdef DEBUG
  fprintf(stderr, "%s: pCurSel->lastTimeChanged [%u]\n", __func__, pCurSel->lastTimeChanged.milliseconds);
  #endif

  if (info->kind == SelectionSetOwner)
  {
    #ifdef DEBUG
    fprintf(stderr, "%s: called with SelectionCallbackKind SelectionSetOwner\n", __func__);
    fprintf(stderr, "%s: pCurSel->pWin [0x%x]\n", __func__, pCurSel->pWin ? pCurSel->pWin->drawable.id : 0);
    fprintf(stderr, "%s: pCurSel->selection [%d][%s]\n", __func__, pCurSel->selection, NameForAtom(pCurSel->selection));
    #endif

    if ((pCurSel->pWin != NULL) &&
        (nxagentOption(Clipboard) != ClipboardNone) &&
        ((pCurSel->selection == XA_PRIMARY) ||
         (pCurSel->selection == MakeAtom("CLIPBOARD", 9, 0))))
    {
      #ifdef DEBUG
      fprintf(stderr, "%s: calling nxagentSetSelectionOwner\n", __func__);
      #endif
      nxagentSetSelectionOwner(pCurSel);
    }
  }
  else if (info->kind == SelectionWindowDestroy)
  {
    #ifdef DEBUG
    fprintf(stderr, "%s: called with SelectionCallbackKind SelectionWindowDestroy\n", __func__);
    #endif
  }
  else if (info->kind == SelectionClientClose)
  {
    #ifdef DEBUG
    fprintf(stderr, "%s: called with SelectionCallbackKind SelectionClientClose\n", __func__);
    #endif
  }
  else
  {
    #ifdef DEBUG
    fprintf(stderr, "%s: called with unknown SelectionCallbackKind\n", __func__);
    #endif
  }
}
#endif

void nxagentSetSelectionOwner(Selection *pSelection)
{
  #ifdef DEBUG
  fprintf(stderr, "%s: Got called.\n", __func__);
  #endif

  if (agentClipboardStatus != 1)
  {
    return;
  }

  #ifdef DEBUG
  fprintf(stderr, "%s: Setting selection owner to serverwindow ([0x%x]).\n", __func__,
              serverWindow);
  #endif

  #ifdef TEST
  if (lastServerRequestor != None)
  {
    fprintf (stderr, "%s: WARNING! Requestor window [0x%x] already found.\n", __func__,
                 lastServerRequestor);
  }
  #endif

  /*
   * Only for PRIMARY and CLIPBOARD selections.
   */

  for (int i = 0; i < nxagentMaxSelections; i++)
  {
    if (pSelection->selection == CurrentSelections[i].selection)
    {
      #ifdef DEBUG
      fprintf(stderr, "%s: lastSelectionOwner[%d].client [0x%x] -> [0x%x]\n", __func__, i, lastSelectionOwner[i].client, pSelection->client);
      fprintf(stderr, "%s: lastSelectionOwner[%d].window [0x%x] -> [0x%x]\n", __func__, i, lastSelectionOwner[i].window, pSelection->window);
      fprintf(stderr, "%s: lastSelectionOwner[%d].windowPtr [0x%x] -> [0x%x] [0x%x] (serverWindow: [0x%x])\n", __func__, i, lastSelectionOwner[i].windowPtr, pSelection->pWin, nxagentWindow(pSelection->pWin), serverWindow);
      fprintf(stderr, "%s: lastSelectionOwner[%d].lastTimeChanged [%u]\n", __func__, i, lastSelectionOwner[i].lastTimeChanged);
      #endif

      /*
       * inform the real X server that our serverWindow is the
       * clipboard owner. The real owner window (inside nxagent) is
       * stored in lastSelectionOwner.window.
       * lastSelectionOwner.windowPtr points to the struct that
       * contains all information about the owner window
       */
      XSetSelectionOwner(nxagentDisplay, lastSelectionOwner[i].selection, serverWindow, CurrentTime);

      lastSelectionOwner[i].client = pSelection->client;
      lastSelectionOwner[i].window = pSelection->window;
      lastSelectionOwner[i].windowPtr = pSelection->pWin;
      lastSelectionOwner[i].lastTimeChanged = GetTimeInMillis();
    }
  }

  lastClientWindowPtr = NULL;
  SetClientSelectionStage(None);

  lastServerRequestor = None;

/*
FIXME

   if (XGetSelectionOwner(nxagentDisplay,pSelection->selection)==serverWindow)
   {
      fprintf (stderr, "%s: SetSelectionOwner OK\n", __func__);

      lastSelectionOwnerSelection = pSelection;
      lastSelectionOwnerClient = pSelection->client;
      lastSelectionOwnerWindow = pSelection->window;
      lastSelectionOwnerWindowPtr = pSelection->pWin;

      lastClientWindowPtr = NULL;
      SetClientSelectionStage(None);

      lastServerRequestor = None;
   }
   else fprintf (stderr, "%s: SetSelectionOwner failed\n", __func__);
*/
}

void nxagentNotifyConvertFailure(ClientPtr client, Window requestor,
                                     Atom selection, Atom target, Time time)
{
  xEvent x;

/*
FIXME: Why this pointer can be not a valid
       client pointer?
*/
  if (clients[client -> index] != client)
  {
    #ifdef WARNING
    fprintf(stderr, "%s: WARNING! Invalid client pointer.", __func__);
    #endif

    return;
  }

  memset(&x, 0, sizeof(xEvent));
  x.u.u.type = SelectionNotify;
  x.u.selectionNotify.time = time;
  x.u.selectionNotify.requestor = requestor;
  x.u.selectionNotify.selection = selection;
  x.u.selectionNotify.target = target;
  x.u.selectionNotify.property = None;

  (void) TryClientEvents(client, &x, 1, NoEventMask,
                           NoEventMask , NullGrab);
}

int nxagentConvertSelection(ClientPtr client, WindowPtr pWin, Atom selection,
                                Window requestor, Atom property, Atom target, Time time)
{
  const char *strTarget;
  int i;

  if (agentClipboardStatus != 1 ||
           nxagentOption(Clipboard) == ClipboardServer)
  {
    return 0;
  }

  /*
   * There is a client owner on the agent side, let normal stuff happen.
   */

  /*
   * Only for PRIMARY and CLIPBOARD selections.
   */

  for (i = 0; i < nxagentMaxSelections; i++)
  {
    if ((selection == CurrentSelections[i].selection) &&
           (lastSelectionOwner[i].client != NULL))
    {
      return 0;
    }
  }

  if (lastClientWindowPtr != NULL)
  {
    #ifdef TEST
    fprintf(stderr, "%s: lastClientWindowPtr != NULL.\n", __func__);
    #endif

    if ((GetTimeInMillis() - lastClientReqTime) > 5000)
    {
      #ifdef DEBUG
      fprintf(stderr, "%s: timeout expired on last request, "
                  "notifying failure to client\n", __func__);
      #endif

      nxagentNotifyConvertFailure(lastClientClientPtr, lastClientRequestor,
                                     lastClientSelection, lastClientTarget, lastClientTime);

      lastClientWindowPtr = NULL;
      SetClientSelectionStage(None);
    }
    else
    {
      #ifdef DEBUG
      fprintf(stderr, "%s: got request "
                  "before timeout expired on last request, notifying failure to client\n", __func__);
      #endif

      nxagentNotifyConvertFailure(client, requestor, selection, target, time);

      return 1;
    }
  }

  #ifdef TEST
  fprintf(stderr, "%s: client [%d] ask for sel [%s] "
              "on window [%x] prop [%s] target [%s].\n", __func__,
                  client -> index, validateString(NameForAtom(selection)), requestor,
                      validateString(NameForAtom(property)), validateString(NameForAtom(target)));
  #endif

  strTarget = NameForAtom(target);

  if (strTarget == NULL)
  {
    return 1;
  }

  if (target == clientTARGETS)
  {
    Atom xa_STRING[4];
    xEvent x;

    /* --- Order changed by dimbor (prevent sending COMPOUND_TEXT to client --- */
    xa_STRING[0] = XA_STRING;
    xa_STRING[1] = clientUTF8_STRING;
    xa_STRING[2] = clientTEXT;
    xa_STRING[3] = clientCOMPOUND_TEXT;

    ChangeWindowProperty(pWin,
                         property,
                         MakeAtom("ATOM", 4, 1),
                         sizeof(Atom)*8,
                         PropModeReplace,
                         4,
                         &xa_STRING, 1);

    memset(&x, 0, sizeof(xEvent));
    x.u.u.type = SelectionNotify;
    x.u.selectionNotify.time = time;
    x.u.selectionNotify.requestor = requestor;
    x.u.selectionNotify.selection = selection;
    x.u.selectionNotify.target = target;
    x.u.selectionNotify.property = property;

    (void) TryClientEvents(client, &x, 1, NoEventMask,
                           NoEventMask , NullGrab);

    return 1;
  }

  if (target == MakeAtom("TIMESTAMP", 9, 1))
  {
    int i = 0;

    while ((i < NumCurrentSelections) &&
              CurrentSelections[i].selection != selection) i++;

    if (i < NumCurrentSelections)
    {
      xEvent x;
      ChangeWindowProperty(pWin,
                           property,
                           target,
                           32,
                           PropModeReplace,
                           1,
                           (unsigned char *) &lastSelectionOwner[i].lastTimeChanged,
                           1);

      memset(&x, 0, sizeof(xEvent));
      x.u.u.type = SelectionNotify;
      x.u.selectionNotify.time = time;
      x.u.selectionNotify.requestor = requestor;
      x.u.selectionNotify.selection = selection;
      x.u.selectionNotify.target = target;
      x.u.selectionNotify.property = property;
  
      (void) TryClientEvents(client, &x, 1, NoEventMask,
                             NoEventMask , NullGrab);
  
      return 1;

    }
  }

  if (lastClientClientPtr == client && (GetTimeInMillis() - lastClientReqTime < 5000))
  {
    /*
     * The same client made consecutive requests
     * of clipboard contents with less than 5
     * seconds time interval between them.
     */

    #ifdef DEBUG
    fprintf(stderr, "%s: Consecutives request from client [%p] selection [%u] "
                "elapsed time [%u] clientAccum [%d]\n", __func__, (void *) client, selection,
                    GetTimeInMillis() - lastClientReqTime, clientAccum);
    #endif

    clientAccum++;
  }
  else
  {
    if (lastClientClientPtr != client)
    {
      clientAccum = 0;
    }
  }

  if ((target == clientTEXT) ||
          (target == XA_STRING) ||
              (target == clientCOMPOUND_TEXT) ||
                  (target == clientUTF8_STRING))
  {
    lastClientWindowPtr = pWin;
    SetClientSelectionStage(None);
    lastClientRequestor = requestor;
    lastClientClientPtr = client;
    lastClientTime = time;
    lastClientProperty = property;
    lastClientSelection = selection;
    lastClientTarget = target;

    lastClientReqTime = (GetTimeInMillis() - lastClientReqTime) > 5000 ?
                          GetTimeInMillis() : lastClientReqTime;

    if (selection == MakeAtom("CLIPBOARD", 9, 0))
    {
      selection = lastSelectionOwner[nxagentClipboardSelection].selection;
    }

    if (target == clientUTF8_STRING)
    {
      XConvertSelection(nxagentDisplay, selection, serverUTF8_STRING, serverCutProperty,
                           serverWindow, CurrentTime);
    }
    else
    {
      XConvertSelection(nxagentDisplay, selection, XA_STRING, serverCutProperty,
                           serverWindow, CurrentTime);
    }

    #ifdef DEBUG
    fprintf(stderr, "%s: Sent XConvertSelection with target=[%s], property [%s]\n", __func__,
                validateString(NameForAtom(target)), validateString(NameForAtom(property)));
    #endif

    return 1;
  }
  else
  {
    xEvent x;

    #ifdef DEBUG
    fprintf(stderr, "%s: Xserver generates a SelectionNotify event "
                "to the requestor with property None.\n", __func__);
    #endif

    memset(&x, 0, sizeof(xEvent));
    x.u.u.type = SelectionNotify;
    x.u.selectionNotify.time = time;
    x.u.selectionNotify.requestor = requestor;
    x.u.selectionNotify.selection = selection;
    x.u.selectionNotify.target = target;
    x.u.selectionNotify.property = None;
    (void) TryClientEvents(client, &x, 1, NoEventMask, NoEventMask , NullGrab);
    return 1;
  }
  return 0;
}

int nxagentSendNotify(xEvent *event)
{
  #ifdef DEBUG
  fprintf(stderr, "%s: Got called.\n", __func__);
  #endif

  if (agentClipboardStatus != 1)
  {
    #ifdef DEBUG
    fprintf(stderr, "%s: agentClipboardStatus != 1 - doing nothing.\n", __func__);
    #endif
    return 0;
  }

  #ifdef DEBUG
  fprintf(stderr, "%s: property is [%d][%s].\n", __func__, event->u.selectionNotify.property, NameForAtom(event->u.selectionNotify.property));
  #endif

  if (event->u.selectionNotify.property == clientCutProperty)
  {
    XSelectionEvent x;
    int result;

    /*
     * Setup selection notify event to real server.
     */

    memset(&x, 0, sizeof(XSelectionEvent));
    x.type = SelectionNotify;
    x.send_event = True;
    x.display = nxagentDisplay;
    x.requestor = serverWindow;

    /*
     * On real server, the right CLIPBOARD atom is
     * XInternAtom(nxagentDisplay, "CLIPBOARD", 1).
     */

    if (event->u.selectionNotify.selection == MakeAtom("CLIPBOARD", 9, 0))
    {
      x.selection = lastSelectionOwner[nxagentClipboardSelection].selection;
    }
    else
    {
      x.selection = event->u.selectionNotify.selection;
    }

    x.target = event->u.selectionNotify.target;
    x.property = event->u.selectionNotify.property;
    x.time = CurrentTime;

    #ifdef DEBUG
    fprintf(stderr, "%s: Propagating clientCutProperty to requestor [%p].\n", __func__, (void *)x.requestor);
    #endif

    result = XSendEvent (nxagentDisplay, x.requestor, False,
                             0L, (XEvent *) &x);

    #ifdef DEBUG
    fprintf(stderr, "%s: XSendEvent() returned [%s]\n", __func__, GetXErrorString(result));
    #endif
    if (result == BadValue || result == BadWindow)
    {
      fprintf (stderr, "%s: XSendEvent failed.\n", __func__);
    }

    return 1;
  }
  #ifdef DEBUG
  fprintf(stderr, "%s: sent nothing.\n", __func__);
  #endif
  return 0;
}

WindowPtr nxagentGetClipboardWindow(Atom property, WindowPtr pWin)
{
  int i = 0;

  #ifdef DEBUG
  fprintf(stderr, "%s: Got called, property [%d][%s] window [%p].\n", __func__, property, NameForAtom(property), (void *)pWin);
  #endif

  while ((i < nxagentMaxSelections) &&
            (lastSelectionOwner[i].selection != nxagentLastRequestedSelection))
  {
    i++;
  }

  if ((i < nxagentMaxSelections) && (property == clientCutProperty) &&
          (lastSelectionOwner[i].windowPtr != NULL))
  {
    #ifdef DEBUG
    fprintf(stderr, "%s: Returning last clipboard owner window [%p].\n", __func__, (void *)lastSelectionOwner[i].windowPtr);
    #endif

    return lastSelectionOwner[i].windowPtr;
  }
  else
  {
    #ifdef DEBUG
    fprintf(stderr, "%s: Returning original target window [%p].\n", __func__, (void *)pWin);
    #endif

    return pWin;
  }

}

int nxagentInitClipboard(WindowPtr pWin)
{
  int i;
  Window iWindow = nxagentWindow(pWin);

  #ifdef DEBUG
  fprintf(stderr, "%s: Got called.\n", __func__);
  #endif

  free(lastSelectionOwner);
  lastSelectionOwner = NULL;

  lastSelectionOwner = (SelectionOwner *) malloc(nxagentMaxSelections * sizeof(SelectionOwner));

  if (lastSelectionOwner == NULL)
  {
    FatalError("nxagentInitClipboard: Failed to allocate memory for the clipboard selections.\n");
  }

  nxagentClipboardAtom = nxagentAtoms[10];   /* CLIPBOARD */
  nxagentTimestampAtom = nxagentAtoms[11];   /* TIMESTAMP */

  lastSelectionOwner[nxagentPrimarySelection].selection = XA_PRIMARY;
  lastSelectionOwner[nxagentPrimarySelection].client = NullClient;
  lastSelectionOwner[nxagentPrimarySelection].window = screenInfo.screens[0]->root->drawable.id;
  lastSelectionOwner[nxagentPrimarySelection].windowPtr = NULL;
  lastSelectionOwner[nxagentPrimarySelection].lastTimeChanged = GetTimeInMillis();

  lastSelectionOwner[nxagentClipboardSelection].selection = nxagentClipboardAtom;
  lastSelectionOwner[nxagentClipboardSelection].client = NullClient;
  lastSelectionOwner[nxagentClipboardSelection].window = screenInfo.screens[0]->root->drawable.id;
  lastSelectionOwner[nxagentClipboardSelection].windowPtr = NULL;
  lastSelectionOwner[nxagentClipboardSelection].lastTimeChanged = GetTimeInMillis();

  #ifdef NXAGENT_TIMESTAMP
  {
    extern unsigned long startTime;

    fprintf(stderr, "%s: Initializing start [%d] milliseconds.\n", __func__,
            GetTimeInMillis() - startTime);
  }
  #endif

  agentClipboardStatus = 0;
  serverWindow = iWindow;

  /*
   * Local property to hold pasted data.
   */

  serverCutProperty = nxagentAtoms[5];  /* NX_CUT_BUFFER_SERVER */
  serverTARGETS = nxagentAtoms[6];  /* TARGETS */
  serverTEXT = nxagentAtoms[7];  /* TEXT */
  serverUTF8_STRING = nxagentAtoms[12]; /* UTF8_STRING */

  if (serverCutProperty == None)
  {
    #ifdef PANIC
    fprintf(stderr, "%s: PANIC! Could not create NX_CUT_BUFFER_SERVER atom\n", __func__);
    #endif

    return -1;
  }

  #ifdef TEST
  fprintf(stderr, "%s: Setting owner of selection [%s][%d] on window 0x%x\n", __func__,
              "NX_CUT_BUFFER_SERVER", (int) serverCutProperty, iWindow);
  #endif

  XSetSelectionOwner(nxagentDisplay, serverCutProperty, iWindow, CurrentTime);

  if (XQueryExtension(nxagentDisplay,
                      "XFIXES",
                      &nxagentXFixesInfo.Opcode,
                      &nxagentXFixesInfo.EventBase,
                      &nxagentXFixesInfo.ErrorBase) == 0)
  {
    ErrorF("Unable to initialize XFixes extension.\n");
  }
  else
  {
    #ifdef TEST
    fprintf(stderr, "%s: Registering for XFixesSelectionNotify events.\n", __func__);
    #endif

    for (i = 0; i < nxagentMaxSelections; i++)
    {
      XFixesSelectSelectionInput(nxagentDisplay, iWindow,
                                 lastSelectionOwner[i].selection,
                                 XFixesSetSelectionOwnerNotifyMask |
                                 XFixesSelectionWindowDestroyNotifyMask |
                                 XFixesSelectionClientCloseNotifyMask);
    }

    nxagentXFixesInfo.Initialized = 1;
  }

  /*
     The first paste from CLIPBOARD did not work directly after
     session start. Removing this code makes it work. It is unsure why
     it was introduced in the first place so it is possible that we
     see other effects by leaving out this code.

     Fixes X2Go bug #952, see https://bugs.x2go.org/952 for details .

  if (nxagentSessionId[0])
  {
    #ifdef TEST
    fprintf(stderr, "%s: setting the ownership of %s to %lx"
                " and registering for PropertyChangeMask events\n", __func__,
                    validateString(XGetAtomName(nxagentDisplay, nxagentAtoms[10])), iWindow);
    #endif

    XSetSelectionOwner(nxagentDisplay, nxagentAtoms[10], iWindow, CurrentTime);
    pWin -> eventMask |= PropertyChangeMask;
    nxagentChangeWindowAttributes(pWin, CWEventMask);
  }
  */

  if (nxagentReconnectTrap)
  {
    /*
     * Only for PRIMARY and CLIPBOARD selections.
     */

    for (i = 0; i < nxagentMaxSelections; i++)
    {
      if (lastSelectionOwner[i].client && lastSelectionOwner[i].window)
      {
        XSetSelectionOwner(nxagentDisplay, lastSelectionOwner[i].selection, iWindow, CurrentTime);
      }
    }
  }
  else
  {
    lastSelectionOwner[nxagentPrimarySelection].client = NULL;
    lastSelectionOwner[nxagentClipboardSelection].client = NULL;

    lastServerRequestor = None;

    lastClientWindowPtr = NULL;
    SetClientSelectionStage(None);
    lastClientReqTime = GetTimeInMillis();

    clientCutProperty = MakeAtom(szAgentNX_CUT_BUFFER_CLIENT,
                                         strlen(szAgentNX_CUT_BUFFER_CLIENT), 1);
    clientTARGETS = MakeAtom(szAgentTARGETS, strlen(szAgentTARGETS), True);
    clientTEXT = MakeAtom(szAgentTEXT, strlen(szAgentTEXT), True);
    clientCOMPOUND_TEXT = MakeAtom(szAgentCOMPOUND_TEXT, strlen(szAgentCOMPOUND_TEXT), True);
    clientUTF8_STRING = MakeAtom(szAgentUTF8_STRING, strlen(szAgentUTF8_STRING), True);

    if (clientCutProperty == None)
    {
      #ifdef PANIC
      fprintf(stderr, "%s: PANIC! "
              "Could not create NX_CUT_BUFFER_CLIENT atom.\n", __func__);
      #endif

      return -1;
    }
  }

  agentClipboardStatus = 1;

  #ifdef DEBUG
  fprintf(stderr, "%s: Clipboard initialization completed.\n", __func__);
  #endif

  #ifdef NXAGENT_TIMESTAMP
  {
    extern unsigned long startTime;

    fprintf(stderr, "%s: initializing ends [%d] milliseconds.\n", __func__,
                GetTimeInMillis() - startTime);
  }
  #endif

  return 1;
}
