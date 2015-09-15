//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentTransform::m_PanelWatchBlockVisible = true;
#endif

ComponentTransform::ComponentTransform()
: ComponentBase()
{
    ClassnameSanityCheck();
    RegisterVariables( this );

    m_BaseType = BaseComponentType_Data;

    m_pParentGameObject = 0;
    m_pParentTransform = 0;

    m_pPositionChangedCallbackList.AllocateObjects( MAX_REGISTERED_CALLBACKS );
}

ComponentTransform::~ComponentTransform()
{
    m_pPositionChangedCallbackList.FreeAllInList();

    // if we had an parent transform, stop it's gameobject from reporting it's deletion.
    if( m_pParentTransform != 0 )
    {
        MyAssert( m_pParentGameObject == m_pParentTransform->m_pGameObject );
        m_pParentTransform->m_pGameObject->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
    }
}

void ComponentTransform::Reset()
{
    // if we had an parent transform, stop it's gameobject from reporting it's deletion.
    if( m_pParentTransform != 0 )
    {
        MyAssert( m_pParentGameObject == m_pParentTransform->m_pGameObject );
        m_pParentTransform->m_pGameObject->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
    }

    ComponentBase::Reset();

    m_Transform.SetIdentity();
    m_pParentGameObject = 0;
    m_pParentTransform = 0;

    m_LocalTransform.SetIdentity();
    m_Position.Set( 0,0,0 );
    m_Scale.Set( 1,1,1 );
    m_Rotation.Set( 0,0,0 );

#if MYFW_USING_WX
    m_ControlID_ParentTransform = -1;
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

// a simple runtime version of offsetof, should be safe with non-POD types, I think.
int MyOffsetOf(void* pObject, void* pMember)
{
    return (char*)pMember - (char*)pObject;
}

void ComponentTransform::RegisterVariables(ComponentTransform* pThis)
{
    if( m_ComponentVariableList.GetHead() != 0 )
        return;

    // just want to make sure these are the same on all compilers.  They should be since this is a simple class.
    MyAssert( offsetof( ComponentTransform, m_pParentGameObject ) == MyOffsetOf( pThis, &pThis->m_pParentGameObject ) );
    MyAssert( offsetof( ComponentTransform, m_pParentTransform )  == MyOffsetOf( pThis, &pThis->m_pParentTransform )  );
    MyAssert( offsetof( ComponentTransform, m_Position )          == MyOffsetOf( pThis, &pThis->m_Position )          );
    MyAssert( offsetof( ComponentTransform, m_Scale )             == MyOffsetOf( pThis, &pThis->m_Scale )             );
    MyAssert( offsetof( ComponentTransform, m_Rotation )          == MyOffsetOf( pThis, &pThis->m_Rotation )          );

    //AddVariable( "ParentGOID",  ComponentVariableType_GameObjectPtr,    offsetof( ComponentTransform, m_pParentGameObject ) );
    //AddVariable( "Pos",         ComponentVariableType_Vector3,          offsetof( ComponentTransform, m_Position )          );
    //AddVariable( "Scale",       ComponentVariableType_Vector3,          offsetof( ComponentTransform, m_Scale )             );
    //AddVariable( "Rot",         ComponentVariableType_Vector3,          offsetof( ComponentTransform, m_Rotation )          );

    AddVariable( "ParentGOID",      ComponentVariableType_GameObjectPtr,    MyOffsetOf( pThis, &pThis->m_pParentGameObject ), true,  false, 0,                  ComponentTransform::StaticOnValueChanged );
    AddVariable( "ParentTransform", ComponentVariableType_ComponentPtr,     MyOffsetOf( pThis, &pThis->m_pParentTransform ),  false,  true, "Parent Transform", ComponentTransform::StaticOnValueChanged );
    AddVariable( "Pos",             ComponentVariableType_Vector3,          MyOffsetOf( pThis, &pThis->m_Position ),          true,   true, 0,                  ComponentTransform::StaticOnValueChanged );
    AddVariable( "Scale",           ComponentVariableType_Vector3,          MyOffsetOf( pThis, &pThis->m_Scale ),             true,   true, 0,                  ComponentTransform::StaticOnValueChanged );
    AddVariable( "Rot",             ComponentVariableType_Vector3,          MyOffsetOf( pThis, &pThis->m_Rotation ),          true,   true, 0,                  ComponentTransform::StaticOnValueChanged );
}

void ComponentTransform::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentTransform>( "ComponentTransform" )
            .addData( "localmatrix", &ComponentTransform::m_LocalTransform )
            .addFunction( "SetPosition", &ComponentTransform::SetPosition )
            .addFunction( "GetPosition", &ComponentTransform::GetPosition )
            .addFunction( "SetRotation", &ComponentTransform::SetRotation )
            .addFunction( "GetLocalRotation", &ComponentTransform::GetLocalRotation )
        .endClass();
}

