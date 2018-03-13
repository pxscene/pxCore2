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
//  xs_ArrayBase.h
// ====================================================================================================================
// ====================================================================================================================
#ifndef _xs_ARRAY_H_
#define _xs_ARRAY_H_

#include "xs_Core.h"
#include "xs_Ptr.h"

#ifdef _MSC_VER
	#include <new.h>
	#pragma warning(disable : 4127)	//disables "conditional expression is constant"
	#pragma warning(disable : 4345)	//disables "default POD ctor for objects with no ctor"
#else
	#include <new>
#endif // Windows_


//configuration
#ifndef XS_ARRAY_SMALLER
#define XS_ARRAY_SMALLER	0
#endif


// ================================================================================================================
//  xs_Alloc
// ================================================================================================================
class xs_Alloc
{
public:
	static inline void*	Alloc(int32 size)				{return xs_PtrAlloc(size);}
	static inline void*	Realloc(void* p, int32 size)	{return xs_PtrRealloc(p,size);}
	static inline bool	Free(void* p)					{return xs_PtrFree(p);}
};


// ================================================================================================================
//  xs_TDynArray
// ================================================================================================================
template<class T, class CBase, class CAlloc=xs_Alloc > class xs_TDynArray : public CBase
{
public:
	typedef  CBase	xs_TDynArrayBase;


	// ================================================================================================================
	//  Constructor/Destructor
	// ================================================================================================================
	xs_TDynArray(int32 count)		{ CBase::Zero(); xs_Verify(Alloc(count)); }
	xs_TDynArray()					{ CBase::Zero(); }
	~xs_TDynArray()					{ Free(); }

	// ================================================================================================================
	//  Functions
	// ================================================================================================================
	inline int32	Count() const				{ return CBase::Count();					}                                                                                                                                                                                             
    inline bool		IsValid() const				{ return (Ptr() != NULL);					}
	inline bool		IsAlias() const 			{ return CBase::mExternal!=0;				}
	inline int32	Bytes()	const				{ return CBase::Count()*sizeof(T);			}
	inline int32	SpaceBytes()	const		{ return CBase::Space()*sizeof(T);			}
	inline bool		InRange(int32 i) const		{ return uint32(i)<uint32(CBase::Count());	}
	inline int32	ElemSize()					{ return sizeof(T);							}
	inline bool		Flush()						{ return SetCount(0);						} 
	inline T*		Base() const				{ return Ptr();         					}
	inline T*		Begin() const				{ return Base();							}
	inline T*		End()						{ return Ptr() ? Ptr() + CBase::Count() : 0;}
    inline T*       Ptr() const					{ return CBase::mPtr;						}
	inline T*		Ptr(int32 c) const			{ xs_Assert(InRange(c)); return Ptr()+c;		}
    inline T*       PtrSafe (int32 i) const		{ if(InRange(i)) return Ptr()+i;	return NULL;}

	//allocation functions
	inline bool	SetCount(int32 count)			{ return count<=CBase::Space() ? CBase::SetCount_(count), true : Alloc(count); }
	inline bool	EnsureCount(int32 index)		{ if (index>=Count()) 
														return SetCount(index); 
												  return true;								}

 	inline bool	Free()							{ return Alloc_(0,		0,  -1, NULL);				} 
	inline bool	SetSpace(int32 s)				{ return Alloc_(xs_Min(Count(),s), s,  -1, NULL);	}
   
	inline T* 	Add(const T& v)									{ return Insert(Count(), v);}
	inline T* 	Add(const T* data, int32 n=1)					{ return Insert(Count(), data, n);}
	inline T* 	AddCount(int32 count)							{ int32 o=Count(); if (Alloc(count+o)==false)				return NULL; return Ptr()+o;}
	inline T* 	Insert(int32 before, const T& data)				{ int32 o=Count(); if (Alloc(1+o, before, &data)==false)	return NULL; return Ptr()+xs_Min(uint32(before),uint32(o)); }
	inline T* 	Insert(int32 before, const T* data, int32 n=1)	{ int32 o=Count(); if (Alloc(n+o, before, data)==false)		return NULL; return Ptr()+xs_Min(uint32(before),uint32(o)); }
	inline T* 	InsertCount(int32 before, int32 n=1)			{ int32 o=Count(); if (Alloc(n+o, before)==false)			return NULL; return Ptr()+xs_Min(uint32(before),uint32(o)); }
	inline bool Remove(int32 before, int32 n=1)					{ int32 o=Count(); if (Alloc(o-n, before)==false)			return false; return true; } 

	//stack operations
	inline void	Pop()				{xs_Assert(CBase::Count()); CBase::SetCount_(CBase::Count()-1);}
	inline void Pop(T& v)			{xs_Assert(CBase::Count()); ByteCopyElem(&v,End()-1); CBase::SetCount_(CBase::Count()-1);}
	inline bool Push(const T& v)	{int32 o=CBase::Count(); if (o+1<CBase::Space()) {*End()=v; CBase::SetCount_(o+1); return true;}
									 return Add(v)!=0;}


	// =================================================================================================================
	// simple operators
	// =================================================================================================================
	inline bool		operator ==(const T* data)	const	{return Base()==data;	}				
	inline bool		operator !=(const T* data)	const	{return Base()!=data;	}
    inline bool		operator !() const					{return Count()==0;		}

	inline int32	LastIndex ()			const	{xs_Assert(CBase::Count());	return Count()-1;	}
	inline int32	LastIndex (int32 i)		const	{xs_Assert(CBase::Count());	return Count()-1-i;	}
    inline T&       LastValue ()			const   {return Ptr()[LastIndex()];					}
    inline T&       LastValue (int32 i)		const   {return Ptr()[LastIndex(i)];					}
    inline T*       LastPtr ()				const   {return Ptr()+LastIndex();					}
    inline T*       LastPtr (int32 i)		const   {return Ptr()+LastIndex(i);					}

	inline T&		operator [] (int32 i)			{xs_Assert(uint32(i)<uint32(Count()));	return Ptr()[i];	}
	inline const T&	operator [] (int32 i)	const	{xs_Assert(uint32(i)<uint32(Count()));	return Ptr()[i];	}

	// ================================================================================================================
	//  Alias
	// ================================================================================================================
	bool Alias (const xs_TDynArray<T,xs_TDynArrayBase>& arr)	{return Alias(arr.Base(), arr.Count());}
	bool Alias (const T* data, int32 count)
	{
		xs_Assert(data);
		Free();
		CBase::mExternal	= true;
		CBase::mPtr		= (T*)data;
		CBase::SetSpace_	(count);
		CBase::SetCount_	(count);
		return true;
	}

	// ================================================================================================================
	//  Copy
	// ================================================================================================================
	bool Copy (const xs_TDynArray<T,xs_TDynArrayBase>& arr)	{return Copy(arr.Base(), arr.Count());}
	bool Copy (const T* data, int32 count)
	{
		if (SetCount(count)==false)	return false;
		if (data==NULL)				return true; //early terminate
		if (CBase::IsClass())
			for (int32 i=0; i<Count(); i++)
				CBase::mPtr[i] = data[i];
		else
			xs_Memcpy (CBase::mPtr, data, Bytes());
		return true;
	}

	bool Split (xs_TDynArray<T,xs_TDynArrayBase>& dst, int32 insLocation, int32 location, int32 count)
	{
		count		= xs_Min(count, Count()-location);
		location	= xs_Clamp(location, 0, Count());
		insLocation	= xs_Clamp(insLocation, 0, dst.Count());

		if (dst.Alloc_(count, count, -1, 0, true)==0)						
			return false;
		if (xs_Insert (dst.Ptr(), 0, ElemSize(), Ptr(location), insLocation, count)==false)
			return false;
		if (xs_Delete(Ptr(), Count(), ElemSize(), location, count)==false)	
			return false;
		SetCount_(Count()-count);
		return true;
	}

	// ================================================================================================================
	//  Allocation Implementation
	// ================================================================================================================
	inline bool	Alloc(int32 newCount, int32 location=-1, const T* srcData=NULL)
	{
		if (RequireAlloc(newCount, location, srcData))
			return Alloc_(newCount, -1, location, srcData);
		return true;
	}

	//actual function
	inline T* AdjustPtr_ (T* p, int32 c)	{return (T*)(((char*)p)+c);}
	bool Alloc_ (int32 newCount, int32 newSpace, int32 location, const T* srcData, bool noClass=false)
	{
		if (newCount==Count() && newSpace==-1)	return true;
		int32 p = CBase::Padding();
		if (newCount<=0 && newSpace==0)
			{
			//deconstruct
			if (CBase::IsClass()&&noClass==false)
				for (int32 i=0; i<Count(); i++)
					CBase::mPtr[i].~T();

			//free
			xs_Assert(newCount==0); //negative allocations are not valid
			if (CBase::mPtr&&CBase::mExternal==false)
				{
				//printf ("freeee....\n");
				CAlloc::Free (AdjustPtr_(CBase::mPtr,-p));
				}

			CBase::mPtr		= NULL;
			CBase::mExternal	= 0;
			CBase::SetCount_	(0);
			CBase::SetSpace_	(0);
			return true;
			}

		//delete elements
		if (newCount<Count())
			{
			if (uint32(location)>=uint32(newCount))	location=newCount;
			int32 delta = Count()-newCount;

			//destruct
			if (CBase::IsClass()&&noClass==false)
				for (int32 i=location; i<location+delta; i++)
					CBase::mPtr[i].~T();

			if (uint32(location)<uint32(newCount))
				xs_Delete (CBase::mPtr, Count(), sizeof(T), location, Count()-newCount);
			}

		//alloc or realloc
		if (CBase::mExternal)	{xs_Assert(0); return false;}
		if (newSpace<0)
			newSpace = CBase::Compute (newCount);

		T* newMem;
		if (newSpace != CBase::Space())
			{
			newMem		= CBase::mPtr ?(T*)CAlloc::Realloc(AdjustPtr_(CBase::mPtr,-p), newSpace*sizeof(T)+p) :
								(T*)CAlloc::Alloc(newSpace*sizeof(T)+p);
			if (newMem==0)		{xs_Assert(0); return false; }
			newMem		= AdjustPtr_(newMem, p);
			}
		else
			{
			newMem		= CBase::mPtr;
			newSpace	= CBase::Space();
			}

		//create elements
		if (newCount>Count())
			{
			if (uint32(location)>=uint32(Count()))	location=Count();

			//make room
			int32 delta = newCount-Count();
			if (location<Count())
				xs_Insert (newMem, Count(), sizeof(T), NULL, location, delta);

			//construct
			if (CBase::IsClass()&&noClass==false)
				{
				if (srcData)	
					for (int32 i=location; i<location+delta; i++)
						new (newMem+i) T(srcData[i-location]);		//copy constructor
				else
					for (int32 i=location; i<location+delta; i++)
						new (newMem+i) T();						//default constructor
				}
			else if (srcData)	
				xs_Memcpy (newMem+location, srcData, delta*sizeof(T));
			}
		
		CBase::mPtr	= newMem;
		CBase::SetCount_ (newCount);	
		CBase::SetSpace_ (newSpace);
		return true;
	}

	//helper class
	struct sByteElem{ char m[sizeof(T)];};
	static finline void ByteCopyElem(T* a, T* b)	{*(sByteElem*)a = *(sByteElem*)b;}
};




// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

// ================================================================================================================
//  xs_MemBase
// ================================================================================================================
template<class T, bool tIsClass> class xs_MemBase
{
protected:
	T*			mPtr;			// data ptr
	int32		mCount:31;		// count in sizeof(T) bytes
	int32		mExternal:1;

	// ================================================================================================================
	//  Allocation/Deallocation
	// ================================================================================================================
	inline void Zero()	{ mPtr=NULL; mExternal=false; mCount=0; }
	inline bool RequireAlloc (int32 newCount, int32 location, const T* srcData)
	{
		if (newCount==mCount)	return false;
		return true;
	}

	inline int32 Compute(int32 s)	{ return s;						}
	inline int32 Padding()			{ return 0;						}

public:
	// ================================================================================================================
	//  Public Functions
	// ================================================================================================================
	inline bool IsClass() const		{ return tIsClass;				}
	inline int32 Count() const		{ return mCount;				}
	inline int32 Space() const		{ return mCount;				}
	inline void SetCount_(int32 c)	{ mCount = c;					}
	inline void SetSpace_(int32 s)	{ mCount = xs_Min(s,mCount);	}
};


// ================================================================================================================
//  xs_MemArraySmall
// ================================================================================================================
template<class T, bool tIsClass> class xs_MemArraySmall
{
protected:
	T*			mPtr;			// data ptr
	int32		mCount:31;		// count in sizeof(T) bytes
	int32		mExternal:1;


	// ================================================================================================================
	//  Allocation/Deallocation
	// ================================================================================================================
	inline void Zero()	{ mPtr=NULL; mExternal=false; mCount=0;}
	inline bool RequireAlloc (int32 newCount, int32 location, const T* srcData)
	{
		if (newCount<=Space() && location==-1)	
			{
			if (IsClass())
				{
				if (newCount<mCount)
					for (int32 i=newCount; i<mCount; i++)
						mPtr[i].~T();
				if (newCount>mCount)
					{
					if (srcData)
						{
						for (int32 i=mCount; i<newCount; i++)
							new (mPtr + i) T(srcData[i-mCount]);
						}
					else
						{
						for (int32 i=mCount; i<newCount; i++)
							new (mPtr + i) T();
						}
					}
				}
			else if (newCount>mCount)
				xs_Memcpy (mPtr+mCount, srcData, (newCount-mCount)*sizeof(T));
			mCount = newCount; 
			return false;
		}

		return true;
	}

	inline int32 Compute(int32 s)	{ if (s<((mCount>>1)+mCount+1))	
										return mCount ? mCount*2 : 1; 
									  return xs_Max(Space(),s);}
	inline int32 Padding() const	{ return sizeof(T)>4 ? 8 : 4;}
	inline int32* SpacePtr() const	{ return (int32*)((char*)mPtr - Padding());}

public:
	// ================================================================================================================
	//  Public Functions
	// ================================================================================================================
	inline bool IsClass() const		{ return tIsClass;				}
	inline int32 Count() const		{ return mCount;				}
	inline void SetCount_(int32 c)	{ mCount = c;					}
	inline int32 Space() const		{ return mPtr&&mExternal==false ? *SpacePtr() : Count();}
	inline void SetSpace_(int32 s)	{ if (mPtr&&mExternal==false) *SpacePtr()=s;}
};

