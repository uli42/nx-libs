/* $XdotOrg: xc/programs/Xserver/mi/miinitext.c,v 1.26 2005/07/16 03:49:59 kem Exp $ */
/* $XFree86: xc/programs/Xserver/mi/miinitext.c,v 3.67 2003/01/12 02:44:27 dawes Exp $ */
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
/* $Xorg: miinitext.c,v 1.4 2001/02/09 02:05:21 xorgcvs Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#ifdef HAVE_DMX_CONFIG_H
#include <dmx-config.h>
#endif

#ifdef HAVE_XNEST_CONFIG_H
#include <xnest-config.h>
#undef DPMSExtension
#endif

#include "misc.h"
#include "extension.h"
#include "micmap.h"

#if defined(QNX4) /* sleaze for Watcom on QNX4 ... */
#undef GLXEXT
#endif

extern Bool noTestExtensions;

#ifdef BIGREQS
extern Bool noBigReqExtension;
#endif
#ifdef COMPOSITE
extern Bool noCompositeExtension;
#endif
#ifdef DAMAGE
extern Bool noDamageExtension;
#endif
#ifdef DBE
extern Bool noDbeExtension;
#endif
#ifdef DPSEXT
extern Bool noDPSExtension;
#endif
#ifdef DPMSExtension
extern Bool noDPMSExtension;
#endif
#ifdef GLXEXT
extern Bool noGlxExtension;
#endif
#ifdef SCREENSAVER
extern Bool noScreenSaverExtension;
#endif
#ifdef MITSHM
extern Bool noMITShmExtension;
#endif
#ifdef RANDR
extern Bool noRRExtension;
#endif
#ifdef RENDER
extern Bool noRenderExtension;
#endif
#ifdef SHAPE
extern Bool noShapeExtension;
#endif
#ifdef XCSECURITY
extern Bool noSecurityExtension;
#endif
#ifdef XSYNC
extern Bool noSyncExtension;
#endif
#ifdef RES
extern Bool noResExtension;
#endif
#ifdef XCMISC
extern Bool noXCMiscExtension;
#endif
#ifdef XF86BIGFONT
extern Bool noXFree86BigfontExtension;
#endif
#ifdef XF86DRI
extern Bool noXFree86DRIExtension;
#endif
#ifdef XFIXES
extern Bool noXFixesExtension;
#endif
#ifdef XKB
/* |noXkbExtension| is defined in xc/programs/Xserver/xkb/xkbInit.c */
extern Bool noXkbExtension;
#endif
#ifdef PANORAMIX
extern Bool noPanoramiXExtension;
#endif
#ifdef XINPUT
extern Bool noXInputExtension;
#endif
#ifdef XIDLE
extern Bool noXIdleExtension;
#endif
#ifdef XV
extern Bool noXvExtension;
#endif

#ifndef XFree86LOADER
#define INITARGS void
typedef void (*InitExtension)(INITARGS);
#else /* XFree86Loader */
#include "loaderProcs.h"
#endif

#ifdef MITSHM
#define _XSHM_SERVER_
#include <X11/extensions/shmproto.h>
#endif
#ifdef XTEST
#define _XTEST_SERVER_
#include <nx-X11/extensions/xtestconst.h>
#endif
#ifdef XKB
#include <nx-X11/extensions/XKB.h>
#endif
#ifdef XCSECURITY
#define _SECURITY_SERVER
#include <nx-X11/extensions/securstr.h>
#endif
#ifdef PANORAMIX
#include <nx-X11/extensions/panoramiXproto.h>
#endif
#ifdef XF86BIGFONT
#include <nx-X11/extensions/xf86bigfstr.h>
#endif
#ifdef RES
#include <nx-X11/extensions/XResproto.h>
#endif

