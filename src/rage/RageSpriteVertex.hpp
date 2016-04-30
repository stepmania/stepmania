#ifndef RAGE_SPRITE_VERTEX_HPP_
#define RAGE_SPRITE_VERTEX_HPP_

#include "RageVColor.hpp"
#include "RageVector2.hpp"
#include "RageVector3.hpp"

namespace Rage
{
	/* Structure for our custom vertex type.  Note that these data structes
	 * have the same layout that D3D expects. */
	struct SpriteVertex
	{
		SpriteVertex();
		SpriteVertex(Vector3 const &pos, Vector3 const &normal, VColor const &color, Vector2 const &coords);
		
		/** @brief The position */
		Vector3 p;
		/** @brief The normal */
		Vector3 n;
		/** @brief The diffuse color */
		VColor c;
		/** @brief The texture coordinates */
		Vector2 t;
	};
	
	inline bool operator==(SpriteVertex const &lhs, SpriteVertex const &rhs)
	{
		return
			lhs.p == rhs.p &&
			lhs.n == rhs.n &&
			lhs.c == rhs.c &&
			lhs.t == rhs.t;
	}
	
	inline bool operator!=(SpriteVertex const &lhs, SpriteVertex const &rhs)
	{
		return !operator==(lhs, rhs);
	}
}

#endif
