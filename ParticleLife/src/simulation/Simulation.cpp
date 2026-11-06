#include "Simulation.h"
#include "world/world.h"
#include "graphic/Renderer.h"

#include <algorithm>
#include <cmath>

namespace PLife
{
    // Wrap a coordinate into [0, size).
    static float WrapCoord(float value, float size)
    {
        value = std::fmod(value, size);
        if (value < 0.0f)
            value += size;
        return value >= size ? 0.0f : value; // fmod can round to exactly 'size'
    }

    // Shortest signed distance on a ring of length 'size'.
    static float WrapDelta(float delta, float size)
    {
        return delta - size * std::round(delta / size);
    }

    // Distinct neighbor indices (center, -1, +1) on a ring of 'count' cells.
    static int NeighborIndices(int center, int count, int out[3])
    {
        int n = 0;
        for (int offset = -1; offset <= 1; ++offset)
        {
            const int index = ((center + offset) % count + count) % count;
            if (std::find(out, out + n, index) == out + n)
                out[n++] = index;
        }
        return n;
    }

    Simulation::Simulation(const World& world, const SimulationSettings& settings)
        : m_World(world), m_Settings(settings), m_Rng(std::random_device{}())
    {
        m_Settings.TypeCount = std::clamp(m_Settings.TypeCount, 1, 255);

        m_TypeColors.resize(m_Settings.TypeCount);
        for (int i = 0; i < m_Settings.TypeCount; ++i)
            m_TypeColors[i] = ColorFromHSV(360.0f * i / m_Settings.TypeCount, 0.75f, 1.0f);

        m_Rules.resize(m_Settings.TypeCount * m_Settings.TypeCount, 0.0f);

        // Grid: as many cells as fit while staying at least InteractionRadius wide.
        m_CellsX = std::max(1, (int)(m_World.GetWidth() / m_Settings.InteractionRadius));
        m_CellsY = std::max(1, (int)(m_World.GetHeight() / m_Settings.InteractionRadius));
        m_CellWidth = m_World.GetWidth() / m_CellsX;
        m_CellHeight = m_World.GetHeight() / m_CellsY;

        RandomizeRules();
        RespawnParticles();
    }

    void Simulation::RandomizeRules()
    {
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        for (float& rule : m_Rules)
            rule = dist(m_Rng);
    }

    void Simulation::RespawnParticles()
    {
        m_Particles.clear();
        m_Particles.reserve(m_Settings.ParticleCount);

        std::uniform_real_distribution<float> x(0.0f, m_World.GetWidth());
        std::uniform_real_distribution<float> y(0.0f, m_World.GetHeight());
        std::uniform_int_distribution<int> type(0, m_Settings.TypeCount - 1);
        for (int i = 0; i < m_Settings.ParticleCount; ++i)
            Spawn({ x(m_Rng), y(m_Rng) }, (uint8_t)type(m_Rng));
    }

    void Simulation::Spawn(Vector2 position, uint8_t type)
    {
        position.x = WrapCoord(position.x, m_World.GetWidth());
        position.y = WrapCoord(position.y, m_World.GetHeight());
        type = (uint8_t)std::min<int>(type, m_Settings.TypeCount - 1);
        m_Particles.push_back({ position, { 0.0f, 0.0f }, type });
    }

    float Simulation::Force(float x, float attraction) const
    {
        const float beta = m_Settings.RepulsionRange;
        if (x < beta)
            return x / beta - 1.0f;                                                // always repel up close
        if (x < 1.0f)
            return attraction * (1.0f - std::abs(2.0f * x - 1.0f - beta) / (1.0f - beta)); // rule-driven bump
        return 0.0f;
    }

