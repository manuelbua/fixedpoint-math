/**
 * Copyright (c) 2006 Manuel Bua
 *
 * THIS SOFTWARE IS PROVIDED 'AS-IS', WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY. IN NO EVENT WILL THE AUTHORS BE HELD LIABLE FOR ANY DAMAGES
 * ARISING FROM THE USE OF THIS SOFTWARE.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not
 *     be misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 *
 */

#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include <stdint.h>


namespace fastmath
{

	/**
	 *	Mantains precalculated basic information such
	 *	as bit-masks and other constants.
	 */
	template<int32_t precision_bits>
	class FixedPointInfo
	{
		protected:
			enum { FRACTION_MASK	= ( 1 << precision_bits ) - 1		};
			enum { ONE				= ( 1 << precision_bits )			};
			enum { ROUND			= ( 1 << ( precision_bits - 1 ) )	};
			enum { HALF_BITS		= ( precision_bits / 2 )			};
			enum { SIGN_BIT			= ( 1 << 31 )						};
	};


	/**
	 *	More comments for me ...
	 */
	template<int32_t precision_bits,
			 template <int32_t> class MulPrecisionPolicy,
			 template <int32_t> class DivPrecisionPolicy>
	class FixedPoint : public FixedPointInfo<precision_bits>
	{
		public:


			/**
			 *	Construction w/ precision conversion
			 */
			template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
			explicit FixedPoint( const FixedPoint<bits, mulP, divP>& rhs )
			{
				v = translate( rhs.v, bits );
			}


			/**
			 *	Assignment w/ precision conversion
			 *
			 *	\think
			 *		remove because subtle ambiguities?
			 */
			template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
			inline FixedPoint& operator=( const FixedPoint<bits, mulP, divP>& rhs )
			{
				v = translate( rhs.v, bits );
				return *this;
			}


			/** Construction */
			inline static FixedPoint fromRaw( int32_t raw )	{ FixedPoint tmp; tmp.v = raw; return tmp; }
			FixedPoint() : v( 0 ) {}
			FixedPoint( const FixedPoint& rhs )	: v( rhs.v ) {}
			explicit FixedPoint( float_t rhs )			: v( (int32_t)( rhs *  (float_t)ONE + ( rhs < 0 ? -0.5f : 0.5f ) ) ) {}
			explicit FixedPoint( double_t rhs )			: v( (int32_t)( rhs * (double_t)ONE + ( rhs < 0 ? -0.5f : 0.5f ) ) ) {}
			explicit FixedPoint( int32_t rhs )			: v( rhs << precision_bits ) {}

			/** FixedPoint assignment */
			inline FixedPoint& operator=( const FixedPoint& rhs )		{ v = rhs.v; return *this; }
			inline FixedPoint& operator+=( const FixedPoint& rhs )	{ v += rhs.v; return *this; }
			inline FixedPoint& operator-=( const FixedPoint& rhs )	{ v -= rhs.v; return *this; }
			//inline FixedPoint& operator*=( const FixedPoint& rhs )	{ v = mul_t::mul( v, rhs.v ); return *this; }
			//inline FixedPoint& operator/=( const FixedPoint& rhs )	{ v = div_t::div( v, rhs.v ); return *this; }
			inline FixedPoint& operator*=( const FixedPoint& rhs )	{ v = MulPrecisionPolicy<precision_bits>::mul( v, rhs.v ); return *this; }
			inline FixedPoint& operator/=( const FixedPoint& rhs )	{ v = DivPrecisionPolicy<precision_bits>::div( v, rhs.v ); return *this; }

			/** int32_t assignment */
			inline FixedPoint& operator=( int32_t rhs )				{ v = FixedPoint( rhs ).v; return *this; }
			inline FixedPoint& operator+=( int32_t rhs )				{ v += rhs << precision_bits; return *this; }
			inline FixedPoint& operator-=( int32_t rhs )				{ v -= rhs << precision_bits; return *this; }
			inline FixedPoint& operator*=( int32_t rhs )				{ v *= rhs; return *this; }
			inline FixedPoint& operator/=( int32_t rhs )				{ v /= rhs; return *this; }

