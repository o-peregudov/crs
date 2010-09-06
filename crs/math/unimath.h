#ifndef UNIMATH_H
#define UNIMATH_H 1
//
//  UNIMATH.H - universal mathematic functions and templates
//  Copyright (c) Feb 4, 2006 Oleg N. Peregudov <op@pochta.ru>
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

// kill macros
#ifdef min
#undef min
#endif

// kill macros
#ifdef max
#undef max
#endif

#include <cmath>
#include <float.h>
#include <valarray>

#define Pi		3.1415926535897932384626433832795

namespace UniMath {

template <class _FLT> inline
bool  isZero ( const _FLT & expr )
{
      return ( ( 1.0 + expr ) == 1.0 );
}

template <class _FLT> inline
int   sign ( const _FLT & expr )
{
      return ( ( expr < 0 ) ? -1 : 1 );
}

template <class _FLT> inline
_FLT  abs ( const _FLT & expr )
{
      return ( ( expr < 0 ) ? -expr : expr );
}

template <class Type> inline
bool  within ( const Type & x, const Type & a, const Type & b )
{
      return ( (a <= x) && (x <= b) );
}

template <class _FLT> inline _FLT min ( _FLT a, _FLT b ) { return ( a < b ? a : b ); }
template <class _FLT> inline _FLT max ( _FLT a, _FLT b ) { return ( a > b ? a : b ); }

//
// converts the floating-point number to a scientific form
//
template <class _FLT> _FLT frexp10 ( const _FLT value, int * order );

// =============================================================================
// переводит вещественное число в E-форму, при этом оставляет только
// указанное число знаков после точки
// =============================================================================
template <class _FLT> inline
_FLT  trunc_after ( const _FLT value, int * order, const unsigned int nDigits = 2 )
{
      _FLT res( frexp10( value, order ) );
      (*order)++;
      res *= pow( 10, nDigits - 1 );
      return static_cast< int >( res ) / pow( 10, nDigits );
}

template <class _FLT> inline
_FLT  modf ( const _FLT value, _FLT * intpart )
{
      return ::modf( value, intpart );
}

inline
int   modf ( const int value, int * intpart )
{
      *intpart = 0;
      return value;
}

// =============================================================================
// возвращает порядковый номер (слева-направо) первого ненулевого разряда
// после точки
// =============================================================================
template <class _FLT> inline
int   first_nzd ( const _FLT & value )
{
      int order( 0 );
      _FLT integer_part( 0 );
      frexp10( modf( value, &integer_part ), &order );
      return abs( order );
}

//
// функции форматирования оси
//
template <class _FLT>
_FLT  axis_delta ( const _FLT & range, const int nGridsDefault, const _FLT & threshold = 0.125 );

template <class _FLT>
_FLT  first_tick ( const _FLT & x1, const _FLT & x2, const _FLT & delta, int & nGrids );

template <class _FLT> inline
_FLT  first_tick ( const _FLT & _x1, const _FLT & _x2, const _FLT & delta )
{
      int nGrids( 0 );
      return first_tick( _x1, _x2, delta, nGrids );
}

//
// прочие функции
//
template <class Function>
double root ( Function, const double &, const double & = .5, const int nStepsMax = 1000 );

template <class _FLT> inline
long  round ( const _FLT & x ) {
      return static_cast<long>( x + 0.5 );
}

#include <crs/math/unimath.cc>

} // namespace UniMath
#endif // UNIMATH_H
