#include "RageMatrix.hpp"
#include <cstring>

using namespace Rage;

Matrix::Matrix()
{
	memset(m, 0, sizeof(float)*4*4);
}

Matrix::Matrix(Matrix const &rhs)
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			m[j][i] = rhs.m[j][i];
		}
	}
}

Matrix::Matrix(float v00, float v01, float v02, float v03, float v10, float v11, float v12, float v13, float v20, float v21, float v22, float v23, float v30, float v31, float v32, float v33)
{
	m[0][0] = v00;
	m[0][1] = v01;
	m[0][2] = v02;
	m[0][3] = v03;
	m[1][0] = v10;
	m[1][1] = v11;
	m[1][2] = v12;
	m[1][3] = v13;
	m[2][0] = v20;
	m[2][1] = v21;
	m[2][2] = v22;
	m[2][3] = v23;
	m[3][0] = v30;
	m[3][1] = v31;
	m[3][2] = v32;
	m[3][3] = v33;
}

float & Matrix::operator()(int row, int col)
{
	return m[col][row];
}

float Matrix::operator()(int row, int col) const
{
	return m[col][row];
}

Matrix Matrix::GetTranspose() const
{
	return Matrix{ m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1], m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3] };
}

Matrix Matrix::GetIdentity()
{
	return Matrix
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
}

Matrix Matrix::GetTranslation(float x, float y, float z)
{
	return Matrix
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		x, y, z, 1
	};
}

Matrix Matrix::GetScaling(float x, float y, float z)
{
	return Matrix
	{
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1
	};
}

Matrix Matrix::GetSkewX(float x)
{
	return Matrix
	{
		1, 0, 0, 0,
		x, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
}

Matrix Matrix::GetSkewY(float y)
{
	return Matrix
	{
		1, y, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
}
