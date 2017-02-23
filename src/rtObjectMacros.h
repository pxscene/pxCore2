// rtCore CopyRight 2005-2015 John Robinson
// rtObjectMacros.h

#ifndef RT_OBJECT_MACROS_H
#define RT_OBJECT_MACROS_H

#define __UNUSED(x)  ((x)=(x))

class rtObject;
class rtValue;
typedef rtError (rtObject::*rtMethodThunk)(int numArgs, const rtValue* args, rtValue& result);
typedef rtError (rtObject::*rtGetPropertyThunk)(rtValue& result) const;
typedef rtError (rtObject::*rtSetPropertyThunk)(const rtValue& value);

struct rtMethodEntry
{
    void init(const char* methodName, const char* argTypes, char returnType, 
	      rtMethodThunk thunk, rtMethodEntry* next)
    {
        mArgTypes = argTypes;
        mMethodName = methodName;
        mNext = next;
        mReturnType = returnType;
        mThunk = thunk;
    }

    rtMethodEntry* mNext;
    const char* mMethodName;
    rtMethodThunk mThunk;
    const char* mArgTypes;
    char mReturnType;
};


typedef struct rtPropertyEntry
{
    const char* mPropertyName;
    rtGetPropertyThunk mGetThunk;
    rtSetPropertyThunk mSetThunk;
    char mPropType;
    rtPropertyEntry* mNext;
} rtPropertyEntry;

typedef rtMethodEntry* (*fnhead)(rtMethodEntry* p);
typedef rtPropertyEntry* (*fnPropHead)(rtPropertyEntry* p);

typedef struct rtMethodMap
{
  const char* className;
  fnhead firstMethod;
  fnPropHead firstProperty;
  
  //unsigned long numEntries;
  rtMethodMap* parentsMap;
  
  rtMethodEntry* getFirstMethod()
  {
    return firstMethod(NULL);
  }
  rtPropertyEntry* getFirstProperty()
  {
    return firstProperty(NULL);
  }
} rtMethodMap;

#if 0
#define rtBeginProperties(class)\
rtPropertyEntry rtObject::rtPropertyEntries[] = {

#define rtEndProperties(class) \
{L"", NULL, NULL, '\0'}};\
int class::rtPropertyCount = sizeof(class::rtPropertyEntries)/sizeof(rtPropertyEntry);

