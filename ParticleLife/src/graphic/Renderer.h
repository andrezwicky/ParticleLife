#pragma once

#include "raylib.h"

namespace PLife
{
    class World;

    // Renders at full window resolution. Draw calls take world coordinates;
    // a Camera2D fits the world into the window (centered, aspect preserved)
    // and supports zooming and panning on top of that.
    // Keeps a reference to the world: the world must outlive the renderer.
    class Renderer
    {
    public:
        explicit Renderer(const World& world);

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        // World-space drawing goes between these. Must be called inside
        // raylib's BeginDrawing()/EndDrawing() (Application does that).
        void BeginFrame();
        void EndFrame();

        // World space (only between BeginFrame/EndFrame).
        void DrawCircle(Vector2 center, float radius, Color color);
        void DrawRect(Rectangle rect, Color color);
        void DrawLine(Vector2 start, Vector2 end, float thickness, Color color);
        void DrawText(const char* text, Vector2 position, float fontSize, Color color);

        // Screen space, for overlays (only after EndFrame). Logical (DPI-independent) pixels.
        void DrawScreenText(const char* text, float x, float y, float fontSize, Color color);
        // Window size in logical pixels, matching DrawScreenText's coordinates.
        Vector2 GetScreenSize() const;

        // View control. 'screenPosition' is a window position, e.g. GetMousePosition().
        void ZoomAt(Vector2 screenPosition, float factor);
        void Pan(Vector2 screenDelta);
        void ResetView();

        Vector2 ScreenToWorld(Vector2 screenPosition) const;
        Vector2 WorldToScreen(Vector2 worldPosition) const;

    private:
        // Built from the current window and world size each time, so resizing just works.
        Camera2D GetCamera() const;

    private:
        const World& m_World;

        // View state on top of "fit world to window": 1 = whole world visible.
        float m_Zoom = 1.0f;
        Vector2 m_ViewCenter; // world position shown at the window center
    };
}
