//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"

#if MYFW_USING_WX
bool ComponentCollisionObject2D::m_PanelWatchBlockVisible = true;
#endif

const char* Physics2DPrimitiveTypeStrings[Physics2DPrimitive_NumTypes] = //ADDING_NEW_Physics2DPrimitiveType
{
    "Box",
    "Circle",
};

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentCollisionObject2D ); //_VARIABLE_LIST

ComponentCollisionObject2D::ComponentCollisionObject2D()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    //m_Type = ComponentType_CollisionObject2D;

    m_pBody = 0;

    m_PrimitiveType = Physics2DPrimitiveType_Box;

    m_Static = false;
    m_Mass = 0;
    //m_Scale.Set( 1,1,1 );
    //m_pMesh = 0;
}

ComponentCollisionObject2D::~ComponentCollisionObject2D()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    if( m_pBody )
    {
        g_pBox2DWorld->m_pWorld->DestroyBody( m_pBody );
    }

    //SAFE_RELEASE( m_pMesh );
}

void ComponentCollisionObject2D::RegisterVariables(CPPListHead* pList, ComponentCollisionObject2D* pThis) //_VARIABLE_LIST
{
    // just want to make sure these are the same on all compilers.  They should be since this is a simple class.
#if MYFW_IOS || MYFW_OSX || MYFW_NACL
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
    //MyAssert( offsetof( ComponentCollisionObject2D, m_SampleVector3 ) == MyOffsetOf( pThis, &pThis->m_SampleVector3 ) );
#if MYFW_IOS || MYFW_OSX
#pragma GCC diagnostic default "-Winvalid-offsetof"
#endif

    AddVar( pList, "PrimitiveType", ComponentVariableType_Int,   MyOffsetOf( pThis, &pThis->m_PrimitiveType ),   true, true, 0, (CVarFunc_ValueChanged)&ComponentCollisionObject2D::OnValueChanged, 0, 0 );
    AddVar( pList, "Static",        ComponentVariableType_Bool,  MyOffsetOf( pThis, &pThis->m_Static ),          true, true, 0, (CVarFunc_ValueChanged)&ComponentCollisionObject2D::OnValueChanged, 0, 0 );
    AddVar( pList, "Mass",          ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_Mass ),            true, true, 0, (CVarFunc_ValueChanged)&ComponentCollisionObject2D::OnValueChanged, 0, 0 );
    //AddVar( pList, "Scale",         ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_Scale ),           true, true, 0, (CVarFunc_ValueChanged)&ComponentCollisionObject2D::OnValueChanged, 0, 0 );
}

void ComponentCollisionObject2D::Reset()
{
    ComponentBase::Reset();

    m_PrimitiveType = Physics2DPrimitiveType_Box;

    m_Static = false;
    m_Mass = 0;
    //m_Scale.Set( 1,1,1 );
    //SAFE_RELEASE( m_pMesh );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentCollisionObject2D::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentCollisionObject2D>( "ComponentCollisionObject2D" )
            .addData( "mass", &ComponentCollisionObject2D::m_Mass )            
            .addFunction( "ApplyForce", &ComponentCollisionObject2D::ApplyForce )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
void ComponentCollisionObject2D::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentCollisionObject2D::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "CollisionObject2D" );
}

void ComponentCollisionObject2D::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentCollisionObject2D::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Template", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}

void* ComponentCollisionObject2D::OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldvalue = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_ComponentPointer )
    {
        (ComponentBase*)g_DragAndDropStruct.m_Value;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        (GameObject*)g_DragAndDropStruct.m_Value;
    }

    return oldvalue;
}

void* ComponentCollisionObject2D::OnValueChanged(ComponentVariable* pVar, bool finishedchanging, double oldvalue)
{
    void* oldpointer = 0;

    //if( controlid == m_ControlID_PrimitiveType )
    //{
    //    // TODO: rethink this, doesn't need refresh if panel isn't visible.
    //    g_pPanelWatch->m_NeedsRefresh = true;
    //}

    //if( pVar->m_Offset == MyOffsetOf( this, &m_SampleVector3 ) )
    //{
    //    MyAssert( pVar->m_ControlID != -1 );
    //}

    return oldpointer;
}

//void ComponentCollisionObject2D::OnDropOBJ(int controlid, wxCoord x, wxCoord y)
//{
//    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
//    {
//        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
//        MyAssert( pFile );
//        //MyAssert( m_pMesh );
//
//        size_t len = strlen( pFile->m_FullPath );
//        const char* filenameext = &pFile->m_FullPath[len-4];
//
//        if( strcmp( filenameext, ".obj" ) == 0 )
//        {
//            MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
//            SetMesh( pMesh );
//
//            // update the panel so new OBJ name shows up.
//            g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pMesh->m_pSourceFile->m_FullPath;
//        }
//    }
//}

void ComponentCollisionObject2D::OnTransformPositionChanged(Vector3& newpos, bool changedbyeditor)
{
    if( changedbyeditor )
        SyncRigidBodyToTransform();
}
#endif //MYFW_USING_WX

//cJSON* ComponentCollisionObject2D::ExportAsJSONObject(bool savesceneid)
//{
//    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid );
//
//    //ExportVariablesToJSON( jComponent ); //_VARIABLE_LIST
//
//    return jComponent;
//}
//
//void ComponentCollisionObject2D::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
//{
//    ComponentBase::ImportFromJSONObject( jsonobj, sceneid );
//
//    //ImportVariablesFromJSON( jsonobj ); //_VARIABLE_LIST
//}

