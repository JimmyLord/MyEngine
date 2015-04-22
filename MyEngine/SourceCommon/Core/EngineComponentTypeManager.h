//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EngineComponentTypeManager_H__
#define __EngineComponentTypeManager_H__

class ComponentBase;

enum EngineComponentTypes // search for ADDING_NEW_ComponentType
{
    ComponentType_Camera,
    ComponentType_Sprite,
    ComponentType_Mesh,
    ComponentType_MeshOBJ,
    ComponentType_MeshPrimitive,
    ComponentType_Light,
    ComponentType_CameraShadow,
    ComponentType_PostEffect,
    ComponentType_CollisionObject,
    ComponentType_LuaScript,
    ComponentType_ParticleEmitter,
    ComponentType_AnimationPlayer,
    Component_NumEngineComponentTypes,
};

class EngineComponentTypeManager : public ComponentTypeManager
{
public:
    virtual ComponentBase* CreateComponent(int type);
    virtual unsigned int GetNumberOfComponentTypes();

    virtual const char* GetTypeCategory(int type);
    virtual const char* GetTypeName(int type);

    virtual int GetTypeByName(const char* name);
};

#endif //__EngineComponentTypeManager_H__
