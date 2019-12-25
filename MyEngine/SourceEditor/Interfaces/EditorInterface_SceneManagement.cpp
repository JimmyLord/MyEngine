//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorInterface_SceneManagement.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/FrameworkComponents/ComponentAnimationPlayer.h"
#include "ComponentSystem/FrameworkComponents/ComponentCameraShadow.h"
#include "Core/EngineCore.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/TransformGizmo.h"
#include "../SourceEditor/Commands/EngineEditorCommands.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"

EditorInterface_SceneManagement::EditorInterface_SceneManagement(EngineCore* pEngineCore)
: EditorInterface( pEngineCore )
{
}

EditorInterface_SceneManagement::~EditorInterface_SceneManagement()
{
}

void EditorInterface_SceneManagement::Initialize()
{
}

void EditorInterface_SceneManagement::OnActivated()
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();
    pEditorState->m_pTransformGizmo->m_VisibleIfObjectsSelected = true;
}

void EditorInterface_SceneManagement::OnDeactivated()
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();
    pEditorState->m_pTransformGizmo->m_VisibleIfObjectsSelected = false;
}

void EditorInterface_SceneManagement::OnDrawFrame(unsigned int canvasid)
{
    MyAssert( canvasid == 1 );

    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    // EditorInterface class will draw the main editor view.
    EditorInterface::OnDrawFrame( canvasid );

    // Get/Create a debug quad sprite.
    MySprite* debugQuad = g_pEngineCore->GetSprite_DebugQuad();

    // Draw a single selected mesh as an overlay on the editor window.
    // TODO: Deal with this in it's own window on ImGui editor builds.
    if( g_pEngineCore->GetDebug_DrawSelectedAnimatedMesh() && g_GLCanvasIDActive == 1 )
    {
        if( pEditorState->m_pSelectedObjects.size() > 0 )
        {
            // TODO: Have the file selecter pick the right game object/mesh.
            GameObject* pObject = pEditorState->m_pSelectedObjects[0];

            // If this has an animation player, render the current animation.
            ComponentAnimationPlayer* pAnim = pObject->GetAnimationPlayer();
            if( pAnim )
            {
                int backupindex = pAnim->m_AnimationIndex;
                float backuptime = pAnim->m_AnimationTime;

                //pAnim->m_AnimationIndex = 0;
                pAnim->m_AnimationTime = (float)MyTime_GetUnpausedTime();
                pAnim->Tick( 0 );

                g_pEngineCore->RenderSingleObject( pObject );

                pAnim->m_AnimationIndex = backupindex;
                pAnim->m_AnimationTime = backuptime;

                float maxu = (float)pEditorState->m_pDebugViewFBO->GetWidth() / pEditorState->m_pDebugViewFBO->GetTextureWidth();
                float maxv = (float)pEditorState->m_pDebugViewFBO->GetHeight() / pEditorState->m_pDebugViewFBO->GetTextureHeight();

                BufferManager* pBufferManager = m_pEngineCore->GetManagers()->GetBufferManager();
                debugQuad->CreateInPlace( pBufferManager, "debug", 0.5f, 0.5f, 1.0f, 1.0f, 0, maxu, maxv, 0, Justify_Center, false );
                g_pEngineCore->GetMaterial_ClipSpaceTexture()->SetTextureColor( pEditorState->m_pDebugViewFBO->GetColorTexture( 0 ) );
                debugQuad->SetMaterial( g_pEngineCore->GetMaterial_ClipSpaceTexture() );
                debugQuad->Draw( 0, 0, 0 );
            }

            // If it's a shadow cam, render the depth texture.
            ComponentCameraShadow* pCamera = dynamic_cast<ComponentCameraShadow*>( pObject->GetFirstComponentOfBaseType( BaseComponentType_Camera ) );
            if( pCamera )
            {
                BufferManager* pBufferManager = m_pEngineCore->GetManagers()->GetBufferManager();
                debugQuad->CreateInPlace( pBufferManager, "debug", 0.5f, 0.5f, 1.0f, 1.0f, 0, 1, 1, 0, Justify_Center, false );
                g_pEngineCore->GetMaterial_ClipSpaceTexture()->SetTextureColor( pCamera->GetFBO()->GetDepthTexture() );
                debugQuad->SetMaterial( g_pEngineCore->GetMaterial_ClipSpaceTexture() );
                debugQuad->Draw( 0, 0, 0 );
            }
        }
    }

    // Draw group select rectangle.
    if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_GroupSelectingObjects )
    {
        Vector2 windowSize( (float)pEditorState->m_EditorWindowRect.w, (float)pEditorState->m_EditorWindowRect.h );
        Vector2 clipStart = pEditorState->m_MouseDownLocation[0] / windowSize * 2 - 1;
        Vector2 clipEnd = pEditorState->m_CurrentMousePosition / windowSize * 2 - 1;
        
        Vector2 clipTopLeft;
        clipTopLeft.x = clipStart.x < clipEnd.x ? clipStart.x : clipEnd.x;
        clipTopLeft.y = clipStart.y > clipEnd.y ? clipStart.y : clipEnd.y;
        
        Vector2 clipSize;
        clipSize.x = fabsf( clipStart.x - clipEnd.x );
        clipSize.y = fabsf( clipStart.y - clipEnd.y );

        if( clipSize.x > 0 || clipSize.y > 0 )
        {
            BufferManager* pBufferManager = m_pEngineCore->GetManagers()->GetBufferManager();
            debugQuad->CreateInPlace( pBufferManager, "debug", clipTopLeft.x, clipTopLeft.y, clipSize.x, clipSize.y, 0, 1, 0, 1, Justify_TopLeft, false );
            MaterialDefinition* pMaterial = g_pEngineCore->GetMaterial_ClipSpaceColor();
            pMaterial->SetColorDiffuse( ColorByte( 0, 255, 0, 64 ) );
            debugQuad->SetMaterial( pMaterial );
            debugQuad->Draw( 0, 0, 0 );
        }
    }

    // Draw Box2D and Bullet debug data.
    if( g_pEngineCore->GetEditorPrefs()->Get_Debug_DrawPhysicsDebugShapes() && g_GLCanvasIDActive == 1 )
    {
        for( int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
        {
#if MYFW_USING_BOX2D
            if( g_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse && g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld )
                g_pComponentSystemManager->m_pSceneInfoMap[i].m_pBox2DWorld->m_pWorld->DrawDebugData();
#endif
        }

#if MYFW_USING_BULLET
        if( g_pEngineCore->GetBulletWorld() )
        {
            g_pEngineCore->GetBulletWorld()->m_pDynamicsWorld->debugDrawWorld();
        }
#endif //MYFW_USING_BULLET
    }
}

