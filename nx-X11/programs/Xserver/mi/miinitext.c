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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#ifdef HAVE_DMX_CONFIG_H
#include <dmx-config.h>
#undef XV
#undef DBE
#undef XF86VIDMODE
#undef XFreeXDGA
#undef XF86DRI
#undef SCREENSAVER
#undef RANDR
#undef XFIXES
#undef DAMAGE
#undef COMPOSITE
#undef MITSHM
#endif

#ifdef HAVE_XNEST_CONFIG_H
#include <xnest-config.h>
#undef COMPOSITE
#undef DPMSExtension
#endif



#include "misc.h"
#include "extension.h"
#include "micmap.h"
#include "globals.h"


extern Bool noTestExtensions;

#ifdef COMPOSITE
extern Bool noCompositeExtension;
#endif
#ifdef DBE
extern Bool noDbeExtension;
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
extern Bool noRenderExtension;
#ifdef XCSECURITY
extern Bool noSecurityExtension;
#endif
#ifdef RES
extern Bool noResExtension;
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
#ifdef PANORAMIX
extern Bool noPanoramiXExtension;
#endif
#ifdef XIDLE
extern Bool noXIdleExtension;
#endif
#ifdef XV
extern Bool noXvExtension;
#endif
extern Bool noGEExtension;

#define INITARGS void
typedef void (*InitExtension)(void);

#ifdef MITSHM
#include <nx-X11/extensions/shm.h>
#endif
#ifdef XTEST
#include <nx-X11/extensions/xtestconst.h>
#endif
#include "../xkb/xkb.h"
#ifdef XCSECURITY
#include "securitysrv.h"
#include <nx-X11/extensions/secur.h>
#endif
#ifdef PANORAMIX
#include <nx-X11/extensions/panoramiXproto.h>
#endif
#ifdef XF86BIGFONT
#include <nx-X11/extensions/xf86bigfproto.h>
#endif
#ifdef RES
#include <nx-X11/extensions/XResproto.h>
#endif

/* FIXME: this whole block of externs should be from the appropriate headers */
#ifdef MITSHM
extern void ShmExtensionInit(void);
#endif
#ifdef PANORAMIX
extern void PanoramiXExtensionInit(void);
#endif
extern void XInputExtensionInit(void);
#ifdef XTEST
extern void XTestExtensionInit(void);
#endif
extern void BigReqExtensionInit(void);
#ifdef SCREENSAVER
extern void ScreenSaverExtensionInit (void);
#endif
#ifdef XV
extern void XvExtensionInit(void);
extern void XvMCExtensionInit(void);
#endif
extern void SyncExtensionInit(void);
extern void XkbExtensionInit(void);
extern void XCMiscExtensionInit(void);
#ifdef XRECORD
extern void RecordExtensionInit(void);
#endif
#ifdef DBE
extern void DbeExtensionInit(void);
#endif
#ifdef XCSECURITY
extern void SecurityExtensionInit(void);
#endif
#ifdef XSELINUX
extern void SELinuxExtensionInit(void);
#endif
#ifdef XF86BIGFONT
extern void XFree86BigfontExtensionInit(void);
#endif
#ifdef GLXEXT
typedef struct __GLXprovider __GLXprovider;
extern __GLXprovider __glXDRISWRastProvider;
extern void GlxPushProvider(__GLXprovider *impl);
extern void GlxExtensionInit(void);
#endif
#ifdef XF86DRI
extern void XFree86DRIExtensionInit(void);
#endif
#ifdef DPMSExtension
extern void DPMSExtensionInit(void);
#endif
extern void RenderExtensionInit(void);
#ifdef RANDR
extern void RRExtensionInit(void);
#endif
#ifdef RES
extern void ResExtensionInit(void);
#endif
#ifdef DMXEXT
extern void DMXExtensionInit(void);
#endif
#ifdef XFIXES
extern void XFixesExtensionInit(void);
#endif
#ifdef DAMAGE
extern void DamageExtensionInit(void);
#endif
#ifdef COMPOSITE
extern void CompositeExtensionInit(void);
#endif
extern void GEExtensionInit(void);

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
    { "Generic Events", &noGEExtension },
