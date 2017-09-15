// ====================================================================================================================
// ====================================================================================================================
//  xs_Core.h
// ====================================================================================================================
// ====================================================================================================================
#ifndef _xs_Core_H_
#define _xs_Core_H_

#ifdef PX_PLATFORM_MAC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#endif

#undef Windows_
#if 0
#ifndef Windows_
#define Windows_					WIN32
#endif
#else
#include <stdio.h>
#endif
// ====================================================================================================================
// Types
// ====================================================================================================================
#ifndef _xs_Types_
#define _xs_Types_                  1
    typedef long                    int32;
    typedef unsigned long           uint32;
    typedef char                    int8;
    typedef short                   int16;
	typedef long long				int64;
    typedef unsigned char           uint8;
    typedef unsigned short          uint16;
	typedef unsigned long long		uint64;
    typedef float                   real32;
    typedef double                  real64;
#endif //_xs_Types_


// ====================================================================================================================
// Memory
// ====================================================================================================================
void* xs_PtrAlloc       (int32 count, int32 size=1);                      
void* xs_PtrRealloc     (void* p, int32 count, int32 size=1);           
bool  xs_PtrFree        (void* p);                                       
void* xs_Memmove        (void* p1, const void* p2, int32 count, int32 size=1);  
void* xs_Memcpy         (void* p1, const void* p2, int32 count, int32 size=1);   
void* xs_Memset         (void* p,  int32 data, int32 count, int32 size=1);   
void* xs_Memzero        (void* p,  int32 count, int32 size=1);   
int32 xs_Memcmp			(const void* p1,  const void* p2, int32 size);


// ====================================================================================================================
// Basic stuff
// ====================================================================================================================
#ifndef _xs_BigEndian_
#define _xs_BigEndian_					0									//intel is little endian
#define _xs_LittleEndian_				(_xs_BigEndian_==0)       
#endif

#ifdef _MSC_VER
#define finline                     	__forceinline
#else
#define finline                     	inline
#endif

// ================ basic math
#define xs_MAXINT                   	0x7ffffff
#define xs_NULL                     	0
#define xs_Clamp(a,b,c)		        	xs_Min(xs_Max(a,b), c)
#define xs_Wrap(a,b,c)					((a)<(b)) ? (c) : (((a)>(c)) ? a=(b) : (a))
#define xs_Abs(a)						(((a)<0) ? (-(a)) : (a))
#define xs_Eps							.0001
#define xs_Epsf							real32(.0001)
#define xs_pif							real32(3.141592)
#define xs_RangeClamp(a,b)				(((a)&(~(b))) ? (b & uint32(-(int32(a)>=0))) : (a)) //only for pow2-1
#define xs_GetBit(a,f)					(((a)&(f))!=0)
#define xs_SetBit(a,f,c)				if (c) a|=(f); else a&=~(f)
#define xs_BitArrPad(s)					((s+7)>>3)
#define xs_BitArrGet(arr,f)				((((char*)arr)[f>>3]&(1<<(f&7)))!=0)
#define xs_BitArrSet(arr,f,c)			if (c) ((char*)arr)[f>>3]|=(1<<(f&7)); else ((char*)arr)[f>>3]&=~(1<<(f&7))

finline int32		xs_ArithShift(int32 x, int32 b)	{return (x>>b)+(uint32(x)>>31);}
finline int32  		xs_Half(int32 x)				{return x>>1;}						//useful for templates
finline real64 		xs_Half(real64 x)				{return x*.5;}
finline bool   		xs_PowerOf2_(uint32 x)			{return  (!((x)&((x)-1)));}			//Is x power of 2? 0 included
finline bool   		xs_PowerOf2(uint32 x)			{return ((!((x)&((x)-1))) && (x));}	//Is x power of 2? 0 excluded
finline real64 		xs_FInverse(real64 x)			{return (x!=0) ? 1/x : 0;}
finline real32 		xs_FInverse(real32 x)			{return (x!=0) ? 1/x : 0;}
finline uint32 		xs_RoundUpPow2(uint32 v)		{v--; v|=v>>1; v|=v>>2; v|=v>>4; v|=v>>8; v|=v>>16; return v+1;}
finline uint32 		xs_Mod(uint32 a, uint32 b)		{if (xs_PowerOf2_(b)) return a&(b-1); return a%b;}

inline int32 		xs_SafeIncr(int32 *i)			{return ++*i;}
inline int32 		xs_SafeDecr(int32 *i)			{return --*i;}

// ================ assertions
#if _DEBUG
    #include <assert.h>
