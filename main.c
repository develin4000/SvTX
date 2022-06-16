/*
->====================================<-
->= SvTX - © Copyright 2022 OnyxSoft =<-
->====================================<-
->= Version  : 1.0                   =<-
->= File     : main.c                =<-
->= Author   : Stefan Blixth         =<-
->= Compiled : 2022-06-16            =<-
->====================================<-

gcc main.c LinkedList.c rendermcc.c urlparser.c base64.c -o SvTX -noixemul

*/

#include "main.h"

LONG __stack = 32768;

struct Library       *MUIMasterBase  = NULL;
struct GfxBase       *GfxBase        = NULL;
struct Library       *IconBase       = NULL;
struct Library       *CyberGfxBase   = NULL;
struct IntuitionBase *IntuitionBase  = NULL;
struct Library       *HttpStreamBase = NULL;
struct Library       *DataTypesBase  = NULL;
struct Library       *UtilityBase    = NULL;


/*=----------------------------- init() --------------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
BOOL init(void)
{
   if (!(IconBase = (struct Library *)OpenLibrary("icon.library", 37L))) return FALSE;
   if (!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 37L))) return FALSE;
   if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 37L))) return FALSE;
   if (!(CyberGfxBase = (struct Library *)OpenLibrary("cybergraphics.library", 41L))) return FALSE;
   if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, MUIMASTER_VMIN))) return FALSE;
   if (!(UtilityBase = (struct Library *)OpenLibrary("utility.library", 39))) return FALSE;
   if (!(DataTypesBase = (struct Library *)OpenLibrary("datatypes.library", 39L))) return FALSE;
   if (!(HttpStreamBase = OpenLibrary("multimedia/http.stream", 52))) return FALSE;

   if (!(render_mcc = Init_Render())) return FALSE;
   if (!(llNew(&llData, NULL, FALSE))) return FALSE;

   return TRUE;
}
/*=*/

/*=----------------------------- cleanup() -----------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void cleanup(void)
{
   if (app) MUI_DisposeObject(app);
   ClearMapData();
   llEnd(&llData);
   if (render_mcc) Cleanup_Render(render_mcc);
   if (decoding_table) base64_cleanup();

   if (HttpStreamBase) CloseLibrary((struct Library *) HttpStreamBase);
   if (DataTypesBase) CloseLibrary((struct Library *) DataTypesBase);
   if (UtilityBase) CloseLibrary((struct Library *) UtilityBase);
   if (MUIMasterBase) CloseLibrary(MUIMasterBase);
   if (CyberGfxBase) CloseLibrary((struct Library *) CyberGfxBase);
   if (IntuitionBase) CloseLibrary((struct Library *) IntuitionBase);
   if (IconBase) CloseLibrary((struct Library *) IconBase);
   if (GfxBase) CloseLibrary((struct Library *) GfxBase);
}
/*=*/

/*=----------------------------- OpenPageLink() ------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void OpenPageLink(WORD pos)
{
   char posstr[5];
   char tmpgauge[30];

   if ((pos <100) || pos >900)
      pos = 100;

   sprintf(posstr, "%d", pos);
   sprintf(tmpgauge, "Opening page %d...", pos);

   set(grp_status, MUIA_Disabled, TRUE);
   set(str_page, MUIA_String_Contents, posstr);
   SetGaugeStatus(tmpgauge);

   DownloadAndParse(posstr);

   DoMethod(render_obj, MUIM_SetDecdata);
   DoMethod(render_obj, MUIM_SetDecsize);
   DoMethod(render_obj, MUIM_DecodeImage);

   set(grp_status, MUIA_Disabled, FALSE);
   
   MUI_Redraw(render_obj, MADF_DRAWOBJECT);
}
/*=*/

/*=----------------------------- SetGaugeStatus() ----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void SetGaugeStatus(char *newstring)
{
   set(gau_info, MUIA_Gauge_InfoText, newstring);
}
/*=*/