#define rtReadWriteProperty(name, getter, setter, type) \
{name, getter, setter, RT_##type##Type},

#define rtReadOnlyProperty(name, getter, type) \
{name, getter, NULL, RT_##type##Type},

#define rtWriteOnlyProperty(name, setter, type) \
{name, NULL, setter, RT_##type##Type},
#endif


#define rtDeclareObjectPtr(CLASSNAME__, PTR__)              \
    static rtMethodEntry* head(rtMethodEntry* p = NULL)     \
    {                                                       \
        static rtMethodEntry* head = NULL;                  \
                                                            \
        rtMethodEntry* oldHead = head;                      \
        if (p)                                              \
        {                                                   \
            head = p;                                       \
        }                                                   \
        return oldHead;                                     \
    }                                                       \
    static rtPropertyEntry* headProperty(rtPropertyEntry* p = NULL)     \
    {                                                       \
        static rtPropertyEntry* head = NULL;                  \
                                                            \
        rtPropertyEntry* oldHead = head;                      \
        if (p)                                              \
        {                                                   \
            head = p;                                       \
        }                                                   \
        return oldHead;                                     \
    }   \
    static rtMethodMap map;                                 \
    virtual rtMethodMap* getMap() const               \
    {                                                       \
        return &map;                                  \
    }                                                       


#define rtDeclareObject(CLASSNAME__, PARENT__)                           \
  rtDeclareObjectPtr(CLASSNAME__, PARENT__::head());                    \
	typedef CLASSNAME__ PARENTTYPE__;


#define rtDeclareObjectBase() \
	rtDeclareObjectPtr(rtObject, NULL) \
	typedef rtObject PARENTTYPE__

#define rtDefineObjectPtr(CLASSNAME__, PTR__)                           \
    rtMethodMap CLASSNAME__::map = {"" #CLASSNAME__ "", CLASSNAME__::head, CLASSNAME__::headProperty, PTR__};

#define rtDefineObject(CLASSNAME__, PARENT__)                           \
    rtDefineObjectPtr(CLASSNAME__, &PARENT__::map)

#define rtGA(i) ((i<numArgs)?args[i]:rtValue())

///////////////////////////////////////////////////////////////////////////////
// Internally used thunk macros
// thunks for functions not returning a value
#define rtThunkNoArgAndNoReturn(method) \
    rtError method##_thunk(int /*numArgs*/, const rtValue* /*args*/, rtValue& /*r*/){ return method(); }

#define rtThunk1ArgAndNoReturn(method, arg1type) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue&){return method(rtGA(0).convert<arg1type>()); }

#define rtThunk2ArgAndNoReturn(method, arg1type, arg2type) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue&){return method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>()); }

#define rtThunk3ArgAndNoReturn(method, arg1type, arg2type, arg3type) \
    rtError method##_thunk(int numArgs, const rtValue* args, rtValue&){return method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), args[2].convert<arg3type>()); }

#define rtThunk4ArgAndNoReturn(method, arg1type, arg2type, arg3type, arg4type) \
    rtError method##_thunk(int numArgs, const rtValue* args, rtValue&){return method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rtGA(3).convert<arg4type>()); }

#define rtThunk5ArgAndNoReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue&){return method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rtGA(3).convert<arg4type>(), rtGA(4).convert<arg5type>()); }

#define rtThunk6ArgAndNoReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue&){return method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rtGA(3).convert<arg4type>(), rtGA(4).convert<arg5type>(), rtGA(5).convert<arg6type>()); }

#define rtThunk7ArgAndNoReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type) \
    rtError method##_thunk(int numArgs, const rtValue* args, rtValue&){return method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rtGA(3).convert<arg4type>(), rtGA(4).convert<arg5type>(), rtGA(5).convert<arg6type>(), rtGA(6).convert<arg7type>()); }

#define rtThunk8ArgAndNoReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, arg8type) \
    rtError method##_thunk(int numArgs, const rtValue* args, rtValue&){return method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rtGA(3).convert<arg4type>(), rtGA(4).convert<arg5type>(), rtGA(5).convert<arg6type>(), rtGA(6).convert<arg7type>(), rtGA(7).convert<arg8type>()); }

#define rtThunk9ArgAndNoReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, arg8type, arg9type) \
    rtError method##_thunk(int numArgs, const rtValue* args, rtValue&){return method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rtGA(3).convert<arg4type>(), rtGA(4).convert<arg5type>(), rtGA(5).convert<arg6type>(), rtGA(6).convert<arg7type>(), rtGA(7).convert<arg8type>(), rtGA(8).convert<arg9type>()); }


///////////////////////////////////////////////////////////////////////////////
// Internally used thunk macros
// thunks for functions returning a value
#define rtThunkNoArgAndReturn(method, returntype) \
  rtError method##_thunk(int /*numArgs*/, const rtValue* /*args*/, rtValue& r){ returntype rv; rtError e =  method(rv); if (&r) r.assign<returntype>(rv); return e;}

#define rtThunk1ArgAndReturn(method, arg1type, returntype) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue& r){returntype rv; rtError e =  method(rtGA(0).convert<arg1type>(), rv); if (&r) r.assign<returntype>(rv); return e;}

#define rtThunk2ArgAndReturn(method, arg1type, arg2type, returntype) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue& r){returntype rv; rtError e =  method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rv); if (&r) r.assign<returntype>(rv); return e;}

#define rtThunk3ArgAndReturn(method, arg1type, arg2type, arg3type, returntype) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue& r){returntype rv; rtError e =  method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rv); r.assign<returntype>(rv); return e;}

#define rtThunk4ArgAndReturn(method, arg1type, arg2type, arg3type, arg4type, returntype) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue& r){returntype rv; rtError e =  method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rtGA(3).convert<arg4type>(), rv); if (&r) r.assign<returntype>(rv); return e;}

#define rtThunk5ArgAndReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, returntype) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue& r){returntype rv; rtError e =  method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rtGA(3).convert<arg4type>(), rtGA(4).convert<arg5type>(), rv); if (&r) r.assign<returntype>(rv); return e;}

#define rtThunk6ArgAndReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, returntype) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue& r){returntype rv; rtError e =  method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rtGA(3).convert<arg4type>(), rtGA(4).convert<arg5type>(), rtGA(5).convert<arg6type>(), rv); r.assign<returntype>(rv); return e;}

#define rtThunk7ArgAndReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, returntype) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue& r){returntype rv; rtError e =  method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rtGA(3).convert<arg4type>(), rtGA(4).convert<arg5type>(), rtGA(5).convert<arg6type>(), rtGA(6).convert<arg7type>(), rv); if (&r) r.assign<returntype>(rv); return e;}

#define rtThunk8ArgAndReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, arg8type, returntype) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue& r){returntype rv; rtError e =  method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rtGA(3).convert<arg4type>(), rgGA(4).convert<arg5type>(), rtGA(5).convert<arg6type>(), rtGA(6).convert<arg7type>(), rtGA(7).convert<arg8type>(), rv); if (&r) r.assign<returntype>(rv); return e;}

#define rtThunk9ArgAndReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, arg8type, arg9type, returntype) \
  rtError method##_thunk(int numArgs, const rtValue* args, rtValue& r){returntype rv; rtError e =  method(rtGA(0).convert<arg1type>(), rtGA(1).convert<arg2type>(), rtGA(2).convert<arg3type>(), rtGA(3).convert<arg4type>(), rtGA(4).convert<arg5type>(), rtGA(5).convert<arg6type>(), rtGA(6).convert<arg7type>(), rtGA(7).convert<arg8type>(), rtGA(8).convert<arg9type>(), rv); if (&r) r.assign<returntype>(rv); return e;}

#define rtThunkProperty(getterMethod, setterMethod, propType) \
  rtError setterMethod##_PropSetterThunk(const rtValue& v) { return setterMethod(v.convert<propType>()); } \
    rtError getterMethod##_PropGetterThunk(rtValue& v) const { propType pv;  rtError e = getterMethod(pv); v.assign<propType>(pv); return e;}

#define rtThunkReadOnlyProperty(getterMethod, propType) \
    rtError getterMethod##_PropGetterThunk(rtValue& v) const { propType pv;  rtError e = getterMethod(pv); v.assign<propType>(pv); return e;}

#define rtThunkConstantProperty(getterMethod, k, propType)		\
    rtError getterMethod##_PropGetterThunk(rtValue& v) const { v.assign<propType>(k); return RT_OK;}

#define rtProperty(name, getMethod, setMethod, propType)\
    rtThunkProperty(getMethod, setMethod, propType); \
    class name##PropEntry \
    { \
    public: \
        name##PropEntry() \
        {\
            rtPropertyEntry* tail = headProperty(&entry); \
            entry.mPropertyName = "" #name ""; \
            entry.mPropType = RT_##propType##Type; \
            entry.mSetThunk = (rtSetPropertyThunk)&PARENTTYPE__::setMethod##_PropSetterThunk; \
            entry.mGetThunk = (rtGetPropertyThunk)&PARENTTYPE__::getMethod##_PropGetterThunk; \
            entry.mNext = tail; \
        }\
        rtPropertyEntry entry; \
    };\
    static name##PropEntry name##PropEntryInstance

#define rtReadOnlyProperty(name, getMethod, propType)\
    rtThunkReadOnlyProperty(getMethod, propType); \
    class name##PropEntry \
    { \
    public: \
        name##PropEntry() \
        {\
            rtPropertyEntry* tail = headProperty(&entry); \
            entry.mPropertyName = "" #name ""; \
            entry.mPropType = RT_##propType##Type; \
            entry.mSetThunk = (rtSetPropertyThunk)NULL; \
            entry.mGetThunk = (rtGetPropertyThunk)&PARENTTYPE__::getMethod##_PropGetterThunk; \
            entry.mNext = tail; \
        }\
        rtPropertyEntry entry; \
    };\
    static name##PropEntry name##PropEntryInstance

#define rtConstantProperty(name, k, propType)\
    rtThunkConstantProperty(name, k, propType);		  \
    class name##PropEntry \
    { \
    public: \
        name##PropEntry() \
        {\
            rtPropertyEntry* tail = headProperty(&entry); \
            entry.mPropertyName = "" #name ""; \
            entry.mPropType = RT_##propType##Type; \
            entry.mSetThunk = (rtSetPropertyThunk)NULL; \
            entry.mGetThunk = (rtGetPropertyThunk)&PARENTTYPE__::name##_PropGetterThunk; \
            entry.mNext = tail; \
        }\
        rtPropertyEntry entry; \
    };\
    static name##PropEntry name##PropEntryInstance

#define  rtMethodNoArgAndNoReturn(name, method) \
    rtThunkNoArgAndNoReturn(method); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.mArgTypes = ""; \
            entry.mMethodName = name; \
            entry.mNext = tail; \
            entry.mReturnType = RT_voidType; \
            entry.mThunk = (rtMethodThunk)&PARENTTYPE__::method##_thunk;	\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance;

#define  rtMethod1ArgAndNoReturn(name, method, arg1type) \
    rtThunk1ArgAndNoReturn(method, arg1type); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.mArgTypes = RT_##arg1type##Type2; \
            entry.mMethodName = name; \
            entry.mNext = tail; \
            entry.mReturnType = RT_voidType; \
            entry.mThunk = (rtMethodThunk)&PARENTTYPE__::method##_thunk;	\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance;\
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod2ArgAndNoReturn(name, method, arg1type, arg2type) \
    rtThunk2ArgAndNoReturn(method, arg1type, arg2type); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.mArgTypes = RT_##arg1type##Type2 RT_##arg2type##Type2; \
            entry.mMethodName = name; \
            entry.mNext = tail; \
            entry.mReturnType = RT_voidType; \
            entry.mThunk = (rtMethodThunk)&PARENTTYPE__::method##_thunk;	\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance;\
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod3ArgAndNoReturn(name, method, arg1type, arg2type, arg3type) \
    rtThunk3ArgAndNoReturn(method, arg1type, arg2type, arg3type); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.mArgTypes = RT_##arg1type##Type2 RT_##arg2type##Type2 RT_##arg3type##Type2; \
            entry.mMethodName = name; \
            entry.mNext = tail; \
            entry.mReturnType = RT_voidType; \
            entry.mThunk = (rtMethodThunk)&PARENTTYPE__::method##_thunk; \
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance;\
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod4ArgAndNoReturn(name, method, arg1type, arg2type, arg3type, arg4type) \
    rtThunk4ArgAndNoReturn(method, arg1type, arg2type, arg3type, arg4type); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.mArgTypes = RT_##arg1type##Type2 RT_##arg2type##Type2 RT_##arg3type##Type2 RT_##arg4type##Type2; \
            entry.mMethodName = name; \
            entry.mNext = tail; \
            entry.mReturnType = RT_voidType; \
            entry.mThunk = (rtMethodThunk)&PARENTTYPE__::method##_thunk; \
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance;\
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod5ArgAndNoReturn(name, method, arg1type, arg2type, arg3type, arg4type, arg5type) \
    rtThunk5ArgAndNoReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.mArgTypes = RT_##arg1type##Type2 RT_##arg2type##Type2 RT_##arg3type##Type2 RT_##arg4type##Type2 RT_##arg5type##Type2; \
            entry.mMethodName = name; \
            entry.mNext = tail; \
            entry.mReturnType = RT_voidType; \
            entry.mThunk = (rtMethodThunk)&PARENTTYPE__::method##_thunk; \
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance;\
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod6ArgAndNoReturn(name, method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type) \
    rtThunk6ArgAndNoReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.mArgTypes = RT_##arg1type##Type2 RT_##arg2type##Type2 RT_##arg3type##Type2 RT_##arg4type##Type2 RT_##arg5type##Type2 RT_##arg6type##Type2; \
            entry.mMethodName = name; \
            entry.mNext = tail; \
            entry.mReturnType = RT_voidType; \
            entry.mThunk = (rtMethodThunk)&PARENTTYPE__::method##_thunk; \
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance;\
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod7ArgAndNoReturn(name, method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type) \
    rtThunk7ArgAndNoReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.mArgTypes = RT_##arg1type##Type2 RT_##arg2type##Type2 RT_##arg3type##Type2 RT_##arg4type##Type2 RT_##arg5type##Type2 RT_##arg6type##Type2 RT_##arg7type##Type2; \
            entry.mMethodName = name; \
            entry.mNext = tail; \
            entry.mReturnType = RT_voidType; \
            entry.mThunk = (rtMethodThunk)&PARENTTYPE__::method##_thunk; \
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance;\
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod8ArgAndNoReturn(name, method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, arg8type) \
    rtThunk8ArgAndNoReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, arg8type); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.mArgTypes = RT_##arg1type##Type2 RT_##arg2type##Type2 RT_##arg3type##Type2 RT_##arg4type##Type2 RT_##arg5type##Type2 RT_##arg6type##Type2 RT_##arg7type##Type2 RT_##arg8type##Type2; \
            entry.mMethodName = name; \
            entry.mNext = tail; \
            entry.mReturnType = RT_voidType; \
            entry.mThunk = (rtMethodThunk)&PARENTTYPE__::method##_thunk; \
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance;\
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod9ArgAndNoReturn(name, method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, arg8type, arg9type) \
    rtThunk2ArgAndNoReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, arg8type, arg9type); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.mArgTypes = RT_##arg1type##Type2 RT_##arg2type##Type2 RT_##arg3type##Type2 RT_##arg4type##Type2 RT_##arg5type##Type2 RT_##arg6type##Type2 RT_##arg7type##Type2 RT_##arg8type##Type2 RT_##arg9type##Type2; \
            entry.mMethodName = name; \
            entry.mNext = tail; \
            entry.mReturnType = RT_voidType; \
            entry.mThunk = (rtMethodThunk)&PARENTTYPE__::method##_thunk; \
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance;\
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }


#define  rtMethodNoArgAndReturn(name, method, returntype) \
    rtThunkNoArgAndReturn(method, returntype); \
    struct method##Entry \
    { \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
			entry.init(name, RT_voidType2, RT_##returntype##Type, (rtMethodThunk)&PARENTTYPE__::method##_thunk, tail);\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance;\
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod1ArgAndReturn(name, method, arg1type, returntype) \
    rtThunk1ArgAndReturn(method, arg1type, returntype); \
    struct method##Entry \
    { \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
			entry.init(name, RT_##arg1type##Type2, RT_##returntype##Type, (rtMethodThunk)&PARENTTYPE__::method##_thunk, tail);\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance;\
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }


#define  rtMethod2ArgAndReturn(name, method, arg1type, arg2type, returntype) \
    rtThunk2ArgAndReturn(method, arg1type, arg2type, returntype); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
			entry.init(name, RT_##arg1type##Type2 RT_##arg2type##Type2, RT_##returntype##Type, (rtMethodThunk)&PARENTTYPE__::method##_thunk, tail);\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance; \
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod3ArgAndReturn(name, method, arg1type, arg2type, arg3type, returntype) \
    rtThunk3ArgAndReturn(method, arg1type, arg2type, returntype); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.init(name, RT_##arg1type##Type2 RT_##arg2type##Type2 RT_##arg3type##Type2, RT_##returntype##Type, (rtMethodThunk)&PARENTTYPE__::method##_thunk, tail);\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance; \
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod4ArgAndReturn(name, method, arg1type, arg2type, arg3type, arg4type, returntype) \
    rtThunk4ArgAndReturn(method, arg1type, arg2type, arg3type, arg4type, returntype); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.init(name, RT_##arg1type##Type2 RT_##arg2type##Type2 RT_##arg3type##Type2 RT_##arg4type##Type2, RT_##returntype##Type, (rtMethodThunk)&PARENTTYPE__::method##_thunk, tail);\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance; \
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod5ArgAndReturn(name, method, arg1type, arg2type, arg3type, arg4type, arg5type, returntype) \
    rtThunk5ArgAndReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, returntype); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.init(name, RT_##arg1type##Type2 RT_##arg2type##Type2 RT_##arg3type##Type2 RT_##arg4type##Type2 RT_##arg5type##Type2, RT_##returntype##Type, (rtMethodThunk)&PARENTTYPE__::method##_thunk, tail);\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance; \
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod6ArgAndReturn(name, method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, returntype) \
    rtThunk6ArgAndReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, returntype); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.init(name, RT_##arg1type##Type2 RT_##arg2type##Type2 RT_##arg3type##Type2 RT_##arg4type##Type2 RT_##arg5type##Type2 RT_##arg6type##Type2, RT_##returntype##Type, (rtMethodThunk)&PARENTTYPE__::method##_thunk, tail);\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance; \
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod7ArgAndReturn(name, method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, returntype) \
    rtThunk7ArgAndReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, returntype); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.init(name, RT_##arg1type##Type2 RT_##arg2type##Type2 RT_##arg3type##Type2 RT_##arg4type##Type2 RT_##arg5type##Type2 RT_##arg6type##Type2 RT_##arg7type##Type2, RT_##returntype##Type, (rtMethodThunk)&PARENTTYPE__::method##_thunk, tail);\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance; \
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod8ArgAndReturn(name, method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, arg8type, returntype) \
    rtThunk8ArgAndReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, arg8type, returntype); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.init(name, RT_##arg1type##type2 RT_##arg2type##type2 RT_##arg3type##type2 RT_##arg4type##type2 RT_##arg5type##type2 RT_##arg6type##type2 RT_##arg7type##type2 RT_##arg8type##type2, RT_##returntype##Type, (rtMethodThunk)&PARENTTYPE__::method##_thunk, tail);\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance; \
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define  rtMethod9ArgAndReturn(name, method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, arg8type, arg9type, returntype) \
    rtThunk9ArgAndReturn(method, arg1type, arg2type, arg3type, arg4type, arg5type, arg6type, arg7type, arg8type, arg9type, returntype); \
    class method##Entry \
    { \
    public: \
        method##Entry() \
        {\
            rtMethodEntry* tail = head(&entry); \
            entry.init(name, RT_##arg1type##type2 RT_##arg2type##type2 RT_##arg3type##type2 RT_##arg4type##type2 RT_##arg5type##type2 RT_##arg6type##type2 RT_##arg7type##type2 RT_##arg8type##type2 RT_##arg9type##type2, RT_##returntype##Type, (rtMethodThunk)&PARENTTYPE__::method##_thunk, tail);\
        } \
        rtMethodEntry entry; \
    }; \
    static method##Entry method##EntryInstance; \
    virtual void* dontStrip##method() { return (void*)&method##EntryInstance; }

#define rtDefineMethod(obj, method) \
    obj::method##Entry obj::method##EntryInstance;

#define rtDefineProperty(obj, name) \
    obj::name##PropEntry obj::name##PropEntryInstance;

#endif
