//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

// sort by category, otherwise right-click menu will have duplicates.
ComponentTypeInfo g_EngineComponentTypeInfo[Component_NumEngineComponentTypes] = // ADDING_NEW_ComponentType
{
    { "Camera",         "Camera",           },
    { "Renderables",    "Sprite",           },
    { "Renderables",    "Mesh",             },
    { "Renderables",    "Mesh-OBJ",         },
    { "Renderables",    "Mesh-Primitive",   },
    { "Lighting",       "Light",            },
    { "Lighting",       "Shadow Dir Light", },
    { "Physics",        "Collision Object", },
    { "Scripts",        "Lua Script",       },
    { "Particles",      "Particle Emitter", },
    { "Animation",      "Animation Player", },
};

ComponentBase* EngineComponentTypeManager::CreateComponent(int type)
{
    ComponentBase* pComponent;

    assert( type != -1 );

    switch( type ) // ADDING_NEW_ComponentType
    {
    case ComponentType_Camera:              pComponent = MyNew ComponentCamera;             break;
    case ComponentType_Sprite:              pComponent = MyNew ComponentSprite;             break;
    case ComponentType_Mesh:                pComponent = MyNew ComponentMesh;               break;
    case ComponentType_MeshOBJ:             pComponent = MyNew ComponentMeshOBJ;            break;
    case ComponentType_MeshPrimitive:       pComponent = MyNew ComponentMeshPrimitive;      break;
    case ComponentType_Light:               pComponent = MyNew ComponentLight;              break;
    case ComponentType_CameraShadow:        pComponent = MyNew ComponentCameraShadow;       break;
    case ComponentType_CollisionObject:     pComponent = MyNew ComponentCollisionObject;    break;
    case ComponentType_LuaScript:           pComponent = MyNew ComponentLuaScript;          break;
    case ComponentType_ParticleEmitter:     pComponent = MyNew ComponentParticleEmitter;    break;
    case ComponentType_AnimationPlayer:     pComponent = MyNew ComponentAnimationPlayer;    break;
    }

    assert( pComponent != 0 );

    pComponent->m_Type = type;
    return pComponent;
}

unsigned int EngineComponentTypeManager::GetNumberOfComponentTypes()
{
    assert( 0 ); // should never reach here, game level type manager should override and provide a proper value.
    return Component_NumEngineComponentTypes;
}

const char* EngineComponentTypeManager::GetTypeCategory(int type)
{
    return g_EngineComponentTypeInfo[type].category;
}

const char* EngineComponentTypeManager::GetTypeName(int type)
{
    return g_EngineComponentTypeInfo[type].name;
}

int EngineComponentTypeManager::GetTypeByName(const char* name)
{
    for( int i=0; i<Component_NumEngineComponentTypes; i++ )
    {
        if( strcmp( g_EngineComponentTypeInfo[i].name, name ) == 0 )
            return i;
    }

    return -1;
}