bool EditorInterface_SceneManagement::CancelCurrentOperation(bool allowClearOfSelectedObjects)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    // Cancel the current action and return true to prevent click from starting a camera rotation (or other op).
    if( pEditorState->m_EditorActionState >= EDITORACTIONSTATE_TranslateX &&
        pEditorState->m_EditorActionState <= EDITORACTIONSTATE_RotateZ )
    {
        pEditorState->m_pTransformGizmo->CancelCurrentOperation( pEditorState );
        pEditorState->m_EditorActionState = EDITORACTIONSTATE_None;
        return true;
    }
    else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_GroupSelectingObjects )
    {
        pEditorState->m_pTransformGizmo->CancelCurrentOperation( pEditorState );
        pEditorState->m_EditorActionState = EDITORACTIONSTATE_None;
        return true;
    }

    if( allowClearOfSelectedObjects )
    {
        pEditorState->ClearSelectedObjectsAndComponents();
    }

    pEditorState->ClearConstraint();
    
    return false;
}

bool EditorInterface_SceneManagement::HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    // If this is a new right-click, cancel gizmo ops.
    if( mouseaction == GCBA_Down && id == 1 )
    {
        if( CancelCurrentOperation( false ) == true )
            return true;
    }

    if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
    {
        if( mouseaction == GCBA_Down && id == 0 )
        {
            // find the object we clicked on.
            GameObject* pObject = GetObjectAtPixel( (unsigned int)x, (unsigned int)y, true, true );

            // reset current transform values, so we can undo by this amount after mouse goes up.
            pEditorState->m_DistanceTranslated.Set( 0, 0, 0 );
            pEditorState->m_AmountScaled.Set( 1, 1, 1 );
            pEditorState->m_DistanceRotated.Set( 0, 0, 0 );
            //LOGInfo( LOGTag, "pEditorState->m_DistanceTranslated.Set zero( %f, %f, %f );\n", pEditorState->m_DistanceTranslated.x, pEditorState->m_DistanceTranslated.y, pEditorState->m_DistanceTranslated.z );

            bool selectedgizmo = false;

            // translate on one axis.
            if( pObject == pEditorState->m_pTransformGizmo->m_pTranslate1Axis[0] )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_TranslateX;
                selectedgizmo = true;
            }
            if( pObject == pEditorState->m_pTransformGizmo->m_pTranslate1Axis[1] )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_TranslateY;
                selectedgizmo = true;
            }
            if( pObject == pEditorState->m_pTransformGizmo->m_pTranslate1Axis[2] )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_TranslateZ;
                selectedgizmo = true;
            }

            // translate on two axes.
            if( pObject == pEditorState->m_pTransformGizmo->m_pTranslate2Axis[0] )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_TranslateXY;
                selectedgizmo = true;
            }
            if( pObject == pEditorState->m_pTransformGizmo->m_pTranslate2Axis[1] )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_TranslateXZ;
                selectedgizmo = true;
            }
            if( pObject == pEditorState->m_pTransformGizmo->m_pTranslate2Axis[2] )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_TranslateYZ;
                selectedgizmo = true;
            }

            // scale.
            if( pObject == pEditorState->m_pTransformGizmo->m_pScale1Axis[0] )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_ScaleX;
                selectedgizmo = true;
            }
            if( pObject == pEditorState->m_pTransformGizmo->m_pScale1Axis[1] )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_ScaleY;
                selectedgizmo = true;
            }
            if( pObject == pEditorState->m_pTransformGizmo->m_pScale1Axis[2] )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_ScaleZ;
                selectedgizmo = true;
            }
            if( pObject == pEditorState->m_pTransformGizmo->m_pScale3Axis )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_ScaleXYZ;
                selectedgizmo = true;
            }

            // rotate.
            if( pObject == pEditorState->m_pTransformGizmo->m_pRotate1Axis[0] )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_RotateX;
                selectedgizmo = true;
            }
            if( pObject == pEditorState->m_pTransformGizmo->m_pRotate1Axis[1] )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_RotateY;
                selectedgizmo = true;
            }
            if( pObject == pEditorState->m_pTransformGizmo->m_pRotate1Axis[2] )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_RotateZ;
                selectedgizmo = true;
            }

            if( selectedgizmo == false )
            {
                // If we didn't select the transform gizmo, we're likely selecting objects.
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_GroupSelectingObjects;

                // If neither shift nor control are held, clear the selected objects.
                if( (pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift) == 0 &&
                    (pEditorState->m_ModifierKeyStates & MODIFIERKEY_Control) == 0 )
                {
                    pEditorState->ClearSelectedObjectsAndComponents();
                }
            }

            // If shift is held, make a copy of the object and control that one.
            if( selectedgizmo && pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
            {
                // Make a copy of the selected objects and clear what the editor thinks is selected.
                std::vector<GameObject*> selectedobjects = pEditorState->m_pSelectedObjects;
                pEditorState->ClearSelectedObjectsAndComponents();

#if MYFW_USING_WX
                g_pPanelObjectList->Freeze();
#endif

                for( unsigned int i=0; i<selectedobjects.size(); i++ )
                {
                    GameObject* pNewObject = g_pComponentSystemManager->EditorCopyGameObject( selectedobjects[i], false );
                    if( g_pEngineCore->IsInEditorMode() )
                    {
                        pNewObject->SetSceneID( selectedobjects[i]->GetSceneID() );
                    }

                    pEditorState->m_pSelectedObjects.push_back( pNewObject );
                    // select the object in the object tree.
#if MYFW_USING_WX
                    g_pPanelObjectList->SelectObject( pNewObject );
#endif
                }

#if MYFW_USING_WX
                g_pPanelObjectList->Thaw();
#endif
            }

            // if ctrl is held, transform in world space
            if( selectedgizmo )
            {
                if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_Control )
                    pEditorState->m_TransformedInLocalSpace = false;
                else
                    pEditorState->m_TransformedInLocalSpace = true;
            }

            // if we didn't select the gizmo and gameplay is running.
            if( selectedgizmo == false && g_pEngineCore->IsInEditorMode() == false )
            {
#if MYFW_USING_BULLET
                // check if we selected a physics object then grab it.
                //if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
                {
                    // Get the mouse click ray.
                    Vector3 raystart, rayend;
                    g_pEngineCore->GetMouseRay( pEditorState->m_CurrentMousePosition, &raystart, &rayend );

                    btVector3 RayStart( raystart.x, raystart.y, raystart.z );
                    btVector3 RayEnd( rayend.x, rayend.y, rayend.z );

                    btCollisionWorld::ClosestRayResultCallback rayCallback( RayStart, RayEnd );
                    g_pBulletWorld->m_pDynamicsWorld->rayTest( RayStart, RayEnd, rayCallback );
                    if( rayCallback.hasHit() )
                    {
                        btVector3 pickPos = rayCallback.m_hitPointWorld;

                        pEditorState->m_MousePicker_PickedBody = 0;

                        //pickObject( pickPos, rayCallback.m_collisionObject );
                        btRigidBody* body = (btRigidBody*)btRigidBody::upcast( rayCallback.m_collisionObject );
                        if( body )
                        {
                            // other exclusions?
                            if( !(body->isStaticObject() || body->isKinematicObject()) )
                            {
                                pEditorState->m_MousePicker_PickedBody = body;
                                pEditorState->m_MousePicker_PickedBody->setActivationState( DISABLE_DEACTIVATION );

                                //printf("pickPos=%f,%f,%f\n",pickPos.getX(),pickPos.getY(),pickPos.getZ());

                                btVector3 localPivot = body->getCenterOfMassTransform().inverse() * pickPos;

                                if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift ) //(m_modifierKeys & BT_ACTIVE_SHIFT) != 0 )
                                {
                                    btTransform tr;
                                    tr.setIdentity();
                                    tr.setOrigin(localPivot);
                                    btGeneric6DofConstraint* dof6 = new btGeneric6DofConstraint(*body, tr,false);
                                    dof6->setLinearLowerLimit( btVector3(0,0,0) );
                                    dof6->setLinearUpperLimit( btVector3(0,0,0) );
                                    dof6->setAngularLowerLimit( btVector3(0,0,0) );
                                    dof6->setAngularUpperLimit( btVector3(0,0,0) );

                                    g_pBulletWorld->m_pDynamicsWorld->addConstraint(dof6,true);
                                    pEditorState->m_MousePicker_PickConstraint = dof6;

                                    dof6->setParam( BT_CONSTRAINT_STOP_CFM, 0.8f, 0 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_CFM, 0.8f, 1 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_CFM, 0.8f, 2 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_CFM, 0.8f, 3 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_CFM, 0.8f, 4 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_CFM, 0.8f, 5 );

                                    dof6->setParam( BT_CONSTRAINT_STOP_ERP, 0.1f, 0 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_ERP, 0.1f, 1 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_ERP, 0.1f, 2 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_ERP, 0.1f, 3 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_ERP, 0.1f, 4 );
                                    dof6->setParam( BT_CONSTRAINT_STOP_ERP, 0.1f, 5 );
                                }
                                else
                                {
                                    btPoint2PointConstraint* p2p = new btPoint2PointConstraint(*body,localPivot);
                                    g_pBulletWorld->m_pDynamicsWorld->addConstraint(p2p,true);
                                    pEditorState->m_MousePicker_PickConstraint = p2p;
                                    btScalar mousePickClamping = 30.f;
                                    p2p->m_setting.m_impulseClamp = mousePickClamping;
                                    //very weak constraint for picking
                                    p2p->m_setting.m_tau = 0.001f;
                                    /*
                                    p2p->setParam(BT_CONSTRAINT_CFM,0.8,0);
                                    p2p->setParam(BT_CONSTRAINT_CFM,0.8,1);
                                    p2p->setParam(BT_CONSTRAINT_CFM,0.8,2);
                                    p2p->setParam(BT_CONSTRAINT_ERP,0.1,0);
                                    p2p->setParam(BT_CONSTRAINT_ERP,0.1,1);
                                    p2p->setParam(BT_CONSTRAINT_ERP,0.1,2);
                                    */
                                }

                                //save mouse position for dragging
                            }
                        }

                        //gOldPickingPos = RayEnd;
                        //gHitPos = pickPos;

                        pEditorState->m_MousePicker_OldPickingDist = (pickPos-RayStart).length();
                        mouseaction = -1;
                    }
                }
