//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyEngine_H__
#define __MyEngine_H__

// This file includes all headers in the MyEngine library.

//============================================================================================================
// C/C++/Library engine includes.
//============================================================================================================

#include "MyEnginePCH.h"

//============================================================================================================
// Core engine includes.
//============================================================================================================

// Core component system code.
#include "ComponentSystem/ComponentTemplate.h"
#include "ComponentSystem/BaseComponents/ComponentBase.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentData.h"
#include "ComponentSystem/BaseComponents/ComponentInputHandler.h"
#include "ComponentSystem/BaseComponents/ComponentMenuPage.h"
#include "ComponentSystem/BaseComponents/ComponentGameObjectProperties.h"
#include "ComponentSystem/BaseComponents/ComponentRenderable.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/BaseComponents/ComponentUpdateable.h"
#include "ComponentSystem/BaseComponents/ComponentVariable.h"
#include "ComponentSystem/BaseComponents/ComponentVariableValue.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/Core/ComponentTypeManager.h"
#include "ComponentSystem/Core/EngineFileManager.h"
#include "ComponentSystem/Core/SceneHandler.h"

// MyEngine components.
#include "ComponentSystem/EngineComponents/ComponentHeightmap.h"
#include "ComponentSystem/EngineComponents/ComponentObjectPool.h"
#include "Voxels/ComponentVoxelMesh.h"
#include "Voxels/ComponentVoxelWorld.h"
#include "Voxels/VoxelRayCast.h"

// MyFramework components. // ADDING_NEW_ComponentType
#include "ComponentSystem/FrameworkComponents/ComponentAnimationPlayer.h"
#include "ComponentSystem/FrameworkComponents/ComponentAnimationPlayer2D.h"
#include "ComponentSystem/FrameworkComponents/ComponentAudioPlayer.h"
#include "ComponentSystem/FrameworkComponents/ComponentCameraShadow.h"
#include "ComponentSystem/FrameworkComponents/ComponentLight.h"
#if MYFW_USING_LUA
#include "ComponentSystem/FrameworkComponents/ComponentLuaScript.h"
#endif //MYFW_USING_LUA
#include "ComponentSystem/FrameworkComponents/ComponentMesh.h"
#include "ComponentSystem/FrameworkComponents/ComponentMeshOBJ.h"
#include "ComponentSystem/FrameworkComponents/ComponentMeshPrimitive.h"
#include "ComponentSystem/FrameworkComponents/ComponentParticleEmitter.h"
#include "ComponentSystem/FrameworkComponents/ComponentPostEffect.h"
#include "ComponentSystem/FrameworkComponents/ComponentSprite.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DCollisionObject.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DJointPrismatic.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DJointRevolute.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DJointWeld.h"
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DCollisionObject.h"
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DJointBase.h"
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DJointHinge.h"
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DJointPoint2Point.h"
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DJointSlider.h"

// Physics code.
#include "Physics/EngineBox2DContactListener.h"

// Misc engine code.
#include "ComponentSystem/Core/PrefabManager.h"

#include "Camera/Camera2D.h"
#include "Camera/Camera3D.h"
#include "Core/EngineComponentTypeManager.h"
#include "Core/InputFinger.h"
#include "Core/EngineCore.h"

#include "ComponentSystem/Core/GameObject.h"

#include "../../../SharedGameCode/Core/MyMeshText.h"
#include "../../../SharedGameCode/Core/RenderTextQuick.h"

#include "../../../SharedGameCode/Menus/LanguageTable.h"
#include "../../../SharedGameCode/Menus/MenuButton.h"
#include "../../../SharedGameCode/Menus/MenuCheckBox.h"
#include "../../../SharedGameCode/Menus/MenuInputBox.h"
#include "../../../SharedGameCode/Menus/MenuItem.h"
#include "../../../SharedGameCode/Menus/MenuScrollingText.h"
#include "../../../SharedGameCode/Menus/MenuSprite.h"
#include "../../../SharedGameCode/Menus/MenuText.h"

#include "../../../SharedGameCode/Menus/Menu_Helpers.h"

#if MYFW_EDITOR
#include "../SourceEditor/DragAndDropHackeryExtended.h"
#include "../SourceEditor/EditorMainFrame.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/GameObjectTemplateManager.h"
#include "../SourceEditor/TransformGizmo.h"
#include "../SourceEditor/Commands/EngineCommandStack.h"
#include "../SourceEditor/Commands/EngineEditorCommands.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"

#if MYFW_USING_IMGUI
#include "../SourceEditor/Editor_ImGui/EditorLayoutManager_ImGui.h"
#include "../SourceEditor/Editor_ImGui/EditorLogWindow_ImGui.h"
#include "../SourceEditor/Editor_ImGui/EditorMainFrame_ImGui.h"
#endif

#include "../SourceEditor/Interfaces/EditorInterface.h"
#include "../SourceEditor/Interfaces/EditorInterface_2DPointEditor.h"
#include "../SourceEditor/Interfaces/EditorInterface_SceneManagement.h"
#include "../SourceEditor/Interfaces/EditorInterface_VoxelMeshEditor.h"
#endif

#endif //__MyEngine_H__
