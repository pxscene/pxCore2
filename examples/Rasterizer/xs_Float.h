// ====================================================================================================================
// ====================================================================================================================
//  xs_Float.h
// ====================================================================================================================
// ====================================================================================================================
#ifndef _xs_FLOAT_H_
#define _xs_FLOAT_H_

#include "xs_Core.h"
#include <math.h>

// ====================================================================================================================
//  Defines
// ====================================================================================================================
#ifndef _xs_DEFAULT_CONVERSION
#define _xs_DEFAULT_CONVERSION      0
#endif //_xs_DEFAULT_CONVERSION


#if _xs_BigEndian_
	#define _xs_iexp_				0
	#define _xs_iman_				1
#else
	#define _xs_iexp_				1       //intel is little endian
	#define _xs_iman_				0
#endif //BigEndian_


#define _xs_doublecopysgn(a,b)      ((int32*)&a)[_xs_iexp_]&=~(((int32*)&b)[_xs_iexp_]&0x80000000) 
#define _xs_doubleisnegative(a)     ((((int32*)&a)[_xs_iexp_])|0x80000000) 
class xs_Fixed;

// ====================================================================================================================
//  Constants
// ====================================================================================================================
const real64 _xs_doublemagic			= real64 (6755399441055744.0); 	    //2^52 * 1.5,  uses limited precisicion to floor
const real64 _xs_doublemagicdelta      	= (1.5e-8);                         //almost .5f = .5f + 1e^(number of exp bit)
const real64 _xs_doublemagicroundeps	= (.5f-_xs_doublemagicdelta);       //almost .5f = .5f - 1e^(number of exp bit)


// ====================================================================================================================
//  Prototypes
// ====================================================================================================================
#define xs_FToInt		  xs_CRoundToInt
int32 xs_CRoundToInt      (real64 val, real64 dmr =  _xs_doublemagic);
int32 xs_ToInt            (real64 val, real64 dme = -_xs_doublemagicroundeps);
int32 xs_FloorToInt       (real64 val, real64 dme =  _xs_doublemagicroundeps);
int32 xs_CeilToInt        (real64 val, real64 dme =  _xs_doublemagicroundeps);
int32 xs_RoundToInt       (real64 val);

//int32 versions
finline int32 xs_CRoundToInt      (int32 val)   {return val;}
finline int32 xs_ToInt            (int32 val)   {return val;}

//fixed versions
finline int32 xs_CRoundToInt      (xs_Fixed val);
finline int32 xs_ToInt            (xs_Fixed val);



// ================================================================================================================
//  xs_Fixed
// ================================================================================================================
class xs_Fixed
{
public:
#define XS_FIX_IS_FLOAT		0

#if XS_FIX_IS_FLOAT
	typedef		double xs_fxint;
	enum		{eSh = 0, eOne=1};
#else
	typedef int64 xs_fxint;
	enum		{eSh = 16, eOne=1<<eSh};
#endif
	xs_fxint	f;

	// ================================================================================================================
	//  Static 
	// ================================================================================================================
public:
	static finline xs_fxint	fixadd		(const xs_fxint a,  const xs_fxint b)		{return a+b;}
	static finline xs_fxint	fixsub		(const xs_fxint a,  const xs_fxint b)		{return a-b;}
#if XS_FIX_IS_FLOAT
	static finline xs_fxint	fixmul		(const xs_fxint a,  const xs_fxint b)		{return a*b;}
	static finline xs_fxint	fixdiv		(const xs_fxint a,  const xs_fxint b)		{return a/b;}
#else
	static finline xs_fxint	fixmul		(const xs_fxint a,  const xs_fxint b)		{return xs_fxint(int64(int64(a)*int64(b))>>eSh);}
	static finline xs_fxint	fixdiv		(const xs_fxint a,  const xs_fxint b)		{return xs_fxint(int64(int64(a)<<(eSh))/(int64(b)<<(0)));}
#endif

public:
	static finline xs_Fixed	fixadd_			(const xs_Fixed& a,  const xs_Fixed& b)		{xs_Fixed t; t.f=fixadd(a.f, b.f); return t;}
	static finline xs_Fixed	fixsub_			(const xs_Fixed& a,  const xs_Fixed& b)		{xs_Fixed t; t.f=fixsub(a.f, b.f); return t;}
	static finline xs_Fixed	fixmul_			(const xs_Fixed& a,  const xs_Fixed& b)		{xs_Fixed t; t.f=fixmul(a.f, b.f); return t;}
	static finline xs_Fixed	fixdiv_			(const xs_Fixed& a,  const xs_Fixed& b)		{xs_Fixed t; t.f=fixdiv(a.f, b.f); return t;}

