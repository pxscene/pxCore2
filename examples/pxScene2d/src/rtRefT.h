// rtCore CopyRight 2005-2015 John Robinson
// rtRefT.h

// A simple smartpointer class for objects utilizing
// COM-like (AddRef/Release) refcounting semantics

#ifndef RT_REFT_H
#define RT_REFT_H

#include <stdlib.h>

template <class T>
class rtRefT
{
public:
  rtRefT():                   mRef(NULL) {}
  rtRefT(const T* p):         mRef(NULL) {asn(p);         }
  rtRefT(const rtRefT<T>& r): mRef(NULL) {asn(r.getPtr());}

  virtual ~rtRefT()                      {term();}

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

  T& operator*()    const {return *mRef;}
 
  bool operator! () const {return mRef == NULL; }  

  inline rtRefT<T>& operator=(const T* p)         {asn(p);      return *this;}
  inline rtRefT<T>& operator=(const rtRefT<T>& r) {asn(r.mRef); return *this;}

  inline friend bool operator==(const T* lhs,const rtRefT<T>& rhs)
  {
    return lhs==rhs.mRef;
  }
  
  inline friend bool operator==(const rtRefT<T>& lhs,const T* rhs)
  {
    return lhs.mRef==rhs;
  }
  
  inline friend bool operator==(const rtRefT<T>& lhs,const rtRefT<T>& rhs)
  {
    return lhs.mRef==rhs.mRef;
  }
  
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

#endif

