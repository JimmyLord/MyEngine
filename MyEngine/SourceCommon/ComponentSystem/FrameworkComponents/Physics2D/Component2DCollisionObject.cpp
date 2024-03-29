//
// Copyright (c) 2015-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "Component2DCollisionObject.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"

#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Renderer_Enums.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Renderer_Base.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"

#if MYFW_EDITOR
#include "../SourceEditor/Interfaces/EditorInterface_2DPointEditor.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"
#endif

const char* Physics2DPrimitiveTypeStrings[Physics2DPrimitive_NumTypes] = //ADDING_NEW_Physics2DPrimitiveType
{
    "Box",
    "Circle",
    "Edge",
    "Chain",
};

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( Component2DCollisionObject ); //_VARIABLE_LIST

Component2DCollisionObject::Component2DCollisionObject(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager)
: ComponentBase( pEngineCore, pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    //m_Type = ComponentType_2DCollisionObject;

    m_pComponentLuaScript = nullptr;

    m_pBox2DWorld = nullptr;
    m_pBody = nullptr;
    m_pFixture = nullptr;

    m_PrimitiveType = Physics2DPrimitiveType_Box;

    m_Offset.Set( 0, 0 );
    m_Scale.Set( 1,1,1 );

    // Body properties.
    m_Static = false;
    m_FixedRotation = false;

    // Fixture properties.
    m_Density = 1.0f;
    m_IsSensor = false;
    m_Friction = 0.2f;
    m_Restitution = 0.0f;
}

Component2DCollisionObject::~Component2DCollisionObject()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    if( m_pBody )
    {
        MyAssert( m_pBox2DWorld );
        m_pBox2DWorld->m_pWorld->DestroyBody( m_pBody );
    }

#if !MYFW_EDITOR
    m_Vertices.FreeAllInList();
#endif
}

void Component2DCollisionObject::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, Component2DCollisionObject* pThis) //_VARIABLE_LIST
{
    ComponentVariable* pVar;

    pVar = AddVar( pList, "Offset", ComponentVariableType::Vector2, MyOffsetOf( pThis, &pThis->m_Offset ), true, true, nullptr, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, nullptr, nullptr );
    //pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&Component2DCollisionObject::ShouldVariableBeAddedToWatchPanel) );

    AddVarEnum( pList, "PrimitiveType", MyOffsetOf( pThis, &pThis->m_PrimitiveType ),   true, true, "Primitive Type", Physics2DPrimitive_NumTypes, Physics2DPrimitiveTypeStrings, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, nullptr, nullptr );

    AddVar( pList, "Static",        ComponentVariableType::Bool,  MyOffsetOf( pThis, &pThis->m_Static ),          true, true, nullptr, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, nullptr, nullptr );
    AddVar( pList, "FixedRotation", ComponentVariableType::Bool,  MyOffsetOf( pThis, &pThis->m_FixedRotation ),   true, true, nullptr, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, nullptr, nullptr );
    AddVar( pList, "Density",       ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_Density ),         true, true, nullptr, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, nullptr, nullptr );
    AddVar( pList, "IsSensor",      ComponentVariableType::Bool,  MyOffsetOf( pThis, &pThis->m_IsSensor ),        true, true, nullptr, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, nullptr, nullptr );
    pVar = AddVar( pList, "Friction",      ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_Friction ),        true, true, nullptr, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, nullptr, nullptr );
#if MYFW_EDITOR
    pVar->SetEditorLimits( 0, 1, 0 );
#endif //MYFW_EDITOR
    AddVar( pList, "Restitution",   ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_Restitution ),     true, true, nullptr, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, nullptr, nullptr );

    //AddVar( pList, "Scale",         ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_Scale ),           true, true, nullptr, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, nullptr, nullptr );
#if MYFW_EDITOR
    //for( int i=0; i<6; i++ )
    //    pVars[i]->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&Component2DCollisionObject::ShouldVariableBeAddedToWatchPanel) );
#endif //MYFW_EDITOR
}