	static finline xs_fxint	ToFix			(const xs_Fixed& a	);
	static finline xs_fxint	ToFix			(const real64    a	);
	static finline xs_fxint	ToFix			(const int       a	);
	static finline xs_fxint	ToFix			(const int32     a	);
	static finline real64	ToReal			(const xs_fxint  a	);
	static finline int		ToInt			(const xs_fxint  a	);

	// ================================================================================================================
	//  Constructor
	// ================================================================================================================
	finline xs_Fixed()								{ /*nothing */	}
	finline xs_Fixed(const xs_Fixed& f_)			{f = f_.f;		}
	finline xs_Fixed(const int i)					{f = ToFix(i);	}
	finline xs_Fixed(const int32 i)					{f = ToFix(i);	}
	finline xs_Fixed(const real32 fl)				{f = ToFix(fl);	}
	finline xs_Fixed(const real64 fl)				{f = ToFix(fl);	}
	finline xs_Fixed(const xs_fxint i, bool /*d*/)	{f = i;			}

	// ================================================================================================================
	//  Casting
	// ================================================================================================================
	finline operator unsigned char () const			{return (unsigned char)ToInt(f);}
	finline operator char () const					{return (char)ToInt(f);}
	finline operator unsigned short () const		{return (unsigned short)ToInt(f);}
	finline operator short () const					{return (short)ToInt(f);}
	finline operator unsigned int () const			{return ToInt(f);}
	finline operator int () const					{return ToInt(f);}
	finline operator unsigned long () const			{return ToInt(f);}
	finline operator long () const					{return ToInt(f);}
	finline operator float () const					{return (float)ToReal(f);}
	finline operator double () const				{return ToReal(f);}

	// ================================================================================================================
	//  operators
	// ================================================================================================================
	finline xs_Fixed &operator= (const xs_Fixed& a) {f = a.f;						return *this;}
	finline xs_Fixed &operator= (int a)				{f = ToFix(a);					return *this;}
	finline xs_Fixed &operator= (int32 a)			{f = ToFix(a);					return *this;}
	finline xs_Fixed &operator= (float a)			{f = ToFix(a);					return *this;}
	finline xs_Fixed &operator= (double a)			{f = ToFix(a);					return *this;}

	xs_Fixed &operator += (xs_Fixed const & a) 		{f += a.f;						return *this;}
	xs_Fixed &operator += (int a)					{f += ToFix(a); 				return *this;}
	xs_Fixed &operator += (int32 a)					{f += ToFix(a); 				return *this;}
	xs_Fixed &operator += (float a)					{f += ToFix(a); 				return *this;}
	xs_Fixed &operator += (double a)				{f += ToFix(a); 				return *this;}

	xs_Fixed &operator -= (xs_Fixed const & a)		{f -= a.f;						return *this;}
	xs_Fixed &operator -= (int a)					{f -= ToFix(a); 				return *this;}
	xs_Fixed &operator -= (int32 a)					{f -= ToFix(a); 				return *this;}
	xs_Fixed &operator -= (float a)					{f -= ToFix(a); 				return *this;}
	xs_Fixed &operator -= (double a)				{f -= ToFix(a); 				return *this;}

