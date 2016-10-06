//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "TransformGizmo.h"

TransformGizmo::TransformGizmo()
{
    m_VisibleIfObjectsSelected = false;
    m_pSelectedPart = 0;

    for( int i=0; i<3; i++ )
    {
        m_pTranslate1Axis[i] = 0;
        m_pTranslate2Axis[i] = 0;
        m_pScale1Axis[i] = 0;

        m_pMaterial_Translate1Axis[i] = 0;
        m_pMaterial_Translate2Axis[i] = 0;
        m_pMaterial_Scale1Axis[i] = 0;
    }

    m_pScale3Axis = 0;
    m_pMaterial_Scale3Axis = 0;

    m_LastIntersectResultIsValid = false;
    m_LastIntersectResultUsed.Set(0,0,0);
}

TransformGizmo::~TransformGizmo()
{
    for( int i=0; i<3; i++ )
    {
        SAFE_DELETE( m_pTranslate1Axis[i] );
        SAFE_DELETE( m_pTranslate2Axis[i] );
        SAFE_DELETE( m_pScale1Axis[i] );

        SAFE_RELEASE( m_pMaterial_Translate1Axis[i] );
        SAFE_RELEASE( m_pMaterial_Translate2Axis[i] );
        SAFE_RELEASE( m_pMaterial_Scale1Axis[i] );
    }

    SAFE_DELETE( m_pScale3Axis );
    SAFE_RELEASE( m_pMaterial_Scale3Axis );
}

