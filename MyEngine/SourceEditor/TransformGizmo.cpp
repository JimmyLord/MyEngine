//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorState.h"
#include "TransformGizmo.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/BaseComponents/ComponentRenderable.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/FrameworkComponents/ComponentMesh.h"
#include "Core/EngineComponentTypeManager.h"
#include "Core/EngineCore.h"
#include "Prefs/EditorPrefs.h"
#include "../SourceEditor/Interfaces/EditorInterface.h"

#if MYFW_USING_IMGUI
#include "../SourceEditor/Editor_ImGui/ImGuiStylePrefs.h"
#endif

float g_TransformScale = 1/25.0f;

TransformGizmo::TransformGizmo(EngineCore* pEngineCore)
{
    m_pEngineCore = pEngineCore;

    m_VisibleIfObjectsSelected = false;
    m_pSelectedPart = 0;

    for( int i=0; i<3; i++ )
    {
        m_pTranslate1Axis[i] = 0;
        m_pTranslate2Axis[i] = 0;
        m_pScale1Axis[i] = 0;
        m_pRotate1Axis[i] = 0;

        m_pMaterial_Translate1Axis[i] = 0;
        m_pMaterial_Translate2Axis[i] = 0;
        m_pMaterial_Scale1Axis[i] = 0;
        m_pMaterial_Rotate1Axis[i] = 0;
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
        SAFE_DELETE( m_pRotate1Axis[i] );

        SAFE_RELEASE( m_pMaterial_Translate1Axis[i] );
        SAFE_RELEASE( m_pMaterial_Translate2Axis[i] );
        SAFE_RELEASE( m_pMaterial_Scale1Axis[i] );
        SAFE_RELEASE( m_pMaterial_Rotate1Axis[i] );
    }

    SAFE_DELETE( m_pScale3Axis );
    SAFE_RELEASE( m_pMaterial_Scale3Axis );
}

void TransformGizmo::Tick(float deltaTime, EditorState* pEditorState)
{
    // Find the center of the object.
    bool GizmoVisible = false;
    Vector3 ObjectPosition;
    MyMatrix ObjectTransform;

    EditorActionState currentaction = pEditorState->m_EditorActionState;

    // if the gizmo is currently being used, lower it's opacity
#if MYFW_USING_IMGUI
    EditorPrefs* pEditorPrefs = pEditorState->GetEngineCore()->GetEditorPrefs();;

    float selectedOpacity = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_TransformGizmoAlphaMax ).w;
    float overallOpacity = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_TransformGizmoAlphaMax ).w;
    if( currentaction != EditorActionState::None )
    {
        selectedOpacity = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_TransformGizmoAlphaInUse ).w;
        overallOpacity = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_TransformGizmoAlphaMin ).w;
    }
#else
    float selectedOpacity = 1.0f;
    float overallOpacity = 1.0f;
    if( currentaction != EditorActionState::None )
    {
        selectedOpacity = 0.2f;
        overallOpacity = 0.05f;
    }