	xs_Fixed &operator *=(xs_Fixed const & a)		{f = fixmul(f, ToFix(a));		return *this;}
	xs_Fixed &operator *=(int a)					{f = f*a;						return *this;}
	xs_Fixed &operator *=(int32 a)					{f = f*a;						return *this;}
	xs_Fixed &operator *=(float a)					{f = fixmul(f, ToFix(a));		return *this;}
	xs_Fixed &operator *=(double a)					{f = fixmul(f, ToFix(a));		return *this;}

	xs_Fixed &operator /=(xs_Fixed const & a)		{f = fixdiv(f, ToFix(a));		return *this;}
	xs_Fixed &operator /=(int a)					{f = f/a;						return *this;}
	xs_Fixed &operator /=(int32 a)					{f = f/a;						return *this;}
	xs_Fixed &operator /=(float a)					{f = fixdiv(f, ToFix(a));		return *this;}
	xs_Fixed &operator /=(double a)					{f = fixdiv(f, ToFix(a));		return *this;}

#if XS_FIX_IS_FLOAT
	xs_Fixed operator >> (int32 s) const			{xs_Fixed t; t.f = f; if (s) t.f/=(1<<s); return t;}
	xs_Fixed operator << (int32 s) const			{xs_Fixed t; t.f = f; if (s) t.f*=(1<<s); return t;}
#else
	xs_Fixed operator >> (int32 s) const			{xs_Fixed t; t.f = f>>s; return t;}
	xs_Fixed operator << (int32 s) const			{xs_Fixed t; t.f = f<<s; return t;}
#endif

	// negation operator
	xs_Fixed operator - () const					{xs_Fixed t; t.f = -f; return t;}
};



// =================================================================================================================
// Implementation of inline arithmetic operations with fixed
// =================================================================================================================
finline const xs_Fixed	operator +	(const xs_Fixed& l, const xs_Fixed& r)	{return xs_Fixed::fixadd_(l, r); }
finline const xs_Fixed	operator +	(const xs_Fixed& l, const real64& r)	{return xs_Fixed::fixadd_(l, r); }
finline const xs_Fixed	operator +	(const xs_Fixed& l, const real32& r)	{return xs_Fixed::fixadd_(l, r); }
finline const xs_Fixed	operator +	(const xs_Fixed& l, const int32& r)		{return xs_Fixed::fixadd_(l, r); }
finline const xs_Fixed	operator +	(const real64 l,	const xs_Fixed& r)	{return xs_Fixed::fixadd_(l, r); }
finline const xs_Fixed	operator +	(const real32 l,	const xs_Fixed& r)	{return xs_Fixed::fixadd_(l, r); }
finline const xs_Fixed	operator +	(const int32 l,		const xs_Fixed& r)	{return xs_Fixed::fixadd_(l, r); }

finline const xs_Fixed	operator -	(const xs_Fixed& l, const xs_Fixed& r)	{return xs_Fixed::fixsub_(l, r); }
finline const xs_Fixed	operator -	(const xs_Fixed& l, const real64& r)	{return xs_Fixed::fixsub_(l, r); }
finline const xs_Fixed	operator -	(const xs_Fixed& l, const real32& r)	{return xs_Fixed::fixsub_(l, r); }
finline const xs_Fixed	operator -	(const xs_Fixed& l, const int32& r)		{return xs_Fixed::fixsub_(l, r); }
finline const xs_Fixed	operator -	(const real64 l,	const xs_Fixed& r)	{return xs_Fixed::fixsub_(l, r); }
finline const xs_Fixed	operator -	(const real32 l,	const xs_Fixed& r)	{return xs_Fixed::fixsub_(l, r); }
finline const xs_Fixed	operator -	(const int32 l,		const xs_Fixed& r)	{return xs_Fixed::fixsub_(l, r); }
finline const xs_Fixed	operator -	(const int l,		const xs_Fixed& r)	{return xs_Fixed::fixsub_(l, r); }

