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
// xs_String.cpp
// ====================================================================================================================
// ====================================================================================================================
#include "xs_Core.h"
#include "xs_String.h"
#include "xs_Float.h"

#ifdef Windows_
#include <stdlib.h>
#endif //Windows_

struct xs_IEEEFloat
{
	int32 sgn:1;
	int32 exp:8;
	int32 mnt:23;
};

// =================================================================================================================
// xs_SkipSpaces
// =================================================================================================================
char* xs_SkipSpaces(const char* str, bool skipSpace)
{
	if (str)
		{
		//skip the spaces (or non-spaces)
		while (*str && xs_IsSpace(str)==skipSpace)
			str++;
		}
		
	return (char*)str;
}

// =================================================================================================================
// xs_ReadSpaces
// =================================================================================================================
char* xs_ReadSpaces(const char* str, char* dst, int32 maxLen, bool skipSpace)
{
	//make sure we have room
	if (str && dst && maxLen>1)
		{
		//copy the spaces (or non-spaces)
		char *end = (char*)dst+maxLen;
		while (*str && xs_IsSpace(str)==skipSpace && *str!='\r' && *str!='\n' && dst<end)
			*dst++ = *str++;
		*dst++ = 0; //terminate
		}
	else str = NULL;
		
	return (char*)str;
}

// =================================================================================================================
// xs_SkipToken - skips a token
// =================================================================================================================
char* xs_SkipToken(const char *str)
{
	str		= xs_SkipSpaces (str, true);
	return	(char*)xs_SkipSpaces (str, false);
}

// =================================================================================================================
// xs_ReadToken - reads a token
// =================================================================================================================
char* xs_ReadToken(const char *str, char *dst, int32 maxLen)
{
	str		= xs_SkipSpaces (str, true);
	return	(char*)xs_ReadSpaces (str, dst, maxLen, false);
}

// =================================================================================================================
// xs_NextToken - reads a token with an additional stop char
// =================================================================================================================
char* xs_NextToken (const char* str, char* dst, int32 maxLen, char stopChar)
{
	str	= xs_SkipSpaces (str, true);

	//make sure we have room
	if (str && dst && maxLen>1)
		{
		//copy the spaces (or non-spaces)
		char *end = (char*)dst+maxLen;
		while (*str && xs_IsSpace(str)==false && *str!=stopChar && dst<end && *str!='\r' && *str!='\n')
			*dst++ = *str++;
		*dst++ = 0; //terminate
		}
	else str = NULL;
		
	return (char*)str;
}

// =================================================================================================================
// xs_LineEnd - Finds the end of a line
// =================================================================================================================
char* xs_LineEnd (const char *strLoc)
{
//	int8 *str1 = 0, *str2 = 0;
	
	while (strLoc && *strLoc)
		{
		switch (*strLoc)
			{
			case '\r':	return (char*)strLoc;
			case '\n':	return (char*)strLoc;
			default:{strLoc++;}
			}
		}
	return (char*)strLoc;
}


// =================================================================================================================
// xs_ReadString - reads a string wrapped by (start)string string(end) characters
// =================================================================================================================
char* xs_ReadString (const char *str, char *dst, int32 maxLen, char start, char end)
{
	//skip to start
	while (*str && *str != '\r' && *str != '\n' && *str !=start)	str++;
	
	//abort if it's not the start;
	if (*str == start)	str++;
	else				return NULL;

	//read the string
	bool ret = false;
	while (*str && *str != '\r' && *str != '\n' && ret==false)	
		{
		//abort properly if we hit the end
		if (*str==end)	ret=true;
		else if (dst)	*dst++ = *str++;
		else			str++;
		}

	//terminate
	if (dst) *dst = 0;

	//return
	return ret ? (char*)str : NULL;
}


// ================================================================================================================
//  pow10 functions
// ================================================================================================================
double gxs_pow10R[] = 
{
	1,10,100,1000,10000,100000,1000000,10000000,100000000
};
static double pow10 (int32 count)
{
	if (count<9)	return gxs_pow10R[count];
	
	//iterate
	double val = 100000000;
	count = count - 8;
	while (count-->0)
		val*=10;
	return val;
}

int32 gxs_pow10I[] = 
{
	1,10,100,1000,10000,100000,1000000,10000000,100000000
};
/*
static uint32 pow10int (int32 count)
{
	if (count<9)	return gxs_pow10I[count];
	
	//iterate
	uint32 val = 100000000;
	count = count - 8;
	while (count-->0)
		val*=10;
	return	val;
}
*/
double gxs_ipow10R[] = 
{
	1,.1,.01,.001,.0001,.00001,.000001,.0000001,.00000001
};
static double ipow10 (int32 count)
{
	if (count<9)	return gxs_ipow10R[count];

	double val = .00000001;
	count = count - 8;
	while (count-->0)
		val*=.1f;
	return val;
}

