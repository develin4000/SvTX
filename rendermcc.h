/*
->====================================<-
->= SvTX - © Copyright 2022 OnyxSoft =<-
->====================================<-
->= Version  : 1.0                   =<-
->= File     : rendermcc.h           =<-
->= Author   : Stefan Blixth         =<-
->= Compiled : 2022-06-16            =<-
->====================================<-
*/

#ifndef __RENDERMCC_H__
#define __RENDERMCC_H__

struct Picture
{
   WORD width;
   WORD height;
   WORD depth;
   struct BitMap *bmap;
};

struct RenderData
{
   BOOL   active;
   UBYTE  *imgdata;
   LONG   imgsize;
   LONG   decode_size; 
   UBYTE  *decdata;

   struct Picture *picdata;
   struct Screen *screen;
};

#define SERIALNUMBER        (1)
#define TAGBASE_DEVELIN     (TAG_USER | (SERIALNUMBER<<16))
#define MUIM_TriggerRedraw  (TAGBASE_DEVELIN | 0x0001)
#define MUIM_DecodeImage    (TAGBASE_DEVELIN | 0x0002)
#define MUIM_SetDecdata     (TAGBASE_DEVELIN | 0x0003)
#define MUIM_SetDecsize     (TAGBASE_DEVELIN | 0x0004)


// Prototypes...
void Cleanup_Render(struct MUI_CustomClass *);
struct MUI_CustomClass *Init_Render(void);


// Global variable...
static struct MUI_CustomClass *render_mcc = NULL;
ULONG render_left, render_top;

#endif // __RENDERMCC_H__