finline const xs_Fixed	operator *	(const xs_Fixed& l, const xs_Fixed& r)	{return xs_Fixed::fixmul_(l, r); }
finline const xs_Fixed	operator *	(const xs_Fixed& l, const real64 r)		{xs_Fixed f=l; f*=r; return f;}
finline const xs_Fixed	operator *	(const xs_Fixed& l, const real32 r)		{xs_Fixed f=l; f*=r; return f;}
finline const xs_Fixed	operator *	(const xs_Fixed& l, const int32 r)		{xs_Fixed f=l; f*=r; return f;}
finline const xs_Fixed	operator *	(const real64 l,	const xs_Fixed& r)	{xs_Fixed f=r; f*=l; return f;}
finline const xs_Fixed	operator *	(const real32 l,	const xs_Fixed& r)	{xs_Fixed f=r; f*=l; return f;}
finline const xs_Fixed	operator *	(const int32 l,		const xs_Fixed& r)	{xs_Fixed f=r; f*=l; return f;}
finline const xs_Fixed	operator *	(const int l,		const xs_Fixed& r)	{xs_Fixed f=r; f*=l; return f;}

finline const xs_Fixed	operator /	(const xs_Fixed& l, const xs_Fixed& r)	{return xs_Fixed::fixdiv_(l, r); }
finline const xs_Fixed	operator /	(const xs_Fixed& l, const real64 r)		{xs_Fixed f=l; f/=r; return f;}
finline const xs_Fixed	operator /	(const xs_Fixed& l, const real32 r)		{xs_Fixed f=l; f/=r; return f;}
finline const xs_Fixed	operator /	(const xs_Fixed& l, const int32 r)		{xs_Fixed f=l; f/=r; return f;}
finline const xs_Fixed	operator /	(const real64 l, const xs_Fixed& r)		{xs_Fixed f=xs_Fixed::fixdiv_(l,r); return f;}
finline const xs_Fixed	operator /	(const real32 l, const xs_Fixed& r)		{xs_Fixed f=xs_Fixed::fixdiv_(l,r); return f;}
finline const xs_Fixed	operator /	(const int32 l, const xs_Fixed& r)		{xs_Fixed f=xs_Fixed::fixdiv_(l,r); return f;}
finline const xs_Fixed	operator /	(const int l, const xs_Fixed& r)		{xs_Fixed f=xs_Fixed::fixdiv_(l,r); return f;}

#define xs_Fix_Compare_Op(OP)	\
	finline const bool		operator OP	(const xs_Fixed& l, const xs_Fixed& r)	{return l.f OP r.f; }\
	finline const bool		operator OP	(const xs_Fixed& l, const real64 r)		{return xs_Fixed::ToFix(l) OP xs_Fixed::ToFix(r); }\
	finline const bool		operator OP	(const xs_Fixed& l, const real32 r) 	{return xs_Fixed::ToFix(l) OP xs_Fixed::ToFix(r); }\
	finline const bool		operator OP	(const xs_Fixed& l, const int32 r) 		{return xs_Fixed::ToFix(l) OP xs_Fixed::ToFix(r); }\
	finline const bool		operator OP	(const xs_Fixed& l, const int r) 		{return xs_Fixed::ToFix(l) OP xs_Fixed::ToFix(r); }\
	finline const bool		operator OP	(const real64 l, const xs_Fixed& r)		{return xs_Fixed::ToFix(l) OP xs_Fixed::ToFix(r); }\
	finline const bool		operator OP	(const real32 l, const xs_Fixed& r) 	{return xs_Fixed::ToFix(l) OP xs_Fixed::ToFix(r); }\
	finline const bool		operator OP	(const int32 l, const xs_Fixed& r) 		{return xs_Fixed::ToFix(l) OP xs_Fixed::ToFix(r); }\
	finline const bool		operator OP	(const int l, const xs_Fixed& r) 		{return xs_Fixed::ToFix(l) OP xs_Fixed::ToFix(r); }

