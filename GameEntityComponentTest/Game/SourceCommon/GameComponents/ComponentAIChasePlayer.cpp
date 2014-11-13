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

ComponentAIChasePlayer::ComponentAIChasePlayer()
: ComponentUpdateable()
{
    m_BaseType = BaseComponentType_Updateable;

    m_pComponentTransform = 0;
    m_pPlayerComponentTransform = 0;
}

ComponentAIChasePlayer::ComponentAIChasePlayer(GameObject* owner)
: ComponentUpdateable( owner )
{
    m_BaseType = BaseComponentType_InputHandler;

    m_pComponentTransform = 0;
    m_pPlayerComponentTransform = 0;
}

ComponentAIChasePlayer::~ComponentAIChasePlayer()
{
}

#if MYFW_USING_WX
void ComponentAIChasePlayer::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentAIChasePlayer::StaticFillPropertiesWindow, ComponentBase::StaticOnRightClick, gameobjectid, "AIChasePlayer" );
}

void ComponentAIChasePlayer::FillPropertiesWindow()
{
    g_pPanelWatch->ClearAllVariables();
}
#endif //MYFW_USING_WX

void ComponentAIChasePlayer::Reset()
{
    ComponentUpdateable::Reset();

    m_pComponentTransform = m_pGameObject->m_pComponentTransform;    
}

void ComponentAIChasePlayer::Tick(double TimePassed)
{
    if( m_pComponentTransform == 0 || m_pPlayerComponentTransform == 0 )
    {
        LOGInfo( LOGTag, "ComponentAIChasePlayer - transform or player's transform not set." );
        return;
    }

    Vector3 posdiff = m_pPlayerComponentTransform->m_Position - m_pComponentTransform->m_Position;
    m_pComponentTransform->SetPosition( m_pComponentTransform->m_Position + posdiff * TimePassed );
}
