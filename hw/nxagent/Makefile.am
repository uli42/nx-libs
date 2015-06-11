#FIXME: add missing stuff from Imakefile.NX.original

bin_PROGRAMS = nxagent

SRCS =  NXwindow.c \
	NXevents.c \
	NXproperty.c \
	NXdixfonts.c \
	NXglyphcurs.c \
	NXdispatch.c \
	NXrender.c \
	NXglyph.c \
	NXpicture.c \
	NXextension.c \
	NXshm.c \
        NXglxext.c \
        NXxvdisp.c \
        NXmiglyph.c \
        NXmiexpose.c \
        NXresource.c \
        NXmiwindow.c \
	NXxrandr.c \
	NXdamage.c \
	NXmitrap.c \
	Args.c \
	Binder.c \
	Colormap.c \
	Cursor.c \
	Dialog.c \
	Display.c \
	Events.c \
	Font.c \
	GC.c \
	GCOps.c \
	Millis.c \
	Handlers.c \
	Init.c \
	Keyboard.c \
	Keystroke.c \
	Pointer.c \
	Screen.c \
	TestExt.c \
	Visual.c \
	Drawable.c \
	Window.c \
	Pixmap.c \
	Render.c \
	Client.c \
	Rootless.c \
	Extensions.c \
	Options.c \
	Clipboard.c \
	Splash.c \
	Split.c \
	Holder.c \
        Reconnect.c \
	Error.c \
	Atoms.c \
	Trap.c \
	Image.c \
	Composite.c \
	Pixels.c \
	stubs.c \
	miinitext.c \
	$(top_srcdir)/Xext/dpmsstubs.c \
	$(top_srcdir)/Xi/stubs.c \
	$(top_srcdir)/mi/miinitext.c \
	$(top_srcdir)/fb/fbcmap.c

nxagent_SOURCES = $(SRCS)

nxagent_LDADD = $(XORG_CORE_LIBS) \
              $(NXAGENT_LIBS) \
              $(NXAGENTMODULES_LIBS)

nxagent_LDFLAGS =

AM_CFLAGS = -DHAVE_NXAGENT_CONFIG_H \
            -DNO_HW_ONLY_EXTS \
            @SERVER_DEFINES@ \
            $(NXAGENTMODULES_CFLAGS)

EXTRA_DIST = os2Stub.c \
             icon \
             screensaver \
	     nxagent.man.pre

# -UDPMSExtension for miinitext? can't put into 
# OS_DEFINES???
# EXT_DEFINES???
# ICONFIGFILES -- SpecialCObjectRule

# Man page
include $(top_srcdir)/cpprules.in

appmandir = $(APP_MAN_DIR)

appman_PRE = nxagent.man
appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)

EXTRAMANDEFS = \
	-D__XCONFIGFILE__=$(__XCONFIGFILE__) \
	-D__XSERVERNAME__=$(XSERVERNAME)

BUILT_SOURCES = $(appman_PRE)
CLEANFILES = $(appman_PRE) $(appman_DATA)

SUFFIXES += .$(APP_MAN_SUFFIX) .man

.man.$(APP_MAN_SUFFIX):
	-rm -f $@
	$(LN_S) $< $@


