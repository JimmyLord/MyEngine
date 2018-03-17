//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
void EditorInterfaceWxEventHandler::OnPopupClick(wxEvent &evt)
{
    EditorInterfaceWxEventHandler* pEvtHandler = (EditorInterfaceWxEventHandler*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    void* pPointer = pEvtHandler->m_pPointer;
    int value = pEvtHandler->m_ValueInt;

    int id = evt.GetId();

    GameObject* m_pGameObjectRightClicked = (GameObject*)pPointer;

    if( m_pGameObjectRightClicked )
    {
        for( unsigned int i=0; i<m_pGameObjectRightClicked->GetComponentCount(); i++ )
        {
            m_pGameObjectRightClicked->GetComponentByIndex( i )->OnRightClickOptionClicked( evt, RightClick_ComponentOps + 1000 * i );
        }
    }
}
#endif

EditorInterface::EditorInterface()
{
    m_ShowRightClickMenu = false;
    m_pGameObjectRightClicked = 0;
}

EditorInterface::~EditorInterface()
{
}

void EditorInterface::OnActivated()
{
}

void EditorInterface::OnDeactivated()
{
}

void EditorInterface::Tick(double TimePassed)
{
    // Force compile of m_pShader_TintColor (0 lights, 4 bones) to avoid stall on first click in mouse picker code.
    if( g_pEngineCore->GetShader_TintColor() )
    {
        Shader_Base* pShader;

        // No lights, 4 bones. Used by mouse picker.
        pShader = (Shader_Base*)g_pEngineCore->GetShader_TintColor()->GlobalPass( 0, 4 );
        if( pShader->m_Initialized == false )
            pShader->CompileShader();

        // 4 lights, 0 bones. Used by transform gizmo.
        pShader = (Shader_Base*)g_pEngineCore->GetShader_TintColor()->GlobalPass( 4, 0 );
        if( pShader->m_Initialized == false )
            pShader->CompileShader();
    }

    if( m_ShowRightClickMenu )
    {
        m_ShowRightClickMenu = false;

        if( m_pGameObjectRightClicked )
        {
#if MYFW_USING_WX
            wxMenu menu;
            menu.SetClientData( &m_EditorInterfaceWxEventHandler );

            m_EditorInterfaceWxEventHandler.m_pPointer = m_pGameObjectRightClicked;
            m_EditorInterfaceWxEventHandler.m_ValueInt = 0;

            // Loop through components, each will add their menu items to the menu object.
            for( unsigned int i=0; i<m_pGameObjectRightClicked->GetComponentCount(); i++ )
            {
                m_pGameObjectRightClicked->GetComponentByIndex( i )->AddRightClickOptionsToMenu( &menu, EditorInterfaceWxEventHandler::RightClick_ComponentOps + 1000 * i );
            }

 	        menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&EditorInterfaceWxEventHandler::OnPopupClick );

            // blocking call.
            g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
#endif
        }
    }
}

