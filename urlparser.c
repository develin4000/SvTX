/*
->====================================<-
->= SvTX - © Copyright 2022 OnyxSoft =<-
->====================================<-
->= Version  : 1.0                   =<-
->= File     : urlparser.c           =<-
->= Author   : Stefan Blixth         =<-
->= Compiled : 2022-06-16            =<-
->====================================<-
*/

#include "main.h"
#include "jsmn.h"

unsigned long decode_size = 0;
unsigned long decoded_size = 0; 
UBYTE *decdatabuf = NULL;
char tags = 0;
char tagdone = 0;
struct MapData *iMap = NULL;
ULONG JSONSize = 0;
WORD xx1=0, yy1=0, xx2=0, yy2=0, ppos=0;


/*=----------------------------- PrepareUrlString() --------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void PrepareUrlString(char *strurl, char *strloc)
{
   int pos;
   NewRawDoFmt("https://www.svt.se/text-tv/%s", NULL, strurl, strloc);
}
/*=*/

/*=----------------------------- GetDecodedSize() ----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
unsigned long GetDecodedSize(void)
{
   return decoded_size;
}
/*=*/

/*=----------------------------- GetDecDatabuf() -----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
UBYTE *GetDecDatabuf(void)
{
   return decdatabuf;
}
/*=*/

/*=----------------------------- DownloadData() ------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
LONG DownloadData(STRPTR url, unsigned char *databuf, ULONG chunksize)
{
   Object *http;
   LONG len=0;
   QUAD total, done = 0;
   LONG chunk;
   LONG error = 0;

   if ((http = NewObject(NULL, "http.stream", MMA_URIStreamName, (LONG)url, TAG_END)))
   {
      total = MediaGetPort64(http, 0, MMA_StreamLength);

      while (!error)
      {
         chunk = DoMethod(http, MMM_Pull, 0, (LONG)databuf, chunksize);
         done += chunk;

         if (chunk < chunksize)
         {
            LONG reggae_error = MediaGetPort(http, 0, MMA_ErrorCode);

            if (reggae_error == MMERR_END_OF_DATA)
            {
               len = chunk;
               break;
            }
            else
            {
               len = error = RETURN_ERROR;
            }
         }
         else
         {
            len = chunk;
         }
      }
      DisposeObject(http);
   }
   else
   {
      len = 0;
   }
   return len;
}
/*=*/


/*=----------------------------- jsoneq() ------------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
   if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start && strncmp(json + tok->start, s, tok->end - tok->start) == 0)
   {
      return 0;
   }
   return -1;
}
/*=*/

/*=----------------------------- FetchJsonString() ---------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
unsigned char* FetchJsonString(char *inputdata, unsigned int datasize, char *starttag, char *endtag)
{
   char *strptr1, *strptr2 = 0;
   unsigned int strpos1, strpos2 = 0;
   char taglen1, taglen2 = 0;
   char *newdata = NULL;
   unsigned int newlen = 0;

   JSONSize = 0;
   strptr1 = strstr((char *)inputdata, (char *)starttag);
   taglen1 = strlen(starttag);
   taglen2 = strlen(endtag);

   if (strptr1)
      strpos1 = strptr1-inputdata;
   else
      return 0;

   strptr2 = strstr((char *)strptr1, (char *)endtag);

   if (strptr2)
      strpos2 = strptr2-inputdata;
   else
      return 0;

   if (strpos2 > strpos1)
   {
      newlen = datasize - (strpos1+taglen1) - (datasize-strpos2) + 1;
      newdata = (char*) AllocVec(newlen, MEMF_ANY);
      JSONSize = newlen;
   }

   if (newdata)
      memcpy(newdata, inputdata+strpos1+taglen1, newlen);

   return newdata;
}
/*=*/

/*=----------------------------- AddMapData() --------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void AddMapData(WORD wx1, WORD wy1, WORD wx2, WORD wy2, WORD pos)
{
   MapData *xMap;

   xMap = AllocVec(sizeof(MapData), MEMF_ANY);

   xMap->x1 = wx1;
   xMap->y1 = wy1;
   xMap->x2 = wx2;
   xMap->y2 = wy2;
   xMap->pos = pos;

   llAdd(&llData, xMap, LL_LAST);
}
/*=*/

/*=----------------------------- ClearMapData() ------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void ClearMapData(void)
{
   int xcnt;
   MapData *tMap;

   for(xcnt=0; xcnt<llData.len; xcnt++)
   {
      tMap = llGet(&llData, xcnt);
      FreeVec(tMap);
   }

   llClear(&llData);
}
/*=*/

