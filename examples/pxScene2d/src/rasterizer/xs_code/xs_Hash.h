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

// =================================================================================================================
// =================================================================================================================
//	xs_Hash.h
// =================================================================================================================
// =================================================================================================================
#ifndef _xs_Hash_H_
#define _xs_Hash_H_

#include "xs_Core.h"
#include "xs_Array.h"

//configuration
#ifndef XS_HASH_SMALLER
#define XS_HASH_SMALLER		1
#endif

#define XS_HASH_FREESEPARATE		0
#if XS_HASH_FREESEPARATE
	#define xs_HashBits				32
	#define xs_HashMask(a)			(a)					// Equal to uint32((1<<xs_HashBits)-1)
#else
	#define xs_HashBits				31
	#define xs_HashMask(a)			((a)&0x7fffffff)	// Equal to uint32((1<<xs_HashBits)-1)
#endif

// =================================================================================================================
// xs_HashNode
// =================================================================================================================
class xs_HashNode
{
public:
#if XS_HASH_FREESEPARATE==0
	int32			mFree:1;
#endif
	int32			mHashCode:xs_HashBits;
	xs_HashNode*	mNext;
	void*			mValue;
};


// =================================================================================================================
// xs_HashBase
// =================================================================================================================
template <class CAlloc=xs_Alloc> class xs_HashBase
{
public:
	// =================================================================================================================
	// struct
	// =================================================================================================================	
	typedef xs_HashBase<CAlloc>    CBase;
	
#if XS_HASH_FREESEPARATE
	#define _xs_FreeInd(a)		(a)
	#define _xs_FreeNode(n)		CBase::Index(n)
#else
	#define _xs_FreeInd(a)		CBase::Node(a)
	#define _xs_FreeNode(n)		(n)
#endif

	// =================================================================================================================
	// Data
	// =================================================================================================================
#if XS_HASH_SMALLER==0
	typedef xs_HashNode* STableHeader;
#else
	struct STableHeader
	{
#endif
		int32			mCount;
		int32			mHashSize;
		xs_HashNode*	mFirstFree;
		int32			mDummy; //pad it out to 16 bytes
#if XS_HASH_SMALLER
	};
#endif

	STableHeader*		mTable;
	int32				mExternal:1;
	int32				mForcePow2Size:1;
	int32				mElemSize:16;
	xs_HashNode**		mTableHash;

	// =================================================================================================================
	// Constructor/Destructor
	// =================================================================================================================
	xs_HashBase()	{Alloc(0,0,sizeof(uint32));}
	~xs_HashBase()	{Free();}

	static int32 ElemMemory(int32 elemSize)	{return sizeof (xs_HashNode)+elemSize-sizeof(uint32);}
	static int32 RequiredMemory(int32 count, int32 elemSize)	{return 0;}

	finline uint8*	FreePtr()	{return (uint8*)(mTableHash+HashSize());}

	// =================================================================================================================
	// Node functions
	// =================================================================================================================
	finline bool			Valid		(int32 index)		{return uint32(index)<uint32(HashSize());}
	finline bool			InRange		(int32 index)		{return uint32(index)<uint32(HashSize());}
	finline xs_HashNode*	Node		(int32 index)		{xs_Assert(Valid(index));	return (xs_HashNode*)((char*)(mTable+1)+index*SSize());}
	finline void*&			Value		(xs_HashNode* node)	{xs_Assert(node!=NULL);		return node->mValue;}
	finline void*&			Value		(int32 index)		{xs_Assert(Valid(index));	return Node(index)->mValue;}
	finline xs_HashNode*	Table		(uint32 code)		{return mTableHash[code];}
	finline void			SetTable	(uint32 code,  
										 xs_HashNode* node)		{xs_Assert(Valid(code));	mTableHash[code]=node;}

#if XS_HASH_FREESEPARATE==0
	finline bool	IsFree		(xs_HashNode* node)		{return node->mFree!=0;}
	finline void	SetFree		(xs_HashNode* node)		{node->mFree=1;}
	finline void	ClearFree	(xs_HashNode* node)		{node->mFree=0;}
	finline void	Init		(xs_HashNode* node)		{SetFree(node); node->mNext=0;}
#else
	finline bool	IsFree		(int32 i)				{return xs_BitArrGet(FreePtr(), i);}
	finline void	SetFree		(int32 i)				{xs_BitArrSet(FreePtr(), i, 1);}
	finline void	ClearFree	(int32 i)				{xs_BitArrSet(FreePtr(), i, 0);}
	finline void	Init		(int32 i)				{SetFree(i); Node(i)->mNext=0;}
#endif
	finline int32	Index		(xs_HashNode* node)		
		{
		int32 s = SSize();
		int32 i = (char*)node-(char*)Node(0);
		switch (s)
			{
			case 8:		return i>>3;
			case 16:	return i>>4;
			case 32:	return i>>5;
			case 64:	return i>>6;
			default:	return i/s;
			}
		}

	// =================================================================================================================
	// Node "search" functions
	// =================================================================================================================
	finline xs_HashNode* Find		(uint32 hashCode)	{return Table(HashCode (hashCode));}
	finline xs_HashNode* Next		(xs_HashNode* node)	{xs_Assert(node); return node->mNext;}


	// =================================================================================================================
	// internal functions
	// =================================================================================================================
	finline int32 AllocExtra()					{return sizeof(STableHeader);}
	finline int32 SSize()						{return mElemSize;}
	finline int32 AllocSize()					{return mElemSize+sizeof(uint32);}
	finline uint32 HashCode(uint32 code)		{int32 hs = HashSize(); return hs ? xs_Mod(xs_HashMask(code), hs) : 0;}


	// =================================================================================================================
	// Alloc/Free
	// =================================================================================================================
	bool Alloc(void* mem, int32 count, int32 elemSize);
	bool Free();

	// =================================================================================================================
	// Member Functions
	// =================================================================================================================
#if XS_HASH_SMALLER
	finline xs_HashNode**	TableHash()						{return mTableHash;}
	finline int32			Count()							{return mTable->mCount;}
	finline int32			Space()							{return mTable->mHashSize;}
	finline int32			HashSize()						{return mTable->mHashSize;}
	finline xs_HashNode*	FirstFree()						{return mTable->mFirstFree;}
	finline void			SetCount(int32 c)				{mTable->mCount			= c;}
	finline void			SetHashSize(int32 h)			{mTable->mHashSize		= h; mTableHash = (xs_HashNode**)((char*)(mTable+1) + h*(SSize()));}
	finline void			SetFirstFree(xs_HashNode* n)	{mTable->mFirstFree		= n;}
#else
	finline xs_HashNode**	TableHash()						{return mTableHash;}
	finline int32			Count()							{return mCount;}
	finline int32			Space()							{return mHashSize;}
	finline int32			HashSize()						{return mHashSize;}
	finline xs_HashNode*	FirstFree()						{return mFirstFree;}
	finline void			SetCount(int32 c)				{mCount			= c;}
	finline void			SetHashSize(int32 h)			{mHashSize		= h; mTableHash = (xs_HashNode**)((char*)(mTable+1) + h*(SSize()));}
	finline void			SetFirstFree(xs_HashNode* n)	{mFirstFree		= n;}
#endif
	void Reset();

	// =================================================================================================================
	// Hash Functions
	// =================================================================================================================
	xs_HashNode* AddNode (uint32 hashCode, void* value, bool setValue=true);
	int32  RemoveNode (xs_HashNode* node);
	int32  ReHash (void* mem, int32 newSize, bool fullRehash=true);
	int32  Copy (xs_HashBase* src);
};


