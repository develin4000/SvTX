/*
->============================================<-
->= SvTX - (c) Copyright 2022, 2026 OnyxSoft =<-
->============================================<-
->= Version  : 1.0                           =<-
->= File     : urlparser.c                   =<-
->= Author   : Stefan Blixth                 =<-
->= Compiled : 2026-11-24                    =<-
->============================================<-
*/

#include "main.h"
#include "jsmn.h"

/* One entry per sub-page found in the "subPages" array of the page JSON */
struct SubPage
{
   UBYTE *gif;          // Decoded GIF image data
   ULONG gifsize;       // Size of the decoded GIF data
   char  *imagemap;     // Raw (still escaped) imageMap HTML, NUL-terminated
};

static struct SubPage *subpages = NULL;
static int subpage_count   = 0;
static int subpage_current = -1;

unsigned long decoded_size = 0;
UBYTE *decdatabuf = NULL;
char tags = 0;
char tagdone = 0;
struct MapData *iMap = NULL;
WORD xx1=0, yy1=0, xx2=0, yy2=0, ppos=0;


/*=----------------------------- PrepareUrlString() --------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void PrepareUrlString(char *strurl, char *strloc)
{
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
 * Downloads the whole document at 'url' into a buffer that grows as needed.  *
 * Returns a NUL-terminated buffer (free with FreeVec()) and its length in    *
 * *outlen, or NULL if the download failed or returned no data.               *
 *----------------------------------------------------------------------------*/
