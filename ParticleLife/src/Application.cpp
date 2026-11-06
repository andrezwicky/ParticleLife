#include "Application.h"
#include "raylib.h"

namespace PLife
{
	static Application* s_Instance = nullptr;
	
    Application::Application(const ApplicationSpecification& spec)
		: m_Specification(spec)
    {
        s_Instance = this;
    }
    Application::~Application()
    {
        s_Instance = nullptr;
    }
    Application& Application::Get()
    {
        return *s_Instance;
    }
    void Application::Loop()
    {
        m_Time += GetFrameTime();
        OnUpdate();

        BeginDrawing();
        ClearBackground(BLACK);
        OnRender();
        EndDrawing();
    }
    void Application::Run()
    {
        SetTraceLogLevel(LOG_INFO); // lower to LOG_DEBUG for verbose raylib output
        // No FLAG_WINDOW_HIGHDPI: in raylib 5.6-dev it corrupts the screen/render size after a
        // resize on Windows. Instead we work in physical pixels and scale the initial size by DPI.
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT); // MSAA: anti-aliased shape edges
        InitWindow(m_Specification.Width, m_Specification.Height, m_Specification.Title.c_str());
        {
            const Vector2 dpi = GetWindowScaleDPI();
            if (dpi.x > 1.0f || dpi.y > 1.0f)
            {
                const int monitor = GetCurrentMonitor();
                const int width = (int)(GetScreenWidth() * dpi.x);
                const int height = (int)(GetScreenHeight() * dpi.y);
                SetWindowSize(width, height);
                SetWindowPosition((GetMonitorWidth(monitor) - width) / 2, (GetMonitorHeight(monitor) - height) / 2);
            }
        }
        SetExitKey(0); // ESC does not close the window
        SetTargetFPS(60);

        m_Game = std::make_unique<Game>(); // after InitWindow: Game may load GPU resources

        while (!WindowShouldClose())    // Detect window close button
        {
            Loop();
        }

        m_Game.reset(); // before CloseWindow: Game frees GPU resources while the context still exists
        CloseWindow();
    }
    void Application::OnUpdate()
    {
        m_Game->OnUpdate(GetFrameTime());
    }

    void Application::OnRender()
    {
        m_Game->OnRender();
    }
}
