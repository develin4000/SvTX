/*
->====================================<-
->= SvTX - © Copyright 2022 OnyxSoft =<-
->====================================<-
->= Version  : 1.0                   =<-
->= File     : rendermcc.c           =<-
->= Author   : Stefan Blixth         =<-
->= Compiled : 2022-06-16            =<-
->====================================<-
*/

#include "main.h"

/*=----------------------------- Render_New() --------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static ULONG Render_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
   struct RenderData *data;

   obj = DoSuperNew(cl, obj,
         InnerSpacing(0, 0),
         MUIA_Frame,        MUIV_Frame_None,
         MUIA_Background,   MUII_WindowBack,
         MUIA_FillArea,     TRUE,
         MUIA_DoubleBuffer, TRUE,
         TAG_MORE,          msg->ops_AttrList);

   if (!obj)
   {
      return(0);
   }

   data = (struct RenderData *)INST_DATA(cl, obj);

   data->active      = FALSE;
   data->imgdata     = NULL; // Imageptr after decoding
   data->imgsize     = 0;    // Size of the decoded image data
   data->screen      = NULL;
   data->picdata     = NULL;
   data->decode_size = 0; 
   data->decdata     = NULL;

   return((ULONG)obj);
}
/*=*/

/*=----------------------------- Render_Dispose() ----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static ULONG Render_Dispose(struct IClass *cl, Object *obj, Msg msg)
{
   struct RenderData *data = (struct RenderData *)INST_DATA(cl, obj);

   if (data->decdata)
      FreeVec(data->decdata);

   if (data->imgdata)
      FreeVec(data->imgdata);

   if (data->picdata)
      FreeVec(data->picdata);

   return(DoSuperMethodA(cl, obj, msg));
}
/*=*/

/*=----------------------------- Render_Setup() ------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static ULONG Render_Setup(struct IClass *cl, Object *obj, Msg msg)
{
   if (!DoSuperMethodA(cl, obj, msg))
      return(FALSE);

   MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS);
   return(TRUE);
}
/*=*/


/*=----------------------------- Render_Cleanup() ----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static ULONG Render_Cleanup(struct IClass *cl, Object *obj, Msg msg)
{
   MUI_RejectIDCMP(obj, IDCMP_MOUSEBUTTONS);
   return(DoSuperMethodA(cl, obj, (Msg)msg));
}
/*=*/

/*=----------------------------- Render_Askminmax() --------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static ULONG Render_Askminmax(struct IClass *cl, Object *obj, struct MUIP_AskMinMax *msg)
{
   struct RenderData *data = (struct RenderData *)INST_DATA(cl, obj);

   DoSuperMethodA(cl, obj, (Msg)msg);

   msg->MinMaxInfo->MinWidth  += 520+12;   // 12 pixels extra to get the image data aligned correct in the window =)
   msg->MinMaxInfo->DefWidth  += 520+12;
   msg->MinMaxInfo->MinHeight += 400;
   msg->MinMaxInfo->DefHeight += 400;
   msg->MinMaxInfo->MaxWidth  += 520+12;
   msg->MinMaxInfo->MaxHeight += 400;

   return(0);
}
/*=*/


/*=----------------------------- Render_Draw() -------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static ULONG Render_Draw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg)
{
   struct RenderData *data = (struct RenderData *)INST_DATA(cl, obj);
   struct RastPort *rp = _rp(obj);

   render_left = _mleft(obj);
   render_top  = _mtop(obj);

   DoSuperMethodA(cl, obj, (Msg)msg);

   if ((msg->flags & MADF_DRAWUPDATE)) // Triggered draw update from function
   {
      return(0);
   }
   else if (msg->flags & MADF_DRAWOBJECT)
   {
      if (data->active)
      {
         FillPixelArray(rp, render_left, render_top, 532, 400, 0x00000000);
         WritePixelArray(data->imgdata, 0, 0, (520*4), _rp(obj), _mleft(obj), _mtop(obj), 520, _mheight(obj), RECTFMT_ARGB);
      }
      else
      {
         FillPixelArray(rp, render_left, render_top, 532, 400, 0x00000000);
      }
   }

   return(0);
}
/*=*/

/*=----------------------------- Render_Show ---------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static ULONG Render_Show(struct IClass *cl, Object *obj, Msg msg)
{
   struct RenderData *data = (struct RenderData *)INST_DATA(cl, obj);

   data->screen = _screen(obj);
   return(DoSuperMethodA(cl, obj, (Msg)msg));
}
/*=*/

/*=----------------------------- Render_Event() ------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static ULONG Render_Input(struct IClass *cl, Object *obj, struct MUIP_HandleInput *msg)
{
   #define _between(a,x,b) ((x)>=(a) && (x)<=(b))
   #define _isinobject(x,y) (_between(_mleft(obj),(x),_mright(obj)) && _between(_mtop(obj),(y),_mbottom(obj)))
   ULONG numb = 0, lpos;
   struct MapData *xMap = NULL;

   if (msg->imsg)
   {
      switch (msg->imsg->Class)
      {
         case IDCMP_MOUSEBUTTONS:
         {
            if (msg->imsg->Code == SELECTDOWN)
            {
               if (_isinobject(msg->imsg->MouseX, msg->imsg->MouseY))
               {
                  for (lpos = 0; lpos < llData.len; lpos++)
                  {
                     xMap = llGet(&llData, lpos);

                     if(_between(xMap->x1, (msg->imsg->MouseX - render_left), xMap->x2))
                     {
                        if(_between(xMap->y1, (msg->imsg->MouseY - render_top), xMap->y2))
                        {
                           OpenPageLink(xMap->pos);
                           break;
                        }
                     }
                  }
               }
            }
         } break;
      }
   }

   return(DoSuperMethodA(cl, obj, (Msg)msg));
}
/*=*/

