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

// rtRef.h

// A simple smartpointer class for objects utilizing
// COM-like (AddRef/Release) refcounting semantics
// TODO restore IUnknown compatibility on Windows

#ifndef RT_REFT_H
#define RT_REFT_H

#include <stdlib.h>

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

