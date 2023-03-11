/* Include configs for this architecture, generated on compile-time */
#include "nebconf.h"

/* beginning of i18n stuff */
#define ENABLE_NLS 1
#define PACKAGE "nebula"
#define GNOMELOCALEDIR "/usr/share/locale/"
/* end of i18n stuff */

/* #define CONFIG_PATH     "/usr/share/nebula/data/" */
#define GENRE_PATH      "/genres/"
#define RACE_PATH       "/races/"
#define PROFESSION_PATH "/professions/"

#define BASE_XML	"/base/base.xml"

/* Pixmaps */
/* #define PIXMAPS_PATH "/usr/share/nebula/pixmaps/" */

#define D10_PIXMAP	PIXMAPS_PATH  "/d10-16x16.png"
#define IGOR_PIXMAP	PIXMAPS_PATH  "/igor.png"
#define WB_LOGO		PIXMAPS_PATH  "/worldmap-small.png"
#define WB_ICON		PIXMAPS_PATH  "/jigsawpiece-48x48.png"
#define STARDUST_ICON PIXMAPS_PATH  "/stardust-48x48.png"
#define STARDUST_LOGO PIXMAPS_PATH  "/stardust.png"

/* 
#define D10_PIXMAP	"/usr/share/nebula/pixmaps/d10-16x16.png"
#define IGOR_PIXMAP	"/usr/share/nebula/pixmaps/igor.png"
#define WB_LOGO		"/usr/share/nebula/pixmaps/worldmap-small.png"
#define WB_ICON		"/usr/share/nebula/pixmaps/jigsawpiece-48x48.png"
*/

/*
#define CHARACTERDTD    "<!DOCTYPE rmsscharacter SYSTEM "  \
                        "\"http://www.nash.nu/Rolemaster/"  \
                        "XML/rmsscharacter/rmsscharacter.dtd\">\n"
*/
#define CHARACTERDTD    "\n" /* USe this when behind a firewall */
#define SKILLSETDTD     "<!DOCTYPE rmsskillset SYSTEM " \
                        "\"http://www.nash.nu/Rolemaster/"  \
                        "XML/rmssskillset/rmssskillset.dtd\">\n"

#ifndef GCONF_H
#define GCONF_H

void config_init(void);

#endif /* CONF_H */