// ================================================================================================================
//  xs_atoi
// ================================================================================================================
char* xs_atoi (const char *str, int32 *value, int32 len) 
{
	xs_Assert (str);

	char *estr		= len ? (char*)str+len : (char*)str+65536;	//should be plenty
	str				= xs_SkipSpaces (str);
	int32	curval	= 0;
	bool retval	= false;
	int32 mulval	= 1;
	while (*str && str<estr && xs_IsSpace(str)==false)  
		{
		if ((*str>='0') && (*str<='9')) 
			{
			curval *= 10;
			curval += *str - '0';
			retval = true;
			}
		else if (*str=='-' && retval==false) 
			{
			mulval = -1;
			}
		else if (*str=='+' && retval==false) 
			{
			//nothing
			}
		else 
			{
			//if(endstr == NULL)	retval = false;
			break;
			}
		str++;
		}

	if (value)	*value	= curval*mulval;
	return retval ? (char*)str : NULL;
}

// ================================================================================================================
//  xs_atou
// ================================================================================================================
char* xs_atou (const char *str, uint32 *value, int32 len) 
{
	xs_Assert (str);

	char *estr		= len ? (char*)str+len : (char*)str+65536;	//should be plenty
	str				= xs_SkipSpaces (str);
	int32	curval	= 0;
	bool retval	= false;
	while (*str && str<estr && xs_IsSpace(str)==false)  
		{
		if ((*str>='0') && (*str<='9')) 
			{
			curval *= 10;
			curval += *str - '0';
			retval = true;
			}
		else 
			{
			//if(endstr == NULL)	retval = false;
			break;
			}
		str++;
		}

	if (value)	*value	= curval;
	return retval ? (char*)str : NULL;
}

// ================================================================================================================
//  xs_atof
// ================================================================================================================
char* xs_atof (const char *str, real32 *value, int32 len)
{
#if xs_SMALLER
	real64 lvalue	= value ? *value : 0;
	char* estr		= xs_atod(str, &lvalue, len);
	if (value)		*value = lvalue;
	return estr;
#else
	if (str==0)		return 0;

	char *estr		= len ? (char*)str+len : (char*)str+65536;	//should be plenty
	int32 maxdigits	= 9;
	str				= xs_SkipSpaces (str);
	real64 curval	= 0;
	int32 digits	= 0;
	double floatval	= 0;
	bool neg		= false;
	bool negExp	= false;
	bool signOK	= true;
	bool dotOK	= true;
	bool expOK	= true;
	bool retval	= false;
	while (*str && str<estr && xs_IsSpace(str)==false)  
		{
		if ((*str>='0') && (*str<='9')) 
			{
			if(digits < maxdigits) 
				{
				curval *= 10;
				curval += *str - '0';
				signOK  = false;
				digits++;
				retval = true;
				}
			else { xs_Assert(0); }		// <- this is not going to fit
			}
		else if (signOK && ((*str == '-') || (*str == '+'))) 
			{
			if (expOK)	neg		= (*str == '-');
			else		negExp	= (*str == '-');
			signOK		= false;
			}
		else if (dotOK && (*str == '.'))
			{
			floatval	= curval;
			curval		= 0;
			digits		= 0;
			dotOK		= false;
			signOK		= false;
			}
		else if (expOK && ((*str == 'e') || (*str == 'E')))// || (*str == 'd') || (*str == 'D')))
			{
			xs_Assert ((*str == 'e') || (*str == 'E')); //not handling 'd' case
			floatval	+= float (curval)*ipow10(digits);	//means there was a decimal point
			curval		 = 0;
			digits		 = 0;
			dotOK		 = false;
			expOK		 = false;
			signOK		 = true;
			}
		else 
			{
			//if(endstr == NULL)	retval	= false;
			break;
			}
		str++;
		}

	int32 icv				= xs_CRoundToInt(curval);
	if (!expOK)				floatval *= negExp ? ipow10(icv) : pow10(icv);
	else if (!dotOK)		floatval += float (curval)*ipow10(digits);	//means there was a decimal point
	else					floatval  = curval; 

	if (neg)				floatval  = -floatval;


	if (value)	*value	= (float)floatval;
	return retval ? (char*)str : NULL;
#endif
}

