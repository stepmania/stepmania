#ifndef RAGE_MODEL_VERTEX_HPP_
#define RAGE_MODEL_VERTEX_HPP_

#include <cstdint>
#include "RageVector2.hpp"
#include "RageVector3.hpp"

namespace Rage
{
	/** @brief A model that does not have color, but instead relies on a material color. */
	struct ModelVertex
	{
		ModelVertex();
		
		ModelVertex(Vector3 const &pos, Vector3 const &normal, Vector2 const &coords, int8_t b, Vector2 const &scale);
		
		/** @brief The position. */
		Vector3 p;
		/** @brief The normal. */
		Vector3 n;
		/** @brief The texture coordinates. */
		Vector2 t;
		/** @brief The bone. */
		int8_t bone;
		/** @brief The texture matrix scale. Normally 1, 1 by default. */
		Vector2 TextureMatrixScale;
	};
	
	inline bool operator==(ModelVertex const &lhs, ModelVertex const &rhs)
	{
		return
			lhs.p == rhs.p &&
			lhs.n == rhs.n &&
			lhs.t == rhs.t &&
			lhs.bone == rhs.bone &&
			lhs.TextureMatrixScale == rhs.TextureMatrixScale;
	}
	
	inline bool operator!=(ModelVertex const &lhs, ModelVertex const &rhs)
	{
		return !operator==(lhs, rhs);
	}
}

#endif
