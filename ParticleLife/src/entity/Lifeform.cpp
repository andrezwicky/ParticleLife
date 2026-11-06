#include <raylib.h>

#include "LifeForm.h"

namespace PLife
{
	void LifeForm::Render()
	{
		for (const auto& particle : m_Particles)
		{
			DrawCircleV(particle.Position, particle.Size, particle.Color);
		}
	}
	void LifeForm::EmplaceParticle(const Particle& particle)
	{
		m_Particles.emplace_back(particle);
	}
}	