// ================================================================================================================
//  xs_atod
// ================================================================================================================
char* xs_atod (const char *str, real64 *value, int32 len)
{
	if (str==0)		return 0;

	char *estr		= len ? (char*)str+len : (char*)str+65536;	//should be plenty
	int32 maxdigits	= 29;
	str				= xs_SkipSpaces (str);
	real64 curval	= 0;
	int32 digits	= 0;
	real64 floatval	= 0;
	bool neg		= false;
	bool negExp	= false;
	bool signOK	= true;
	bool dotOK	= true;
	bool expOK	= true;
	bool retval	= false;
	while (*str && str<estr && xs_IsSpace(str)==false)  
		{
		if ((*str>='0') && (*str<='9')) 
			{
			if(digits < maxdigits) 
				{
				curval *= 10;
				curval += *str - '0';
				signOK  = false;
				digits++;
				retval = true;
				}
			else { xs_Assert(0); }		// <- this is not going to fit
			}
		else if (signOK && ((*str == '-') || (*str == '+'))) 
			{
			if (expOK)	neg		= (*str == '-');
			else		negExp	= (*str == '-');
			signOK		= false;
			}
		else if (dotOK && (*str == '.'))
			{
			floatval	= curval;
			curval		= 0;
			digits		= 0;
			dotOK		= false;
			signOK		= false;
			}
		else if (expOK && ((*str == 'e') || (*str == 'E')))// || (*str == 'd') || (*str == 'D')))
			{
			xs_Assert ((*str == 'e') || (*str == 'E')); //not handling 'd' case
			floatval	+= float (curval)*ipow10(digits);	//means there was a decimal point
			curval		 = 0;
			digits		 = 0;
			dotOK		 = false;
			expOK		 = false;
			signOK		 = true;
			}
		else 
			{
			//if(endstr == NULL)	retval	= false;
			break;
			}
		str++;
		}

	int32 icv				= xs_CRoundToInt(curval);
	if (!expOK)				floatval *= negExp ? ipow10(icv) : pow10(icv);
	else if (!dotOK)		floatval += float (curval)*ipow10(digits);	//means there was a decimal point
	else					floatval  = curval; 

	if (neg)				floatval  = -floatval;


	if (value)	*value	= floatval;
	return retval ? (char*)str : NULL;
}


// ================================================================================================================
//  xs_StringIsNumber
// ================================================================================================================
int32 xs_StringIsNumber (const char *str, int32 len)
{
	real64 l;
	if (xs_atod(str, &l, len)==NULL)	return 0;
	if (real64(xs_RoundToInt(l))==l)	return 1;
	return 2;
}

// =================================================================================================================
// xs_itoa
// =================================================================================================================
char* xs_itoa (int32 val, char* str, int32 maxlength, int32 rad)
{
	char* start = str;
	if (str==NULL&&maxlength!=0)	return NULL;
	if (str!=NULL&&maxlength<2)		{if (str) *str=0; return NULL;}

	//negative
	if (val<0)	{*str++	= '-';	val=uint32(-((int32)val)); maxlength--;}
	
	//first char (for inversion)
	char* beg	= str;
	char* end	= str + maxlength - 1; //-1 for termination

	//loop for digits
	do //always do at least a 0
		{
		int32 oval		= val;
		val				= val/rad;
		uint32 digval	= oval-(val*rad); //same as: dival = uint32 (val%rad);

		if (digval>9)	*str++ = char(digval-10+'a');
		else			*str++ = char(digval   +'0');
		}
	while (val>0 && str<end);

	//terminate
	*str = 0;

	//reverse digits
	end	 = str-1;	//-1 for termination, -1 for last digit
	do
		{
		char temp	= *end;
		*end--		= *beg;
		*beg++		= temp;
		}
	while (beg<end);

	return start;
}


// =================================================================================================================
// xs_dtoa - not very robust
// =================================================================================================================
char* xs_dtoa (real64 rval, char* str, int32 maxlength)
{
/*
	xs_IEEEFloat& num = (xs_IEEEFloat&)rval;
	
	//negative sign
	if (num.sgn)	{*str++	= '-';	val=-val; maxlength--;}
*/
#ifdef Windows_
	_gcvt (rval, 12, str);
#else
	//xs_Incomplete();
	sprintf (str, "%g", rval);
#endif
	return str;
}

// =================================================================================================================
// xs_ftoa - not very robust
// =================================================================================================================
char* xs_ftoa (real32 rval, char* str, int32 maxlength)
{
/*
	xs_IEEEFloat& num = (xs_IEEEFloat&)rval;
	
	//negative sign
	if (num.sgn)	{*str++	= '-';	val=-val; maxlength--;}
*/
#ifdef Windows_
	_gcvt (rval, 6, str);
#else
	xs_Incomplete();
	sprintf (str, "%g", rval);
#endif
	return str;
/*
	real32 val	= rval;
	char *start = str;

	//negative
	if (val<0)	{*str++	= '-';	val=-val; maxlength--;}

	//round to int
	char* beg	= str;
	int32 i		= int32 (val);
	val			= val - i;
	if (i)		str		= xs_itoa (i, str, maxlength);
	else		str[0]	= 0;
	if (str)
		{
		str += strlen (str);

		//recover decimals
		while (val != real32(int32(val)))	val*=10;

		if (val)
			{
			//compute new length and add decimal
			maxlength += str-beg;
			if (maxlength)	
				{
				*str++ = '.';
				maxlength--;
				}

			//now add decimal digits
			str = xs_itoa (val, str, maxlength);
			}
		}

	return start;
*/
}