Component2DCollisionObject* CastAs_Component2DCollisionObject(ComponentBase* pComponent)
{
    MyAssert( pComponent->IsA( "2DCollisionObjectComponent" ) );
    return (Component2DCollisionObject*)pComponent;
}

void Component2DCollisionObject::Reset()
{
    ComponentBase::Reset();

    m_PrimitiveType = Physics2DPrimitiveType_Box;

    if( m_pBody )
    {
        MyAssert( m_pBox2DWorld );
        m_pBox2DWorld->m_pWorld->DestroyBody( m_pBody );
    }
    m_pBody = nullptr;
    m_pFixture = nullptr;
    m_pBox2DWorld = nullptr;

    m_pComponentLuaScript = nullptr;

    m_Scale.Set( 1,1,1 );

    m_Static = false;
    m_FixedRotation = false;

    m_Density = 1.0f;
    m_IsSensor = false;
    m_Friction = 0.2f;
    m_Restitution = 0.0f;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void Component2DCollisionObject::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate ).addFunction( "CastAs_Component2DCollisionObject", CastAs_Component2DCollisionObject ); // Component2DCollisionObject* CastAs_Component2DCollisionObject(ComponentBase* pComponent)

    luabridge::getGlobalNamespace( luastate )
        .deriveClass<Component2DCollisionObject, ComponentBase>( "Component2DCollisionObject" )
            .addData( "density", &Component2DCollisionObject::m_Density ) // float
            .addFunction( "ClearVelocity", &Component2DCollisionObject::ClearVelocity ) // void Component2DCollisionObject::ClearVelocity()
            .addFunction( "SetPositionAndAngle", &Component2DCollisionObject::SetPositionAndAngle ) // void Component2DCollisionObject::SetPositionAndAngle(Vector2 newPosition, float angle)
            .addFunction( "SetSensor", &Component2DCollisionObject::SetSensor ) // float Component2DCollisionObject::SetSensor(bool isSensor)
            .addFunction( "ApplyForce", &Component2DCollisionObject::ApplyForce ) // void Component2DCollisionObject::ApplyForce(Vector2 force, Vector2 localpoint)
            .addFunction( "ApplyLinearImpulse", &Component2DCollisionObject::ApplyLinearImpulse ) // void Component2DCollisionObject::ApplyLinearImpulse(Vector2 impulse, Vector2 localpoint)
            .addFunction( "GetLinearVelocity", &Component2DCollisionObject::GetLinearVelocity ) // Vector2 Component2DCollisionObject::GetLinearVelocity()
            .addFunction( "GetMass", &Component2DCollisionObject::GetMass ) // float Component2DCollisionObject::GetMass()
            .addFunction( "GetPrimitiveTypeName", &Component2DCollisionObject::GetPrimitiveTypeName ) // const char* Component2DCollisionObject::GetPrimitiveTypeName()
            .addFunction( "IsStatic", &Component2DCollisionObject::IsStatic ) // bool Component2DCollisionObject::IsStatic()
            .addFunction( "IsFixedRotation", &Component2DCollisionObject::IsFixedRotation ) // bool Component2DCollisionObject::IsFixedRotation()
            .addFunction( "GetDensity", &Component2DCollisionObject::GetDensity ) // float Component2DCollisionObject::GetDensity()
            .addFunction( "IsSensor", &Component2DCollisionObject::IsSensor ) // bool Component2DCollisionObject::IsSensor()
            .addFunction( "GetFriction", &Component2DCollisionObject::GetFriction ) // float Component2DCollisionObject::GetFriction()
            .addFunction( "GetRestitution", &Component2DCollisionObject::GetRestitution ) // float Component2DCollisionObject::GetRestitution()
#if MYFW_EDITOR
            .addFunction( "Editor_SetVertices", &Component2DCollisionObject::SetVertices ) // void Component2DCollisionObject::SetVertices(const luabridge::LuaRef verts, unsigned int count)
#endif //MYFW_EDITOR
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR

#if MYFW_USING_IMGUI
void Component2DCollisionObject::AddAllVariablesToWatchPanel(CommandStack* pCommandStack)
{
    ComponentBase::AddAllVariablesToWatchPanel( pCommandStack );

    if( m_PrimitiveType == Physics2DPrimitiveType_Chain )
    {
        //g_pPanelWatch->AddButton( "Edit Chain", this, -1, Component2DCollisionObject::StaticOnButtonEditChain );
        if( ImGui::Button( "Edit Chain" ) )
        {
            OnButtonEditChain( 0 );
        }
    }
}
#endif //MYFW_USING_IMGUI

#if MYFW_USING_WX
void Component2DCollisionObject::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, Component2DCollisionObject::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "2DCollisionObject", ObjectListIcon_Component );
}

void Component2DCollisionObject::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void Component2DCollisionObject::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "2D Collision Object", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST

        if( m_PrimitiveType == Physics2DPrimitiveType_Chain )
        {
            g_pPanelWatch->AddButton( "Edit Chain", this, -1, Component2DCollisionObject::StaticOnButtonEditChain );
        }
    }
}

bool Component2DCollisionObject::ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar)
{
    //if( m_PrimitiveType == Physics2DPrimitiveType_Box )
    //{
    //    if( strcmp( pVar->m_Label, "PlaneSize" ) == 0 )         return true;
    //}
    //if( m_PrimitiveType == Physics2DPrimitiveType_Circle )
    //{
    //    if( strcmp( pVar->m_Label, "PlaneSize" ) == 0 )         return true;
    //}
    //if( m_PrimitiveType == Physics2DPrimitiveType_Edge )
    //{
    //    if( strcmp( pVar->m_Label, "PlaneSize" ) == 0 )         return true;
    //}
    //if( m_PrimitiveType == Physics2DPrimitiveType_Chain )
    //{
    //    if( strcmp( pVar->m_Label, "PlaneSize" ) == 0 )         return true;
    //}
    return true;
}
#endif //MYFW_USING_WX

#if MYFW_USING_LUA
void Component2DCollisionObject::SetVertices(const luabridge::LuaRef verts, unsigned int count)
{
    m_PrimitiveType = Physics2DPrimitiveType_Chain; // TODO: Don't hardcode this.
    m_Static = true; // TODO: Or this.
    
    m_Vertices.resize( count );

    for( uint32 i=0; i<count; i++ )
    {
        Vector2* pVert = verts[i+1];
        m_Vertices[i] = Vector2( pVert->x, pVert->y );
    }
}
#endif //MYFW_USING_LUA

void* Component2DCollisionObject::OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
{
    void* oldPointer = nullptr;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
    {
        // oldPointer = pointer to component.
        //(ComponentBase*)pDropItem->m_Value;
    }

    if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
    {
        // oldPointer = pointer to gameobject.
        //(GameObject*)pDropItem->m_Value;
    }

    return oldPointer;
}

