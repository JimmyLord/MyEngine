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

ComponentBase::ComponentBase()
: m_BaseType( BaseComponentType_None )
, m_pGameObject( 0 )
, m_Type(-1)
, m_ID(0)
{
}

ComponentBase::ComponentBase(GameObject* owner)
: m_BaseType( BaseComponentType_None )
, m_pGameObject( owner )
, m_Type(-1)
, m_ID(0)
{
}

ComponentBase::~ComponentBase()
{
}

#if MYFW_USING_WX
void ComponentBase::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentBase::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Unknown component" );
    g_pPanelObjectList->SetDragAndDropFunctions( this, ComponentBase::StaticOnDrag, ComponentBase::StaticOnDrop );
}

void ComponentBase::OnLeftClick(bool clear)
{
    if( clear )
        g_pPanelWatch->ClearAllVariables();
}

void ComponentBase::OnRightClick()
{
 	wxMenu menu;
    menu.SetClientData( this );

    menu.Append( 0, "Delete Component" );
 	menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentBase::OnPopupClick );

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void ComponentBase::OnPopupClick(wxEvent &evt)
{
    ComponentBase* pComponent = (ComponentBase*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    int id = evt.GetId();
    if( id == 0 )
    {
        g_pComponentSystemManager->DeleteComponent( pComponent );
    }
}

void ComponentBase::OnDrag()
{
    g_DragAndDropStruct.m_Type = DragAndDropType_ComponentPointer;
    g_DragAndDropStruct.m_Value = this;
}

void ComponentBase::OnDrop()
{
}
#endif //MYFW_USING_WX

cJSON* ComponentBase::ExportAsJSONObject()
{
    cJSON* component = cJSON_CreateObject();

    //cJSON_AddNumberToObject( component, "BaseType", m_BaseType );

    if( m_Type != -1 )
    {
        char* componenttypename = g_pComponentTypeManager->GetTypeName( m_Type );
        assert( componenttypename );
        if( componenttypename )
            cJSON_AddStringToObject( component, "Type", componenttypename );
    }

    if( m_pGameObject )
        cJSON_AddNumberToObject( component, "GOID", m_pGameObject->m_ID );

    cJSON_AddNumberToObject( component, "ID", m_ID );

    return component;
}

void ComponentBase::ImportFromJSONObject(cJSON* jsonobj)
{
}

void ComponentBase::Reset()
{
}

ComponentBase& ComponentBase::operator=(const ComponentBase& other)
{
    assert( &other != this );

    return *this;
}