void TransformGizmo::Tick(double TimePassed, EditorState* pEditorState)
{
    // Find the center of the object.
    bool GizmoVisible = false;
    Vector3 ObjectPosition;
    MyMatrix ObjectTransform;

    if( pEditorState->m_pSelectedObjects.size() == 1 )
    {
        GizmoVisible = true;
        ObjectPosition = pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->GetWorldPosition();
        ObjectTransform = *pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->GetWorldTransform();
    }
    else if( pEditorState->m_pSelectedObjects.size() > 1 )
    {
        GizmoVisible = true;

        // find the center point between all selected objects.
        ObjectPosition.Set( 0, 0, 0 );
        for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
        {
            ObjectPosition += pEditorState->m_pSelectedObjects[i]->m_pComponentTransform->GetLocalPosition();
        }
        ObjectPosition /= (float)pEditorState->m_pSelectedObjects.size();

        ObjectTransform.SetIdentity();
    }
    else
    {
        GizmoVisible = false;
    }

    if( m_VisibleIfObjectsSelected == false )
        GizmoVisible = false;

    // Update 1 axis transform gizmos
    for( int i=0; i<3; i++ )
    {
        MyAssert( m_pTranslate1Axis[i] );

        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pTranslate1Axis[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );

        ComponentMesh* pMesh = dynamic_cast<ComponentMesh*>( pRenderable );
        MyAssert( pMesh );
        if( pMesh )
        {
            MaterialDefinition* pMaterial = pMesh->GetMaterial( 0 );
            if( pMaterial )
            {
                if( i == 0 )
                    pMaterial->m_ColorDiffuse.Set( 255, 100, 100, 255 );
                if( i == 1 )
                    pMaterial->m_ColorDiffuse.Set( 100, 255, 100, 255 );
                if( i == 2 )
                    pMaterial->m_ColorDiffuse.Set( 100, 100, 255, 255 );

                if( m_pTranslate1Axis[i] == m_pSelectedPart )
                {
                    pMaterial->m_ColorDiffuse.Set( 255,255,255,255 );
                }
            }
        }

        pRenderable->SetVisible( GizmoVisible );

        if( GizmoVisible )
        {
            // move the gizmo to the object position.
            m_pTranslate1Axis[i]->m_pComponentTransform->SetLocalPosition( ObjectPosition );

            // rotate the gizmo.
            MyMatrix matrot;
            matrot.SetIdentity();
            if( i == 0 )
                matrot.Rotate( 90, 0, 0, 1 );
            if( i == 1 )
                matrot.Rotate( 0, 1, 0, 0 );
            if( i == 2 )
                matrot.Rotate( -90, 1, 0, 0 );

            MyMatrix matrotobj;
            matrotobj.SetIdentity();
            matrotobj.CreateSRT( Vector3(1,1,1), ObjectTransform.GetEulerAngles(), Vector3(0,0,0) );

            matrot = matrotobj * matrot;

            Vector3 rot = matrot.GetEulerAngles() * 180.0f/PI;

            m_pTranslate1Axis[i]->m_pComponentTransform->SetLocalRotation( rot );

            float distance = (pEditorState->m_pEditorCamera->m_pComponentTransform->GetLocalPosition() - ObjectPosition).Length();
            m_pTranslate1Axis[i]->m_pComponentTransform->SetLocalScale( Vector3( distance / 15.0f ) );
        }
    }

    // Update 2 axis transform gizmos
    for( int i=0; i<3; i++ )
    {
        MyAssert( m_pTranslate2Axis[i] );

        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pTranslate2Axis[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );

        ComponentMesh* pMesh = dynamic_cast<ComponentMesh*>( pRenderable );
        MyAssert( pMesh );
        if( pMesh )
        {
            MaterialDefinition* pMaterial = pMesh->GetMaterial( 0 );
            if( pMaterial )
            {
                if( i == 0 )
                    pMaterial->m_ColorDiffuse.Set( 100, 255, 255, 180 );
                if( i == 1 )
                    pMaterial->m_ColorDiffuse.Set( 255, 100, 255, 180 );
                if( i == 2 )
                    pMaterial->m_ColorDiffuse.Set( 255, 255, 100, 180 );

                if( m_pTranslate2Axis[i] == m_pSelectedPart )
                {
                    pMaterial->m_ColorDiffuse.Set( 255, 255, 255, 255 );
                }
            }
        }

        pRenderable->SetVisible( GizmoVisible );

        if( GizmoVisible )
        {
            ComponentCamera* pCamera = pEditorState->GetEditorCamera();
            Vector3 campos = pCamera->m_pGameObject->GetTransform()->GetLocalPosition();

            Vector3 pos = ObjectPosition;

            // rotate the gizmo.
            MyMatrix matrot;
            matrot.SetIdentity();
            if( i == 0 ) // yz
            {
                if( campos.x  > ObjectPosition.x ) { matrot.Rotate( -90, 0, 0, 1 ); matrot.Rotate( 180, 0, 1, 0 ); }
                if( campos.x <= ObjectPosition.x ) { matrot.Rotate( +90, 1, 0, 0 ); matrot.Rotate( -90, 0, 1, 0 ); }
                //if( campos.y  < ObjectPosition.y ) pos.y -= 2;
            }
            if( i == 1 ) // xz
            {
                if( campos.y  > ObjectPosition.y ) { matrot.Rotate( -90, 0, 1, 0 ); }
                if( campos.y <= ObjectPosition.y ) { matrot.Rotate( 180, 1, 0, 0 ); }
            }
            if( i == 2 ) // xy
            {
                if( campos.z  > ObjectPosition.z ) { matrot.Rotate( -90, 1, 0, 0 ); matrot.Rotate( -90, 0, 0, 1 ); }
                if( campos.z <= ObjectPosition.z ) { matrot.Rotate( 90, 1, 0, 0 ); }
            }

            MyMatrix matrotobj;
            matrotobj.SetIdentity();
            matrotobj.CreateSRT( Vector3(1,1,1), ObjectTransform.GetEulerAngles(), Vector3(0,0,0) );

            matrot = matrotobj * matrot;

            Vector3 rot = matrot.GetEulerAngles() * 180.0f/PI;

            // move the gizmo to the object position.
            m_pTranslate2Axis[i]->m_pComponentTransform->SetLocalPosition( pos );

            m_pTranslate2Axis[i]->m_pComponentTransform->SetLocalRotation( rot );

            float distance = (pEditorState->m_pEditorCamera->m_pComponentTransform->GetLocalPosition() - ObjectPosition).Length();
            m_pTranslate2Axis[i]->m_pComponentTransform->SetLocalScale( Vector3( distance / 15.0f ) );
        }
    }

    // Update 1 axis scale gizmos
    for( int i=0; i<3; i++ )
    {
        MyAssert( m_pScale1Axis[i] );

        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pScale1Axis[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );

        ComponentMesh* pMesh = dynamic_cast<ComponentMesh*>( pRenderable );
        MyAssert( pMesh );
        if( pMesh )
        {
            MaterialDefinition* pMaterial = pMesh->GetMaterial( 0 );
            if( pMaterial )
            {
                if( i == 0 )
                    pMaterial->m_ColorDiffuse.Set( 255, 100, 100, 255 );
                if( i == 1 )
                    pMaterial->m_ColorDiffuse.Set( 100, 255, 100, 255 );
                if( i == 2 )
                    pMaterial->m_ColorDiffuse.Set( 100, 100, 255, 255 );

                if( m_pScale1Axis[i] == m_pSelectedPart )
                    pMaterial->m_ColorDiffuse.Set( 255, 255, 255, 255 );
            }
        }

        pRenderable->SetVisible( GizmoVisible );

        if( GizmoVisible )
        {
            // move the gizmo to the object position.
            m_pScale1Axis[i]->m_pComponentTransform->SetLocalPosition( ObjectPosition );

            // rotate the gizmo.
            MyMatrix matrot;
            matrot.SetIdentity();

            MyMatrix matrotobj;
            matrotobj.SetIdentity();
            matrotobj.CreateSRT( Vector3(1,1,1), ObjectTransform.GetEulerAngles(), Vector3(0,0,0) );

            matrot = matrotobj * matrot;

            Vector3 rot = matrot.GetEulerAngles() * 180.0f/PI;

            m_pScale1Axis[i]->m_pComponentTransform->SetLocalRotation( rot );

            float distance = (pEditorState->m_pEditorCamera->m_pComponentTransform->GetLocalPosition() - ObjectPosition).Length();
            m_pScale1Axis[i]->m_pComponentTransform->SetLocalScale( Vector3( distance / 15.0f ) );
        }
    }

    // Update 3 axis scale gizmo
    {
        MyAssert( m_pScale3Axis );

        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pScale3Axis->GetFirstComponentOfBaseType( BaseComponentType_Renderable );

        ComponentMesh* pMesh = dynamic_cast<ComponentMesh*>( pRenderable );
        MyAssert( pMesh );
        if( pMesh )
        {
            MaterialDefinition* pMaterial = pMesh->GetMaterial( 0 );
            if( pMaterial )
            {
                pMaterial->m_ColorDiffuse.Set( 100, 100, 100, 180 );

                if( m_pScale3Axis == m_pSelectedPart )
                {
                    pMaterial->m_ColorDiffuse.Set( 255, 255, 255, 180 );
                }
            }
        }

        pRenderable->SetVisible( GizmoVisible );

        if( GizmoVisible )
        {
            ComponentCamera* pCamera = pEditorState->GetEditorCamera();
            Vector3 campos = pCamera->m_pGameObject->GetTransform()->GetLocalPosition();

            Vector3 pos = ObjectPosition;

            // rotate the gizmo.
            MyMatrix matrot;
            matrot.SetIdentity();

            MyMatrix matrotobj;
            matrotobj.SetIdentity();
            matrotobj.CreateSRT( Vector3(1,1,1), ObjectTransform.GetEulerAngles(), Vector3(0,0,0) );

            matrot = matrotobj * matrot;

            Vector3 rot = matrot.GetEulerAngles() * 180.0f/PI;

            // move the gizmo to the object position.
            m_pScale3Axis->m_pComponentTransform->SetLocalPosition( pos );

            m_pScale3Axis->m_pComponentTransform->SetLocalRotation( rot );

            float distance = (pEditorState->m_pEditorCamera->m_pComponentTransform->GetLocalPosition() - ObjectPosition).Length();
            m_pScale3Axis->m_pComponentTransform->SetLocalScale( Vector3( distance / 15.0f ) );
        }
    }
}

