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
//  xs_Ptr.h
// ====================================================================================================================
// ====================================================================================================================
#ifndef _xs_PTR_H_
#define _xs_PTR_H_


// ====================================================================================================================
// Classes
// ====================================================================================================================
class ixs_Counted;
class xs_RefCount;

//templates
template<class T>									class xs_AutoPtr;
template<class T>									class xs_RefPtr;
template<class T, class CCompare=T, class keyT=T>	class xs_BinSearch;
template <class T>									class xs_BlockPtr;

bool xs_Insert (const void* dst, int32 count, int32 elemSize, 
				const void* src, int32 index, int32 insCount);
bool xs_Delete (const void* dst, int32 count, int32 elemSize, 
								 int32 index, int32 delCount);




// =================================================================================================================
// xs_BlockPtr
// =================================================================================================================
template <class T> class xs_BlockPtr
{
public:
	// =================================================================================================================
	// Data
	// =================================================================================================================
	T*	mPtr;

	// =================================================================================================================
	// Constructor/Destructor
	// =================================================================================================================
	xs_BlockPtr()	{mPtr=0;}
	~xs_BlockPtr()	{Alloc(0);}

	// =================================================================================================================
	// Functions
	// =================================================================================================================
	bool Alloc(int32 count)
	{
		T* p = mPtr;
		if (count)
			{
			if (p)		p = (T*)xs_PtrRealloc(p, count*sizeof(T));	
			else		p = (T*)xs_PtrAlloc(count*sizeof(T));	
			if (p==0)	{xs_Assert(0); return false;}
			}
		else if (p)		{(void)xs_PtrFree(p); p=0;}
		mPtr = p;
		return true;
	}

	bool Free()	{return Alloc(0);}

	// =================================================================================================================
	// Access
	// =================================================================================================================
	inline const T*		Ptr() const					{return mPtr;	}
	inline T*			Ptr()						{return mPtr;	}
	inline T&			operator [] (int32 i)		{return mPtr[i];} //no safety, just convenience
	inline const T&		operator [] (int32 i) const {return mPtr[i];} //no safety, just convenience
};



// ====================================================================================================================
// xs_AutoPtr
// ====================================================================================================================
template <class T> class xs_AutoPtr
{
public:
	T*	mPtr;

    // ====================================================================================================================
	// Constructor/Destructor
    // ====================================================================================================================
	xs_AutoPtr()									{mPtr = NULL;}
	xs_AutoPtr(T* ptr)								{mPtr = ptr;}
	xs_AutoPtr(const xs_AutoPtr<T>& dp)				{mPtr = ((xs_AutoPtr<T>&)dp).Detach();}
	~xs_AutoPtr()									{Free();} 

	void operator =(xs_AutoPtr<T> &dp)				{mPtr = dp.Detach();}
	void operator =(T *t)							{Set(t);}

    T& operator *()							const	{return *mPtr;}
    T* operator ->()						const	{return mPtr;}
	inline	operator T*()					const	{return mPtr;}
	inline	operator const T*()				const	{return mPtr;}
	bool	operator ==(const T* data)		const	{return mPtr==data;}				
	bool	operator !=(const T* data)		const	{return mPtr!=data;}
    bool	operator !()					const	{return mPtr == 0;}
	T*		Ptr()							const	{return mPtr;}

	// =================================================================================================================
	// Functions
	// =================================================================================================================
	T*		Detach()				{T* ptr=mPtr;   mPtr = NULL; return ptr;}
	void	Free()					{T* ptr=mPtr;   mPtr = NULL; if (ptr) delete ptr;}
	void	Set(T* ptr)				{Free();        mPtr = ptr;}
};



// ================================================================================================================
//  xs_AutoPtrArray - for use with xs_AutoPtr and simple type for arrays
// ================================================================================================================
template <class T> class xs_AutoPtrArray
{
public:
	T*	mPtr;

	xs_AutoPtrArray(T* ptr)		{mPtr = ptr;}
	xs_AutoPtrArray()			{mPtr = 0;}
	~xs_AutoPtrArray()			{if (mPtr) delete [] mPtr;}

    inline T& operator *()		const	{return *mPtr;}
    inline T* operator ->()		const	{return mPtr;}
    inline operator T*()		const	{return mPtr;}
    inline operator const T*()	const	{return mPtr;}
};



