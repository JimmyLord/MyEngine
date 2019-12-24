//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EnginePCH_H__
#define __EnginePCH_H__

#define MYFW_USING_MYENGINE 1 // Used by a couple of old modules in SharedGameCode.

#if defined(MYFW_USE_LUA) && MYFW_USE_LUA == 0
#define MYFW_USING_LUA 0
#else
#define MYFW_USING_LUA 1
#endif

#if MYFW_USE_MONO
#define MYFW_USING_MONO 1
#endif

#if defined(MYFW_USE_BULLET) && MYFW_USE_BULLET == 0
#define MYFW_USING_BULLET 0
#else
#define MYFW_USING_BULLET 1
#endif

const int g_NumberOfVisibilityLayers = 8;

//============================================================================================================
// MyFramework includes.
//============================================================================================================

#include "../../../Framework/MyFramework/SourceCommon/MyFramework.h"
#if MYFW_USING_WX
#include "../../../Framework/MyFramework/SourceWidgets/MYFWMainWx.h"
#endif
#if MYFW_EDITOR
#include "../../../Framework/MyFramework/SourceEditor/EditorCommands.h"
#include "../../../Framework/MyFramework/SourceEditor/CommandStack.h"
#endif

//============================================================================================================
// Libraries.
//============================================================================================================

// Bullet.
#if MYFW_WINDOWS
#pragma warning( push )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4263 )
#pragma warning( disable : 4264 )
#pragma warning( disable : 4640 )
#pragma warning( disable : 4305 )
#endif
#include "../../Libraries/bullet3/src/btBulletDynamicsCommon.h"
#if MYFW_WINDOWS
#pragma warning( pop )
#endif
#include "Physics/BulletWorld.h"

// Lua.
#if MYFW_USING_LUA
extern "C"
{
    #include "../../Libraries/Lua/src/lua.h"
    #include "../../Libraries/Lua/src/lualib.h"
    #include "../../Libraries/Lua/src/lauxlib.h"
}
#undef Nil
#undef None // defined in X.h
#pragma warning( push )
#pragma warning( disable : 4640 )
#include "../../Libraries/LuaBridge/LuaBridge.h"
#pragma warning( pop )
#include "Core/LuaGLFunctions.h"
#include "Core/LuaGameState.h"
#endif //MYFW_USING_LUA

// dear ImGui.
#include "GUI/ImGuiConfig.h"
#include "../../Libraries/imgui/imgui.h"
#include "GUI/ImGuiManager.h"

//============================================================================================================
// Libraries.
//============================================================================================================

static const int MAX_SCENES_LOADED = 10;
static const int MAX_SCENES_LOADED_INCLUDING_UNMANAGED = 11;
static const int MAX_SCENES_CREATED = 12; // Includes Unmanaged and Editor Object Scenes.

enum SceneID
{
    SCENEID_MainScene     = 0, // First scene that should be loaded // Must be 0.
    // Scenes 0-MAX_SCENES_LOADED are all valid, but will need to be typecast when used.
    SCENEID_Unmanaged     = MAX_SCENES_LOADED,   // For unmanaged/runtime objects.
    SCENEID_EngineObjects = MAX_SCENES_LOADED+1, // For Editor Objects (Transform Gizmo, Editor cam, etc)
    SCENEID_TempPlayStop  = 12345, // Used to load temp scene, which contains objects from all scenes.
    SCENEID_AllScenes     = 23456, // ID passed in to indicate all scenes (when saving/unloading).
    SCENEID_NotFound      = 34567, // ID returned when an object isn't found.
    SCENEID_NotSet        = 45678, // ID to initialize to if required.
    SCENEID_Any           = 56789, // ID passed in when requesting to load a scene into any slot.
};

#endif //__EnginePCH_H__