// =================================================================================================================
// Hash Pair
// =================================================================================================================
template <class K, class V> class xs_HashPair
{
public:
	K	first;
	V	second;
};

// =================================================================================================================
// Hash Tempate
// =================================================================================================================
template <class K, class V, class CCompare, class A=xs_Alloc> class xs_Hash : protected xs_HashBase<A>
{
public:
    typedef xs_HashBase<A>                  CBase;
    
	// =================================================================================================================
	// Constructor/Destructor
	// =================================================================================================================
	xs_Hash()	{CBase::Alloc(0,0,sizeof(xs_HashPair<K,V>));}
	~xs_Hash();

	// =================================================================================================================
	// BasicFunctions
	// =================================================================================================================
	V*				Get			(const K& i, bool create=false);
	bool			Remove		(const K& i);
	inline V*		Add			(const K& i)						{return Add_(i,CCompare::HashCode(i));}								
	inline void		SetSpace	(int32 space)						{CBase::ReHash(0, space);}
	inline int32	Space		()									{return CBase::HashSize();}
	inline int32	Count		()									{return CBase::Count();}
	inline bool		Copy		(xs_Hash<K,V,CCompare,A>* src)		{return CBase::Copy(src);}

	inline V&		operator [] (const K& i)						{V* v = Get(i, true); xs_Assert(v); return *v;}


	//internal
	V* Add_ (const K& i, int32 h);
};


