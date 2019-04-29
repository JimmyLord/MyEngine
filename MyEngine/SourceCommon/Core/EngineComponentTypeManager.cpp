//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EngineComponentTypeManager.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentMenuPage.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/ComponentTypeManager.h"
#include "ComponentSystem/EngineComponents/ComponentObjectPool.h"
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
#include "Voxels/ComponentVoxelMesh.h"
#include "Voxels/ComponentVoxelWorld.h"


// sort by category, otherwise right-click menu will have duplicates.
// must be in same order as enum EngineComponentTypes
// name(2nd column) is saved into the scene files, changing it will break objects.
ComponentTypeInfo g_EngineComponentTypeInfo[Component_NumEngineComponentTypes] = // ADDING_NEW_ComponentType
{
    { "Base",           "Transform",               },  //ComponentType_Transform,
    { "Camera",         "Camera",                  },  //ComponentType_Camera,
    { "Renderables",    "Sprite",                  },  //ComponentType_Sprite,
    { "Renderables",    "Mesh",                    },  //ComponentType_Mesh,
    { "Renderables",    "Mesh-OBJ",                },  //ComponentType_MeshOBJ,
    { "Renderables",    "Mesh-Primitive",          },  //ComponentType_MeshPrimitive,
    { "Renderables",    "Voxel Mesh",              },  //ComponentType_VoxelMesh,
    { "Renderables",    "Voxel World",             },  //ComponentType_VoxelWorld,
    { "Lighting",       "Light",                   },  //ComponentType_Light,
    { "Lighting",       "Shadow Dir Light",        },  //ComponentType_CameraShadow,
    { "Effects",        "Post Effect Quad",        },  //ComponentType_PostEffect,
    { "Physics",        "3D Collision Object",     },  //ComponentType_3DCollisionObject,
    { "Physics",        "3D Joint - Point2Point",  },  //ComponentType_3DJointPoint2Point,
    { "Physics",        "3D Joint - Hinge",        },  //ComponentType_3DJointHinge,
    { "Physics",        "3D Joint - Slider",       },  //ComponentType_3DJointSlider,
    { "Physics",        "2D Collision Object",     },  //ComponentType_2DCollisionObject,
    { "Physics",        "2D Joint - Revolute",     },  //ComponentType_2DJointRevolute,
    { "Physics",        "2D Joint - Prismatic",    },  //ComponentType_2DJointPrismatic,
    { "Physics",        "2D Joint - Weld",         },  //ComponentType_2DJointWeld,
    { "Scripts",        "Lua Script",              },  //ComponentType_LuaScript,
    { "Particles",      "Particle Emitter",        },  //ComponentType_ParticleEmitter,
    { "Animation",      "Animation Player",        },  //ComponentType_AnimationPlayer,
    { "Animation",      "2D Animation Player",     },  //ComponentType_AnimationPlayer2D,
    { "Audio",          "Audio Player",            },  //ComponentType_AudioPlayer,
    { "Logic",          "Object Pool",             },  //ComponentType_ObjectPool,
    { "Menus",          "Menu Page",               },  //ComponentType_MenuPage,
};

EngineComponentTypeManager::EngineComponentTypeManager()
: ComponentTypeManager()
{
}

ComponentBase* EngineComponentTypeManager::CreateComponent(int type)
{
    ComponentBase* pComponent = 0;

    MyAssert( type != -1 );

    switch( type ) // ADDING_NEW_ComponentType
    {
    case ComponentType_Transform:           pComponent = MyNew ComponentTransform( m_pComponentSystemManager );          break;
    case ComponentType_Camera:              pComponent = MyNew ComponentCamera( m_pComponentSystemManager );             break;
    case ComponentType_Sprite:              pComponent = MyNew ComponentSprite( m_pComponentSystemManager );             break;
    case ComponentType_Mesh:                pComponent = MyNew ComponentMesh( m_pComponentSystemManager );               break;
    case ComponentType_MeshOBJ:             pComponent = MyNew ComponentMeshOBJ( m_pComponentSystemManager );            break;
    case ComponentType_MeshPrimitive:       pComponent = MyNew ComponentMeshPrimitive( m_pComponentSystemManager );      break;
    case ComponentType_VoxelMesh:           pComponent = MyNew ComponentVoxelMesh( m_pComponentSystemManager );          break;
    case ComponentType_VoxelWorld:          pComponent = MyNew ComponentVoxelWorld( m_pComponentSystemManager );         break;
    case ComponentType_Light:               pComponent = MyNew ComponentLight( m_pComponentSystemManager );              break;
    case ComponentType_CameraShadow:        pComponent = MyNew ComponentCameraShadow( m_pComponentSystemManager );       break;
    case ComponentType_PostEffect:          pComponent = MyNew ComponentPostEffect( m_pComponentSystemManager );         break;
    case ComponentType_3DCollisionObject:   pComponent = MyNew Component3DCollisionObject( m_pComponentSystemManager );  break;
    case ComponentType_3DJointPoint2Point:  pComponent = MyNew Component3DJointPoint2Point( m_pComponentSystemManager ); break;
    case ComponentType_3DJointHinge:        pComponent = MyNew Component3DJointHinge( m_pComponentSystemManager );       break;
    case ComponentType_3DJointSlider:       pComponent = MyNew Component3DJointSlider( m_pComponentSystemManager );      break;
    case ComponentType_2DCollisionObject:   pComponent = MyNew Component2DCollisionObject( m_pComponentSystemManager );  break;
    case ComponentType_2DJointRevolute:     pComponent = MyNew Component2DJointRevolute( m_pComponentSystemManager );    break;
    case ComponentType_2DJointPrismatic:    pComponent = MyNew Component2DJointPrismatic( m_pComponentSystemManager );   break;
    case ComponentType_2DJointWeld:         pComponent = MyNew Component2DJointWeld( m_pComponentSystemManager );        break;
#if MYFW_USING_LUA
    case ComponentType_LuaScript:           pComponent = MyNew ComponentLuaScript( m_pComponentSystemManager );          break;
#else
    case ComponentType_LuaScript:           pComponent = MyNew ComponentData( m_pComponentSystemManager );               break;
#endif //MYFW_USING_LUA
    case ComponentType_ParticleEmitter:     pComponent = MyNew ComponentParticleEmitter( m_pComponentSystemManager );    break;
    case ComponentType_AnimationPlayer:     pComponent = MyNew ComponentAnimationPlayer( m_pComponentSystemManager );    break;
    case ComponentType_AnimationPlayer2D:   pComponent = MyNew ComponentAnimationPlayer2D( m_pComponentSystemManager );  break;
    case ComponentType_AudioPlayer:         pComponent = MyNew ComponentAudioPlayer( m_pComponentSystemManager );        break;
    case ComponentType_ObjectPool:          pComponent = MyNew ComponentObjectPool( m_pComponentSystemManager );         break;
    case ComponentType_MenuPage:            pComponent = MyNew ComponentMenuPage( m_pComponentSystemManager );           break;
    }

    MyAssert( pComponent != 0 );
    if( pComponent == 0 )
        return 0;

    pComponent->SetType( type );
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