/* FIXME: this whole block of externs should be from the appropriate headers */
#ifdef XTESTEXT1
extern void XTestExtension1Init(INITARGS);
#endif
#ifdef SHAPE
extern void ShapeExtensionInit(INITARGS);
#endif
#ifdef MITSHM
extern void ShmExtensionInit(INITARGS);
#endif
#ifdef PANORAMIX
extern void PanoramiXExtensionInit(INITARGS);
#endif
#ifdef XINPUT
extern void XInputExtensionInit(INITARGS);
#endif
#ifdef XTEST
extern void XTestExtensionInit(INITARGS);
#endif
#ifdef BIGREQS
extern void BigReqExtensionInit(INITARGS);
#endif
#ifdef XIDLE
extern void XIdleExtensionInit(INITARGS);
#endif
#ifdef SCREENSAVER
extern void ScreenSaverExtensionInit (INITARGS);
#endif
#ifdef XV
extern void XvExtensionInit(INITARGS);
extern void XvMCExtensionInit(INITARGS);
#endif
#ifdef XSYNC
extern void SyncExtensionInit(INITARGS);
#endif
#ifdef XKB
extern void XkbExtensionInit(INITARGS);
#endif
#ifdef XCMISC
extern void XCMiscExtensionInit(INITARGS);
#endif
#ifdef XRECORD
extern void RecordExtensionInit(INITARGS);
#endif
#ifdef DBE
extern void DbeExtensionInit(INITARGS);
#endif
#ifdef XCSECURITY
extern void SecurityExtensionInit(INITARGS);
#endif
#ifdef XF86BIGFONT
extern void XFree86BigfontExtensionInit(INITARGS);
#endif
#ifdef GLXEXT
#ifndef __DARWIN__
extern void GlxExtensionInit(INITARGS);
extern void GlxWrapInitVisuals(miInitVisualsProcPtr *);
#else
extern void DarwinGlxExtensionInit(INITARGS);
extern void DarwinGlxWrapInitVisuals(miInitVisualsProcPtr *);
#endif
#endif
#ifdef XF86DRI
extern void XFree86DRIExtensionInit(INITARGS);
#endif
#ifdef DPMSExtension
extern void DPMSExtensionInit(INITARGS);
#endif
#ifdef DPSEXT
extern void DPSExtensionInit(INITARGS);
#endif
#ifdef RENDER
extern void RenderExtensionInit(INITARGS);
#endif
#ifdef RANDR
extern void RRExtensionInit(INITARGS);
#endif
#ifdef RES
extern void ResExtensionInit(INITARGS);
#endif
#ifdef DMXEXT
extern void DMXExtensionInit(INITARGS);
#endif
#ifdef XFIXES
extern void XFixesExtensionInit(INITARGS);
#endif
#ifdef DAMAGE
extern void DamageExtensionInit(INITARGS);
#endif
#ifdef COMPOSITE
extern void CompositeExtensionInit(INITARGS);
#endif

/* The following is only a small first step towards run-time
 * configurable extensions.
 */
typedef struct {
    char *name;
    Bool *disablePtr;
} ExtensionToggle;

static ExtensionToggle ExtensionToggleList[] =
{
    /* sort order is extension name string as shown in xdpyinfo */
#ifdef BIGREQS
    { "BIG-REQUESTS", &noBigReqExtension },
#endif
#ifdef COMPOSITE
    { "Composite", &noCompositeExtension },
#endif
#ifdef DAMAGE
    { "DAMAGE", &noDamageExtension },
#endif
#ifdef DBE
    { "DOUBLE-BUFFER", &noDbeExtension },
#endif
#ifdef DPSEXT
    { "DPSExtension", &noDPSExtension },
#endif
#ifdef DPMSExtension
    { "DPMS", &noDPMSExtension },
#endif
#ifdef GLXEXT
    { "GLX", &noGlxExtension },
#endif
#ifdef SCREENSAVER
    { "MIT-SCREEN-SAVER", &noScreenSaverExtension },
#endif
#ifdef MITSHM
    { SHMNAME, &noMITShmExtension },
#endif
#ifdef RANDR
    { "RANDR", &noRRExtension },
#endif
#ifdef RENDER
    { "RENDER", &noRenderExtension },
#endif
#ifdef SHAPE
    { "SHAPE", &noShapeExtension },
#endif
#ifdef XCSECURITY
    { "SECURITY", &noSecurityExtension },
#endif
#ifdef XSYNC
    { "SYNC", &noSyncExtension },
#endif
#ifdef RES
    { "X-Resource", &noResExtension },
#endif
#ifdef XCMISC
    { "XC-MISC", &noXCMiscExtension },
#endif
#ifdef XF86BIGFONT
    { "XFree86-Bigfont", &noXFree86BigfontExtension },
#endif
#ifdef XF86DRI
    { "XFree86-DRI", &noXFree86DRIExtension },
#endif
#ifdef XFIXES
    { "XFIXES", &noXFixesExtension },
#endif
#ifdef PANORAMIX
    { "XINERAMA", &noPanoramiXExtension },
#endif
#ifdef XINPUT
    { "XInputExtension", &noXInputExtension },
#endif
#ifdef XKB
    { "XKEYBOARD", &noXkbExtension },
#endif
    { "XTEST", &noTestExtensions },
#ifdef XV
    { "XVideo", &noXvExtension },
#endif
    { NULL, NULL }
};

