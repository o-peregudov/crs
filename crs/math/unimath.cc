//
//  UNIMATH.CC - universal mathematic functions and templates
//  Copyright (c) Dec 15, 2005 Oleg N. Peregudov <op@pochta.ru>
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

template <class _FLT>
_FLT  frexp10 ( const _FLT value, int * order )
{
      *order = 0;
      _FLT mantiss ( abs( value ) );
      if( mantiss > 1 )
            while( mantiss >= 10 )
            {
                  (*order)++;
                  mantiss /= 10;
            }
      else if( ( mantiss > 0 ) && ( mantiss < 1 ) )
            while( mantiss < 1 )
            {
                  (*order)--;
                  mantiss *= 10;
            }
      return ( mantiss * sign( value ) );
}

template <class _FLT>
_FLT  axis_delta ( const _FLT & range, const int nGridsDefault, const _FLT & threshold )
{
      int   order = 0;
      _FLT  delta = 1,
            metric = frexp10( abs( range ), &order );
      int   nGrids = static_cast< int >( metric );
      
      if( UniMath::isZero( metric - 1 ) )
      {
            nGrids = 10;
            delta = .1;
      }
      
      while( nGrids < nGridsDefault )
      {
            delta /= 2;
            nGrids *= 2;
            if( delta < threshold )
            {
                  delta = 1;
                  nGrids = static_cast< int >( metric * 10 );
                  --order;
            }
      }
      
      delta *= pow( 10.0, order );
      
      return delta;
}

template <class _FLT>
_FLT  first_tick ( const _FLT & _x1, const _FLT & _x2, const _FLT & delta, int & nGrids )
{
      _FLT x1( _x1 ), x2( _x2 ), ix1( 0 ), ix2( 0 ), d( UniMath::abs( delta ) );
      
      modf( _x1, &ix1 );
      modf( _x2, &ix2 );
      
      bool f( ( ix1 != 0 ) && ( ix1 == ix2 ) );
      if( f ) x1 -= ix1, x2 -= ix2;
      
      int n ( static_cast<int>( x1 / d ) );
      if( x1 > 0 ) while( n * d < x1 ) n++;
      _FLT the1st ( n * d );
      
      nGrids = static_cast<int>( (x2 - the1st) / d );
      for( _FLT last = the1st + nGrids * d; last <= x2; nGrids++, last += d );
      
      return ( the1st + ( f ? ix1 : 0 ) );
}

template <class Function>
double _derivative ( Function f, const double & x )
{
      static const double dx = DBL_EPSILON * 1e2;
      return ( f( x + dx ) - f( x ) ) / dx;
}

template <class Function>
double _root ( Function f, const double & a, const double & b )
{
      int signm = 0;
      static const double EPSILON = DBL_EPSILON * 1e3;
      double l = a, r = b, fl = f( l ), fr = f( r ), m, fm;
      if( sign( fl ) == sign( fr ) )
            return abs( sqrt( -1.0 ) );               // NaN
      for(;;)
      {
            fm = f( m = ( l + r ) / 2 );
            if( abs( fm ) <= EPSILON || abs( r - l ) <= EPSILON )
                  break;
            else
            {
                  signm = sign( fm );
                  if( sign( fl ) != signm )
                        r = m, fr = fm;
                  else if( signm != sign( fr ) )
                        l = m, fl = fm;
                  else
                        break;
            }
      }
      return m;
}

template <class Function>
double root ( Function f, const double & g, const double & dx, const int nStepsMax )
{
      int nSteps = 0;
      double p = g, c = g, s = -dx * sign( f( g ) ) * sign( _derivative( f, g ) );
      while( ( f( p ) * f( c += s ) >= 0 ) && ( nSteps < nStepsMax ) )
      {
            ++nSteps;
            p = c;
      }
      if( nSteps == nStepsMax )
            return abs( sqrt( -1.0 ) );   // NaN
      return ( p < c ) ? _root( f, p, c ) : _root( f, c, p );
}

template <class ContainerType>
std::valarray<double> ipLagrange2 ( const ContainerType & c )
{
      std::valarray<double> r ( 3 ), b ( 3 ), xm ( 3 ), xa ( 3 );
      for( size_t i = 0; i < 3; ++i )
      {
            b[ i ] = c[ i ].second;
            xa[ i ] = 0, xm[ i ] = 1;
            for( size_t j = 0; j < 3; ++j )
            {
                  if( i != j )
                  {
                        b[ i ] /= ( c[ i ].first - c[ j ].first );
                        xm[ i ] *= c[ j ].first;
                        xa[ i ] -= c[ j ].first;
                  }
            }
      }
      r[ 2 ] = b.sum();
      r[ 1 ] = ( xa *= b ).sum();
      r[ 0 ] = ( xm *= b ).sum();
      return r;
}