bool TransformGizmo::HandleInput(EngineCore* pGame, int keydown, int keycode, int action, int id, float x, float y, float pressure)
{
    ComponentRenderable* pRenderable = (ComponentRenderable*)m_pTranslate1Axis[0]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
    ComponentMesh* pMesh = dynamic_cast<ComponentMesh*>( pRenderable );

    if( pMesh->IsVisible() == false )
        return false;

    //MyAssert( x >= 0 && y >= 0 );
    if( x < 0 || y < 0 )
        return false;

    // find the object we're hovering on
    m_pSelectedPart = pGame->GetCurrentEditorInterface()->GetObjectAtPixel( (unsigned int)x, (unsigned int)y, true );

    return false;
}

void TransformGizmo::CreateAxisObjects(unsigned int sceneid, float scale, EditorState* pEditorState)
{
    GameObject* pGameObject;
    ComponentMesh* pComponentMesh;

    m_pMaterial_Translate1Axis[0] = MyNew MaterialDefinition( g_pEngineCore->m_pShader_TintColor, ColorByte(255,0,0,255) );
    m_pMaterial_Translate1Axis[1] = MyNew MaterialDefinition( g_pEngineCore->m_pShader_TintColor, ColorByte(0,255,0,255) );
    m_pMaterial_Translate1Axis[2] = MyNew MaterialDefinition( g_pEngineCore->m_pShader_TintColor, ColorByte(0,0,255,255) );

    // Create single axis translators.
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - x-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Translate1Axis[0], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.05f, ColorByte(255, 100, 100, 255) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
            pComponentMesh->AddToSceneGraph();
        }

        pEditorState->m_pTransformGizmo->m_pTranslate1Axis[0] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - y-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Translate1Axis[1], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.05f, ColorByte(100, 255, 100, 255) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
            pComponentMesh->AddToSceneGraph();
        }

        pEditorState->m_pTransformGizmo->m_pTranslate1Axis[1] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - z-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Translate1Axis[2], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.05f, ColorByte(100, 100, 255, 255) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
            pComponentMesh->AddToSceneGraph();
        }

        pEditorState->m_pTransformGizmo->m_pTranslate1Axis[2] = pGameObject;
    }

    m_pMaterial_Translate2Axis[0] = MyNew MaterialDefinition( g_pEngineCore->m_pShader_TintColor, ColorByte(255,0,0,100) );
    m_pMaterial_Translate2Axis[1] = MyNew MaterialDefinition( g_pEngineCore->m_pShader_TintColor, ColorByte(0,255,0,100) );
    m_pMaterial_Translate2Axis[2] = MyNew MaterialDefinition( g_pEngineCore->m_pShader_TintColor, ColorByte(0,0,255,100) );

    // Create 2 axis translators
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - yz-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Translate2Axis[0], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreatePlane( Vector3(0,0,0), Vector2(1,1), Vector2Int(2,2), Vector2(0,0), Vector2(1,1) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
            pComponentMesh->AddToSceneGraph();
        }

        pEditorState->m_pTransformGizmo->m_pTranslate2Axis[0] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - xz-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Translate2Axis[1], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreatePlane( Vector3(0,0,0), Vector2(1,1), Vector2Int(2,2), Vector2(0,0), Vector2(1,1) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
            pComponentMesh->AddToSceneGraph();
        }

        pEditorState->m_pTransformGizmo->m_pTranslate2Axis[1] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - xy-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Translate2Axis[2], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreatePlane( Vector3(0,0,0), Vector2(1,1), Vector2Int(2,2), Vector2(0,0), Vector2(1,1) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
            pComponentMesh->AddToSceneGraph();
        }

        pEditorState->m_pTransformGizmo->m_pTranslate2Axis[2] = pGameObject;
    }

    m_pMaterial_Scale1Axis[0] = MyNew MaterialDefinition( g_pEngineCore->m_pShader_TintColor, ColorByte(255,0,0,255) );
    m_pMaterial_Scale1Axis[1] = MyNew MaterialDefinition( g_pEngineCore->m_pShader_TintColor, ColorByte(0,255,0,255) );
    m_pMaterial_Scale1Axis[2] = MyNew MaterialDefinition( g_pEngineCore->m_pShader_TintColor, ColorByte(0,0,255,255) );

    // Create single axis scalers.
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - x-scale" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Scale1Axis[0], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateBox( 0.5f, 0.5f, 0.5f, 0, 1, 0, 1, Justify_Center, Vector3( 3, 0, 0 ) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
            pComponentMesh->AddToSceneGraph();
        }

        pEditorState->m_pTransformGizmo->m_pScale1Axis[0] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - y-scale" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Scale1Axis[1], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateBox( 0.5f, 0.5f, 0.5f, 0, 1, 0, 1, Justify_Center, Vector3( 0, 3, 0 ) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
            pComponentMesh->AddToSceneGraph();
        }

        pEditorState->m_pTransformGizmo->m_pScale1Axis[1] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - z-scale" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Scale1Axis[2], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateBox( 0.5f, 0.5f, 0.5f, 0, 1, 0, 1, Justify_Center, Vector3( 0, 0, 3 ) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
            pComponentMesh->AddToSceneGraph();
        }

        pEditorState->m_pTransformGizmo->m_pScale1Axis[2] = pGameObject;
    }

    m_pMaterial_Scale3Axis = MyNew MaterialDefinition( g_pEngineCore->m_pShader_TintColor, ColorByte(255,0,0,100) );

    // Create 3 axis scaler
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - xyz-scale" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Scale3Axis, 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateBox( 0.5f, 0.5f, 0.5f, 0, 1, 0, 1, Justify_Center, Vector3(0,0,0) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
            pComponentMesh->AddToSceneGraph();
        }

        pEditorState->m_pTransformGizmo->m_pScale3Axis = pGameObject;
    }
}

