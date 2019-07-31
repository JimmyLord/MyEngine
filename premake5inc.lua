-- This is not a complete premake5 lua script, it's meant to be included from another script that defines the workspace.
-- Like this, for example:
--     local rootDir = os.getcwd();
--     os.chdir( "../Engine/" )
--     include( "premake5inc.lua" )
--     os.chdir( rootDir )

project "MyEngine"
    configurations      { "Debug", "Release", "EditorDebug", "EditorRelease" }
    uuid                "CBF52F7E-23CE-4846-B48C-0146D0879805"
    kind                "StaticLib"
    language            "C++"
    targetdir           "$(SolutionDir)Output/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    objdir              "$(SolutionDir)Output/Intermediate/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    pchheader           "MyEnginePCH.h"
    pchsource           "MyEngine/SourceCommon/MyEnginePCH.cpp"

    includedirs {
        "MyEngine/SourceCommon",
		"Libraries/bullet3/src",
        "$(SolutionDir)../Framework/Libraries/b2Settings",
        "$(SolutionDir)../Framework/Libraries/Box2D",
        "C:/Program Files (x86)/Mono/include/mono-2.0",
    }

    files {
        "MyEngine/SourceCommon/**.cpp",
        "MyEngine/SourceCommon/**.h",
        "Libraries/imgui/im*.cpp",
        "Libraries/imgui/im*.h",
        "Libraries/imgui/README.md",
        "Libraries/Lua/src/*.c",
        "Libraries/Lua/src/*.h",
        "Libraries/LuaBridge/**.h",
        "DataEngineSource/**.cs",
        "README.md",
        "premake5inc.lua",
    }

    vpaths {
        -- Place these files in the root of the project.
        [""] = {
            "README.md",
            "premake5inc.lua",
        },
        -- Place the SourceCommon and SourceEditor folders in the root of the project.
        ["*"] = {
            "MyEngine",
        },
        -- Place the Libraries folder in the root of the project.
        ["Libraries*"] = {
            "Libraries",
        },
        -- Place the DataEngineSource folder in the root of the project.
        ["DataEngineSource*"] = {
            "DataEngineSource",
        },
    }

    filter { "files:Libraries/Lua/src/lua.c"
            .. " or Libraries/Lua/src/luac.c"
            .. " or MyEngine/SourceEditor/EditorMainFrame_Wx.*"
            .. " or MyEngine/SourceEditor/Dialogs/DialogGridSettings.*"
           }
        flags           "ExcludeFromBuild"

    filter { "files:Libraries/**" }
        flags           "NoPCH"

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

    filter "configurations:EditorDebug or EditorRelease"
        files {
            "MyEngine/SourceEditor/**.cpp",
            "MyEngine/SourceEditor/**.h",
        }