#endif

    if( pEditorState->m_pSelectedObjects.size() == 1 )
    {
        ComponentTransform* pTransform = pEditorState->m_pSelectedObjects[0]->GetTransform();

        if( pTransform )
        {
            GizmoVisible = true;
            ObjectPosition = pTransform->GetWorldPosition();
            ObjectTransform.CreateRotation( pTransform->GetWorldRotation() );

            m_GizmoWorldTransform = ObjectTransform;
            m_GizmoWorldTransform.Translate( pTransform->GetWorldPosition() );

            m_GizmoWorldRotation = pTransform->GetWorldRotation();
        }
        else
        {
            m_GizmoWorldTransform.SetIdentity();
            m_GizmoWorldTransform.Translate( Vector3( 0, 0, 0 ) );

            m_GizmoWorldRotation.Set( 0, 0, 0 );
        }
    }
    else if( pEditorState->m_pSelectedObjects.size() > 1 )
    {
        GizmoVisible = true;

        // find the center point between all selected objects.
        ObjectPosition.Set( 0, 0, 0 );
        unsigned int count = 0;
        for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
        {
            if( pEditorState->m_pSelectedObjects[i]->GetTransform() )
            {
                ObjectPosition += pEditorState->m_pSelectedObjects[i]->GetTransform()->GetLocalPosition();
                count++;
            }
        }
        ObjectPosition /= (float)count;

        ObjectTransform.SetIdentity();

        m_GizmoWorldTransform = ObjectTransform;
        m_GizmoWorldTransform.SetTranslation( ObjectPosition );
        m_GizmoWorldRotation.Set( 0, 0, 0 );
    }
    else
    {
        GizmoVisible = false;
    }

    if( m_VisibleIfObjectsSelected == false )
        GizmoVisible = false;

    // If we're rotating in world space, then rotate the gizmo when the mouse is down
    Vector3 AmountOfDistanceRotatedToAddToGizmo = pEditorState->m_DistanceRotated;

    // Create an object rotation matrix, used to rotate the gizmos.
    MyMatrix ObjectRotation;
    ObjectRotation.SetIdentity();

    // Create an inverse world transform to bring the camera into object space
    MyMatrix InverseWorldTransform;

    // if we're dealing with a local space transform, then fix the rotation matrices to match
    if( (currentaction != EditorActionState::None && pEditorState->m_TransformedInLocalSpace == true ) ||
        (currentaction == EditorActionState::None && (pEditorState->m_ModifierKeyStates & MODIFIERKEY_Control) == false) )
    {
        // We're transforming in object space, so set up a rotation matrix
        ObjectRotation.CreateRotation( ObjectTransform.GetEulerAngles() * 180.0f/PI );

        // Since gizmo is being rotated by object matrix, we don't need additional rotation.
        AmountOfDistanceRotatedToAddToGizmo.Set( 0, 0, 0 );

        // Create inverse world transform to bring the camera into object space
        if( pEditorState->m_pSelectedObjects.size() > 0 )
        {
            InverseWorldTransform = m_GizmoWorldTransform;
            InverseWorldTransform.Inverse();
        }
    }
    else
    {
        // Create inverse world transform to bring the camera into object space
        InverseWorldTransform.CreateTranslation( ObjectPosition );
        InverseWorldTransform.Inverse();
    }

    // Update 1 axis transform gizmos
    for( int i=0; i<3; i++ )
    {
        MyAssert( m_pTranslate1Axis[i] );

        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pTranslate1Axis[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );

        ComponentMesh* pMesh = dynamic_cast<ComponentMesh*>( pRenderable );
        MyAssert( pMesh );
        if( pMesh )
        {
            // Hide object from editor debug draw call list.
            pMesh->m_pRenderGraphObjects[0]->SetAsEditorObject();

            MaterialDefinition* pMaterial = pMesh->GetMaterial( 0 );
            if( pMaterial )
            {
                if( i == 0 )
                    pMaterial->m_ColorDiffuse.Set( 255, 100, 100, (unsigned char)(255*overallOpacity) );
                if( i == 1 )
                    pMaterial->m_ColorDiffuse.Set( 100, 255, 100, (unsigned char)(255*overallOpacity) );
                if( i == 2 )
                    pMaterial->m_ColorDiffuse.Set( 100, 100, 255, (unsigned char)(255*overallOpacity) );

                if( ( m_pTranslate1Axis[i] == m_pSelectedPart && currentaction == EditorActionState::None ) ||
                    (int)currentaction == (int)EditorActionState::TranslateX + i )
                {
                    pMaterial->m_ColorDiffuse.Set( 255, 255, 255, (unsigned char)(255*selectedOpacity) );
                }
            }
        }

        pRenderable->SetVisible( GizmoVisible );

        if( GizmoVisible )
        {
            // rotate the gizmo.
            MyMatrix matrot;
            matrot.SetIdentity();
            if( i == 0 )
                matrot.Rotate( 90, 0, 0, 1 );
            if( i == 1 )
                matrot.Rotate( 0, 1, 0, 0 );
            if( i == 2 )
                matrot.Rotate( -90, 1, 0, 0 );

            matrot = ObjectRotation * matrot;
            Vector3 rot = matrot.GetEulerAngles() * 180.0f/PI + AmountOfDistanceRotatedToAddToGizmo;
            m_pTranslate1Axis[i]->GetTransform()->SetLocalRotation( rot );

            // move the gizmo to the object position.
            m_pTranslate1Axis[i]->GetTransform()->SetLocalPosition( ObjectPosition );

            float distance = (pEditorState->m_pEditorCamera->GetTransform()->GetLocalPosition() - ObjectPosition).Length();
            m_pTranslate1Axis[i]->GetTransform()->SetLocalScale( Vector3( distance * g_TransformScale ) );
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
            // Hide object from editor debug draw call list.
            pMesh->m_pRenderGraphObjects[0]->SetAsEditorObject();

            MaterialDefinition* pMaterial = pMesh->GetMaterial( 0 );
            if( pMaterial )
            {
                if( i == 0 )
                    pMaterial->m_ColorDiffuse.Set( 255, 255, 100, (unsigned char)(180*overallOpacity) ); // XY
                if( i == 1 )
                    pMaterial->m_ColorDiffuse.Set( 255, 100, 255, (unsigned char)(180*overallOpacity) ); // XZ
                if( i == 2 )
                    pMaterial->m_ColorDiffuse.Set( 100, 255, 255, (unsigned char)(180*overallOpacity) ); // YZ

                if( ( m_pTranslate2Axis[i] == m_pSelectedPart && currentaction == EditorActionState::None ) ||
                    (int)currentaction == (int)EditorActionState::TranslateXY + i )
                {
                    pMaterial->m_ColorDiffuse.Set( 255, 255, 255, (unsigned char)(255*selectedOpacity) );
                }
            }
        }

        pRenderable->SetVisible( GizmoVisible );

        if( GizmoVisible )
        {
            ComponentCamera* pCamera = pEditorState->GetEditorCamera();
            Vector3 campos = pCamera->GetGameObject()->GetTransform()->GetLocalPosition();

            Vector3 pos = ObjectPosition;

            // move camera position into object space for comparisons.
            MyMatrix worldTransform = m_GizmoWorldTransform;
            worldTransform.Inverse();
            Vector3 objectSpaceCamPos = InverseWorldTransform * campos;

            // rotate the 2-axis translation gizmo.
            MyMatrix matrot;
            matrot.SetIdentity();
            if( i == 0 ) // xy
            {
                if( objectSpaceCamPos.z  > 0 ) { matrot.Rotate( -90, 1, 0, 0 ); matrot.Rotate( -90, 0, 0, 1 ); }
                if( objectSpaceCamPos.z <= 0 ) { matrot.Rotate( 90, 1, 0, 0 ); }
            }
            if( i == 1 ) // xz
            {
                if( objectSpaceCamPos.y  > 0 ) { matrot.Rotate( -90, 0, 1, 0 ); }
                if( objectSpaceCamPos.y <= 0 ) { matrot.Rotate( 180, 1, 0, 0 ); }
            }
            if( i == 2 ) // yz
            {
                if( objectSpaceCamPos.x  > 0 ) { matrot.Rotate( -90, 0, 0, 1 ); matrot.Rotate( 180, 0, 1, 0 ); }
                if( objectSpaceCamPos.x <= 0 ) { matrot.Rotate( +90, 1, 0, 0 ); matrot.Rotate( -90, 0, 1, 0 ); }
            }

            matrot = ObjectRotation * matrot;
            Vector3 rot = matrot.GetEulerAngles() * 180.0f/PI + AmountOfDistanceRotatedToAddToGizmo;
            m_pTranslate2Axis[i]->GetTransform()->SetLocalRotation( rot );

            // move the gizmo to the object position.
            m_pTranslate2Axis[i]->GetTransform()->SetLocalPosition( pos );

            float distance = (pEditorState->m_pEditorCamera->GetTransform()->GetLocalPosition() - ObjectPosition).Length();
            m_pTranslate2Axis[i]->GetTransform()->SetLocalScale( Vector3( distance * g_TransformScale ) );
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
            // Hide object from editor debug draw call list.
            pMesh->m_pRenderGraphObjects[0]->SetAsEditorObject();

            MaterialDefinition* pMaterial = pMesh->GetMaterial( 0 );
            if( pMaterial )
            {
                if( i == 0 )
                    pMaterial->m_ColorDiffuse.Set( 255, 100, 100, (unsigned char)(255*overallOpacity) );
                if( i == 1 )
                    pMaterial->m_ColorDiffuse.Set( 100, 255, 100, (unsigned char)(255*overallOpacity) );
                if( i == 2 )
                    pMaterial->m_ColorDiffuse.Set( 100, 100, 255, (unsigned char)(255*overallOpacity) );

                if( ( m_pScale1Axis[i] == m_pSelectedPart && currentaction == EditorActionState::None ) ||
                    (int)currentaction == (int)EditorActionState::ScaleX + i )
                {
                    pMaterial->m_ColorDiffuse.Set( 255, 255, 255, (unsigned char)(255*selectedOpacity) );
                }
            }
        }

        pRenderable->SetVisible( GizmoVisible );

        if( GizmoVisible )
        {
            // rotate the gizmo.
            MyMatrix matrot;
            matrot.SetIdentity();

            matrot = ObjectRotation * matrot;
            Vector3 rot = matrot.GetEulerAngles() * 180.0f/PI + AmountOfDistanceRotatedToAddToGizmo;
            m_pScale1Axis[i]->GetTransform()->SetLocalRotation( rot );

            // move the gizmo to the object position.
            m_pScale1Axis[i]->GetTransform()->SetLocalPosition( ObjectPosition );

            float distance = (pEditorState->m_pEditorCamera->GetTransform()->GetLocalPosition() - ObjectPosition).Length();
            m_pScale1Axis[i]->GetTransform()->SetLocalScale( Vector3( distance * g_TransformScale ) );
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
            // Hide object from editor debug draw call list.
            pMesh->m_pRenderGraphObjects[0]->SetAsEditorObject();

            MaterialDefinition* pMaterial = pMesh->GetMaterial( 0 );
            if( pMaterial )
            {
                pMaterial->m_ColorDiffuse.Set( 100, 100, 100, (unsigned char)(180*overallOpacity) );

                if( m_pScale3Axis == m_pSelectedPart )
                {
                    pMaterial->m_ColorDiffuse.Set( 255, 255, 255, (unsigned char)(180*selectedOpacity) );
                }
            }
        }

        pRenderable->SetVisible( GizmoVisible );

        if( GizmoVisible )
        {
            Vector3 pos = ObjectPosition;

            // rotate the gizmo.
            MyMatrix matrot;
            matrot.SetIdentity();

            matrot = ObjectRotation * matrot;
            Vector3 rot = matrot.GetEulerAngles() * 180.0f/PI + AmountOfDistanceRotatedToAddToGizmo;
            m_pScale3Axis->GetTransform()->SetLocalRotation( rot );

            // move the gizmo to the object position.
            m_pScale3Axis->GetTransform()->SetLocalPosition( pos );

            float distance = (pEditorState->m_pEditorCamera->GetTransform()->GetLocalPosition() - ObjectPosition).Length();
            m_pScale3Axis->GetTransform()->SetLocalScale( Vector3( distance * g_TransformScale ) );
        }
    }

    // Update 1 axis rotate gizmos
    for( int i=0; i<3; i++ )
    {
        MyAssert( m_pRotate1Axis[i] );

        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pRotate1Axis[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );

        ComponentMesh* pMesh = dynamic_cast<ComponentMesh*>( pRenderable );
        MyAssert( pMesh );
        if( pMesh )
        {
            // Hide object from editor debug draw call list.
            pMesh->m_pRenderGraphObjects[0]->SetAsEditorObject();

            MaterialDefinition* pMaterial = pMesh->GetMaterial( 0 );
            if( pMaterial )
            {
                if( i == 0 )
                    pMaterial->m_ColorDiffuse.Set( 255, 100, 100, (unsigned char)(255*overallOpacity) );
                if( i == 1 )
                    pMaterial->m_ColorDiffuse.Set( 100, 255, 100, (unsigned char)(255*overallOpacity) );
                if( i == 2 )
                    pMaterial->m_ColorDiffuse.Set( 100, 100, 255, (unsigned char)(255*overallOpacity) );

                if( ( m_pRotate1Axis[i] == m_pSelectedPart && currentaction == EditorActionState::None ) ||
                    (int)currentaction == (int)EditorActionState::RotateX + i )
                {
                    pMaterial->m_ColorDiffuse.Set( 255, 255, 255, (unsigned char)(255*selectedOpacity) );
                }
            }
        }

        pRenderable->SetVisible( GizmoVisible );

        if( GizmoVisible )
        {
            ComponentCamera* pCamera = pEditorState->GetEditorCamera();
            Vector3 campos = pCamera->GetGameObject()->GetTransform()->GetLocalPosition();

            // move camera position into object space for comparisons.
            MyMatrix worldTransform = m_GizmoWorldTransform;
            worldTransform.Inverse();
            Vector3 objectSpaceCamPos = InverseWorldTransform * campos;

            // rotate the rotation gizmo.
            MyMatrix matrot;
            matrot.SetIdentity();
            if( i == 0 )
            {
                if( objectSpaceCamPos.x  > 0 ) { matrot.Rotate( 90, 0, 1, 0 ); }
                if( objectSpaceCamPos.x <= 0 ) { matrot.Rotate( -90, 0, 1, 0 ); matrot.Rotate( -90, 1, 0, 0 ); }
            }
            if( i == 1 )
            {
                if( objectSpaceCamPos.y  > 0 ) { matrot.Rotate( -90, 1, 0, 0 ); }
                if( objectSpaceCamPos.y <= 0 ) { matrot.Rotate( 90, 1, 0, 0 ); matrot.Rotate( 90, 0, 1, 0 ); }
            }
            if( i == 2 )
            {
                if( objectSpaceCamPos.z  > 0 ) { matrot.Rotate( 180, 1, 0, 0 ); matrot.Rotate( -90, 0, 0, 1 ); }
            }

            matrot = ObjectRotation * matrot;
            Vector3 rot = matrot.GetEulerAngles() * 180.0f/PI + AmountOfDistanceRotatedToAddToGizmo;
            m_pRotate1Axis[i]->GetTransform()->SetLocalRotation( rot );

            // move the gizmo to the object position.
            m_pRotate1Axis[i]->GetTransform()->SetLocalPosition( ObjectPosition );

            float distance = (pEditorState->m_pEditorCamera->GetTransform()->GetLocalPosition() - ObjectPosition).Length();
            m_pRotate1Axis[i]->GetTransform()->SetLocalScale( Vector3( distance * g_TransformScale ) );
        }
    }
}