Bool EnableDisableExtension(char *name, Bool enable)
{
    ExtensionToggle *ext = &ExtensionToggleList[0];

    for (ext = &ExtensionToggleList[0]; ext->name != NULL; ext++) {
	if (strcmp(name, ext->name) == 0) {
	    *ext->disablePtr = !enable;
	    return TRUE;
	}
    }

    return FALSE;
}

void EnableDisableExtensionError(char *name, Bool enable)
{
    ExtensionToggle *ext = &ExtensionToggleList[0];

    ErrorF("Extension \"%s\" is not recognized\n", name);
    ErrorF("Only the following extensions can be run-time %s:\n",
	   enable ? "enabled" : "disabled");
    for (ext = &ExtensionToggleList[0]; ext->name != NULL; ext++)
	ErrorF("    %s\n", ext->name);
}

#ifndef XFree86LOADER

/*ARGSUSED*/
void
InitExtensions(argc, argv)
    int		argc;
    char	*argv[];
{
#ifdef PANORAMIX
# if !defined(PRINT_ONLY_SERVER) && !defined(NO_PANORAMIX)
  if (!noPanoramiXExtension) PanoramiXExtensionInit();
# endif
#endif
#ifdef XTESTEXT1
    if (!noTestExtensions) XTestExtension1Init();
#endif
#ifdef SHAPE
    if (!noShapeExtension) ShapeExtensionInit();
#endif
#ifdef MITSHM
    if (!noMITShmExtension) ShmExtensionInit();
#endif
#if defined(XINPUT) && !defined(NO_HW_ONLY_EXTS)
    if (!noXInputExtension) XInputExtensionInit();
#endif
#ifdef XTEST
    if (!noTestExtensions) XTestExtensionInit();
#endif
#ifdef BIGREQS
    if (!noBigReqExtension) BigReqExtensionInit();
#endif
#ifdef XIDLE
    if (!noXIdleExtension) XIdleExtensionInit();
#endif
#if defined(SCREENSAVER) && !defined(PRINT_ONLY_SERVER)
    if (!noScreenSaverExtension) ScreenSaverExtensionInit ();
#endif
#ifdef XV
    if (!noXvExtension) {
      XvExtensionInit();
      XvMCExtensionInit();
    }
#endif
#ifdef XSYNC
    if (!noSyncExtension) SyncExtensionInit();
#endif
#if defined(XKB) && !defined(PRINT_ONLY_SERVER) && !defined(NO_HW_ONLY_EXTS)
    if (!noXkbExtension) XkbExtensionInit();
#endif
#ifdef XCMISC
    if (!noXCMiscExtension) XCMiscExtensionInit();
#endif
#ifdef XRECORD
    if (!noTestExtensions) RecordExtensionInit(); 
#endif
#ifdef DBE
    if (!noDbeExtension) DbeExtensionInit();
#endif
#ifdef XCSECURITY
    if (!noSecurityExtension) SecurityExtensionInit();
#endif
#if defined(DPMSExtension) && !defined(NO_HW_ONLY_EXTS)
    if (!noDPMSExtension) DPMSExtensionInit();
#endif
#ifdef XF86BIGFONT
    if (!noXFree86BigfontExtension) XFree86BigfontExtensionInit();
#endif
#if !defined(PRINT_ONLY_SERVER) && !defined(NO_HW_ONLY_EXTS)
#ifdef XF86DRI
    if (!noXFree86DRIExtension) XFree86DRIExtensionInit();
#endif
#endif
#ifdef GLXEXT
#ifndef __DARWIN__
    if (!noGlxExtension) GlxExtensionInit();
#else
    if (!noGlxExtension) DarwinGlxExtensionInit();
#endif
#endif
#ifdef XFIXES
    /* must be before Render to layer DisplayCursor correctly */
    if (!noXFixesExtension) XFixesExtensionInit();
#endif
#ifdef RENDER
    if (!noRenderExtension) RenderExtensionInit();
#endif
#ifdef RANDR
    if (!noRRExtension) RRExtensionInit();
#endif
#ifdef RES
    if (!noResExtension) ResExtensionInit();
#endif
#ifdef DMXEXT
    DMXExtensionInit(); /* server-specific extension, cannot be disabled */
#endif
#ifdef COMPOSITE
    if (!noCompositeExtension) CompositeExtensionInit();
#endif
#ifdef DAMAGE
    if (!noDamageExtension) DamageExtensionInit();
#endif
}