/*=----------------------------- Render_Trigger() ----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static ULONG Render_Trigger(struct IClass *cl, Object *obj)
{
   MUI_Redraw(obj, MADF_DRAWUPDATE);
   return (TRUE);
}
/*=*/

/*=----------------------------- Render_Decode() -----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static ULONG Render_Decode(struct IClass *cl, Object *obj)
{
   Object *dtobj = NULL;
   struct BitMapHeader *bmhead = NULL;
   struct RenderData *data = (struct RenderData *)INST_DATA(cl, obj);
   
   dtobj = NewDTObject(NULL,
                       DTA_SourceType,        DTST_MEMORY,
                       DTA_SourceAddress,     data->decdata,
                       DTA_SourceSize,        data->decode_size,
                       DTA_GroupID,           GID_PICTURE,
                       PDTA_FreeSourceBitMap, TRUE,
                       PDTA_Screen,           data->screen,
                       PDTA_DestMode,         PMODE_V43,
                       TAG_DONE);

   if(!dtobj)
      return 0;

   if( GetDTAttrs(dtobj, PDTA_BitMapHeader, &bmhead, TAG_DONE) == 0 )
   {
      DisposeDTObject(dtobj);
      return 0;
   }

   if (data->picdata)
   {
      FreeVec(data->picdata);
      data->picdata = NULL;
   }

   data->picdata = AllocVec(sizeof(struct Picture), MEMF_ANY);
   
   data->picdata->width  = bmhead->bmh_Width;
   data->picdata->height = bmhead->bmh_Height;
   data->picdata->depth  = bmhead->bmh_Depth;

   data->picdata->bmap = AllocBitMap(bmhead->bmh_Width, bmhead->bmh_Height, 8, BMF_MINPLANES|BMF_DISPLAYABLE, NULL);

   if(!data->picdata->bmap)
   {
      DisposeDTObject(dtobj);
      return 0;
   }

   if (data->imgdata)
   {
      FreeVec(data->imgdata);
      data->imgdata = NULL;
   }

   data->imgdata = AllocVec(data->picdata->width*data->picdata->height*4, MEMF_CLEAR);

   if(!data->imgdata)
   {
      DisposeDTObject(dtobj);
      FreeBitMap(data->picdata->bmap);
      return 0;
   }

   data->active = TRUE;

   DoMethod(dtobj, PDTM_READPIXELARRAY, data->imgdata, PBPAFMT_ARGB, data->picdata->width<<2, 0, 0, data->picdata->width, data->picdata->height);
   DisposeDTObject(dtobj);
   FreeBitMap(data->picdata->bmap);
   return 1;
}
/*=*/

/*=----------------------------- Render_Setdata() ----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static ULONG Render_Setdata(struct IClass *cl, Object *obj)
{
   struct RenderData *data = (struct RenderData *)INST_DATA(cl, obj);

   data->decdata = GetDecDatabuf();

   return 0;
}
/*=*/

/*=----------------------------- Render_Setsize() ----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static ULONG Render_Setsize(struct IClass *cl, Object *obj)
{
   struct RenderData *data = (struct RenderData *)INST_DATA(cl, obj);

   data->decode_size = GetDecodedSize();

   return 0;
}
/*=*/

/*=----------------------------- DISPATCHER ----------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
DISPATCHER(Render)
{
   switch (msg->MethodID)
   {
      case OM_NEW             : return Render_New       (cl, obj, (APTR)msg); break;
      case OM_DISPOSE         : return Render_Dispose   (cl, obj, (APTR)msg); break;
      case MUIM_Setup         : return Render_Setup     (cl, obj, (APTR)msg); break;
      case MUIM_Cleanup       : return Render_Cleanup   (cl, obj, (APTR)msg); break;
      case MUIM_AskMinMax     : return Render_Askminmax (cl, obj, (APTR)msg); break;
      case MUIM_Draw          : return Render_Draw      (cl, obj, (APTR)msg); break;
      case MUIM_Show          : return Render_Show      (cl, obj, (APTR)msg); break;
      case MUIM_HandleInput   : return Render_Input     (cl, obj, (APTR)msg); break;
      case MUIM_DecodeImage   : return Render_Decode    (cl, obj);            break;
      case MUIM_SetDecdata    : return Render_Setdata   (cl, obj);            break;
      case MUIM_SetDecsize    : return Render_Setsize   (cl, obj);            break;
   }

   return DoSuperMethodA(cl, obj, msg);
}
DISPATCHER_END
/*=*/


/*=----------------------------- Cleanup_Render() ----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void Cleanup_Render(struct MUI_CustomClass *mcc)
{
   if (mcc) MUI_DeleteCustomClass(mcc);
}
/*=*/

/*=----------------------------- Init_Render() -------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
struct MUI_CustomClass *Init_Render(void)
{
   struct MUI_CustomClass *mcc = NULL;

   mcc = MUI_CreateCustomClass(NULL, MUIC_Area, NULL, sizeof(struct RenderData), DISPATCHER_REF(Render));
   return(mcc);
}
/*=*/