void TransformGizmo::ScaleGizmosForMousePickRendering(bool doscale)
{
    float scaleamount = 7;

    if( doscale == false )
        scaleamount = 1/scaleamount;

    for( int i=0; i<3; i++ )
    {
        Vector3 currentscale = m_pTranslate1Axis[i]->m_pComponentTransform->GetLocalScale();
        Vector3 newscale( currentscale.x * scaleamount, currentscale.y, currentscale.z * scaleamount );
        m_pTranslate1Axis[i]->m_pComponentTransform->SetLocalScale( newscale );

        // update the world matrix, so the scale will apply when the scene graph renders the scene.
        m_pTranslate1Axis[i]->m_pComponentTransform->UpdateTransform();
    }
}

void TransformGizmo::TranslateSelectedObjects(EngineCore* pGame, EditorState* pEditorState)
{
    if( pEditorState->m_pSelectedObjects.size() == 0 )
        return;

    // move the selected objects along a plane or axis
    if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateX ||
        pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateY ||
        pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateZ ||
        pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateXY ||
        pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateXZ ||
        pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateYZ )
    {
        // move all selected objects by the same amount, use object 0 to create a plane.
        {
            //pEditorState->m_EditorActionState = EDITORACTIONSTATE_TranslateYZ;

            MyMatrix* pObjectTransform = pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->GetLocalTransform();

            // create a plane based on the axis we want.
            Vector3 axisvector;
            Plane plane;
            {
                ComponentCamera* pCamera = pEditorState->GetEditorCamera();
                Vector3 camInvAt = pCamera->m_pGameObject->GetTransform()->GetLocalTransform()->GetAt() * -1;

                Vector3 normal;
                if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateX )
                {
                    camInvAt.x = 0;
                    normal = camInvAt; // set plane normal to face the camera.
                    axisvector = Vector3(1,0,0);
                }
                else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateY )
                {
                    camInvAt.y = 0;
                    normal = camInvAt; // set plane normal to face the camera.
                    axisvector = Vector3(0,1,0);
                }
                else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateZ )
                {
                    camInvAt.z = 0;
                    normal = camInvAt; // set plane normal to face the camera.
                    axisvector = Vector3(0,0,1);
                }
                else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateXY )
                {
                    normal = Vector3(0,0,1);
                    axisvector = Vector3(1,0,0);
                }
                else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateXZ )
                {
                    normal = Vector3(0,1,0);
                    axisvector = Vector3(0,0,1);
                }
                else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateYZ )
                {
                    normal = Vector3(1,0,0);
                    axisvector = Vector3(0,1,0);
                }

                // TODO: support local space translation.
                if( 1 ) // if( world space translation )
                {
                    // create a world space plane.
                    plane.Set( normal, pObjectTransform->GetTranslation() );
                }
