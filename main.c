/*
->============================================<-
->= SvTX - (c) Copyright 2022, 2026 OnyxSoft =<-
->============================================<-
->= Version  : 1.0                           =<-
->= File     : main.c                        =<-
->= Author   : Stefan Blixth                 =<-
->= Compiled : 2026-11-24                    =<-
->============================================<-

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
   FreeSubPages();
   base64_cleanup();

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

static WORD current_page = 100;

/*=----------------------------- ShowSubPage() -------------------------------*
 * Shows sub-page 'idx' of the current page (an invalid index clears the      *
 * view) and updates the sub-page controls.                                   *
 *----------------------------------------------------------------------------*/
void ShowSubPage(int idx)
{
   static char subtxt[32];
   char tmpgauge[64];
   int count = GetSubPageCount();
   BOOL ok   = SelectSubPage(idx);

   DoMethod(render_obj, MUIM_SetDecdata);
   DoMethod(render_obj, MUIM_SetDecsize);
   DoMethod(render_obj, MUIM_DecodeImage);
   MUI_Redraw(render_obj, MADF_DRAWOBJECT);

   if (ok && count > 1)
   {
      sprintf(subtxt, "%d/%d", idx + 1, count);
      set(txt_sub, MUIA_Text_Contents, subtxt);

      // Enable the group first, then the individual buttons
      //set(grp_sub, MUIA_Disabled, FALSE);
      //set(but_subprev, MUIA_Disabled, idx == 0);
      //set(but_subnext, MUIA_Disabled, idx == count - 1);

      sprintf(tmpgauge, "Sida %d, Undersida %d av %d", current_page, idx + 1, count);
      SetGaugeStatus(tmpgauge);
   }
   else
   {
      set(txt_sub, MUIA_Text_Contents, ok ? "1/1" : "-");
      //set(grp_sub, MUIA_Disabled, TRUE);

      if (ok)
      {
         sprintf(tmpgauge, "Sida %d", current_page);
         SetGaugeStatus(tmpgauge);
      }
   }
}
/*=*/

/*=----------------------------- OpenPageLink() ------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void OpenPageLink(WORD pos)
{
   char posstr[5];
   char tmpgauge[64];

   if ((pos <100) || pos >899)
      pos = 100;

   current_page = pos;

   sprintf(posstr, "%d", pos);
   sprintf(tmpgauge, "Öppnar sida %d...", pos);

   set(grp_status, MUIA_Disabled, TRUE);
   //set(grp_sub, MUIA_Disabled, TRUE);
   set(str_page, MUIA_String_Contents, posstr);
   SetGaugeStatus(tmpgauge);

   if (DownloadAndParse(posstr))
   {
      if (GetSubPageCount() > 0)
         ShowSubPage(0);
      else
      {
         ShowSubPage(-1);
         sprintf(tmpgauge, "Sida %d kunde inte laddas", pos);
         SetGaugeStatus(tmpgauge);
      }
   }
   else
      ShowSubPage(-1);   // Error text is already set by DownloadAndParse()

   set(grp_status, MUIA_Disabled, FALSE);
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

                                    Child, grp_sub = HGroup, NoFrame,
                                       //MUIA_Disabled, TRUE,

                                       Child, but_subprev = TextObject,
                                          ButtonFrame,
                                          MUIA_Weight, 1,
                                          MUIA_Background, MUII_ButtonBack,
                                          MUIA_Text_Contents, "<",
                                          MUIA_InputMode,  MUIV_InputMode_RelVerify,
                                       End,

                                       Child, txt_sub = TextObject,
                                          MUIA_Weight, 1,
                                          MUIA_Text_PreParse, "\33c",
                                          MUIA_Text_Contents, "-",
                                          MUIA_Text_SetMin, FALSE,
                                       End,

                                       Child, but_subnext = TextObject,
                                          ButtonFrame,
                                          MUIA_Weight, 1,
                                          MUIA_Background, MUII_ButtonBack,
                                          MUIA_Text_Contents, ">",
                                          MUIA_InputMode,  MUIV_InputMode_RelVerify,
                                       End,
                                    End,

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
   DoMethod(but_subprev, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, ID_BUT_SUBPREV);
   DoMethod(but_subnext, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, ID_BUT_SUBNEXT);

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

            if (page < 899)
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

         case ID_BUT_SUBPREV :
         {
            if (GetCurrentSubPage() > 0)
               ShowSubPage(GetCurrentSubPage() - 1);
         }  break;

         case ID_BUT_SUBNEXT :
         {
            if (GetCurrentSubPage() + 1 < GetSubPageCount())
               ShowSubPage(GetCurrentSubPage() + 1);
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
                  case 1: page = 100; break;
                  //case 2:  page = 200; break;
                  case 2: page = 300; break;
                  case 3: page = 330; break;
                  case 4: page = 377; break;
                  case 5: page = 400; break;
                  case 6: page = 500; break;
                  case 7: page = 600; break;
                  case 8: page = 700; break;
                  case 9: page = 800; break;
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