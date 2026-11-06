#include "Game.h"
#include "graphic/Renderer.h"

#include <cmath>

#include "imgui.h"

namespace PLife
{
    static constexpr float s_StepTime = 1.0f / 60.0f;
    static constexpr int s_MaxStepsPerFrame = 4; // drop time instead of spiraling when a frame is slow

    Game::Game()
        : m_World(1600.0f, 1000.0f), m_Simulation(m_World), m_Renderer(std::make_unique<Renderer>(m_World))
    {
    }

    Game::~Game()
    {
    }

    void Game::OnUpdate(float deltaTime)
    {
        HandleViewInput();
        HandleSimulationInput();

        if (m_Paused)
            return;

        m_StepAccumulator += deltaTime;
        int steps = 0;
        while (m_StepAccumulator >= s_StepTime && steps < s_MaxStepsPerFrame)
        {
            m_Simulation.Step(s_StepTime);
            m_StepAccumulator -= s_StepTime;
            ++steps;
        }
        if (steps == s_MaxStepsPerFrame)
            m_StepAccumulator = 0.0f;
    }

    void Game::OnRender()
    {
        m_Renderer->BeginFrame();

        m_World.Render(*m_Renderer);
        m_Simulation.Render(*m_Renderer);

        m_Renderer->EndFrame();

        const float bottom = m_Renderer->GetScreenSize().y;
        m_Renderer->DrawScreenText(TextFormat("%d particles%s", (int)m_Simulation.GetParticles().size(), m_Paused ? "   PAUSED" : ""),
            10.0f, 10.0f, 18.0f, RAYWHITE);
        m_Renderer->DrawScreenText("Wheel: zoom   Right drag: pan   R: reset view   Space: pause   S: single step   N: new rules   P: respawn   Tab: panel",
            10.0f, bottom - 26.0f, 16.0f, GRAY);
    }

    void Game::OnImGuiRender()
    {
        if (m_ShowPanel)
            DrawSimulationPanel();
    }

    // Background for a rule cell: red for repulsion, green for attraction, dark at 0.
    static ImVec4 RuleColor(float value)
    {
        const float t = std::abs(value);
        return value >= 0.0f
            ? ImVec4(0.12f, 0.12f + 0.55f * t, 0.12f, 1.0f)
            : ImVec4(0.12f + 0.65f * t, 0.12f, 0.12f, 1.0f);
    }

    static ImVec4 ToImVec4(Color c)
    {
        return ImVec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
    }

