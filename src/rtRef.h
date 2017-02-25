// rtCore CopyRight 2005-2015 John Robinson
// rtRef.h

// A simple smartpointer class for objects utilizing
// COM-like (AddRef/Release) refcounting semantics

#ifndef RT_REFT_H
#define RT_REFT_H

#include <stdlib.h>

//#define UNUSED_PARAM(x) (void (x))

template <class T>
class rtRef
{
public:
  rtRef():                  mRef(NULL) {}
  rtRef(const T* p):        mRef(NULL) {asn(p);         }
  rtRef(const rtRef<T>& r): mRef(NULL) {asn(r.getPtr());}

  virtual ~rtRef()                     {term();}

  void term()
  {
    if (mRef) 
    {
      mRef->Release();
      mRef = NULL;
    }
  }
  
  T* operator->()   const {return mRef;}
  operator T* ()    const {return mRef;}
  T* getPtr()       const {return mRef;}
  T* ptr()          const {return mRef;}

  T& operator*()    const {return *mRef;}
 
  bool operator! () const {return mRef == NULL; }  

  inline rtRef<T>& operator=(const T* p)                                  {asn(p);      return *this;}
  inline rtRef<T>& operator=(const rtRef<T>& r)                           {asn(r.mRef); return *this;}

  inline friend bool operator==(const T* lhs,const rtRef<T>& rhs)         {return lhs==rhs.mRef;}
  inline friend bool operator==(const rtRef<T>& lhs,const T* rhs)         {return lhs.mRef==rhs;}
  
  inline friend bool operator==(const rtRef<T>& lhs,const rtRef<T>& rhs)  {return lhs.mRef==rhs.mRef;}
  
protected:

  void asn(const T* p) 
  {
    if (mRef != p) 
    {
      if (mRef) 
      {
        mRef->Release();
        mRef = NULL;
      }
      mRef = const_cast<T*>(p);
      if (mRef) 
        mRef->AddRef();
    }
  }
  
  T* mRef;
};

#define rtRefT rtRef

#endif