void
InitVisualWrap()
{
    miResetInitVisuals();
#ifdef GLXEXT
#ifndef __DARWIN__
    GlxWrapInitVisuals(&miInitVisualsProc);
#else
    DarwinGlxWrapInitVisuals(&miInitVisualsProc);
#endif
#endif
}

#else /* XFree86LOADER */
/* List of built-in (statically linked) extensions */
static ExtensionModule staticExtensions[] = {
#ifdef XTESTEXT1
    { XTestExtension1Init, "XTEST1", &noTestExtensions, NULL, NULL },
#endif
#ifdef MITSHM
    { ShmExtensionInit, SHMNAME, &noMITShmExtension, NULL, NULL },
#endif
#ifdef XINPUT
    { XInputExtensionInit, "XInputExtension", &noXInputExtension, NULL, NULL },
#endif
#ifdef XTEST
    { XTestExtensionInit, XTestExtensionName, &noTestExtensions, NULL, NULL },
#endif
#ifdef XIDLE
    { XIdleExtensionInit, "XIDLE", &noXIdleExtension, NULL, NULL },
#endif
#ifdef XKB
    { XkbExtensionInit, XkbName, &noXkbExtension, NULL, NULL },
#endif
#ifdef XCSECURITY
    { SecurityExtensionInit, SECURITY_EXTENSION_NAME, &noSecurityExtension, NULL, NULL },
#endif
#ifdef PANORAMIX
    { PanoramiXExtensionInit, PANORAMIX_PROTOCOL_NAME, &noPanoramiXExtension, NULL, NULL },
#endif
#ifdef XFIXES
    /* must be before Render to layer DisplayCursor correctly */
    { XFixesExtensionInit, "XFIXES", &noXFixesExtension, NULL, NULL },
#endif
#ifdef XF86BIGFONT
    { XFree86BigfontExtensionInit, XF86BIGFONTNAME, &noXFree86BigfontExtension, NULL, NULL },
#endif
#ifdef RENDER
    { RenderExtensionInit, "RENDER", &noRenderExtension, NULL, NULL },
#endif
#ifdef RANDR
    { RRExtensionInit, "RANDR", &noRRExtension, NULL, NULL },
#endif
#ifdef COMPOSITE
    { CompositeExtensionInit, "COMPOSITE", &noCompositeExtension, NULL },
#endif
#ifdef DAMAGE
    { DamageExtensionInit, "DAMAGE", &noDamageExtension, NULL },
#endif
    { NULL, NULL, NULL, NULL, NULL }
};
    
/*ARGSUSED*/
void
InitExtensions(argc, argv)
    int		argc;
    char	*argv[];
{
    int i;
    ExtensionModule *ext;
    static Bool listInitialised = FALSE;

    if (!listInitialised) {
	/* Add built-in extensions to the list. */
	for (i = 0; staticExtensions[i].name; i++)
	    LoadExtension(&staticExtensions[i], TRUE);

	/* Sort the extensions according the init dependencies. */
	LoaderSortExtensions();
	listInitialised = TRUE;
    }

    for (i = 0; ExtensionModuleList[i].name != NULL; i++) {
	ext = &ExtensionModuleList[i];
	if (ext->initFunc != NULL && 
	    (ext->disablePtr == NULL || 
	     (ext->disablePtr != NULL && !*ext->disablePtr))) {
	    (ext->initFunc)();
	}
    }
}

static void (*__miHookInitVisualsFunction)(miInitVisualsProcPtr *);

void
InitVisualWrap()
{
    miResetInitVisuals();
    if (__miHookInitVisualsFunction)
	(*__miHookInitVisualsFunction)(&miInitVisualsProc);
}

void miHookInitVisuals(void (**old)(miInitVisualsProcPtr *),
		       void (*new)(miInitVisualsProcPtr *))
{
    if (old)
	*old = __miHookInitVisualsFunction;
    __miHookInitVisualsFunction = new;
}

#endif /* XFree86LOADER */
