//
// Copyright (c) 2014-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentTransform.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/Core/GameObject.h"

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentTransform );

ComponentTransform::ComponentTransform(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager)
: ComponentBase( pEngineCore, pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR();

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pParentTransform = nullptr;
}

ComponentTransform::~ComponentTransform()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR();
}

void ComponentTransform::SystemStartup()
{
}

void ComponentTransform::SystemShutdown()
{
}

void ComponentTransform::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentTransform* pThis) //_VARIABLE_LIST
{
    // just want to make sure these are the same on all compilers.  They should be since this is a simple class.
#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
    //MyAssert( offsetof( ComponentTransform, m_pParentGameObject ) == MyOffsetOf( pThis, &pThis->m_pParentGameObject ) );
    MyAssert( offsetof( ComponentTransform, m_pParentTransform )  == MyOffsetOf( pThis, &pThis->m_pParentTransform )  );
    MyAssert( offsetof( ComponentTransform, m_LocalPosition )     == MyOffsetOf( pThis, &pThis->m_LocalPosition )     );
    MyAssert( offsetof( ComponentTransform, m_LocalRotation )     == MyOffsetOf( pThis, &pThis->m_LocalRotation )     );
    MyAssert( offsetof( ComponentTransform, m_LocalScale )        == MyOffsetOf( pThis, &pThis->m_LocalScale )        );
#if __GNUC__
#pragma GCC diagnostic pop
#endif

    //AddVar( pList, "ParentGOID",      ComponentVariableType::GameObjectPtr,    MyOffsetOf( pThis, &pThis->m_pParentGameObject ),  true, false, 0,                  (CVarFunc_ValueChanged)&ComponentTransform::OnValueChanged,                                                         nullptr, nullptr );
#if MYFW_USING_WX
    AddVar( pList, "ParentTransform", ComponentVariableType::ComponentPtr,     MyOffsetOf( pThis, &pThis->m_pParentTransform ),  false, false, "Parent Transform", (CVarFunc_ValueChanged)&ComponentTransform::OnValueChanged, (CVarFunc_DropTarget)&ComponentTransform::OnDropTransform, nullptr );
#else
    AddVar( pList, "ParentTransform", ComponentVariableType::ComponentPtr,     MyOffsetOf( pThis, &pThis->m_pParentTransform ),  false, false, "Parent Transform", (CVarFunc_ValueChanged)&ComponentTransform::OnValueChanged, nullptr, nullptr );
#endif
    AddVar( pList, "WorldPos",        ComponentVariableType::Vector3,          MyOffsetOf( pThis, &pThis->m_WorldPosition ),     false,  true, "World Pos",        (CVarFunc_ValueChanged)&ComponentTransform::OnValueChanged,                                                         nullptr, nullptr );
    AddVar( pList, "WorldRot",        ComponentVariableType::Vector3,          MyOffsetOf( pThis, &pThis->m_WorldRotation ),     false,  true, "World Rot",        (CVarFunc_ValueChanged)&ComponentTransform::OnValueChanged,                                                         nullptr, nullptr );
    AddVar( pList, "WorldScale",      ComponentVariableType::Vector3,          MyOffsetOf( pThis, &pThis->m_WorldScale ),        false,  true, "World Scale",      (CVarFunc_ValueChanged)&ComponentTransform::OnValueChanged,                                                         nullptr, nullptr );

    // FillPropertiesWindow() changes m_DisplayInWatch for these 3, will show if parented, won't otherwise.
    AddVar( pList, "Pos",             ComponentVariableType::Vector3,          MyOffsetOf( pThis, &pThis->m_LocalPosition ),      true, false, "Local Pos",        (CVarFunc_ValueChanged)&ComponentTransform::OnValueChanged,                                                         nullptr, nullptr );
    AddVar( pList, "Rot",             ComponentVariableType::Vector3,          MyOffsetOf( pThis, &pThis->m_LocalRotation ),      true, false, "Local Rot",        (CVarFunc_ValueChanged)&ComponentTransform::OnValueChanged,                                                         nullptr, nullptr );
    AddVar( pList, "Scale",           ComponentVariableType::Vector3,          MyOffsetOf( pThis, &pThis->m_LocalScale ),         true, false, "Local Scale",      (CVarFunc_ValueChanged)&ComponentTransform::OnValueChanged,                                                         nullptr, nullptr );
}

