#include "Eddy_Particle.h"

namespace PLife
{
	void Eddy_Particle::Update(float deltaTime)
	{
		// Update the particles in the lifeform.
		// For example, you can update their position based on velocity and acceleration.
		for (auto& particle : m_Particles)
		{
			// Update velocity based on acceleration
			particle.Velocity.x += particle.Acceleration.x * deltaTime;
			particle.Velocity.y += particle.Acceleration.y * deltaTime;
			// Update position based on velocity
			particle.Position.x += particle.Velocity.x * deltaTime;
			particle.Position.y += particle.Velocity.y * deltaTime;
			// Optionally, you can add some logic to change the color or size over time.
			// For example, fade out the color as the particle ages.
			// This is just a placeholder; you can implement your own logic.
			particle.Color.a = static_cast<unsigned char>(std::max(0.0f, particle.Color.a - 50.0f * deltaTime));
		}
	}
}	