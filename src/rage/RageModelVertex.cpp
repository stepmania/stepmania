#include "RageModelVertex.hpp"

using namespace Rage;

ModelVertex::ModelVertex(): p(), n(), t(), bone(0), TextureMatrixScale({1, 1})
{
}

ModelVertex::ModelVertex(Vector3 const &pos, Vector3 const &normal, Vector2 const &coords, int8_t b, Vector2 const &scale): p(pos), n(normal), t(coords), bone(b), TextureMatrixScale(scale)
{
}