			/** float_t assignment */
			inline FixedPoint& operator=( float_t rhs )				{ v = FixedPoint( rhs ).v; return *this; }
			inline FixedPoint& operator+=( float_t rhs )				{ v += FixedPoint( rhs ).v; return *this; }
			inline FixedPoint& operator-=( float_t rhs )				{ v -= FixedPoint( rhs ).v; return *this; }
			inline FixedPoint& operator*=( float_t rhs )				{ *this *= FixedPoint( rhs ); return *this; }
			inline FixedPoint& operator/=( float_t rhs )				{ *this /= FixedPoint( rhs ); return *this; }

			/** double_t assignment */
			inline FixedPoint& operator=( double_t rhs )				{ v = FixedPoint( rhs ).v; return *this; }
			inline FixedPoint& operator+=( double_t rhs )			{ v += FixedPoint( rhs ).v; return *this; }
			inline FixedPoint& operator-=( double_t rhs )			{ v -= FixedPoint( rhs ).v; return *this; }
			inline FixedPoint& operator*=( double_t rhs )			{ *this *= FixedPoint( rhs ); return *this; }
			inline FixedPoint& operator/=( double_t rhs )			{ *this /= FixedPoint( rhs ); return *this; }

			/** shift assignment */
			inline FixedPoint& operator<<=( int32_t shift )			{ v <<= shift; return *this; }
			inline FixedPoint& operator>>=( int32_t shift )			{ v >>= shift; return *this; }

			/** converters */
			inline operator int32_t()								{ return v >> precision_bits; }
			inline operator float_t()								{ return v * ( 1.0f / static_cast<float_t>( ONE ) ); }
			inline operator double_t()								{ return v * ( 1.0f / static_cast<double_t>( ONE ) ); }


			/**
			 *	Gives access to the integer part of the number.
			 *
			 *	An use of the integer portion could be for lerping fixed-point
			 *	quantities.
			 */
			inline int32_t		getInteger() const  { return( v >> precision_bits ); }

			/**
			 *	Gives access to the fractional part of the number.
			 */
			inline uint32_t		getFraction() const { return( v & FRACTION_MASK ); }

			inline int32_t		getRaw() const		{ return v; }

			/** Unary operators */
			inline FixedPoint operator+() const		{ return FixedPoint::fromRaw(  v ); }
			inline FixedPoint operator-() const		{ return FixedPoint::fromRaw( -v ); }


			/** Comparison operators */

			// FixedPoint
			inline bool_t operator==( const FixedPoint& other )		{ return v == other.v; }
			inline bool_t operator!=( const FixedPoint& other )		{ return v != other.v; }
			inline bool_t operator<( const FixedPoint& other )		{ return v  < other.v; }
			inline bool_t operator>( const FixedPoint& other )		{ return v  > other.v; }
			inline bool_t operator<=( const FixedPoint& other )		{ return v <= other.v; }
			inline bool_t operator>=( const FixedPoint& other )		{ return v >= other.v; }

			// int32_t
			inline bool_t operator==( int32_t other )				{ return v == FixedPoint( other ).v; }
			inline bool_t operator!=( int32_t other )				{ return v != FixedPoint( other ).v; }
			inline bool_t operator<( int32_t other )				{ return v  < FixedPoint( other ).v; }
			inline bool_t operator>( int32_t other )				{ return v  > FixedPoint( other ).v; }
			inline bool_t operator<=( int32_t other )				{ return v <= FixedPoint( other ).v; }
			inline bool_t operator>=( int32_t other )				{ return v >= FixedPoint( other ).v; }

			// float
			inline bool_t operator==( float_t other )				{ return v == FixedPoint( other ).v; }
			inline bool_t operator!=( float_t other )				{ return v != FixedPoint( other ).v; }
			inline bool_t operator<( float_t other )				{ return v  < FixedPoint( other ).v; }
			inline bool_t operator>( float_t other )				{ return v  > FixedPoint( other ).v; }
			inline bool_t operator<=( float_t other )				{ return v <= FixedPoint( other ).v; }
			inline bool_t operator>=( float_t other )				{ return v >= FixedPoint( other ).v; }

