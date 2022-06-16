/*
->====================================<-
->= SvTX - © Copyright 2022 OnyxSoft =<-
->====================================<-
->= Version  : 1.0                   =<-
->= File     : base64.h              =<-
->= Author   : Stefan Blixth         =<-
->= Compiled : 2022-06-16            =<-
->====================================<-
*/

#ifndef __BASE64_H__
#define __BASE64_H__

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

// Prototypes...
void build_decoding_table(void);
void base64_cleanup(void);
unsigned char *base64_decode(const char *, unsigned long, unsigned long *);

#endif // __BASE64_H__
