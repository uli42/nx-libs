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

/************************************************************

Copyright 1987, 1989, 1998  The Open Group

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


Copyright 1987, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

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

********************************************************/

/* The panoramix components contained the following notice */
/*****************************************************************

Copyright (c) 1991, 1997 Digital Equipment Corporation, Maynard, Massachusetts.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR ANY CLAIM, DAMAGES, INCLUDING,
BUT NOT LIMITED TO CONSEQUENTIAL OR INCIDENTAL DAMAGES, OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Digital Equipment Corporation
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Digital
Equipment Corporation.

******************************************************************/


#include "../../dix/selection.c"

#include "Xatom.h"
#include "Options.h"
#include "Clipboard.h"

/*
 * Set here the required log level.
 */

#define PANIC
#define WARNING
#undef  TEST
#undef  WATCH

#ifdef NXAGENT_CLIPBOARD
extern int nxagentPrimarySelection;
extern int nxagentClipboardSelection;
extern int nxagentMaxSelections;
#endif

void
InitSelections(void)
{
    xorg_InitSelections();

#ifdef NXAGENT_CLIPBOARD
    {
      Selection *newsels;
      newsels = malloc(nxagentMaxSelections * sizeof(Selection));
      if (!newsels)
        return;
      NumCurrentSelections += nxagentMaxSelections;
      CurrentSelections = newsels;

      /* Note: these are the same values that will be set on a SelectionClear event */

      pSel = malloc(sizeof(Selection));
      psel->selection = XA_PRIMARY;

      CurrentSelections[nxagentPrimarySelection].selection = XA_PRIMARY;
      CurrentSelections[nxagentPrimarySelection].lastTimeChanged = ClientTimeToServerTime(CurrentTime);
      CurrentSelections[nxagentPrimarySelection].window = screenInfo.screens[0]->root->drawable.id;
      CurrentSelections[nxagentPrimarySelection].pWin = NULL;
      CurrentSelections[nxagentPrimarySelection].client = NullClient;

      CurrentSelections[nxagentClipboardSelection].selection = MakeAtom("CLIPBOARD", 9, 1);
      CurrentSelections[nxagentClipboardSelection].lastTimeChanged = ClientTimeToServerTime(CurrentTime);
      CurrentSelections[nxagentClipboardSelection].window = screenInfo.screens[0]->root->drawable.id;
      CurrentSelections[nxagentClipboardSelection].pWin = NULL;
      CurrentSelections[nxagentClipboardSelection].client = NullClient;
    }
#endif
}

int
ProcConvertSelection(ClientPtr client)
{
    Bool paramsOkay;
    xEvent event;
    WindowPtr pWin;
    Selection *pSel;
    int rc;

    REQUEST(xConvertSelectionReq);
    REQUEST_SIZE_MATCH(xConvertSelectionReq);

    rc = dixLookupWindow(&pWin, stuff->requestor, client, DixSetAttrAccess);
    if (rc != Success)
        return rc;

#ifdef NXAGENT_CLIPBOARD
    if (((stuff->selection == XA_PRIMARY) ||
           (stuff->selection == MakeAtom("CLIPBOARD", 9, 0))) &&
               nxagentOption(Clipboard) != ClipboardNone)
    {
      int index = nxagentFindCurrentSelectionIndex(stuff->selection);
      if ((index != -1) && (CurrentSelections[index].window != None))
      {
        if (nxagentConvertSelection(client, pWin, stuff->selection, stuff->requestor,
                                       stuff->property, stuff->target, stuff->time))
        {
          return (client->noClientException);
        }
      }
    }
#endif

    paramsOkay = (ValidAtom(stuff->selection) && ValidAtom(stuff->target));
    paramsOkay &= (stuff->property == None) || ValidAtom(stuff->property);
    if (!paramsOkay) {
	client->errorValue = stuff->property;
        return BadAtom;
    }

    rc = dixLookupSelection(&pSel, stuff->selection, client, DixReadAccess);

    memset(&event, 0, sizeof(xEvent));
    if (rc != Success && rc != BadMatch)
	return rc;
    else if (rc == Success && pSel->window != None
#if defined(NXAGENT_SERVER) && defined(NXAGENT_CLIPBOARD)
            /*
             * .window can be set and pointing to our server window to
             * signal the clipboard owner being on the real X
             * server. Therefore we need to check .client in addition
             * to ensure having a local owner.
             */
	    && (pSel->client != NullClient)
#endif
      ) {
	event.u.u.type = SelectionRequest;
	event.u.selectionRequest.owner = pSel->window;
	event.u.selectionRequest.time = stuff->time;
	event.u.selectionRequest.requestor = stuff->requestor;
	event.u.selectionRequest.selection = stuff->selection;
	event.u.selectionRequest.target = stuff->target;
	event.u.selectionRequest.property = stuff->property;
	if (pSel->client && pSel->client != serverClient && !pSel->client->clientGone)
	{
	    WriteEventsToClient(pSel->client, 1, &event);
	    return Success;
	}
    }

    event.u.u.type = SelectionNotify;
    event.u.selectionNotify.time = stuff->time;
    event.u.selectionNotify.requestor = stuff->requestor;
    event.u.selectionNotify.selection = stuff->selection;
    event.u.selectionNotify.target = stuff->target;
    event.u.selectionNotify.property = None;
    WriteEventsToClient(client, 1, &event);
    return Success;
}
