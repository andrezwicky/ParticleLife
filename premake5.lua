workspace "ParticleLife"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "ParticleLife"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "vendor/rlImGui"
include "ParticleLife"