void ComponentTransform::Reset()
{
    ComponentBase::Reset();

    m_pParentTransform = nullptr;

    m_WorldTransform.SetIdentity();
    m_WorldTransformIsDirty = true;
    m_WorldPosition.Set( 0,0,0 );
    m_WorldRotation.Set( 0,0,0 );
    m_WorldScale.Set( 1,1,1 );

    m_LocalTransform.SetIdentity();
    m_LocalTransformIsDirty = true;
    m_LocalPosition.Set( 0,0,0 );
    m_LocalRotation.Set( 0,0,0 );
    m_LocalScale.Set( 1,1,1 );
}

#if MYFW_USING_LUA
void ComponentTransform::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentTransform>( "ComponentTransform" )
            .addData( "localtransform", &ComponentTransform::m_LocalTransform ) // MyMatrix

            .addFunction( "GetWorldTransform", &ComponentTransform::GetWorldTransform ) // MyMatrix* ComponentTransform::GetWorldTransform(bool markDirty)
            .addFunction( "SetWorldTransform", &ComponentTransform::SetWorldTransform ) // void ComponentTransform::SetWorldTransform(MyMatrix* mat)
            .addFunction( "UpdateWorldSRT",    &ComponentTransform::UpdateWorldSRT )    // void ComponentTransform::UpdateWorldSRT()

            .addFunction( "GetLocalTransform", &ComponentTransform::GetLocalTransform ) // MyMatrix* ComponentTransform::GetLocalTransform(bool markDirty)
            .addFunction( "SetLocalTransform", &ComponentTransform::SetLocalTransform ) // void ComponentTransform::SetLocalTransform(MyMatrix* mat)
            .addFunction( "UpdateLocalSRT",    &ComponentTransform::UpdateLocalSRT )    // void ComponentTransform::UpdateLocalSRT()

            .addFunction( "SetLocalPosition", &ComponentTransform::SetLocalPosition ) // void ComponentTransform::SetLocalPosition(Vector3 pos)
            .addFunction( "SetLocalRotation", &ComponentTransform::SetLocalRotation ) // void ComponentTransform::SetLocalRotation(Vector3 rot)
            .addFunction( "SetLocalScale",    &ComponentTransform::SetLocalScale )    // void ComponentTransform::SetLocalScale(Vector3 scale)
            .addFunction( "GetLocalPosition", &ComponentTransform::GetLocalPosition ) // Vector3 ComponentTransform::GetLocalPosition()
            .addFunction( "GetLocalRotation", &ComponentTransform::GetLocalRotation ) // Vector3 ComponentTransform::GetLocalRotation()
            .addFunction( "GetLocalScale",    &ComponentTransform::GetLocalScale )    // Vector3 ComponentTransform::GetLocalScale()

            .addFunction( "SetWorldPosition", &ComponentTransform::SetWorldPosition ) // void ComponentTransform::SetWorldPosition(Vector3 pos)
            .addFunction( "SetWorldRotation", &ComponentTransform::SetWorldRotation ) // void ComponentTransform::SetWorldRotation(Vector3 rot)
            .addFunction( "SetWorldScale",    &ComponentTransform::SetWorldScale )    // void ComponentTransform::SetWorldScale(Vector3 scale)
            .addFunction( "GetWorldPosition", &ComponentTransform::GetWorldPosition ) // Vector3 ComponentTransform::GetWorldPosition()
            .addFunction( "GetWorldRotation", &ComponentTransform::GetWorldRotation ) // Vector3 ComponentTransform::GetWorldRotation()
            .addFunction( "GetWorldScale",    &ComponentTransform::GetWorldScale )    // Vector3 ComponentTransform::GetWorldScale()

            .addFunction( "LookAt", &ComponentTransform::LookAt ) // void ComponentTransform::LookAt(Vector3 pos)
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
bool ComponentTransform::IsAnyParentInList(std::vector<GameObject*>& gameObjects)
{
    ComponentTransform* pTransform = m_pParentTransform;

    while( pTransform )
    {
        for( unsigned int i=0; i<gameObjects.size(); i++ )
        {
            GameObject* pGameObject = gameObjects[i];
            MyAssert( pGameObject->IsA( "GameObject" ) );

            if( pGameObject == pTransform->m_pGameObject )
                return true;
        }

        pTransform = pTransform->m_pParentTransform;
    }

    return false;
}
#endif //MYFW_EDITOR

