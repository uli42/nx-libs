if DMX
DMX_SUBDIRS = dmx
endif

if XORG
XORG_SUBDIRS = xfree86
endif

if XVFB
XVFB_SUBDIRS = vfb
endif

if XNEST
XNEST_SUBDIRS = xnest
endif

if NXAGENT
NXAGENT_SUBDIRS = nxagent
endif

if XWIN
XWIN_SUBDIRS = xwin
endif

# need to add darwin support here

SUBDIRS =			\
	$(XORG_SUBDIRS)		\
	$(XWIN_SUBDIRS)		\
	$(XVFB_SUBDIRS)		\
	$(XNEST_SUBDIRS)	\
	$(NXAGENT_SUBDIRS)	\
	$(DMX_SUBDIRS)

DIST_SUBDIRS = dmx xfree86 vfb xnest xwin darwin