#ifdef COMPOSITE
    { "Composite", &noCompositeExtension },
#endif
#ifdef DAMAGE
    { "DAMAGE", &noDamageExtension },
#endif
#ifdef DBE
    { "DOUBLE-BUFFER", &noDbeExtension },
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
    { "RENDER", &noRenderExtension },
#ifdef XCSECURITY
    { "SECURITY", &noSecurityExtension },
#endif
#ifdef RES
    { "X-Resource", &noResExtension },
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
    { "XInputExtension", NULL },
    { "XKEYBOARD", NULL },
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
	    if (ext->disablePtr != NULL) {
	    *ext->disablePtr = !enable;
	    return TRUE;
	    } else {
		/* Extension is always on, impossible to disable */
		return enable; /* okay if they wanted to enable,
				  fail if they tried to disable */
	    }
	}
    }

    return FALSE;
}

void EnableDisableExtensionError(char *name, Bool enable)
{
    ExtensionToggle *ext = &ExtensionToggleList[0];
    Bool found = FALSE;

    for (ext = &ExtensionToggleList[0]; ext->name != NULL; ext++) {
	if ((strcmp(name, ext->name) == 0) && (ext->disablePtr == NULL)) {
	    ErrorF("[mi] Extension \"%s\" can not be disabled\n", name);
	    found = TRUE;
	    break;
	}
    }
    if (found == FALSE)
	ErrorF("[mi] Extension \"%s\" is not recognized\n", name);
    ErrorF("[mi] Only the following extensions can be run-time %s:\n",
	   enable ? "enabled" : "disabled");
    for (ext = &ExtensionToggleList[0]; ext->name != NULL; ext++) {
	if (ext->disablePtr != NULL) {
	    ErrorF("[mi]    %s\n", ext->name);
	}
    }
}


/*ARGSUSED*/
void
InitExtensions(int argc, char *argv[])
{
    if (!noGEExtension) GEExtensionInit();

#ifdef PANORAMIX
# if !defined(NO_PANORAMIX)
  if (!noPanoramiXExtension) PanoramiXExtensionInit();
# endif
#endif
    ShapeExtensionInit();
#ifdef MITSHM
    if (!noMITShmExtension) ShmExtensionInit();
#endif
    XInputExtensionInit();
#ifdef XTEST
    if (!noTestExtensions) XTestExtensionInit();
#endif
    BigReqExtensionInit();
#if defined(SCREENSAVER)
    if (!noScreenSaverExtension) ScreenSaverExtensionInit ();
#endif
#ifdef XV
    if (!noXvExtension) {
      XvExtensionInit();
      XvMCExtensionInit();
    }
#endif
    SyncExtensionInit();
    XkbExtensionInit();
    XCMiscExtensionInit();
#ifdef XRECORD
    if (!noTestExtensions) RecordExtensionInit();
#endif
#ifdef DBE
    if (!noDbeExtension) DbeExtensionInit();
#endif
#ifdef XCSECURITY
    if (!noSecurityExtension) SecurityExtensionInit();
#endif
#ifdef XSELINUX
    if (!noSELinuxExtension) SELinuxExtensionInit();
#endif
#if defined(DPMSExtension) && !defined(NO_HW_ONLY_EXTS)
    if (!noDPMSExtension) DPMSExtensionInit();
#endif
#ifdef XF86BIGFONT
    if (!noXFree86BigfontExtension) XFree86BigfontExtensionInit();
#endif
#if !defined(NO_HW_ONLY_EXTS)
#ifdef XF86DRI
    if (!noXFree86DRIExtension) XFree86DRIExtensionInit();
#endif
#endif
#ifdef XFIXES
    /* must be before Render to layer DisplayCursor correctly */
    if (!noXFixesExtension) XFixesExtensionInit();
#endif
    if (!noRenderExtension) RenderExtensionInit();
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

#ifdef GLXEXT
#if 0
    if (serverGeneration == 1)
    GlxPushProvider(&__glXDRISWRastProvider);
    if (!noGlxExtension) GlxExtensionInit();
#endif
#endif
}