void EditorInterface::OnDrawFrame(unsigned int canvasid)
{
    MyAssert( canvasid == 1 );

    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    MyAssert( pEditorState->m_pEditorCamera );
    if( pEditorState->m_pEditorCamera )
    {
        // draw scene from editor camera and editorFG camera
        for( unsigned int i=0; i<pEditorState->m_pEditorCamera->GetComponentCount(); i++ )
        {
            ComponentCamera* pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->GetComponentByIndex( i ) );

            if( pCamera )
            {
                pCamera->OnDrawFrame();

                if( pCamera->m_LayersToRender & Layer_MainScene ) // only draw selected objects over "main" layer
                {
                    //glDisable( GL_DEPTH_TEST );
                    glDisable( GL_CULL_FACE );
                    glDepthFunc( GL_LEQUAL );

                    // Draw selected objects in editor view.
                    ShaderGroup* pShaderOverride = g_pEngineCore->GetShader_SelectedObjects();

                    Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
                    if( pShader->ActivateAndProgramShader() )
                    {
                        for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
                        {
                            // draw an outline around the selected object
                            if( g_pEngineCore->GetEditorPrefs()->Get_View_SelectedObjects_ShowWireframe() )
                            {
                                glEnable( GL_BLEND );
                                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

                                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                                glEnable( GL_POLYGON_OFFSET_LINE );
                                glEnable( GL_POLYGON_OFFSET_FILL ); // enabling GL_POLYGON_OFFSET_LINE doesn't work on my intel 4000
                                glPolygonOffset( -0.5, -0.5 );
                                pShader->ProgramTint( ColorByte(255,255,255,50) );
                                g_pComponentSystemManager->DrawSingleObject( &pCamera->m_Camera3D.m_matViewProj,
                                                                             pEditorState->m_pSelectedObjects[i],
                                                                             pShaderOverride );
                                glPolygonOffset( 0, 0 );
                                glDisable( GL_POLYGON_OFFSET_FILL );
                                glDisable( GL_POLYGON_OFFSET_LINE );
                                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
                            }
                            
                            // draw the entire selected shape with the shader
                            if( g_pEngineCore->GetEditorPrefs()->Get_View_SelectedObjects_ShowEffect() )
                            {
                                glEnable( GL_BLEND );
                                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

                                pShader->ProgramBaseUniforms( 0, 0, 0, ColorByte(0,0,0,0), ColorByte(0,0,0,0), 0 );
                                pShader->ProgramTint( ColorByte(0,0,0,0) );
                                g_pComponentSystemManager->DrawSingleObject( &pCamera->m_Camera3D.m_matViewProj,
                                                                             pEditorState->m_pSelectedObjects[i],
                                                                             pShaderOverride );
                            }
                        }
                    }

                    pShader->DeactivateShader( 0, true );

                    // always disable blending
                    glDisable( GL_BLEND );

                    glEnable( GL_CULL_FACE );
                    glEnable( GL_DEPTH_TEST );
                }
            }
        }
    }

    // Draw our mouse picker frame over the screen
    if( g_pEngineCore->GetDebug_DrawMousePickerFBO() && g_GLCanvasIDActive == 1 )
    {
        MySprite* pDebugQuad = g_pEngineCore->GetSprite_DebugQuad();

        if( pDebugQuad )
        {
            pDebugQuad->CreateInPlace( "debug", 0.75f, 0.75f, 0.5f, 0.5f, 0, 1, 1, 0, Justify_Center, false );
            g_pEngineCore->GetMaterial_MousePicker()->SetTextureColor( pEditorState->m_pMousePickerFBO->m_pColorTexture );
            pDebugQuad->SetMaterial( g_pEngineCore->GetMaterial_MousePicker() );
            pDebugQuad->Draw( 0, 0 );
        }
    }
}

