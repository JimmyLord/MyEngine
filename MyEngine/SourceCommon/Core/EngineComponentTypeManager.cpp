//
// Copyright (c) 2014-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

// sort by category, otherwise right-click menu will have duplicates.
// must be in same order as enum EngineComponentTypes
// name(2nd column) is saved into the scene files, changing it will break objects.
ComponentTypeInfo g_EngineComponentTypeInfo[Component_NumEngineComponentTypes] = // ADDING_NEW_ComponentType
{
    { "Camera",         "Camera",               },  //ComponentType_Camera,
    { "Renderables",    "Sprite",               },  //ComponentType_Sprite,
    { "Renderables",    "Mesh",                 },  //ComponentType_Mesh,
    { "Renderables",    "Mesh-OBJ",             },  //ComponentType_MeshOBJ,
    { "Renderables",    "Mesh-Primitive",       },  //ComponentType_MeshPrimitive,
    { "Lighting",       "Light",                },  //ComponentType_Light,
    { "Lighting",       "Shadow Dir Light",     },  //ComponentType_CameraShadow,
    { "Effects",        "Post Effect Quad",     },  //ComponentType_PostEffect,
    { "Physics",        "Collision Object",     },  //ComponentType_CollisionObject,
    { "Physics",        "2D Collision Object",  },  //ComponentType_2DCollisionObject,
    { "Physics",        "2D Joint - Revolute",  },  //ComponentType_2DJointRevolute,
    { "Physics",        "2D Joint - Prismatic", },  //ComponentType_2DJointPrismatic,
    { "Physics",        "2D Joint - Weld",      },  //ComponentType_2DJointWeld,
    { "Scripts",        "Lua Script",           },  //ComponentType_LuaScript,
    { "Particles",      "Particle Emitter",     },  //ComponentType_ParticleEmitter,
    { "Animation",      "Animation Player",     },  //ComponentType_AnimationPlayer,
    { "Audio",          "Audio Player",         },  //ComponentType_AudioPlayer,
    { "Menus",          "Menu Page",            },  //ComponentType_MenuPage,
};

ComponentBase* EngineComponentTypeManager::CreateComponent(int type)
{
    ComponentBase* pComponent = 0;

    MyAssert( type != -1 );

    switch( type ) // ADDING_NEW_ComponentType
    {
    case ComponentType_Camera:              pComponent = MyNew ComponentCamera;             break;
    case ComponentType_Sprite:              pComponent = MyNew ComponentSprite;             break;
    case ComponentType_Mesh:                pComponent = MyNew ComponentMesh;               break;
    case ComponentType_MeshOBJ:             pComponent = MyNew ComponentMeshOBJ;            break;
    case ComponentType_MeshPrimitive:       pComponent = MyNew ComponentMeshPrimitive;      break;
    case ComponentType_Light:               pComponent = MyNew ComponentLight;              break;
    case ComponentType_CameraShadow:        pComponent = MyNew ComponentCameraShadow;       break;
    case ComponentType_PostEffect:          pComponent = MyNew ComponentPostEffect;         break;
    case ComponentType_CollisionObject:     pComponent = MyNew ComponentCollisionObject;    break;
    case ComponentType_2DCollisionObject:   pComponent = MyNew Component2DCollisionObject;  break;
    case ComponentType_2DJointRevolute:     pComponent = MyNew Component2DJointRevolute;    break;
    case ComponentType_2DJointPrismatic:    pComponent = MyNew Component2DJointPrismatic;   break;
    case ComponentType_2DJointWeld:         pComponent = MyNew Component2DJointWeld;        break;
#if MYFW_USING_LUA
    case ComponentType_LuaScript:           pComponent = MyNew ComponentLuaScript;          break;
#else
    case ComponentType_LuaScript:           pComponent = MyNew ComponentData;               break;
#endif //MYFW_USING_LUA
    case ComponentType_ParticleEmitter:     pComponent = MyNew ComponentParticleEmitter;    break;
    case ComponentType_AnimationPlayer:     pComponent = MyNew ComponentAnimationPlayer;    break;
    case ComponentType_AudioPlayer:         pComponent = MyNew ComponentAudioPlayer;        break;
    case ComponentType_MenuPage:            pComponent = MyNew ComponentMenuPage;           break;
    }

    MyAssert( pComponent != 0 );
    if( pComponent == 0 )
        return 0;

    pComponent->m_Type = type;
    return pComponent;
}

unsigned int EngineComponentTypeManager::GetNumberOfComponentTypes()
{
    MyAssert( 0 ); // should never reach here, game level type manager should override and provide a proper value.
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
