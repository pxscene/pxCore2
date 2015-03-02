// rtRefPtr.h CopyRight 2005-2015 John Robinson
// A simple smartpointer class for objects utilizing
// COM-like (AddRef/Release) refcounting semantics

#ifndef RT_REFPTR_H
#define RT_REFPTR_H

#ifndef NULL
#define NULL 0
#endif

template <class T>
class rtRefPtr
{
public:
  rtRefPtr(): mRef(NULL) {}

  rtRefPtr(T* p): mRef(NULL)
  {
    mRef = p;
    if (mRef)
      mRef->AddRef();
  }
  
  rtRefPtr(const rtRefPtr<T>& r): mRef(NULL)
  {
    set(r.get());
  }
  
  ~rtRefPtr()
  {
    if (mRef)
    {
      mRef->Release();
      mRef = NULL;
    }
  }
  
  // Release any object pointed to by the smart ptr and returning a ptr to the internal
  // ptr reference.
  T** ref()
  {
    if (mRef)
    {
      mRef->Release();
      mRef = NULL;
    }
    return &mRef;
  }

  T* operator->() const
  {
    return mRef;
  }

  operator T* () const        {return mRef;}
  T* get() const              {return mRef;}

  T& operator*() const 
	{
		return *mRef;
	}

  bool operator! ()           {return mRef == NULL; }

  void set(T* p)                                      
  {
    if (mRef != p)
    {
      if (mRef)
      {
        mRef->Release();
        mRef = NULL;
      }
      mRef = p;
      if (mRef)
      {
        mRef->AddRef();
      }
    }
  }

  const rtRefPtr<T>& operator =(const rtRefPtr<T>& r)             
    {set(r.mRef); return *this; }
  const rtRefPtr<T>& operator =(const T* p)                       
    {set(const_cast<T*>(p)); return *this;}
    
  inline friend bool operator==(const rtRefPtr<T>& lhs,
                                const rtRefPtr<T>& rhs)
  {
    return lhs.mRef == rhs.mRef;
  }        
    
  inline friend bool operator==(const rtRefPtr<T>& lhs,
                                const T* rhs)
  {
    return lhs.mRef == rhs;
  }

  inline friend bool operator==(const T* lhs,
                                const rtRefPtr<T>& rhs)
  {
    return lhs == rhs.mRef;
  }

  inline friend bool operator!=(const rtRefPtr<T>& lhs,
                                const rtRefPtr<T>& rhs)
  {
    return lhs.mRef != rhs.mRef;
  }

  inline friend bool operator!=(const rtRefPtr<T>& lhs,
                                const T* rhs)
  {
    return lhs.mRef != rhs;
  }

  inline friend bool operator!=(const T* lhs,
                                const rtRefPtr<T>& rhs)
  {
    return lhs != rhs.mRef;
  }

private:
  T* mRef;
};

#endif
