//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool Component2DCollisionObject::m_PanelWatchBlockVisible = true;
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

Component2DCollisionObject::Component2DCollisionObject()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    //m_Type = ComponentType_2DCollisionObject;

    m_pComponentLuaScript = 0;

    m_pBox2DWorld = 0;
    m_pBody = 0;
    m_pFixture = 0;

    m_PrimitiveType = Physics2DPrimitiveType_Box;

    m_Offset.Set( 0, 0 );
    m_Scale.Set( 1,1,1 );

    m_Static = false;
    m_Density = 1.0f;
    m_IsSensor = false;
    m_Friction = 0.2f;
    m_Restitution = 0;
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

void Component2DCollisionObject::RegisterVariables(CPPListHead* pList, Component2DCollisionObject* pThis) //_VARIABLE_LIST
{
    ComponentVariable* pVar;

    pVar = AddVar( pList, "Offset", ComponentVariableType_Vector2, MyOffsetOf( pThis, &pThis->m_Offset ), true, true, 0, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, 0, 0 );
    //pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&Component2DCollisionObject::ShouldVariableBeAddedToWatchPanel) );

    AddVarEnum( pList, "PrimitiveType", MyOffsetOf( pThis, &pThis->m_PrimitiveType ),   true, true, "Primitive Type", Physics2DPrimitive_NumTypes, Physics2DPrimitiveTypeStrings, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, 0, 0 );

    AddVar( pList, "Static",        ComponentVariableType_Bool,  MyOffsetOf( pThis, &pThis->m_Static ),          true, true, 0, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, 0, 0 );
    AddVar( pList, "Density",       ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_Density ),         true, true, 0, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, 0, 0 );
    AddVar( pList, "IsSensor",      ComponentVariableType_Bool,  MyOffsetOf( pThis, &pThis->m_IsSensor ),        true, true, 0, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, 0, 0 );
    pVar = AddVar( pList, "Friction",      ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_Friction ),        true, true, 0, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, 0, 0 );
    pVar->SetEditorLimits( 0, 1 );
    AddVar( pList, "Restitution",   ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_Restitution ),     true, true, 0, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, 0, 0 );

    //AddVar( pList, "Scale",         ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_Scale ),           true, true, 0, (CVarFunc_ValueChanged)&Component2DCollisionObject::OnValueChanged, 0, 0 );
#if MYFW_EDITOR
    //for( int i=0; i<6; i++ )
    //    pVars[i]->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&Component2DCollisionObject::ShouldVariableBeAddedToWatchPanel) );
#endif //MYFW_EDITOR
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
    m_pBody = 0;
    m_pFixture = 0;
    m_pBox2DWorld = 0;

    m_pComponentLuaScript = 0;

    m_Scale.Set( 1,1,1 );

    m_Static = false;
    m_Density = 0;
    m_IsSensor = false;
    m_Friction = 0.2f;
    m_Restitution = 0;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void Component2DCollisionObject::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<Component2DCollisionObject>( "Component2DCollisionObject" )
            .addData( "density", &Component2DCollisionObject::m_Density ) // float
            .addFunction( "ApplyForce", &Component2DCollisionObject::ApplyForce ) // void Component2DCollisionObject::ApplyForce(Vector2 force, Vector2 localpoint)
            .addFunction( "ApplyLinearImpulse", &Component2DCollisionObject::ApplyLinearImpulse ) // void Component2DCollisionObject::ApplyLinearImpulse(Vector2 impulse, Vector2 localpoint)
            .addFunction( "GetLinearVelocity", &Component2DCollisionObject::GetLinearVelocity ) // Vector2 Component2DCollisionObject::GetLinearVelocity()
            .addFunction( "GetMass", &Component2DCollisionObject::GetMass ) // float Component2DCollisionObject::GetMass()
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
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

void* Component2DCollisionObject::OnDrop(ComponentVariable* pVar, int x, int y)
{
    void* oldvalue = 0;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
    {
        //(ComponentBase*)pDropItem->m_Value;
    }

    if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
    {
        //(GameObject*)pDropItem->m_Value;
    }

    return oldvalue;
}

