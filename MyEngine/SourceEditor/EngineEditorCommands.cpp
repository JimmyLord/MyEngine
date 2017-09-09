//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "EngineEditorCommands.h"

//====================================================================================================
// EditorCommand_MoveObjects
//====================================================================================================

EditorCommand_MoveObjects::EditorCommand_MoveObjects(Vector3 distancemoved, const std::vector<GameObject*>& selectedobjects)
{
    m_Name = "EditorCommand_MoveObjects";

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
        ComponentTransform* pTransform = m_ObjectsMoved[i]->GetTransform();

        Vector3 newpos = pTransform->GetLocalTransform()->GetTranslation() + m_DistanceMoved;
        pTransform->SetPositionByEditor( newpos );
        pTransform->UpdateTransform();
    }
}

void EditorCommand_MoveObjects::Undo()
{
    //LOGInfo( LOGTag, "EditorCommand_MoveObjects::Undo %f,%f,%f\n", m_DistanceMoved.x, m_DistanceMoved.y, m_DistanceMoved.z );
    for( unsigned int i=0; i<m_ObjectsMoved.size(); i++ )
    {
        ComponentTransform* pTransform = m_ObjectsMoved[i]->GetTransform();

        Vector3 newpos = pTransform->GetLocalTransform()->GetTranslation() - m_DistanceMoved;
        pTransform->SetPositionByEditor( newpos );
        pTransform->UpdateTransform();
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
// EditorCommand_ScaleObjects
//====================================================================================================

EditorCommand_ScaleObjects::EditorCommand_ScaleObjects(Vector3 amountscaled, bool localspace, Vector3 pivot, const std::vector<GameObject*>& selectedobjects)
{
    m_Name = "EditorCommand_ScaleObjects";

    m_AmountScaled = amountscaled;
    m_TransformedInLocalSpace = localspace;
    m_WorldSpacePivot = pivot;

    //LOGInfo( LOGTag, "EditorCommand_ScaleObjects:: %f,%f,%f\n", m_AmountScaled.x, m_AmountScaled.y, m_AmountScaled.z );

    for( unsigned int i=0; i<selectedobjects.size(); i++ )
    {
        m_ObjectsScaled.push_back( selectedobjects[i] );
    }
}

EditorCommand_ScaleObjects::~EditorCommand_ScaleObjects()
{
}

void EditorCommand_ScaleObjects::Do()
{
    for( unsigned int i=0; i<m_ObjectsScaled.size(); i++ )
    {
        ComponentTransform* pTransform = m_ObjectsScaled[i]->GetTransform();

        // only scale in local space
        //if( m_TransformedInLocalSpace == true )
        {
            Vector3 newscale = pTransform->GetLocalTransform()->GetScale() * m_AmountScaled;

            pTransform->SetScaleByEditor( newscale );
            pTransform->UpdateTransform();
        }
        //else
        //{
        //    MyMatrix matscale;
        //    matscale.CreateScale( m_AmountScaled );
        //    pTransform->Scale( &matscale, m_WorldSpacePivot );
        //}
    }
}

void EditorCommand_ScaleObjects::Undo()
{
    //LOGInfo( LOGTag, "EditorCommand_ScaleObjects::Undo %f,%f,%f\n", m_AmountScaled.x, m_AmountScaled.y, m_AmountScaled.z );
    for( unsigned int i=0; i<m_ObjectsScaled.size(); i++ )
    {
        ComponentTransform* pTransform = m_ObjectsScaled[i]->GetTransform();

        // only scale in local space
        //if( m_TransformedInLocalSpace == true )
        {
            Vector3 newscale = pTransform->GetLocalTransform()->GetScale() / m_AmountScaled;

            pTransform->SetScaleByEditor( newscale );
            pTransform->UpdateTransform();
        }
        //else
        //{
        //    MyMatrix matscale;
        //    matscale.CreateScale( 1/m_AmountScaled );
        //    pTransform->Scale( &matscale, m_WorldSpacePivot );
        //}
    }
}

EditorCommand* EditorCommand_ScaleObjects::Repeat()
{
    EditorCommand_ScaleObjects* pCommand;
    pCommand = MyNew EditorCommand_ScaleObjects( *this );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_RotateObjects
//====================================================================================================

EditorCommand_RotateObjects::EditorCommand_RotateObjects(Vector3 amountRotated, bool localspace, Vector3 pivot, const std::vector<GameObject*>& selectedobjects)
{
    m_Name = "EditorCommand_RotateObjects";

    m_AmountRotated = amountRotated;
    m_TransformedInLocalSpace = localspace;
    m_WorldSpacePivot = pivot;

    //LOGInfo( LOGTag, "EditorCommand_RotateObjects:: %f,%f,%f\n", m_AmountRotated.x, m_AmountRotated.y, m_AmountRotated.z );

    for( unsigned int i=0; i<selectedobjects.size(); i++ )
    {
        m_ObjectsRotated.push_back( selectedobjects[i] );
    }
}

EditorCommand_RotateObjects::~EditorCommand_RotateObjects()
{
}

void EditorCommand_RotateObjects::Do()
{
    for( unsigned int i=0; i<m_ObjectsRotated.size(); i++ )
    {
        ComponentTransform* pTransform = m_ObjectsRotated[i]->GetTransform();

        if( m_TransformedInLocalSpace == true )
        {
            MyMatrix objectRotation;
            MyMatrix newRotation;
            MyMatrix combinedRotation;

            objectRotation.CreateRotation( pTransform->GetWorldRotation() );
            newRotation.CreateRotation( m_AmountRotated );
            combinedRotation = objectRotation * newRotation;
            
            Vector3 eulerdegrees = combinedRotation.GetEulerAngles() * 180.0f / PI;
            pTransform->SetWorldRotation( eulerdegrees );
            pTransform->UpdateTransform();
        }
        else
        {
            MyMatrix newRotation;

            newRotation.CreateRotation( m_AmountRotated );
            pTransform->Rotate( &newRotation, m_WorldSpacePivot );
        }

        //pTransform->SetWorldTransform( &combinedRotation );
    }
}

void EditorCommand_RotateObjects::Undo()
{
    //LOGInfo( LOGTag, "EditorCommand_RotateObjects::Undo %f,%f,%f\n", m_AmountRotated.x, m_AmountRotated.y, m_AmountRotated.z );
    for( unsigned int i=0; i<m_ObjectsRotated.size(); i++ )
    {
        ComponentTransform* pTransform = m_ObjectsRotated[i]->GetTransform();

        if( m_TransformedInLocalSpace == true )
        {
            MyMatrix objectRotation;
            MyMatrix newRotation;
            MyMatrix combinedRotation;

            objectRotation.CreateRotation( pTransform->GetWorldRotation() );
            newRotation.CreateRotation( m_AmountRotated * -1 );
            combinedRotation = objectRotation * newRotation;

            Vector3 eulerdegrees = combinedRotation.GetEulerAngles() * 180.0f / PI;
            pTransform->SetWorldRotation( eulerdegrees );
            pTransform->UpdateTransform();
        }
        else
        {
            MyMatrix newRotation;

            newRotation.CreateRotation( m_AmountRotated * -1 );
            pTransform->Rotate( &newRotation, m_WorldSpacePivot );
        }
    }
}

EditorCommand* EditorCommand_RotateObjects::Repeat()
{
    EditorCommand_RotateObjects* pCommand;
    pCommand = MyNew EditorCommand_RotateObjects( *this );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_DeleteObjects
//====================================================================================================

EditorCommand_DeleteObjects::EditorCommand_DeleteObjects(const std::vector<GameObject*>& selectedobjects)
{
    m_Name = "EditorCommand_DeleteObjects";

    MyAssert( selectedobjects.size() > 0 );

    for( unsigned int i=0; i<selectedobjects.size(); i++ )
    {
        m_PreviousGameObjectsInObjectList.push_back( (GameObject*)selectedobjects[i]->GetPrev() );
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
    g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();

    for( unsigned int i=0; i<m_ObjectsDeleted.size(); i++ )
    {
        m_ObjectsDeleted[i]->UnregisterAllComponentCallbacks( true );
        m_ObjectsDeleted[i]->SetEnabled( false );
        g_pComponentSystemManager->UnmanageGameObject( m_ObjectsDeleted[i] );
        m_ObjectsDeleted[i]->GetSceneInfo()->m_GameObjects.MoveTail( m_ObjectsDeleted[i] );
        m_ObjectsDeleted[i]->NotifyOthersThisWasDeleted();
    }
    m_DeleteGameObjectsWhenDestroyed = true;
}

void EditorCommand_DeleteObjects::Undo()
{
    g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();

    for( unsigned int i=0; i<m_ObjectsDeleted.size(); i++ )
    {
        // Place gameobject in old spot in tree.
        if( m_PreviousGameObjectsInObjectList[i] == 0 )
        {
            if( m_ObjectsDeleted[i]->GetParentGameObject() )
            {
                m_ObjectsDeleted[i]->GetParentGameObject()->GetChildList()->MoveHead( m_ObjectsDeleted[i] );
            }
            else
            {
                m_ObjectsDeleted[i]->GetSceneInfo()->m_GameObjects.MoveHead( m_ObjectsDeleted[i] );
            }
        }
        else
        {
            m_ObjectsDeleted[i]->MoveAfter( m_PreviousGameObjectsInObjectList[i] );
        }

        // Undo everything we did to "delete" this object
        g_pComponentSystemManager->ManageGameObject( m_ObjectsDeleted[i] );
        m_ObjectsDeleted[i]->SetEnabled( true );
        m_ObjectsDeleted[i]->RegisterAllComponentCallbacks( false );

        // Place gameobject in old spot in tree.
        if( m_ObjectsDeleted[i]->Prev && m_ObjectsDeleted[i]->GetPrev() != 0 )
        {
            g_pPanelObjectList->Tree_MoveObject( m_ObjectsDeleted[i], m_ObjectsDeleted[i]->GetPrev(), false );
        }
        else
        {
            if( m_ObjectsDeleted[i]->GetParentGameObject() )
            {
                g_pPanelObjectList->Tree_MoveObject( m_ObjectsDeleted[i], m_ObjectsDeleted[i]->GetParentGameObject(), true );
            }
            else
            {
                wxTreeItemId treeidtomove = g_pPanelObjectList->FindObject( m_ObjectsDeleted[i] );
                wxTreeItemId rootid = g_pComponentSystemManager->GetTreeIDForScene( m_ObjectsDeleted[i]->GetSceneID() );
                g_pPanelObjectList->Tree_MoveObject( treeidtomove, rootid, true );
            }
        }
    }

    m_DeleteGameObjectsWhenDestroyed = false;
}

EditorCommand* EditorCommand_DeleteObjects::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_DeleteComponents
//====================================================================================================

EditorCommand_DeleteComponents::EditorCommand_DeleteComponents(const std::vector<ComponentBase*>& selectedcomponents)
{
    m_Name = "EditorCommand_DeleteComponents";

    for( unsigned int i=0; i<selectedcomponents.size(); i++ )
    {
        // can't delete an objects transform component.
        //if( selectedcomponents[i]->m_pGameObject &&
        //    selectedcomponents[i] == selectedcomponents[i]->m_pGameObject->m_pComponentTransform )
        //{
        //    MyAssert( false );
        //}
        
        m_ComponentsDeleted.push_back( selectedcomponents[i] );
    }

    m_DeleteComponentsWhenDestroyed = false;
}

EditorCommand_DeleteComponents::~EditorCommand_DeleteComponents()
{
    if( m_DeleteComponentsWhenDestroyed )
    {
        for( unsigned int i=0; i<m_ComponentsDeleted.size(); i++ )
        {
            g_pComponentSystemManager->DeleteComponent( m_ComponentsDeleted[i] );
        }
    }
}

void EditorCommand_DeleteComponents::Do()
{
    g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();

    for( unsigned int i=0; i<m_ComponentsDeleted.size(); i++ )
    {
        m_ComponentsDeleted[i]->m_pGameObject->RemoveComponent( m_ComponentsDeleted[i] );
    }
    m_DeleteComponentsWhenDestroyed = true;
}

void EditorCommand_DeleteComponents::Undo()
{
    g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();

    for( unsigned int i=0; i<m_ComponentsDeleted.size(); i++ )
    {
        m_ComponentsDeleted[i]->m_pGameObject->AddExistingComponent( m_ComponentsDeleted[i], false );
    }
    m_DeleteComponentsWhenDestroyed = false;
}

EditorCommand* EditorCommand_DeleteComponents::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_CreateGameObject
//====================================================================================================

EditorCommand_CreateGameObject::EditorCommand_CreateGameObject(GameObject* objectcreated)
{
    m_Name = "EditorCommand_CreateGameObject";

    MyAssert( m_ObjectCreated );
    m_ObjectCreated = objectcreated;
}

EditorCommand_CreateGameObject::~EditorCommand_CreateGameObject()
{
    g_pComponentSystemManager->DeleteGameObject( m_ObjectCreated, true );
}

void EditorCommand_CreateGameObject::Do()
{
    g_pComponentSystemManager->ManageGameObject( m_ObjectCreated );
}

void EditorCommand_CreateGameObject::Undo()
{
    g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();

    g_pComponentSystemManager->UnmanageGameObject( m_ObjectCreated );
    m_ObjectCreated->SetEnabled( false );
}

EditorCommand* EditorCommand_CreateGameObject::Repeat()
{
    EditorCommand_CopyGameObject* pCommand;
    pCommand = MyNew EditorCommand_CopyGameObject( m_ObjectCreated, false );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_CopyGameObject
//====================================================================================================

EditorCommand_CopyGameObject::EditorCommand_CopyGameObject(GameObject* objecttocopy, bool NewObjectInheritsFromOld)
{
    m_Name = "EditorCommand_CopyGameObject";

    m_ObjectToCopy = objecttocopy;
    m_ObjectCreated = 0;
    m_DeleteGameObjectWhenDestroyed = false;
    m_NewObjectInheritsFromOld = NewObjectInheritsFromOld;
}

EditorCommand_CopyGameObject::~EditorCommand_CopyGameObject()
{
    MyAssert( m_ObjectCreated );
    if( m_DeleteGameObjectWhenDestroyed && m_ObjectCreated )
        g_pComponentSystemManager->DeleteGameObject( m_ObjectCreated, true );
}

void CreateUniqueName(char* newname, int SizeInBytes, const char* oldname)
{
    int oldnamelen = (int)strlen( oldname );

    // find number at end of string
    int indexofnumber = -1;
    {
        for( int i=oldnamelen-1; i>=0; i-- )
        {
            if( oldname[i] < '0' || oldname[i] > '9' )
            {
                if( i != oldnamelen-1 )
                {
                    indexofnumber = i+1;
                }
                break;
            }
        }
    }

    // find the old number, or 0 if one didn't exist
    int number = 0;
    if( indexofnumber != -1 )
        number = atoi( &oldname[indexofnumber] );

    // if the string didn't end with a number, set the offset to the end of the string
    if( indexofnumber == -1 )
        indexofnumber = oldnamelen;

    // keep incrementing number until unique name is found
    do
    {
        number += 1;
        snprintf_s( newname, SizeInBytes, indexofnumber, "%s", oldname );
        snprintf_s( newname+indexofnumber, SizeInBytes-indexofnumber, SizeInBytes-1-indexofnumber, "%d", number );
    } while( g_pComponentSystemManager->FindGameObjectByName( newname ) != 0 );
}

void EditorCommand_CopyGameObject::Do()
{
    if( m_ObjectCreated == 0 )
    {
        char newname[50];
        const char* oldname = m_ObjectToCopy->GetName();

        if( m_NewObjectInheritsFromOld == false ) // if making a copy
        {
            CreateUniqueName( newname, 50, oldname );
            m_ObjectCreated = g_pComponentSystemManager->CopyGameObject( m_ObjectToCopy, newname );
        }
        else // if making a child object
        {
            snprintf_s( newname, 50, 49, "%s - child", m_ObjectToCopy->GetName() );
            m_ObjectCreated = g_pComponentSystemManager->CopyGameObject( m_ObjectToCopy, newname );
        }
    }
    else
    {
        g_pComponentSystemManager->ManageGameObject( m_ObjectCreated );
        m_ObjectCreated->SetEnabled( m_ObjectToCopy->IsEnabled() );
    }

    // if done/redone, then object exists in the scene, don't destroy it if undo stack get wiped.
    m_DeleteGameObjectWhenDestroyed = false;
}

void EditorCommand_CopyGameObject::Undo()
{
    g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();

    g_pComponentSystemManager->UnmanageGameObject( m_ObjectCreated );
    m_ObjectCreated->SetEnabled( false );

    // if undone and redo stack gets wiped, then object only exist here, destroy it when this command gets deleted.
    m_DeleteGameObjectWhenDestroyed = true;
}

EditorCommand* EditorCommand_CopyGameObject::Repeat()
{
    EditorCommand_CopyGameObject* pCommand;
    pCommand = MyNew EditorCommand_CopyGameObject( m_ObjectToCopy, m_NewObjectInheritsFromOld );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_EnableObject
//====================================================================================================

EditorCommand_EnableObject::EditorCommand_EnableObject(GameObject* pObject, bool enabled)
{
    m_Name = "EditorCommand_EnableObject";

    m_pGameObject = pObject;
    m_ObjectWasEnabled = enabled;
}

EditorCommand_EnableObject::~EditorCommand_EnableObject()
{
}

void EditorCommand_EnableObject::Do()
{
    m_pGameObject->SetEnabled( m_ObjectWasEnabled );
    g_pPanelWatch->SetNeedsRefresh();
}

void EditorCommand_EnableObject::Undo()
{
    m_pGameObject->SetEnabled( !m_ObjectWasEnabled );
    g_pPanelWatch->SetNeedsRefresh();
}

EditorCommand* EditorCommand_EnableObject::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_ChangeMaterialOnMesh
//====================================================================================================

EditorCommand_ChangeMaterialOnMesh::EditorCommand_ChangeMaterialOnMesh(ComponentRenderable* pComponent, ComponentVariable* pVar, int submeshindex, MaterialDefinition* pMaterial)
{
    m_Name = "EditorCommand_ChangeMaterialOnMesh";

    MyAssert( m_pComponent );

    m_pComponent = pComponent;
    m_SubmeshIndex = submeshindex;
    m_pNewMaterial = pMaterial;

    m_pVar = pVar;
    m_VariableWasDivorced = pComponent->IsDivorced( m_pVar->m_Index );
}

EditorCommand_ChangeMaterialOnMesh::~EditorCommand_ChangeMaterialOnMesh()
{
}

void EditorCommand_ChangeMaterialOnMesh::Do()
{
    m_pOldMaterial = m_pComponent->GetMaterial( m_SubmeshIndex );

    m_pComponent->SetMaterial( m_pNewMaterial, m_SubmeshIndex );

    g_pPanelWatch->SetNeedsRefresh();
}

void EditorCommand_ChangeMaterialOnMesh::Undo()
{
    m_pComponent->SetMaterial( m_pOldMaterial, m_SubmeshIndex );
    g_pPanelWatch->SetNeedsRefresh();
}

EditorCommand* EditorCommand_ChangeMaterialOnMesh::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_ChangeTextureOnMaterial
//====================================================================================================

EditorCommand_ChangeTextureOnMaterial::EditorCommand_ChangeTextureOnMaterial(MaterialDefinition* material, TextureDefinition* texture)
{
    m_Name = "EditorCommand_ChangeTextureOnMaterial";

    m_pMaterial = material;
    m_pNewTexture = texture;
}

EditorCommand_ChangeTextureOnMaterial::~EditorCommand_ChangeTextureOnMaterial()
{
}

void EditorCommand_ChangeTextureOnMaterial::Do()
{
    m_pOldTexture = m_pMaterial->GetTextureColor();

    m_pMaterial->SetTextureColor( m_pNewTexture );
}

void EditorCommand_ChangeTextureOnMaterial::Undo()
{
    m_pMaterial->SetTextureColor( m_pOldTexture );
}

EditorCommand* EditorCommand_ChangeTextureOnMaterial::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_ChangeShaderOnMaterial
//====================================================================================================

EditorCommand_ChangeShaderOnMaterial::EditorCommand_ChangeShaderOnMaterial(MaterialDefinition* material, ShaderGroup* shadergroup)
{
    m_Name = "EditorCommand_ChangeShaderOnMaterial";

    m_pMaterial = material;
    m_pNewShaderGroup = shadergroup;
}

EditorCommand_ChangeShaderOnMaterial::~EditorCommand_ChangeShaderOnMaterial()
{
}

void EditorCommand_ChangeShaderOnMaterial::Do()
{
    m_pOldShaderGroup = m_pMaterial->GetShader();

    m_pMaterial->SetShader( m_pNewShaderGroup );
}

void EditorCommand_ChangeShaderOnMaterial::Undo()
{
    m_pMaterial->SetShader( m_pOldShaderGroup );
}

EditorCommand* EditorCommand_ChangeShaderOnMaterial::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_ChangeAllScriptsOnGameObject
//====================================================================================================

EditorCommand_ChangeAllScriptsOnGameObject::EditorCommand_ChangeAllScriptsOnGameObject(GameObject* object, MyFileObject* scriptfile)
{
    m_Name = "EditorCommand_ChangeAllScriptsOnGameObject";

    m_pGameObject = object;
    m_pNewScriptFile = scriptfile;
}

EditorCommand_ChangeAllScriptsOnGameObject::~EditorCommand_ChangeAllScriptsOnGameObject()
{
}

void EditorCommand_ChangeAllScriptsOnGameObject::Do()
{
    for( unsigned int i=0; i<m_pGameObject->GetComponentCount(); i++ )
    {
        ComponentLuaScript* pLuaComponent = dynamic_cast<ComponentLuaScript*>( m_pGameObject->GetComponentByIndex( i ) );

        if( pLuaComponent )
        {
            m_ComponentsChanged.push_back( pLuaComponent );
            m_OldScriptFiles.push_back( pLuaComponent->GetScriptFile() );
        }
    }

    m_pGameObject->SetScriptFile( m_pNewScriptFile );
}

void EditorCommand_ChangeAllScriptsOnGameObject::Undo()
{
    for( unsigned int i=0; i<m_ComponentsChanged.size(); i++ )
    {
        ComponentLuaScript* pLuaComponent = dynamic_cast<ComponentLuaScript*>( m_ComponentsChanged[i] );
        MyAssert( pLuaComponent );

        if( pLuaComponent )
        {
            pLuaComponent->SetScriptFile( m_OldScriptFiles[i] );
        }
    }
}

EditorCommand* EditorCommand_ChangeAllScriptsOnGameObject::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_ChangeSoundCue
//====================================================================================================

EditorCommand_ChangeSoundCue::EditorCommand_ChangeSoundCue(ComponentAudioPlayer* pComponent, SoundCue* pSoundCue)
{
    m_Name = "EditorCommand_ChangeSoundCue";

    m_pComponent = pComponent;
    m_pSoundCue = pSoundCue;

    m_pOldSoundCue = pComponent->GetSoundCue();
}

EditorCommand_ChangeSoundCue::~EditorCommand_ChangeSoundCue()
{
}

void EditorCommand_ChangeSoundCue::Do()
{
    m_pComponent->SetSoundCue( m_pSoundCue );
}

void EditorCommand_ChangeSoundCue::Undo()
{
    m_pComponent->SetSoundCue( m_pOldSoundCue );
}

EditorCommand* EditorCommand_ChangeSoundCue::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_Move2DPoint
//====================================================================================================

EditorCommand_Move2DPoint::EditorCommand_Move2DPoint(b2Vec2 distancemoved, Component2DCollisionObject* pCollisionObject, int indexmoved)
{
    m_Name = "EditorCommand_Move2DPoint";

    MyAssert( m_pCollisionObject );
    MyAssert( indexmoved >= 0 && indexmoved < (int)pCollisionObject->m_Vertices.size() );

    m_pCollisionObject = pCollisionObject;
    m_DistanceMoved = distancemoved;
    m_IndexOfPointMoved = indexmoved;

    //LOGInfo( LOGTag, "EditorCommand_Move2DPoint:: %f,%f\n", m_DistanceMoved.x, m_DistanceMoved.y );
}

EditorCommand_Move2DPoint::~EditorCommand_Move2DPoint()
{
}

void EditorCommand_Move2DPoint::Do()
{
    m_pCollisionObject->m_Vertices[m_IndexOfPointMoved].x += m_DistanceMoved.x;
    m_pCollisionObject->m_Vertices[m_IndexOfPointMoved].y += m_DistanceMoved.y;
}

void EditorCommand_Move2DPoint::Undo()
{
    m_pCollisionObject->m_Vertices[m_IndexOfPointMoved].x -= m_DistanceMoved.x;
    m_pCollisionObject->m_Vertices[m_IndexOfPointMoved].y -= m_DistanceMoved.y;
}

EditorCommand* EditorCommand_Move2DPoint::Repeat()
{
    EditorCommand_Move2DPoint* pCommand;
    pCommand = MyNew EditorCommand_Move2DPoint( *this );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_Insert2DPoint
//====================================================================================================

EditorCommand_Insert2DPoint::EditorCommand_Insert2DPoint(Component2DCollisionObject* pCollisionObject, int indexinserted)
{
    m_Name = "EditorCommand_Insert2DPoint";

    MyAssert( m_pCollisionObject );
    MyAssert( indexinserted >= 0 && indexinserted < (int)pCollisionObject->m_Vertices.size() );

    m_pCollisionObject = pCollisionObject;
    m_IndexOfPointInserted = indexinserted;
}

EditorCommand_Insert2DPoint::~EditorCommand_Insert2DPoint()
{
}

void EditorCommand_Insert2DPoint::Do()
{
    std::vector<b2Vec2>::iterator it = m_pCollisionObject->m_Vertices.begin();
    m_pCollisionObject->m_Vertices.insert( it + m_IndexOfPointInserted, m_pCollisionObject->m_Vertices[m_IndexOfPointInserted] );
}

void EditorCommand_Insert2DPoint::Undo()
{
    std::vector<b2Vec2>::iterator it = m_pCollisionObject->m_Vertices.begin();
    m_pCollisionObject->m_Vertices.erase( it + m_IndexOfPointInserted );
}

EditorCommand* EditorCommand_Insert2DPoint::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_Delete2DPoint
//====================================================================================================

EditorCommand_Delete2DPoint::EditorCommand_Delete2DPoint(Component2DCollisionObject* pCollisionObject, int indexdeleted, b2Vec2 position)
{
    m_Name = "EditorCommand_Delete2DPoint";

    MyAssert( m_pCollisionObject );
    MyAssert( indexdeleted >= 0 && indexdeleted < (int)pCollisionObject->m_Vertices.size() );

    m_pCollisionObject = pCollisionObject;
    m_IndexOfPointDeleted = indexdeleted;
    m_Position = position;
}

EditorCommand_Delete2DPoint::~EditorCommand_Delete2DPoint()
{
}

void EditorCommand_Delete2DPoint::Do()
{
    std::vector<b2Vec2>::iterator it = m_pCollisionObject->m_Vertices.begin();
    m_pCollisionObject->m_Vertices.erase( it + m_IndexOfPointDeleted );
}

void EditorCommand_Delete2DPoint::Undo()
{
    std::vector<b2Vec2>::iterator it = m_pCollisionObject->m_Vertices.begin();
    m_pCollisionObject->m_Vertices.insert( it + m_IndexOfPointDeleted, m_Position );
}

EditorCommand* EditorCommand_Delete2DPoint::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_ComponentVariablePointerChanged
//====================================================================================================

EditorCommand_ComponentVariablePointerChanged::EditorCommand_ComponentVariablePointerChanged(ComponentBase* pComponent, ComponentVariable* pVar, ComponentVariableValue* pNewValue)
{
    m_Name = "EditorCommand_ComponentVariablePointerChanged";

    MyAssert( pComponent && pVar );

    m_pComponent = pComponent;
    m_pVar = pVar;

    m_NewPointer = *pNewValue;
    m_OldPointer.GetValueFromVariable( pComponent, pVar );
}

EditorCommand_ComponentVariablePointerChanged::~EditorCommand_ComponentVariablePointerChanged()
{
}

void EditorCommand_ComponentVariablePointerChanged::Do()
{
    g_pPanelWatch->UpdatePanel();

    // this could likely be dangerous, the object might not be in focus anymore and how it handles callbacks could cause issues.
    (m_pComponent->*(m_pVar->m_pOnValueChangedCallbackFunc))( m_pVar, false, true, 0, &m_NewPointer );
}

void EditorCommand_ComponentVariablePointerChanged::Undo()
{
    g_pPanelWatch->UpdatePanel();

    // this could likely be dangerous, the object might not be in focus anymore and how it handles callbacks could cause issues.
    (m_pComponent->*(m_pVar->m_pOnValueChangedCallbackFunc))( m_pVar, false, true, 0, &m_OldPointer );
}

EditorCommand* EditorCommand_ComponentVariablePointerChanged::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_LuaExposedVariablePointerChanged
//====================================================================================================

EditorCommand_LuaExposedVariablePointerChanged::EditorCommand_LuaExposedVariablePointerChanged(void* newvalue, ExposedVariableDesc* pVar, LuaExposedVarValueChangedCallback callbackfunc, void* callbackobj)
{
    m_Name = "EditorCommand_LuaExposedVariablePointerChanged";

    m_NewValue = newvalue;
    m_pVar = pVar;

    m_OldValue = pVar->pointer;

    m_pOnValueChangedCallBackFunc = callbackfunc;
    m_pCallbackObj = callbackobj;
}

EditorCommand_LuaExposedVariablePointerChanged::~EditorCommand_LuaExposedVariablePointerChanged()
{
}

void EditorCommand_LuaExposedVariablePointerChanged::Do()
{
    m_pVar->pointer = m_NewValue;

    g_pPanelWatch->UpdatePanel();

    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, m_pVar, 0, true, 0, m_OldValue );
}

void EditorCommand_LuaExposedVariablePointerChanged::Undo()
{
    m_pVar->pointer = m_OldValue;

    g_pPanelWatch->UpdatePanel();

    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, m_pVar, 0, true, 0, m_NewValue );
}

EditorCommand* EditorCommand_LuaExposedVariablePointerChanged::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_DeletePrefabs
//====================================================================================================

EditorCommand_DeletePrefabs::EditorCommand_DeletePrefabs(const std::vector<PrefabObject*>& selectedprefabs)
{
    m_Name = "EditorCommand_DeletePrefabs";

    MyAssert( selectedprefabs.size() > 0 );

    for( unsigned int i=0; i<selectedprefabs.size(); i++ )
    {
        PrefabObject* pPrefab = selectedprefabs[i];

        PrefabInfo info;
        info.m_pPrefab = pPrefab;
        info.m_pPreviousPrefabInObjectList = (PrefabObject*)pPrefab->GetPrev();
        g_pComponentSystemManager->Editor_GetListOfGameObjectsThatUsePrefab( &info.m_pListOfGameObjectsThatUsedPrefab, pPrefab );

        m_PrefabInfo.push_back( info );
    }

    m_DeletePrefabsWhenDestroyed = false;
}

EditorCommand_DeletePrefabs::~EditorCommand_DeletePrefabs()
{
    if( m_DeletePrefabsWhenDestroyed )
    {
        for( unsigned int i=0; i<m_PrefabInfo.size(); i++ )
        {
            delete m_PrefabInfo[i].m_pPrefab;
        }
    }
}

void EditorCommand_DeletePrefabs::Do()
{
    g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();

    for( unsigned int i=0; i<m_PrefabInfo.size(); i++ )
    {
        PrefabObject* pPrefab = m_PrefabInfo[i].m_pPrefab;
        PrefabFile* pFile = pPrefab->GetPrefabFile();

        // Loop through gameobjects and unset which prefab they inherit from.
        for( unsigned int j=0; j<m_PrefabInfo[i].m_pListOfGameObjectsThatUsedPrefab.size(); j++ )
        {
            m_PrefabInfo[i].m_pListOfGameObjectsThatUsedPrefab[j]->Editor_SetPrefab( 0 );
        }
        
        // Remove prefab from PrefabFile
        pFile->RemovePrefab( pPrefab );
    }

    m_DeletePrefabsWhenDestroyed = true;
}

void EditorCommand_DeletePrefabs::Undo()
{
    g_pEngineCore->m_pEditorState->ClearSelectedObjectsAndComponents();

    for( unsigned int i=0; i<m_PrefabInfo.size(); i++ )
    {
        PrefabObject* pPrefab = m_PrefabInfo[i].m_pPrefab;
        PrefabObject* pPreviousPrefab = m_PrefabInfo[i].m_pPreviousPrefabInObjectList;
        PrefabFile* pFile = pPrefab->GetPrefabFile();

        // Place prefab in old spot in PrefabFile and object list
        pFile->AddExistingPrefab( pPrefab, pPreviousPrefab );

        // Loop through gameobjects and reset which prefab they inherited from.
        for( unsigned int j=0; j<m_PrefabInfo[i].m_pListOfGameObjectsThatUsedPrefab.size(); j++ )
        {
            m_PrefabInfo[i].m_pListOfGameObjectsThatUsedPrefab[j]->Editor_SetPrefab( pPrefab );
        }
    }

    m_DeletePrefabsWhenDestroyed = false;
}

EditorCommand* EditorCommand_DeletePrefabs::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_DivorceOrMarryComponentVariable
//====================================================================================================

EditorCommand_DivorceOrMarryComponentVariable::EditorCommand_DivorceOrMarryComponentVariable(ComponentBase* pComponent, ComponentVariable* pVar, bool divorcethevariable)
{
    m_Name = "EditorCommand_DivorceOrMarryComponentVariable";

    MyAssert( pComponent && pVar );

    m_pComponent = pComponent;
    m_pVar = pVar;

    if( divorcethevariable )
    {
        MyAssert( m_pComponent->IsDivorced( pVar->m_Index ) == false );
    }
    else
    {
        MyAssert( m_pComponent->IsDivorced( pVar->m_Index ) == true );
    }

    m_DivorceTheVariable = divorcethevariable;

    m_OldValue.GetValueFromVariable( pComponent, pVar );
}

EditorCommand_DivorceOrMarryComponentVariable::~EditorCommand_DivorceOrMarryComponentVariable()
{
}

void EditorCommand_DivorceOrMarryComponentVariable::Do()
{
    if( m_DivorceTheVariable )
    {
        // Divorce the variables.
        m_pComponent->SetDivorced( m_pVar->m_Index, true );
        //if( m_pVar->m_ControlID >= 0 )
        //{
        //    g_pPanelWatch->ChangeStaticTextFontStyle( m_pVar->m_ControlID, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD );
        //    g_pPanelWatch->ChangeStaticTextBGColor( m_pVar->m_ControlID, wxColour( 255, 200, 200, 255 ) );
        //}

        g_pPanelWatch->SetNeedsRefresh();
    }
    else
    {
        // Marry the variables.
        m_pComponent->SetDivorced( m_pVar->m_Index, false );
        //if( m_pVar->m_ControlID >= 0 )
        //{
        //    g_pPanelWatch->ChangeStaticTextFontStyle( m_pVar->m_ControlID, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
        //    g_pPanelWatch->ChangeStaticTextBGColor( m_pVar->m_ControlID, wxNullColour );
        //}

        // Set this components value to the parent's value.
        ComponentBase* pParentComponent = m_pComponent->FindMatchingComponentInParent();
        MyAssert( pParentComponent );
        if( pParentComponent )
        {
            // Get parent objects value.
            ComponentVariableValue newvalue( pParentComponent, m_pVar );

            // Update and inform component and children.
            newvalue.UpdateComponentAndChildrenWithValue( m_pComponent, m_pVar );
        }

        g_pPanelWatch->SetNeedsRefresh();
    }
}

void EditorCommand_DivorceOrMarryComponentVariable::Undo()
{
    // Do the opposite
    if( m_DivorceTheVariable )
    {
        // Marry the variables.
        m_pComponent->SetDivorced( m_pVar->m_Index, false );
        //if( m_pVar->m_ControlID >= 0 )
        //{
        //    g_pPanelWatch->ChangeStaticTextFontStyle( m_pVar->m_ControlID, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
        //    g_pPanelWatch->ChangeStaticTextBGColor( m_pVar->m_ControlID, wxNullColour );
        //}

        g_pPanelWatch->SetNeedsRefresh();
    }
    else
    {
        // Divorce the variables.
        m_pComponent->SetDivorced( m_pVar->m_Index, true );
        //if( m_pVar->m_ControlID >= 0 )
        //{
        //    g_pPanelWatch->ChangeStaticTextFontStyle( m_pVar->m_ControlID, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD );
        //    g_pPanelWatch->ChangeStaticTextBGColor( m_pVar->m_ControlID, wxColour( 255, 200, 200, 255 ) );
        //}

        // Update and inform component and children.
        m_OldValue.UpdateComponentAndChildrenWithValue( m_pComponent, m_pVar );

        g_pPanelWatch->SetNeedsRefresh();
    }
}

EditorCommand* EditorCommand_DivorceOrMarryComponentVariable::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_ComponentVariableIndirectPointerChanged
//====================================================================================================

EditorCommand_ComponentVariableIndirectPointerChanged::EditorCommand_ComponentVariableIndirectPointerChanged(ComponentBase* pComponent, ComponentVariable* pVar, void* newvalue)
{
    m_Name = "EditorCommand_ComponentVariableIndirectPointerChanged";

    MyAssert( pComponent && pVar );

    m_pComponent = pComponent;
    m_pVar = pVar;

    m_OldValue = (pComponent->*(pVar->m_pGetPointerValueCallBackFunc))( pVar );
    m_NewValue = newvalue;
}

EditorCommand_ComponentVariableIndirectPointerChanged::~EditorCommand_ComponentVariableIndirectPointerChanged()
{
}

void EditorCommand_ComponentVariableIndirectPointerChanged::Do()
{
    // Set the value in the component.
    (m_pComponent->*(m_pVar->m_pSetPointerValueCallBackFunc))( m_pVar, m_NewValue );

    g_pPanelWatch->SetNeedsRefresh();
}

void EditorCommand_ComponentVariableIndirectPointerChanged::Undo()
{
    // Set the value in the component.
    (m_pComponent->*(m_pVar->m_pSetPointerValueCallBackFunc))( m_pVar, m_OldValue );

    g_pPanelWatch->SetNeedsRefresh();
}

EditorCommand* EditorCommand_ComponentVariableIndirectPointerChanged::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_ReorderOrReparentGameObjects
//====================================================================================================

EditorCommand_ReorderOrReparentGameObjects::EditorCommand_ReorderOrReparentGameObjects(const std::vector<GameObject*>& selectedobjects, GameObject* pObjectDroppedOn, uint32 sceneid, bool setaschild)
{
    m_Name = "EditorCommand_ReorderOrReparentGameObjects";

    for( unsigned int i=0; i<selectedobjects.size(); i++ )
    {
        m_SelectedObjects.push_back( selectedobjects[i] );

        m_OldSceneIDs.push_back( selectedobjects[i]->GetSceneID() );
        m_OldPreviousObjectInList.push_back( (GameObject*)selectedobjects[i]->GetPrev() );
        m_OldParent.push_back( selectedobjects[i]->GetParentGameObject() );
    }

    m_pObjectDroppedOn = pObjectDroppedOn;
    m_SceneIDDroppedOn = sceneid;
    m_MakeSelectedObjectsChildren = setaschild;

    MyAssert( m_pObjectDroppedOn == 0 || m_pObjectDroppedOn->GetSceneID() == sceneid );
}

EditorCommand_ReorderOrReparentGameObjects::~EditorCommand_ReorderOrReparentGameObjects()
{
}

void EditorCommand_ReorderOrReparentGameObjects::Do()
{
    // Move/Reparent all of the selected items.
    for( int i=m_SelectedObjects.size()-1; i>=0; i-- )
    {
        GameObject* pGameObject = (GameObject*)m_SelectedObjects[i];

        // Change the selected gameobject's sceneid to match the one dropped on.
        pGameObject->SetSceneID( m_SceneIDDroppedOn );

        if( m_MakeSelectedObjectsChildren == false )
        {
            // Move below the selected item.
            g_pPanelObjectList->Tree_MoveObject( pGameObject, m_pObjectDroppedOn, false );
            pGameObject->MoveAfter( m_pObjectDroppedOn );
            GameObject* thisparent = m_pObjectDroppedOn->GetParentGameObject();
            pGameObject->SetParentGameObject( thisparent );
        }
        else
        {
            if( m_pObjectDroppedOn )
            {
                // Parent the object dropped to this.
                pGameObject->SetParentGameObject( m_pObjectDroppedOn );

                // Move as first item in parent.
                g_pPanelObjectList->Tree_MoveObject( pGameObject, m_pObjectDroppedOn, true );
            }
            else
            {
                // If no m_pObjectDroppedOn, then we'll add these objects as the first objects of the scene.

                // We dropped it directly on the scene, so it shouldn't have a parent anymore.
                pGameObject->SetParentGameObject( 0 );

                SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( m_SceneIDDroppedOn );
                pSceneInfo->m_GameObjects.MoveHead( pGameObject );

                // Move the wx tree item to the correct spot.
                wxTreeItemId treeidtomove = g_pPanelObjectList->FindObject( pGameObject );
                g_pPanelObjectList->Tree_MoveObject( treeidtomove, pSceneInfo->m_TreeID, true );
            }
        }
    }
}

void EditorCommand_ReorderOrReparentGameObjects::Undo()
{
    // Move/Reparent all of the selected items.
    for( unsigned int i=0; i<m_SelectedObjects.size(); i++ )
    {
        GameObject* pGameObject = (GameObject*)m_SelectedObjects[i];

        // Change the selected gameobject's sceneid back to it's original.
        pGameObject->SetSceneID( m_OldSceneIDs[i] );

        // If this wasn't the first child of a parent object.
        if( m_OldPreviousObjectInList[i] != 0 )
        {
            // Move back to old position.
            g_pPanelObjectList->Tree_MoveObject( pGameObject, m_OldPreviousObjectInList[i], false );
            pGameObject->SetParentGameObject( m_OldParent[i] );
            pGameObject->MoveAfter( m_OldPreviousObjectInList[i] );

            MyAssert( m_OldPreviousObjectInList[i]->GetParentGameObject() == m_OldParent[i] );
        }
        else
        {
            // If this was the first object in the scene.
            if( m_OldParent[i] == 0 )
            {
                // Unparent the gameobject and move it be be first in the scene list.
                pGameObject->SetParentGameObject( 0 );
                g_pComponentSystemManager->GetSceneInfo( m_OldSceneIDs[i] )->m_GameObjects.MoveHead( pGameObject );

                // Move the wx tree item to the correct spot.
                wxTreeItemId treeidtomove = g_pPanelObjectList->FindObject( pGameObject );
                wxTreeItemId scenetreeid = g_pComponentSystemManager->GetSceneInfo( m_OldSceneIDs[i] )->m_TreeID;
                g_pPanelObjectList->Tree_MoveObject( treeidtomove, scenetreeid, true );
            }
            else // This was the first child of a parent.
            {
                // Reset parent to original parent.
                pGameObject->SetParentGameObject( m_OldParent[i] );

                // Move as first item in parent.
                g_pPanelObjectList->Tree_MoveObject( pGameObject, m_OldParent[i], true );
            }
        }
    }
}

EditorCommand* EditorCommand_ReorderOrReparentGameObjects::Repeat()
{
    return 0;
}

//====================================================================================================
//====================================================================================================