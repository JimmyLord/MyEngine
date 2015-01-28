//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __GameCommonHeader_H__
#define __GameCommonHeader_H__

// framework code
#include "../../Framework/MyFramework/SourceCommon/CommonHeader.h"
#if MYFW_USING_WX
#include "../../Framework/MyFramework/SourceWindows/MYFWWinMainWx.h"
#include "Core/GameMainFrame.h"
#include "../../Framework/MyFramework/SourceWidgets/EditorCommands.h"
#include "../../Framework/MyFramework/SourceWidgets/CommandStack.h"
#endif

// bullet
#if MYFW_WINDOWS
#pragma warning (disable : 4263)
#pragma warning (disable : 4264)
#endif
#include "../../../bullet-2.82-r2704/src/btBulletDynamicsCommon.h"
#if MYFW_WINDOWS
#pragma warning (default : 4263)
#pragma warning (default : 4264)
#endif
#include "Core/BulletWorld.h"

// lua
extern "C"
{
    #include "../../lua/src/lua.h"
    #include "../../lua/src/lualib.h"
    #include "../../lua/src/lauxlib.h"
}
#include "../../lua/LuaBridge/LuaBridge.h"
#include "Core/LuaGameState.h"

// core component system code
#include "ComponentSystem/Core/ComponentTypeManager.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/Core/BaseComponents/ComponentBase.h"
#include "ComponentSystem/Core/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/BaseComponents/ComponentData.h"
#include "ComponentSystem/Core/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/Core/BaseComponents/ComponentInputHandler.h"
#include "ComponentSystem/Core/BaseComponents/ComponentUpdateable.h"
#include "ComponentSystem/Core/BaseComponents/ComponentRenderable.h"

// myframework components
#include "Core/GameComponentTypeManager.h"
#include "ComponentSystem/CustomComponents/ComponentSprite.h"
#include "ComponentSystem/CustomComponents/ComponentMesh.h"
#include "ComponentSystem/CustomComponents/ComponentMeshOBJ.h"
#include "ComponentSystem/CustomComponents/ComponentMeshPlane.h"

// game code
#include "Camera/Camera3D.h"
#include "Camera/Camera2D.h"
#include "GameComponents/ComponentInputTrackMousePos.h"
#include "GameComponents/ComponentAIChasePlayer.h"
#include "GameComponents/ComponentCollisionObject.h"
#include "GameComponents/ComponentLuaScript.h"

#include "ComponentSystem/Core/GameObject.h"

#if MYFW_USING_WX
#include "Core/EditorState.h"
#include "ComponentSystem/Editor/GameCommandStack.h"
#include "ComponentSystem/Editor/GameEditorCommands.h"
#endif

#include "Core/GameEntityComponentTest.h"

#endif //__GameCommonHeader_H__
