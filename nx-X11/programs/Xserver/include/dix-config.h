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
#define DRI_DRIVER_PATH "/usr/lib/x86_64-linux-gnu/dri"

/* Support DRI extension */
#undef XF86DRI

/* Build DRI2 extension */
#define DRI2

#endif