void* Component2DCollisionObject::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = nullptr;

    if( pVar->m_Offset == MyOffsetOf( this, &m_PrimitiveType ) )
    {
#if MYFW_USING_WX
        // TODO: rethink this, doesn't need refresh if panel isn't visible.
        g_pPanelWatch->SetNeedsRefresh();
#endif //MYFW_USING_WX
    }

    // limit some properties
    if( pVar->m_Offset == MyOffsetOf( this, &m_Density ) )
    {
        if( m_Density < 0 )
            m_Density = 0;
    }

    if( pVar->m_Offset == MyOffsetOf( this, &m_Friction ) )
    {
        MyClamp( m_Friction, 0.0f, 1.0f );
    }

    if( pVar->m_Offset == MyOffsetOf( this, &m_Restitution ) )
    {
        MyClamp( m_Restitution, 0.0f, 1.0f );
    }

    // if a body exists, game is running, change the already existing body or fixture.
    if( m_pBody )
    {
        if( pVar->m_Offset == MyOffsetOf( this, &m_Static ) )
        {
            if( m_Static )
                m_pBody->SetType( b2_staticBody );
            else
                m_pBody->SetType( b2_dynamicBody );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_FixedRotation ) )
        {
            m_pBody->SetFixedRotation( m_FixedRotation );
        }        
    }

    if( m_pFixture )
    {
        if( pVar->m_Offset == MyOffsetOf( this, &m_Density ) )
        {
            m_pFixture->SetDensity( m_Density );
            
            if( m_pBody )
            {
                m_pBody->ResetMassData();
            }
            else
            {
                Component2DCollisionObject* pComponentWithBody = (Component2DCollisionObject*)m_pGameObject->GetFirstComponentOfType( "2DCollisionObjectComponent" );
        
                MyAssert( pComponentWithBody != this );
                pComponentWithBody->GetBody()->ResetMassData();
            }
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_IsSensor ) )
        {
            m_pFixture->SetSensor( m_IsSensor );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_Friction ) )
        {
            m_pFixture->SetFriction( m_Friction );
        }

        if( pVar->m_Offset == MyOffsetOf( this, &m_Restitution ) )
        {
            m_pFixture->SetRestitution( m_Restitution );
        }
    }

    return oldpointer;
}

void Component2DCollisionObject::OnTransformChanged(const Vector3& newPos, const Vector3& newRot, const Vector3& newScale, bool changedByUserInEditor)
{
    if( changedByUserInEditor )
        SyncRigidBodyToTransform();
}

void Component2DCollisionObject::OnButtonEditChain(int buttonID)
{
    g_pEngineCore->SetEditorInterface( EditorInterfaceType::The2DPointEditor );
    ((EditorInterface_2DPointEditor*)g_pEngineCore->GetCurrentEditorInterface())->Set2DCollisionObjectToEdit( this );
}
#endif //MYFW_EDITOR

cJSON* Component2DCollisionObject::ExportAsJSONObject(bool saveSceneID, bool saveID)
{
    cJSON* jComponent = ComponentBase::ExportAsJSONObject( saveSceneID, saveID );

#if MYFW_EDITOR
    int count = (int)m_Vertices.size();
#else
    int count = (int)m_Vertices.Count();
#endif

    if( count > 0 )
        cJSONExt_AddFloatArrayToObject( jComponent, "Vertices", &m_Vertices[0].x, count * 2 );

    return jComponent;
}

void Component2DCollisionObject::ImportFromJSONObject(cJSON* jObject, SceneID sceneID)
{
    ComponentBase::ImportFromJSONObject( jObject, sceneID );

    cJSON* jVertexArray = cJSON_GetObjectItem( jObject, "Vertices" );
    if( jVertexArray )
    {
        int arraysize = cJSON_GetArraySize( jVertexArray );

#if MYFW_EDITOR
        m_Vertices.resize( arraysize / 2 );
#else
        m_Vertices.FreeAllInList();
        m_Vertices.AllocateObjects( arraysize / 2 );
        Vector2 zerovec(0,0);
        for( int i=0; i<arraysize/2; i++ )
            m_Vertices.Add( zerovec );
#endif

        cJSONExt_GetFloatArray( jObject, "Vertices", &m_Vertices[0].x, arraysize );
    }
}

Component2DCollisionObject& Component2DCollisionObject::operator=(const Component2DCollisionObject& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_PrimitiveType = other.m_PrimitiveType;

    m_Offset = other.m_Offset;
    //m_Scale = other.m_Scale;

    // Body properties.
    m_Static = other.m_Static;
    m_FixedRotation = other.m_FixedRotation;

    // Fixture properties.
    m_Density = other.m_Density;
    m_IsSensor = other.m_IsSensor;
    m_Friction = other.m_Friction;
    m_Restitution = other.m_Restitution;

    // Copy vertices.
#if MYFW_EDITOR
    m_Vertices = other.m_Vertices;
#else
    m_Vertices.FreeAllInList();
    m_Vertices.AllocateObjects( other.m_Vertices.Count() );
    for( unsigned int i=0; i<m_Vertices.Count(); i++ )
        m_Vertices[i] = other.m_Vertices[i];
#endif

    return *this;
}

