project "ParticleLife"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++23"
   warnings "Extra" -- MSVC /W4
   staticruntime "off"

   files
   { 
      "src/**.h", 
      "src/**.cpp",
   }


   includedirs
   {
      "src",
      "../vendor/raylib/include",
      "../vendor/ImGui",
      "../vendor/rlImGui"
   }

   links
   {
      "raylib",
      "rlImGui"
   }

   targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "PLATFORM_WINDOWS", }
      links
      {
         "winmm"
      }

   filter "configurations:Debug"
      defines { "DEBUG" }
      libdirs { "../vendor/lib/Debug" }
      linkoptions { "/ignore:4099" } -- Debug raylib.lib ships without glfw.pdb
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "RELEASE" }
      libdirs { "../vendor/lib/Release" }
      runtime "Release"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      kind "WindowedApp"
      defines { "DIST" }
      libdirs { "../vendor/lib/Release" }
      runtime "Release"
      optimize "On"
      symbols "Off"
      entrypoint "mainCRTStartup"