#if MYFW_EDITOR
void* ComponentTransform::OnDropTransform(ComponentVariable* pVar, bool changedByInterface, int x, int y)
{
    void* oldPointer = nullptr;

    ComponentTransform* pComponent = nullptr;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
    {
        pComponent = (ComponentTransform*)pDropItem->m_Value;
    }

    if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
    {
        pComponent = ((GameObject*)pDropItem->m_Value)->GetTransform();
    }

    if( pComponent )
    {
        if( pComponent == this )
            return nullptr;

        if( pComponent->IsA( "TransformComponent" ) )
        {
            if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
            {
                oldPointer = this->m_pParentTransform;
            }

            if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
            {
                if( this->m_pParentTransform )
                    oldPointer = this->m_pParentTransform->m_pGameObject;
            }

            this->m_pGameObject->SetParentGameObject( pComponent->m_pGameObject );
        }
    }

    return oldPointer;
}

void* ComponentTransform::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldPointer = nullptr;

    if( pVar->m_Offset == MyOffsetOf( this, &m_pParentTransform ) )
    {
        if( changedByInterface )
        {
        }
        else //if( pNewValue->GetComponentPtr() != nullptr )
        {
            MyAssert( false ); // This block is untested.
            oldPointer = this->GetParentTransform();
            this->SetParentTransform( pNewValue ? (ComponentTransform*)pNewValue->GetComponentPtr() : nullptr );
        }
    }
    else
    {
        if( pVar->m_Offset == MyOffsetOf( this, &m_WorldPosition ) ||
            pVar->m_Offset == MyOffsetOf( this, &m_WorldRotation ) ||
            pVar->m_Offset == MyOffsetOf( this, &m_WorldScale ) )
        {
            if( m_pParentTransform == nullptr )
            {
                m_LocalPosition = m_WorldPosition;
                m_LocalRotation = m_WorldRotation;
                m_LocalScale = m_WorldScale;
                m_LocalTransformIsDirty = true;
            }
            else
            {
                // Calculate new local transform matrix, decompose it into local SRT.
                MyMatrix matworld;
                matworld.CreateSRT( m_WorldScale, m_WorldRotation, m_WorldPosition );

                MyMatrix matparentworld = *m_pParentTransform->GetWorldTransform();
                matparentworld.Inverse();
                MyMatrix matlocal = matparentworld * matworld;
                
                SetLocalTransform( &matlocal );
                UpdateLocalSRT();
            }

            m_WorldTransformIsDirty = true;
            UpdateTransform();

            for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
            {
                TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

                pCallbackStruct->pFunc( pCallbackStruct->pObj, m_WorldPosition, m_WorldRotation, m_WorldScale, true );
            }
        }
        else if( pVar->m_Offset == MyOffsetOf( this, &m_LocalPosition ) ||
                 pVar->m_Offset == MyOffsetOf( this, &m_LocalRotation ) ||
                 pVar->m_Offset == MyOffsetOf( this, &m_LocalScale ) )
        {
            m_LocalTransformIsDirty = true;

            for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
            {
                TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

                pCallbackStruct->pFunc( pCallbackStruct->pObj, m_WorldPosition, m_WorldRotation, m_WorldScale, true );
            }
        }
    }

    return oldPointer;
}
#endif //MYFW_EDITOR