    void Simulation::BuildGrid()
    {
        // Counting sort of particle indices by cell.
        const int cellCount = m_CellsX * m_CellsY;
        m_CellStart.assign(cellCount + 1, 0);
        m_ParticleCell.resize(m_Particles.size());

        for (size_t i = 0; i < m_Particles.size(); ++i)
        {
            const Vector2 p = m_Particles[i].Position;
            const int cx = std::min((int)(p.x / m_CellWidth), m_CellsX - 1);
            const int cy = std::min((int)(p.y / m_CellHeight), m_CellsY - 1);
            const int cell = cy * m_CellsX + cx;
            m_ParticleCell[i] = cell;
            ++m_CellStart[cell + 1];
        }
        for (int c = 0; c < cellCount; ++c)
            m_CellStart[c + 1] += m_CellStart[c];

        m_CellParticles.resize(m_Particles.size());
        std::vector<int> fill(m_CellStart.begin(), m_CellStart.end() - 1);
        for (size_t i = 0; i < m_Particles.size(); ++i)
            m_CellParticles[fill[m_ParticleCell[i]]++] = (int)i;
    }

    void Simulation::Step(float dt)
    {
        const float width = m_World.GetWidth();
        const float height = m_World.GetHeight();
        const float rMax = m_Settings.InteractionRadius;
        const float rMaxSq = rMax * rMax;
        const int typeCount = m_Settings.TypeCount;

        BuildGrid();

        // 1) Forces. Reads positions only, so the order of particles doesn't matter.
        m_Accelerations.assign(m_Particles.size(), { 0.0f, 0.0f });
        for (size_t i = 0; i < m_Particles.size(); ++i)
        {
            const Particle& a = m_Particles[i];
            const float* rules = &m_Rules[a.Type * typeCount];
            const int cell = m_ParticleCell[i];

            int xs[3], ys[3];
            const int nx = NeighborIndices(cell % m_CellsX, m_CellsX, xs);
            const int ny = NeighborIndices(cell / m_CellsX, m_CellsY, ys);

            Vector2 sum = { 0.0f, 0.0f };
            for (int iy = 0; iy < ny; ++iy)
            {
                for (int ix = 0; ix < nx; ++ix)
                {
                    const int neighborCell = ys[iy] * m_CellsX + xs[ix];
                    for (int k = m_CellStart[neighborCell]; k < m_CellStart[neighborCell + 1]; ++k)
                    {
                        const int j = m_CellParticles[k];
                        if (j == (int)i)
                            continue;

                        const Particle& b = m_Particles[j];
                        const float dx = WrapDelta(b.Position.x - a.Position.x, width);
                        const float dy = WrapDelta(b.Position.y - a.Position.y, height);
                        const float distSq = dx * dx + dy * dy;
                        if (distSq <= 0.0f || distSq >= rMaxSq)
                            continue;

                        const float dist = std::sqrt(distSq);
                        const float f = Force(dist / rMax, rules[b.Type]);
                        sum.x += dx / dist * f;
                        sum.y += dy / dist * f;
                    }
                }
            }
            m_Accelerations[i] = { sum.x * rMax * m_Settings.ForceFactor, sum.y * rMax * m_Settings.ForceFactor };
        }

        // 2) Integrate with frame-rate independent friction, then wrap around the edges.
        const float friction = std::pow(0.5f, dt / m_Settings.FrictionHalfLife);
        for (size_t i = 0; i < m_Particles.size(); ++i)
        {
            Particle& p = m_Particles[i];
            p.Velocity.x = p.Velocity.x * friction + m_Accelerations[i].x * dt;
            p.Velocity.y = p.Velocity.y * friction + m_Accelerations[i].y * dt;
            p.Position.x = WrapCoord(p.Position.x + p.Velocity.x * dt, width);
            p.Position.y = WrapCoord(p.Position.y + p.Velocity.y * dt, height);
        }
    }

    void Simulation::Render(Renderer& renderer) const
    {
        for (const Particle& p : m_Particles)
            renderer.DrawCircle(p.Position, m_Settings.ParticleRadius, m_TypeColors[p.Type]);
    }
}