			// double
			inline bool_t operator==( double_t other )				{ return v == FixedPoint( other ).v; }
			inline bool_t operator!=( double_t other )				{ return v != FixedPoint( other ).v; }
			inline bool_t operator<( double_t other )				{ return v  < FixedPoint( other ).v; }
			inline bool_t operator>( double_t other )				{ return v  > FixedPoint( other ).v; }
			inline bool_t operator<=( double_t other )				{ return v <= FixedPoint( other ).v; }
			inline bool_t operator>=( double_t other )				{ return v >= FixedPoint( other ).v; }


			/** Utilities */
/*
			inline FixedPoint mulDiv( const FixedPoint& m1, const FixedPoint& m2, const FixedPoint& d )
			{
				int64_t t = int64_t( m1 ) * int64_t( m2 );
				t += static_cast<int32_t>( ROUND );
				t /= d;
				return int32_t(t);
			}
*/
			inline FixedPoint abs()
			{
				return FixedPoint::fromRaw( ( v & ( 1 << 31 ) ) ? -v : v );
			}

			inline FixedPoint sign()
			{
				return FixedPoint::fromRaw( ( v & ( 1 << 31 ) ) ? -static_cast<int32_t>( ONE ): static_cast<int32_t>( ONE ) );
			}

			inline FixedPoint floor()
			{
				return FixedPoint( (int32_t)( v >> precision_bits ) );
			}

			inline FixedPoint ceil()
			{
				int32_t tmp = -( -v >> precision_bits );
				return FixedPoint( tmp );
			}

			inline FixedPoint round()
			{
				int64_t tmp = int64_t( v );
				if( tmp > 0 ) tmp += static_cast<int32_t>( ROUND ); else tmp-= static_cast<int32_t>( ROUND );
				tmp /= static_cast<int32_t>( ONE );
				return FixedPoint( int32_t( tmp ) );
			}


		private:

			int32_t v;

			inline static int32_t translate( int32_t src, int32_t bits )
			{
				if( bits < precision_bits )
				{
					return( src << ( precision_bits - bits ) );
				}
				else
				{
					return( src >> ( bits - precision_bits ) );
				}
			}


			//friend FixedPoint<0>;    friend FixedPoint<1>;    friend FixedPoint<2>;    friend FixedPoint<3>;
			//friend FixedPoint<4>;    friend FixedPoint<5>;    friend FixedPoint<6>;    friend FixedPoint<7>;
			//friend FixedPoint<8>;    friend FixedPoint<9>;    friend FixedPoint<10>;   friend FixedPoint<11>;
			//friend FixedPoint<12>;   friend FixedPoint<13>;   friend FixedPoint<14>;   friend FixedPoint<15>;
			//friend FixedPoint<16>;   friend FixedPoint<17>;   friend FixedPoint<18>;   friend FixedPoint<19>;
			//friend FixedPoint<20>;   friend FixedPoint<21>;   friend FixedPoint<22>;   friend FixedPoint<23>;
			//friend FixedPoint<24>;   friend FixedPoint<25>;   friend FixedPoint<26>;   friend FixedPoint<27>;
			//friend FixedPoint<28>;   friend FixedPoint<29>;   friend FixedPoint<30>;   friend FixedPoint<31>;
	};


	//////////////////////////////////////////////////////////////////////////
	// "Add" operation on FixedPoint (global)
	//////////////////////////////////////////////////////////////////////////


