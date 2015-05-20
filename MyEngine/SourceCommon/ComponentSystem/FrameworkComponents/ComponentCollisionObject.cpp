//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"

#if MYFW_USING_WX
bool ComponentCollisionObject::m_PanelWatchBlockVisible = true;
#endif

const char* PhysicsPrimitiveTypeStrings[PhysicsPrimitive_NumTypes] = //ADDING_NEW_PhysicsPrimitiveType
{
    "Cube",
    "Sphere",
    "Static Plane",
    "Convex Hull",
};

ComponentCollisionObject::ComponentCollisionObject()
: ComponentUpdateable()
{
    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Updateable;
    m_Type = ComponentType_CollisionObject;

    m_pBody = 0;

    m_PrimitiveType = PhysicsPrimitiveType_Cube;
    m_pMesh = 0;
}

ComponentCollisionObject::~ComponentCollisionObject()
{
    if( m_pBody )
    {
        g_pBulletWorld->m_pDynamicsWorld->removeRigidBody( m_pBody );
        delete m_pBody;
    }

    SAFE_RELEASE( m_pMesh );
}

void ComponentCollisionObject::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentCollisionObject>( "ComponentCollisionObject" )
            .addData( "mass", &ComponentCollisionObject::m_Mass )            
            .addFunction( "ApplyForce", &ComponentCollisionObject::ApplyForce )
        .endClass();
}

void ComponentCollisionObject::Reset()
{
    ComponentUpdateable::Reset();

    m_PrimitiveType = PhysicsPrimitiveType_Cube;

    m_Mass = 0;
    m_Scale.Set( 1,1,1 );
    SAFE_RELEASE( m_pMesh );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
    m_ControlID_PrimitiveType = -1;

    m_pComponentTransform->RegisterPositionChangedCallback( this, StaticOnTransformPositionChanged );
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void ComponentCollisionObject::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentCollisionObject::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Collision object" );
}

void ComponentCollisionObject::OnLeftClick(bool clear)
{
    ComponentBase::OnLeftClick( clear );
}

void ComponentCollisionObject::FillPropertiesWindow(bool clear)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Collision Object", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentBase::FillPropertiesWindow( clear );

        g_pPanelWatch->AddFloat( "Mass", &m_Mass, 0, 100 );

        m_ControlID_PrimitiveType = g_pPanelWatch->AddEnum( "Primitive Type", &m_PrimitiveType, PhysicsPrimitive_NumTypes, PhysicsPrimitiveTypeStrings, this, StaticOnValueChanged );

        switch( m_PrimitiveType )
        {
            case PhysicsPrimitiveType_Cube:
            {
                g_pPanelWatch->AddVector3( "Scale", &m_Scale, 0, 100 );
            }
            break;

            case PhysicsPrimitiveType_Sphere:
            {
                g_pPanelWatch->AddFloat( "Scale", &m_Scale.x, 0, 100 );
            }
            break;

            case PhysicsPrimitiveType_StaticPlane:
            {
                // nothing here, maybe add an pos/rot offset, but ATM uses object transform.
            }
            break;

            case PhysicsPrimitiveType_ConvexHull:
            {
                const char* desc = "no mesh";
                if( m_pMesh && m_pMesh->m_pSourceFile )
                    desc = m_pMesh->m_pSourceFile->m_FullPath;
                g_pPanelWatch->AddPointerWithDescription( "Collision Mesh", 0, desc, this, ComponentCollisionObject::StaticOnDropOBJ );
            }
            break;
        }
    }
}

void ComponentCollisionObject::OnValueChanged(int controlid, bool finishedchanging)
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

void ComponentCollisionObject::OnDropOBJ(int controlid, wxCoord x, wxCoord y)
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

void ComponentCollisionObject::OnTransformPositionChanged(Vector3& newpos, bool changedbyeditor)
{
    if( changedbyeditor )
        SyncRigidBodyToTransform();
}
#endif //MYFW_USING_WX