// =================================================================================================================
// xs_ComparePtr - example Compare Class for Hash Template
// =================================================================================================================
class xs_ComparePtr
{
public:
	static inline int32 HashCode(void* p)						{int32 i=(int32)p; return ((i)>>2) | (i<<(32-2));}
	static inline int32 Equal(void* i, void* j)					{return i==j;}
	static inline int32 Compare(const void* i, const void* j)	{return int32((char*)i-(char*)j);}
};

// =================================================================================================================
// xs_CompareInt - example Compare Class for Hash Template
// =================================================================================================================
class xs_CompareInt
{
public:
	static inline int32 HashCode(int32 i)						{return i;}
	static inline int32 Equal(int32 i, int32 j)					{return i==j;}
	static inline int32 Compare(const int32 i, const int32 j)	{return i-j;}
};


















// =================================================================================================================
// 
// =================================================================================================================
// =================================================================================================================
// Implementation
// =================================================================================================================
// =================================================================================================================
// 
// =================================================================================================================
// =================================================================================================================
// Destructor
// =================================================================================================================
template <class K, class V, class C, class A> xs_Hash<K,V,C,A>::~xs_Hash()
{
	for (int32 i=0; i<CBase::HashSize(); i++)
	{
		xs_HashNode* node = CBase::Node(i);
		if (CBase::IsFree(_xs_FreeInd(i))==false)
		{
			xs_HashPair<K,V>& pair = (xs_HashPair<K,V>&)CBase::Value(node);
			pair.first.~K();
			pair.second.~V();
		}
	}
	CBase::Free();
}

// =================================================================================================================
// Add_
// =================================================================================================================
template <class K, class V, class C, class A> inline V* xs_Hash<K,V,C,A>::Add_(const K& i, int32 h)
{
	xs_HashNode* node	= CBase::AddNode (h, 0, false);
	xs_HashPair<K,V>& pair = (xs_HashPair<K,V>&)CBase::Value(node);
	new (&pair.first) K(i);
	new (&pair.second) V();
	return &pair.second;
}

// =================================================================================================================
// Get
// =================================================================================================================
template <class K, class V, class C, class A> inline V* xs_Hash<K,V,C,A>::Get(const K& i, bool create)
{
	int32 h = C::HashCode(i);
	xs_HashNode* node = CBase::Find(h);
	while (node)
		{
		xs_HashPair<K,V>& pair = (xs_HashPair<K,V>&)CBase::Value(node);
		if (C::Equal(pair.first, i))
			return &pair.second;
		node = CBase::Next(node);
		}

	return create ? Add_(i, h) : NULL;
}


// =================================================================================================================
// Remove
// =================================================================================================================
template <class K, class V, class C, class A> bool xs_Hash<K,V,C,A>::Remove(const K& i)
{
	int32 h = C::HashCode(i);
	xs_HashNode* node = CBase::Find(h);

	while (node)
		{
		xs_HashPair<K,V>& pair = (xs_HashPair<K,V>&)CBase::Value(node);
		if (C::Equal(pair.first, i))
			{
			pair.first.~K();;
			pair.second.~V();
			CBase::RemoveNode(node);
			return true;
			}
		node = CBase::Next(node);
		}

	//didn't find it
	return false;
}






