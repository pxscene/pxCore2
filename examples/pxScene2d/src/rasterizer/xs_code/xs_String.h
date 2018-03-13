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
// xs_String.h
// ====================================================================================================================
// ====================================================================================================================
#ifndef _xs_String_H_ // for entire file
#define _xs_String_H_

#include "xs_Array.h"
#include "xs_Hash.h"

#ifdef PX_PLATFORM_MAC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#endif

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


// =================================================================================================================
// xs_String
// =================================================================================================================
template <class T, class BaseClass=xs_ArraySmall<T> > class xs_String : private BaseClass//private xs_ArrayBase <T, xs_SimpleAlloc<T> >
{
public:
	// =================================================================================================================
	// Constructor/Destructor - constructors always alias
	// =================================================================================================================
	xs_String()											{}
	xs_String(const xs_String& str, int32 n=-1)			{Copy (str, n);}
	xs_String(const T* str, int32 n=-1)					{Alias (str, n);}	//should only be used for const string literals
	~xs_String()										{Free();}

	xs_String& operator=(const T* str)					{if (str!=*this)  Copy (str); return *this;}
	xs_String& operator=(const xs_String& str)			{if (str!=Ptr())  Copy (str); return *this;}

	// =================================================================================================================
	// Attributes
	// =================================================================================================================
	finline bool		IsAlias			() const		{return BaseClass::IsAlias();		}

	// =================================================================================================================
	// Length Functions
	// =================================================================================================================
	finline int32		Space			() const		{int32 s=BaseClass::Space(); return s?(IsAlias()?s:s-1):0;}
	finline int32		Length			() const		{return BaseClass::Count() ? (IsAlias()?BaseClass::Count():BaseClass::Count()-1) : 0;}
	finline bool		IsEmpty			() const		{return Length()==0;}
	finline bool		Update			()				{return SetLength(xs_Strlen(Ptr(),Space()));}
															
	finline bool		SetLength		(int32 len)		{return BaseClass::SetCount(len ? (IsAlias()?len:len+1) : 0);}
	finline bool		SetSpace		(int32 len)		{return BaseClass::SetSpace(len?(IsAlias()?len:len+1):0);}
	finline bool		Compact			()				{return BaseClass::Compact();}
															
	finline bool		Free			()				{return BaseClass::Free();}	//frees the memory
	finline bool		Flush			()				{return BaseClass::Flush();}

	inline T&		operator [] (int32 i)			{xs_Assert(uint32(i)<uint32(BaseClass::Count()));	return BaseClass::Ptr()[i];	}
	inline const T&	operator [] (int32 i)	const	{xs_Assert(uint32(i)<uint32(BaseClass::Count()));	return BaseClass::Ptr()[i];	}

	// =================================================================================================================
	// String functions
	// =================================================================================================================
	bool				Append	(const T* str, int32 n=-1);
	finline bool		Set		(const T* str, int32 n=-1)				{Flush(); return Append(str,n);}
	finline bool		Copy	(const T* str, int32 n=-1)				{Flush(); return Append(str,n);}
	bool				Alias	(const T* str, int32 n=-1);

	//xs_String versions
	finline bool		Append	(const xs_String& str, int32 n=-1)		{return Append(str.Ptr(),n<0?str.Length():n);}
	finline bool		Set		(const xs_String& str, int32 n=-1)		{return Copy(str.Ptr(),n<0?str.Length():n);}
	finline bool		Copy	(const xs_String& str, int32 n=-1)		{return Copy(str.Ptr(),n<0?str.Length():n);}
	finline bool		Alias	(const xs_String& str, int32 n=-1)		{return Alias(str.Ptr(),n<0?str.Length():n);}


	// =================================================================================================================
	// Casting
	// =================================================================================================================
	inline const T*			Base() const	{return BaseClass::Ptr();}
	inline const T*			Get() const		{return BaseClass::Ptr();}
	inline const T*			Ptr() const		{return BaseClass::Ptr();}
	inline operator			T*() const		{return (T*)Ptr();}
	inline operator const	T*() const		{return Ptr();}

	inline xs_String& operator <<  (const T* str)			{Append(str); return *this;}
	inline xs_String& operator <<  (const xs_String& str)	{Append(str); return *this;}
	inline xs_String& operator <<  (xs_String& str)			{Append(str); return *this;}
	inline xs_String& operator <<  (const real64  d)		{char str[256]; xs_dtoa(d,str,256);	Append(str); return *this;}
	inline xs_String& operator <<  (const int32  i)			{char str[256]; xs_itoa(i,str,256);	Append(str); return *this;}
	inline xs_String& operator <<  (const uint32 u)			{char str[256]; xs_utoa(u,str,256);	Append(str); return *this;}

	// =================================================================================================================
	// string comparison
	// =================================================================================================================
	inline int32 Compare	  (const T* str)		  const		{ return xs_Strcmp(Ptr(), str);}
	inline int32 Compare	  (const xs_String<T>& str) const	{ return xs_Strcmp(Ptr(), str.Ptr());}
	
	inline bool operator== (const xs_String<T>& s2)  const	{ return Compare(s2) == 0; }
	inline bool operator!= (const xs_String<T>& s2)  const	{ return Compare(s2) != 0; }
	inline bool operator<  (const xs_String<T>& s2)  const	{ return Compare(s2) < 0;  }
	inline bool operator>  (const xs_String<T>& s2)  const	{ return Compare(s2) > 0;  }
	inline bool operator<= (const xs_String<T>& s2)  const	{ return Compare(s2) <= 0; }
	inline bool operator>= (const xs_String<T>& s2)  const	{ return Compare(s2) >= 0; }

	inline bool operator== (const char* s2)		  const		{ return Compare(s2) == 0; }
	inline bool operator!= (const char* s2)		  const		{ return Compare(s2) != 0; }
	inline bool operator<  (const char* s2)		  const		{ return Compare(s2) < 0;  }
	inline bool operator>  (const char* s2)		  const		{ return Compare(s2) > 0;  }
	inline bool operator<= (const char* s2)		  const		{ return Compare(s2) <= 0; }
	inline bool operator>= (const char* s2)		  const		{ return Compare(s2) >= 0; }

};


