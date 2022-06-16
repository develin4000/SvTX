/*
->====================================<-
->= SvTX - © Copyright 2022 OnyxSoft =<-
->====================================<-
->= Version  : 1.0                   =<-
->= File     : urlparser.h           =<-
->= Author   : Stefan Blixth         =<-
->= Compiled : 2022-06-16            =<-
->====================================<-
*/

#ifndef __URLPARSER_H__
#define __URLPARSER_H__

#define MAP_NONE 0
#define MAP_CORD 1
#define MAP_HREF 2
#define MAP_DONE 3

#define URL_BUFFERSIZE  2048000  // Kolla denna storlek!
#define CHUNK_SIZE      102400

// Prototypes...
unsigned long GetDecodedSize(void);
UBYTE *GetDecDatabuf(void);
void DownloadAndParse(char *);
void ClearMapData(void);

#endif // __URLPARSER_H__