void TransformGizmo::Hide()
{
    // Update 1 axis transform gizmos
    for( int i=0; i<3; i++ )
    {
        MyAssert( m_pTranslate1Axis[i] );
        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pTranslate1Axis[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
        pRenderable->SetVisible( false );
    }

    // Update 2 axis transform gizmos
    for( int i=0; i<3; i++ )
    {
        MyAssert( m_pTranslate2Axis[i] );
        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pTranslate2Axis[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
        pRenderable->SetVisible( false );
    }

    // Update 1 axis scale gizmos
    for( int i=0; i<3; i++ )
    {
        MyAssert( m_pScale1Axis[i] );
        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pScale1Axis[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
        pRenderable->SetVisible( false );
    }

    // Update 3 axis scale gizmo
    {
        MyAssert( m_pScale3Axis );
        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pScale3Axis->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
        pRenderable->SetVisible( false );
    }

    // Update 1 axis rotate gizmos
    for( int i=0; i<3; i++ )
    {
        MyAssert( m_pRotate1Axis[i] );
        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pRotate1Axis[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
        pRenderable->SetVisible( false );
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
    m_pSelectedPart = pGame->GetCurrentEditorInterface()->GetObjectAtPixel( (unsigned int)x, (unsigned int)y, true, true );

    return false;
}

void TransformGizmo::CreateAxisObjects(SceneID sceneid, float scale, EditorState* pEditorState)
{
    GameObject* pGameObject;
    ComponentMesh* pComponentMesh;

    MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();

    m_pMaterial_Translate1Axis[0] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(255,0,0,255) );
    m_pMaterial_Translate1Axis[1] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(0,255,0,255) );
    m_pMaterial_Translate1Axis[2] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(0,0,255,255) );

    // Create single axis translators.
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - x-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Translate1Axis[0], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.05f ); //, ColorByte(255, 100, 100, 255) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pTranslate1Axis[0] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - y-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Translate1Axis[1], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.05f ); //, ColorByte(100, 255, 100, 255) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pTranslate1Axis[1] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - z-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Translate1Axis[2], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.05f ); //, ColorByte(100, 100, 255, 255) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pTranslate1Axis[2] = pGameObject;
    }

    m_pMaterial_Translate2Axis[0] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(255,0,0,100) );
    m_pMaterial_Translate2Axis[1] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(0,255,0,100) );
    m_pMaterial_Translate2Axis[2] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(0,0,255,100) );

    // Create 2 axis translators
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - xy-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Translate2Axis[0], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->CreatePlane( Vector3(0,0,1), Vector2(1,1), Vector2Int(2,2), Vector2(0,0), Vector2(1,1) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pTranslate2Axis[2] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - xz-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Translate2Axis[1], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->CreatePlane( Vector3(0,0,1), Vector2(1,1), Vector2Int(2,2), Vector2(0,0), Vector2(1,1) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pTranslate2Axis[1] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - yz-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Translate2Axis[2], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->CreatePlane( Vector3(0,0,1), Vector2(1,1), Vector2Int(2,2), Vector2(0,0), Vector2(1,1) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pTranslate2Axis[0] = pGameObject;
    }

    m_pMaterial_Scale1Axis[0] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(255,0,0,255) );
    m_pMaterial_Scale1Axis[1] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(0,255,0,255) );
    m_pMaterial_Scale1Axis[2] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(0,0,255,255) );

    Vector3 scaleboxsize( 0.7f, 0.7f, 0.7f );

    // Create single axis scalers.
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - x-scale" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Scale1Axis[0], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->CreateBox( scaleboxsize.x, scaleboxsize.y, scaleboxsize.z, 0, 1, 0, 1, Justify_Center, Vector3( 3, 0, 0 ) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pScale1Axis[0] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - y-scale" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Scale1Axis[1], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->CreateBox( scaleboxsize.x, scaleboxsize.y, scaleboxsize.z, 0, 1, 0, 1, Justify_Center, Vector3( 0, 3, 0 ) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pScale1Axis[1] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - z-scale" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Scale1Axis[2], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->CreateBox( scaleboxsize.x, scaleboxsize.y, scaleboxsize.z, 0, 1, 0, 1, Justify_Center, Vector3( 0, 0, 3 ) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pScale1Axis[2] = pGameObject;
    }

    m_pMaterial_Scale3Axis = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(255,0,0,100) );

    // Create 3 axis scaler
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - xyz-scale" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Scale3Axis, 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->CreateBox( scaleboxsize.x, scaleboxsize.y, scaleboxsize.z, 0, 1, 0, 1, Justify_Center, Vector3(0,0,0) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pScale3Axis = pGameObject;
    }

    m_pMaterial_Rotate1Axis[0] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(255,0,0,255) );
    m_pMaterial_Rotate1Axis[1] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(0,255,0,255) );
    m_pMaterial_Rotate1Axis[2] = MyNew MaterialDefinition( pMaterialManager, g_pEngineCore->GetShader_TintColorWithAlpha(), ColorByte(0,0,255,255) );

    float startradius = 2.8f;
    float endradius = 3.0f;

    // Create single axis rotators.
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - x-axis-rotate" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Rotate1Axis[0], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->Create2DArc( Vector3(0), 10, 80, startradius, endradius, 5 );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pRotate1Axis[0] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - y-axis-rotate" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Rotate1Axis[1], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->Create2DArc( Vector3(0), 10, 80, startradius, endradius, 5 );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pRotate1Axis[1] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false, sceneid ); // not managed.
        pGameObject->SetName( "3D Transform Gizmo - z-axis-rotate" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid, g_pComponentSystemManager );
        if( pComponentMesh )
        {
            pComponentMesh->SetVisible( true );
            pComponentMesh->SetMaterial( m_pMaterial_Rotate1Axis[2], 0 );
            pComponentMesh->SetLayersThisExistsOn( Layer_EditorFG );
            pComponentMesh->m_pMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );
            pComponentMesh->m_pMesh->Create2DArc( Vector3(0), 10, 80, startradius, endradius, 5 );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->GetSubmesh( 0 )->m_PrimitiveType;
            pComponentMesh->AddToRenderGraph();
        }

        pEditorState->m_pTransformGizmo->m_pRotate1Axis[2] = pGameObject;
    }
}

