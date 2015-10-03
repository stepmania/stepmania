#ifndef RAGE_RECT_HPP_
#define RAGE_RECT_HPP_

namespace Rage
{
	template <typename T>
	class RectBase
	{
	protected:
		RectBase(): left(0), top(0), right(0), bottom(0)
		{
		}
		
		RectBase(T l, T t, T r, T b): left(l), top(t), right(r), bottom(b)
		{
		}
	public:
		T GetWidth() const
		{
			return right - left;
		}
		
		T GetHeight() const
		{
			return bottom - top;
		}
		
		T GetCenterX() const
		{
			return (left + right) / 2;
		}
		
		T GetCenterY() const
		{
			return (top + bottom) / 2;
		}
		
		T left;
		T top;
		T right;
		T bottom;
	};
	
	template <typename T>
	inline bool operator==(RectBase<T> const &lhs, RectBase<T> const &rhs)
	{
		return
		lhs.left == rhs.left &&
		lhs.top == rhs.top &&
		lhs.right == rhs.right &&
		lhs.bottom == rhs.bottom;
	}
	
	template <typename T>
	inline bool operator!=(RectBase<T> const &lhs, RectBase <T>const &rhs)
	{
		return !operator==(lhs, rhs);
	}
	
	class RectI : public RectBase<int>
	{
	public:
		RectI(): RectBase<int>(0, 0, 0, 0)
		{
		}
		
		RectI(int l, int t, int r, int b): RectBase<int>(l, t, r, b)
		{
		}
	};
	
	class RectF : public RectBase<float>
	{
	public:
		RectF(): RectBase<float>(0.f, 0.f, 0.f, 0.f)
		{
		}
		
		RectF(float l, float t, float r, float b): RectBase<float>(l, t, r, b)
		{
		}
	};
}

#endif