void Component2DCollisionObject::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( Component2DCollisionObject, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DCollisionObject, OnSurfaceChanged );
#if MYFW_EDITOR
        //MYFW_FILL_COMPONENT_CALLBACK_STRUCT( Component2DCollisionObject, Draw );
        MYFW_REGISTER_COMPONENT_CALLBACK( Component2DCollisionObject, Draw );
#endif
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DCollisionObject, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DCollisionObject, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DCollisionObject, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( Component2DCollisionObject, OnFileRenamed );
    }
}

void Component2DCollisionObject::UnregisterCallbacks()
{
    MyAssert( m_EnabledState != EnabledState_Enabled );

    if( m_CallbacksRegistered == true )
    {
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
#if MYFW_EDITOR
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
#endif
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

void Component2DCollisionObject::OnPlay()
{
    ComponentBase::OnPlay();

    if( m_pComponentLuaScript == nullptr )
    {
        m_pComponentLuaScript = (ComponentLuaScript*)m_pGameObject->GetFirstComponentOfType( "LuaScriptComponent" );
    }

    MyAssert( m_pGameObject->GetPhysicsSceneID() < MAX_SCENES_LOADED );
    SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( m_pGameObject->GetPhysicsSceneID() );
    if( pSceneInfo )
    {
        m_pBox2DWorld = pSceneInfo->m_pBox2DWorld;
        MyAssert( m_pBox2DWorld );
    }

    MyAssert( m_pBody == nullptr );
    if( m_pBody != nullptr )
    {
        if( m_pBox2DWorld )
        {
            m_pBox2DWorld->m_pWorld->DestroyBody( m_pBody );
            m_pBody = nullptr;
        }
    }
    MyAssert( m_pFixture == nullptr );
    m_pFixture = nullptr;

    CreateBody();
}

void Component2DCollisionObject::OnStop()
{
    ComponentBase::OnStop();

    if( m_pBody )
    {
        m_pBox2DWorld->m_pWorld->DestroyBody( m_pBody );
        m_pBody = nullptr;
    }
    m_pFixture = nullptr;
    m_pBox2DWorld = nullptr;
}

bool Component2DCollisionObject::SetEnabled(bool enableComponent)
{
    if( ComponentBase::SetEnabled( enableComponent ) == false )
        return false;

    if( m_pBody == nullptr )
        return true;

    m_pBody->SetEnabled( enableComponent );

    return true;
}

void Component2DCollisionObject::CreateBody()
{
    MyAssert( m_pBody == nullptr );
    MyAssert( m_pBox2DWorld );

    if( m_pBody != nullptr )
        return;

    if( m_pBox2DWorld == nullptr )
        return;

    b2Body* pBody = nullptr;

    // create a body on start
    if( m_pBody == nullptr )
    {
        Component2DCollisionObject* pComponentWithBody = (Component2DCollisionObject*)m_pGameObject->GetFirstComponentOfType( "2DCollisionObjectComponent" );
        
        // if this is the first component of this type, create the body, otherwise get the body from the first component.
        if( pComponentWithBody == this )
        {
            Vector3 pos = m_pGameObject->GetTransform()->GetWorldPosition();
            Vector3 rot = m_pGameObject->GetTransform()->GetWorldRotation();

            b2BodyDef bodyDef;
        
            bodyDef.position = b2Vec2( pos.x, pos.y );
            bodyDef.angle = -rot.z / 180 * PI;
            if( m_Static )
                bodyDef.type = b2_staticBody;
            else
                bodyDef.type = b2_dynamicBody;
            bodyDef.fixedRotation = m_FixedRotation;
            bodyDef.userData.pointer = (uintptr_t)(void*)this;

            m_pBody = m_pBox2DWorld->m_pWorld->CreateBody( &bodyDef );

            m_Scale = m_pGameObject->GetTransform()->GetWorldScale();

            pBody = m_pBody;
        }
        else
        {
            pBody = pComponentWithBody->GetBody();
        }
    }

    if( pBody != nullptr )
    {
        // Set up the fixture
        b2FixtureDef fixtureDef;
        fixtureDef.density = m_Density;
        fixtureDef.isSensor = m_IsSensor;
        fixtureDef.friction = m_Friction;
        fixtureDef.restitution = m_Restitution;
        fixtureDef.filter.categoryBits = 0x0001;
        fixtureDef.filter.maskBits = 0xFFFF;
        fixtureDef.filter.groupIndex = 0;

        // Create the right shape and add it to the fixture def
        switch( m_PrimitiveType )
        {
        case Physics2DPrimitiveType_Box:
            {
                b2PolygonShape boxShape;

                b2Vec2 verts[4];
                verts[0].Set( -0.5f * m_Scale.x + m_Offset.x, -0.5f * m_Scale.y + m_Offset.y );
                verts[1].Set(  0.5f * m_Scale.x + m_Offset.x, -0.5f * m_Scale.y + m_Offset.y );
                verts[2].Set(  0.5f * m_Scale.x + m_Offset.x,  0.5f * m_Scale.y + m_Offset.y );
                verts[3].Set( -0.5f * m_Scale.x + m_Offset.x,  0.5f * m_Scale.y + m_Offset.y );

                boxShape.Set( verts, 4 );

                fixtureDef.shape = &boxShape;

                m_pFixture = pBody->CreateFixture( &fixtureDef );
            }
            break;

        case Physics2DPrimitiveType_Circle:
            {
                b2CircleShape circleShape;
                circleShape.m_p.Set( m_Offset.x, m_Offset.y );
                circleShape.m_radius = m_Scale.x;

                fixtureDef.shape = &circleShape;

                m_pFixture = pBody->CreateFixture( &fixtureDef );
            }
            break;

        case Physics2DPrimitiveType_Edge:
            {
                b2EdgeShape edgeShape;

                // TODO: define edges so they're not limited to x/y axes.
                if( m_Scale.x > m_Scale.y )
                    edgeShape.SetTwoSided( b2Vec2( m_Offset.x + -0.5f * m_Scale.x, m_Offset.y + 0 ), b2Vec2( m_Offset.x + 0.5f * m_Scale.x, m_Offset.y + 0 ) );
                else
                    edgeShape.SetTwoSided( b2Vec2( m_Offset.x + 0, m_Offset.y + -0.5f * m_Scale.y ), b2Vec2( m_Offset.x + 0, m_Offset.y + 0.5f * m_Scale.y ) );

                fixtureDef.shape = &edgeShape;

                m_pFixture = pBody->CreateFixture( &fixtureDef );
            }
            break;

        case Physics2DPrimitiveType_Chain:
            {
                b2ChainShape chainShape;

#if MYFW_EDITOR
                int count = (int)m_Vertices.size();

                if( count == 0 )
                {
                    m_Vertices.push_back( Vector2( -5, 0 ) );
                    m_Vertices.push_back( Vector2(  5, 0 ) );
                    count = 2;
                }
#else
                int count = m_Vertices.Count();
#endif
                for( int i=0; i<count; i++ )
                {
                    m_Vertices[i].x += m_Offset.x;
                    m_Vertices[i].y += m_Offset.y;
                }

                if( count > 0 )
                {
                    chainShape.CreateChain( (b2Vec2*)&m_Vertices[0], count, *(b2Vec2*)&m_Vertices[0], *(b2Vec2*)&m_Vertices[count-1] );

                    fixtureDef.shape = &chainShape;

                    m_pFixture = pBody->CreateFixture( &fixtureDef );
                }
            }
        }

        pBody->SetEnabled( m_EnabledState == EnabledState_Enabled );
    }
}

void Component2DCollisionObject::TickCallback(float deltaTime)
{
    //ComponentBase::Tick( deltaTime );

    if( deltaTime == 0 )
        return;

    if( m_pBody == nullptr )
        return;

    b2Vec2 pos = m_pBody->GetPosition();
    float angle = -m_pBody->GetAngle() / PI * 180.0f;

    MyMatrix matWorld;

    Vector3 oldPos = m_pGameObject->GetTransform()->GetWorldPosition();
    //Vector3 oldRot = m_pGameObject->GetTransform()->GetWorldRotation();

    matWorld.CreateSRT( m_Scale, Vector3( 0, 0, angle ), Vector3( pos.x, pos.y, oldPos.z ) );
    m_pGameObject->GetTransform()->SetWorldTransform( &matWorld );
}

#if MYFW_EDITOR
void Component2DCollisionObject::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride)
{
    if( m_Vertices.size() == 0 )
        return;

    if( g_GLCanvasIDActive != 1 )
        return;

    // Kick out early if we don't want to draw the lines for the vertices.
    EditorInterfaceType interfaceType = g_pEngineCore->GetCurrentEditorInterfaceType();
    if( interfaceType == EditorInterfaceType::The2DPointEditor )
    {
        // if we're not the chain being edited, don't draw the lines.
        EditorInterface_2DPointEditor* pInterface = (EditorInterface_2DPointEditor*)g_pEngineCore->GetCurrentEditorInterface();
        if( pInterface->Get2DCollisionObjectBeingEdited() != this )
            return;
    }
    else
    {
        if( g_pEngineCore->GetEditorPrefs()->Get_Debug_DrawPhysicsDebugShapes() == false )
            return;
    }

    //ComponentCamera* pCamera = g_pEngineCore->GetEditorState()->GetEditorCamera();
    MyMatrix* pEditorMatProj = &pCamera->m_Camera3D.m_matProj;
    MyMatrix* pEditorMatView = &pCamera->m_Camera3D.m_matView;

    //MaterialDefinition* pMaterial = g_pEngineCore->GetEditorState()->m_pTransformGizmo->m_pMaterial_Translate1Axis[1];
    EditorInterface_2DPointEditor* pInterface = ((EditorInterface_2DPointEditor*)g_pEngineCore->GetEditorInterface( EditorInterfaceType::The2DPointEditor ));
    MyAssert( pInterface );
    MaterialDefinition* pMaterial = pInterface->GetMaterial( EditorInterface_2DPointEditor::Mat_Lines );
    MyAssert( pMaterial );

    // Draw lines for the vertices (connecting the circles if editing verts)
    {
        // Set the material to the correct color and draw the shape.
        Shader_Base* pShader = (Shader_Base*)pMaterial->GetShader()->GlobalPass( 0, 0 );
        if( pShader->Activate() == false )
            return;

        pMaterial->SetColorDiffuse( ColorByte( 0, 255, 0, 255 ) );

        // Setup our position attribute, pass in the array of verts, not using a VBO.
        pShader->InitializeAttributeArrays( VertexFormat_None, nullptr, 0, 0 );
        pShader->InitializeAttributeArray( Shader_Base::Attribute_Position, 2, MyRE::AttributeType_Float, false, sizeof(float)*2, (void*)&m_Vertices[0] );

        ComponentTransform* pParentTransformComponent = m_pGameObject->GetTransform();
        MyMatrix worldMat;
        worldMat.SetIdentity();
        worldMat.SetTranslation( pParentTransformComponent->GetWorldPosition() );

        // Setup uniforms.
        pShader->ProgramMaterialProperties( nullptr, pMaterial->m_ColorDiffuse, pMaterial->m_ColorSpecular, pMaterial->m_Shininess );
        pShader->ProgramTransforms( pEditorMatProj, pEditorMatView, &worldMat );

        g_pRenderer->SetLineWidth( 3.0f );

        g_pRenderer->SetBlendEnabled( true );
        g_pRenderer->SetBlendFunc( MyRE::BlendFactor_SrcAlpha, MyRE::BlendFactor_OneMinusSrcAlpha );

        //g_pRenderer->SetCullingEnabled( false );
        //g_pRenderer->SetDepthTestEnabled( false );

        g_pRenderer->DrawArrays( MyRE::PrimitiveType_LineStrip, 0, (int)m_Vertices.size(), false );

        g_pRenderer->SetLineWidth( 1.0f );

        //g_pRenderer->SetCullingEnabled( true );
        //g_pRenderer->SetDepthTestEnabled( true );

        // Always disable blending.
        g_pRenderer->SetBlendEnabled( false );
    }
}
#endif //MYFW_EDITOR

void Component2DCollisionObject::SyncRigidBodyToTransform()
{
    if( m_pBody == nullptr )
        return;
}

// Exposed to Lua, change elsewhere if function signature changes.
Vector2 Component2DCollisionObject::GetLinearVelocity()
{
    b2Vec2 b2velocity = m_pBody->GetLinearVelocity();
    return Vector2( b2velocity.x, b2velocity.y );
}

// Exposed to Lua, change elsewhere if function signature changes.
float Component2DCollisionObject::GetMass()
{
    if( m_pBody )
        return m_pBody->GetMass();

    return 0.0f;
}

const char* Component2DCollisionObject::GetPrimitiveTypeName()
{
    if( m_PrimitiveType >= 0 && m_PrimitiveType < Physics2DPrimitive_NumTypes )
        return Physics2DPrimitiveTypeStrings[m_PrimitiveType];

    return nullptr;
}

bool Component2DCollisionObject::IsStatic()
{
    return m_Static;
}

bool Component2DCollisionObject::IsFixedRotation()
{
    return m_FixedRotation;
}

float Component2DCollisionObject::GetDensity()
{
    return m_Density;
}

bool Component2DCollisionObject::IsSensor()
{
    return m_IsSensor;
}

float Component2DCollisionObject::GetFriction()
{
    return m_Friction;
}

float Component2DCollisionObject::GetRestitution()
{
    return m_Restitution;
}

// Exposed to Lua, change elsewhere if function signature changes.
void Component2DCollisionObject::SetPositionAndAngle(Vector2 newPosition, float angle)
{
    b2Vec2 b2Position = b2Vec2( newPosition.x, newPosition.y );

    m_pBody->SetTransform( b2Position, angle ); 
}

// Exposed to Lua, change elsewhere if function signature changes.
void Component2DCollisionObject::SetSensor(bool isSensor)
{
    m_IsSensor = isSensor;

    if( m_pFixture )
    {
        m_pFixture->SetSensor( m_IsSensor );
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
void Component2DCollisionObject::ClearVelocity()
{
    if( m_pBody )
    {
        m_pBody->SetLinearVelocity( b2Vec2(0,0) );
        m_pBody->SetAngularVelocity( 0.0f );
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
void Component2DCollisionObject::ApplyForce(Vector2 force, Vector2 localPoint)
{
    b2Vec2 b2force = b2Vec2( force.x, force.y );

    // apply force to center of mass + offset
    b2MassData massData;
    m_pBody->GetMassData( &massData );
    b2Vec2 worldPoint = m_pBody->GetWorldPoint( massData.center + *(b2Vec2*)&localPoint );

    m_pBody->ApplyForce( b2force, worldPoint, true );
}

// Exposed to Lua, change elsewhere if function signature changes.
void Component2DCollisionObject::ApplyLinearImpulse(Vector2 impulse, Vector2 localPoint)
{
    b2Vec2 b2impulse = b2Vec2( impulse.x, impulse.y );
    
    // apply force to center of mass + offset
    b2MassData massData;
    m_pBody->GetMassData( &massData );
    b2Vec2 worldpoint = m_pBody->GetWorldPoint( massData.center + *(b2Vec2*)&localPoint );
    
    m_pBody->ApplyLinearImpulse( b2impulse, worldpoint, true );
}
