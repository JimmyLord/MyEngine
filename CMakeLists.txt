cmake_minimum_required( VERSION 3.4.1 )

###############################################################################
# Source code file list.
###############################################################################

# MyEngine source files.
file( GLOB_RECURSE sourceFiles
    "MyEngine/SourceCommon/*.cpp"
    "Libraries/imgui/imgui.cpp"
    "Libraries/imgui/imgui_demo.cpp"
    "Libraries/imgui/imgui_draw.cpp"
    "Libraries/imgui/imgui_widgets.cpp"
    "Libraries/Lua/src/*.c"
    "Libraries/bullet3/src/BulletCollision/*.cpp"
    "Libraries/bullet3/src/BulletDynamics/*.cpp"
    "Libraries/bullet3/src/LinearMath/*.cpp"
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
list( REMOVE_ITEM sourceFiles "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Lua/src/lua.c" )
list( REMOVE_ITEM sourceFiles "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Lua/src/luac.c" )

# Editor specific source files.
if( CMAKE_BUILD_TYPE MATCHES EditorDebug OR CMAKE_BUILD_TYPE MATCHES EditorRelease )
    file( GLOB_RECURSE editorFiles "MyEngine/SourceEditor/*.cpp" )
    set( sourceFiles ${sourceFiles} ${editorFiles} )
    list( REMOVE_ITEM sourceFiles "${CMAKE_CURRENT_SOURCE_DIR}/MyEngine/SourceEditor/Dialogs/DialogGridSettings.cpp" )
endif()

###############################################################################
# Library setup.
###############################################################################

# Library
add_library( MyEngine STATIC ${sourceFiles} )

# MyEngine include directories.
target_include_directories( MyEngine PUBLIC
    "MyEngine/SourceCommon"
	"MyEngine/SourceEditor"
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
    "../Framework/MyFramework/SourceEditor"
    "../Framework/MyFramework/SourceLinux"
    "../Framework/Libraries/b2Settings"
    "../Framework/Libraries/Box2D"
    "../Framework/Libraries/cJSON"
    "../Framework/Libraries/LodePNG"
    "../Framework/Libraries/OpenSimplexInC"
)

###############################################################################
# Platform specific settings.
###############################################################################

# Android defines.
if( CMAKE_SYSTEM_NAME MATCHES Android )
    target_compile_definitions( MyEngine PRIVATE MYFW_ANDROID=1 )
endif()

# Linux defines.
if( CMAKE_SYSTEM_NAME MATCHES Linux )
    target_compile_definitions( MyEngine PRIVATE MYFW_LINUX=1 )
    target_include_directories( MyEngine PUBLIC "../Framework/MyFramework/SourceLinux" )
endif()

# Windows/VisualStudio defines.
if( CMAKE_SYSTEM_NAME MATCHES Windows )
    target_compile_definitions( MyEngine PRIVATE MYFW_WINDOWS=1 )
    target_include_directories( MyEngine PUBLIC "../Framework/MyFramework/SourceWindows" )
	add_definitions( -DUNICODE -D_UNICODE )
endif()

# Editor settings.
if( CMAKE_BUILD_TYPE MATCHES EditorDebug OR CMAKE_BUILD_TYPE MATCHES EditorRelease )
    target_compile_definitions( MyEngine PRIVATE MYFW_USING_IMGUI=1 MYFW_EDITOR=1 )

    # Set base CMake build type to debug or release.
    if( CMAKE_BUILD_TYPE MATCHES EditorDebug )
        set( CMAKE_BUILD_TYPE Debug )
    elseif( CMAKE_BUILD_TYPE MATCHES EditorRelease )
        set( CMAKE_BUILD_TYPE Release )
    endif()
endif()

# Debug settings.
if( CMAKE_BUILD_TYPE MATCHES Debug )
    target_compile_definitions( MyEngine PRIVATE _DEBUG=1 )
endif()

# Currently disabled since some bullet headers don't like being in a unity build.
# cotire (Compile time reducer)
#include( ../Framework/Libraries/Cotire/CMake/cotire.cmake )
#set_target_properties( MyEngine PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "MyEngine/SourceCommon/EngineCommonHeader.h" )
#cotire( MyEngine )