cJSON* ComponentTransform::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid, saveid );

    return jComponent;
}

void ComponentTransform::ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid)
{
    // Moved into GameObject, here for old scene files.
    {
        // Import the parent goid, set the parent, then import the rest.        
        //ImportVariablesFromJSON( jsonobj, "ParentGOID" );
        unsigned int parentGOID = 0;
        cJSONExt_GetUnsignedInt( jsonobj, "ParentGOID", &parentGOID );

        if( parentGOID > 0 )
        {
            GameObject* pParentGameObject = g_pComponentSystemManager->FindGameObjectByID( sceneid, parentGOID );
            MyAssert( pParentGameObject );

            if( pParentGameObject )
            {
                m_pGameObject->SetParentGameObject( pParentGameObject );
                m_WorldTransformIsDirty = true;
            }
            m_LocalTransformIsDirty = true;
        }
    }

    // Load all the registered variables.
    ComponentBase::ImportFromJSONObject( jsonobj, sceneid );

    // Local scale/rotation/position should be loaded, update the transform.
    m_LocalTransformIsDirty = true;
    UpdateTransform();

    // Inform all children/other objects that our transform changed.
    for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

        pCallbackStruct->pFunc( pCallbackStruct->pObj, m_WorldPosition, m_WorldRotation, m_WorldScale, true );
    }
}

cJSON* ComponentTransform::ExportLocalTransformAsJSONObject()
{
    cJSON* jComponent = cJSON_CreateObject();

    //AddVar( pList, "Pos",             ComponentVariableType::Vector3,          MyOffsetOf( pThis, &pThis->m_LocalPosition ),      true, false, "Local Pos",        (CVarFunc_ValueChanged)&ComponentTransform::OnValueChanged,                                                         nullptr, nullptr );
    //AddVar( pList, "Rot",             ComponentVariableType::Vector3,          MyOffsetOf( pThis, &pThis->m_LocalRotation ),      true, false, "Local Rot",        (CVarFunc_ValueChanged)&ComponentTransform::OnValueChanged,                                                         nullptr, nullptr );
    //AddVar( pList, "Scale",           ComponentVariableType::Vector3,          MyOffsetOf( pThis, &pThis->m_LocalScale ),         true, false, "Local Scale",      (CVarFunc_ValueChanged)&ComponentTransform::OnValueChanged,                                                         nullptr, nullptr );
    cJSONExt_AddFloatArrayToObject( jComponent, "Pos",   &this->m_LocalPosition.x, 3 );
    cJSONExt_AddFloatArrayToObject( jComponent, "Rot",   &this->m_LocalRotation.x, 3 );
    cJSONExt_AddFloatArrayToObject( jComponent, "Scale", &this->m_LocalScale.x, 3 );

    return jComponent;
}

void ComponentTransform::ImportLocalTransformFromJSONObject(cJSON* jsonobj)
{
    cJSONExt_GetFloatArray( jsonobj, "Pos", &this->m_LocalPosition.x, 3 );
    cJSONExt_GetFloatArray( jsonobj, "Rot", &this->m_LocalRotation.x, 3 );
    cJSONExt_GetFloatArray( jsonobj, "Scale", &this->m_LocalScale.x, 3 );

    m_LocalTransformIsDirty = true;
    UpdateTransform();

    for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

        pCallbackStruct->pFunc( pCallbackStruct->pObj, m_WorldPosition, m_WorldRotation, m_WorldScale, true );
    }
}