// ================================================================================================================
//  xs_RefPtr
// ================================================================================================================
template<class T> class xs_RefPtr 
{
protected:
    T *mPtr;
    
	// ================================================================================================================
	//  illegal ops
	// ================================================================================================================
#if Debug_
	void operator +(void*)	{ xs_Assert(0); }
	void operator -(void*)	{ xs_Assert(0); }
	void operator [](void*) { xs_Assert(0); }
	void operator <(void*)	{ xs_Assert(0); }
	void operator <=(void*) { xs_Assert(0); }
	void operator >(void*)	{ xs_Assert(0); }
	void operator >=(void*) { xs_Assert(0); }
#endif

public:
 	// ================================================================================================================
	//  Constructor/Destructor
	// ================================================================================================================
	xs_RefPtr()											{mPtr=0;}
    xs_RefPtr(T *t, bool addRef=true) : mPtr(t)			{if (addRef) AddRef();}
    xs_RefPtr(const xs_RefPtr<T> &t) : mPtr(t.mPtr)		{AddRef();}
    ~xs_RefPtr()										{if (mPtr) mPtr->Release(); mPtr = 0;}
  
  	// ================================================================================================================
	//  Functions
	// ================================================================================================================
	void AddRef		()									{if (mPtr) mPtr->AddRef();}
	void Release	()									{if (mPtr) {if (mPtr->Release()==0) mPtr=0;}}
	void Swap		(xs_RefPtr &t)						{T* temp = mPtr; mPtr = t.mPtr; t.mPtr = temp;}	

    T*	 Ptr()		const								{return mPtr;}
	void Set		(const T *t, bool addRef=true)		{if (mPtr != t) 
															{T *tmp = mPtr;
															 mPtr = (T*)t;
															 if (mPtr&&addRef) mPtr->AddRef();
															 if (tmp) tmp->Release();}
														}

  	// ================================================================================================================
	//  Attach functions
	// ================================================================================================================
    void			Attach(T *t)						{Set(NULL); mPtr = t;}

  	// ================================================================================================================
	//  Detach functions
	// ================================================================================================================
    template<class U> void	Detach(U **t)				{*t=mPtr;	mPtr=0;}
    xs_RefPtr<T>			Detach()					{T* p=mPtr;	mPtr=0; return xs_RefPtr<T>(p);}
    void					Detach(xs_RefPtr<T> &to)	{if (to.mPtr)	to.mPtr->Release();  
														 to.mPtr			= mPtr;	// save ourselves incr/decr
														 mPtr			= 0;}	


	// ================================================================================================================
	//  operators
	// ================================================================================================================
	void operator =(const xs_RefPtr<T> &t)			{Set(t.mPtr);}
	void operator =(const T *t)						{Set(t);}


    T**Ref()										{if (mPtr) {mPtr->Release();} mPtr=0;return &mPtr;} 
    T& operator *()							const	{return *mPtr;}
    T* operator ->()						const	{return mPtr;}
    inline operator T*()					const	{return mPtr;}
	bool operator !()						const	{return mPtr == 0;}
    bool operator ==(const xs_RefPtr<T> &t)	const	{return mPtr==t.mPtr;}
    bool operator !=(const xs_RefPtr<T> &t)	const	{return mPtr!=t.mPtr;}
	bool operator ==(T *t)					const	{return mPtr==t;}
	bool operator !=(T *t)					const	{return mPtr!=t; }
};



// ================================================================================================================
//  ixs_Counted - the simplest counted interface
// ================================================================================================================
class ixs_Counted 
{
public:
	virtual uint32	AddRef()		= 0;
    virtual uint32	Release()		= 0;
    virtual uint32	CountRef()		= 0;
};


// ================================================================================================================
//  xs_RefCount - the simplest counted object base
// ================================================================================================================
class xs_RefCount : public ixs_Counted
{
public:
	int32	mRefCount;
	//int32	mDeleteObject; 

#if Debug_
	//copying is forbidden
    xs_RefCount(const xs_RefCount &)		{xs_Assert(0);}
    void operator =(const xs_RefCount &)	{xs_Assert(0);}
#endif

public:
 	// ================================================================================================================
	// reference counting virtual functions
	// ================================================================================================================
	virtual uint32	AddRef()		{return AddRef_();}
    virtual uint32	Release()		{return Release_();}
    virtual uint32	CountRef()		{return mRefCount;}

