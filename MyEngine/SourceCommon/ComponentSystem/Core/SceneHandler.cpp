//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

void SceneInfo::ChangePath(const char* newfullpath)
{
    sprintf_s( m_FullPath, MAX_PATH, "%s", newfullpath ); 

#if MYFW_USING_WX
    if( m_TreeID.IsOk() )
    {
        const char* filenamestart;
        int i;
        for( i=strlen(newfullpath)-1; i>=0; i-- )
        {
            if( newfullpath[i] == '\\' || newfullpath[i] == '/' )
                break;
        }
        filenamestart = &newfullpath[i+1];

        g_pPanelObjectList->RenameObject( m_TreeID, filenamestart );
    }
#endif //MYFW_USING_WX
}

SceneHandler::SceneHandler()
{
}

SceneHandler::~SceneHandler()
{
}

#if MYFW_USING_WX
void SceneHandler::OnLeftClick(wxTreeItemId treeid, unsigned int count, bool clear)
{
}

void SceneHandler::OnRightClick(wxTreeItemId treeid)
{
 	wxMenu menu;
    menu.SetClientData( this );

    MyAssert( treeid.IsOk() );
    wxString itemname = g_pPanelObjectList->m_pTree_Objects->GetItemText( treeid );
    
    m_SceneIDBeingAffected = g_pComponentSystemManager->GetSceneIDFromSceneTreeID( treeid );
    
    menu.Append( RightClick_AddGameObject, "Add Game Object" );

    wxMenu* templatesmenu = MyNew wxMenu;
    menu.AppendSubMenu( templatesmenu, "Add Game Object Template" );
    AddGameObjectTemplatesToMenu( templatesmenu, 0 );

    menu.Append( RightClick_AddFolder, "Add Folder" );
    menu.Append( RightClick_AddLogicGameObject, "Add Logical Game Object" );
    menu.Append( RightClick_UnloadScene, "Unload scene" );

    menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&SceneHandler::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

int SceneHandler::AddGameObjectTemplatesToMenu(wxMenu* menu, int startindex)
{
    GameObjectTemplateManager* pManager = g_pComponentSystemManager->m_pGameObjectTemplateManager;
    
    cJSON* jFirstParent = pManager->GetParentTemplateJSONObject( startindex );

    unsigned int i = startindex;
    while( i < pManager->GetNumberOfTemplates() )
    {
        bool isfolder = pManager->IsTemplateAFolder( i );
        const char* name = pManager->GetTemplateName( i );

        if( pManager->GetParentTemplateJSONObject( i ) != jFirstParent )
            return i;

        if( isfolder )
        {
            wxMenu* submenu = MyNew wxMenu;
            menu->AppendSubMenu( submenu, name );

            i = AddGameObjectTemplatesToMenu( submenu, i+1 );
        }
        else
        {
            menu->Append( RightClick_AddGameObjectFromTemplate + i, name );
        }

        i++;
    }

    return i;
}

void SceneHandler::OnPopupClick(wxEvent &evt)
{
    GameObject* pGameObjectCreated = 0;
    SceneHandler* pSceneHandler = (SceneHandler*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();

    int id = evt.GetId();
    switch( id )
    {
    case RightClick_UnloadScene:
        {
            g_pComponentSystemManager->UnloadScene( pSceneHandler->m_SceneIDBeingAffected );
        }
        break;

    case RightClick_AddGameObject:
        {
            pGameObjectCreated = g_pComponentSystemManager->CreateGameObject( true, pSceneHandler->m_SceneIDBeingAffected );
            pGameObjectCreated->SetName( "New Game Object" );
        }
        break;

    case RightClick_AddFolder:
        {
            pGameObjectCreated = g_pComponentSystemManager->CreateGameObject( true, pSceneHandler->m_SceneIDBeingAffected, true, false );
            pGameObjectCreated->SetName( "New Folder" );
        }
        break;

    case RightClick_AddLogicGameObject:
        {
            pGameObjectCreated = g_pComponentSystemManager->CreateGameObject( true, pSceneHandler->m_SceneIDBeingAffected, false, false );
            pGameObjectCreated->SetName( "New Logical Game Object" );
        }
        break;

    case RightClick_AddGameObjectFromTemplate:
        // handled below
        break;
    }

    if( id >= RightClick_AddGameObjectFromTemplate )
    {
        int templateid = id - RightClick_AddGameObjectFromTemplate;
        pGameObjectCreated = g_pComponentSystemManager->CreateGameObjectFromTemplate( templateid, pSceneHandler->m_SceneIDBeingAffected );
    }

    if( pGameObjectCreated )
    {
        g_pEngineMainFrame->m_pCommandStack->Add( MyNew EditorCommand_CreateGameObject( pGameObjectCreated ) );        
    }
}

void SceneHandler::OnDrag()
{
}

void SceneHandler::OnDrop(wxTreeItemId treeid, int controlid, wxCoord x, wxCoord y)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        GameObject* pGameObject = (GameObject*)g_DragAndDropStruct.m_Value;

        // If we dropped a gameobject on our scene, move the game object to the new scene.
        unsigned int sceneid = g_pComponentSystemManager->GetSceneIDFromSceneTreeID( treeid );

        // Don't allow gameobjects to be dropped onto "Unmanaged" scene
        if( sceneid == 0 )
            return;

        pGameObject->SetSceneID( sceneid );
        wxTreeItemId treeidtomove = g_pPanelObjectList->FindObject( pGameObject );
        g_pPanelObjectList->Tree_MoveObject( treeidtomove, treeid, true );

        // We dropped it directly on the scene, so it shouldn't have a parent anymore.
        pGameObject->SetParentGameObject( 0 );

        GameObject* pFirstGameObject = g_pComponentSystemManager->GetFirstGameObjectFromScene( sceneid );
        if( pFirstGameObject )
        {
            if( pFirstGameObject != pGameObject )
            {
                pGameObject->MoveBefore( pFirstGameObject );
            }
        }
        else
        {
            g_pComponentSystemManager->GetSceneInfo( sceneid )->m_GameObjects.MoveHead( pGameObject );
        }

        SceneInfo* pScene = g_pComponentSystemManager->GetSceneInfo( sceneid );
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropTypeEngine_Prefab )
    {
        PrefabObject* pPrefab = (PrefabObject*)g_DragAndDropStruct.m_Value;

        // If we dropped a gameobject on our scene, move the game object to the new scene.
        unsigned int sceneid = g_pComponentSystemManager->GetSceneIDFromSceneTreeID( treeid );

        // Don't allow gameobjects to be dropped onto "Unmanaged" scene
        if( sceneid == 0 )
            return;

        g_pComponentSystemManager->CreateGameObjectFromPrefab( pPrefab, sceneid );
    }
}

void SceneHandler::OnLabelEdit(wxTreeItemId treeid, wxString newlabel)
{
}
#endif //MYFW_USING_WX