ComponentTransform& ComponentTransform::operator=(const ComponentTransform& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    this->m_WorldTransform = other.m_WorldTransform;
    this->m_WorldTransformIsDirty = other.m_WorldTransformIsDirty;
    this->m_WorldPosition = other.m_WorldPosition;
    this->m_WorldRotation = other.m_WorldRotation;
    this->m_WorldScale = other.m_WorldScale;

    this->SetParentTransform( other.m_pParentTransform );

    this->m_LocalTransform = other.m_LocalTransform;
    this->m_LocalTransformIsDirty = other.m_LocalTransformIsDirty;
    this->m_LocalPosition = other.m_LocalPosition;
    this->m_LocalRotation = other.m_LocalRotation;
    this->m_LocalScale = other.m_LocalScale;

    return *this;
}

#if MYFW_EDITOR
void ComponentTransform::SetPositionByEditor(Vector3 pos)
{
    m_LocalPosition = pos;
    m_LocalTransform.CreateSRT( m_LocalScale, m_LocalRotation, m_LocalPosition );

    if( m_pParentTransform == nullptr )
    {
        m_WorldPosition = pos;
        m_WorldTransform.CreateSRT( m_WorldScale, m_WorldRotation, m_WorldPosition );
    }
    else
    {
        m_LocalTransformIsDirty = true;
    }

    for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

        pCallbackStruct->pFunc( pCallbackStruct->pObj, m_WorldPosition, m_WorldRotation, m_WorldScale, true );
    }
}

void ComponentTransform::SetScaleByEditor(Vector3 scale)
{
    m_LocalScale = scale;
    m_LocalTransform.CreateSRT( m_LocalScale, m_LocalRotation, m_LocalPosition );

    if( m_pParentTransform == nullptr )
    {
        m_WorldScale = scale;
        m_WorldTransform.CreateSRT( m_WorldScale, m_WorldRotation, m_WorldPosition );
    }
    else
    {
        m_LocalTransformIsDirty = true;
    }

    for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

        pCallbackStruct->pFunc( pCallbackStruct->pObj, m_WorldPosition, m_WorldRotation, m_WorldScale, true );
    }
}

void ComponentTransform::SetRotationByEditor(Vector3 eulerAngles)
{
    m_LocalRotation = eulerAngles;
    m_LocalTransform.CreateSRT( m_LocalScale, m_LocalRotation, m_LocalPosition );

    if( m_pParentTransform == nullptr )
    {
        m_WorldRotation = eulerAngles;
        m_WorldTransform.CreateSRT( m_WorldScale, m_WorldRotation, m_WorldPosition );
    }
    else
    {
        m_LocalTransformIsDirty = true;
    }

    for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

        pCallbackStruct->pFunc( pCallbackStruct->pObj, m_WorldPosition, m_WorldRotation, m_WorldScale, true );
    }
}
#endif //MYFW_EDITOR

void ComponentTransform::SetWorldPosition(Vector3 pos)
{
    m_WorldPosition = pos;
    m_WorldTransformIsDirty = true;
    if( m_pParentTransform == nullptr )
    {
        m_LocalPosition = m_WorldPosition;
        m_LocalTransformIsDirty = true;
    }

    for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

        pCallbackStruct->pFunc( pCallbackStruct->pObj, m_WorldPosition, m_WorldRotation, m_WorldScale, true );
    }
}

void ComponentTransform::SetWorldRotation(Vector3 rot)
{
    m_WorldRotation = rot;
    m_WorldTransformIsDirty = true;
    if( m_pParentTransform == nullptr )
    {
        m_LocalRotation = m_WorldRotation;
        m_LocalTransformIsDirty = true;
    }

    for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

        pCallbackStruct->pFunc( pCallbackStruct->pObj, m_WorldPosition, m_WorldRotation, m_WorldScale, true );
    }
}