	// op: FixedPoint + FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator+( const FixedPoint<bits, mulP, divP>& lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result += rhs;
		return result;
	}


	/**
	 *	int32_t
	 */

	// op: FixedPoint + int32_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator+( const FixedPoint<bits, mulP, divP>& lhs, int32_t rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result += rhs;
		return result;
	}

	// op: int32_t + FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator+( int32_t lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		return rhs + lhs;
	}


	/**
	 *	float_t
	 */

	// op: FixedPoint + float_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator+( const FixedPoint<bits, mulP, divP>& lhs, float_t rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result += rhs;
		return result;
	}

	// op: float_t + FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator+( float_t lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		return rhs + lhs;
	}


	/**
	 *	double_t
	 */

	// op: FixedPoint + double_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator+( const FixedPoint<bits, mulP, divP>& lhs, double_t rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result += rhs;
		return result;
	}

	// op: double_t + FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator+( double_t lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		return rhs + lhs;
	}


	//////////////////////////////////////////////////////////////////////////
	// "Subtract" operation on FixedPoint (global)
	//////////////////////////////////////////////////////////////////////////


	// op: FixedPoint - FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator-( const FixedPoint<bits, mulP, divP>& lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result -= rhs;
		return result;
	}


	/**
	 *	int32_t
	 */

	// op: FixedPoint - int32_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator-( const FixedPoint<bits, mulP, divP>& lhs, int32_t rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result -= rhs;
		return result;
	}

	// op: int32_t - FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator-( int32_t lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		return FixedPoint<bits, mulP, divP>( lhs ) - rhs;
	}


	/**
	 *	float_t
	 */

	// op: FixedPoint - float_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator-( const FixedPoint<bits, mulP, divP>& lhs, float_t rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result -= rhs;
		return result;
	}

	// op: float_t - FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator-( float_t lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		return FixedPoint<bits, mulP, divP>( lhs ) - rhs;
	}


	/**
	 *	double_t
	 */

	// op: FixedPoint - double_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator-( const FixedPoint<bits, mulP, divP>& lhs, double_t rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result -= rhs;
		return result;
	}

	// op: double_t - FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator-( double_t lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		return FixedPoint<bits, mulP, divP>( lhs ) - rhs;
	}


	//////////////////////////////////////////////////////////////////////////
	// "Multiply" operation on FixedPoint (global)
	//////////////////////////////////////////////////////////////////////////


	// op: FixedPoint * FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator*( const FixedPoint<bits, mulP, divP>& lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result *= rhs;
		return result;
	}


	/**
	*	int32_t
	*/

	// op: FixedPoint * int32_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator*( const FixedPoint<bits, mulP, divP>& lhs, int32_t rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result *= rhs;
		return result;
	}

	// op: int32_t * FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator*( int32_t lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		return rhs * lhs;
	}


	/**
	*	float_t
	*/

	// op: FixedPoint * float_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator*( const FixedPoint<bits, mulP, divP>& lhs, float_t rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result *= rhs;
		return result;
	}

	// op: float_t * FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator*( float_t lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		return rhs * lhs;
	}


	/**
	*	double_t
	*/

	// op: FixedPoint * double_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator*( const FixedPoint<bits, mulP, divP>& lhs, double_t rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result *= rhs;
		return result;
	}

	// op: double_t * FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator*( double_t lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		return rhs * lhs;
	}


	//////////////////////////////////////////////////////////////////////////
	// "Division" operation on FixedPoint (global)
	//////////////////////////////////////////////////////////////////////////


	// op: FixedPoint / FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator/( const FixedPoint<bits, mulP, divP>& lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result /= rhs;
		return result;
	}


	/**
	 *	int32_t
	 */

	// op: FixedPoint / int32_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator/( const FixedPoint<bits, mulP, divP>& lhs, int32_t rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result /= rhs;
		return result;
	}

	// op: int32_t / FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator/( int32_t lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		return FixedPoint<bits, mulP, divP>( lhs ) / rhs;
	}


	/**
	 *	float_t
	 */

	// op: FixedPoint / float_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator/( const FixedPoint<bits, mulP, divP>& lhs, float_t rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result /= rhs;
		return result;
	}

	// op: float_t / FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator/( float_t lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		return FixedPoint<bits, mulP, divP>( lhs ) / rhs;
	}


	/**
	 *	double_t
	 */

	// op: FixedPoint / double_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator/( const FixedPoint<bits, mulP, divP>& lhs, double_t rhs )
	{
		FixedPoint<bits, mulP, divP> result( lhs );
		result /= rhs;
		return result;
	}

	// op: double_t / FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator/( double_t lhs, const FixedPoint<bits, mulP, divP>& rhs )
	{
		return FixedPoint<bits, mulP, divP>( lhs ) / rhs;
	}


	//////////////////////////////////////////////////////////////////////////
	// Shift operators
	//////////////////////////////////////////////////////////////////////////

	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator<<( const FixedPoint<bits, mulP, divP>& x, int32_t shift )
	{
		FixedPoint<bits, mulP, divP> result( x );
		result <<= shift;
		return result;
	}

	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>
	inline FixedPoint<bits, mulP, divP> operator>>( const FixedPoint<bits, mulP, divP>& x, int32_t shift )
	{
		FixedPoint<bits, mulP, divP> result( x );
		result >>= shift;
		return result;
	}


	//////////////////////////////////////////////////////////////////////////
	// Comparison operators
	//////////////////////////////////////////////////////////////////////////

	// FixedPoint
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator==( const FixedPoint<bits, mulP, divP>& lhs, const FixedPoint<bits, mulP, divP>& rhs )	{ return lhs == rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator!=( const FixedPoint<bits, mulP, divP>& lhs, const FixedPoint<bits, mulP, divP>& rhs )	{ return lhs != rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<( const FixedPoint<bits, mulP, divP>& lhs, const FixedPoint<bits, mulP, divP>& rhs )	{ return lhs  < rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<=( const FixedPoint<bits, mulP, divP>& lhs, const FixedPoint<bits, mulP, divP>& rhs )	{ return lhs <= rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>( const FixedPoint<bits, mulP, divP>& lhs, const FixedPoint<bits, mulP, divP>& rhs )	{ return lhs  > rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>=( const FixedPoint<bits, mulP, divP>& lhs, const FixedPoint<bits, mulP, divP>& rhs )	{ return lhs >= rhs;	}

	// int32_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator==( int32_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs ) == rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator!=( int32_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs ) != rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<( int32_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs )  < rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<=( int32_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs ) <= rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>( int32_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs )  > rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>=( int32_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs ) >= rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator==( const FixedPoint<bits, mulP, divP>& lhs, int32_t rhs )					{ return lhs == FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator!=( const FixedPoint<bits, mulP, divP>& lhs, int32_t rhs )					{ return lhs != FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<( const FixedPoint<bits, mulP, divP>& lhs, int32_t rhs )					{ return lhs <  FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<=( const FixedPoint<bits, mulP, divP>& lhs, int32_t rhs )					{ return lhs <= FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>( const FixedPoint<bits, mulP, divP>& lhs, int32_t rhs )					{ return lhs >  FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>=( const FixedPoint<bits, mulP, divP>& lhs, int32_t rhs )					{ return lhs >= FixedPoint<bits, mulP, divP>( rhs );	}

	// float_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator==( float_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs ) == rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator!=( float_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs ) != rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<( float_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs )  < rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<=( float_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs ) <= rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>( float_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs )  > rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>=( float_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs ) >= rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator==( const FixedPoint<bits, mulP, divP>& lhs, float_t rhs )					{ return lhs == FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator!=( const FixedPoint<bits, mulP, divP>& lhs, float_t rhs )					{ return lhs != FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<( const FixedPoint<bits, mulP, divP>& lhs, float_t rhs )					{ return lhs <  FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<=( const FixedPoint<bits, mulP, divP>& lhs, float_t rhs )					{ return lhs <= FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>( const FixedPoint<bits, mulP, divP>& lhs, float_t rhs )					{ return lhs >  FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>=( const FixedPoint<bits, mulP, divP>& lhs, float_t rhs )					{ return lhs >= FixedPoint<bits, mulP, divP>( rhs );	}

	// double_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator==( double_t lhs, const FixedPoint<bits, mulP, divP>& rhs )				{ return FixedPoint<bits, mulP, divP>( lhs ) == rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator!=( double_t lhs, const FixedPoint<bits, mulP, divP>& rhs )				{ return FixedPoint<bits, mulP, divP>( lhs ) != rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<( double_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs )  < rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<=( double_t lhs, const FixedPoint<bits, mulP, divP>& rhs )				{ return FixedPoint<bits, mulP, divP>( lhs ) <= rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>( double_t lhs, const FixedPoint<bits, mulP, divP>& rhs )					{ return FixedPoint<bits, mulP, divP>( lhs )  > rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>=( double_t lhs, const FixedPoint<bits, mulP, divP>& rhs )				{ return FixedPoint<bits, mulP, divP>( lhs ) >= rhs;	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator==( const FixedPoint<bits, mulP, divP>& lhs, double_t rhs )				{ return lhs == FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator!=( const FixedPoint<bits, mulP, divP>& lhs, double_t rhs )				{ return lhs != FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<( const FixedPoint<bits, mulP, divP>& lhs, double_t rhs )					{ return lhs <  FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator<=( const FixedPoint<bits, mulP, divP>& lhs, double_t rhs )				{ return lhs <= FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>( const FixedPoint<bits, mulP, divP>& lhs, double_t rhs )					{ return lhs >  FixedPoint<bits, mulP, divP>( rhs );	}
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline bool_t operator>=( const FixedPoint<bits, mulP, divP>& lhs, double_t rhs )				{ return lhs >= FixedPoint<bits, mulP, divP>( rhs );	}


	//////////////////////////////////////////////////////////////////////////
	// Other assignments
	//////////////////////////////////////////////////////////////////////////

	// int32_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline int32_t& operator+=( int32_t& lhs, const FixedPoint<bits, mulP, divP>& rhs)		{ lhs = (FixedPoint<bits, mulP, divP>)lhs + rhs; return lhs; }
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline int32_t& operator-=( int32_t& lhs, const FixedPoint<bits, mulP, divP>& rhs)		{ lhs = (FixedPoint<bits, mulP, divP>)lhs - rhs; return lhs; }
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline int32_t& operator*=( int32_t& lhs, const FixedPoint<bits, mulP, divP>& rhs)		{ lhs = (FixedPoint<bits, mulP, divP>)lhs * rhs; return lhs; }
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline int32_t& operator/=( int32_t& lhs, const FixedPoint<bits, mulP, divP>& rhs)		{ lhs = (FixedPoint<bits, mulP, divP>)lhs / rhs; return lhs; }

	// float_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline float_t& operator+=( float_t& lhs, const FixedPoint<bits, mulP, divP>& rhs)		{ lhs = (FixedPoint<bits, mulP, divP>)lhs + rhs; return lhs; }
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline float_t& operator-=( float_t& lhs, const FixedPoint<bits, mulP, divP>& rhs)		{ lhs = (FixedPoint<bits, mulP, divP>)lhs - rhs; return lhs; }
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline float_t& operator*=( float_t& lhs, const FixedPoint<bits, mulP, divP>& rhs)		{ lhs = (FixedPoint<bits, mulP, divP>)lhs * rhs; return lhs; }
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline float_t& operator/=( float_t& lhs, const FixedPoint<bits, mulP, divP>& rhs)		{ lhs = (FixedPoint<bits, mulP, divP>)lhs / rhs; return lhs; }

	// double_t
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline double_t& operator+=( double_t& lhs, const FixedPoint<bits, mulP, divP>& rhs)		{ lhs = (FixedPoint<bits, mulP, divP>)lhs + rhs; return lhs; }
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline double_t& operator-=( double_t& lhs, const FixedPoint<bits, mulP, divP>& rhs)		{ lhs = (FixedPoint<bits, mulP, divP>)lhs - rhs; return lhs; }
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline double_t& operator*=( double_t& lhs, const FixedPoint<bits, mulP, divP>& rhs)		{ lhs = (FixedPoint<bits, mulP, divP>)lhs * rhs; return lhs; }
	template<int32_t bits, template <int32_t> class mulP, template <int32_t> class divP>	inline double_t& operator/=( double_t& lhs, const FixedPoint<bits, mulP, divP>& rhs)		{ lhs = (FixedPoint<bits, mulP, divP>)lhs / rhs; return lhs; }



	// local helpers
	inline int32_t neg( int32_t r )		{ return (r >> 31); }


	///////////////////////////////////////////////////////////////////////
	// High-precision policies (int64_t)
	///////////////////////////////////////////////////////////////////////

	template<int32_t bits>
	class HighPrecision : public FixedPointInfo<bits>
	{
		public:

			inline static int32_t mul( int32_t l, int32_t r )
			{
				int64_t t = int64_t(l) * int64_t(r);
				t += static_cast<int32_t>( ROUND );
				t >>= bits;
				return int32_t(t);

/*
				int64_t tmp32_32;
				int32_t res16_16;
				tmp32_32 = l;
				tmp32_32 *= r;
				// result is now 32:32
				tmp32_32 >>= 16; // chop off the lower 16 bits
				res16_16 = ( int ) tmp32_32; // chop off the upper 16bits.
				// result is now back at 16:16
				return res16_16;
*/
			}

			inline static int32_t div( int32_t l, int32_t r )
			{
				int64_t t = int64_t(l) << bits;
				int32_t q = int32_t( t / r );
				int32_t rem = int32_t( t % r );
				q += 1 + neg( (int32_t)( (rem << 1 ) - r ) );
				return q;
			}

/*
			inline static int32_t mul( int32_t l, int32_t r )

			{
				int64_t t = int64_t(l) * int64_t(r);
				t += static_cast<int32_t>( ROUND );
				t >>= bits;
				return int32_t(t);
			}

			inline static int32_t div( int32_t l, int32_t r )
			{
				int64_t t = int64_t(l) << bits;
				t /= r;
				return int32_t(t);
			}
*/
	};


	///////////////////////////////////////////////////////////////////////
	// Mid-precision policies (int32_t)		*UNSAFE*
	///////////////////////////////////////////////////////////////////////

	// at now this is UNSAFE if going to overflow
	template<int32_t bits>
	class MidPrecision : public FixedPointInfo<bits>
	{
		public:

			inline static int32_t mul( int32_t l, int32_t r )
			{
				register unsigned int a,b;
				register bool sign;

				int32_t a1 = l, b1 = r;

				sign = (a1 ^ b1) < 0;
				if (a1 < 0) {a1 = -a1;}
				if (b1 < 0) {b1 = -b1;}
				// a and b contain integer part
				// a1 and b1 contain fractional part.
				a = (((unsigned int)a1) >> bits); a1 &= ~(a << bits);
				b = (((unsigned int)b1) >> bits); b1 &= ~(b << bits);
				a1 = ((a*b) << bits) + (a*b1 + b*a1) +
					((unsigned int)((a1*b1) + (1 << (bits-1))) >> bits);

				if (a1 < 0) {a1 ^= static_cast<int32_t>( SIGN_BIT );}
				if (sign) {a1 = -a1;}
				return a1;
			}

			inline static int32_t div( int32_t l, int32_t r )
			{
				register int res, mask;
				register bool sign;

				int32_t a = l, b = r;

				sign = (a ^ b) < 0;
				if (a < 0) {a = -a;}
				if (b < 0) {b = -b;}
				mask = (1<<bits); res = 0;
				while (a > b) {b <<= 1; mask <<= 1;}
				//while (mask != 0)
				while (mask > 0)
				{
					if (a >= b) {res |= mask; a -= b;}
					mask >>= 1; b >>= 1;
				}

				if (res < 0) {res ^= static_cast<int32_t>( SIGN_BIT );}
				if (sign) {res = -res;}
				return res;
			}

	};


	///////////////////////////////////////////////////////////////////////
	// Low-precision policies (int32_t)
	///////////////////////////////////////////////////////////////////////

	template<int32_t bits>
	class LowPrecision : public FixedPointInfo<bits>
	{
		public:

			inline static int32_t mul( int32_t l, int32_t r )
			{
				return ( ( l * r ) >> bits );
			}

			inline static int32_t div( int32_t l, int32_t r )
			{
				return( ( l << bits ) / r );
			}

	};


	// Predefines
	typedef FixedPoint<  8, LowPrecision,  LowPrecision  > fixed8_t;
	typedef FixedPoint< 16, HighPrecision, HighPrecision > fixed16_t;
	typedef fixed16_t fixed_t;

}	// end of namespace fastmath


#endif	// FIXEDPOINT_H