void EditorInterface::SetModifierKeyStates(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( keyaction == GCBA_Down )
    {
        if( keycode == MYKEYCODE_LCTRL )    pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Control;
        if( keycode == MYKEYCODE_LALT )     pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Alt;
        if( keycode == MYKEYCODE_LSHIFT )   pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Shift;
        if( keycode == ' ' )                pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Space;
    }

    // since keys can be pressed while a different window frame has focus, let's just manually query the modifiers.
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LCTRL ) )  pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Control;
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LALT ) )   pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Alt;
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LSHIFT ) ) pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Shift;
    if( PlatformSpecific_CheckKeyState( ' ' ) )              pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Space;

    if( mouseaction != -1 )
    {
        pEditorState->m_CurrentMousePosition.Set( x, y );
        //pEditorState->m_LastMousePosition.Set( x, y );

        if( mouseaction == GCBA_Down )
        {
            if( id == 0 )
            {
                pEditorState->m_MouseDownLocation[id] = pEditorState->m_CurrentMousePosition;
                pEditorState->m_ModifierKeyStates |= MODIFIERKEY_LeftMouse;
            }
            if( id == 1 )
            {
                pEditorState->m_MouseDownLocation[id] = pEditorState->m_CurrentMousePosition;
                pEditorState->m_ModifierKeyStates |= MODIFIERKEY_RightMouse;
            }
            if( id == 2 )
            {
                pEditorState->m_MouseDownLocation[id] = pEditorState->m_CurrentMousePosition;
                pEditorState->m_ModifierKeyStates |= MODIFIERKEY_MiddleMouse;
            }
        }

        if( mouseaction == GCBA_Held || mouseaction == GCBA_RelativeMovement )
        {
            if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
            {
                if( pEditorState->m_MouseDownLocation[0] != pEditorState->m_CurrentMousePosition )
                    pEditorState->m_HasMouseMovedSinceButtonPressed[0] = true;
            }
            if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_RightMouse )
            {
                if( pEditorState->m_MouseDownLocation[1] != pEditorState->m_CurrentMousePosition )
                    pEditorState->m_HasMouseMovedSinceButtonPressed[1] = true;
            }
            if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_MiddleMouse )
            {
                if( pEditorState->m_MouseDownLocation[2] != pEditorState->m_CurrentMousePosition )
                    pEditorState->m_HasMouseMovedSinceButtonPressed[2] = true;
            }
        }
    }
}

void EditorInterface::ClearModifierKeyStates(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( keyaction == GCBA_Up )
    {
        if( keycode == MYKEYCODE_LCTRL )    pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Control;
        if( keycode == MYKEYCODE_LALT )     pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Alt;
        if( keycode == MYKEYCODE_LSHIFT )   pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Shift;
        if( keycode == ' ' )                pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Space;
    }

    // since keys can be pressed while a different window frame has focus, let's just manually query the modifiers.
    // this shouldn't be necessary since mouse down will have given us focus if we care about keys
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LCTRL ) == false )  pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Control;
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LALT ) == false )   pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Alt;
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LSHIFT ) == false ) pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Shift;
    if( PlatformSpecific_CheckKeyState( ' ' ) == false )              pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Space;

    if( mouseaction != -1 )
    {
        if( mouseaction == GCBA_Up )
        {
            pEditorState->m_MouseDownLocation[id] = Vector2( -1, -1 );

            if( id == 0 )
            {
                pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_LeftMouse;
            }
            else if( id == 1 )
            {
                pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_RightMouse;
            }
            else if( id == 2 )
            {
                pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_MiddleMouse;
            }

            // Unlock the mouse, even if it wasn't locked.
#if !(MYFW_OSX && MYFW_USING_WX)
#if MYFW_USING_WX
            g_pEngineMainFrame->GetGLCanvasEditor()->LockMouse( false );
#elif MYFW_USING_IMGUI
            //LOGInfo( LOGTag, "Request mouse unlock\n" );
            SetMouseLock( false, Vector2(-1,-1) );
#endif //MYFW_USING_WX
#endif //!(MYFW_OSX && MYFW_USING_WX)

            pEditorState->m_HasMouseMovedSinceButtonPressed[id] = false;
        }
    }

    pEditorState->m_LastMousePosition = pEditorState->m_CurrentMousePosition;
}

