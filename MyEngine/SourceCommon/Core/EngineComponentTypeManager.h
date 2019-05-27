//
// Copyright (c) 2014-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EngineComponentTypeManager_H__
#define __EngineComponentTypeManager_H__

#include "ComponentSystem/Core/ComponentTypeManager.h"

class ComponentBase;

enum EngineComponentTypes // search for ADDING_NEW_ComponentType
{
    ComponentType_Transform, // must stay as first component, special handling of adding transform components in GameObject.cpp
    ComponentType_Camera,
    ComponentType_Sprite,
    ComponentType_Mesh, // this is a base type, shouldn't be in the editor's "add component" list
    ComponentType_MeshOBJ,
    ComponentType_MeshPrimitive,
    ComponentType_Heightmap,
    ComponentType_VoxelMesh,
    ComponentType_VoxelWorld,
    ComponentType_Light,
    ComponentType_CameraShadow,
    ComponentType_PostEffect,
    ComponentType_3DCollisionObject,
    ComponentType_3DJointPoint2Point,
    ComponentType_3DJointHinge,
    ComponentType_3DJointSlider,
    ComponentType_2DCollisionObject,
    ComponentType_2DJointRevolute,
    ComponentType_2DJointPrismatic,
    ComponentType_2DJointWeld,
    ComponentType_LuaScript,
    ComponentType_ParticleEmitter,
    ComponentType_AnimationPlayer,
    ComponentType_AnimationPlayer2D,
    ComponentType_AudioPlayer,
    ComponentType_ObjectPool,
    ComponentType_MenuPage,
    Component_NumEngineComponentTypes,
};

class EngineComponentTypeManager : public ComponentTypeManager
{
public:
    EngineComponentTypeManager();
    virtual ~EngineComponentTypeManager() override;

    virtual ComponentBase* CreateComponent(int type) override;
    virtual unsigned int GetNumberOfComponentTypes() override;

    virtual const char* GetTypeCategory(int type) override;
    virtual const char* GetTypeName(int type) override;

    virtual int GetTypeByName(const char* name) override;
};

#endif //__EngineComponentTypeManager_H__
