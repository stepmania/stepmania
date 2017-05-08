#ifndef RAGE_UTIL_HPP_
#define RAGE_UTIL_HPP_

namespace Rage
{
	/** @brief Safely delete array pointers. */
	template<typename T>
	void safe_delete_array( T*& p )
	{
		if (p != nullptr)
		{
			delete[] p;
		}
		p = nullptr;
	}

	/** @brief Safely delete pointers. */
	template<typename T>
	void safe_delete( T*& p )
	{
		if (p != nullptr)
		{
			delete p;
		}
		p = nullptr;
	}
	
}

#endif
