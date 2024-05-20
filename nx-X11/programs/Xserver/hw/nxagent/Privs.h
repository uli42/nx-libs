
#ifndef __NXPRIVS_H__
#define __NXPRIVS_H__

/* Client */
extern DevPrivateKeyRec nxagentClientPrivateKeyRec;
#define nxagentClientPrivateKey (&nxagentClientPrivateKeyRec)

/* GC */
extern DevPrivateKeyRec nxagentGCPrivateKeyRec;
#define nxagentGCPrivateKey (&nxagentGCPrivateKeyRec)

/* Pixmap */
extern DevPrivateKeyRec nxagentPixmapPrivateKeyRec;
#define nxagentPixmapPrivateKey (&nxagentPixmapPrivateKeyRec)

/* Picture */
extern DevPrivateKeyRec nxagentPicturePrivateKeyRec;
#define nxagentPicturePrivateKey (&nxagentPicturePrivateKeyRec)

/* Window */
extern DevPrivateKeyRec nxagentWindowPrivateKeyRec;
#define nxagentWindowPrivateKey (&nxagentWindowPrivateKeyRec)

#endif
