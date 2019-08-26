-- This is not a complete premake5 lua script, it's meant to be included from another script that defines the workspace.
-- Like this, for example:
--     local rootDir = os.getcwd();
--     os.chdir( "../Engine/" )
--     include( "premake5inc.lua" )
--     os.chdir( rootDir )

------------------------------------------------------------------------------------------------------------------

project "BulletCollision"
    configurations          { "Debug", "Release" }
    configmap               {
                                ["EditorDebug"] = "Debug",
                                ["EditorRelease"] = "Release"
                            }
    uuid                    "1BD50A98-FB78-7F43-84A9-82901F5A00D0"
    kind                    "StaticLib"
    language                "C++"
    targetdir               "$(SolutionDir)Output/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    objdir                  "$(SolutionDir)Output/Intermediate/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    floatingpoint           "Fast"
    rtti                    "Off"
    warnings                "Off"

    includedirs {
        "$(SolutionDir)../Engine/Libraries/bullet3/src/",
    }

    files {
        "bullet3/src/BulletCollision/**",
    }

    filter "system:windows"
        platforms           { "x86", "x64" }
        systemversion       "latest"
        characterset        "MBCS"

    filter "configurations:Debug"
        defines             "_DEBUG"
        symbols             "on"

    filter "configurations:Release"
        optimize            "Full"
        vectorextensions    "SSE2"

-- String pooling is causing build errors in VS2019, don't feel like looking into it, so disabling.
--    filter { "system:windows", "configurations:Release" }
--        buildoptions        { "\\GF" } -- /GF -> Enable String Pooling 

------------------------------------------------------------------------------------------------------------------

project "BulletDynamics"
    configurations          { "Debug", "Release" }
    configmap               {
                                ["EditorDebug"] = "Debug",
                                ["EditorRelease"] = "Release"
                            }
    uuid                    "AD82F95E-C422-7443-A26D-57999762CED9"
    kind                    "StaticLib"
    language                "C++"
    targetdir               "$(SolutionDir)Output/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    objdir                  "$(SolutionDir)Output/Intermediate/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    floatingpoint           "Fast"
    rtti                    "Off"
    warnings                "Off"

    includedirs {
        "$(SolutionDir)../Engine/Libraries/bullet3/src/",
    }

    files {
        "bullet3/src/BulletDynamics/**",
    }

    filter "system:windows"
        platforms           { "x86", "x64" }
        systemversion       "latest"
        characterset        "MBCS"

    filter "configurations:Debug"
        defines             "_DEBUG"
        symbols             "on"

    filter "configurations:Release"
        optimize            "Full"
        vectorextensions    "SSE2"

-- String pooling is causing build errors in VS2019, don't feel like looking into it, so disabling.
--    filter { "system:windows", "configurations:Release" }
--        buildoptions        { "\\GF" } -- /GF -> Enable String Pooling 

------------------------------------------------------------------------------------------------------------------

project "LinearMath"
    configurations          { "Debug", "Release" }
    configmap               {
                                ["EditorDebug"] = "Debug",
                                ["EditorRelease"] = "Release"
                            }
    uuid                    "5E81B361-BFF0-A443-A143-44C7B2164E8E"
    kind                    "StaticLib"
    language                "C++"
    targetdir               "$(SolutionDir)Output/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    objdir                  "$(SolutionDir)Output/Intermediate/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    floatingpoint           "Fast"
    rtti                    "Off"
    warnings                "Off"

    includedirs {
        "$(SolutionDir)../Engine/Libraries/bullet3/src/",
    }

    files {
        "bullet3/src/LinearMath/**",
    }

    filter "system:windows"
        platforms           { "x86", "x64" }
        systemversion       "latest"
        characterset        "MBCS"

    filter "configurations:Debug"
        defines             "_DEBUG"
        symbols             "on"

    filter "configurations:Release"
        optimize            "Full"
        vectorextensions    "SSE2"

-- String pooling is causing build errors in VS2019, don't feel like looking into it, so disabling.
--    filter { "system:windows", "configurations:Release" }
--        buildoptions        { "\\GF" } -- /GF -> Enable String Pooling 
