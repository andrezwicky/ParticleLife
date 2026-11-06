#include "world.h"
#include "graphic/Renderer.h"

namespace PLife
{
    World::World(float width, float height, Color background)
        : m_Width(width), m_Height(height), m_Background(background)
    {
    }

    void World::Render(Renderer& renderer) const
    {
        renderer.DrawRect(GetBounds(), m_Background);
    }
}