// =================================================================================================================
// 
// =================================================================================================================
// =================================================================================================================
// xs_HashBase<A> Implementation
// =================================================================================================================
// =================================================================================================================
// 
// =================================================================================================================

// =================================================================================================================
// Alloc
// =================================================================================================================
template<class A> bool xs_HashBase<A>::Alloc (void* mem, int32 size, int32 elemSize)
{
	mForcePow2Size		= 0;
	size				= xs_Max (size, 0);
	mElemSize			= xs_Max (elemSize, (int32)sizeof(uint32));
	mElemSize			= ElemMemory(mElemSize);

	//allocate Node
	extern int32 gxs_EmptyTable[];
	mTable				= (STableHeader*)&gxs_EmptyTable;
	SetHashSize			(0);
	xs_Verify (ReHash (mem, size, false)==0);
	if (mTable==NULL)	{xs_Assert(0);	return false;}

	//setup Node and table
	Reset();

	//success
	return true;

}

// =================================================================================================================
// Free
// =================================================================================================================
template<class A> bool xs_HashBase<A>::Free ()
{
	extern int32 gxs_EmptyTable[];
	if (mExternal==false && mTable && mTable!=(STableHeader*)&gxs_EmptyTable)	
		A::Free (mTable);	

	mTable		= NULL;
	mExternal	= false;
	return true;
}

// =================================================================================================================
// Reset
// =================================================================================================================
template<class A> void xs_HashBase<A>::Reset()
{
	//reset count
	SetCount (0);
	SetFirstFree (0);

	//setup Node and table
	for (int32 i=0; i<HashSize(); i++)	
	{
		SetTable(i, 0);
		Init(_xs_FreeInd(i)); //set it to free
	}
}


// =================================================================================================================
// AddNode
// =================================================================================================================
template<class A> xs_HashNode* xs_HashBase<A>::AddNode (uint32 hashCode, void* value, bool setValue)
{
	//hash it
	uint32 code		= HashCode(hashCode);

	//allocate the node
	xs_HashNode* newnode = FirstFree();
	if (newnode)
	{
		//pull from "free" list
		//printf ("from free list\n");
		SetFirstFree (newnode->mNext);	
		ClearFree(_xs_FreeNode(newnode));
	}

	//no more room?
	else
	{
		if (Count()>=HashSize())
			{
			//can't allocate more room this way...
			if (mExternal!=false)
			{
				xs_Assert(0); 
				return NULL;
			}

			//try to realloc more room
			int32 newsize	= HashSize()*2;
			if (newsize==0)	newsize++;
			if (ReHash (NULL, newsize)!=0)
				return NULL;

			code	= HashCode(hashCode);
			}

		newnode	= Node(Count()); 
		ClearFree(_xs_FreeInd(Count()));
	} 

	//set the node
	if (setValue)					
		Value(newnode)		= value;
	newnode->mHashCode		= hashCode;
	newnode->mNext			= Table(code);
	SetTable(code, newnode);

	//increment the count 
	//and return
	SetCount (Count()+1);

	//done
	return newnode;
}


