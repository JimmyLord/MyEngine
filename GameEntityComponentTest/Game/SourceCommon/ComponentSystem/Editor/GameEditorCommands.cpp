//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"
#include "GameEditorCommands.h"

//====================================================================================================
// EditorCommand_MoveObjects
//====================================================================================================

EditorCommand_MoveObjects::EditorCommand_MoveObjects(Vector3 distancemoved, const std::vector<GameObject*> &selectedobjects)
{
    m_DistanceMoved = distancemoved;
    for( unsigned int i=0; i<selectedobjects.size(); i++ )
    {
        m_ObjectsMoved.push_back( selectedobjects[i] );
    }
}

EditorCommand_MoveObjects::~EditorCommand_MoveObjects()
{
}

void EditorCommand_MoveObjects::Do()
{
    for( unsigned int i=0; i<m_ObjectsMoved.size(); i++ )
    {
        Vector3 newpos = m_ObjectsMoved[i]->m_pComponentTransform->GetPosition() + m_DistanceMoved;
        m_ObjectsMoved[i]->m_pComponentTransform->SetPosition( newpos );
    }
}

void EditorCommand_MoveObjects::Undo()
{
    for( unsigned int i=0; i<m_ObjectsMoved.size(); i++ )
    {
        Vector3 newpos = m_ObjectsMoved[i]->m_pComponentTransform->GetPosition() - m_DistanceMoved;
        m_ObjectsMoved[i]->m_pComponentTransform->SetPosition( newpos );
    }
}

//====================================================================================================
// EditorCommand_DeleteObject
//====================================================================================================

EditorCommand_DeleteObject::EditorCommand_DeleteObject(GameObject* objectdeleted)
{
    m_ObjectDeleted = objectdeleted;
    m_DeleteGameObjectWhenDestroyed = false;
}

EditorCommand_DeleteObject::~EditorCommand_DeleteObject()
{
    if( m_DeleteGameObjectWhenDestroyed )
        g_pComponentSystemManager->DeleteGameObject( m_ObjectDeleted, true );
}

void EditorCommand_DeleteObject::Do()
{
    g_pComponentSystemManager->UnmanageGameObject( m_ObjectDeleted );
    m_DeleteGameObjectWhenDestroyed = true;
}

void EditorCommand_DeleteObject::Undo()
{
    g_pComponentSystemManager->ManageGameObject( m_ObjectDeleted );
    m_DeleteGameObjectWhenDestroyed = false;
}

//====================================================================================================
//====================================================================================================