void TransformGizmo::ScaleGizmosForMousePickRendering(bool doscale)
{
    float scaleamount = 7;

    if( doscale == false )
        scaleamount = 1/scaleamount;

    for( int i=0; i<3; i++ )
    {
        Vector3 currentscale = m_pTranslate1Axis[i]->GetTransform()->GetLocalScale();
        Vector3 newscale( currentscale.x * scaleamount, currentscale.y, currentscale.z * scaleamount );
        m_pTranslate1Axis[i]->GetTransform()->SetLocalScale( newscale );

        // update the world matrix, so the scale will apply when the scene graph renders the scene.
        m_pTranslate1Axis[i]->GetTransform()->UpdateTransform();
    }
}

void TransformGizmo::CancelCurrentOperation(EditorState* pEditorState)
{
    if( pEditorState->m_EditorActionState == EditorActionState::TranslateX ||
        pEditorState->m_EditorActionState == EditorActionState::TranslateY ||
        pEditorState->m_EditorActionState == EditorActionState::TranslateZ ||
        pEditorState->m_EditorActionState == EditorActionState::TranslateXY ||
        pEditorState->m_EditorActionState == EditorActionState::TranslateXZ ||
        pEditorState->m_EditorActionState == EditorActionState::TranslateYZ )
    {
        CancelLastTranslation( pEditorState );
    }

    if( pEditorState->m_EditorActionState == EditorActionState::RotateX ||
        pEditorState->m_EditorActionState == EditorActionState::RotateY ||
        pEditorState->m_EditorActionState == EditorActionState::RotateZ )
    {
        CancelLastRotation( pEditorState );
    }

    if( pEditorState->m_EditorActionState == EditorActionState::ScaleX ||
        pEditorState->m_EditorActionState == EditorActionState::ScaleY ||
        pEditorState->m_EditorActionState == EditorActionState::ScaleZ ||
        pEditorState->m_EditorActionState == EditorActionState::ScaleXYZ )
    {
        CancelLastScale( pEditorState );
    }
}