// =================================================================================================================
// RemoveNode
// =================================================================================================================
template<class A> int32 xs_HashBase<A>::RemoveNode (xs_HashNode *node)
{
	if (node==NULL)	{xs_Assert(0); return -1;}

	//find "start" node (in order to find previous)
	uint32 code			= HashCode(node->mHashCode);
	xs_HashNode* cur	= Table(code);
	if (cur==NULL)	{xs_Assert(0); return -2;} //not a hash code in the table

	//removing first node
	if (node==cur)
	{
		SetTable(code, node->mNext);

		//re-assign first free
		node->mNext		= FirstFree();
		SetFirstFree	(node);
		SetFree			(_xs_FreeNode(node));

		//done
		SetCount(Count()-1);
		return 0;
	}

	//loop through nodes
	xs_HashNode* next = Next(cur);
	while (next && next!=node)
	{
		//iterate
		cur		= next;
		next	= next->mNext;
	}

	//did we find it?
	if (next==node)
	{
		//"skip" the node we're removing
		xs_Assert (cur && next);
		cur->mNext		=next->mNext;

		//re-assign first free
		next->mNext		= FirstFree();
		SetFirstFree	(next);
		SetFree			(_xs_FreeNode(next));

		//done
		SetCount(Count()-1);
		return 0;
	}


	//error - couldn't find it
	xs_Assert(0);
	return -3;
}


// =================================================================================================================
// ReHash
// =================================================================================================================
template<class A> int32 xs_HashBase<A>::ReHash (void* mem, int32 newSize, bool fullRehash)
{
	if (mForcePow2Size)
		newSize = (int32)xs_RoundUpPow2(uint32(newSize));

	//can't shrink when we rehash
	if (newSize<HashSize())		{xs_Assert(0); return -1;}	

	//reallocate
	extern int32 gxs_EmptyTable[];
	int32 ram		= 0;
	bool newTable	= (mTable==(STableHeader*)&gxs_EmptyTable);
	if (mem==NULL)
	{
		mExternal = false;
		xs_Assert (mem==NULL);
		if (newSize != HashSize())
		{
			//printf ("resize %ld %ld\n", newSize, HashSize());
			STableHeader* ntable;
			if (newSize)
			{
//				int32 hsize		= HashSize();
			#if XS_HASH_FREESEPARATE
				int32 freeSize	= xs_BitArrPad(newSize);
			#else
				int32 freeSize	= 0;
			#endif
				int32 allocSize = AllocSize()*newSize + AllocExtra() + freeSize;
				ntable = (newTable==false)  ? 
						(STableHeader*)A::Realloc (mTable, allocSize) :
						(STableHeader*)A::Alloc (allocSize);

			#if XS_HASH_FREESEPARATE
				int32 oldPos = AllocSize()*hsize + AllocExtra();
				xs_Memmove ((char*)ntable + allocSize - freeSize,
							(char*)ntable + oldPos, xs_BitArrPad(hsize));
			#endif
				ram = allocSize;
			}
			else
			{
				if (newTable==false)
					A::Free(mTable);
				ntable = (STableHeader*)&gxs_EmptyTable;
			}

			if (ntable==NULL)	{xs_Assert(0); return -108;}
			mTable				= ntable;
		}
	}
	else 
	{
		mTable = (STableHeader*)mem;
		mExternal = true;
	}

	if (newTable)
	{
		SetCount (0);
		SetFirstFree (0);
		SetHashSize (0);
	}

	//failure
	if (HashSize() && mTable==NULL)
		{xs_Assert(0); return -1;}	

	//set new size
	int32 i, oldHashSize	= HashSize();
	SetHashSize(newSize);

	//rehash the entries
	if (fullRehash)
	{
		SetFirstFree (0);

		//reset table
		int32 tableCount = xs_Min (HashSize(), oldHashSize);
		for (i=0; i<tableCount; i++)
			SetTable(i, 0);

		for (/*nothing*/; i<HashSize(); i++)
		{
			SetTable(i, 0);
			Init(_xs_FreeInd(i));
		}
			
		//re-add nodes
		xs_HashNode* lastFree = 0;
		for (i=0; i<oldHashSize; i++)
		{
			xs_HashNode* node = Node(i);
			if (IsFree(_xs_FreeInd(i)))
			{
				if (lastFree==0)
					SetFirstFree (node);
				else lastFree->mNext = node;
				lastFree = node;
			}
			else
			{
				//re-insert into the table
				int32 code		= HashCode (node->mHashCode);
				node->mNext		= Table(code);
				SetTable(code, node);
			}
		}
	}

	//done
	return 0;
}

