#include "RageSpriteVertex.hpp"

using namespace Rage;

SpriteVertex::SpriteVertex(): p(), n(), c(), t()
{
}

SpriteVertex::SpriteVertex(Vector3 const &pos, Vector3 const &normal, VColor const &color, Vector2 const &coords): p(pos), n(normal), c(color), t(coords)
{
}
