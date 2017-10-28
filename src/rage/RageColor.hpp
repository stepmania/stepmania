#ifndef RAGE_COLOR_HPP_
#define RAGE_COLOR_HPP_

#include <string>
#include <ostream>

namespace Rage
{
	struct Color
	{
		Color();
		
		Color(float red, float green, float blue, float alpha);
		
		// assignment operators
		Color& operator += ( Color const &rhs );
		Color& operator -= ( Color const &rhs );
		Color& operator *= ( Color const &rhs );
		Color& operator *= ( float rhs );
		// Division is rarely useful, but set it up anyway. It's only proper.
		Color& operator /= ( float rhs );
		
		bool FromString ( std::string const &colorString );
		
		std::string ToString() const;
		static std::string NormalizeColorString( std::string color );
		
	private:
		bool FromCommaString( std::string const &colorString );
		bool FromHexString( std::string const &colorString );

	public:
		
		float r;
		float g;
		float b;
		float a;
	};
	
	inline bool operator==(Color const &lhs, Color const &rhs)
	{
		return
			lhs.r == rhs.r &&
			lhs.g == rhs.g &&
			lhs.b == rhs.b &&
			lhs.a == rhs.a;
	}
	
	inline bool operator!=(Color const &lhs, Color const &rhs)
	{
		return !operator==(lhs, rhs);
	}
	
	inline Color operator+(Color lhs, Color const &rhs)
	{
		lhs += rhs;
		return lhs;
	}
	
	inline Color operator-(Color lhs, Color const &rhs)
	{
		lhs -= rhs;
		return lhs;
	}
	
	inline Color operator*(Color lhs, Color const &rhs)
	{
		lhs *= rhs;
		return lhs;
	}
	
	inline Color operator*(Color lhs, float rhs)
	{
		lhs *= rhs;
		return lhs;
	}
	
	inline Color operator/(Color lhs, float rhs)
	{
		lhs /= rhs;
		return lhs;
	}
	
	inline std::ostream& operator<<(std::ostream &os, Color const &rhs)
	{
		os << rhs.ToString();
		return os;
	}
}

#endif
