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

ComponentCollisionObject2D::ComponentCollisionObject2D()
: ComponentUpdateable()
{
    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Updateable;
    m_Type = ComponentType_CollisionObject;

    m_pBody = 0;

    m_PrimitiveType = Physics2DPrimitiveType_Box;
    m_pMesh = 0;
}

ComponentCollisionObject2D::~ComponentCollisionObject2D()
{
    if( m_pBody )
    {
        g_pBox2DWorld->m_pWorld->DestroyBody( m_pBody );
    }

    SAFE_RELEASE( m_pMesh );
}

void ComponentCollisionObject2D::Reset()
{
    ComponentUpdateable::Reset();

    m_PrimitiveType = Physics2DPrimitiveType_Box;

    m_Mass = 0;
    m_Scale.Set( 1,1,1 );
    SAFE_RELEASE( m_pMesh );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
    m_ControlID_PrimitiveType = -1;

    m_pComponentTransform->RegisterPositionChangedCallback( this, StaticOnTransformPositionChanged );
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
    g_pPanelObjectList->AddObject( this, ComponentCollisionObject2D::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Collision object" );
}

void ComponentCollisionObject2D::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentCollisionObject2D::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Collision Object", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        g_pPanelWatch->AddFloat( "Mass", &m_Mass, 0, 100000 );

        m_ControlID_PrimitiveType = g_pPanelWatch->AddEnum( "Primitive Type", &m_PrimitiveType, Physics2DPrimitive_NumTypes, Physics2DPrimitiveTypeStrings, this, StaticOnValueChanged );

        switch( m_PrimitiveType )
        {
            case Physics2DPrimitiveType_Box:
            {
                g_pPanelWatch->AddVector3( "Scale", &m_Scale, 0, 0 );
            }
            break;

            case Physics2DPrimitiveType_Circle:
            {
                g_pPanelWatch->AddFloat( "Scale", &m_Scale.x, 0, 0 );
            }
            break;

            //case Physics2DPrimitiveType_ConvexHull:
            //{
            //    const char* desc = "no mesh";
            //    if( m_pMesh && m_pMesh->m_pSourceFile )
            //        desc = m_pMesh->m_pSourceFile->m_FullPath;
            //    g_pPanelWatch->AddPointerWithDescription( "Collision Mesh", 0, desc, this, ComponentCollisionObject2D::StaticOnDropOBJ );
            //}
            break;
        }
    }
}

void ComponentCollisionObject2D::OnValueChanged(int controlid, bool finishedchanging)
{
    if( finishedchanging )
    {
        if( controlid == m_ControlID_PrimitiveType )
        {
            // TODO: rethink this, doesn't need refresh if panel isn't visible.
            g_pPanelWatch->m_NeedsRefresh = true;
        }
    }
}

void ComponentCollisionObject2D::OnDropOBJ(int controlid, wxCoord x, wxCoord y)
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        MyAssert( pFile );
        //MyAssert( m_pMesh );

        size_t len = strlen( pFile->m_FullPath );
        const char* filenameext = &pFile->m_FullPath[len-4];

        if( strcmp( filenameext, ".obj" ) == 0 )
        {
            MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
            SetMesh( pMesh );

            // update the panel so new OBJ name shows up.
            g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pMesh->m_pSourceFile->m_FullPath;
        }
    }
}

void ComponentCollisionObject2D::OnTransformPositionChanged(Vector3& newpos, bool changedbyeditor)
{
    if( changedbyeditor )
        SyncRigidBodyToTransform();
}
#endif //MYFW_USING_WX

cJSON* ComponentCollisionObject2D::ExportAsJSONObject(bool savesceneid)
{
    cJSON* component = ComponentUpdateable::ExportAsJSONObject( savesceneid );

    // physics primitive type, stored as string
    const char* primitivetypename = Physics2DPrimitiveTypeStrings[m_PrimitiveType];
    MyAssert( primitivetypename );
    if( primitivetypename )
        cJSON_AddStringToObject( component, "Primitive", primitivetypename );

    cJSON_AddNumberToObject( component, "Mass", m_Mass );
    cJSONExt_AddFloatArrayToObject( component, "Scale", &m_Scale.x, 3 );
    
    // OBJ filename
    if( m_pMesh && m_pMesh->m_pSourceFile )
        cJSON_AddStringToObject( component, "OBJ", m_pMesh->m_pSourceFile->m_FullPath );

    return component;
}