// =================================================================================================================
// xs_StringHashHelper class
// =================================================================================================================
class xs_StringHashHelper
{
public:
	static int HashCode(const char* s)		
	{
		if (s==0) return 0;
		int val = 0;
		while (*s)	val = 33*val + (*s++);
		return val;
	}
	static int Equal(const char* is, const char* js)		{return xs_Strcmp(is, js)==0;}
	static int Compare(const char* i, const char* j)		{return xs_Strcmp (i, j);}
};

// =================================================================================================================
// xs_StringHash
// =================================================================================================================
template <class KBase, class V> class xs_StringHash : public xs_Hash< xs_AutoPtrArray<KBase>, V, xs_StringHashHelper>
{
public:
	typedef xs_StringHashHelper	CCompare;
	typedef KBase*				K;
	typedef xs_Hash< xs_AutoPtrArray<KBase>, V, xs_StringHashHelper>  CBase;

	
	// =================================================================================================================
	// Functions - refer to xs_Hash for complete function list
	// =================================================================================================================
	V* Add (const K i)						{return Add_(i,CCompare::HashCode(i));}								
	inline V&	operator [] (const K i)		{return *Get(i, true);}

	// =================================================================================================================
	// Add_ - internal
	// =================================================================================================================
	V* Add_(const K i, int32 h)
	{
		xs_HashNode *node	= CBase::AddNode (h, 0, false);
		xs_HashPair<K,V>& pair = (xs_HashPair<K,V>&)CBase::Value(node);

		int32 len = xs_Strlen(i)+1;
		K key = new KBase[len+1];
		if (key==NULL)	return NULL;
		if (i) xs_Memcpy (key, i, len+1);
		else   key[0] = 0;

		new (&pair.first) K(key);
		new (&pair.second) V();
		return &pair.second;
	}

	// =================================================================================================================
	// Get
	// =================================================================================================================
	V* Get(const K i, bool create=false)
	{
		int32 h = CCompare::HashCode(i);
		xs_HashNode* node = CBase::Find(h);
		while (node)
			{
			xs_HashPair<K,V>& pair = (xs_HashPair<K,V>&)CBase::Value(node);
			if (CCompare::Equal(pair.first, i))
				return &pair.second;
			node = CBase::Next(node);
			}

		return create ? Add_(i, h) : NULL;
	}
};


// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// Implementation
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
    #ifndef _xs_Strcpy_
#define _xs_Strcpy_ 
#include <string.h>
finline int32	xs_Strlen(const char* str, long maxLen)			{long l=0; if (str) while (*str&&l<maxLen) {str++; l++;}	return l;}
finline char*   xs_Strcat(char* str1, const char*str2)			{char* os=str1; if(str1==0||str2==0) return str1; while(*str1) str1++; do {*str1++=*str2;} while (*str2++); return os;}
finline char*	xs_Strcpy(char* str1, const char*str2)			{if(str1==0||str1==str2) return str1; *str1=0; return xs_Strcat(str1, str2);}
finline int32   xs_Strcmp(const char *str1, const char *str2)	{int32 c=0; if (str1==str2) return 0;
																 if(str1==0||str2==0) return (str1==0)?(str2==0?0:1):-1;
																 while (*str1&&*str2&&c==0) c=*str1++ - *str2++;
																 return (c!=0) ? c : *str1-*str2;}

finline int32	xs_Strlen(const int16* str, long maxLen)		{long l=0; if (str) while (*str&&l<maxLen) {str++; l++;}	return l;}
finline int32   xs_Strlen(const int16 *str)						{int32 l=0; if (str) while (*str) {str++; l++;}	return l;}
finline int16*  xs_Strcat(int16* str1, const int16*str2)		{int16* os=str1; if(str1==0||str2==0) return str1; while(*str1) str1++; do {*str1++=*str2;} while (*str2++); return os;}
finline int16*	xs_Strcpy(int16* str1, const int16*str2)		{if(str1==0||str1==str2) return str1; *str1=0; return xs_Strcat(str1, str2);}
finline int32   xs_Strcmp(const int16 *str1, const int16 *str2)	{int32 c=0; if (str1==str2) return 0;
																 if(str1==0) return -1; if (str2==0) return 1;
																 while (*str1&&*str2&&c==0) c=*str1++ - *str2++;
																 return (c!=0) ? c : *str1-*str2;}
#endif




// =================================================================================================================
// Append
// =================================================================================================================
template<class T, class BaseClass> bool xs_String<T, BaseClass>::Append (const T* str, int32 n)
{
	if (str==NULL||n==0)	return true;
	
	//lengths
	int32 len		= (n<0) ? xs_Strlen(str) : n;
	int32 curlen	= Length();		
	
	//setlength (adds room for termination)
	if (SetLength(curlen+len)==false)			return false;
	if (str) xs_Memcpy ((char*)Ptr()+curlen, (void*)str, len*BaseClass::ElemSize());

	//and terminate
	if (BaseClass::Count() && BaseClass::Count()!=Length())	(*this)[BaseClass::Count()-1]=0;
	return true; 
}

// =================================================================================================================
// Alias
// =================================================================================================================
template<class T, class BaseClass> bool xs_String<T, BaseClass>::Alias (const T* str, int32 n)
{
	xs_Assert(str);
	//preserve the trailing 0
	int32 len = (n>0) ? n : xs_Strlen(str);
	return len ? BaseClass::Alias (str, len+1) : Flush();
}

#endif // for entire file

#ifdef PX_PLATFORM_MAC
#pragma clang diagnostic pop
#endif