/*=----------------------------- Coordinator() -------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void Coordinator(char *inputdata)
{
   char *sch, *sptr;
   unsigned int pos = 0;

   sch = strtok_r(inputdata, ",", &sptr);

   while (sch != NULL)
   {
      switch(pos)
      {
         case 0 : xx1 = atoi(sch); break;
         case 1 : yy1 = atoi(sch); break;
         case 2 : xx2 = atoi(sch); break;
         case 3 : yy2 = atoi(sch); break;
         default : break;
      }
      pos++;
      sch = strtok_r(NULL, ",", &sptr);
   }
}
/*=*/

/*=----------------------------- FetchImageMap() -----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
BOOL FetchImageMap(char *inputdata, char *coordstag, char *hreftag)
{
   char *sch;
   
   ClearMapData();

   sch = strtok(inputdata, "\"");

   while (sch != NULL)
   {
      if (tags == MAP_CORD)
      {
         Coordinator(sch);
         tagdone += MAP_CORD;
         tags = MAP_NONE;
      }
      else if (tags == MAP_HREF)
      {
         ppos = atoi(sch);
         tagdone += MAP_HREF;
         tags = MAP_NONE;
      }
      else   // MAP_NONE
      {
         if (strcmp(coordstag, sch) == 0)
            tags = MAP_CORD;
         else if (strcmp(hreftag, sch) == 0)
            tags = MAP_HREF;
      }

      if (tagdone == MAP_DONE)
      {
         AddMapData(xx1, yy1, xx2, yy2, ppos);
         tagdone = MAP_NONE;
      }
      sch = strtok(NULL, "\"");
   }

   return TRUE;
}
/*=*/

/*=----------------------------- DownloadAndParse() --------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void DownloadAndParse(char *searchstring)
{
   int done, len;
   char oki = 1;
   BPTR ofh = 0;
   int i;
   int r;
   jsmn_parser p;
   char urlstr[500];
   jsmntok_t t[10240];   /* We expect no more than 128 tokens */
   char txtres[204800];
   unsigned char rawbuf[URL_BUFFERSIZE];
   unsigned char *JSONBuffer = NULL;

   if (decdatabuf)
   {
      FreeVec(decdatabuf);
      decdatabuf = NULL;
   }

   PrepareUrlString(urlstr, searchstring);
   len = DownloadData(urlstr, (unsigned char *)&rawbuf, CHUNK_SIZE);

   if (len)
   {
      JSONBuffer = FetchJsonString(rawbuf, len, "type=\"application/json\">", "</script>");

      if (JSONSize != 0)
      {
         jsmn_init(&p);
         r = jsmn_parse(&p, JSONBuffer, JSONSize, t, sizeof(t) / sizeof(t[0]));

         if (r < 0) 
         {
            SetGaugeStatus("Failed to parse JSON data...");
            oki = 0;
         }

         // Assume the top-level element is an object
         if (r < 1 || t[0].type != JSMN_OBJECT)
         {
            SetGaugeStatus("Failed to parse JSON data...");
            oki = 0;
         }

         if (oki)
         {
            // Loop over all keys of the root object 
            for (i = 1; i < r; i++)
            {
               if (jsoneq(JSONBuffer, &t[i], "gifAsBase64") == 0)
               {
                  // We may want to do strtol() here to get numeric value
                  sprintf(txtres, "%.*s\0", t[i + 1].end - t[i + 1].start, JSONBuffer + t[i + 1].start);
                  decode_size = strlen(txtres);
                  decdatabuf = base64_decode(txtres, decode_size, &decoded_size);
                  i++;
               }
               else if (jsoneq(JSONBuffer, &t[i], "imageMap") == 0)
               {
                  // We may additionally check if the value is either "true" or "false"
                  sprintf(txtres, "%.*s\0", t[i + 1].end - t[i + 1].start, JSONBuffer + t[i + 1].start);
                  FetchImageMap(txtres, " COORDS=\\", " HREF=\\");
                  i++;
               }
            }
            SetGaugeStatus("Done!");
            base64_cleanup();
         }
      }
      else
         SetGaugeStatus("Failed to parse JSON data...");

      if (JSONBuffer) FreeVec(JSONBuffer);
   }
   else
   {
      SetGaugeStatus("Error fetching data...");
   }
}
/*=*/

