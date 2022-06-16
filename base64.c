/*
->====================================<-
->= SvTX - © Copyright 2022 OnyxSoft =<-
->====================================<-
->= Version  : 1.0                   =<-
->= File     : base64.c              =<-
->= Author   : Stefan Blixth         =<-
->= Compiled : 2022-06-16            =<-
->====================================<-
*/

#include "main.h"

/*=----------------------------- build_decoding_table() ----------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void build_decoding_table(void) 
{
   decoding_table = AllocVec(256, MEMF_ANY);

   for (int i = 0; i < 64; i++)
      decoding_table[(unsigned char) encoding_table[i]] = i;
}
/*=*/

/*=----------------------------- base64_cleanup() ----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
void base64_cleanup(void) 
{
   FreeVec(decoding_table);
   decoding_table = NULL;
}
/*=*/

/*=----------------------------- base64_decode() -----------------------------*
 *                                                                            *
 *----------------------------------------------------------------------------*/
unsigned char *base64_decode(const char *data, unsigned long input_length, unsigned long *output_length)
{
   if (decoding_table == NULL) build_decoding_table();
   if (input_length % 4 != 0) return NULL;

   *output_length = input_length / 4 * 3;

   if (data[input_length - 1] == '=') (*output_length)--;
   if (data[input_length - 2] == '=') (*output_length)--;

   unsigned char *decoded_data = AllocVec(*output_length, MEMF_ANY);

   if (decoded_data == NULL) return NULL;

   for (int i = 0, j = 0; i < input_length;)
   {
      uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
      uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
      uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
      uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

      uint32_t triple = (sextet_a << 3 * 6)
      + (sextet_b << 2 * 6)
      + (sextet_c << 1 * 6)
      + (sextet_d << 0 * 6);

      if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
      if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
      if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
   }

   return decoded_data;
}
/*=*/
