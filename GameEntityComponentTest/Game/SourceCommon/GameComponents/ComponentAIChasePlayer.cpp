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

void ComponentAIChasePlayer::FillPropertiesWindow(bool clear)
{
    if( clear )
        g_pPanelWatch->ClearAllVariables();

    const char* desc = "not following";
    if( m_pPlayerComponentTransform )
        desc = m_pPlayerComponentTransform->m_pGameObject->m_Name;
    g_pPanelWatch->AddPointerWithDescription( "Object following", m_pPlayerComponentTransform, desc, this, ComponentAIChasePlayer::StaticOnNewParentTransformDrop );
}

void ComponentAIChasePlayer::OnNewParentTransformDrop()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_ComponentPointer )
    {
        ComponentTransform* pComponent = (ComponentTransform*)g_DragAndDropStruct.m_Value;
        assert( pComponent );

        if( pComponent->m_BaseType == BaseComponentType_Transform )
        {
            this->m_pPlayerComponentTransform = pComponent;
        }
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentAIChasePlayer::ExportAsJSONObject()
{
    cJSON* component = ComponentUpdateable::ExportAsJSONObject();

    if( m_pPlayerComponentTransform )
        cJSON_AddNumberToObject( component, "ChasingGOID", m_pPlayerComponentTransform->m_pGameObject->m_ID );

    return component;
}

void ComponentAIChasePlayer::ImportFromJSONObject(cJSON* jsonobj)
{
    ComponentUpdateable::ImportFromJSONObject( jsonobj );

    unsigned int chasingid = 0;
    cJSONExt_GetUnsignedInt( jsonobj, "ChasingGOID", &chasingid );
    if( chasingid != 0 )
    {
        GameObject* pGameObject = g_pComponentSystemManager->FindGameObjectByID( chasingid );
        assert( pGameObject );

        if( pGameObject )
            m_pPlayerComponentTransform = pGameObject->m_pComponentTransform;
    }
}

void ComponentAIChasePlayer::Reset()
{
    ComponentUpdateable::Reset();

    m_pComponentTransform = m_pGameObject->m_pComponentTransform;
}

ComponentAIChasePlayer& ComponentAIChasePlayer::operator=(const ComponentAIChasePlayer& other)
{
    assert( &other != this );

    ComponentUpdateable::operator=( other );

    this->m_pPlayerComponentTransform = other.m_pPlayerComponentTransform;

    return *this;
}

void ComponentAIChasePlayer::Tick(double TimePassed)
{
    if( m_pComponentTransform == 0 || m_pPlayerComponentTransform == 0 )
    {
        LOGInfo( LOGTag, "ComponentAIChasePlayer - transform or player's transform not set." );
        return;
    }

    Vector3 posdiff = m_pPlayerComponentTransform->GetPosition() - m_pComponentTransform->GetPosition();
    m_pComponentTransform->SetPosition( m_pComponentTransform->GetPosition() + posdiff * (float)TimePassed );
}
