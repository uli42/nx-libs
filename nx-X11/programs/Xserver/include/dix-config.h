/* */
#ifndef _DIX_CONFIG_H_
#define _DIX_CONFIG_H_

/* Build GLX extension */
/* is set via imake
  #define GLXEXT
*/
/* Build GLX DRI loader */
#define GLX_DRI

/* Path to DRI drivers */
//#define DRI_DRIVER_PATH "/usr/lib/x86_64-linux-gnu/dri"
#define DRI_DRIVER_PATH "/tmp/mesa792build/lib/dri"

/* Support DRI extension */
#undef XF86DRI

/* don't build DRI2 extension for now */
#define DRI2

/* FIXME: use some NX installation path here */
#define SERVER_MISC_CONFIG_PATH "/tmp"

/* enable xorg_backtrace function */
#define HAVE_BACKTRACE

#endif
