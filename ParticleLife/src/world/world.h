#pragma once

#include "raylib.h"

namespace PLife
{
    class Renderer;

    // The simulation area in world units. Independent of the window size.
    class World
    {
    public:
        World(float width, float height, Color background = { 18, 18, 24, 255 });

        // World space (call between Renderer::BeginFrame/EndFrame).
        void Render(Renderer& renderer) const;

        float GetWidth() const { return m_Width; }
        float GetHeight() const { return m_Height; }
        Vector2 GetSize() const { return { m_Width, m_Height }; }
        Vector2 GetCenter() const { return { m_Width * 0.5f, m_Height * 0.5f }; }
        Rectangle GetBounds() const { return { 0.0f, 0.0f, m_Width, m_Height }; }

    private:
        float m_Width;
        float m_Height;
        Color m_Background;
    };
}