	// ================================================================================================================
	// Constructor/Destructor
	// ================================================================================================================
	xs_RefCount()						{mRefCount = 0;}// mDeleteObject = true;}
	virtual ~xs_RefCount()				{xs_Assert(mRefCount==0);}//(mDeleteObject==0));}


 	// ================================================================================================================
	// non virtual functions
	// ================================================================================================================
	uint32	AddRef_()					{return xs_SafeIncr(&mRefCount);}
    uint32	Release_()					{int32 rc = xs_SafeDecr(&mRefCount);	
										 xs_Assert(rc >= 0);
										 if (rc<=0) delete this;// && mDeleteObject)	delete this; 
										 return rc;}

 	// ================================================================================================================
	// use this define in the public of every derived class
	// ================================================================================================================
	#define XS_COUNTEDIMPL()\
		virtual uint32 AddRef()		{return xs_RefCount::AddRef();}\
		virtual uint32 Release()	{return xs_RefCount::Release();}\
		virtual uint32 CountRef()	{return xs_RefCount::CountRef();}
};


// =================================================================================================================
//  xs_RefCountNoVirtual  
// =================================================================================================================
class xs_RefCountNoVirtual
{
public:
	int32	mRefCount;

public:
 	// ================================================================================================================
	// reference counting virtual functions
	// ================================================================================================================
	uint32	AddRef()		{return xs_SafeIncr(&mRefCount);}
    uint32	Release()		{int32 r=xs_SafeDecr(&mRefCount); xs_Assert(r>=0); if (r<=0) delete this; return r;}
    uint32	CountRef()		{return mRefCount;}

	// ================================================================================================================
	// Constructor/Destructor
	// ================================================================================================================
	xs_RefCountNoVirtual()			{mRefCount = 0;}
	~xs_RefCountNoVirtual()			{xs_Assert(mRefCount==0);}

 	// ================================================================================================================
	// use this define in the public of every derived class
	// ================================================================================================================
	#define XS_COUNTEDIMPL_NOVIRT()\
		uint32 AddRef()		{return xs_RefCountNoVirtual::AddRef();}\
		uint32 Release()	{return xs_RefCountNoVirtual::Release();}\
		uint32 CountRef()	{return xs_RefCountNoVirtual::CountRef();}
};

// =================================================================================================================
// class xs_BinSearch  
// =================================================================================================================
template <class keyT, class T, class CCompare> class xs_BinSearch
{
public:
    enum {eBinSearchThresh=0}; //hard-coded to 8? test on other systems....

	// =================================================================================================================
	// Search  
	//		>=0 indicated index, 
	//		<0 indicates (-index+1) insertion position
	// =================================================================================================================
	static int32 Search (const keyT& key, const T* base, int32 count, void* customData)
		{
		if (count==0)	return -1;

		//binary search
		int32 i = 0;
		int32 n = count;

		//compare extremes
		int32 k			= CCompare::Compare(key, base[0], customData);
		if (k<=0)		return k==0 ? 0 : -1;

		k				= CCompare::Compare(key, base[n-1], customData);
		if (k>=0)		return k==0 ? n-1 : -(n+1);
		
		//compare interiors
		while (i<n)
			{
			//if it's only a few left,
			//just iterate
			if ((n-i)<eBinSearchThresh)		
				{
				for (int32 j=i; j<n; j++)
					{
					const T& guess	= base[j];
					int32 k			= CCompare::Compare(key, guess, customData);
					if (k==0)		return j;
					else if (k<0)	{n=j; break;}
					}

				//failed to find it
				return -(n+1);
				}

			//otherwise binary search
			int32 j			= (i+n-1)>>1;
			const T& guess	= base[j];
			int32 k			= CCompare::Compare(key, guess, customData);
			if (k==0)		return j;
			if (k<0)        n=j;
			else            i=j+1;
			}

		//failed to find it
		return -(n+1);
		}
};


// =================================================================================================================
// class xs_Sort  
// =================================================================================================================
template <class T, class CCompare=T> class xs_Sort
{
protected:
	static void QuickSortInternal (T *base, uint32 iStart, uint32 iEnd, void *customData);

public:
	static bool Sort (T* base, int32 count, void* customData)
	{
		if (!base)		{xs_Assert (0); return false;}
		if (count<=1)	return true;

		switch (count)
			{
			case 2:		
				if (CCompare::Compare (base+0, base+1, customData) > 0)
					{
					T temp	= base[0];
					base[0]	= base[1];
					base[1]	= temp;
					}
				break;
			default:
				QuickSortInternal (base, 0, count-1, customData);
				break;
			}

		return true;		
	}

};


