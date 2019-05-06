-- This is not a complete premake5 lua script, it's meant to be included from another script that defines the workspace.
-- Like this, for example:
--     local rootDir = os.getcwd();
--     os.chdir( "../Engine/" )
--     include( "premake5inc.lua" )
--     os.chdir( rootDir )

project "SharedGameCode"
    configurations      { "Debug", "Release", "EditorDebug", "EditorRelease" }
    uuid                "9E4D91C3-6ED5-4DFD-B6CD-ADA011C049B8"
    kind                "StaticLib"
    language            "C++"
    targetdir           "$(SolutionDir)Output/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    objdir              "$(SolutionDir)Output/Intermediate/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    pchheader           "SharedCommonHeader.h"
    pchsource           "SharedCommonHeader.cpp"
    includedirs {
        "$(SolutionDir)../Engine/MyEngine/SourceCommon",
        "$(SolutionDir)../Engine/Libraries/bullet3/src/",
        "$(SolutionDir)../Engine/Libraries/SharedGameCode",
        "$(SolutionDir)../Framework/Libraries/b2Settings",
        "$(SolutionDir)../Framework/Libraries/Box2D",
        "$(SolutionDir)../SharedGameCode",
    }

    files {
        "../../../SharedGameCode/Camera/SharedCamera3D.*",
        "../../../SharedGameCode/Core/MyMeshText.*",
        "../../../SharedGameCode/Core/RenderTextQuick.*",
        "../../../SharedGameCode/Menus/*",
        "SharedCommonHeader.*",
        "premake5inc.lua",
    }

    vpaths {
        -- Place these files in the root of the project.
        [""] = {
            "premake5inc.lua",
        },
        -- Place the PCH files in the SharedGameCode folder.
        ["SharedGameCode"] = {
            "SharedCommonHeader.*",
        },
    }

    filter "system:windows"
        platforms       { "x86", "x64" }
        defines         "MYFW_WINDOWS"
        systemversion   "latest"
        characterset    "MBCS"

    filter "configurations:Debug or EditorDebug"
        defines         "_DEBUG"
        symbols         "on"

    filter "configurations:Release or EditorRelease"
        defines         "NDEBUG"
        optimize        "Full"

    filter "configurations:EditorDebug or EditorRelease"
        defines         { "MYFW_EDITOR", "MYFW_USING_IMGUI" }