// ================================================================================================================
//  xs_MemArray
// ================================================================================================================
template<class T, bool tIsClass> class xs_MemArray
{
protected:
	T*			mPtr;		// data ptr
	int32		mCount;		// count in sizeof(T) bytes
	int32		mExternal:1;
	int32		mSpace:31;

	// ================================================================================================================
	//  Allocation/Deallocation
	// ================================================================================================================
	inline void Zero()	{ mPtr=NULL; mExternal=false; mCount=0; mSpace=0;}
	inline bool RequireAlloc (int32 newCount, int32 location, const T* srcData)
	{
		if (newCount<=Space() && srcData==NULL && location==-1)	
			{
			if (newCount<mCount)
				for (int32 i=newCount; i<mCount; i++)
					mPtr[i].~T();
			if (newCount>mCount)
				for (int32 i=mCount; i<newCount; i++)
					new (mPtr + i) T();
			mCount = newCount; 
			return false;
			}

		return true;
	}

	inline int32 Compute(int32 s)	{ if (s<=Space()) return Space();
									  if (s<((mCount>>1)+mCount+1))	
									  return mCount*2+1; return xs_Max(Space(),s);}
	inline int32 Padding() const	{ return 0;}

public:
	// ================================================================================================================
	//  Public Functions
	// ================================================================================================================
	inline bool IsClass() const		{ return tIsClass;				}
	inline int32 Count() const		{ return mCount;				}
	inline void SetCount_(int32 c)	{ mCount = c;					}
	inline int32 Space() const		{ return mSpace;				}
	inline void SetSpace_(int32 s)	{ mSpace = s;					}
};


#if XS_ARRAY_SMALLER
#define _arrayMemBase	xs_MemArraySmall
#else
#define _arrayMemBase	xs_MemArray
#endif

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
template<class T, class A=xs_Alloc>	class xs_Array :			public xs_TDynArray< T, _arrayMemBase <T, false>, A > {};
template<class T, class A=xs_Alloc>	class xs_ClassArray :		public xs_TDynArray< T, _arrayMemBase <T, true>, A > {};
template<class T, class A=xs_Alloc>	class xs_ClassPtrArray :	public xs_TDynArray< xs_AutoPtr<T>, _arrayMemBase < xs_AutoPtr<T>, true>, A > {};
template<class T, class A=xs_Alloc>	class xs_MemBlock :			public xs_TDynArray< T, xs_MemBase<T, false>, A > {};

template<class T, class A=xs_Alloc>	class xs_ArraySmall :		public xs_TDynArray< T, xs_MemArraySmall <T, false>, A > {};

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================


#if _MSC_VER
	#pragma warning(default : 4127)		//renables "conditional expression is constant"
	#pragma warning(default : 4345)		//renables "default POD ctor for objects with no ctor"
#endif // Windows_

#endif // for entire file