#if MYFW_USING_WX
bool ComponentTransform::IsAnyParentInList(std::vector<GameObject*>& gameobjects)
{
    ComponentTransform* pTransform = m_pParentTransform;

    while( pTransform )
    {
        for( unsigned int i=0; i<gameobjects.size(); i++ )
        {
            GameObject* pGameObject = (GameObject*)gameobjects[i];
            MyAssert( pGameObject->IsA( "GameObject" ) );

            if( pGameObject == pTransform->m_pGameObject )
                return true;
        }

        pTransform = pTransform->m_pParentTransform;
    }

    return false;
}

void ComponentTransform::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentTransform::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Transform" );
    g_pPanelObjectList->SetDragAndDropFunctions( id, ComponentBase::StaticOnDrag, ComponentBase::StaticOnDrop );
}

void ComponentTransform::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentTransform::FillPropertiesWindow(bool clear)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Transform", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentBase::FillPropertiesWindow( clear );

        //const char* desc = "none";
        //if( m_pParentTransform )
        //{
        //    MyAssert( m_pParentGameObject == m_pParentTransform->m_pGameObject );
        //    desc = m_pParentTransform->m_pGameObject->GetName();
        //}
        //m_ControlID_ParentTransform = g_pPanelWatch->AddPointerWithDescription( "Parent transform", m_pParentTransform, desc, this, ComponentTransform::StaticOnDropTransform, ComponentTransform::StaticOnValueChanged );

        //g_pPanelWatch->AddVector3( "Pos", &m_Position, 0.0f, 0.0f, this, ComponentTransform::StaticOnValueChanged );
        //g_pPanelWatch->AddVector3( "Scale", &m_Scale, 0.0f, 0.0f, this, ComponentTransform::StaticOnValueChanged );
        //g_pPanelWatch->AddVector3( "Rot", &m_Rotation, 0, 0, this, ComponentTransform::StaticOnValueChanged );

        for( CPPListNode* pNode = m_ComponentVariableList.GetHead(); pNode; pNode = pNode->GetNext() )
        {
            ComponentVariable* pVar = (ComponentVariable*)pNode;
            MyAssert( pVar );

            if( pVar->m_DisplayInWatch == false )
                continue;

            if( pVar->m_Offset != -1 )
            {
                if( pVar->m_Type == ComponentVariableType_Vector3 )
                {
                    //g_pPanelWatch->AddVector3( pVar->m_WatchLabel, (Vector3*)((char*)this + pVar->m_Offset), 0.0f, 0.0f, this, ComponentTransform::StaticOnValueChanged );
                    pVar->m_ControlID = g_pPanelWatch->AddVector3( pVar->m_WatchLabel, (Vector3*)((char*)this + pVar->m_Offset), 0.0f, 0.0f, this, ComponentBase::StaticOnValueChanged );
                }

                if( pVar->m_Type == ComponentVariableType_GameObjectPtr )
                {
                }

                if( pVar->m_Type == ComponentVariableType_ComponentPtr )
                {
                    ComponentTransform* pTransformComponent = *(ComponentTransform**)((char*)this + pVar->m_Offset);

                    const char* desc = "none";
                    if( pTransformComponent )
                    {
                        desc = pTransformComponent->m_pGameObject->GetName();
                    }

                    m_ControlID_ParentTransform = g_pPanelWatch->AddPointerWithDescription( pVar->m_WatchLabel, pTransformComponent, desc, this, ComponentTransform::StaticOnDropTransform, ComponentTransform::StaticOnValueChanged );
                }
            }
        }
    }
}