//                else
//                {
//                    // TODO: support this.
//                    // transform the normal into the selected objects space.
//                    plane.Set( (*pObjectTransform * Vector4( normal, 0 )).XYZ(), pObjectTransform->GetTranslation() );
//                }
            }

            // Get the mouse click ray... current and last frame.
            Vector3 currentraystart, currentrayend;
            pGame->GetMouseRay( pEditorState->m_CurrentMousePosition, &currentraystart, &currentrayend );

            Vector3 lastraystart, lastrayend;
            pGame->GetMouseRay( pEditorState->m_LastMousePosition, &lastraystart, &lastrayend );

            //LOGInfo( LOGTag, "current->(%0.0f,%0.0f) (%0.2f,%0.2f,%0.2f) (%0.2f,%0.2f,%0.2f)\n",
            //        pEditorState->m_CurrentMousePosition.x,
            //        pEditorState->m_CurrentMousePosition.y,
            //        currentraystart.x,
            //        currentraystart.y,
            //        currentraystart.z,
            //        currentrayend.x,
            //        currentrayend.y,
            //        currentrayend.z
            //    );

            // find the intersection point of the plane.
            Vector3 currentresult;
            Vector3 lastresult;
            if( plane.IntersectRay( currentraystart, currentrayend, &currentresult ) &&
                plane.IntersectRay( lastraystart, lastrayend, &lastresult ) )
            {
                //LOGInfo( LOGTag, "currentresult( %f, %f, %f );\n", currentresult.x, currentresult.y, currentresult.z );
                //LOGInfo( LOGTag, "lastresult( %f, %f, %f );", lastresult.x, lastresult.y, lastresult.z );
                //LOGInfo( LOGTag, "axisvector( %f, %f, %f );\n", axisvector.x, axisvector.y, axisvector.z );

                // TODO: support local space translation.
                // lock to one of the 3 axis.
                if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateX )
                {
                    currentresult.y = currentresult.z = 0;
                    lastresult.y = lastresult.z = 0;
                }
                if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateY )
                {
                    currentresult.x = currentresult.z = 0;
                    lastresult.x = lastresult.z = 0;
                }
                if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateZ )
                {
                    currentresult.x = currentresult.y = 0;
                    lastresult.x = lastresult.y = 0;
                }

                // find the diff pos between this frame and last.
                Vector3 diff = currentresult - lastresult;

                // if snapping to grid is enabled, then use m_LastIntersectResultUsed instead of last frames result.
                if( g_pEngineMainFrame->m_GridSettings.snapenabled )
                {
                    // snap object 0 to grid, all other will stay relative.
                    Vector3 pos = pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->GetLocalPosition();

                    if( m_LastIntersectResultIsValid == false )
                    {
                        m_LastIntersectResultUsed = lastresult;
                        m_LastIntersectResultIsValid = true;
                    }

                    diff = currentresult - m_LastIntersectResultUsed;

                    Vector3 finalpos = pos + diff/2;
                    Vector3 newfinalpos;
                    newfinalpos.x = MyRoundToMultipleOf( finalpos.x, g_pEngineMainFrame->m_GridSettings.stepsize.x );
                    newfinalpos.y = MyRoundToMultipleOf( finalpos.y, g_pEngineMainFrame->m_GridSettings.stepsize.y );
                    newfinalpos.z = MyRoundToMultipleOf( finalpos.z, g_pEngineMainFrame->m_GridSettings.stepsize.z );

                    diff = newfinalpos - pos;

                    if( diff.x != 0 )
                        m_LastIntersectResultUsed.x = currentresult.x;
                    if( diff.y != 0 )
                        m_LastIntersectResultUsed.y = currentresult.y;
                    if( diff.z != 0 )
                        m_LastIntersectResultUsed.z = currentresult.z;
                }

                // GIZMOTRANSLATE: move all of the things. // undo is handled by EngineCore.cpp when mouse is lifted.
                pEditorState->m_DistanceTranslated += diff;
                //LOGInfo( LOGTag, "pEditorState->m_DistanceTranslated.Set( %f, %f, %f );", pEditorState->m_DistanceTranslated.x, pEditorState->m_DistanceTranslated.y, pEditorState->m_DistanceTranslated.z );
                //LOGInfo( LOGTag, "diff( %f, %f, %f, %d );", diff.x, diff.y, diff.z, pEditorState->m_pSelectedObjects.size() );

                TranslateSelectedObjects( pEditorState, diff );
            }
        }
    }
}