void TransformGizmo::TranslateSelectedObjects(EngineCore* pGame, EditorState* pEditorState)
{
    if( pEditorState->m_pSelectedObjects.size() == 0 )
        return;

    // move the selected objects along a plane or axis
    if( pEditorState->m_EditorActionState == EditorActionState::TranslateX ||
        pEditorState->m_EditorActionState == EditorActionState::TranslateY ||
        pEditorState->m_EditorActionState == EditorActionState::TranslateZ ||
        pEditorState->m_EditorActionState == EditorActionState::TranslateXY ||
        pEditorState->m_EditorActionState == EditorActionState::TranslateXZ ||
        pEditorState->m_EditorActionState == EditorActionState::TranslateYZ )
    {
        // move all selected objects by the same amount, use object 0 to create a plane.
        {
            Vector3 AxisX( 1, 0, 0 );
            Vector3 AxisY( 0, 1, 0 );
            Vector3 AxisZ( 0, 0, 1 );

            // create a plane based on the axis we want.
            Vector3 axisvector;
            Plane plane;
            {
                ComponentCamera* pCamera = pEditorState->GetEditorCamera();
                Vector3 camInvAt = pCamera->GetGameObject()->GetTransform()->GetLocalTransform()->GetAt() * -1;

                MyMatrix ObjectRotation;
                ObjectRotation.CreateRotation( m_GizmoWorldRotation );

                Vector3 normal;
                if( pEditorState->m_EditorActionState == EditorActionState::TranslateX )
                {
                    normal = Vector3(0,0,1); // xy plane
                    if( fabs(camInvAt.Dot( normal )) < 0.7071f ) // if cam dot normal is under 45degrees
                        normal = Vector3(0,1,0); // xz plane

                    axisvector = Vector3(1,0,0);
                }
                else if( pEditorState->m_EditorActionState == EditorActionState::TranslateY )
                {
                    normal = Vector3(1,0,0); // yz plane
                    if( fabs(camInvAt.Dot( normal )) < 0.7071f ) // if cam dot normal is under 45degrees
                        normal = Vector3(0,0,1); // xy plane

                    axisvector = Vector3(0,1,0);
                }
                else if( pEditorState->m_EditorActionState == EditorActionState::TranslateZ )
                {
                    normal = Vector3(0,1,0); // xz plane
                    if( fabs(camInvAt.Dot( normal )) < 0.7071f ) // if cam dot normal is under 45degrees
                        normal = Vector3(1,0,0); // yz plane

                    axisvector = Vector3(0,0,1);
                }
                else if( pEditorState->m_EditorActionState == EditorActionState::TranslateXY )
                {
                    normal = Vector3(0,0,1);
                    axisvector = Vector3(1,1,0);
                }
                else if( pEditorState->m_EditorActionState == EditorActionState::TranslateXZ )
                {
                    normal = Vector3(0,1,0);
                    axisvector = Vector3(1,0,1);
                }
                else if( pEditorState->m_EditorActionState == EditorActionState::TranslateYZ )
                {
                    normal = Vector3(1,0,0);
                    axisvector = Vector3(0,1,1);
                }

                // if object space
                if( pEditorState->m_TransformedInLocalSpace )
                {
                    // get our object space axis vectors
                    AxisX = ObjectRotation * AxisX;
                    AxisY = ObjectRotation * AxisY;
                    AxisZ = ObjectRotation * AxisZ;

                    normal = ObjectRotation * normal;
                }

                //LOGInfo( "TransformGizmo", "camInvAt( %f, %f, %f ) normal( %f, %f, %f )\n", camInvAt.x, camInvAt.y, camInvAt.z, normal.x, normal.y, normal.z );

                // create a plane. // TODO: fix the plane rotation for object space translations
                plane.Set( normal, m_GizmoWorldTransform.GetTranslation() );
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
                //LOGInfo( "TransformGizmo", "currentresult( %f, %f, %f );", currentresult.x, currentresult.y, currentresult.z );
                //LOGInfo( "TransformGizmo", "lastresult( %f, %f, %f );", lastresult.x, lastresult.y, lastresult.z );
                //LOGInfo( "TransformGizmo", "axisvector( %f, %f, %f );\n", axisvector.x, axisvector.y, axisvector.z );

                Vector3 diff = currentresult - lastresult;

                // if snapping to grid is enabled, then use m_LastIntersectResultUsed instead of last frames result.
                if( g_pEngineCore->GetEditorPrefs()->Get_Grid_SnapEnabled() || pEditorState->m_ModifierKeyStates & MODIFIERKEY_Alt )
                {
                    // snap object 0 to grid, all other will stay relative.
                    Vector3 pos = m_GizmoWorldTransform.GetTranslation();

                    if( m_LastIntersectResultIsValid == false )
                    {
                        m_LastIntersectResultUsed = lastresult;
                        m_LastIntersectResultIsValid = true;
                    }

                    diff = currentresult - m_LastIntersectResultUsed;

                    Vector3 finalpos = pos + diff/2;
                    Vector3 newfinalpos;
                    newfinalpos.x = MyRoundToMultipleOf( finalpos.x, g_pEngineCore->GetEditorPrefs()->GetGridSettings()->stepSize.x );
                    newfinalpos.y = MyRoundToMultipleOf( finalpos.y, g_pEngineCore->GetEditorPrefs()->GetGridSettings()->stepSize.y );
                    newfinalpos.z = MyRoundToMultipleOf( finalpos.z, g_pEngineCore->GetEditorPrefs()->GetGridSettings()->stepSize.z );

                    diff = newfinalpos - pos;

                    if( diff.x != 0 )
                        m_LastIntersectResultUsed.x = currentresult.x;
                    if( diff.y != 0 )
                        m_LastIntersectResultUsed.y = currentresult.y;
                    if( diff.z != 0 )
                        m_LastIntersectResultUsed.z = currentresult.z;
                }

                // For single axis translations, apply the correct amount of diff on that axis
                if( pEditorState->m_EditorActionState == EditorActionState::TranslateX )
                {
                    diff = AxisX * diff.Dot( AxisX );
                }
                else if( pEditorState->m_EditorActionState == EditorActionState::TranslateY)
                {
                    diff = AxisY * diff.Dot( AxisY );
                }
                else if( pEditorState->m_EditorActionState == EditorActionState::TranslateZ )
                {
                    diff = AxisZ * diff.Dot( AxisZ );
                }
                else
                {
                    // for 2-axis translation, nothing needs to be done since the plane created is in object space
                }

                // GIZMOTRANSLATE: move all of the things. // undo is handled by EngineCore.cpp when mouse is lifted.
                pEditorState->m_DistanceTranslated += diff;
                //LOGInfo( LOGTag, "pEditorState->m_DistanceTranslated.Set( %f, %f, %f );", pEditorState->m_DistanceTranslated.x, pEditorState->m_DistanceTranslated.y, pEditorState->m_DistanceTranslated.z );
                //LOGInfo( LOGTag, "diff( %f, %f, %f, %d );\n", diff.x, diff.y, diff.z, pEditorState->m_pSelectedObjects.size() );

                TranslateSelectedObjects( pEditorState, diff );
            }
        }
    }
}