bool EditorInterface::HandleInputForEditorCamera(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    ComponentCamera* pCamera = pEditorState->GetEditorCamera();
    MyMatrix StartCamTransform = *pCamera->m_pComponentTransform->GetLocalTransform( false );

    // if mouse message. down, up, dragging or wheel.
    if( mouseaction != -1 )
    {
        unsigned int mods = pEditorState->m_ModifierKeyStates;

        // get the editor camera's local transform.
        MyMatrix* matCamera = pCamera->m_pComponentTransform->GetLocalTransform( true );

        // move camera in/out if mousewheel spinning
        if( mouseaction == GCBA_Wheel )
        {
            // pressure is also mouse wheel movement rate in wx configurations.
#if MYFW_RIGHTHANDED
            Vector3 dir = Vector3( 0, 0, 1 ) * -(pressure/fabs(pressure));
#else
            Vector3 dir = Vector3( 0, 0, 1 ) * (pressure/fabs(pressure));
#endif
            float speed = 100.0f;
            if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
                speed *= 5;

            if( dir.LengthSquared() > 0 )
                matCamera->TranslatePreRotScale( dir * speed * 1/60.0f); //* g_pEngineCore->GetTimePassedUnpausedLastFrame() );
        }

        // if left mouse down, reset the transform gizmo tool.
        if( mouseaction == GCBA_Down && id == 0 )
        {
            pEditorState->m_pTransformGizmo->m_LastIntersectResultIsValid = false;
        }

        // if space is held, left button will pan the camera around.  or just middle button
        if( ( (mods & MODIFIERKEY_LeftMouse) && (mods & MODIFIERKEY_Space) ) || (mods & MODIFIERKEY_MiddleMouse) )
        {
#if MYFW_OSX && MYFW_USING_WX
            // TODO: fix OSX to support locked mouse cursor.
            Vector3 dir = pEditorState->m_LastMousePosition - pEditorState->m_CurrentMousePosition;
#else //MYFW_OSX && MYFW_USING_WX
            // Try to lock the editor mouse cursor so we can move camera with raw mouse input.
#if MYFW_USING_WX // TODO_FIX_EDITOR
            if( g_pEngineMainFrame->GetGLCanvasEditor()->IsMouseLocked() == false )
            {
                g_pEngineMainFrame->GetGLCanvasEditor()->LockMouse( true );
            }
#elif MYFW_USING_IMGUI
            if( IsMouseLocked() == false )
            {
                //LOGInfo( LOGTag, "Request mouse lock\n" );
                SetMouseLock( true, g_pEngineCore->GetEditorMainFrame_ImGui()->GetEditorWindowCenterPosition() );
            }
#endif //MYFW_USING_WX
            Vector2 dir( 0, 0 );
            if( mouseaction == GCBA_RelativeMovement )
            {
                //LOGInfo( LOGTag, "relative movement.\n" );
                dir.Set( -x, -y );
            }
#endif //MYFW_OSX && MYFW_USING_WX

            //LOGInfo( LOGTag, "dir (%0.2f, %0.2f)\n", dir.x, dir.y );
            if( dir.LengthSquared() > 0 )
                matCamera->TranslatePreRotScale( dir * 0.05f );
        }
        else if( mouseaction == GCBA_Held &&
                 pEditorState->m_EditorActionState == EDITORACTIONSTATE_GroupSelectingObjects &&
                 (mods & MODIFIERKEY_LeftMouse) )
        {
            // TODO: draw a box in the foreground.
            //int smallerx = pEditorState->m_CurrentMousePosition.x > pEditorState->m_MouseLeftDownLocation.x ? pEditorState->m_MouseLeftDownLocation.x : pEditorState->m_CurrentMousePosition.x;
            //int biggerx = pEditorState->m_CurrentMousePosition.x < pEditorState->m_MouseLeftDownLocation.x ? pEditorState->m_MouseLeftDownLocation.x : pEditorState->m_CurrentMousePosition.x;

            //int smallery = pEditorState->m_CurrentMousePosition.y > pEditorState->m_MouseLeftDownLocation.y ? pEditorState->m_MouseLeftDownLocation.y : pEditorState->m_CurrentMousePosition.y;
            //int biggery = pEditorState->m_CurrentMousePosition.y < pEditorState->m_MouseLeftDownLocation.y ? pEditorState->m_MouseLeftDownLocation.y : pEditorState->m_CurrentMousePosition.y;

            //LOGInfo( LOGTag, "group selecting: %d,%d  %d,%d\n", smallerx, smallery, biggerx, biggery );

            //pEditorState->ClearSelectedObjectsAndComponents();
            //for( int y=smallery; y<biggery; y++ )
            //{
            //    for( int x=smallerx; x<biggerx; x++ )
            //    {
            //        GameObject* pObject = GetObjectAtPixel( x, y, false );
            //        if( pObject )
            //            g_pPanelObjectList->SelectObject( pObject ); // passing in 0 will unselect all items.
            //    }
            //}
        }
        // if right mouse is down, rotate the camera around selected object or around it's current position
        else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_None &&
                 (mods & MODIFIERKEY_RightMouse) )
        {
#if MYFW_USING_WX && MYFW_OSX
            // TODO: fix OSX to support locked mouse cursor.
            Vector3 dir = (pEditorState->m_LastMousePosition - pEditorState->m_CurrentMousePosition) * -1;
#else //MYFW_USING_WX && MYFW_OSX
            // Try to lock the editor mouse cursor so we can move camera with raw mouse input.
#if MYFW_USING_WX
            if( g_pEngineMainFrame->GetGLCanvasEditor()->IsMouseLocked() == false )
            {
                g_pEngineMainFrame->GetGLCanvasEditor()->LockMouse( true );
            }
#elif MYFW_USING_IMGUI
            if( IsMouseLocked() == false )
            {
                //LOGInfo( LOGTag, "Request mouse lock\n" );
                SetMouseLock( true, g_pEngineCore->GetEditorMainFrame_ImGui()->GetEditorWindowCenterPosition() );
            }
#endif //MYFW_USING_WX
            Vector2 dir( 0, 0 );
            if( mouseaction == GCBA_RelativeMovement )
            {
                //LOGInfo( LOGTag, "Relative Movement\n" );
                dir.Set( x, y );
            }
#endif //MYFW_USING_WX && MYFW_OSX

            if( dir.LengthSquared() > 0 )
            {
                Vector3 pivot;
                float distancefrompivot;

                // different pivot and distance from pivot depending if Alt is held.
                if( mods & MODIFIERKEY_Alt && pEditorState->m_pSelectedObjects.size() > 0 && pEditorState->m_pTransformGizmo->m_pTranslate1Axis[0] )
                {
                    // pivot around the transform gizmo
                    pivot = pEditorState->m_pTransformGizmo->m_pTranslate1Axis[0]->GetTransform()->GetWorldTransform()->GetTranslation();
                    distancefrompivot = (matCamera->GetTranslation() - pivot).Length();
                }
                else
                {
                    // pivot on the camera, just change rotation.
                    pivot = matCamera->GetTranslation();
                    distancefrompivot = 0;
                }

                //LOGInfo( LOGTag, "dir (%0.2f, %0.2f)\n", dir.x, dir.y );

                Vector3 angle = pCamera->m_pComponentTransform->GetLocalRotation();

                // TODO: make this degrees per inch
                float degreesperpixel = 0.125f;

#if MYFW_RIGHTHANDED
                angle.y += dir.x * degreesperpixel;
                angle.x -= dir.y * degreesperpixel;
#else
                angle.y -= dir.x * degreesperpixel;
                angle.x += dir.y * degreesperpixel;
#endif
                MyClamp( angle.x, -90.0f, 90.0f );

                // Create a new local transform
                matCamera->SetIdentity();
#if MYFW_RIGHTHANDED
                matCamera->Translate( 0, 0, distancefrompivot );
#else
                matCamera->Translate( 0, 0, -distancefrompivot );
#endif
                matCamera->Rotate( angle.x, 1, 0, 0 );
                matCamera->Rotate( angle.y, 0, 1, 0 );
                matCamera->Translate( pivot );

                // Update the local scale/rotation/translation from the local transform
                pCamera->m_pComponentTransform->UpdateLocalSRT();
            }
        }

        // pull the scale/pos/rot from the local matrix and update the values in the watch window.
        pCamera->m_pComponentTransform->UpdateLocalSRT();
        pCamera->m_pComponentTransform->UpdateTransform();
    }

    // handle editor keys
    if( keyaction == GCBA_Held )
    {
        // get the editor camera's local transform.
        MyMatrix* matCamera = pCamera->m_pComponentTransform->GetLocalTransform( true );

        // WASD to move camera
        Vector3 dir( 0, 0, 0 );
        if( keycode == 'W' ) dir.z +=  1;
        if( keycode == 'A' ) dir.x += -1;
        if( keycode == 'S' ) dir.z += -1;
        if( keycode == 'D' ) dir.x +=  1;
        if( keycode == 'Q' ) dir.y +=  1;
        if( keycode == 'Z' ) dir.y -=  1;

        float speed = 7.0f;
        if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
            speed *= 5;

        if( dir.LengthSquared() > 0 )
            matCamera->TranslatePreRotScale( dir * speed * g_pEngineCore->GetTimePassedUnpausedLastFrame() );

        pCamera->m_pComponentTransform->UpdateLocalSRT();
        pCamera->m_pComponentTransform->UpdateTransform();
    }

    // If the camera is locked to an object,
    //     apply any changes we made to the editor camera to the offset from the locked object.
    if( pEditorState->m_CameraState == EditorCameraState_LockedToObject )
    {
        MyMatrix EndCamTransform = *pCamera->m_pComponentTransform->GetLocalTransform( false );
        MyMatrix StartCamInverseTransform = StartCamTransform.GetInverse();

        MyMatrix ChangeInCamTransform = StartCamInverseTransform * EndCamTransform;
        MyMatrix newtransform = pEditorState->m_OffsetFromObject * ChangeInCamTransform;

        pEditorState->m_OffsetFromObject = newtransform;
    }

    return false;
}