#include <windows.h>
    #define xs_Verify(e)             if((e)==0) DebugBreak()
    #define xs_Assert(e)             if((e)==0) DebugBreak()
#else
    #define xs_Verify(e)             (e)
    #define xs_Assert(e)
#endif

// ================ min/max functions
#define xs_MINMAX
inline int8		xs_Min	(int8	a,	int8	b)		{ return a<b ? a : b; }
inline int16	xs_Min	(int16	a,	int16	b)		{ return a<b ? a : b; }
inline int32	xs_Min	(int32	a,	int32	b)		{ return a<b ? a : b; }
inline int64	xs_Min	(int64	a,	int64	b)		{ return a<b ? a : b; }
inline uint8	xs_Min	(uint8	a,	uint8	b)		{ return a<b ? a : b; }
inline uint16	xs_Min	(uint16	a,	uint16	b)		{ return a<b ? a : b; }
inline uint32	xs_Min	(uint32	a,	uint32	b)		{ return a<b ? a : b; }
inline uint64	xs_Min	(uint64	a,	uint64	b)		{ return a<b ? a : b; }
inline real32	xs_Min	(real32	a,	real32	b)		{ return a<b ? a : b; }
inline real64	xs_Min	(real64	a,	real64	b)		{ return a<b ? a : b; }

inline int8		xs_Max	(int8	a,	int8	b)		{ return a>b ? a : b; }
inline int16	xs_Max	(int16	a,	int16	b)		{ return a>b ? a : b; }
inline int32	xs_Max	(int32	a,	int32	b)		{ return a>b ? a : b; }
inline int64	xs_Max	(int64	a,	int64	b)		{ return a>b ? a : b; }
inline uint8	xs_Max	(uint8	a,	uint8	b)		{ return a>b ? a : b; }
inline uint16	xs_Max	(uint16	a,	uint16	b)		{ return a>b ? a : b; }
inline uint32	xs_Max	(uint32	a,	uint32	b)		{ return a>b ? a : b; }
inline uint64	xs_Max	(uint64	a,	uint64	b)		{ return a>b ? a : b; }
inline real32	xs_Max	(real32	a,	real32	b)		{ return a>b ? a : b; }
inline real64	xs_Max	(real64	a,	real64	b)		{ return a>b ? a : b; }


template <class T> class xs_Swap	{public: static finline void Swap(T& a, T& b) {T t; t=a; a=b; b=t;}};



// ====================================================================================================================
// ====================================================================================================================
// Memory - Implementation
// ====================================================================================================================
// ====================================================================================================================
//you can override the implementation by 
//defining _xs_MemoryAlloc_ in your project
//and then implementing to the prototypes above

#ifndef _xs_MemoryAlloc_
#define _xs_MemoryAlloc_ 
#include <stdlib.h>
inline void* xs_PtrAlloc    (int32 count, int32 size)                    	{return malloc(count*size);}
inline void* xs_PtrRealloc  (void* p, int32 count, int32 size)           	{return realloc(p, count*size);}
inline bool  xs_PtrFree     (void* p)                                    	{free(p); return true;}
#endif

#ifndef _xs_MemoryMove_
#define _xs_MemoryMove_ 
#include <string.h>
inline void* xs_Memmove (void* p1, const void* p2, int32 count, int32 size) {return memmove(p1,p2,count*size);}
inline void* xs_Memcpy  (void* p1, const void* p2, int32 count, int32 size)	{return memcpy(p1,p2,count*size);}
inline void* xs_Memset  (void* p,  int32 data, int32 count, int32 size)		{return memset(p,data,count*size);}
inline void* xs_Memzero (void* p,  int32 count, int32 size)					{return memset(p,0,count*size);}
inline int32 xs_Memcmp  (const void* p1,  const void* p2, int32 size)		{return memcmp(p1,p2,size);}
#endif




// ====================================================================================================================
// ====================================================================================================================
// =================================================================================================================
// ANSI memory equivalence
// =================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
#define xs_Memcmp_(a,b,c)		((int32)xs_Memcmp__  ((uint8*)(a),(uint8*)(b),c))
#define xs_Memcpy_(a,b,c)		((void*)xs_Memcpy__  ((uint8*)(a),(uint8*)(b),c))
#define xs_Memmove_(a,b,c)		((void*)xs_Memmove__ ((uint8*)(a),(uint8*)(b),c))
#define xs_Memset_(a,b,c)		((void*)xs_Memset__  ((uint8*)(a),b,c))
#define xs_Memzero_(a,b,c)		((void*)xs_Memset__  ((uint8*)(a),b,c))

