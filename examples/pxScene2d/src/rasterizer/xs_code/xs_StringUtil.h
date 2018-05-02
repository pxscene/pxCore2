/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

// ====================================================================================================================
// ====================================================================================================================
// xs_StringUtil.h
// ====================================================================================================================
// ====================================================================================================================
#ifndef _xs_StringUtil_H_ // for entire file
#define _xs_StringUtil_H_

#include "xs_Core.h"

// ====================================================================================================================
// Basic string functions
// ====================================================================================================================
int32	xs_Strlen(const char *str, int32 maxLen=xs_MAXINT);						
char*	xs_Strcat(char* str1, const char*str2);			
char*	xs_Strcpy(char* str1, const char*str2);			
int32   xs_Strcmp(const char *str1, const char *str2);	

int32	xs_Strlen(const int16 *str, int32 maxLen=xs_MAXINT);						
int16*	xs_Strcat(int16* str1, const int16* str2);			
int16*	xs_Strcpy(int16* str1, const int16* str2);			
int32   xs_Strcmp(const int16 *str1, const int16 *str2);	

//0 is false, 1 is integer, 2 is real
int32 xs_StringIsNumber (const char *str, int32 len);	

// =================================================================================================================
// Token routiness
// =================================================================================================================
char*			xs_SkipSpaces	(const char* str, bool skipSpace=true);
char*			xs_ReadSpaces	(const char* str, char* dst, int32 maxLen, bool skipSpace=true);
char*			xs_SkipToken	(const char *str);
char*			xs_ReadToken	(const char *str, char *dst, int32 maxLen);
char* 			xs_ReadString	(const char *str, char *dst, int32 maxLen, char start='\"', char end='\"');
inline char*	xs_EOL			(const char *str)	{while (str && *str && *str!='\r' && *str!='\n') str++;
													 if (str && *str=='\r' && str[1]=='\n') str++; return (char*)str;}
inline bool		xs_IsSpace		(const char* str)	{return (*str) && ((*str == '\t') || (*str == ' '));}
char*			xs_NextToken	(const char* str, char* dst, int32 maxLen, char stopChar);

// =================================================================================================================
// Numeric/Ascii routines
// =================================================================================================================
//returns end of parse location - 0 length implies NULL terminated string
char* xs_atof		(const char* str, real32* f, int32 len=0);
char* xs_atod		(const char* str, real64* d, int32 len=0);
char* xs_atoi		(const char* str, int32*  i, int32 len=0);
char* xs_atou		(const char* str, uint32* u, int32 len=0);

//returns input str
char* xs_ftoa		(real32	f, char* str, int32 maxlength);
char* xs_dtoa		(real64 d, char* str, int32 maxlength);
char* xs_itoa		(int32  i, char* str, int32 maxlength, int32 rad=10);
char* xs_utoa		(uint32 u, char* str, int32 maxlength, int32 rad=10);


#endif // for entire file
