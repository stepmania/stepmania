#ifndef RAGE_MATRIX_HPP_
#define RAGE_MATRIX_HPP_

#include <array>

namespace Rage
{
	/**
	 * @brief A matrix often used with graphical rendering.
	 * 
	 * RageMatrix elements are specified in row-major order.  This
	 * means that the translate terms are located in the fourth row and the
	 * projection terms in the fourth column.  This is consistent with the way
	 * MAX, Direct3D, and OpenGL all handle matrices.  Even though the OpenGL
	 * documentation is in column-major form, the OpenGL code is designed to
	 * handle matrix operations in row-major form.
	 */
	struct Matrix
	{
		Matrix();
		Matrix(Matrix const &rhs);
		Matrix(float v00, float v01, float v02, float v03, float v10, float v11, float v12, float v13, float v20, float v21, float v22, float v23, float v30, float v31, float v32, float v33);
		
		// Allow easier access since overloading [][] is not trivial.
		float& operator() (int row, int col);
		float operator() (int row, int col) const;
		
		// casting operators needed for now due to graphic issues.
		// Kept inside the class for now due to confusion on how to define in cpp
		operator float * ()
		{
			return m[0];
		}
		operator float const * () const
		{
			return m[0];
		}
		
		Matrix GetTranspose() const;

		/** @brief Get the identity matrix. */
		static Matrix GetIdentity();

		/** @brief Get the translation matrix. */
		static Matrix GetTranslation(float x, float y, float z);

		/** @brief Get the scaling matrix. */
		static Matrix GetScaling(float x, float y, float z);

		/** @brief Get the skewed X matrix. */
		static Matrix GetSkewX(float x);

		/** @brief Get the skewed Y matrix. */
		static Matrix GetSkewY(float y);

		// It is preferable to use the std::array syntax, but switch for temporary backwards compatibility.
		//std::array<std::array<float, 4>, 4> m;
		float m[4][4];
	};
	
	inline bool operator==(Matrix const &lhs, Matrix const &rhs)
	{
		for (auto i = 0; i < 4; ++i)
		{
			for (auto j = 0; j < 4; ++j)
			{
				if (lhs(i, j) != rhs(i, j))
				{
					return false;
				}
			}
		}
		
		return true;
	}
	
	inline bool operator!=(Matrix const &lhs, Matrix const &rhs)
	{
		return !operator==(lhs, rhs);
	}
}

#endif