void TransformGizmo::TranslateSelectedObjects(EditorState* pEditorState, Vector3 distance)
{
    for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
    {
        ComponentTransform* pTransform = pEditorState->m_pSelectedObjects[i]->GetTransform();

        // if this object has a selected parent, don't move it, only move the parent.
        if( pTransform && pTransform->IsAnyParentInList( pEditorState->m_pSelectedObjects ) == false )
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
    if( pEditorState->m_EditorActionState == EditorActionState::ScaleX ||
        pEditorState->m_EditorActionState == EditorActionState::ScaleY ||
        pEditorState->m_EditorActionState == EditorActionState::ScaleZ ||
        pEditorState->m_EditorActionState == EditorActionState::ScaleXYZ )
    {
        // move all selected objects by the same amount, use object 0 to create a plane.
        {
            if( pEditorState->m_TransformedInLocalSpace ) // local space
            {
                pEditorState->m_WorldSpacePivot.Set( 0, 0, 0 );
            }
            else
            {
                Vector3 gizmopos = m_pTranslate1Axis[0]->GetTransform()->GetWorldPosition();
                pEditorState->m_WorldSpacePivot = gizmopos;
            }

            {
                float distance = pEditorState->m_CurrentMousePosition.x - pEditorState->m_LastMousePosition.x
                               + pEditorState->m_CurrentMousePosition.y - pEditorState->m_LastMousePosition.y;

                // TODO: this is scaling in pixels rather than centimetres travelled, will behave differently on different resolutions
                distance /= 100;

                // negative distance is scale down, so flip and fabs the float
                if( distance < 0 )
                    distance = 1 / ( 1 + fabsf(distance) );
                else
                    distance = 1 + distance;

                Vector3 diff( distance );

                if( pEditorState->m_EditorActionState == EditorActionState::ScaleX )
                    diff.y = diff.z = 1;
                if( pEditorState->m_EditorActionState == EditorActionState::ScaleY )
                    diff.x = diff.z = 1;
                if( pEditorState->m_EditorActionState == EditorActionState::ScaleZ )
                    diff.x = diff.y = 1;

                // GIZMOSCALE: scale all of the things. // undo is handled by EngineCore.cpp when mouse is lifted.
                pEditorState->m_AmountScaled = pEditorState->m_AmountScaled.MultiplyComponents( diff );
                //LOGInfo( "Scale Gizmo", "pEditorState->m_AmountScaled.Set( %f, %f, %f ); ", pEditorState->m_AmountScaled.x, pEditorState->m_AmountScaled.y, pEditorState->m_AmountScaled.z );
                //LOGInfo( "Scale Gizmo", "diff( %f, %f, %f, %d );\n", diff.x, diff.y, diff.z, pEditorState->m_pSelectedObjects.size() );

                ScaleSelectedObjects( pEditorState, diff );
            }
        }
    }
}

void TransformGizmo::ScaleSelectedObjects(EditorState* pEditorState, Vector3 scale)
{
    for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
    {
        ComponentTransform* pTransform = pEditorState->m_pSelectedObjects[i]->GetTransform();

        // if this object has a selected parent, don't move it, only move the parent.
        if( pTransform && pTransform->IsAnyParentInList( pEditorState->m_pSelectedObjects ) == false )
        {
            // only scale in local space
            //if( pEditorState->m_TransformedInLocalSpace ) // local space
            {
                Vector3 newscale = pTransform->GetLocalTransform()->GetScale() * scale;

                pTransform->SetScaleByEditor( newscale );
                pTransform->UpdateTransform();
            }
            //else
            //{
            //    MyMatrix matscale;
            //    matscale.CreateScale( scale );
            //    pTransform->Scale( &matscale, pEditorState->m_WorldSpacePivot );
            //}
        }
    }
}

void TransformGizmo::CancelLastScale(EditorState* pEditorState)
{
    ScaleSelectedObjects( pEditorState, Vector3( 1 / pEditorState->m_AmountScaled.x, 1 / pEditorState->m_AmountScaled.y, 1 / pEditorState->m_AmountScaled.z ) );
}

void TransformGizmo::RotateSelectedObjects(EngineCore* pGame, EditorState* pEditorState)
{
    if( pEditorState->m_pSelectedObjects.size() == 0 )
        return;

    // rotate the selected objects along a plane or axis
    if( pEditorState->m_EditorActionState == EditorActionState::RotateX ||
        pEditorState->m_EditorActionState == EditorActionState::RotateY ||
        pEditorState->m_EditorActionState == EditorActionState::RotateZ )
    {
        // rotate all selected objects by the same amount, use object 0 to create a plane.
        {
            Vector3 eulerdegrees = m_GizmoWorldRotation;
            MyMatrix objectRotation;
            objectRotation.CreateRotation( eulerdegrees );

            // create a plane based on the axis we want.
            Plane plane;
            {
                ComponentCamera* pCamera = pEditorState->GetEditorCamera();
                Vector3 camInvAt = pCamera->GetGameObject()->GetTransform()->GetLocalTransform()->GetAt() * -1;

                Vector3 normal;
                if( pEditorState->m_EditorActionState == EditorActionState::RotateX )
                {
                    normal = Vector3(1,0,0); // yz plane
                }
                else if( pEditorState->m_EditorActionState == EditorActionState::RotateY )
                {
                    normal = Vector3(0,1,0); // xz plane
                }
                else if( pEditorState->m_EditorActionState == EditorActionState::RotateZ )
                {
                    normal = Vector3(0,0,1); // xy plane
                }

                if( pEditorState->m_TransformedInLocalSpace ) // local space
                {
                    // bring normal into world space
                    normal = objectRotation * normal;
                    pEditorState->m_WorldSpacePivot.Set( 0, 0, 0 );
                }
                else
                {
                    // set the pivot to match the current gizmo.
                    Vector3 gizmopos = m_pTranslate1Axis[0]->GetTransform()->GetWorldPosition();
                    pEditorState->m_WorldSpacePivot = gizmopos;
                }

                //LOGInfo( "TransformGizmo", "normal( %f, %f, %f );\n", normal.x, normal.y, normal.z );

                // create a world space plane.
                plane.Set( normal, m_GizmoWorldTransform.GetTranslation() );
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
                Vector3 pos = m_GizmoWorldTransform.GetTranslation();

                currentresult -= pos;
                lastresult -= pos;

                if( pEditorState->m_TransformedInLocalSpace ) // local space
                {
                    // bring results from world space into local space.
                    MyMatrix invRotation = objectRotation.GetInverse();

                    currentresult = invRotation * currentresult;
                    lastresult = invRotation * lastresult;
                }

                //LOGInfo( LOGTag, "currentresult( %f, %f, %f );", currentresult.x, currentresult.y, currentresult.z );
                //LOGInfo( LOGTag, "lastresult( %f, %f, %f );\n", lastresult.x, lastresult.y, lastresult.z );
                //LOGInfo( LOGTag, "axisvector( %f, %f, %f );\n", axisvector.x, axisvector.y, axisvector.z );

                Vector3 currentangle;
                Vector3 lastangle;

                currentangle.x = atan2f( currentresult.z, currentresult.y );
                currentangle.y = atan2f( currentresult.x, currentresult.z );
                currentangle.z = atan2f( currentresult.y, currentresult.x );

                lastangle.x = atan2f( lastresult.z, lastresult.y );
                lastangle.y = atan2f( lastresult.x, lastresult.z );
                lastangle.z = atan2f( lastresult.y, lastresult.x );

                // lock to one of the 3 axis.
                if( pEditorState->m_EditorActionState == EditorActionState::RotateX )
                {
                    currentangle.y = currentangle.z = 0;
                    lastangle.y = lastangle.z = 0;
                    //LOGInfo( LOGTag, "angles( %f -> %f );", currentangle.x * 180 / PI, lastangle.x * 180 / PI );
                }
                if( pEditorState->m_EditorActionState == EditorActionState::RotateY )
                {
                    currentangle.x = currentangle.z = 0;
                    lastangle.x = lastangle.z = 0;
                }
                if( pEditorState->m_EditorActionState == EditorActionState::RotateZ )
                {
                    currentangle.x = currentangle.y = 0;
                    lastangle.x = lastangle.y = 0;
                    //LOGInfo( LOGTag, "angles( %f -> %f );", currentangle.z * 180 / PI, lastangle.z * 180 / PI );
                }

                // find the diff pos between this frame and last.
                Vector3 diff = (lastangle - currentangle) * 180 / PI;

                // GIZMOROTATE: rotate all of the things. // undo is handled by EngineCore.cpp when mouse is lifted.
                pEditorState->m_DistanceRotated += diff;
                //LOGInfo( "Rotate Gizmo", "pEditorState->m_DistanceRotated.Set( %f, %f, %f ); ", pEditorState->m_DistanceRotated.x, pEditorState->m_DistanceRotated.y, pEditorState->m_DistanceRotated.z );
                //LOGInfo( "Rotate Gizmo", "diff( %f, %f, %f, %d );\n", diff.x, diff.y, diff.z, pEditorState->m_pSelectedObjects.size() );

                RotateSelectedObjects( pEditorState, diff );
            }
        }
    }
}

void TransformGizmo::RotateSelectedObjects(EditorState* pEditorState, Vector3 eulerdegrees)
{
    for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
    {
        ComponentTransform* pTransform = pEditorState->m_pSelectedObjects[i]->GetTransform();

        // if this object has a selected parent, don't move it, only move the parent.
        if( pTransform && pTransform->IsAnyParentInList( pEditorState->m_pSelectedObjects ) == false )
        {
            if( pEditorState->m_TransformedInLocalSpace ) // local space
            {
                MyMatrix objectRotation;
                MyMatrix newRotation;
                MyMatrix combinedRotation;

                objectRotation.CreateRotation( pTransform->GetWorldRotation() );
                newRotation.CreateRotation( eulerdegrees );
                combinedRotation = objectRotation * newRotation;

                Vector3 neweulerdegrees = combinedRotation.GetEulerAngles() * 180.0f / PI;
                pTransform->SetWorldRotation( neweulerdegrees );
                pTransform->UpdateTransform();
            }
            else
            {
                MyMatrix newRotation;
                newRotation.CreateRotation( eulerdegrees );
                pTransform->Rotate( &newRotation, pEditorState->m_WorldSpacePivot );
            }
        }
    }
}

void TransformGizmo::CancelLastRotation(EditorState* pEditorState)
{
    RotateSelectedObjects( pEditorState, pEditorState->m_DistanceRotated * -1 );
}