void EditorInterface::RenderObjectIDsToFBO()
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( pEditorState->m_pMousePickerFBO->IsFullyLoaded() == false )
        return;

    // bind our FBO so we can render the scene to it.
    pEditorState->m_pMousePickerFBO->Bind( true );

    pEditorState->m_pTransformGizmo->ScaleGizmosForMousePickRendering( true );

    glDisable( GL_SCISSOR_TEST );
    glViewport( 0, 0, pEditorState->m_pMousePickerFBO->m_Width, pEditorState->m_pMousePickerFBO->m_Height );

    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw all editor camera components.
    ComponentCamera* pCamera = 0;
    for( unsigned int i=0; i<pEditorState->m_pEditorCamera->GetComponentCount(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->GetComponentByIndex( i ) );
        if( pCamera )
        {
            g_pComponentSystemManager->DrawMousePickerFrame( pCamera, &pCamera->m_Camera3D.m_matViewProj, g_pEngineCore->GetShader_TintColor() );
            glClear( GL_DEPTH_BUFFER_BIT );
        }
    }

    pEditorState->m_pMousePickerFBO->Unbind( true );

    pEditorState->m_pTransformGizmo->ScaleGizmosForMousePickRendering( false );
}

unsigned int EditorInterface::GetIDAtPixel(unsigned int x, unsigned int y, bool createnewbitmap, bool includetransformgizmo)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( pEditorState->m_pMousePickerFBO->IsFullyLoaded() == false )
        return 0;

    if( x >= pEditorState->m_pMousePickerFBO->m_Width ||
        y >= pEditorState->m_pMousePickerFBO->m_Height )
        return 0;

    if( createnewbitmap )
    {
        if( includetransformgizmo == false )
        {
            // Hide the transform gizmo
            pEditorState->m_pTransformGizmo->Hide();
        }

        RenderObjectIDsToFBO();
    }

    // bind our FBO so we can render sample from it.
    pEditorState->m_pMousePickerFBO->Bind( true );

    // Find the first camera again.
    ComponentCamera* pCamera = 0;
    for( unsigned int i=0; i<pEditorState->m_pEditorCamera->GetComponentCount(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->GetComponentByIndex( i ) );
        break;
    }

    MyAssert( pCamera );
    if( pCamera == 0 )
        return 0;

    // get a pixel from the FBO... use m_WindowStartX/m_WindowStartY from any camera component.
    unsigned char pixel[4];
    glReadPixels( x - (unsigned int)pCamera->m_WindowStartX, y - (unsigned int)pCamera->m_WindowStartY,
                  1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel );

    pEditorState->m_pMousePickerFBO->Unbind( true );

    // Convert the RGB value to an id.
    uint64_t id = pixel[0] + pixel[1]*256 + pixel[2]*256*256 + pixel[3]*256*256*256;
    id = (((uint64_t)UINT_MAX+1) * (id % 641) + id) / 641; // 1, 641, 6700417, 4294967297,
    //LOGInfo( LOGTag, "pixel - %d, %d, %d, %d - id - %d\n", pixel[0], pixel[1], pixel[2], pixel[3], id );

    return (unsigned int)id;
}

