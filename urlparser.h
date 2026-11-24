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

#ifndef __URLPARSER_H__
#define __URLPARSER_H__

#define MAP_NONE 0
#define MAP_CORD 1
#define MAP_HREF 2
#define MAP_DONE 3

#define URL_INITIAL_SIZE  262144    // Download buffer grows from this size...
#define URL_MAX_SIZE      4194304   // ...up to this size
#define CHUNK_SIZE        102400

// Prototypes...
unsigned long GetDecodedSize(void);
UBYTE *GetDecDatabuf(void);
BOOL DownloadAndParse(char *);
void ClearMapData(void);
void FreeSubPages(void);
BOOL SelectSubPage(int);
int GetSubPageCount(void);
int GetCurrentSubPage(void);

#endif // __URLPARSER_H__
