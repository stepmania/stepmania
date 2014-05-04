/* OptionsBinding - little helpers so that SongOptions and PlayerOptions can more easily have similar interfaces. */

// No .cpp file because it's just some #defines, and not in a makefile because it doesn't need to be.
// DefaultNilArgs would be in here, but then there would need to be a .cpp file, and DefaultNilArgs is more widely useful.

#ifndef OptionsBinding_H
#define OptionsBinding_H
	// Functions are designed to combine Get and Set into one, to be less clumsy to use. -Kyz
	// If a valid arg is passed, the value is set.
	// The previous value is returned.
#define FLOAT_INTERFACE(func_name, member, valid) \
	static int func_name(T* p, lua_State* L) \
	{ \
		DefaultNilArgs(L, 2); \
		lua_pushnumber(L, p->m_f ## member); \
		lua_pushnumber(L, p->m_Speedf ## member); \
		if(lua_isnumber(L, 1)) \
		{ \
			float v= FArg(1); \
			if(!valid) \
			{ \
				luaL_error(L, "Invalid value %f", v); \
			} \
			p->m_f ## member = v; \
			if(p->m_Speedf ## member <= 0.0f) \
			{ \
				p->m_Speedf ## member = 1.0f; \
			} \
		} \
		if(lua_isnumber(L, 2)) \
		{ \
			p->m_Speedf ## member = FArgGTEZero(L, 2); \
		} \
		return 2; \
	}
#define INT_INTERFACE(func_name, member) \
	static int func_name(T* p, lua_State* L) \
	{ \
		DefaultNilArgs(L, 1); \
		lua_pushnumber(L, p->m_i ## member); \
		if(lua_isnumber(L, 1)) \
		{ \
			p->m_i ## member = IArg(1); \
		} \
		return 1; \
	}
#define BOOL_INTERFACE(func_name, member) \
	static int func_name(T* p, lua_State* L) \
	{ \
		DefaultNilArgs(L, 1); \
		lua_pushboolean(L, p->m_b ## member); \
		if(lua_isboolean(L, 1)) \
		{ \
			p->m_b ## member = BArg(1); \
		} \
		return 1; \
	}
#define ENUM_INTERFACE(func_name, member, enum_name) \
	static int func_name(T* p, lua_State* L) \
	{ \
		DefaultNilArgs(L, 1); \
		Enum::Push(L, p->m_ ## member); \
		if(!lua_isnil(L, 1)) \
		{ \
			p->m_ ## member= Enum::Check<enum_name>(L, 1); \
		} \
		return 1; \
	}

#endif

/*
 * (c) 2014 Eric Reese
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