void ComponentTransform::SetWorldScale(Vector3 scale)
{
    m_WorldScale = scale;
    m_WorldTransformIsDirty = true;
    if( m_pParentTransform == nullptr )
    {
        m_LocalScale = m_WorldScale;
        m_LocalTransformIsDirty = true;
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
void ComponentTransform::SetLocalTransform(MyMatrix* mat)
{
    m_LocalTransform = *mat;
    UpdateLocalSRT();

    if( m_pParentTransform )
    {
        m_WorldTransform = *m_pParentTransform->GetWorldTransform() * m_LocalTransform;
        UpdateWorldSRT();
    }
    else
    {
        m_WorldTransformIsDirty = false;
        m_WorldTransform = m_LocalTransform;
        m_WorldPosition = m_LocalPosition;
        m_WorldRotation = m_LocalRotation;
        m_WorldScale = m_LocalScale;
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
void ComponentTransform::SetLocalPosition(Vector3 pos)
{
    if( m_LocalPosition == pos )
        return;

    m_LocalPosition = pos;
    m_LocalTransformIsDirty = true;
    if( m_pParentTransform == nullptr )
    {
        m_WorldPosition = m_LocalPosition;
        m_WorldTransformIsDirty = true;
    }

    UpdateTransform();

    for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

        pCallbackStruct->pFunc( pCallbackStruct->pObj, m_WorldPosition, m_WorldRotation, m_WorldScale, true );
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
void ComponentTransform::SetLocalRotation(Vector3 rot)
{
    m_LocalRotation = rot;
    m_LocalTransformIsDirty = true;
    if( m_pParentTransform == nullptr )
    {
        m_WorldRotation = m_LocalRotation;
        m_WorldTransformIsDirty = true;
    }

    UpdateTransform();
}

void ComponentTransform::SetLocalScale(Vector3 scale)
{
    m_LocalScale = scale;
    m_LocalTransformIsDirty = true;
    if( m_pParentTransform == nullptr )
    {
        m_WorldScale = m_LocalScale;
        m_WorldTransformIsDirty = true;
    }

    UpdateTransform();
}

// Exposed to Lua, change elsewhere if function signature changes.
void ComponentTransform::SetWorldTransform(const MyMatrix* mat)
{
    //MyAssert( false );
    m_WorldTransform = *mat;
    UpdateWorldSRT();

    if( m_pParentTransform )
    {
        m_pParentTransform->UpdateTransform();
        m_LocalTransform = m_pParentTransform->m_WorldTransform.GetInverse() * m_WorldTransform;
        UpdateLocalSRT();
    }
    else
    {
        m_LocalTransform = m_WorldTransform;
        UpdateLocalSRT();
    }

    for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

        pCallbackStruct->pFunc( pCallbackStruct->pObj, m_WorldPosition, m_WorldRotation, m_WorldScale, false );
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
MyMatrix* ComponentTransform::GetWorldTransform(bool markDirty)
{
    UpdateTransform();

    if( markDirty )
        m_WorldTransformIsDirty = true;

    //if( m_pParentTransform == nullptr )
    //    return &m_LocalTransform;

    return &m_WorldTransform;
}

Vector3 ComponentTransform::GetWorldPosition()
{
    //return m_WorldTransform.GetTranslation();
    return m_WorldPosition;
}
Vector3 ComponentTransform::GetWorldScale()
{
    //return m_WorldTransform.GetScale();
    return m_WorldScale;
}
Vector3 ComponentTransform::GetWorldRotation()
{
    //return m_WorldTransform.GetEulerAngles();
    return m_WorldRotation;
}

MyMatrix ComponentTransform::GetWorldRotPosMatrix()
{
    MyMatrix world;
    world.CreateSRT( Vector3(1,1,1), m_WorldRotation, m_WorldPosition );

    return world;
}

// Exposed to Lua, change elsewhere if function signature changes.
MyMatrix* ComponentTransform::GetLocalTransform(bool markDirty)
{
    UpdateTransform();

    if( markDirty )
        m_LocalTransformIsDirty = true;

    return &m_LocalTransform;
}

// Exposed to Lua, change elsewhere if function signature changes.
Vector3 ComponentTransform::GetLocalPosition()
{
    //return m_LocalTransform.GetTranslation();
    return m_LocalPosition;
}
Vector3 ComponentTransform::GetLocalScale()
{
    //return m_LocalTransform.GetScale();
    return m_LocalScale;
}
// Exposed to Lua, change elsewhere if function signature changes.
Vector3 ComponentTransform::GetLocalRotation()
{
    //return m_LocalTransform.GetEulerAngles();
    return m_LocalRotation;
}

MyMatrix ComponentTransform::GetLocalRotPosMatrix()
{
    MyMatrix local;
    local.CreateSRT( Vector3(1,1,1), m_LocalRotation, m_LocalPosition );

    return local;
}

void ComponentTransform::Scale(MyMatrix* pScaleMatrix, Vector3 pivot)
{
    MyMatrix worldTransform = *GetWorldTransform();
    worldTransform.Translate( -pivot );
    worldTransform = *pScaleMatrix * worldTransform;
    worldTransform.Translate( pivot );

    SetWorldTransform( &worldTransform );
}

void ComponentTransform::Rotate(MyMatrix* pRotMatrix, Vector3 pivot)
{
    MyMatrix worldTransform = *GetWorldTransform();
    worldTransform.Translate( -pivot );
    worldTransform = *pRotMatrix * worldTransform;
    worldTransform.Translate( pivot );

    SetWorldTransform( &worldTransform );
}

// Exposed to Lua, change elsewhere if function signature changes.
void ComponentTransform::LookAt(Vector3 pos)
{
    MyMatrix temp;
    temp.CreateLookAtWorld( m_WorldPosition, Vector3(0,1,0), pos );

    Vector3 rot = temp.GetEulerAngles() * 180.0f/PI;

    //LOGInfo( "LookAt", "(%0.2f, %0.2f, %0.2f)\n", rot.x, rot.y, rot.z );

    SetWorldRotation( rot );
}

void ComponentTransform::SetParentTransform(ComponentTransform* pNewParentTransform)
{
    if( m_pParentTransform == pNewParentTransform )
        return;

    MyMatrix wantedWorldSpaceTransform;

    // If we had an old parent:
    if( m_pParentTransform != nullptr )
    {
        // Stop sending old parent position changed messages.
        m_pParentTransform->m_pGameObject->GetTransform()->UnregisterTransformChangedCallbacks( this );

        // Maintain our world space position by setting local transform to match world.
        wantedWorldSpaceTransform = m_WorldTransform;
        m_LocalPosition = m_WorldPosition;
        m_LocalRotation = m_WorldRotation;
        m_LocalScale = m_WorldScale;

        m_LocalTransformIsDirty = true;
    }
    else
    {
        wantedWorldSpaceTransform = m_WorldTransform;
    }

    if( pNewParentTransform == nullptr )
    {
        // If no new parent, set world transform to match local.
        m_WorldTransform = wantedWorldSpaceTransform;
        m_WorldPosition = m_LocalPosition;
        m_WorldRotation = m_LocalRotation;
        m_WorldScale = m_LocalScale;

        m_pParentTransform = nullptr;
    }
    else
    {
        // If there's a new parent, set it as the parent, then recalculate it's world/local transform.
        m_pParentTransform = pNewParentTransform;
        SetWorldTransform( &wantedWorldSpaceTransform );

        // Register this transform with it's parent to notify us if it changes.
        GameObject* pParentGameObject = m_pGameObject->GetParentGameObject();
        pParentGameObject->GetTransform()->RegisterTransformChangedCallback( this, StaticOnParentTransformChanged );
    }

    UpdateTransform();
    UpdateWorldSRT();
}

// Exposed to Lua, change elsewhere if function signature changes.
void ComponentTransform::UpdateWorldSRT()
{
    m_WorldPosition = m_WorldTransform.GetTranslation();
    m_WorldRotation = m_WorldTransform.GetEulerAngles() * 180.0f/PI;
    m_WorldScale = m_WorldTransform.GetScale();
}

// Exposed to Lua, change elsewhere if function signature changes.
void ComponentTransform::UpdateLocalSRT()
{
    m_LocalPosition = m_LocalTransform.GetTranslation();
    m_LocalRotation = m_LocalTransform.GetEulerAngles() * 180.0f/PI;
    m_LocalScale = m_LocalTransform.GetScale();
}

void ComponentTransform::UpdateTransform()
{
    if( m_pParentTransform )
    {
        if( m_WorldTransformIsDirty )
        {
            //MyAssert( m_LocalTransformIsDirty == false );

            m_WorldTransformIsDirty = false;
            m_WorldTransform.CreateSRT( m_WorldScale, m_WorldRotation, m_WorldPosition );
        }

        if( m_LocalTransformIsDirty )
        {
            //MyAssert( m_WorldTransformIsDirty == false );

            m_LocalTransformIsDirty = false;
            m_LocalTransform.CreateSRT( m_LocalScale, m_LocalRotation, m_LocalPosition );

            m_WorldTransform = *m_pParentTransform->GetWorldTransform() * m_LocalTransform;
            UpdateWorldSRT();
        }
    }
    else
    {
        if( m_LocalTransformIsDirty )
        {
            m_LocalTransformIsDirty = false;
            m_LocalTransform.CreateSRT( m_LocalScale, m_LocalRotation, m_LocalPosition );

            m_WorldTransformIsDirty = false;
            m_WorldTransform = m_LocalTransform;
            m_WorldPosition = m_LocalPosition;
            m_WorldRotation = m_LocalRotation;
            m_WorldScale = m_LocalScale;
        }
    }
}

//MyMatrix* ComponentTransform::GetMatrix()
//{
//    return &m_Transform;
//}

void ComponentTransform::RegisterTransformChangedCallback(void* pObj, TransformChangedCallbackFunc* pCallback)
{
    MyAssert( pCallback != nullptr );

    TransformChangedCallbackStruct* pCallbackStruct = m_pComponentSystemManager->GetTransformChangedCallbackPool()->GetObjectFromPool();

    //LOGInfo( "TransformPool", "Grabbed an object (%d) - %s\n", m_pComponentTransform_TransformChangedCallbackPool.GetNumUsed(), ((ComponentBase*)pObj)->m_pGameObject->GetName() );

    if( pCallbackStruct != nullptr )
    {
        pCallbackStruct->pObj = pObj;
        pCallbackStruct->pFunc = pCallback;

        m_TransformChangedCallbackList.AddTail( pCallbackStruct );
    }
}

void ComponentTransform::UnregisterTransformChangedCallbacks(void* pObj)
{
    for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; )
    {
        CPPListNode* pNextNode = pNode->GetNext();

        TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

        if( pCallbackStruct->pObj == pObj )
        {
            pCallbackStruct->Remove();
            m_pComponentSystemManager->GetTransformChangedCallbackPool()->ReturnObjectToPool( pCallbackStruct );

            //LOGInfo( "TransformPool", "Returned an object (%d) %s\n", m_pComponentTransform_TransformChangedCallbackPool.GetNumUsed(), ((ComponentBase*)pObj)->m_pGameObject->GetName() );
        }

        pNode = pNextNode;
    }
}

void ComponentTransform::OnParentTransformChanged(const Vector3& newPos, const Vector3& newRot, const Vector3& newScale, bool changedByUserInEditor)
{
    m_LocalTransformIsDirty = true;
    UpdateTransform();

    for( CPPListNode* pNode = m_TransformChangedCallbackList.GetHead(); pNode != nullptr; pNode = pNode->GetNext() )
    {
        TransformChangedCallbackStruct* pCallbackStruct = (TransformChangedCallbackStruct*)pNode;

        pCallbackStruct->pFunc( pCallbackStruct->pObj, m_WorldPosition, m_WorldRotation, m_WorldScale, changedByUserInEditor );
    }
}