// =================================================================================================================
// xs_Insert - requires preallocated memory for insertion  
// =================================================================================================================
finline bool xs_Insert (const void* dst, int32 count, int32 elemSize, 
						const void* src, int32 index, int32 insCount)
{
	//in range?
	if (uint32(index) > uint32(count))	return false;
	if (dst==NULL)						return false;

	//move the memory around
	uint8* pos			= ((uint8*)dst) + index*elemSize;
	xs_Memmove			(pos + insCount*elemSize, pos,  (count-index)*elemSize);
	if (src) xs_Memcpy	(pos, src, insCount*elemSize);
	return true;
}


// =================================================================================================================
// xs_Delete 
// =================================================================================================================
finline bool xs_Delete (const void* dst, int32 count, int32 elemSize, 
										 int32 index, int32 delCount)
{
	//in range?
	if (uint32(index) >= uint32(count))			return false;
	if (dst==NULL)								return false;
	if (uint32(index+delCount) > uint32(count))	delCount = count-index;

	//pull the overlap over
	uint8* pos		= ((uint8*)dst) + index*elemSize;
	xs_Memcpy		(pos, pos+delCount*elemSize, (count-(index+delCount))*elemSize);
	return true;
}




// =================================================================================================================
// QuickSortInternal - Implementation 
// =================================================================================================================
template<class T, class CCompare> void xs_Sort<T,CCompare>::QuickSortInternal 
	(T *base, uint32 iStart, uint32 iEnd, void* customData)
{
	#define xs_local_qsort_swap(a,b)		{temp=base[a]; base[a]=base[b]; base[b]=temp;}
	#define xs_local_qsort_comp(a,b)		CCompare::Compare (base+a, base+b, customData)
	
	
	int32 stack[64];						//stack - since qsort splits, this should be large enough forever?
	int32 si		= 0;					//stack index 
	int32 bubThresh	= 16;					//element minimum to quicksort
	int32 start		= iStart;				//start address
	int32 limit		= iEnd-iStart+1;		//past last element
	T temp;									//for swapping

	while (true)
		{
		if (limit-start < bubThresh) 
			{
			// ========================
			// bubble sort
			// ========================
			int32 j = start;
			int32 i = start + 1;
			while (i < limit)
				{
				while (xs_local_qsort_comp (j, j+1) < 0)
					{
					xs_local_qsort_swap	(j, j+1);
					if (j == start)		break;
					j--;
				    }

				j = i++;
				}

			//pop stuff off the stack (if there is anything)
			if (si != 0)
				{        
				limit	 = stack[--si];
				start	 = stack[--si];
				}
			else break;
			}
		else
			{
			// ========================
			// quick sort
			// ========================
			int32 mid	= ((limit-start)>>1) + start;
			xs_local_qsort_swap (mid, start);

			int32 i		= start  + 1;           
			int32 j		= limit - 1;
			
			//turn start to pivot
			if (xs_local_qsort_comp (i, j)	< 0)		xs_local_qsort_swap (i, j);
			if (xs_local_qsort_comp (start, j)	< 0)	xs_local_qsort_swap (start, j);
			if (xs_local_qsort_comp (i, start)	< 0)	xs_local_qsort_swap (i, start);

			//partition
			while (true)
				{
				do		i++;
				while	((i < limit) && (xs_local_qsort_comp (i, start) > 0));

				do		j--;
				while	((j >= start) && (xs_local_qsort_comp (j, start) < 0));

				//check bounds
				if (i >= limit)				i = limit;
				if (j <  start)				j = start;

				//are we done?
				if (i > j)					break;        
				xs_local_qsort_swap			(i, j);	//swap and continue
				}

			//place pivot correctly
			xs_local_qsort_swap (start, j);	

			//push to stack
			if (j-start > limit-i)
				{
				//left larger
				stack[si++] = start;
				stack[si++] = j;
				start		= i;
				}
			else
				{            
				//right larger
				stack[si++] = i;
				stack[si++] = limit;
				limit		= j; 
				}
		
			if (si >= 64)	{xs_Assert(0); return;} //yikes, overflow
			}
		}

	#undef xs_local_qsort_swap
	#undef xs_local_qsort_comp
}

// ====================================================================================================================
// ====================================================================================================================

#endif // _xs_PTR_H_
