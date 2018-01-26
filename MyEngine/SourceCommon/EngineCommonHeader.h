//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EngineCommonHeader_H__
#define __EngineCommonHeader_H__

#define MYFW_USING_MYENGINE 1

#define MYFW_USING_LUA 1

const int g_NumberOfVisibilityLayers = 8;

// framework code
#include "../../../Framework/MyFramework/SourceCommon/CommonHeader.h"
#if MYFW_USING_WX
#include "../../../Framework/MyFramework/SourceWidgets/MYFWMainWx.h"
#include "../SourceEditor/EngineMainFrame.h"
#endif
#if MYFW_EDITOR
#include "../../../Framework/MyFramework/SourceEditor/EditorCommands.h"
#include "../../../Framework/MyFramework/SourceEditor/CommandStack.h"
#endif

// bullet
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

// lua
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

// dear ImGui
#include "../../Libraries/imgui/imgui.h"
#include "GUI/ImGuiManager.h"

// core component system code
#include "ComponentSystem/Core/EngineFileManager.h"
#include "ComponentSystem/Core/ComponentTypeManager.h"
#include "ComponentSystem/Core/SceneHandler.h"
#include "ComponentSystem/BaseComponents/ComponentVariable.h"
#include "ComponentSystem/BaseComponents/ComponentVariableValue.h"
#include "ComponentSystem/BaseComponents/ComponentBase.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/ComponentTemplate.h"
#include "ComponentSystem/BaseComponents/ComponentGameObjectProperties.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/BaseComponents/ComponentData.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentInputHandler.h"
#include "ComponentSystem/BaseComponents/ComponentUpdateable.h"
#include "ComponentSystem/BaseComponents/ComponentRenderable.h"
#include "ComponentSystem/BaseComponents/ComponentMenuPage.h"

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
#include "ComponentSystem/FrameworkComponents/ComponentAnimationPlayer2D.h"
#include "ComponentSystem/FrameworkComponents/ComponentAudioPlayer.h"
#if MYFW_USING_LUA
#include "ComponentSystem/FrameworkComponents/ComponentLuaScript.h"
#endif //MYFW_USING_LUA
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DCollisionObject.h"
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DJointBase.h"
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DJointPoint2Point.h"
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DJointHinge.h"
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DJointSlider.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DCollisionObject.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DJointRevolute.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DJointPrismatic.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DJointWeld.h"

// Other components
#include "Voxels/VoxelRayCast.h"
#include "Voxels/ComponentVoxelMesh.h"
#include "Voxels/ComponentVoxelWorld.h"

// Physics code
#include "Physics/EngineBox2DContactListener.h"

// misc engine code
#include "ComponentSystem/Core/PrefabManager.h"

#include "Camera/Camera3D.h"
#include "Camera/Camera2D.h"
#include "Core/InputFinger.h"

#include "ComponentSystem/Core/GameObject.h"

#include "../../../SharedGameCode/Core/RenderTextQuick.h"
#include "../../../SharedGameCode/Menus/MenuItem.h"
#include "../../../SharedGameCode/Core/MeshShapes.h"

#include "../../../SharedGameCode/Menus/LanguageTable.h"
#include "../../../SharedGameCode/Menus/MenuButton.h"
#include "../../../SharedGameCode/Menus/MenuCheckBox.h"
#include "../../../SharedGameCode/Menus/MenuSprite.h"
#include "../../../SharedGameCode/Menus/MenuText.h"
#include "../../../SharedGameCode/Menus/MenuInputBox.h"
#include "../../../SharedGameCode/Menus/MenuScrollingText.h"

#include "../../../SharedGameCode/Menus/Menu_Helpers.h"

#if MYFW_EDITOR
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/DragAndDropHackeryExtended.h"
#include "../SourceEditor/GameObjectTemplateManager.h"
#include "../SourceEditor/EngineCommandStack.h"
#include "../SourceEditor/EngineEditorCommands.h"
#include "../SourceEditor/TransformGizmo.h"

#if MYFW_USING_WX
#include "../SourceEditor/Dialogs/DialogGridSettings.h"
#endif

#include "../SourceEditor/Interfaces/EditorInterface.h"
#include "../SourceEditor/Interfaces/EditorInterface_SceneManagement.h"
#include "../SourceEditor/Interfaces/EditorInterface_2DPointEditor.h"
#include "../SourceEditor/Interfaces/EditorInterface_VoxelMeshEditor.h"
#endif

#include "Core/EngineCore.h"

#endif //__EngineCommonHeader_H__