#endif //MYFW_USING_BULLET
            }
        }

        if( mouseaction == GCBA_Held && id == 0 ) //id == 1 << 0 )
        {
#if MYFW_USING_BULLET
            // gameplay is running and we picked up a physics object in the editor view, so move it around.
            if( pEditorState->m_MousePicker_PickConstraint && g_pBulletWorld->m_pDynamicsWorld )
            {
                // if we clicked and moved while on a 3d physics body, cancel the EDITORACTIONSTATE_GroupSelectingObjects state
                if( pEditorState->HasMouseMovedSinceButtonPressed( 0 ) )
                    pEditorState->m_EditorActionState = EDITORACTIONSTATE_None;

                // move the constraint pivot
                if( pEditorState->m_MousePicker_PickConstraint->getConstraintType() == D6_CONSTRAINT_TYPE )
                {
                    btGeneric6DofConstraint* pickCon = static_cast<btGeneric6DofConstraint*>( pEditorState->m_MousePicker_PickConstraint );
                    if( pickCon )
                    {
                        //keep it at the same picking distance

                        // Get the mouse click ray.
                        Vector3 raystart, rayend;
                        g_pEngineCore->GetMouseRay( pEditorState->m_CurrentMousePosition, &raystart, &rayend );

                        btVector3 newRayTo( rayend.x, rayend.y, rayend.z );
                        btVector3 rayFrom;
                        btVector3 oldPivotInB = pickCon->getFrameOffsetA().getOrigin();

                        btVector3 newPivotB;
                        //if( m_ortho )
                        //{
                        //    newPivotB = oldPivotInB;
                        //    newPivotB.setX(newRayTo.getX());
                        //    newPivotB.setY(newRayTo.getY());
                        //}
                        //else
                        {
                            rayFrom = btVector3( raystart.x, raystart.y, raystart.z );
                            btVector3 dir = newRayTo - rayFrom;
                            dir.normalize();
                            dir *= pEditorState->m_MousePicker_OldPickingDist;

                            newPivotB = rayFrom + dir;
                        }

                        pickCon->getFrameOffsetA().setOrigin(newPivotB);
                    }
                }
                else
                {
                    btPoint2PointConstraint* pickCon = static_cast<btPoint2PointConstraint*>( pEditorState->m_MousePicker_PickConstraint );
                    if (pickCon)
                    {
                        //keep it at the same picking distance

                        // Get the mouse click ray.
                        Vector3 raystart, rayend;
                        g_pEngineCore->GetMouseRay( pEditorState->m_CurrentMousePosition, &raystart, &rayend );

                        btVector3 newRayTo( rayend.x, rayend.y, rayend.z );
                        btVector3 rayFrom;
                        btVector3 oldPivotInB = pickCon->getPivotInB();
                        btVector3 newPivotB;

                        //if( m_ortho )
                        //{
                        //    newPivotB = oldPivotInB;
                        //    newPivotB.setX(newRayTo.getX());
                        //    newPivotB.setY(newRayTo.getY());
                        //}
                        //else
                        {
                            rayFrom = btVector3( raystart.x, raystart.y, raystart.z );
                            btVector3 dir = newRayTo - rayFrom;
                            dir.normalize();
                            dir *= pEditorState->m_MousePicker_OldPickingDist;

                            newPivotB = rayFrom + dir;
                        }

                        pickCon->setPivotB(newPivotB);
                    }
                }

                //float dx, dy;
                //dx = btScalar(x) - m_mouseOldX;
                //dy = btScalar(y) - m_mouseOldY;


                ///only if ALT key is pressed (Maya style)
                //if (m_modifierKeys& BT_ACTIVE_ALT)
                //{
                //    if(m_mouseButtons & 2)
                //    {
                //        btVector3 hor = getRayTo(0,0)-getRayTo(1,0);
                //        btVector3 vert = getRayTo(0,0)-getRayTo(0,1);
                //        btScalar multiplierX = btScalar(0.001);
                //        btScalar multiplierY = btScalar(0.001);
                //        if (m_ortho)
                //        {
                //            multiplierX = 1;
                //            multiplierY = 1;
                //        }


                //        m_cameraTargetPosition += hor* dx * multiplierX;
                //        m_cameraTargetPosition += vert* dy * multiplierY;
                //    }

                //    if(m_mouseButtons & (2 << 2) && m_mouseButtons & 1)
                //    {
                //    }
                //    else if(m_mouseButtons & 1)
                //    {
                //        m_azi += dx * btScalar(0.2);
                //        m_azi = fmodf(m_azi, btScalar(360.f));
                //        m_ele += dy * btScalar(0.2);
                //        m_ele = fmodf(m_ele, btScalar(180.f));
                //    }
                //    else if(m_mouseButtons & 4)
                //    {
                //        m_cameraDistance -= dy * btScalar(0.02f);
                //        if (m_cameraDistance<btScalar(0.1))
                //            m_cameraDistance = btScalar(0.1);


                //    }
                //}


                //m_mouseOldX = x;
                //m_mouseOldY = y;
                //updateCamera();

                mouseaction = -1;
            }
#endif //MYFW_USING_BULLET
        }

        if( mouseaction == GCBA_Up && id == 0 )
        {
            pEditorState->ClearConstraint();
        }

        // If the mouse moved while a button is down.
        if( mouseaction == GCBA_Held || mouseaction == GCBA_RelativeMovement )
        {
            Vector3 mousedragdir = pEditorState->m_CurrentMousePosition - pEditorState->m_LastMousePosition;

            if( mousedragdir.LengthSquared() != 0 )
            {
                // If the mouse moved, translate/scale/rotate the selected objects along a plane or axis
                // the checks for which editor tool is active is inside these functions.
                pEditorState->m_pTransformGizmo->TranslateSelectedObjects( g_pEngineCore, pEditorState );
                pEditorState->m_pTransformGizmo->ScaleSelectedObjects( g_pEngineCore, pEditorState );
                pEditorState->m_pTransformGizmo->RotateSelectedObjects( g_pEngineCore, pEditorState );
            }
        }
    }

    // handle camera movement, with both mouse and keyboard.
    EditorInterface::HandleInputForEditorCamera( keyaction, keycode, mouseaction, id, x, y, pressure );

    if( keyaction == GCBA_Up )
    {
        // If there's a current op, cancel it, otherwise clear selected objects.
        if( keycode == MYKEYCODE_ESC )
        {
            if( CancelCurrentOperation( true ) == true )
                return true;
        }

        // Lock to current object, 
        if( keycode == 'L' )
        {
            if( pEditorState->m_CameraState == EditorCameraState_Default && pEditorState->m_pSelectedObjects.size() > 0 )
            {
                GameObject* pSelectedObject = pEditorState->m_pSelectedObjects[0];
                pEditorState->LockCameraToGameObject( pSelectedObject );
            }
            else
            {
                pEditorState->LockCameraToGameObject( 0 );
            }
            return true;
        }

        if( keycode == MYKEYCODE_DELETE )
        {
            // delete the current selected gameobjects.
            pEditorState->DeleteSelectedObjects();
            return true;
        }
    }

    // check for mouse ups
    if( mouseaction != -1 )
    {
        if( mouseaction == GCBA_Up )
        {
            if( id == 0 && pEditorState->m_EditorActionState != EDITORACTIONSTATE_None ) // left button up
            {
                // when mouse up, select all object in the box.
                if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_GroupSelectingObjects )
                {
                    SelectObjectsInRectangle(
                        (unsigned int)pEditorState->m_MouseDownLocation[0].x, (unsigned int)pEditorState->m_MouseDownLocation[0].y,
                        (unsigned int)pEditorState->m_CurrentMousePosition.x, (unsigned int)pEditorState->m_CurrentMousePosition.y );
                }

                // GIZMOTRANSLATE: add translation to undo stack, action itself is done each frame.  We only want to undo to last mouse down.
                if( pEditorState->m_EditorActionState >= EDITORACTIONSTATE_TranslateX &&
                    pEditorState->m_EditorActionState <= EDITORACTIONSTATE_TranslateYZ )
                {
                    if( pEditorState->m_pSelectedObjects.size() > 0 && pEditorState->m_DistanceTranslated.LengthSquared() != 0 )
                    {
                        // Create a new list of selected objects, don't include objects that have parents that are selected.
                        std::vector<GameObject*> selectedobjects;
                        for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
                        {
                            ComponentTransform* pTransform = pEditorState->m_pSelectedObjects[i]->GetTransform();

                            // if this object has a selected parent, don't move it, only move the parent.
                            if( pTransform && pTransform->IsAnyParentInList( pEditorState->m_pSelectedObjects ) == false )
                            {
                                selectedobjects.push_back( pEditorState->m_pSelectedObjects[i] );
                            }
                        }

                        if( selectedobjects.size() > 0 )
                        {
#if MYFW_EDITOR
                            g_pEngineCore->GetCommandStack()->Add( MyNew EditorCommand_MoveObjects( pEditorState->m_DistanceTranslated, selectedobjects ) );
#endif
                        }
                    }
                }

                // GIZMOSCALE: add scale to undo stack, action itself is done each frame.  We only want to undo to last mouse down.
                if( pEditorState->m_EditorActionState >= EDITORACTIONSTATE_ScaleX &&
                    pEditorState->m_EditorActionState <= EDITORACTIONSTATE_ScaleXYZ )
                {
                    if( pEditorState->m_pSelectedObjects.size() > 0 &&
                        ( pEditorState->m_AmountScaled.x != 1.0f ||
                          pEditorState->m_AmountScaled.y != 1.0f ||
                          pEditorState->m_AmountScaled.z != 1.0f ) )
                    {
                        // Create a new list of selected objects, don't include objects that have parents that are selected.
                        std::vector<GameObject*> selectedobjects;
                        for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
                        {
                            ComponentTransform* pTransform = pEditorState->m_pSelectedObjects[i]->GetTransform();

                            // if this object has a selected parent, don't move it, only move the parent.
                            if( pTransform && pTransform->IsAnyParentInList( pEditorState->m_pSelectedObjects ) == false )
                            {
                                selectedobjects.push_back( pEditorState->m_pSelectedObjects[i] );
                            }
                        }

                        if( selectedobjects.size() > 0 )
                        {
#if MYFW_EDITOR
                            g_pEngineCore->GetCommandStack()->Add( MyNew EditorCommand_ScaleObjects( pEditorState->m_AmountScaled, pEditorState->m_TransformedInLocalSpace, pEditorState->m_WorldSpacePivot, selectedobjects ) );
#endif
                        }
                    }
                }

                // GIZMOROTATE: add rotation to undo stack, action itself is done each frame.  We only want to undo to last mouse down.
                if( pEditorState->m_EditorActionState >= EDITORACTIONSTATE_RotateX &&
                    pEditorState->m_EditorActionState <= EDITORACTIONSTATE_RotateZ )
                {
                    if( pEditorState->m_pSelectedObjects.size() > 0 && pEditorState->m_DistanceRotated.LengthSquared() != 0 )
                    {
                        // Create a new list of selected objects, don't include objects that have parents that are selected.
                        std::vector<GameObject*> selectedobjects;
                        for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
                        {
                            ComponentTransform* pTransform = pEditorState->m_pSelectedObjects[i]->GetTransform();

                            // if this object has a selected parent, don't move it, only move the parent.
                            if( pTransform && pTransform->IsAnyParentInList( pEditorState->m_pSelectedObjects ) == false )
                            {
                                selectedobjects.push_back( pEditorState->m_pSelectedObjects[i] );
                            }
                        }

                        if( selectedobjects.size() > 0 )
                        {
#if MYFW_EDITOR
                            g_pEngineCore->GetCommandStack()->Add(
                                MyNew EditorCommand_RotateObjects( pEditorState->m_DistanceRotated, pEditorState->m_TransformedInLocalSpace, pEditorState->m_WorldSpacePivot, selectedobjects ) );
#endif
                        }
                    }
                }

                // reset current transform values.
                pEditorState->m_DistanceTranslated.Set( 0, 0, 0 );
                pEditorState->m_AmountScaled.Set( 1, 1, 1 );
                pEditorState->m_DistanceRotated.Set( 0, 0, 0 );

                pEditorState->m_EditorActionState = EDITORACTIONSTATE_None;
            }

            // Check for rightmouse up (MODIFIERKEY_RightMouse)
            if( mouseaction == GCBA_Up && id == 1 && pEditorState->m_EditorActionState == EDITORACTIONSTATE_None )
            {
                m_ShowRightClickMenu = true;

                // find the object we clicked on.
                m_pGameObjectRightClicked = GetObjectAtPixel( (unsigned int)x, (unsigned int)y, true, true );
            }
        }
    }

    return false;
}
