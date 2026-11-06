#include "Renderer.h"
#include "world/world.h"

#include <algorithm>
#include <cmath>

namespace PLife
{
    static constexpr float s_MinZoom = 0.25f;
    static constexpr float s_MaxZoom = 50.0f;

    Renderer::Renderer(const World& world)
        : m_World(world)
    {
        ResetView();
    }

    void Renderer::BeginFrame()
    {
        BeginMode2D(GetCamera());
    }

    void Renderer::EndFrame()
    {
        EndMode2D();
    }

    void Renderer::DrawCircle(Vector2 center, float radius, Color color)
    {
        // raylib picks the segment count from the radius in world units, which looks
        // polygonal when zoomed in. Pick it from the on-screen radius instead.
        const float screenRadius = radius * GetCamera().zoom;
        int segments = 8;
        if (screenRadius > 1.0f)
        {
            const float maxError = 0.25f; // pixels between true circle and polygon edge
            const float angle = 2.0f * std::acos(std::max(1.0f - maxError / screenRadius, -1.0f));
            segments = std::clamp((int)std::ceil(2.0f * PI / angle), 8, 256);
        }
        ::DrawCircleSector(center, radius, 0.0f, 360.0f, segments, color);
    }

    void Renderer::DrawRect(Rectangle rect, Color color)
    {
        ::DrawRectangleRec(rect, color);
    }

    void Renderer::DrawLine(Vector2 start, Vector2 end, float thickness, Color color)
    {
        ::DrawLineEx(start, end, thickness, color);
    }

    void Renderer::DrawText(const char* text, Vector2 position, float fontSize, Color color)
    {
        ::DrawTextEx(GetFontDefault(), text, position, fontSize, fontSize / 10.0f, color);
    }

    void Renderer::DrawScreenText(const char* text, float x, float y, float fontSize, Color color)
    {
        // Logical pixels -> physical pixels (we don't use FLAG_WINDOW_HIGHDPI, see Application).
        const float dpi = GetWindowScaleDPI().x;
        const float size = fontSize * dpi;
        ::DrawTextEx(GetFontDefault(), text, { x * dpi, y * dpi }, size, size / 10.0f, color);
    }

    Vector2 Renderer::GetScreenSize() const
    {
        const Vector2 dpi = GetWindowScaleDPI();
        return { (float)GetScreenWidth() / dpi.x, (float)GetScreenHeight() / dpi.y };
    }

    void Renderer::ZoomAt(Vector2 screenPosition, float factor)
    {
        // Keep the world point under the cursor fixed while zooming.
        const Vector2 before = ScreenToWorld(screenPosition);
        m_Zoom = std::clamp(m_Zoom * factor, s_MinZoom, s_MaxZoom);
        const Vector2 after = ScreenToWorld(screenPosition);
        m_ViewCenter.x += before.x - after.x;
        m_ViewCenter.y += before.y - after.y;
    }

    void Renderer::Pan(Vector2 screenDelta)
    {
        const float zoom = GetCamera().zoom;
        m_ViewCenter.x -= screenDelta.x / zoom;
        m_ViewCenter.y -= screenDelta.y / zoom;
    }

    void Renderer::ResetView()
    {
        m_Zoom = 1.0f;
        m_ViewCenter = m_World.GetCenter();
    }

    Vector2 Renderer::ScreenToWorld(Vector2 screenPosition) const
    {
        return GetScreenToWorld2D(screenPosition, GetCamera());
    }

    Vector2 Renderer::WorldToScreen(Vector2 worldPosition) const
    {
        return GetWorldToScreen2D(worldPosition, GetCamera());
    }

    Camera2D Renderer::GetCamera() const
    {
        const float screenWidth = (float)GetScreenWidth();
        const float screenHeight = (float)GetScreenHeight();
        const float fitZoom = std::min(screenWidth / m_World.GetWidth(), screenHeight / m_World.GetHeight());

        Camera2D camera{};
        camera.offset = { screenWidth * 0.5f, screenHeight * 0.5f };
        camera.target = m_ViewCenter;
        camera.rotation = 0.0f;
        camera.zoom = fitZoom * m_Zoom;
        return camera;
    }
}
