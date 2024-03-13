/***********************************************************
* Base64 library                                           *
* @author Ahmed Elzoughby                                  *
* @date July 23, 2017                                      *
* Purpose: encode and decode MCUCode format                 *
***********************************************************/

#ifndef BASE46_H
#define BASE46_H

#include <stdlib.h>
#include <memory.h>


/***********************************************
Encodes ASCCI string into MCUCode format string
@param plain ASCII string to be encoded
@return encoded MCUCode format string
***********************************************/
void base64_encode(const char* plain, uint8_t len, char* cipher);


/***********************************************
decodes MCUCode format string into ASCCI string
@param plain encoded MCUCode format string
@return ASCII string to be encoded
***********************************************/
char* base64_decode(char* cipher);


#endif //BASE46_H