cJSON* ComponentCollisionObject::ExportAsJSONObject()
{
    cJSON* component = ComponentUpdateable::ExportAsJSONObject();

    // physics primitive type, stored as string
    const char* primitivetypename = PhysicsPrimitiveTypeStrings[m_PrimitiveType];
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

void ComponentCollisionObject::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentUpdateable::ImportFromJSONObject( jsonobj, sceneid );

    // physics primitive type, stored as string
    cJSON* typeobj = cJSON_GetObjectItem( jsonobj, "Primitive" );
    //MyAssert( typeobj );
    if( typeobj )
    {
        for( int i=0; i<PhysicsPrimitive_NumTypes; i++ )
        {
            if( strcmp( PhysicsPrimitiveTypeStrings[i], typeobj->valuestring ) == 0 )
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

ComponentCollisionObject& ComponentCollisionObject::operator=(const ComponentCollisionObject& other)
{
    MyAssert( &other != this );

    ComponentUpdateable::operator=( other );

    m_Mass = other.m_Mass;
    m_PrimitiveType = other.m_PrimitiveType;

    return *this;
}

void ComponentCollisionObject::SetMesh(MyMesh* pMesh)
{
    if( pMesh )
        pMesh->AddRef();

    SAFE_RELEASE( m_pMesh );
    m_pMesh = pMesh;
}

void ComponentCollisionObject::OnPlay()
{
    ComponentUpdateable::OnPlay();

    //// set the collision object scale on play, guess this should be set whenever the object is scaled.
    ////   TODO: find a better way to handle the object being scaled in editor.
    //Vector3 localscale = m_pComponentTransform->GetScale();
    //btVector3 scale( localscale.x, localscale.y, localscale.z );
    //m_pBody->getCollisionShape()->setLocalScaling( scale );

    // create a rigidbody on start
    if( m_pBody == 0 )
    {
        btCollisionShape* colShape;
        
        if( m_pMesh && m_pMesh->m_MeshReady )
        {
            //btStridingMeshInterface meshinterface;
            //colShape = new btBvhTriangleMeshShape( meshinterface );

            // TODO: fix this, it assumes position at the start of the vertex format.
            unsigned int stride = m_pMesh->GetStride( 0 );
            colShape = new btConvexHullShape( (btScalar*)m_pMesh->GetVerts( false ), m_pMesh->GetNumVerts(), stride );
            ((btConvexHullShape*)colShape)->setMargin( 0.2f );

	        btShapeHull* hull = new btShapeHull( (btConvexShape*)colShape );
	        btScalar margin = colShape->getMargin();
	        hull->buildHull(margin);
            delete colShape;

	        colShape = new btConvexHullShape( (btScalar*)hull->getVertexPointer(), hull->numVertices() );
            delete hull;
        }
        else
        {
            if( m_PrimitiveType == PhysicsPrimitiveType_Cube )
            {
                colShape = new btBoxShape( btVector3(m_Scale.x/2, m_Scale.y/2, m_Scale.z/2) ); // half-extents
            }
            else if( m_PrimitiveType == PhysicsPrimitiveType_Sphere )
            {
                colShape = new btSphereShape( btScalar(m_Scale.x) );
            }
            else if( m_PrimitiveType == PhysicsPrimitiveType_StaticPlane )
            {
                // plane is flat on x/z and at origin, rigid will have transform to place plane in space.
                colShape = new btStaticPlaneShape( btVector3(0, 1, 0), 0 );
            }
        }

        g_pBulletWorld->m_CollisionShapes.push_back( colShape );

        // Create Dynamic Objects
        btTransform startTransform;
        startTransform.setIdentity();

        //btScalar mass( 1.0f );

        // rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (m_Mass != 0.0f);

        btVector3 localInertia( 0,0,0 );
        if( isDynamic )
            colShape->calculateLocalInertia( m_Mass, localInertia );

        //btVector3 pos(m_pComponentTransform->m_Position.x, m_pComponentTransform->m_Position.y, m_pComponentTransform->m_Position.z );
        //startTransform.setOrigin( pos );
        Vector3 localscale = m_pComponentTransform->GetLocalScale();
        btVector3 scale( localscale.x, localscale.y, localscale.z );
        colShape->setLocalScaling( scale );

        MyMatrix localmat = m_pComponentTransform->GetLocalRotPosMatrix(); //GetLocalTransform();
        startTransform.setFromOpenGLMatrix( &localmat.m11 );

        // using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState( startTransform );
        btRigidBody::btRigidBodyConstructionInfo rbInfo( m_Mass, myMotionState, colShape, localInertia );
        m_pBody = new btRigidBody( rbInfo );
        
        g_pBulletWorld->m_pDynamicsWorld->addRigidBody( m_pBody );
    }
}

void ComponentCollisionObject::OnStop()
{
    ComponentUpdateable::OnStop();

    // shouldn't get hit, all objects are deleted/recreated when gameplay is stopped.
    if( m_pBody )
    {
        g_pBulletWorld->m_pDynamicsWorld->removeRigidBody( m_pBody );
        SAFE_DELETE( m_pBody );
    }
}

void ComponentCollisionObject::Tick(double TimePassed)
{
    //ComponentUpdateable::Tick( TimePassed );

    if( TimePassed == 0 )
    {
        return;
    }

    if( m_pBody == 0 )
        return;

    MyMatrix* matLocal = m_pComponentTransform->GetLocalTransform();

    btTransform transform;
    m_pBody->getMotionState()->getWorldTransform( transform );
    MyMatrix matRotPos;
    MyMatrix matBulletGL;
    transform.getOpenGLMatrix( &matBulletGL.m11 );

    *matLocal = matBulletGL;

#if MYFW_USING_WX
    m_pComponentTransform->UpdatePosAndRotFromLocalMatrix();
#endif

    // if the collisionshape is scaled, scale our object to match.
    btVector3 scale = m_pBody->getCollisionShape()->getLocalScaling();
    if( scale.x() != 1 || scale.y() != 1 || scale.z() != 1 )
    {
        MyMatrix matScale;
        matScale.CreateScale( scale.x(), scale.y(), scale.z() );
        *matLocal = *matLocal * matScale;
    }

    //btVector3 pos = transform.getOrigin();
    //btQuaternion rot = transform.getRotation();
    //m_pComponentTransform->SetPosition( Vector3( pos.getX(), pos.getY(), pos.getZ() ) );
    //m_pComponentTransform->SetRotation( Vector3( rot.g, pos.getY(), pos.getZ() ) );
}

void ComponentCollisionObject::SyncRigidBodyToTransform()
{
    if( m_pBody == 0 )
        return;

    btTransform transform;
    //btVector3 pos(m_pComponentTransform->m_Position.x, m_pComponentTransform->m_Position.y, m_pComponentTransform->m_Position.z );
    //transform.setIdentity();
    //transform.setOrigin( pos );
    MyMatrix localmat = m_pComponentTransform->GetLocalRotPosMatrix(); //GetLocalTransform();
    transform.setFromOpenGLMatrix( &localmat.m11 );

    m_pBody->getMotionState()->setWorldTransform( transform );
    m_pBody->setWorldTransform( transform );

    m_pBody->activate( true );

    g_pBulletWorld->m_pDynamicsWorld->removeRigidBody( m_pBody );
    g_pBulletWorld->m_pDynamicsWorld->addRigidBody( m_pBody );
}

void ComponentCollisionObject::ApplyForce(Vector3 force, Vector3 relpos)
{
    btVector3 btforce( force.x, force.y, force.z );
    btVector3 btrelpos( relpos.x, relpos.y, relpos.z );

    m_pBody->activate();
    m_pBody->applyForce( btforce, btrelpos );
}
