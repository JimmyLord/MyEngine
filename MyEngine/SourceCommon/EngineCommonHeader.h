//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EngineCommonHeader_H__
#define __EngineCommonHeader_H__

// framework code
#include "../../Framework/MyFramework/SourceCommon/CommonHeader.h"
#if MYFW_USING_WX
#include "../../Framework/MyFramework/SourceWindows/MYFWWinMainWx.h"
#include "Core/EngineMainFrame.h"
#include "../../Framework/MyFramework/SourceWidgets/EditorCommands.h"
#include "../../Framework/MyFramework/SourceWidgets/CommandStack.h"
#endif

// bullet
#if MYFW_WINDOWS
#pragma warning (disable : 4263)
#pragma warning (disable : 4264)
#endif
#include "../../../bullet3/src/btBulletDynamicsCommon.h"
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
#undef Nil
#include "../../lua/LuaBridge/LuaBridge.h"
#include "Core/LuaGameState.h"

// core component system code
#include "ComponentSystem/Core/ComponentTypeManager.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/BaseComponents/ComponentBase.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/BaseComponents/ComponentData.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentInputHandler.h"
#include "ComponentSystem/BaseComponents/ComponentUpdateable.h"
#include "ComponentSystem/BaseComponents/ComponentRenderable.h"

// myframework components // ADDING_NEW_ComponentType
#include "Core/EngineComponentTypeManager.h"
#include "ComponentSystem/FrameworkComponents/ComponentLight.h"
#include "ComponentSystem/FrameworkComponents/ComponentPostEffect.h"
#include "ComponentSystem/FrameworkComponents/ComponentCameraShadow.h"
#include "ComponentSystem/FrameworkComponents/ComponentSprite.h"
#include "ComponentSystem/FrameworkComponents/ComponentMesh.h"
#include "ComponentSystem/FrameworkComponents/ComponentMeshOBJ.h"
#include "ComponentSystem/FrameworkComponents/ComponentMeshPrimitive.h"
#include "ComponentSystem/FrameworkComponents/ComponentParticleEmitter.h"
#include "ComponentSystem/FrameworkComponents/ComponentAnimationPlayer.h"
#include "ComponentSystem/FrameworkComponents/ComponentLuaScript.h"
#include "ComponentSystem/FrameworkComponents/ComponentCollisionObject.h"

// misc engine code
#include "Camera/Camera3D.h"
#include "Camera/Camera2D.h"

#include "ComponentSystem/Core/GameObject.h"

#if MYFW_USING_WX
#include "Core/EditorState.h"
#include "Editor/EngineCommandStack.h"
#include "Editor/EngineEditorCommands.h"
#include "Editor/TransformGizmo.h"
#include "Editor/Dialogs/DialogGridSettings.h"
#endif

#include "Core/EngineCore.h"

#endif //__EngineCommonHeader_H__
