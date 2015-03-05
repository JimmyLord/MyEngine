//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

#if MYFW_USING_WX
bool ComponentInputTrackMousePos::m_PanelWatchBlockVisible = true;
#endif

ComponentInputTrackMousePos::ComponentInputTrackMousePos()
: ComponentInputHandler()
{
    m_BaseType = BaseComponentType_InputHandler;

    m_pComponentTransform = 0;
}

ComponentInputTrackMousePos::~ComponentInputTrackMousePos()
{
}

#if MYFW_USING_WX
void ComponentInputTrackMousePos::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentInputTrackMousePos::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "InputTrackMousePos" );
}

void ComponentInputTrackMousePos::OnLeftClick(bool clear)
{
    ComponentBase::OnLeftClick( clear );
}

void ComponentInputTrackMousePos::FillPropertiesWindow(bool clear)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Input Track Mouse Pos", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentBase::FillPropertiesWindow( clear );
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentInputTrackMousePos::ExportAsJSONObject()
{
    cJSON* component = ComponentInputHandler::ExportAsJSONObject();

    return component;
}

void ComponentInputTrackMousePos::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentInputHandler::ImportFromJSONObject( jsonobj, sceneid );
}

void ComponentInputTrackMousePos::Reset()
{
    ComponentInputHandler::Reset();

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

ComponentInputTrackMousePos& ComponentInputTrackMousePos::operator=(const ComponentInputTrackMousePos& other)
{
    assert( &other != this );

    ComponentInputHandler::operator=( other );

    return *this;
}

bool ComponentInputTrackMousePos::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    // snap the object to the mouse pos.
    m_pComponentTransform->SetPosition( Vector3( x, y, 0 ) );

    //return true;
    return false; // allow other input handlers to use touch messages.
}

bool ComponentInputTrackMousePos::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    return false;
}