void* Component2DCollisionObject::OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = 0;

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

void Component2DCollisionObject::OnTransformChanged(Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor)
{
    if( changedbyuserineditor )
        SyncRigidBodyToTransform();
}

void Component2DCollisionObject::OnButtonEditChain(int buttonid)
{
    g_pEngineCore->SetEditorInterface( EditorInterfaceType_2DPointEditor );
    ((EditorInterface_2DPointEditor*)g_pEngineCore->GetCurrentEditorInterface())->Set2DCollisionObjectToEdit( this );
}
#endif //MYFW_EDITOR

cJSON* Component2DCollisionObject::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid, saveid );

#if MYFW_EDITOR
    int count = (int)m_Vertices.size();
#else
    int count = (int)m_Vertices.Count();
#endif

    if( count > 0 )
        cJSONExt_AddFloatArrayToObject( jComponent, "Vertices", &m_Vertices[0].x, count * 2 );

    return jComponent;
}

void Component2DCollisionObject::ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid)
{
    ComponentBase::ImportFromJSONObject( jsonobj, sceneid );

    cJSON* jVertexArray = cJSON_GetObjectItem( jsonobj, "Vertices" );
    if( jVertexArray )
    {
        int arraysize = cJSON_GetArraySize( jVertexArray );

#if MYFW_EDITOR
        m_Vertices.resize( arraysize / 2 );
#else
        m_Vertices.FreeAllInList();
        m_Vertices.AllocateObjects( arraysize / 2 );
        b2Vec2 zerovec(0,0);
        for( int i=0; i<arraysize/2; i++ )
            m_Vertices.Add( zerovec );
#endif

        cJSONExt_GetFloatArray( jsonobj, "Vertices", &m_Vertices[0].x, arraysize );
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

    m_Static = other.m_Static;
    m_Density = other.m_Density;
    m_IsSensor = other.m_IsSensor;
    m_Friction = other.m_Friction;
    m_Restitution = other.m_Restitution;

    // copy vertices
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
    if( m_Enabled && m_CallbacksRegistered == false )
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

    if( m_pComponentLuaScript == 0 )
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

    MyAssert( m_pBody == 0 );
    if( m_pBody != 0 )
    {
        if( m_pBox2DWorld )
        {
            m_pBox2DWorld->m_pWorld->DestroyBody( m_pBody );
            m_pBody = 0;
        }
    }
    MyAssert( m_pFixture == 0 );
    m_pFixture = 0;

    CreateBody();
}

void Component2DCollisionObject::OnStop()
{
    ComponentBase::OnStop();

    if( m_pBody )
    {
        m_pBox2DWorld->m_pWorld->DestroyBody( m_pBody );
        m_pBody = 0;
    }
    m_pFixture = 0;
    m_pBox2DWorld = 0;
}

void Component2DCollisionObject::CreateBody()
{
    MyAssert( m_pBody == 0 );
    MyAssert( m_pBox2DWorld );

    if( m_pBody != 0 )
        return;

    if( m_pBox2DWorld == 0 )
        return;

    b2Body* pBody = 0;

    // create a body on start
    if( m_pBody == 0 )
    {
        Component2DCollisionObject* pComponentWithBody = (Component2DCollisionObject*)m_pGameObject->GetFirstComponentOfType( "2DCollisionObjectComponent" );
        
        // if this is the first component of this type, create the body, otherwise get the body from the first component.
        if( pComponentWithBody == this )
        {
            Vector3 pos = m_pGameObject->GetTransform()->GetWorldPosition();
            Vector3 rot = m_pGameObject->GetTransform()->GetWorldRotation();

            b2BodyDef bodydef;
        
            bodydef.position = b2Vec2( pos.x, pos.y );
            bodydef.angle = -rot.z / 180 * PI;
            if( m_Static )
                bodydef.type = b2_staticBody;
            else
                bodydef.type = b2_dynamicBody;

            m_pBody = m_pBox2DWorld->m_pWorld->CreateBody( &bodydef );
            m_pBody->SetUserData( this );

            m_Scale = m_pGameObject->GetTransform()->GetWorldScale();

            pBody = m_pBody;
        }
        else
        {
            pBody = pComponentWithBody->GetBody();
        }
    }

    if( pBody != 0 )
    {
        // Set up the fixture
        b2FixtureDef fixturedef;
        fixturedef.density = m_Density;
        fixturedef.isSensor = m_IsSensor;
        fixturedef.friction = m_Friction;
        fixturedef.restitution = m_Restitution;
        fixturedef.filter.categoryBits = 0x0001;
        fixturedef.filter.maskBits = 0xFFFF;
        fixturedef.filter.groupIndex = 0;

        // Create the right shape and add it to the fixture def
        switch( m_PrimitiveType )
        {
        case Physics2DPrimitiveType_Box:
            {
                b2PolygonShape boxshape;

                b2Vec2 verts[4];
                verts[0].Set( -0.5f * m_Scale.x + m_Offset.x, -0.5f * m_Scale.y + m_Offset.y );
                verts[1].Set(  0.5f * m_Scale.x + m_Offset.x, -0.5f * m_Scale.y + m_Offset.y );
                verts[2].Set(  0.5f * m_Scale.x + m_Offset.x,  0.5f * m_Scale.y + m_Offset.y );
                verts[3].Set( -0.5f * m_Scale.x + m_Offset.x,  0.5f * m_Scale.y + m_Offset.y );

                boxshape.Set( verts, 4 );

                fixturedef.shape = &boxshape;

                m_pFixture = pBody->CreateFixture( &fixturedef );
            }
            break;

        case Physics2DPrimitiveType_Circle:
            {
                b2CircleShape circleshape;
                circleshape.m_p.Set( m_Offset.x, m_Offset.y );
                circleshape.m_radius = m_Scale.x;

                fixturedef.shape = &circleshape;

                m_pFixture = pBody->CreateFixture( &fixturedef );
            }
            break;

        case Physics2DPrimitiveType_Edge:
            {
                b2EdgeShape edgeshape;

                // TODO: define edges so they're not limited to x/y axes.
                if( m_Scale.x > m_Scale.y )
                    edgeshape.Set( b2Vec2( m_Offset.x + -0.5f * m_Scale.x, m_Offset.y + 0 ), b2Vec2( m_Offset.x + 0.5f * m_Scale.x, m_Offset.y + 0 ) );
                else
                    edgeshape.Set( b2Vec2( m_Offset.x + 0, m_Offset.y + -0.5f * m_Scale.y ), b2Vec2( m_Offset.x + 0, m_Offset.y + 0.5f * m_Scale.y ) );

                fixturedef.shape = &edgeshape;

                m_pFixture = pBody->CreateFixture( &fixturedef );
            }
            break;

        case Physics2DPrimitiveType_Chain:
            {
                b2ChainShape chainshape;

#if MYFW_EDITOR
                int count = (int)m_Vertices.size();

                if( count == 0 )
                {
                    m_Vertices.push_back( b2Vec2( -5, 0 ) );
                    m_Vertices.push_back( b2Vec2(  5, 0 ) );
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
                    chainshape.CreateChain( &m_Vertices[0], count );

                    fixturedef.shape = &chainshape;

                    m_pFixture = pBody->CreateFixture( &fixturedef );
                }
            }
        }
    }
}

void Component2DCollisionObject::TickCallback(double TimePassed)
{
    //ComponentBase::Tick( TimePassed );

    if( TimePassed == 0 )
        return;

    if( m_pBody == 0 )
        return;

    b2Vec2 pos = m_pBody->GetPosition();
    float32 angle = -m_pBody->GetAngle() / PI * 180.0f;

    MyMatrix matWorld;

    Vector3 oldpos = m_pGameObject->GetTransform()->GetWorldPosition();
    //Vector3 oldrot = m_pGameObject->GetTransform()->GetWorldRotation();

    matWorld.CreateSRT( m_Scale, Vector3( 0, 0, angle ), Vector3( pos.x, pos.y, oldpos.z ) );
    m_pGameObject->GetTransform()->SetWorldTransform( &matWorld );
}

#if MYFW_EDITOR
void Component2DCollisionObject::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    if( m_Vertices.size() == 0 )
        return;

    if( g_GLCanvasIDActive != 1 )
        return;

    // Kick out early if we don't want to draw the lines for the vertices.
    EditorInterfaceTypes interfacetype = g_pEngineCore->GetCurrentEditorInterfaceType();
    if( interfacetype == EditorInterfaceType_2DPointEditor )
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
    MyMatrix* pEditorMatViewProj = &pCamera->m_Camera3D.m_matViewProj;

    //MaterialDefinition* pMaterial = g_pEngineCore->GetEditorState()->m_pTransformGizmo->m_pMaterial_Translate1Axis[1];
    EditorInterface_2DPointEditor* pInterface = ((EditorInterface_2DPointEditor*)g_pEngineCore->GetEditorInterface(EditorInterfaceType_2DPointEditor));
    MyAssert( pInterface );
    MaterialDefinition* pMaterial = pInterface->GetMaterial( EditorInterface_2DPointEditor::Mat_Lines );
    MyAssert( pMaterial );

    // Draw lines for the vertices (connecting the circles if editing verts)
    {
        // Set the material to the correct color and draw the shape.
        Shader_Base* pShader = (Shader_Base*)pMaterial->GetShader()->GlobalPass( 0, 0 );
        if( pShader->ActivateAndProgramShader() == false )
            return;

        pMaterial->SetColorDiffuse( ColorByte( 0, 255, 0, 255 ) );

        // Setup our position attribute, pass in the array of verts, not using a VBO.
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        pShader->InitializeAttributeArray( pShader->m_aHandle_Position, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)&m_Vertices[0] );

        ComponentTransform* pParentTransformComponent = m_pGameObject->GetTransform();
        MyMatrix worldmat;
        worldmat.SetIdentity();
        worldmat.SetTranslation( pParentTransformComponent->GetWorldPosition() );

        // Setup uniforms, mainly viewproj and tint.
        pShader->ProgramBaseUniforms( pEditorMatViewProj, &worldmat, 0, pMaterial->m_ColorDiffuse, pMaterial->m_ColorSpecular, pMaterial->m_Shininess );

        glLineWidth( 3 );

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        //glDisable( GL_CULL_FACE );
        //glDisable( GL_DEPTH_TEST );

        MyDrawArrays( GL_LINE_STRIP, 0, (int)m_Vertices.size() );

        glLineWidth( 1 );

        //glEnable( GL_CULL_FACE );
        //glEnable( GL_DEPTH_TEST );

        glDisable( GL_BLEND );
    }
}
#endif //MYFW_EDITOR

