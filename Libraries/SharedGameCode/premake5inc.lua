-- This is not a complete premake5 lua script, it's meant to be included from another script that defines the workspace.
-- Like this, for example:
--     local rootDir = os.getcwd();
--     os.chdir( "../Engine/" )
--     include( "premake5inc.lua" )
--     os.chdir( rootDir )

if SharedGameCodePremakeConfig_EngineFolder == nil then
    SharedGameCodePremakeConfig_EngineFolder = "$(SolutionDir)../Engine"
end
if SharedGameCodePremakeConfig_FrameworkFolder == nil then
    SharedGameCodePremakeConfig_FrameworkFolder = "$(SolutionDir)../Framework"
end
if SharedGameCodePremakeConfig_SharedGameCodeFolder == nil then
    SharedGameCodePremakeConfig_SharedGameCodeFolder = "$(SolutionDir)../SharedGameCode"
end

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
        SharedGameCodePremakeConfig_EngineFolder .. "MyEngine/SourceCommon",
        SharedGameCodePremakeConfig_EngineFolder .. "Libraries/bullet3/src/",
        SharedGameCodePremakeConfig_EngineFolder .. "Libraries/SharedGameCode",
        SharedGameCodePremakeConfig_FrameworkFolder .. "/Libraries/b2Settings",
        SharedGameCodePremakeConfig_FrameworkFolder .. "/Libraries/Box2D",
        SharedGameCodePremakeConfig_SharedGameCodeFolder,
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
