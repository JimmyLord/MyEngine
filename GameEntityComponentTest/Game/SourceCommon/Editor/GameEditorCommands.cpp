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

    //LOGInfo( LOGTag, "EditorCommand_MoveObjects:: %f,%f,%f\n", m_DistanceMoved.x, m_DistanceMoved.y, m_DistanceMoved.z );

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
        m_ObjectsMoved[i]->m_pComponentTransform->UpdateMatrix();
    }
}

void EditorCommand_MoveObjects::Undo()
{
    //LOGInfo( LOGTag, "EditorCommand_MoveObjects::Undo %f,%f,%f\n", m_DistanceMoved.x, m_DistanceMoved.y, m_DistanceMoved.z );
    for( unsigned int i=0; i<m_ObjectsMoved.size(); i++ )
    {
        Vector3 newpos = m_ObjectsMoved[i]->m_pComponentTransform->GetPosition() - m_DistanceMoved;
        m_ObjectsMoved[i]->m_pComponentTransform->SetPosition( newpos );
        m_ObjectsMoved[i]->m_pComponentTransform->UpdateMatrix();
    }
}

EditorCommand* EditorCommand_MoveObjects::Repeat()
{
    EditorCommand_MoveObjects* pCommand;
    pCommand = MyNew EditorCommand_MoveObjects( *this );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_DeleteObjects
//====================================================================================================

EditorCommand_DeleteObjects::EditorCommand_DeleteObjects(const std::vector<GameObject*> &selectedobjects)
{
    for( unsigned int i=0; i<selectedobjects.size(); i++ )
    {
        m_ObjectsDeleted.push_back( selectedobjects[i] );
    }

    m_DeleteGameObjectsWhenDestroyed = false;
}

EditorCommand_DeleteObjects::~EditorCommand_DeleteObjects()
{
    if( m_DeleteGameObjectsWhenDestroyed )
    {
        for( unsigned int i=0; i<m_ObjectsDeleted.size(); i++ )
        {
            g_pComponentSystemManager->DeleteGameObject( m_ObjectsDeleted[i], true );
        }
    }
}

void EditorCommand_DeleteObjects::Do()
{
    ((GameEntityComponentTest*)g_pGameCore)->m_EditorState.ClearSelectedObjects();

    for( unsigned int i=0; i<m_ObjectsDeleted.size(); i++ )
    {
        g_pComponentSystemManager->UnmanageGameObject( m_ObjectsDeleted[i] );
    }
    m_DeleteGameObjectsWhenDestroyed = true;
}

void EditorCommand_DeleteObjects::Undo()
{
    ((GameEntityComponentTest*)g_pGameCore)->m_EditorState.ClearSelectedObjects();

    for( unsigned int i=0; i<m_ObjectsDeleted.size(); i++ )
    {
        g_pComponentSystemManager->ManageGameObject( m_ObjectsDeleted[i] );
    }
    m_DeleteGameObjectsWhenDestroyed = false;
}

EditorCommand* EditorCommand_DeleteObjects::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_CopyGameObject
//====================================================================================================

EditorCommand_CopyGameObject::EditorCommand_CopyGameObject(GameObject* objecttocopy)
{
    m_ObjectToCopy = objecttocopy;
    m_ObjectCreated = 0;
    m_DeleteGameObjectWhenDestroyed = false;
}

EditorCommand_CopyGameObject::~EditorCommand_CopyGameObject()
{
    assert( m_ObjectCreated );
    if( m_DeleteGameObjectWhenDestroyed && m_ObjectCreated )
        g_pComponentSystemManager->DeleteGameObject( m_ObjectCreated, true );
}

void EditorCommand_CopyGameObject::Do()
{
    if( m_ObjectCreated == 0 )
    {
        char newname[50];
        sprintf_s( newname, 50, "%s - copy", m_ObjectToCopy->GetName() );
        m_ObjectCreated = g_pComponentSystemManager->CopyGameObject( m_ObjectToCopy, newname );
    }
    else
    {
        g_pComponentSystemManager->ManageGameObject( m_ObjectCreated );
    }

    // if done/redone, then object exists in the scene, don't destroy it if undo stack get wiped.
    m_DeleteGameObjectWhenDestroyed = false;
}

void EditorCommand_CopyGameObject::Undo()
{
    ((GameEntityComponentTest*)g_pGameCore)->m_EditorState.ClearSelectedObjects();

    g_pComponentSystemManager->UnmanageGameObject( m_ObjectCreated );

    // if undone and redo stack gets wiped, then object only exist here, destroy it when this command gets deleted.
    m_DeleteGameObjectWhenDestroyed = true;
}

EditorCommand* EditorCommand_CopyGameObject::Repeat()
{
    EditorCommand_CopyGameObject* pCommand;
    pCommand = MyNew EditorCommand_CopyGameObject( m_ObjectToCopy );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
//====================================================================================================