UBYTE *DownloadData(STRPTR url, ULONG *outlen)
{
   Object *http;
   UBYTE *buf = NULL, *nbuf;
   ULONG size = 0, cap = 0, ncap;
   LONG chunk;
   BOOL ok = FALSE;

   *outlen = 0;

   if (!(http = NewObject(NULL, "http.stream", MMA_URIStreamName, (LONG)url, TAG_END)))
      return NULL;

   for (;;)
   {
      // Always keep room for one more chunk plus the NUL terminator
      if (cap - size < CHUNK_SIZE + 1)
      {
         ncap = cap ? cap * 2 : URL_INITIAL_SIZE;

         if (ncap > URL_MAX_SIZE)
            break;

         if (!(nbuf = AllocVec(ncap, MEMF_ANY)))
            break;

         if (buf)
         {
            CopyMem(buf, nbuf, size);
            FreeVec(buf);
         }
         buf = nbuf;
         cap = ncap;
      }

      chunk = DoMethod(http, MMM_Pull, 0, (LONG)(buf + size), CHUNK_SIZE);

      if (chunk > 0)
         size += chunk;

      if (chunk < CHUNK_SIZE)
      {
         ok = (MediaGetPort(http, 0, MMA_ErrorCode) == MMERR_END_OF_DATA);
         break;
      }
   }

   DisposeObject(http);

   if (!ok || size == 0)
   {
      if (buf) FreeVec(buf);
      return NULL;
   }

   buf[size] = '\0';
   *outlen = size;
   return buf;
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

/*=----------------------------- SkipToken() ---------------------------------*
 * Returns the index of the first token after token i and all its children.   *
 *----------------------------------------------------------------------------*/
static int SkipToken(jsmntok_t *t, int count, int i)
{
   int end = t[i].end;

   for (i++; i < count && t[i].start < end; i++);

   return i;
}
/*=*/

/*=----------------------------- FindJson() ----------------------------------*
 * Locates the JSON embedded in the page's <script type="application/json">  *
 * element. Returns a pointer into 'html' and the length in *jsonlen.         *
 *----------------------------------------------------------------------------*/
static char *FindJson(char *html, ULONG *jsonlen)
{
   static const char starttag[] = "type=\"application/json\">";
   char *start, *end;

   *jsonlen = 0;

   if (!(start = strstr(html, starttag)))
      return NULL;

   start += sizeof(starttag) - 1;

   if (!(end = strstr(start, "</script>")))
      return NULL;

   *jsonlen = end - start;
   return start;
}
/*=*/

/*=----------------------------- CopyString() --------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static char *CopyString(const char *src, ULONG len)
{
   char *dst = AllocVec(len + 1, MEMF_ANY);

   if (dst)
   {
      CopyMem((APTR)src, dst, len);
      dst[len] = '\0';
   }
   return dst;
}
/*=*/

/*=----------------------------- AddMapData() --------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void AddMapData(WORD wx1, WORD wy1, WORD wx2, WORD wy2, WORD pos)
{
   MapData *xMap;

   if (!(xMap = AllocVec(sizeof(MapData), MEMF_ANY)))
      return;

   xMap->x1 = wx1;
   xMap->y1 = wy1;
   xMap->x2 = wx2;
   xMap->y2 = wy2;
   xMap->pos = pos;

   if (!llAdd(&llData, xMap, LL_LAST))
      FreeVec(xMap);
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

   // Start every map from a clean state, so one malformed area can't
   // affect the areas (or pages) that follow
   tags    = MAP_NONE;
   tagdone = MAP_NONE;

   sch = strtok(inputdata, "\"");

   while (sch != NULL)
   {
      if (tags == MAP_CORD)
      {
         Coordinator(sch);
         tagdone |= MAP_CORD;
         tags = MAP_NONE;
      }
      else if (tags == MAP_HREF)
      {
         ppos = atoi(sch);
         tagdone |= MAP_HREF;
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

/*=----------------------------- FreeSubPages() ------------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void FreeSubPages(void)
{
   int i;

   if (subpages)
   {
      for (i = 0; i < subpage_count; i++)
      {
         if (subpages[i].gif)      FreeVec(subpages[i].gif);
         if (subpages[i].imagemap) FreeVec(subpages[i].imagemap);
      }
      FreeVec(subpages);
   }

   subpages        = NULL;
   subpage_count   = 0;
   subpage_current = -1;
   decdatabuf      = NULL;
   decoded_size    = 0;
}
/*=*/

/*=----------------------------- GetSubPageCount() ---------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
int GetSubPageCount(void)
{
   return subpage_count;
}
/*=*/

/*=----------------------------- GetCurrentSubPage() -------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
int GetCurrentSubPage(void)
{
   return subpage_current;
}
/*=*/

/*=----------------------------- SelectSubPage() -----------------------------*
 * Makes sub-page 'idx' current: its image becomes the decode buffer and its  *
 * image map the clickable link list. An invalid index clears both.           *
 *----------------------------------------------------------------------------*/
BOOL SelectSubPage(int idx)
{
   char *tmp;

   ClearMapData();

   if (idx < 0 || idx >= subpage_count)
   {
      subpage_current = -1;
      decdatabuf      = NULL;
      decoded_size    = 0;
      return FALSE;
   }

   subpage_current = idx;
   decdatabuf      = subpages[idx].gif;
   decoded_size    = subpages[idx].gifsize;

   // FetchImageMap() tokenizes in place, so work on a copy
   if (subpages[idx].imagemap && (tmp = CopyString(subpages[idx].imagemap, strlen(subpages[idx].imagemap))))
   {
      FetchImageMap(tmp, " COORDS=\\", " HREF=\\");
      FreeVec(tmp);
   }

   return TRUE;
}
/*=*/

/*=----------------------------- ParseSubPages() -----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
static BOOL ParseSubPages(const char *json, jsmntok_t *t, int r)
{
   int i, j, k, n, s, objend;
   struct SubPage *sp;

   // Find the "subPages" array
   for (i = 1; i + 1 < r; i++)
   {
      if (jsoneq(json, &t[i], "subPages") == 0 && t[i + 1].type == JSMN_ARRAY)
         break;
   }

   if (i + 1 >= r)
      return TRUE;   // No sub-pages, i.e. the page doesn't exist

   n = t[i + 1].size;

   if (n <= 0)
      return TRUE;

   if (!(subpages = AllocVec(n * sizeof(struct SubPage), MEMF_CLEAR)))
      return FALSE;

   k = i + 2;   // First element of the array

   for (s = 0; s < n && k < r; s++)
   {
      objend = SkipToken(t, r, k);

      if (t[k].type == JSMN_OBJECT)
      {
         sp = &subpages[subpage_count];

         for (j = k + 1; j + 1 < objend; j++)
         {
            if (jsoneq(json, &t[j], "gifAsBase64") == 0 && t[j + 1].type == JSMN_STRING && !sp->gif)
            {
               unsigned long gifsize = 0;

               sp->gif     = base64_decode(json + t[j + 1].start, t[j + 1].end - t[j + 1].start, &gifsize);
               sp->gifsize = gifsize;
               j++;
            }
            else if (jsoneq(json, &t[j], "imageMap") == 0 && t[j + 1].type == JSMN_STRING && !sp->imagemap)
            {
               sp->imagemap = CopyString(json + t[j + 1].start, t[j + 1].end - t[j + 1].start);
               j++;
            }
         }

         if (sp->gif)
            subpage_count++;
         else if (sp->imagemap)
         {
            FreeVec(sp->imagemap);
            sp->imagemap = NULL;
         }
      }

      k = objend;
   }

   return TRUE;
}
/*=*/

/*=----------------------------- DownloadAndParse() --------------------------*
 * Downloads page 'searchstring' and extracts all of its sub-pages.          *
 * Returns FALSE (and sets the status text) if the page couldn't be fetched   *
 * or parsed. On success GetSubPageCount() tells how many sub-pages were      *
 * found, 0 meaning that the page doesn't exist. No sub-page is selected.     *
 *----------------------------------------------------------------------------*/
BOOL DownloadAndParse(char *searchstring)
{
   jsmn_parser p;
   jsmntok_t *t = NULL;
   char urlstr[500];
   UBYTE *rawbuf;
   ULONG rawlen, jsonlen;
   char *json;
   int r;
   BOOL ok = FALSE;

   FreeSubPages();
   ClearMapData();

   PrepareUrlString(urlstr, searchstring);

   if (!(rawbuf = DownloadData(urlstr, &rawlen)))
   {
      SetGaugeStatus("Error fetching data...");
      return FALSE;
   }

   if ((json = FindJson((char *)rawbuf, &jsonlen)))
   {
      // First pass counts the tokens, second pass fills them in
      jsmn_init(&p);
      r = jsmn_parse(&p, json, jsonlen, NULL, 0);

      if (r > 0 && (t = AllocVec(r * sizeof(jsmntok_t), MEMF_ANY)))
      {
         jsmn_init(&p);
         r = jsmn_parse(&p, json, jsonlen, t, r);

         if (r > 0 && t[0].type == JSMN_OBJECT)
            ok = ParseSubPages(json, t, r);

         FreeVec(t);
      }
   }

   base64_cleanup();
   FreeVec(rawbuf);

   if (!ok)
   {
      FreeSubPages();
      SetGaugeStatus("Failed to parse JSON data...");
   }

   return ok;
}
/*=*/