GameObject* EditorInterface::GetObjectAtPixel(unsigned int x, unsigned int y, bool createnewbitmap, bool includetransformgizmo)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    unsigned int id = GetIDAtPixel( x, y, createnewbitmap, includetransformgizmo );

    SceneID sceneid = (SceneID)(id / 100000);
    id = id % 100000;

    // find the object clicked on.
    GameObject* pGameObject = g_pComponentSystemManager->FindGameObjectByID( sceneid, (unsigned int)id );

    // if we didn't click on something, check if it's the transform gizmo.
    //   has to be checked manually since they are unmanaged.
    if( pGameObject == 0 )
    {
        for( int i=0; i<3; i++ )
        {
            if( pEditorState->m_pTransformGizmo->m_pTranslate1Axis[i]->GetID() == id )
            {
                pGameObject = pEditorState->m_pTransformGizmo->m_pTranslate1Axis[i];
            }

            if( pEditorState->m_pTransformGizmo->m_pTranslate2Axis[i]->GetID() == id )
            {
                pGameObject = pEditorState->m_pTransformGizmo->m_pTranslate2Axis[i];
            }

            if( pEditorState->m_pTransformGizmo->m_pScale1Axis[i]->GetID() == id )
            {
                pGameObject = pEditorState->m_pTransformGizmo->m_pScale1Axis[i];
            }

            if( pEditorState->m_pTransformGizmo->m_pRotate1Axis[i]->GetID() == id )
            {
                pGameObject = pEditorState->m_pTransformGizmo->m_pRotate1Axis[i];
            }
        }

        if( pEditorState->m_pTransformGizmo->m_pScale3Axis->GetID() == id )
        {
            pGameObject = pEditorState->m_pTransformGizmo->m_pScale3Axis;
        }
    }

    return pGameObject;
}