void TransformGizmo::TranslateSelectedObjects(EditorState* pEditorState, Vector3 distance)
{
    for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
    {
        ComponentTransform* pTransform = pEditorState->m_pSelectedObjects[i]->m_pComponentTransform;

        // if this object has a selected parent, don't move it, only move the parent.
        if( pTransform->IsAnyParentInList( pEditorState->m_pSelectedObjects ) == false )
        {
            Vector3 pos = pTransform->GetLocalTransform()->GetTranslation();

            pTransform->SetPositionByEditor( pos + distance );
            pTransform->UpdateTransform();
        }
    }
}

void TransformGizmo::CancelLastTranslation(EditorState* pEditorState)
{
    TranslateSelectedObjects( pEditorState, pEditorState->m_DistanceTranslated * -1 );
}

void TransformGizmo::ScaleSelectedObjects(EngineCore* pGame, EditorState* pEditorState)
{
    if( pEditorState->m_pSelectedObjects.size() == 0 )
        return;

    // move the selected objects along a plane or axis
    if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_ScaleX ||
        pEditorState->m_EditorActionState == EDITORACTIONSTATE_ScaleY ||
        pEditorState->m_EditorActionState == EDITORACTIONSTATE_ScaleZ ||
        pEditorState->m_EditorActionState == EDITORACTIONSTATE_ScaleXYZ )
    {
        // move all selected objects by the same amount, use object 0 to create a plane.
        {
            MyMatrix* pObjectTransform = pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->GetLocalTransform();

            // create a plane based on the axis we want.
            Vector3 axisvector;
            Plane plane;
            {
                ComponentCamera* pCamera = pEditorState->GetEditorCamera();
                Vector3 camInvAt = pCamera->m_pGameObject->GetTransform()->GetLocalTransform()->GetAt() * -1;

                Vector3 normal;
                if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_ScaleX )
                {
                    camInvAt.x = 0;
                    normal = camInvAt; // set plane normal to face the camera.
                    axisvector = Vector3(1,0,0);
                }
                else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_ScaleY )
                {
                    camInvAt.y = 0;
                    normal = camInvAt; // set plane normal to face the camera.
                    axisvector = Vector3(0,1,0);
                }
                else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_ScaleZ )
                {
                    camInvAt.z = 0;
                    normal = camInvAt; // set plane normal to face the camera.
                    axisvector = Vector3(0,0,1);
                }
                else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_ScaleXYZ )
                {
                    // TODO: fix
                    normal = Vector3(0,0,1);
                    axisvector = Vector3(1,0,0);
                }

                // TODO: support local space translation.
                if( 1 ) // if( world space translation )
                {
                    // create a world space plane.
                    plane.Set( normal, pObjectTransform->GetTranslation() );
                }