#define xs_DuffRepeat(OPER,LEN)		\
	{if(LEN>0)						\
		{							\
		int32 n=(LEN+3)>>2;			\
		switch (LEN & 3)			\
			{						\
			case 0: do {OPER;		\
			case 3:		OPER;		\
			case 2:		OPER;		\
			case 1:		OPER;}		\
					while (--n > 0);\
			}						\
		}							\
	}

// ========================
// memcpy
// ========================
finline void* xs_Memcpy__(uint8* dst, uint8* src, int32 len)		
{
	if(dst==NULL||src==NULL) return dst; 
	if(dst==src)			 return dst;
#if xs_SMALLER
#else
	if ((uint32(src)&3)==0 && (uint32(dst)&3)==0)
		{
		int32 l4=len&~3;
		len -= l4;
			{
			l4>>=2;
			uint32* dl = (uint32*)dst;
			uint32* sl = (uint32*)src;
			xs_DuffRepeat (*dl++=*sl++, l4);
			dst = (uint8*)dl;
			src = (uint8*)sl;
			}
		if (len==0)	return (void*)dst;
		}
#endif
	xs_DuffRepeat (*dst++=*src++, len);	
	return (void*)dst;
}

// ========================
// memset
// ========================
finline void* xs_Memset__(uint8* mem, uint8 c, int32 len)		
{
	if(mem==NULL)			  return mem; 
#if xs_SMALLER
#else
	if ((uint32(mem)&3)==0)
		{
		int32 l4=len&~3;
		len -= l4;
			{
			l4>>=2;
			uint32* ml = (uint32*)mem;
			uint32 cc  = (c<<24) | (c<<16) | (c<<8) | c;
			xs_DuffRepeat (*ml++=cc, l4);
			mem = (uint8*)ml;
			}
		}
#endif
	xs_DuffRepeat (*mem++=c, len);		
	return(void*)mem;
}


// ========================
// memset
// ========================
finline void* xs_Memzero__(uint8* mem, int32 len)		
{
	if(mem==NULL)			  return mem; 
#if xs_SMALLER
#else
	if ((uint32(mem)&3)==0)
		{
		int32 l4=len&~3;
		len -= l4;
			{
			l4>>=2;
			uint32* ml = (uint32*)mem;
			xs_DuffRepeat (*ml++=0, l4);
			mem = (uint8*)ml;
			}
		}
#endif
	xs_DuffRepeat (*mem++=0, len);		
	return(void*)mem;
}

// ========================
// memmove
// ========================
finline void* xs_Memmove__(uint8* dst, uint8* src, int32 len)
	{
	//already there
	if (dst==src || len<=0 || dst==NULL || src==NULL) return (void*)dst;

	//slice it
	uint8* srce	= src+len;
	if (dst>src && dst<srce)	
		{
		//problem only if the dst is in the middle of the source
		//otherwise, we can just use memcpy
		int32 endoverlap	= srce-dst;		xs_Assert(endoverlap<len);
		xs_Memcpy__			(dst+len-endoverlap, srce-endoverlap, endoverlap);
		len					= len-endoverlap;
		if (len<=0)			{xs_Assert(len==0); return (void*)dst;}
		}
	
	//memcopy what's left
	return xs_Memcpy__ (dst, src, len);
	}

// ========================
// memcmp
// ========================
finline int32 xs_Memcmp__(uint8* dst, uint8* src, int32 len)
{
	if (dst==src)				return 0;
	if (dst==0)					return 1;
	if (src==0)					return -1;

	//duff's device (from flipcode)
	#define xs_COMPARE_MEM		d = *src++ - *dst++; if (d) return d;	
	#define xs_COMPARE_MEM_L	d = *sl++  - *dl++;  if (d) return d;	
	int32 d;

#if xs_SMALLER
#else
	if ((uint32(src)&3)==0 && (uint32(dst)&3)==0)
		{
		int32 l4=len&~3;
		len -= l4;
			{
			l4>>=2;
			uint32* dl = (uint32*)dst;
			uint32* sl = (uint32*)src;
			xs_DuffRepeat (xs_COMPARE_MEM_L, l4);
			dst = (uint8*)dl;
			src = (uint8*)sl;
			}
		}
#endif

	xs_DuffRepeat (xs_COMPARE_MEM, len);
	#undef xs_COMPARE_MEM
	#undef xs_COMPARE_MEM_L
  
  return 0;
}

inline void  xs_Incomplete()
{
}

// ====================================================================================================================
// ====================================================================================================================
#endif // _xs_Core_H_

#ifdef PX_PLATFORM_MAC
#pragma clang diagnostic pop
#endif