void ComponentCollisionObject2D::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentUpdateable::ImportFromJSONObject( jsonobj, sceneid );

    // physics primitive type, stored as string
    cJSON* typeobj = cJSON_GetObjectItem( jsonobj, "Primitive" );
    //MyAssert( typeobj );
    if( typeobj )
    {
        for( int i=0; i<Physics2DPrimitive_NumTypes; i++ )
        {
            if( strcmp( Physics2DPrimitiveTypeStrings[i], typeobj->valuestring ) == 0 )
                m_PrimitiveType = i;
        }
    }

    cJSONExt_GetFloat( jsonobj, "Mass", &m_Mass );
    cJSONExt_GetFloatArray( jsonobj, "Scale", &m_Scale.x, 3 );

    // get the OBJ filename and load the actual file.
    cJSON* objstringobj = cJSON_GetObjectItem( jsonobj, "OBJ" );
    if( objstringobj )
    {
        MyFileObject* pFile = g_pFileManager->FindFileByName( objstringobj->valuestring );
        if( pFile )
        {
            MyMesh* pMesh = g_pMeshManager->FindMeshBySourceFile( pFile );
            SetMesh( pMesh );
        }
    }
}

ComponentCollisionObject2D& ComponentCollisionObject2D::operator=(const ComponentCollisionObject2D& other)
{
    MyAssert( &other != this );

    ComponentUpdateable::operator=( other );

    m_Mass = other.m_Mass;
    m_PrimitiveType = other.m_PrimitiveType;

    return *this;
}

void ComponentCollisionObject2D::SetMesh(MyMesh* pMesh)
{
    if( pMesh )
        pMesh->AddRef();

    SAFE_RELEASE( m_pMesh );
    m_pMesh = pMesh;
}

void ComponentCollisionObject2D::OnPlay()
{
    ComponentUpdateable::OnPlay();

    // create a body on start
    if( m_pBody == 0 )
    {
        Vector3 pos = m_pComponentTransform->GetPosition();

        b2BodyDef bodydef;
        bodydef.type = b2_dynamicBody;
        bodydef.position = b2Vec2( pos.x, pos.y );

        m_pBody = g_pBox2DWorld->m_pWorld->CreateBody( &bodydef );

        b2PolygonShape boxshape;
        boxshape.SetAsBox( 1, 1 );
  
        b2FixtureDef fixturedef;
        fixturedef.shape = &boxshape;
        fixturedef.density = 1;

        m_pBody->CreateFixture( &fixturedef );
    }
}

void ComponentCollisionObject2D::OnStop()
{
    ComponentUpdateable::OnStop();

    // shouldn't get hit, all objects are deleted/recreated when gameplay is stopped.
    if( m_pBody )
    {
        g_pBox2DWorld->m_pWorld->DestroyBody( m_pBody );
        m_pBody = 0;
    }
}

void ComponentCollisionObject2D::Tick(double TimePassed)
{
    //ComponentUpdateable::Tick( TimePassed );

    if( TimePassed == 0 )
        return;

    if( m_pBody == 0 )
        return;

    b2Vec2 pos = m_pBody->GetPosition();
    float32 angle = m_pBody->GetAngle();

    MyMatrix* matLocal = m_pComponentTransform->GetLocalTransform();

    matLocal->SetIdentity();
    matLocal->CreateSRT( Vector3( 1 ), Vector3( 0, 0, angle ), Vector3( pos.x, pos.y, 0 ) );

#if MYFW_USING_WX
    m_pComponentTransform->UpdatePosAndRotFromLocalMatrix();
#endif
}

void ComponentCollisionObject2D::SyncRigidBodyToTransform()
{
    if( m_pBody == 0 )
        return;

    //btTransform transform;
    ////btVector3 pos(m_pComponentTransform->m_Position.x, m_pComponentTransform->m_Position.y, m_pComponentTransform->m_Position.z );
    ////transform.setIdentity();
    ////transform.setOrigin( pos );
    //MyMatrix localmat = m_pComponentTransform->GetLocalRotPosMatrix(); //GetLocalTransform();
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