xs_Fix_Compare_Op(==)
xs_Fix_Compare_Op(!=)
xs_Fix_Compare_Op(<)
xs_Fix_Compare_Op(>)
xs_Fix_Compare_Op(<=)
xs_Fix_Compare_Op(>=)


#if _MSC_VER //disable << too large warning
#pragma warning (disable: 4293)
#endif

// ====================================================================================================================
//  Fix Class
// ====================================================================================================================
template <int32 N> class xs_Fix
{
public:
    typedef int32 Fix;
	enum {eSh = xs_Fixed::eSh};

    // ====================================================================================================================
    //  Basic Conversion from Numbers
    // ====================================================================================================================
    finline static Fix       ToFix       (const int val)	  {return val<<N;}
    finline static Fix       ToFix       (const int32 val)    {return val<<N;}
    finline static Fix       ToFix       (const real64 val)   {return xs_ConvertToFixed(val);}
	finline static Fix       ToFix       (const xs_Fixed val) {return Fix(N>eSh ? (val<<(N-eSh)) : (val>>(eSh-N)));}
	finline static int32     Round		 (const Fix f)        {return (f+xs_Fix<N>::Half())	 &xs_Fix<N>::IntMask();}
    finline static int32     Floor		 (const Fix f)        {return (f					)&xs_Fix<N>::IntMask();}
    finline static int32     Ceil		 (const Fix f)        {return (f+xs_Fix<N>::FracMask())&xs_Fix<N>::IntMask();}

    // ====================================================================================================================
    //  Basic Conversion to Numbers
    // ====================================================================================================================
    finline static real64    ToReal      (const Fix f)        {return real64(f)*Div();}
    finline static int32     ToInt       (const Fix f)        {return f>>N;}
    finline static int32     RoundToInt  (const Fix f)        {return (f+(1<<(N-1)))>>N;}
    finline static int32     FloorToInt  (const Fix f)        {return f>>N;}
    finline static int32     CeilToInt   (const Fix f)        {return (f+((1<<N)-1))>>N;}

    // ====================================================================================================================
    //  Const
    // ====================================================================================================================
    finline static int32	 Half()	    {return 1<<(N-1);}
    finline static int32	 One()	    {return 1<<N;}
    finline static int32	 FracMask() {return (1<<N)-1;}
    finline static int32	 IntMask()  {return ~((1<<N)-1);}
	finline static int32	 Shift()	{return N;}
	finline static real64    Div()      {const real64 div1=real64(1)/real64(1<<N); return div1;}

protected:
    // ====================================================================================================================
    // Helper function - mainly to preserve _xs_DEFAULT_CONVERSION
    // ====================================================================================================================
    finline static int32 xs_ConvertToFixed (real64 val)
    {
    #if _xs_DEFAULT_CONVERSION==0
        return xs_CRoundToInt(val, _xs_doublemagic/(1<<N));
    #else
        return (long)((val)*(1<<N));
    #endif
    }
};











// ====================================================================================================================

// ====================================================================================================================
// ====================================================================================================================
//  Inline implementation
// ====================================================================================================================
// ====================================================================================================================

// ====================================================================================================================
#if XS_FIX_IS_FLOAT
	finline xs_Fixed::xs_fxint	xs_Fixed::ToFix	(const xs_Fixed&  a	)	{return a.f;}
	finline xs_Fixed::xs_fxint	xs_Fixed::ToFix	(const real64    a	)	{return a;}
	finline xs_Fixed::xs_fxint	xs_Fixed::ToFix	(const int32     a	)	{return a;}
	finline xs_Fixed::xs_fxint	xs_Fixed::ToFix	(const int       a	)	{return a;}
	finline real64				xs_Fixed::ToReal(const xs_fxint  a	)	{return a;}
	finline int					xs_Fixed::ToInt	(const xs_fxint  a	)	{return xs_CRoundToInt(a);}
