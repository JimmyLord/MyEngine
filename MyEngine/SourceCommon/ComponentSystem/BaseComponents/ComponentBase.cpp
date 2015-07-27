//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

ComponentBase::ComponentBase()
: m_SceneIDLoadedFrom( 0 )
, m_BaseType( BaseComponentType_None )
, m_pGameObject( 0 )
, m_Type(-1)
, m_ID(0)
, m_Enabled( true )
{
    ClassnameSanityCheck();

#if MYFW_USING_WX
    m_ControlID_ComponentTitleLabel = -1;
    m_pPanelWatchBlockVisible = 0;
#endif
}

ComponentBase::~ComponentBase()
{
    // if it's in a list, remove it.
    if( this->Prev != 0 )
        Remove();
}

void ComponentBase::Reset()
{
#if MYFW_USING_WX
    m_ControlID_ComponentTitleLabel = -1;
    m_pPanelWatchBlockVisible = 0;
#endif
}

void ComponentBase::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentBase>( "ComponentBase" )
            //.addData( "localmatrix", &ComponentBase::m_LocalTransform )
            
            .addFunction( "SetEnabled", &ComponentBase::SetEnabled )
            .addFunction( "IsEnabled", &ComponentBase::IsEnabled )
            
            .addFunction( "SetSceneID", &ComponentBase::SetSceneID )
            .addFunction( "GetSceneID", &ComponentBase::GetSceneID )
            
            .addFunction( "SetID", &ComponentBase::SetID )
            .addFunction( "GetID", &ComponentBase::GetID )
        .endClass();
}

void ComponentBase::SetEnabled(bool enabled)
{
    if( m_Enabled == enabled )
        return;

    m_Enabled = enabled;

    if( enabled )
        RegisterCallbacks();
    else
        UnregisterCallbacks();
}

#if MYFW_USING_WX
void ComponentBase::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentBase::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Unknown component" );
    g_pPanelObjectList->SetDragAndDropFunctions( id, ComponentBase::StaticOnDrag, ComponentBase::StaticOnDrop );
}

void ComponentBase::OnComponentTitleLabelClicked(int id, bool finishedchanging)
{
    if( id != -1 && id == m_ControlID_ComponentTitleLabel )
    {
        if( m_pPanelWatchBlockVisible )
        {
            *m_pPanelWatchBlockVisible = !(*m_pPanelWatchBlockVisible);
            g_pPanelWatch->m_NeedsRefresh = true;
        }
    }
}

void ComponentBase::OnLeftClick(unsigned int count, bool clear)
{
    // select this Component in the editor window.
    g_pEngineCore->m_pEditorState->m_pSelectedComponents.push_back( this );

    if( clear )
        g_pPanelWatch->ClearAllVariables();

    FillPropertiesWindow( clear );
}

void ComponentBase::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );

    AppendItemsToRightClickMenu( &menu );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void ComponentBase::AppendItemsToRightClickMenu(wxMenu* pMenu)
{
    pMenu->Append( 1000, "Delete Component" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentBase::OnPopupClick );
}

void ComponentBase::OnPopupClick(wxEvent &evt)
{
    //ComponentBase* pComponent = (ComponentBase*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    int id = evt.GetId();
    if( id == 1000 )
    {
        EditorState* pEditorState = g_pEngineCore->m_pEditorState;

        // deselect all "main" transform components.
        for( unsigned int i=0; i<pEditorState->m_pSelectedComponents.size(); i++ )
        {
            ComponentBase* pSelComp = pEditorState->m_pSelectedComponents[i];
            if( pSelComp->m_pGameObject && pSelComp == pSelComp->m_pGameObject->m_pComponentTransform )
            {
                pEditorState->m_pSelectedComponents[i] = pEditorState->m_pSelectedComponents.back();
                pEditorState->m_pSelectedComponents.pop_back();
                i--;
            }
        }

        // if anything is still selected, delete it/them.
        if( pEditorState->m_pSelectedComponents.size() > 0 )
        {
            g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_DeleteComponents( pEditorState->m_pSelectedComponents ) );
        }

        //// if the object isn't selected, delete just the one object, otherwise delete all selected objects.
        //if( pEditorState->IsComponentSelected( pComponent ) )
        //{
        //    g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_DeleteComponents( pEditorState->m_pSelectedComponents ) );
        //}
        //else
        //{
        //    // create a temp vector to pass into command.
        //    std::vector<ComponentBase*> components;
        //    components.push_back( pComponent );
        //    g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_DeleteComponents( components ) );
        //}
    }
}

void ComponentBase::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_ComponentPointer;
    g_DragAndDropStruct.m_Value = this;
}

void ComponentBase::OnDrop(int controlid, wxCoord x, wxCoord y)
{
}
#endif //MYFW_USING_WX

cJSON* ComponentBase::ExportAsJSONObject(bool savesceneid)
{
    cJSON* component = cJSON_CreateObject();

    //cJSON_AddNumberToObject( component, "BaseType", m_BaseType );

    if( savesceneid )
        cJSON_AddNumberToObject( component, "SceneID", m_SceneIDLoadedFrom );

    if( m_Type != -1 )
    {
        const char* componenttypename = g_pComponentTypeManager->GetTypeName( m_Type );
        MyAssert( componenttypename );
        if( componenttypename )
            cJSON_AddStringToObject( component, "Type", componenttypename );
    }

    if( m_pGameObject )
        cJSON_AddNumberToObject( component, "GOID", m_pGameObject->GetID() );

    cJSON_AddNumberToObject( component, "ID", m_ID );

    return component;
}

void ComponentBase::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    cJSONExt_GetUnsignedInt( jsonobj, "ID", &m_ID );

    MyAssert( m_SceneIDLoadedFrom == 0 || m_SceneIDLoadedFrom == sceneid );
    SetSceneID( sceneid );
}

ComponentBase& ComponentBase::operator=(const ComponentBase& other)
{
    MyAssert( &other != this );

    return *this;
}

void ComponentBase::OnLoad()
{
    if( m_Enabled && m_pGameObject && m_pGameObject->IsEnabled() )
        RegisterCallbacks();
    else
        UnregisterCallbacks();
}

void ComponentBase::OnGameObjectEnabled()
{
    if( m_Enabled )
        RegisterCallbacks();
}

void ComponentBase::OnGameObjectDisabled()
{
    UnregisterCallbacks();
}
