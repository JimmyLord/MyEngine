//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

// sort by category, otherwise right-click menu will have duplicates.
ComponentTypeInfo g_ComponentTypeInfo[Component_NumComponentTypes] = // ADDING_NEW_ComponentType
{
    { "Camera",         "3D Camera",        },
    { "Renderables",    "Sprite",           },
    { "Renderables",    "Mesh",             },
    { "Renderables",    "Mesh-OBJ",         },
    { "Input handlers", "Track Mouse",      },
    { "AI",             "AI Chase Player",  },
    { "Physics",        "Collision Object", },
};

ComponentBase* GameComponentTypeManager::CreateComponent(int type)
{
    ComponentBase* pComponent;

    switch( type ) // ADDING_NEW_ComponentType
    {
    case ComponentType_Camera:              pComponent = MyNew ComponentCamera;             break;
    case ComponentType_Sprite:              pComponent = MyNew ComponentSprite;             break;
    case ComponentType_Mesh:                pComponent = MyNew ComponentMesh;               break;
    case ComponentType_MeshOBJ:             pComponent = MyNew ComponentMeshOBJ;            break;
    case ComponentType_InputTrackMousePos:  pComponent = MyNew ComponentInputTrackMousePos; break;
    case ComponentType_AIChasePlayer:       pComponent = MyNew ComponentAIChasePlayer;      break;
    case ComponentType_CollisionObject:     pComponent = MyNew ComponentCollisionObject;    break;
    }

    assert( pComponent != 0 );

    pComponent->m_Type = type;
    return pComponent;
}

unsigned int GameComponentTypeManager::GetNumberOfComponentTypes()
{
    return Component_NumComponentTypes;
}

char* GameComponentTypeManager::GetTypeCategory(int type)
{
    return g_ComponentTypeInfo[type].category;
}

char* GameComponentTypeManager::GetTypeName(int type)
{
    return g_ComponentTypeInfo[type].name;
}

int GameComponentTypeManager::GetTypeByName(const char* name)
{
    for( int i=0; i<Component_NumComponentTypes; i++ )
    {
        if( strcmp( g_ComponentTypeInfo[i].name, name ) == 0 )
            return i;
    }

    return -1;
}
