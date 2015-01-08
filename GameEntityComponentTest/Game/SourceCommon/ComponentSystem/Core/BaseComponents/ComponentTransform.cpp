//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

ComponentTransform::ComponentTransform()
: ComponentBase()
{
    m_BaseType = BaseComponentType_Transform;
}

ComponentTransform::~ComponentTransform()
{
}

void ComponentTransform::Reset()
{
    ComponentBase::Reset();

    m_Transform.SetIdentity();
    m_pParentTransform = 0;

    m_LocalTransform.SetIdentity();
    m_Position.Set( 0,0,0 );
    m_Scale.Set( 1,1,1 );
    m_Rotation.Set( 0,0,0 );
}

void ComponentTransform::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentTransform>( "Transform" )
            .addData( "localmatrix", &ComponentTransform::m_LocalTransform )
            .addFunction( "SetPosition", &ComponentTransform::SetPosition )
            .addFunction( "GetPosition", &ComponentTransform::GetPosition )
        .endClass();
}

#if MYFW_USING_WX
void ComponentTransform::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentTransform::StaticFillPropertiesWindow, ComponentBase::StaticOnRightClick, gameobjectid, "Transform" );
    g_pPanelObjectList->SetDragAndDropFunctions( this, ComponentBase::StaticOnDrag, ComponentBase::StaticOnDrop );
}

void ComponentTransform::FillPropertiesWindow(bool clear)
{
    if( clear )
        g_pPanelWatch->ClearAllVariables();

    g_pPanelWatch->AddFloat( "x", &m_Position.x, -1.0f, 1.0f, this, ComponentTransform::StaticOnValueChanged );
    g_pPanelWatch->AddFloat( "y", &m_Position.y, -1.0f, 1.0f, this, ComponentTransform::StaticOnValueChanged );
    g_pPanelWatch->AddFloat( "z", &m_Position.z, -1.0f, 1.0f, this, ComponentTransform::StaticOnValueChanged );

    g_pPanelWatch->AddFloat( "scale x", &m_Scale.x, 0.0f, 10.0f, this, ComponentTransform::StaticOnValueChanged );
    g_pPanelWatch->AddFloat( "scale y", &m_Scale.y, 0.0f, 10.0f, this, ComponentTransform::StaticOnValueChanged );
    g_pPanelWatch->AddFloat( "scale z", &m_Scale.z, 0.0f, 10.0f, this, ComponentTransform::StaticOnValueChanged );

    g_pPanelWatch->AddFloat( "rot x", &m_Rotation.x, 0, 360, this, ComponentTransform::StaticOnValueChanged );
    g_pPanelWatch->AddFloat( "rot y", &m_Rotation.y, 0, 360, this, ComponentTransform::StaticOnValueChanged );
    g_pPanelWatch->AddFloat( "rot z", &m_Rotation.z, 0, 360, this, ComponentTransform::StaticOnValueChanged );

    const char* desc = "no parent";
    if( m_pParentTransform )
        desc = m_pParentTransform->m_pGameObject->GetName();
    g_pPanelWatch->AddPointerWithDescription( "Parent transform", m_pParentTransform, desc, this, ComponentTransform::StaticOnDropTransform );
}

void ComponentTransform::OnDropTransform()
{
    ComponentTransform* pComponent = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_ComponentPointer )
    {
        pComponent = (ComponentTransform*)g_DragAndDropStruct.m_Value;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        pComponent = ((GameObject*)g_DragAndDropStruct.m_Value)->m_pComponentTransform;
    }

    if( pComponent )
    {
        if( pComponent == this )
            return;

        if( pComponent->m_BaseType == BaseComponentType_Transform )
        {
            this->SetParent( pComponent );
        }

        // update the panel so new OBJ name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pParentTransform->m_pGameObject->GetName();
    }
}

void ComponentTransform::OnValueChanged()
{
    m_LocalTransform.CreateSRT( m_Scale, m_Rotation, m_Position );
}
#endif //MYFW_USING_WX

cJSON* ComponentTransform::ExportAsJSONObject()
{
    cJSON* component = ComponentBase::ExportAsJSONObject();

    if( m_pParentTransform )
        cJSON_AddNumberToObject( component, "ParentGOID", m_pParentTransform->m_ID );
    cJSONExt_AddFloatArrayToObject( component, "Pos", &m_Position.x, 3 );
    cJSONExt_AddFloatArrayToObject( component, "Scale", &m_Scale.x, 3 );
    cJSONExt_AddFloatArrayToObject( component, "Rot", &m_Rotation.x, 3 );

    return component;
}

void ComponentTransform::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentBase::ImportFromJSONObject( jsonobj, sceneid );

    unsigned int parentid = 0;
    cJSONExt_GetUnsignedInt( jsonobj, "ParentGOID", &parentid );
    if( parentid != 0 )
    {
        GameObject* pParentGameObject = g_pComponentSystemManager->FindGameObjectByID( parentid );
        m_pParentTransform = pParentGameObject->m_pComponentTransform;
    }

    cJSONExt_GetFloatArray( jsonobj, "Pos", &m_Position.x, 3 );
    cJSONExt_GetFloatArray( jsonobj, "Scale", &m_Scale.x, 3 );
    cJSONExt_GetFloatArray( jsonobj, "Rot", &m_Rotation.x, 3 );

    m_LocalTransform.CreateSRT( m_Scale, m_Rotation, m_Position );
}

ComponentTransform& ComponentTransform::operator=(const ComponentTransform& other)
{
    assert( &other != this );

    ComponentBase::operator=( other );

    this->m_Transform = other.m_Transform;
    this->m_pParentTransform = other.m_pParentTransform;

    this->m_LocalTransform = other.m_LocalTransform;
    this->m_Position = other.m_Position;
    this->m_Scale = other.m_Scale;
    this->m_Rotation = other.m_Rotation;

    return *this;
}

void ComponentTransform::SetPosition(Vector3 pos)
{
    m_Position = pos;
    m_LocalTransform.CreateSRT( m_Scale, m_Rotation, m_Position );
}

void ComponentTransform::SetScale(Vector3 scale)
{
    m_Scale = scale;
    m_LocalTransform.CreateSRT( m_Scale, m_Rotation, m_Position );
}

void ComponentTransform::SetRotation(Vector3 rot)
{
    m_Rotation = rot;
    m_LocalTransform.CreateSRT( m_Scale, m_Rotation, m_Position );
}

MyMatrix ComponentTransform::GetLocalRotPosMatrix()
{
    MyMatrix local;
    local.CreateSRT( Vector3(1,1,1), m_Rotation, m_Position );

    return local;
}

void ComponentTransform::SetParent(ComponentTransform* pNewParent)
{
    m_Position = m_Position - pNewParent->m_Position;
    m_pParentTransform = pNewParent;
}

void ComponentTransform::UpdateMatrix()
{
    //m_Transform.CreateSRT( m_Scale, m_Rotation, m_Position );
    m_Transform = m_LocalTransform;

    if( m_pParentTransform )
    {
        m_pParentTransform->UpdateMatrix();
        m_Transform = m_pParentTransform->m_Transform * m_Transform;
    }
}

//MyMatrix* ComponentTransform::GetMatrix()
//{
//    return &m_Transform;
//}