void Component2DCollisionObject::SyncRigidBodyToTransform()
{
    if( m_pBody == 0 )
        return;
}

// Exposed to Lua, change elsewhere if function signature changes.
void Component2DCollisionObject::ApplyForce(Vector2 force, Vector2 localpoint)
{
    b2Vec2 b2force = b2Vec2( force.x, force.y );

    // apply force to center of mass + offset
    b2MassData massData;
    m_pBody->GetMassData( &massData );
    b2Vec2 worldpoint = m_pBody->GetWorldPoint( massData.center + *(b2Vec2*)&localpoint );

    m_pBody->ApplyForce( b2force, worldpoint, true );
}

// Exposed to Lua, change elsewhere if function signature changes.
void Component2DCollisionObject::ApplyLinearImpulse(Vector2 impulse, Vector2 localpoint)
{
    b2Vec2 b2impulse = b2Vec2( impulse.x, impulse.y );
    
    // apply force to center of mass + offset
    b2MassData massData;
    m_pBody->GetMassData( &massData );
    b2Vec2 worldpoint = m_pBody->GetWorldPoint( massData.center + *(b2Vec2*)&localpoint );
    
    m_pBody->ApplyLinearImpulse( b2impulse, worldpoint, true );
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
    return m_pBody->GetMass();
}