ComponentCollisionObject2D& ComponentCollisionObject2D::operator=(const ComponentCollisionObject2D& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_PrimitiveType = other.m_PrimitiveType;

    m_Static = other.m_Static;
    m_Mass = other.m_Mass;
    //m_Scale = other.m_Scale;
    //m_pMesh

    return *this;
}

void ComponentCollisionObject2D::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCollisionObject2D, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCollisionObject2D, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCollisionObject2D, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCollisionObject2D, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCollisionObject2D, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCollisionObject2D, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCollisionObject2D, OnFileRenamed );
    }
}

void ComponentCollisionObject2D::UnregisterCallbacks()
{
    if( m_CallbacksRegistered == true )
    {
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

//void ComponentCollisionObject2D::SetMesh(MyMesh* pMesh)
//{
//    if( pMesh )
//        pMesh->AddRef();
//
//    SAFE_RELEASE( m_pMesh );
//    m_pMesh = pMesh;
//}

void ComponentCollisionObject2D::OnPlay()
{
    ComponentBase::OnPlay();

    MyAssert( m_pBody == 0 );
    if( m_pBody != 0 )
    {
        g_pBox2DWorld->m_pWorld->DestroyBody( m_pBody );
        m_pBody = 0;
    }

    CreateBody();
}

void ComponentCollisionObject2D::OnStop()
{
    ComponentBase::OnStop();

    // shouldn't get hit, all objects are deleted/recreated when gameplay is stopped.
    if( m_pBody )
    {
        g_pBox2DWorld->m_pWorld->DestroyBody( m_pBody );
        m_pBody = 0;
    }
}

void ComponentCollisionObject2D::CreateBody()
{
    MyAssert( m_pBody == 0 );

    if( m_pBody != 0 )
        return;

    // create a body on start
    if( m_pBody == 0 )
    {
        Vector3 pos = m_pGameObject->m_pComponentTransform->GetPosition();

        b2BodyDef bodydef;
        
        bodydef.position = b2Vec2( pos.x, pos.y );
        if( m_Static )
            bodydef.type = b2_staticBody;
        else
            bodydef.type = b2_dynamicBody;

        m_pBody = g_pBox2DWorld->m_pWorld->CreateBody( &bodydef );

        switch( m_PrimitiveType )
        {
        case Physics2DPrimitiveType_Box:
            {
                b2PolygonShape boxshape;
                boxshape.SetAsBox( 1, 1 );

                b2FixtureDef fixturedef;
                fixturedef.shape = &boxshape;
                fixturedef.density = 1;

                m_pBody->CreateFixture( &fixturedef );
            }
            break;

        case Physics2DPrimitiveType_Circle:
            {
                b2CircleShape circleshape;
                circleshape.m_p.Set( 0, 0 );
                circleshape.m_radius = 1;

                b2FixtureDef fixturedef;
                fixturedef.shape = &circleshape;
                fixturedef.density = 1;

                m_pBody->CreateFixture( &fixturedef );
            }
            break;
        }
    }
}

void ComponentCollisionObject2D::TickCallback(double TimePassed)
{
    //ComponentBase::Tick( TimePassed );

    if( TimePassed == 0 )
        return;

    if( m_pBody == 0 )
        return;

    b2Vec2 pos = m_pBody->GetPosition();
    float32 angle = m_pBody->GetAngle();

    MyMatrix* matLocal = m_pGameObject->m_pComponentTransform->GetLocalTransform();

    matLocal->SetIdentity();
    matLocal->CreateSRT( Vector3( 1 ), Vector3( 0, 0, angle ), Vector3( pos.x, pos.y, 0 ) );

#if MYFW_USING_WX
    m_pGameObject->m_pComponentTransform->UpdatePosAndRotFromLocalMatrix();
#endif
}

void ComponentCollisionObject2D::SyncRigidBodyToTransform()
{
    if( m_pBody == 0 )
        return;

    //btTransform transform;
    ////btVector3 pos(m_pGameObject->m_pComponentTransform->m_Position.x, m_pGameObject->m_pComponentTransform->m_Position.y, m_pGameObject->m_pComponentTransform->m_Position.z );
    ////transform.setIdentity();
    ////transform.setOrigin( pos );
    //MyMatrix localmat = m_pGameObject->m_pComponentTransform->GetLocalRotPosMatrix(); //GetLocalTransform();
    //transform.setFromOpenGLMatrix( &localmat.m11 );

    //m_pBody->getMotionState()->setWorldTransform( transform );
    //m_pBody->setWorldTransform( transform );

    //m_pBody->activate( true );

    //g_pBulletWorld->m_pDynamicsWorld->removeRigidBody( m_pBody );
    //g_pBulletWorld->m_pDynamicsWorld->addRigidBody( m_pBody );
}

void ComponentCollisionObject2D::ApplyForce(Vector3 force, Vector3 relpos)
{
    //btVector3 btforce( force.x, force.y, force.z );
    //btVector3 btrelpos( relpos.x, relpos.y, relpos.z );

    //m_pBody->activate();
    //m_pBody->applyForce( btforce, btrelpos );
}
