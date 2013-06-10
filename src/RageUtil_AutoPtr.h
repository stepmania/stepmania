/* AutoPtrCopyOnWrite - Simple smart pointer template. */

#ifndef RAGE_UTIL_AUTO_PTR_H
#define RAGE_UTIL_AUTO_PTR_H

/*
 * This is a simple copy-on-write refcounted smart pointer.  Once constructed, all read-only
 * access to the object is made without extra copying.  If you need read-write access, you
 * can get a pointer with Get(), which will cause the object to deep-copy.  (Don't free
 * the resulting pointer.)
 *
 * Note that there are no non-const operator* or operator-> overloads, because that would
 * cause all const access by code with non-const permissions to deep-copy.  For example,
 *
 *   AutoPtrCopyOnWrite<int> a( new int(1) );
 *   AutoPtrCopyOnWrite<int> b( a );
 *   printf( "%i\n", *a );
 *
 * If we have a non-const operator*, this *a will use it (even though it only needs const
 * access), and will copy the underlying object wastefully.  g++ std::string has this behavior,
 * which is why it's important to qualify strings as "const" when const access is desired,
 * but that's brittle, so let's make all potential deep-copying explicit.
 */

template<class T>
class AutoPtrCopyOnWrite
{
public:
	/* This constructor only exists to make us work with STL containers. */
	inline AutoPtrCopyOnWrite()
	{
		m_pPtr = NULL;
		m_iRefCount = new int(1);
	}

	explicit inline AutoPtrCopyOnWrite( T *p )
	{
		m_pPtr = p;
		m_iRefCount = new int(1);
	}

	inline AutoPtrCopyOnWrite( const AutoPtrCopyOnWrite &rhs )
	{
		m_pPtr = rhs.m_pPtr;
		m_iRefCount = rhs.m_iRefCount;
		++(*m_iRefCount);
	}

	void Swap( AutoPtrCopyOnWrite<T> &rhs )
	{
		swap( m_pPtr, rhs.m_pPtr );
		swap( m_iRefCount, rhs.m_iRefCount );
	}

	inline AutoPtrCopyOnWrite<T> &operator=( const AutoPtrCopyOnWrite &rhs )
	{
		AutoPtrCopyOnWrite<T> obj( rhs );
		this->Swap( obj );
		return *this;
	}

	~AutoPtrCopyOnWrite()
	{
		--(*m_iRefCount);
		if( *m_iRefCount == 0 )
		{
			delete m_pPtr;
			delete m_iRefCount;
		}
	}

	/* Get a non-const pointer.  This will deep-copy the object if necessary. */
	T *Get()
	{
		if( *m_iRefCount > 1 )
		{
			--*m_iRefCount;
			m_pPtr = new T(*m_pPtr);
			m_iRefCount = new int(1);
		}

		return m_pPtr;
	}

	int GetReferenceCount() const { return *m_iRefCount; }

	const T &operator *() const { return *m_pPtr; }
	const T *operator ->() const { return m_pPtr; }

private:
	T *m_pPtr;
	int *m_iRefCount;
};

template<class T>
inline void swap( AutoPtrCopyOnWrite<T> &a, AutoPtrCopyOnWrite<T> &b )
{
	a.Swap(b);
}

/*
 * This smart pointer template is used to safely hide implementations from
 * headers, to reduce dependencies.  This is the same as declaring a pointer
 * to a class, and allocating/deallocating it in the implementation: only
 * the implementation needs to include that class.  This makes copying
 * and deletion automatic, so you don't need to include a copy ctor or
 * remember to delete it.
 *
 * There's one subtlety: in order to copy or delete an object, we need its
 * definition.  This is intended to avoid pulling in the definition.  So,
 * we use a traits class to hide it.  Use REGISTER_CLASS_TRAITS for each
 * class used with this template.
 *
 * Concepts from http://www.gotw.ca/gotw/062.htm.
 */
template<class T>
struct HiddenPtrTraits
{
	static T *Copy( const T *pCopy );
	static void Delete( T *p );
};
#define REGISTER_CLASS_TRAITS(T, CopyExpr) \
	template<> T *HiddenPtrTraits<T>::Copy( const T *pCopy ) { return CopyExpr; } \
	template<> void HiddenPtrTraits<T>::Delete( T *p ) { delete p; }

template<class T>
class HiddenPtr
{
public:
	const T& operator*() const { return *m_pPtr; }
	const T* operator->() const { return m_pPtr; }
	T& operator*() { return *m_pPtr; }
	T* operator->() { return m_pPtr; }

	explicit HiddenPtr( T *p = NULL )
	{
		m_pPtr = p;
	}

	HiddenPtr( const HiddenPtr<T> &cpy )
	{
		if( cpy.m_pPtr == NULL )
			m_pPtr = NULL;
		else
			m_pPtr = HiddenPtrTraits<T>::Copy( cpy.m_pPtr );
	}

#if 0 // broken VC6
	template<class U>
	HiddenPtr( const HiddenPtr<U> &cpy )
	{
		if( cpy.m_pPtr == NULL )
			m_pPtr = NULL;
		else
			m_pPtr = HiddenPtrTraits<U>::Copy( cpy.m_pPtr );
	}
#endif

	~HiddenPtr()
	{
		HiddenPtrTraits<T>::Delete( m_pPtr );
	}
	void Swap( HiddenPtr<T> &rhs ) { swap( m_pPtr, rhs.m_pPtr ); }

	HiddenPtr<T> &operator=( T *p )
	{
		HiddenPtr<T> t( p );
		Swap( t );
		return *this;
	}

	HiddenPtr<T> &operator=( const HiddenPtr &cpy )
	{
		HiddenPtr<T> t( cpy );
		Swap( t );
		return *this;
	}

#if 0 // broken VC6
	template<class U>
	HiddenPtr<T> &operator=( const HiddenPtr<U> &cpy )
	{
		HiddenPtr<T> t( cpy );
		Swap( t );
		return *this;
	}
#endif

private:
	T *m_pPtr;

#if 0 // broken VC6
	template<class U>
	friend class HiddenPtr;
#endif
};

template<class T>
inline void swap( HiddenPtr<T> &a, HiddenPtr<T> &b )
{
	a.Swap(b);
}

#endif

/*
 * (c) 2005 Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