//                else
//                {
//                    // TODO: support this.
//                    // transform the normal into the selected objects space.
//                    plane.Set( (*pObjectTransform * Vector4( normal, 0 )).XYZ(), pObjectTransform->GetTranslation() );
//                }
            }

            // Get the mouse click ray... current and last frame.
            Vector3 currentraystart, currentrayend;
            pGame->GetMouseRay( pEditorState->m_CurrentMousePosition, &currentraystart, &currentrayend );

            Vector3 lastraystart, lastrayend;
            pGame->GetMouseRay( pEditorState->m_LastMousePosition, &lastraystart, &lastrayend );

            //LOGInfo( LOGTag, "current->(%0.0f,%0.0f) (%0.2f,%0.2f,%0.2f) (%0.2f,%0.2f,%0.2f)\n",
            //        pEditorState->m_CurrentMousePosition.x,
            //        pEditorState->m_CurrentMousePosition.y,
            //        currentraystart.x,
            //        currentraystart.y,
            //        currentraystart.z,
            //        currentrayend.x,
            //        currentrayend.y,
            //        currentrayend.z
            //    );

            // find the intersection point of the plane.
            Vector3 currentresult;
            Vector3 lastresult;
            if( plane.IntersectRay( currentraystart, currentrayend, &currentresult ) &&
                plane.IntersectRay( lastraystart, lastrayend, &lastresult ) )
            {
                //LOGInfo( LOGTag, "currentresult( %f, %f, %f );\n", currentresult.x, currentresult.y, currentresult.z );
                //LOGInfo( LOGTag, "lastresult( %f, %f, %f );", lastresult.x, lastresult.y, lastresult.z );
                //LOGInfo( LOGTag, "axisvector( %f, %f, %f );\n", axisvector.x, axisvector.y, axisvector.z );

                // TODO: support local space scale?.
                // lock to one of the 3 axis.
                if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_ScaleX )
                {
                    currentresult.y = currentresult.z = 0;
                    lastresult.y = lastresult.z = 0;

                    LOGInfo( "Scale Gizmo", "ScaleX: " );
                }
                if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_ScaleY )
                {
                    currentresult.x = currentresult.z = 0;
                    lastresult.x = lastresult.z = 0;

                    LOGInfo( "Scale Gizmo", "ScaleY: " );
                }
                if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_ScaleZ )
                {
                    currentresult.x = currentresult.y = 0;
                    lastresult.x = lastresult.y = 0;

                    LOGInfo( "Scale Gizmo", "ScaleZ: " );
                }

                // find the diff pos between this frame and last.
                Vector3 diff = currentresult - lastresult;

                // GIZMOSCALE: scale all of the things. // undo is handled by EngineCore.cpp when mouse is lifted.
                pEditorState->m_DistanceTranslated += diff;
                LOGInfo( "Scale Gizmo", "pEditorState->m_DistanceTranslated.Set( %f, %f, %f ); ", pEditorState->m_DistanceTranslated.x, pEditorState->m_DistanceTranslated.y, pEditorState->m_DistanceTranslated.z );
                LOGInfo( "Scale Gizmo", "diff( %f, %f, %f, %d );\n", diff.x, diff.y, diff.z, pEditorState->m_pSelectedObjects.size() );

                ScaleSelectedObjects( pEditorState, diff );
            }
        }
    }
}

void TransformGizmo::ScaleSelectedObjects(EditorState* pEditorState, Vector3 scale)
{
    for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
    {
        ComponentTransform* pTransform = pEditorState->m_pSelectedObjects[i]->m_pComponentTransform;

        // if this object has a selected parent, don't move it, only move the parent.
        if( pTransform->IsAnyParentInList( pEditorState->m_pSelectedObjects ) == false )
        {
            Vector3 currscale = pTransform->GetLocalTransform()->GetScale();

            pTransform->SetScaleByEditor( currscale + scale );
            pTransform->UpdateTransform();
        }
    }
}

void TransformGizmo::CancelLastScale(EditorState* pEditorState)
{
    ScaleSelectedObjects( pEditorState, Vector3( 1 / pEditorState->m_DistanceTranslated.x, 1 / pEditorState->m_DistanceTranslated.y, 1 / pEditorState->m_DistanceTranslated.z ) );
}