void EditorInterface::SelectObjectsInRectangle(unsigned int sx, unsigned int sy, unsigned int ex, unsigned int ey)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    int smallerx = sx > ex ? ex : sx;
    int biggerx = sx < ex ? ex : sx;

    int smallery = sy > ey ? ey : sy;
    int biggery = sy < ey ? ey : sy;

    //LOGInfo( LOGTag, "group selecting: %d,%d  %d,%d\n", smallerx, smallery, biggerx, biggery );

    // render to the FBO.
    RenderObjectIDsToFBO();

    if( pEditorState->m_pMousePickerFBO->IsFullyLoaded() == false )
        return;

    // bind our FBO so we can sample from it.
    pEditorState->m_pMousePickerFBO->Bind( true );

    // Find the first camera again.
    ComponentCamera* pCamera = 0;
    for( unsigned int i=0; i<pEditorState->m_pEditorCamera->GetComponentCount(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->GetComponentByIndex( i ) );
        break;
    }

    MyAssert( pCamera );
    if( pCamera == 0 )
        return;

    // get a pixel from the FBO... use m_WindowStartX/m_WindowStartY from any camera component.
    unsigned int fbowidth = pEditorState->m_pMousePickerFBO->m_Width;
    unsigned int fboheight = pEditorState->m_pMousePickerFBO->m_Height;
    unsigned char* pixels = MyNew unsigned char[fbowidth * fboheight * 4];
    glReadPixels( 0, 0, fbowidth, fboheight, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
    pEditorState->m_pMousePickerFBO->Unbind( true );

    bool controlheld = pEditorState->m_ModifierKeyStates & MODIFIERKEY_Control ? true : false;

    // if user isn't holding control, then clear objects and items selected in tree.
    if( controlheld == false )
        pEditorState->ClearSelectedObjectsAndComponents();

    // potentially about to multi-select, so disable tree callbacks.
#if MYFW_USING_WX
    g_pPanelObjectList->m_UpdatePanelWatchOnSelection = false;
#endif

    //unsigned int pixelsbeingtested = (biggerx - smallerx) * (biggery - smallery);

    // check whether or not the first object clicked on was selected, we only care if control is held.
    bool firstobjectwasselected = false;
    if( controlheld )
    {
        unsigned int offset = (sy*fbowidth + sx)*4;
        unsigned long long id = pixels[offset+0] + pixels[offset+1]*256 + pixels[offset+2]*256*256 + pixels[offset+3]*256*256*256;
        id = (((uint64_t)UINT_MAX+1) * (id % 641) + id) / 641; // 1, 641, 6700417, 4294967297,

        SceneID sceneid = (SceneID)(id / 100000);
        id = id % 100000;

        // if the object's not already selected, select it.
        GameObject* pObject = g_pComponentSystemManager->FindGameObjectByID( sceneid, (unsigned int)id );

        if( pObject && pEditorState->IsGameObjectSelected( pObject ) )
            firstobjectwasselected = true;
    }

    for( int y=smallery; y<=biggery; y++ )
    {
        for( int x=smallerx; x<=biggerx; x++ )
        {
            unsigned int offset = (y*fbowidth + x)*4;
            unsigned long long id = pixels[offset+0] + pixels[offset+1]*256 + pixels[offset+2]*256*256 + pixels[offset+3]*256*256*256;
            id = (((uint64_t)UINT_MAX+1) * (id % 641) + id) / 641; // 1, 641, 6700417, 4294967297,

            SceneID sceneid = (SceneID)(id / 100000);
            id = id % 100000;

            // if the object's not already selected, select it.
            GameObject* pObject = g_pComponentSystemManager->FindGameObjectByID( sceneid, (unsigned int)id );

            if( pObject )
            {
                // When selecting with mouse, don't allow selection of subobjects of a prefab, always pick the root of the prefab instance.
                if( pObject->GetPrefabRef()->GetPrefab() != 0 )
                {
                    pObject = pObject->FindRootGameObjectOfPrefabInstance();
                }

                bool objectselected = pEditorState->IsGameObjectSelected( pObject );

                // if we're selecting objects, then select the unselected objects.
                if( firstobjectwasselected == false )
                {
                    if( objectselected == false )
                    {
                        // select the object
                        pEditorState->SelectGameObject( pObject );

                        // select the object in the object tree.
#if MYFW_USING_WX
                        g_pPanelObjectList->SelectObject( pObject );
#endif //MYFW_USING_WX
                    }
                }
                else if( controlheld ) // if the first object was already selected, deselect all dragged if control is held.
                {
                    if( objectselected == true )
                    {
                        pEditorState->UnselectGameObject( pObject );
#if MYFW_USING_WX
                        g_pPanelObjectList->UnselectObject( pObject );
#endif //MYFW_USING_WX
                    }
                }
            }
        }
    }

#if MYFW_USING_WX
    g_pPanelObjectList->m_UpdatePanelWatchOnSelection = true;
    UpdatePanelWatchWithSelectedItems(); // will reset and update pEditorState->m_pSelectedObjects
#endif //MYFW_USING_WX

    //LOGInfo( LOGTag, "Done selecting objects.\n" );

    delete[] pixels;
}