    void Game::DrawSimulationPanel()
    {
        const float dpi = GetWindowScaleDPI().x;
        ImGui::SetNextWindowPos(ImVec2(10.0f * dpi, 40.0f * dpi), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Simulation", &m_ShowPanel, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::End();
            return;
        }

        ImGui::PushItemWidth(ImGui::GetFontSize() * 14.0f);
        if (ImGui::CollapsingHeader("Rules", ImGuiTreeNodeFlags_DefaultOpen))
            DrawRulesSection();
        if (ImGui::CollapsingHeader("Type forces", ImGuiTreeNodeFlags_DefaultOpen))
            DrawTypeForcesSection();

        // Every setting edits a copy; any change is applied at once (SetSettings handles resizing).
        SimulationSettings settings = m_Simulation.GetSettings();
        bool changed = false;

        if (ImGui::CollapsingHeader("Temperature", ImGuiTreeNodeFlags_DefaultOpen))
        {
            changed |= ImGui::SliderFloat("Temperature##Value", &settings.Temperature, 0.0f, 10.0f, "%.2f");
            changed |= ImGui::DragFloatRange2("Eddy diffusion", &settings.EddyDiffusionMin, &settings.EddyDiffusionMax,
                0.1f, 0.0f, 200.0f, "min %.1f", "max %.1f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::TextDisabled("Each particle gets a random eddy diffusion factor from this range.");
            ImGui::TextDisabled("Random motion per step: sqrt(2 * temperature * factor * dt).");
            if (ImGui::Button("Reroll eddy factors"))
                m_Simulation.RerollEddyDiffusion();
        }

        if (ImGui::CollapsingHeader("Settings"))
        {
            ImGui::SeparatorText("Particles");
            changed |= ImGui::SliderInt("Particles", &settings.ParticleCount, 0, 10000, "%d", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::SliderInt("Types", &settings.TypeCount, 1, 12, "%d", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::SliderFloat("Particle radius", &settings.ParticleRadius, 0.5f, 10.0f, "%.1f");

            ImGui::SeparatorText("Forces");
            changed |= ImGui::SliderFloat("Force factor", &settings.ForceFactor, 0.0f, 50.0f, "%.1f");
            changed |= ImGui::SliderFloat("Friction half-life", &settings.FrictionHalfLife, 0.005f, 1.0f, "%.3f s",
                ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);

            ImGui::Spacing();
            if (ImGui::Button("Respawn particles"))
                m_Simulation.RespawnParticles();
            ImGui::SameLine();
            if (ImGui::Button("Reset settings"))
            {
                settings = SimulationSettings();
                changed = true;
                m_Simulation.ResetTypeSettings();
            }
        }
        ImGui::PopItemWidth();

        if (changed)
            m_Simulation.SetSettings(settings);

        ImGui::End();
    }

    void Game::DrawTypeForcesSection()
    {
        const int typeCount = m_Simulation.GetSettings().TypeCount;
        const ImGuiColorEditFlags swatchFlags = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop;
        const float columnWidth = ImGui::GetFontSize() * 9.0f;

        ImGui::TextDisabled("How each type senses others (as the reacting type).");

        if (ImGui::BeginTable("TypeForces", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX))
        {
            ImGui::TableSetupColumn("##Type");
            ImGui::TableSetupColumn("Interaction radius");
            ImGui::TableSetupColumn("Repulsion range");
            ImGui::TableHeadersRow();

            for (int type = 0; type < typeCount; ++type)
            {
                ImGui::PushID(type);
                TypeSettings ts = m_Simulation.GetTypeSettings(type);
                bool changed = false;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::ColorButton("##Color", ToImVec4(m_Simulation.GetTypeColor(type)), swatchFlags);

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(columnWidth);
                changed |= ImGui::SliderFloat("##Radius", &ts.InteractionRadius, 10.0f, 200.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(columnWidth);
                changed |= ImGui::SliderFloat("##Repulsion", &ts.RepulsionRange, 0.05f, 0.95f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

                if (changed)
                    m_Simulation.SetTypeSettings(type, ts);
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        if (ImGui::Button("Reset type forces"))
            m_Simulation.ResetTypeSettings();
    }

    void Game::DrawRulesSection()
    {
        const int typeCount = m_Simulation.GetSettings().TypeCount;
        const ImGuiColorEditFlags swatchFlags = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop;
        const float cellWidth = ImGui::GetFontSize() * 3.2f;
        const ImVec2 swatchSize(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());

        ImGui::TextDisabled("Row type reacts to column type.");
        ImGui::TextDisabled("Drag to change, Ctrl+click to type, right-click to zero.");

        if (ImGui::BeginTable("RuleMatrix", typeCount + 1, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX))
        {
            // Header row: color of each target type.
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            for (int b = 0; b < typeCount; ++b)
            {
                ImGui::TableNextColumn();
                ImGui::PushID(b);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellWidth - swatchSize.x) * 0.5f);
                ImGui::ColorButton("##col", ToImVec4(m_Simulation.GetTypeColor(b)), swatchFlags, swatchSize);
                ImGui::PopID();
            }

            for (int a = 0; a < typeCount; ++a)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::PushID(a);
                ImGui::ColorButton("##row", ToImVec4(m_Simulation.GetTypeColor(a)), swatchFlags, swatchSize);

                for (int b = 0; b < typeCount; ++b)
                {
                    ImGui::TableNextColumn();
                    ImGui::PushID(b);

                    float value = m_Simulation.GetRule(a, b);
                    const ImVec4 bg = RuleColor(value);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, bg);
                    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(bg.x + 0.1f, bg.y + 0.1f, bg.z + 0.1f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(bg.x + 0.2f, bg.y + 0.2f, bg.z + 0.2f, 1.0f));
                    ImGui::SetNextItemWidth(cellWidth);
                    if (ImGui::DragFloat("##rule", &value, 0.01f, -1.0f, 1.0f, "%+.2f", ImGuiSliderFlags_AlwaysClamp))
                        m_Simulation.SetRule(a, b, value);
                    ImGui::PopStyleColor(3);

                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                        m_Simulation.SetRule(a, b, 0.0f);

                    if (ImGui::BeginItemTooltip())
                    {
                        ImGui::ColorButton("##a", ToImVec4(m_Simulation.GetTypeColor(a)), swatchFlags);
                        ImGui::SameLine();
                        ImGui::TextUnformatted(value > 0.0f ? "is attracted to" : value < 0.0f ? "is repelled by" : "ignores");
                        ImGui::SameLine();
                        ImGui::ColorButton("##b", ToImVec4(m_Simulation.GetTypeColor(b)), swatchFlags);
                        ImGui::SameLine();
                        ImGui::Text("(%+.2f)", value);
                        ImGui::EndTooltip();
                    }

                    ImGui::PopID();
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        if (ImGui::Button("Randomize"))
            m_Simulation.RandomizeRules();
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
            m_Simulation.ClearRules();
    }

    void Game::HandleViewInput()
    {
        if (ImGui::GetIO().WantCaptureMouse) // mouse is over a panel
            return;

        const float wheel = GetMouseWheelMove();
        if (wheel != 0.0f)
            m_Renderer->ZoomAt(GetMousePosition(), std::pow(1.15f, wheel));

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            m_Renderer->Pan(GetMouseDelta());

        if (IsKeyPressed(KEY_R))
            m_Renderer->ResetView();
    }

    void Game::HandleSimulationInput()
    {
        if (ImGui::GetIO().WantCaptureKeyboard) // typing into a panel
            return;

        if (IsKeyPressed(KEY_TAB))
            m_ShowPanel = !m_ShowPanel;
        if (IsKeyPressed(KEY_SPACE))
        {
            m_Paused = !m_Paused;
            m_StepAccumulator = 0.0f;
        }
        if (IsKeyPressed(KEY_S) && m_Paused)
            m_Simulation.Step(s_StepTime);
        if (IsKeyPressed(KEY_N))
            m_Simulation.RandomizeRules();
        if (IsKeyPressed(KEY_P))
            m_Simulation.RespawnParticles();
    }
}