#else
	finline xs_Fixed::xs_fxint	xs_Fixed::ToFix	(const xs_Fixed&  a	)	{return a.f;}
	finline xs_Fixed::xs_fxint	xs_Fixed::ToFix	(const real64    a	)	{return xs_Fix<eSh>::ToFix(a);}
	finline xs_Fixed::xs_fxint	xs_Fixed::ToFix	(const int32     a	)	{return xs_Fix<eSh>::ToFix(int32(a));}
	finline xs_Fixed::xs_fxint	xs_Fixed::ToFix	(const int      a	)	{return xs_Fix<eSh>::ToFix(int(a));}
	finline real64				xs_Fixed::ToReal(const xs_fxint  a	)	{return xs_Fix<eSh>::ToReal(int32(a));}
	finline int					xs_Fixed::ToInt	(const xs_fxint  a	)	{return xs_Fix<eSh>::ToInt(int32(a));}
#endif

finline xs_Fixed sqrt (xs_Fixed a)	{return xs_Fixed(::sqrt(double(a)));}
finline xs_Fixed cos (xs_Fixed a)	{return xs_Fixed(::cos(double(a)));}
finline xs_Fixed sin (xs_Fixed a)	{return xs_Fixed(::sin(double(a)));}
finline xs_Fixed fabs (xs_Fixed a)	{return a.f<0 ? -a : a;}


finline int32		xs_CRoundToInt  (xs_Fixed val)				{return int32(val);}
finline int32		xs_ToInt        (xs_Fixed val)				{return int32(val);}
finline xs_Fixed	xs_FInverse		(xs_Fixed val)				{return val.f!=0 ? xs_Fixed(int32(1))/val : xs_Fixed(int32(0));}
inline xs_Fixed		xs_Min			(xs_Fixed a, xs_Fixed b)	{return a<b ? a : b;}
inline xs_Fixed		xs_Max			(xs_Fixed a, xs_Fixed b)	{return a<b ? a : b;}


// ====================================================================================================================
finline int32 xs_CRoundToInt(real64 val, real64 dmr)
{
#if _xs_DEFAULT_CONVERSION==0
    val		= val + dmr;
	return ((int32*)&val)[_xs_iman_];
    //return 0;
#else
    return int32(floor(val+.5));
#endif
}


// ====================================================================================================================
finline int32 xs_ToInt(real64 val, real64 dme)
{
    /* unused - something else I tried...
            _xs_doublecopysgn(dme,val);
            return xs_CRoundToInt(val+dme);
            return 0;
    */

#if _xs_DEFAULT_CONVERSION==0
	return (val<0) ?   xs_CRoundToInt(val-dme) : 
					   xs_CRoundToInt(val+dme);
#else
    return int32(val);
#endif
}


// ====================================================================================================================
finline int32 xs_FloorToInt(real64 val, real64 dme)
{
#if _xs_DEFAULT_CONVERSION==0
    return xs_CRoundToInt (val - dme);
#else
    return floor(val);
#endif
}


// ====================================================================================================================
finline int32 xs_CeilToInt(real64 val, real64 dme)
{
#if _xs_DEFAULT_CONVERSION==0
    return xs_CRoundToInt (val + dme);
#else
    return ceil(val);
#endif
}


// ====================================================================================================================
finline int32 xs_RoundToInt(real64 val)
{
#if _xs_DEFAULT_CONVERSION==0
    return xs_CRoundToInt (val + _xs_doublemagicdelta);
#else
    return floor(val+.5);
#endif
}

#undef int32


// ====================================================================================================================
// ====================================================================================================================
#endif // _xs_FLOAT_H_