/*=----------------------------- opengui() -----------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
BOOL opengui(void)
{
   app = ApplicationObject,MUIA_Application_Title           , NAME,
                           MUIA_Application_Version         , VERSTAG_MUI,
                           MUIA_Application_Copyright       , COPYRIGHT,
                           MUIA_Application_Author          , AUTHOR,
                           MUIA_Application_Description     , NAME,
                           MUIA_Application_Base            , BASE,
                           MUIA_Application_UseCommodities  , TRUE,

                           SubWindow, win = WindowObject,
                              MUIA_Frame,                MUIV_Frame_None,
                              MUIA_Window_Borderless,    FALSE,
                              MUIA_Window_CloseGadget,   TRUE,
                              MUIA_Window_DepthGadget,   TRUE,
                              MUIA_Window_SizeGadget,    TRUE,
                              MUIA_Window_DragBar,       TRUE,
                              MUIA_Window_Title,         NAME,
                              MUIA_Window_ID,            MAKE_ID('S','V','T','X'),

                              WindowContents, VGroup,

                                 Child, HGroup, NoFrame,

                                    Child, cyc_index = CycleObject,
                                       MUIA_Background, MUII_ButtonBack,
                                       MUIA_Cycle_Entries, cycle_index,
                                    End,
                                    Child, HVSpace,

                                    Child, grp_status = HGroup, NoFrame,

                                       Child, HVSpace,

                                       Child, but_prev = TextObject,
                                          ButtonFrame,
                                          MUIA_Weight, 1,
                                          MUIA_Background, MUII_ButtonBack,
                                          MUIA_Text_Contents, "<<",
                                          MUIA_InputMode,  MUIV_InputMode_RelVerify,
                                          MUIA_Disabled, TRUE,
                                       End,

                                       Child, str_page = StringObject,
                                          StringFrame,
                                          MUIA_Weight, 1,
                                          MUIA_String_MaxLen, 4,
                                          MUIA_String_Accept, (IPTR)"0123456789", 
                                          MUIA_String_Contents, "100",
                                          MUIA_InputMode,  MUIV_InputMode_RelVerify,
                                       End,

                                       Child, but_next = TextObject,
                                          ButtonFrame,
                                          MUIA_Weight, 1,
                                          MUIA_Background, MUII_ButtonBack,
                                          MUIA_Text_Contents, ">>",
                                          MUIA_InputMode,  MUIV_InputMode_RelVerify,
                                       End,
                                    End,
                                 End,

                                 Child, render_obj = NewObject(render_mcc->mcc_Class, NULL, NoFrame, MUIA_InnerLeft, 0, MUIA_InnerRight, 0, MUIA_InnerTop, 0, MUIA_InnerBottom, 0, MUIA_Background, MUII_WindowBack, TAG_DONE),

                                 Child, gau_info = GaugeObject,
                                    GaugeFrame,
                                    MUIA_Gauge_Max, 100,
                                    MUIA_Gauge_Current, 0,
                                    MUIA_Gauge_InfoText, "Welcome to SvTX",
                                    MUIA_Gauge_Horiz, TRUE,
                                 End,

                              End,
                           End,
                        End;

   if (!app)
   {
      puts("Could not create application!\n");
      return FALSE;
   }

   DoMethod(win, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
   DoMethod(but_prev, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, ID_BUT_PREV);
   DoMethod(but_next, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, ID_BUT_NEXT);
   DoMethod(str_page, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, app, 2, MUIM_Application_ReturnID, ID_STR_PAGE);
   DoMethod(cyc_index, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, app, 2, MUIM_Application_ReturnID, ID_CYC_INDEX);

   set(win, MUIA_Window_Open, TRUE);
   OpenPageLink(100);
   return TRUE;
}
/*=*/


/*=----------------------------- handler() -----------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void handler(void)
{
   BOOL going   = TRUE;
   ULONG muisig = 0;
   APTR temps;
   unsigned int page = 100, cycpos = 0;

   while (going)
   {
      switch (DoMethod(app, MUIM_Application_NewInput, &muisig))
      {
         case MUIV_Application_ReturnID_Quit :
            going = FALSE;
            break;

         case ID_BUT_PREV :
         {
            get(str_page, MUIA_String_Contents, &temps);
            page = atoi(temps);

            if (page >= 101)
            {
               page -= 1;
               OpenPageLink(page);
            }
         }  break;

         case ID_BUT_NEXT :
         {
            get(str_page, MUIA_String_Contents, &temps);
            page = atoi(temps);

            if (page <= 900)
            {
               OpenPageLink(page+1);
            }
         }  break;

         case ID_STR_PAGE :
         {
            get(str_page, MUIA_String_Contents, &temps);
            page = atoi(temps);

            if (page >= 100)
            {
               OpenPageLink(page);
            }
         }  break;

         case ID_CYC_INDEX :
         {
            get(cyc_index, MUIA_Cycle_Active, &temps);
            cycpos = (unsigned int)temps;
            set(cyc_index, MUIA_Disabled, TRUE);

            if (cycpos > 0)
            {
               switch(cycpos)
               {
                  case 1:  page = 100; break;
                  case 2:  page = 200; break;
                  case 3:  page = 300; break;
                  case 4:  page = 330; break;
                  case 5:  page = 377; break;
                  case 6:  page = 400; break;
                  case 7:  page = 500; break;
                  case 8:  page = 600; break;
                  case 9:  page = 700; break;
                  case 10: page = 800; break;
               }
               OpenPageLink(page);
               set(cyc_index, MUIA_Cycle_Active, 0);
            }

            set(cyc_index, MUIA_Disabled, FALSE);

         }  break;
         default:
            break;
      }

      if (going && muisig)
         Wait(muisig);
   }
}
/*=*/


/*=----------------------------- main() --------------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
int main(void)
{
   if (init())
   {
      if (opengui())
      {
         handler();
      }
   }
   
   cleanup();
   return 0;
}
/*=*/