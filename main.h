/*
->====================================<-
->= SvTX - © Copyright 2022 OnyxSoft =<-
->====================================<-
->= Version  : 1.0                   =<-
->= File     : main.c                =<-
->= Author   : Stefan Blixth         =<-
->= Compiled : 2022-06-16            =<-
->====================================<-
*/

#ifndef __MAIN_H__
#define __MAIN_H__

/* Standard... */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* System... */
#include <clib/alib_protos.h>
#include <dos/dos.h>
#include <exec/libraries.h>
#include <utility/tagitem.h>
#include <libraries/iffparse.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/icon.h>
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>
#include <libraries/mui.h>
#include <proto/multimedia.h>

#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <proto/datatypes.h>

#include "LinkedList.h"
#include "rendermcc.h"
#include "base64.h"
#include "urlparser.h"


/*=----------------------------- Patches and Macros()-------------------------*
 * Patching some incompatible functions and adds some useful macros           *
 *----------------------------------------------------------------------------*/

/* Compiler specific stuff */

#ifdef __MORPHOS__

#define REG(x)

#ifndef DISPATCHER
#define DISPATCHER(Name) \
static ULONG Name##_Dispatcher(void); \
struct EmulLibEntry GATE ##Name##_Dispatcher = { TRAP_LIB, 0, (void (*)(void)) Name##_Dispatcher }; \
static ULONG Name##_Dispatcher(void) { struct IClass *cl=(struct IClass*)REG_A0; Msg msg=(Msg)REG_A1; Object *obj=(Object*)REG_A2;
#define DISPATCHER_REF(Name) &GATE##Name##_Dispatcher
#define DISPATCHER_END }
#endif

#else

#define REG(x) register __ ## x

#define DISPATCHER(Name) ULONG ASM SAVEDS Name##Dispatcher(REG(a0) struct IClass *cl,REG(a2),REG(a1) Msg msg)
#define DISPATCHER_REF(Name) Name##Dispatcher
#define DISPATCHER_END

#endif

/*=*/

#define VYEARS    "2022"
#define AUTHOR    "Stefan Blixth"
#define EMAIL     "stefan@onyxsoft.se"
#define URL       "www.onyxsoft.se"
#define COMPANY   "OnyxSoft"
#define COPYRIGHT "© " VYEARS " " AUTHOR ", " COMPANY

#define VERSION   "1"
#define REVISION  "0"

#ifndef __AMIGADATE__
 #define __AMIGADATE__   "16.06.22"
#endif

#define DATE            __AMIGADATE__

#define NAME            "SvTX"
#define VERS            "SvTX "VERSION"."REVISION
#define BASE            "SVTX"

#if defined (__MORPHOS__)
   #define VERSSHORT       VERSION"."REVISION" [MorphOS, PowerPC]"
#elif defined (__AROS__)
   #define VERSSHORT       VERSION"."REVISION" [AROS, x86]"
#elif defined (__amigaos4__)
   #define VERSSHORT       VERSION"."REVISION" [AmigaOS, PowerPC]"
#else
   #define VERSSHORT       VERSION"."REVISION" [AmigaOS, 68k]"
#endif


#define VSTRING         VERS" ("DATE") © "VYEARS " " AUTHOR ", " COMPANY
#define VERSTAG         "$VER:" VSTRING
#define VERSTAG_MUI     "$VER: "VERS " ("DATE")"
#define VERSTAG_SCREEN  VERS " ("DATE") "
#define ABOUT           "SvTX is little Teletext-viewer for the Swedish SVT Text-service.\n\nAuthor : "AUTHOR " ("EMAIL")\nVersion : "VERSSHORT"\n\n\33c"URL"\n\n\33c©Copyright "VYEARS " " COMPANY


static const char version[] = VERSTAG;

#ifndef MAKE_ID
 #define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif


/* Global variables... */
static Object *app        = NULL;  // MUI-Application object
static Object *win        = NULL;  // MUI-Window object
static Object *gau_info   = NULL;  // Info-Gauge
static Object *render_obj = NULL;  // Render object
static Object *cyc_index  = NULL;  // Cycle-gadget
static Object *but_prev   = NULL;  // Button
static Object *but_next   = NULL;  // Button
static Object *str_page   = NULL;  // String
static Object *grp_status = NULL;  // Group object

// Enumerated Objects...
enum
{
   ID_MENU_ABOUT = 1,
   ID_MENU_ICONIFY,
   ID_MENU_QUIT,
   ID_MENU_SNAPSHOT,
   ID_MENU_UNSNAPSHOT,
   ID_MENU_PREFERENCES,
   ID_MENU_MUISETTINGS,
   ID_STR_PAGE,
   ID_BUT_PREV,
   ID_BUT_NEXT,
   ID_CYC_INDEX,
};


static char *cycle_index[] =
{
   "\33bInnehåll...",
   "\33l100 - Nyheter",
   "\33l200 - Ekonomi",
   "\33l300 - Sport",
   "\33l330 - Resultatbörsen",
   "\33l377 - Målservice",
   "\33l400 - Väder",
   "\33l500 - Blandat",
   "\33l600 - På TV",
   "\33l700 - Innehåll",
   "\33l800 - UR",   
   NULL,
};


LinkedList llData;
extern struct MapData *iMap;

// Prototypes...
void OpenPageLink(WORD);
void SetGaugeStatus(char *);

#endif // __MAIN_H__