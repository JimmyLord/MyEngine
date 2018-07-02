cmake_minimum_required( VERSION 3.7 )

# Project
project( MyEngine_Project VERSION 0.1 LANGUAGES C CXX )

# MyEngine source files.
file( GLOB_RECURSE sourcefiles
    "MyEngine/SourceCommon/*.cpp"
    "Libraries/imgui/imgui.cpp"
    "Libraries/imgui/imgui_draw.cpp"
    "Libraries/Lua/src/*.c"
    "../SharedGameCode/Core/RenderTextQuick.cpp"
    "../SharedGameCode/Menus/MenuItem.cpp"
    "../SharedGameCode/Core/MeshShapes.cpp"
    "../SharedGameCode/Menus/LanguageTable.cpp"
    "../SharedGameCode/Menus/MenuButton.cpp"
    "../SharedGameCode/Menus/MenuCheckBox.cpp"
    "../SharedGameCode/Menus/MenuSprite.cpp"
    "../SharedGameCode/Menus/MenuText.cpp"
    "../SharedGameCode/Menus/MenuInputBox.cpp"
    "../SharedGameCode/Menus/MenuScrollingText.cpp"
    "../SharedGameCode/Menus/MenuScrollBox.cpp"
    "../SharedGameCode/Menus/Menu_Helpers.cpp"
)
list( REMOVE_ITEM sourcefiles "${CMAKE_SOURCE_DIR}/Libraries/Lua/src/lua.c" )
list( REMOVE_ITEM sourcefiles "${CMAKE_SOURCE_DIR}/Libraries/Lua/src/luac.c" )
list( REMOVE_ITEM sourcefiles "${CMAKE_SOURCE_DIR}/Libraries/Lua/src/luac.c" )

# Extra files specific to Editor (wxWidgets) builds.
if( CMAKE_BUILD_TYPE MATCHES wxDebug OR CMAKE_BUILD_TYPE MATCHES wxRelease )
    file( GLOB_RECURSE editorfiles "MyEngine/SourceEditor/*.cpp" )
    set( sourcefiles ${sourcefiles} ${editorfiles} )
endif()

# Library
add_library( MyEngine ${sourcefiles} )

#set_property( GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time" )

# MyEngine include directories.
target_include_directories( MyEngine PUBLIC
    "MyEngine/SourceCommon"
    "Libraries/imgui"
    "Libraries/Lua/src"
    "Libraries/LuaBridge"
    "Libraries/bullet3/src/"
    "Libraries/SharedGameCode"
    "../SharedGameCode/Core"
    "../SharedGameCode/Menus"
)

# MyFramework include directories.
target_include_directories( MyEngine PUBLIC
    "../Framework/MyFramework/SourceCommon"
    "../Framework/MyFramework/SourceLinux"
    "../Framework/MyFramework/SourceWidgets"
    "../Framework/Libraries/Box2D"
    "../Framework/Libraries/cJSON"
    "../Framework/Libraries/LodePNG"
    "../Framework/Libraries/wxWidgets/include"
    "../Framework/Libraries/wxWidgets/gtk-build-debug/lib/wx/include/gtk3-unicode-static-3.1"
)

# Base preprocessor defines.
target_compile_definitions( MyEngine PRIVATE MYFW_LINUX=1 )

if( CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES wxDebug )
    target_compile_definitions( MyEngine PRIVATE _DEBUG=1 )
endif()

# WxWidgets specific settings.
if( CMAKE_BUILD_TYPE MATCHES wxDebug OR CMAKE_BUILD_TYPE MATCHES wxRelease )

    # Preprocessor defines.
    target_compile_definitions( MyEngine PRIVATE MYFW_USING_WX=1 _FILE_OFFSET_BITS=64 __WXGTK__ )

    # Set base CMake build type to debug or release. not sure if works.
    if( CMAKE_BUILD_TYPE MATCHES wxDebug )
        set( CMAKE_BUILD_TYPE Debug )
    elseif( CMAKE_BUILD_TYPE MATCHES wxRelease )
        set( CMAKE_BUILD_TYPE Release )
    endif()

endif()

# cotire (Compile time reducer)
include( ../Framework/Libraries/Cotire/CMake/cotire.cmake )
set_target_properties( MyEngine PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "MyEngine/SourceCommon/EngineCommonHeader.h" )
cotire( MyEngine )
