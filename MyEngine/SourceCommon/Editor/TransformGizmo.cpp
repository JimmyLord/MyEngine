//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
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
    m_SelectedPart = -1;

    for( int i=0; i<3; i++ )
        m_pTransformGizmos[i] = 0;

    m_LastIntersectResultIsValid = false;
    m_LastIntersectResultUsed.Set(0,0,0);
}

TransformGizmo::~TransformGizmo()
{
    for( int i=0; i<3; i++ )
    {
        SAFE_DELETE( m_pTransformGizmos[i] );
    }
}

void TransformGizmo::Tick(double TimePassed, EditorState* pEditorState)
{
    // Update transform gizmos
    for( int i=0; i<3; i++ )
    {
        MyAssert( m_pTransformGizmos[i] );

        ComponentRenderable* pRenderable = (ComponentRenderable*)m_pTransformGizmos[i]->GetFirstComponentOfBaseType( BaseComponentType_Renderable );

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

                if( i == m_SelectedPart )
                {
                    pMaterial->m_ColorDiffuse.Set( 255,255,255,255 );
                }
            }
        }

        Vector3 ObjectPosition;
        MyMatrix ObjectTransform;

        if( pEditorState->m_pSelectedObjects.size() == 1 )
        {
            pRenderable->m_Visible = true;
            ObjectPosition = pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->GetPosition();
            ObjectTransform = *pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->GetLocalTransform();
        }
        else if( pEditorState->m_pSelectedObjects.size() > 1 )
        {
            pRenderable->m_Visible = true;

            // find the center point between all selected objects.
            ObjectPosition.Set( 0, 0, 0 );
            for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
            {
                ObjectPosition += pEditorState->m_pSelectedObjects[i]->m_pComponentTransform->GetPosition();
            }
            ObjectPosition /= (float)pEditorState->m_pSelectedObjects.size();

            ObjectTransform.SetIdentity();
        }
        else
        {
            pRenderable->m_Visible = false;
        }

        if( pRenderable->m_Visible )
        {
            // move the gizmo to the object position.
            m_pTransformGizmos[i]->m_pComponentTransform->SetPosition( ObjectPosition );

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

            m_pTransformGizmos[i]->m_pComponentTransform->SetRotation( rot );

            float distance = (pEditorState->m_pEditorCamera->m_pComponentTransform->GetPosition() - ObjectPosition).Length();
            m_pTransformGizmos[i]->m_pComponentTransform->SetScale( Vector3( distance / 15.0f ) );
        }
    }
}

bool TransformGizmo::HandleInput(EngineCore* pGame, int keydown, int keycode, int action, int id, float x, float y, float pressure)
{
    MyAssert( x >= 0 && y >= 0 );
    if( x < 0 || y < 0 )
        return false;

    // find the object we're hovering on
    GameObject* pObject = pGame->GetObjectAtPixel( (unsigned int)x, (unsigned int)y, true );

    m_SelectedPart = -1;

    if( pObject == m_pTransformGizmos[0] )
        m_SelectedPart = 0;
    if( pObject == m_pTransformGizmos[1] )
        m_SelectedPart = 1;
    if( pObject == m_pTransformGizmos[2] )
        m_SelectedPart = 2;

    return false;
}

void TransformGizmo::CreateAxisObjects(unsigned int sceneid, float scale, MaterialDefinition* pMaterialX, MaterialDefinition* pMaterialY, MaterialDefinition* pMaterialZ, EditorState* pEditorState)
{
    GameObject* pGameObject;
    ComponentMesh* pComponentMesh;

    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false ); // not managed.
        pGameObject->SetSceneID( sceneid );
        pGameObject->SetName( "3D Transform Gizmo - x-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = true;
            pComponentMesh->SetMaterial( pMaterialX, 0 );
            pComponentMesh->m_LayersThisExistsOn = Layer_EditorFG;
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.05f, ColorByte(255, 100, 100, 255) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
        }

        pEditorState->m_pTransformGizmo->m_pTransformGizmos[0] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false ); // not managed.
        pGameObject->SetSceneID( sceneid );
        pGameObject->SetName( "3D Transform Gizmo - y-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = true;
            pComponentMesh->SetMaterial( pMaterialY, 0 );
            pComponentMesh->m_LayersThisExistsOn = Layer_EditorFG;
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.05f, ColorByte(100, 255, 100, 255) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
        }

        pEditorState->m_pTransformGizmo->m_pTransformGizmos[1] = pGameObject;
    }
    {
        pGameObject = g_pComponentSystemManager->CreateGameObject( false ); // not managed.
        pGameObject->SetSceneID( sceneid );
        pGameObject->SetName( "3D Transform Gizmo - z-axis" );

        pComponentMesh = (ComponentMesh*)pGameObject->AddNewComponent( ComponentType_Mesh, sceneid );
        if( pComponentMesh )
        {
            pComponentMesh->m_Visible = true;
            pComponentMesh->SetMaterial( pMaterialZ, 0 );
            pComponentMesh->m_LayersThisExistsOn = Layer_EditorFG;
            pComponentMesh->m_pMesh = MyNew MyMesh();
            pComponentMesh->m_pMesh->CreateEditorTransformGizmoAxis( 3, 0.05f, ColorByte(100, 100, 255, 255) );
            pComponentMesh->m_GLPrimitiveType = pComponentMesh->m_pMesh->m_SubmeshList[0]->m_PrimitiveType;
        }

        pEditorState->m_pTransformGizmo->m_pTransformGizmos[2] = pGameObject;
    }
}

void TransformGizmo::ScaleGizmosForMousePickRendering(bool doscale)
{
    float scaleamount = 7;

    if( doscale == false )
        scaleamount = 1/scaleamount;

    for( int i=0; i<3; i++ )
    {
        Vector3 currentscale = m_pTransformGizmos[i]->m_pComponentTransform->GetLocalScale();
        Vector3 newscale( currentscale.x * scaleamount, currentscale.y, currentscale.z * scaleamount );
        m_pTransformGizmos[i]->m_pComponentTransform->SetScale( newscale );
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

            //ComponentCamera* pCamera = pEditorState->GetEditorCamera();
            MyMatrix* pObjectTransform = &pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->m_Transform;

            // create a plane based on the axis we want.
            Vector3 axisvector;
            Plane plane;
            {
                Vector3 normal;
                if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateXY ||
                    pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateX )
                {
                    normal = Vector3(0,0,1);
                    axisvector = Vector3(1,0,0);
                }
                else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateXZ ||
                         pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateZ )
                {
                    normal = Vector3(0,1,0);
                    axisvector = Vector3(0,0,1);
                }
                else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateYZ ||
                         pEditorState->m_EditorActionState == EDITORACTIONSTATE_TranslateY )
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
                //LOGInfo( LOGTag, "currentresult( %f, %f, %f );", currentresult.x, currentresult.y, currentresult.z );
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
                    Vector3 pos = pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->GetPosition();

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

                for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
                {
                    ComponentTransform* pTransform = pEditorState->m_pSelectedObjects[i]->m_pComponentTransform;

                    // if this object has a selected parent, don't move it, only move the parent.
                    if( pTransform->IsAnyParentInList( pEditorState->m_pSelectedObjects ) == false )
                    {
                        Vector3 pos = pTransform->GetLocalTransform()->GetTranslation();

                        pTransform->SetPositionByEditor( pos + diff );
                        pTransform->UpdateMatrix();
                    }
                }
            }
        }
    }
}
