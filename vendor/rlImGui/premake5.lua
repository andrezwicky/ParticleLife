project "rlImGui"
    kind "StaticLib"
	language "C++"
	cdialect "C99"
	cppdialect "C++17"
    staticruntime "off"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    
	files
	{
        "../ImGui/*.h",
        "../ImGui/*.cpp",
        "*.cpp",
        "*.h",
        "extras/**.h"
	}
    
    includedirs
    {
        ".",
        "../raylib/include",
        "../ImGui"
    }
    
    filter "system:windows"
        systemversion "latest"
        
    filter "system:linux"
        pic "On"
        systemversion "latest"
        
    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        
    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        
    filter "configurations:Dist"
        runtime "Release"
        optimize "On"
        symbols "Off"