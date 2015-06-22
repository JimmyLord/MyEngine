//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

void SceneInfo::ChangePath(const char* newfullpath)
{
    sprintf_s( fullpath, MAX_PATH, "%s", newfullpath ); 
#if MYFW_USING_WX
    if( treeid.IsOk() )
    {
        const char* filenamestart;
        int i;
        for( i=strlen(newfullpath)-1; i>=0; i-- )
        {
            if( newfullpath[i] == '\\' || newfullpath[i] == '/' )
                break;
        }
        filenamestart = &newfullpath[i+1];

        g_pPanelObjectList->RenameObject( treeid, filenamestart );
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
void SceneHandler::OnLeftClick(wxTreeItemId id, unsigned int count, bool clear)
{
}

void SceneHandler::OnRightClick(wxTreeItemId id)
{
 	wxMenu menu;
    menu.SetClientData( this );

    MyAssert( id.IsOk() );
    wxString itemname = g_pPanelObjectList->m_pTree_Objects->GetItemText( id );
    
    m_SceneIDBeingAffected = g_pComponentSystemManager->GetSceneIDFromSceneTreeID( id );
    
    menu.Append( RightClick_AddGameObject, "Add Game Object" );
    menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&SceneHandler::OnPopupClick );

    menu.Append( RightClick_UnloadScene, "Unload scene" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&SceneHandler::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void SceneHandler::OnPopupClick(wxEvent &evt)
{
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
            GameObject* pGameObject = g_pComponentSystemManager->CreateGameObject( true, pSceneHandler->m_SceneIDBeingAffected );
            pGameObject->SetName( "New Game Object" );
        }
        break;
    }
}

void SceneHandler::OnDrag()
{
}

void SceneHandler::OnDrop(wxTreeItemId id, int controlid, wxCoord x, wxCoord y)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        GameObject* pGameObject = (GameObject*)g_DragAndDropStruct.m_Value;

        unsigned int sceneid = g_pComponentSystemManager->GetSceneIDFromSceneTreeID( id );

        pGameObject->SetSceneID( sceneid );
    }
}

void SceneHandler::OnLabelEdit(wxTreeItemId id, wxString newlabel)
{
}
#endif //MYFW_USING_WX