void ComponentTransform::OnDropTransform(int controlid, wxCoord x, wxCoord y)
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

        if( pComponent->IsA( "TransformComponent" ) )
        {
            this->SetParent( pComponent );
        }

        // update the panel so new OBJ name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pParentTransform->m_pGameObject->GetName();
    }
}

void ComponentTransform::OnValueChanged(int controlid, bool finishedchanging)
{
    if( controlid != -1 && controlid == m_ControlID_ParentTransform )
    {
        wxString text = g_pPanelWatch->m_pVariables[m_ControlID_ParentTransform].m_Handle_TextCtrl->GetValue();
        if( text == "" )
        {
            g_pPanelWatch->ChangeDescriptionForPointerWithDescription( m_ControlID_ParentTransform, "none" );
            this->SetParent( 0 );
        }
    }
    else
    {
        m_LocalTransform.CreateSRT( m_Scale, m_Rotation, m_Position );
        UpdateMatrix();

        for( unsigned int i=0; i<m_pPositionChangedCallbackList.Count(); i++ )
            m_pPositionChangedCallbackList[i].pFunc( m_pPositionChangedCallbackList[i].pObj, m_Position, true );
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentTransform::ExportAsJSONObject(bool savesceneid)
{
    cJSON* component = ComponentBase::ExportAsJSONObject( savesceneid );

    //if( m_pParentTransform )
    //{
    //    MyAssert( m_pParentGameObject == m_pParentTransform->m_pGameObject );
    //    cJSON_AddNumberToObject( component, "ParentGOID", m_pParentTransform->m_pGameObject->GetID() );
    //}
    //cJSONExt_AddFloatArrayToObject( component, "Pos", &m_Position.x, 3 );
    //cJSONExt_AddFloatArrayToObject( component, "Scale", &m_Scale.x, 3 );
    //cJSONExt_AddFloatArrayToObject( component, "Rot", &m_Rotation.x, 3 );

    for( CPPListNode* pNode = m_ComponentVariableList.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        ComponentVariable* pVar = (ComponentVariable*)pNode;
        MyAssert( pVar );

        if( pVar->m_SaveLoad == false )
            continue;

        if( pVar->m_Offset != -1 )
        {
            if( pVar->m_Type == ComponentVariableType_Vector3 )
            {
                cJSONExt_AddFloatArrayToObject( component, pVar->m_Label, (float*)((char*)this + pVar->m_Offset), 3 );
            }

            if( pVar->m_Type == ComponentVariableType_GameObjectPtr )
            {
                GameObject* pParentGameObject = *(GameObject**)((char*)this + pVar->m_Offset);
                if( pParentGameObject )
                {
                    cJSON_AddNumberToObject( component, pVar->m_Label, pParentGameObject->GetID() );
                }
            }
        }
    }

    return component;
}

void ComponentTransform::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentBase::ImportFromJSONObject( jsonobj, sceneid );

    //unsigned int parentid = 0;
    //cJSONExt_GetUnsignedInt( jsonobj, "ParentGOID", &parentid );
    //if( parentid != 0 )
    //{
    //    GameObject* pParentGameObject = g_pComponentSystemManager->FindGameObjectByID( sceneid, parentid );
    //    MyAssert( pParentGameObject );
    //    if( pParentGameObject )
    //        SetParent( pParentGameObject->m_pComponentTransform );
    //}

    //cJSONExt_GetFloatArray( jsonobj, "Pos", &m_Position.x, 3 );
    //cJSONExt_GetFloatArray( jsonobj, "Scale", &m_Scale.x, 3 );
    //cJSONExt_GetFloatArray( jsonobj, "Rot", &m_Rotation.x, 3 );

    // import the parent goid, set the parent, then import the rest.
    ImportVariablesFromJSON( jsonobj, "ParentGOID" );
    if( m_pParentGameObject )
        SetParent( m_pParentGameObject->m_pComponentTransform );

    ImportVariablesFromJSON( jsonobj );

    m_LocalTransform.CreateSRT( m_Scale, m_Rotation, m_Position );
    UpdateMatrix();
}

ComponentTransform& ComponentTransform::operator=(const ComponentTransform& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    this->m_Transform = other.m_Transform;
    this->m_pParentGameObject = other.m_pParentGameObject;
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

    for( unsigned int i=0; i<m_pPositionChangedCallbackList.Count(); i++ )
        m_pPositionChangedCallbackList[i].pFunc( m_pPositionChangedCallbackList[i].pObj, m_Position, false );
}

#if MYFW_USING_WX
void ComponentTransform::SetPositionByEditor(Vector3 pos)
{
    m_Position = pos;
    m_LocalTransform.CreateSRT( m_Scale, m_Rotation, m_Position );

    for( unsigned int i=0; i<m_pPositionChangedCallbackList.Count(); i++ )
        m_pPositionChangedCallbackList[i].pFunc( m_pPositionChangedCallbackList[i].pObj, m_Position, true );
}
#endif

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

void ComponentTransform::SetParent(ComponentTransform* pNewParent, bool unregisterondeletecallback)
{
    // if we had an old parent, stop it's gameobject from reporting it's deletion.
    if( m_pParentTransform != 0 && unregisterondeletecallback )
    {
        m_pParentTransform->m_pGameObject->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
    }

    if( pNewParent == 0 || pNewParent == this )
    {
        m_LocalTransform = m_Transform;
        m_pParentGameObject = 0;
        m_pParentTransform = 0;
    }
    else
    {
        MyMatrix matparent = pNewParent->m_Transform;
        matparent.Inverse();
        m_LocalTransform = matparent * m_Transform;

        m_pParentGameObject = pNewParent->m_pGameObject;
        m_pParentTransform = pNewParent;

        // register this component with the gameobject of the parent to notify us of it's deletion.
        m_pParentTransform->m_pGameObject->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
    }

    UpdateMatrix();

    UpdatePosAndRotFromLocalMatrix();
}

void ComponentTransform::UpdatePosAndRotFromLocalMatrix()
{
    m_Position = m_LocalTransform.GetTranslation();
    m_Rotation = m_LocalTransform.GetEulerAngles() * 180.0f/PI;
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

void ComponentTransform::RegisterPositionChangedCallback(void* pObj, TransformPositionChangedCallbackFunc pCallback)
{
    MyAssert( pCallback != 0 );
    MyAssert( m_pPositionChangedCallbackList.Count() < MAX_REGISTERED_CALLBACKS );

    TransformPositionChangedCallbackStruct callbackstruct;
    callbackstruct.pObj = pObj;
    callbackstruct.pFunc = pCallback;

    m_pPositionChangedCallbackList.Add( callbackstruct );
}

void ComponentTransform::OnGameObjectDeleted(GameObject* pGameObject)
{
    // if our parent was deleted, clear the pointer.
    MyAssert( m_pParentTransform == pGameObject->m_pComponentTransform ); // the callback should have only been registered if needed.
    if( m_pParentTransform == pGameObject->m_pComponentTransform )
    {
        // we're in the callback, so don't unregister the callback.
        SetParent( 0, false );
    }
}