// =================================================================================================================
// Copy
// =================================================================================================================
template<class A> int32 xs_HashBase<A>::Copy (xs_HashBase* src)
{
	mForcePow2Size = src->mForcePow2Size;
	if (src->mElemSize != mElemSize)
	{
		//bad param
		xs_Assert(0);
		return -50;
	}

	//reallocate
	ReHash(mTable, src->Space(), false);

	//add old hash
	SetFirstFree(0);
	for (int32 i=0; i<Space(); i++)
	{
		//copy node
		xs_HashNode* srcnode	= src->Node(i);
		xs_HashNode* dstnode	= Node(i);
		*dstnode				= *srcnode;

		xs_HashNode* fnode	= FirstFree();
		if (IsFree(dstnode))
		{
			if (fnode==0)
				SetFirstFree(dstnode);
			else fnode->mNext = dstnode;
		}

		//copy table codes
		SetTable(i, src->Table(i));
	}

	//data
	SetCount(src->Count());

	//done
	return 0;
}


// =================================================================================================================
// 
// =================================================================================================================
// =================================================================================================================
// Experimental xs_HashLight
// =================================================================================================================
// =================================================================================================================
// 
// =================================================================================================================
/*
template <class K, class V, class CCompare, class Alloc=xs_Alloc> class xs_HashLight
{
public:
	class xs_HashPair
	{
	public:
		K	first;
		V	second;
	};


	class CArrayCompare
	{
	public:
		static finline int32 Compare (const K& a, const xs_Array<xs_HashPair>& b, void* customData)
		{
			if (b.Count()==0)			return -1;
			if (a<b[0].first)			return -1;
			if (a>b.LastValue().first)	return 1;
			return 0;
		}
	};

	xs_MemBlock< xs_Array<xs_HashPair> >	mData;
	int32									mLast;

	xs_HashLight()
	{
		mLast = -1;
	}

	void resize (int size)
	{
		//mmgr.ReHash(0, size, true);
	}

	void set_empty_key(const K& i)
	{
	}
	
	enum
	{
		eThresh=128
	};

	static inline int32 Compare (const K& k1, const xs_HashPair& k2, void*)
	{
		return CCompare::Compare (k2.first, k1);
	}

	inline V& operator [] (const K& i)	
	{
		if (mData.Count()==0) xs_Verify(mData.SetCount(1));

		int32 ind = -1;
		if (mData.InRange(mLast))
			{
			if (CArrayCompare::Compare(i, mData[mLast], NULL)==0)
				ind = mLast;
			}
		if (ind==-1)
			{
			ind = xs_BinSearch<K, xs_Array<xs_HashPair>, CArrayCompare>::Search (i, mData.Base(), mData.Count(), 0);
			if (ind<0)	
				ind = -ind-1;
			if (mData.InRange(ind)==0)
				ind=mData.LastIndex();
			if (mData.InRange(ind) && mData[ind].Count()>eThresh)
				{
				xs_Array<xs_HashPair>* arr = mData.InsertCount(ind+1);
				int32 loc	= mData[ind].Count()>>1;
				int32 count = mData[ind].Count()-loc;
				if (count)
					{
					arr->Copy (mData[ind].Ptr(loc), count);
					mData[ind].SetSpace(loc);
					if (Compare(i, mData[ind+1][0], NULL)>=0)
						ind++;
					}
				}
			}

		mLast = ind;
		int32 result = xs_BinSearch<K, xs_HashPair, xs_HashLight<K,V,CCompare> >::Search (i, mData[ind].Base(), mData[ind].Count(), 0);
		if (result>=0)	return mData[ind][result].second;

		result = -result-1;
		
		xs_HashPair* pair = mData[ind].InsertCount(result);

		new (&pair->first) K(i);
		new (&pair->second) V();
		return pair->second;
	}

	inline xs_HashPair* find(const K& i)
	{
		return 0;
	}

	inline const xs_HashPair* end()
	{
		return 0;
	}

	inline void erase(const K& i)
	{
	}
};
*/
#endif // for entire file
