//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

SceneInfo::SceneInfo()
{
    m_pBox2DWorld = 0;

    Reset();
}

void SceneInfo::Reset()
{
#if MYFW_USING_WX
    m_TreeID.Unset();
#endif

    // Delete any managed objects from the scene.
    while( m_GameObjects.GetHead() )
    {
        GameObject* pObject = (GameObject*)m_GameObjects.RemHead();

        // Ignore unmanaged object, they should be deleted elsewhere.
        if( pObject->IsManaged() == false )
            continue;

        delete pObject;
    }

    SAFE_DELETE( m_pBox2DWorld );

    m_FullPath[0] = 0;

    m_NextGameObjectID = 1;
    m_NextComponentID = 1;

    m_InUse = false;
}

void SceneInfo::ChangePath(const char* newfullpath)
{
    // Store a relative path, rather than the full path.
    char path[MAX_PATH];
    strcpy_s( path, MAX_PATH, newfullpath );
    const char* relativepath = GetRelativePath( path );

    // If newfullpath was full, store the relative part. Otherwise, assume it was already relative.
    if( relativepath )
        sprintf_s( m_FullPath, MAX_PATH, "%s", relativepath );
    else
        sprintf_s( m_FullPath, MAX_PATH, "%s", newfullpath );

#if MYFW_USING_WX
    if( m_TreeID.IsOk() )
    {
        const char* filenamestart;
        int i;
        for( i=(int)strlen(newfullpath)-1; i>=0; i-- )
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

    if( m_SceneIDBeingAffected != SCENEID_NotFound )
    {
        AddGameObjectMenuOptionsToMenu( &menu, 0, m_SceneIDBeingAffected );
        menu.Append( RightClick_UnloadScene, "Unload scene" );

        menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&SceneHandler::OnPopupClick );

        // blocking call.
        g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
    }
}

void SceneHandler::AddGameObjectMenuOptionsToMenu(wxMenu* menu, int itemidoffset, SceneID sceneid)
{
    m_SceneIDBeingAffected = sceneid;

    menu->Append( itemidoffset + RightClick_AddGameObject, "Add Game Object" );

    wxMenu* templatesmenu = MyNew wxMenu;
    menu->AppendSubMenu( templatesmenu, "Add Game Object Template" );
    AddGameObjectTemplatesToMenu( templatesmenu, itemidoffset + RightClick_AddGameObjectFromTemplate, 0 );

    menu->Append( itemidoffset + RightClick_AddFolder, "Add Folder" );
    menu->Append( itemidoffset + RightClick_AddLogicGameObject, "Add Logical Game Object" );
}

int SceneHandler::AddGameObjectTemplatesToMenu(wxMenu* menu, int itemidoffset, int startindex)
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

            i = AddGameObjectTemplatesToMenu( submenu, itemidoffset, i+1 );
        }
        else
        {
            menu->Append( itemidoffset + i, name );
        }

        i++;
    }

    return i;
}

void SceneHandler::OnPopupClick(wxEvent &evt)
{
    SceneHandler* pSceneHandler = (SceneHandler*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();

    int id = evt.GetId();
    pSceneHandler->HandleRightClickCommand( id, 0 );
}

void SceneHandler::HandleRightClickCommand(int id, GameObject* pParentGameObject)
{
    GameObject* pGameObjectCreated = 0;

    switch( id )
    {
    case RightClick_UnloadScene:
        {
            g_pComponentSystemManager->UnloadScene( m_SceneIDBeingAffected );
        }
        break;

    case RightClick_AddGameObject:
        {
            pGameObjectCreated = g_pComponentSystemManager->CreateGameObject( true, m_SceneIDBeingAffected );
            pGameObjectCreated->SetName( "New Game Object" );
        }
        break;

    case RightClick_AddFolder:
        {
            pGameObjectCreated = g_pComponentSystemManager->CreateGameObject( true, m_SceneIDBeingAffected, true, false );
            pGameObjectCreated->SetName( "New Folder" );
        }
        break;

    case RightClick_AddLogicGameObject:
        {
            pGameObjectCreated = g_pComponentSystemManager->CreateGameObject( true, m_SceneIDBeingAffected, false, false );
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
        pGameObjectCreated = g_pComponentSystemManager->CreateGameObjectFromTemplate( templateid, m_SceneIDBeingAffected );
    }

    if( pGameObjectCreated )
    {
        g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_CreateGameObject( pGameObjectCreated ) );

        if( pParentGameObject )
        {
#if MYFW_USING_WX
            // Move as last item in parent.
            GameObject* pLastChild = (GameObject*)pParentGameObject->GetChildList()->GetTail();
            if( pLastChild != 0 )
                g_pPanelObjectList->Tree_MoveObject( pGameObjectCreated, pLastChild, false );
            else
                g_pPanelObjectList->Tree_MoveObject( pGameObjectCreated, pParentGameObject, true );
#endif

            pGameObjectCreated->SetParentGameObject( pParentGameObject );
        }
    }
}

void SceneHandler::OnDrag()
{
}

void SceneHandler::OnDrop(wxTreeItemId treeid, int controlid, int x, int y)
{
    // Figure out the sceneid for the scene objects were dropped on.
    SceneID sceneid = g_pComponentSystemManager->GetSceneIDFromSceneTreeID( treeid );

    // Don't allow gameobjects or prefabs to be dropped onto "Unmanaged" scene
    if( sceneid == SCENEID_Unmanaged || sceneid == SCENEID_NotFound )
        return;

    // Check for both gameobjects and prefabs, if a mix is found, ignore the prefabs.
    bool foundgameobjects = false;
    bool foundprefabs = false;

    for( unsigned int i=0; i<g_DragAndDropStruct.GetItemCount(); i++ )
    {
        DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( i );

        if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
            foundgameobjects = true;

        if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
            foundprefabs = true;
    }

    if( foundgameobjects )
    {
        std::vector<GameObject*> selectedObjects;

        for( unsigned int i=0; i<g_DragAndDropStruct.GetItemCount(); i++ )
        {
            DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( i );

            if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
            {
                GameObject* pGameObject = (GameObject*)pDropItem->m_Value;

                selectedObjects.push_back( pGameObject );
            }
        }

        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ReorderOrReparentGameObjects( selectedObjects, 0, sceneid, true ) );
    }
    else if( foundprefabs )
    {
        DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

        if( (int)pDropItem->m_Type == (int)DragAndDropTypeEngine_Prefab )
        {
            PrefabObject* pPrefab = (PrefabObject*)pDropItem->m_Value;

            // Don't allow gameobjects to be dropped onto "Unmanaged" scene
            if( sceneid == SCENEID_Unmanaged )
                return;

            // Create the game object
            GameObject* pGameObjectCreated = g_pComponentSystemManager->CreateGameObjectFromPrefab( pPrefab, true, sceneid );

            if( pGameObjectCreated )
            {
                // Undo/Redo
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_CreateGameObject( pGameObjectCreated ) );
            }
        }
    }
}

void SceneHandler::OnLabelEdit(wxTreeItemId treeid, wxString newlabel)
{
}
#endif //MYFW_USING_WX
