//
// Copyright (c) 2016-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorInterface.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"
#include "../SourceEditor/EditorPrefs.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/TransformGizmo.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"

EditorInterface::EditorInterface(EngineCore* pEngineCore)
{
    m_pEngineCore = pEngineCore;

    m_ShowRightClickMenu = false;
    m_pGameObjectRightClicked = nullptr;
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

void EditorInterface::Tick(float deltaTime)
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
        // Draw scene from editor camera and editorFG camera.
        for( unsigned int i=0; i<pEditorState->m_pEditorCamera->GetComponentCount(); i++ )
        {
            ComponentCamera* pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->GetComponentByIndex( i ) );

            if( pCamera )
            {
                pCamera->OnDrawFrame();

                if( pCamera->m_LayersToRender & Layer_MainScene ) // Only draw selected objects over "main" layer.
                {
                    //g_pRenderer->SetDepthTestEnabled( false );
                    g_pRenderer->SetCullingEnabled( false );
                    g_pRenderer->SetDepthFunction( MyRE::DepthFunc_LEqual );

                    // Draw selected objects in editor view.
                    ShaderGroup* pShaderOverride = g_pEngineCore->GetShader_SelectedObjects();

                    Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
                    if( pShader->Activate() )
                    {
                        for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
                        {
                            // Draw an outline around the selected object.
                            if( g_pEngineCore->GetEditorPrefs()->Get_View_SelectedObjects_ShowWireframe() )
                            {
                                g_pRenderer->SetBlendEnabled( true );
                                g_pRenderer->SetBlendFunc( MyRE::BlendFactor_SrcAlpha, MyRE::BlendFactor_OneMinusSrcAlpha );

                                g_pRenderer->SetPolygonMode( MyRE::PolygonDrawMode_Line );
                                g_pRenderer->SetPolygonOffset( true, -0.5f, -0.5f );
                                pShader->ProgramTint( ColorByte(255,255,255,50) );
                                g_pComponentSystemManager->DrawSingleObject( &pCamera->m_Camera3D.m_matProj,
                                                                             &pCamera->m_Camera3D.m_matView,
                                                                             pEditorState->m_pSelectedObjects[i],
                                                                             pShaderOverride );

                                g_pRenderer->SetPolygonOffset( false, 0, 0 );
                                g_pRenderer->SetPolygonMode( MyRE::PolygonDrawMode_Fill );
                            }
                            
                            // Draw the entire selected shape with the shader.
                            if( g_pEngineCore->GetEditorPrefs()->Get_View_SelectedObjects_ShowEffect() )
                            {
                                g_pRenderer->SetBlendEnabled( true );
                                g_pRenderer->SetBlendFunc( MyRE::BlendFactor_SrcAlpha, MyRE::BlendFactor_OneMinusSrcAlpha );

                                pShader->ProgramMaterialProperties( nullptr, ColorByte(0,0,0,0), ColorByte(0,0,0,0), 0 );
                                pShader->ProgramTransforms( nullptr, nullptr, nullptr );
                                pShader->ProgramTint( ColorByte(0,0,0,0) );
                                g_pComponentSystemManager->DrawSingleObject( &pCamera->m_Camera3D.m_matProj,
                                                                             &pCamera->m_Camera3D.m_matView,
                                                                             pEditorState->m_pSelectedObjects[i],
                                                                             pShaderOverride );
                            }
                        }
                    }

                    pShader->DeactivateShader( nullptr, true );

                    // Always disable blending.
                    g_pRenderer->SetBlendEnabled( false );

                    g_pRenderer->SetCullingEnabled( true );
                    g_pRenderer->SetDepthTestEnabled( true );
                }
            }
        }
    }

    // Draw our mouse picker frame over the screen.
    if( g_pEngineCore->GetDebug_DrawMousePickerFBO() && g_GLCanvasIDActive == 1 )
    {
        MySprite* pDebugQuad = g_pEngineCore->GetSprite_DebugQuad();

        if( pDebugQuad )
        {
            pDebugQuad->CreateInPlace( "debug", 0.75f, 0.75f, 0.5f, 0.5f, 0, 1, 1, 0, Justify_Center, false );
            g_pEngineCore->GetMaterial_MousePicker()->SetTextureColor( pEditorState->m_pMousePickerFBO->GetColorTexture( 0 ) );
            pDebugQuad->SetMaterial( g_pEngineCore->GetMaterial_MousePicker() );
            pDebugQuad->Draw( nullptr, nullptr, nullptr );
        }
    }
}

void EditorInterface::SetModifierKeyStates(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( keyAction == GCBA_Down )
    {
        if( keyCode == MYKEYCODE_LCTRL )    pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Control;
        if( keyCode == MYKEYCODE_LALT )     pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Alt;
        if( keyCode == MYKEYCODE_LSHIFT )   pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Shift;
        if( keyCode == ' ' )                pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Space;
    }

    // Since keys can be pressed while a different window frame has focus, let's just manually query the modifiers.
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LCTRL ) )  pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Control;
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LALT ) )   pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Alt;
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LSHIFT ) ) pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Shift;
    if( PlatformSpecific_CheckKeyState( ' ' ) )              pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Space;

    if( mouseAction != -1 )
    {
        pEditorState->m_CurrentMousePosition.Set( x, y );
        //pEditorState->m_LastMousePosition.Set( x, y );

        if( mouseAction == GCBA_Down )
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

        if( mouseAction == GCBA_Held || mouseAction == GCBA_RelativeMovement )
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

void EditorInterface::ClearModifierKeyStates(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( keyAction == GCBA_Up )
    {
        if( keyCode == MYKEYCODE_LCTRL )    pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Control;
        if( keyCode == MYKEYCODE_LALT )     pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Alt;
        if( keyCode == MYKEYCODE_LSHIFT )   pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Shift;
        if( keyCode == ' ' )                pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Space;
    }

    // Since keys can be pressed while a different window frame has focus, let's just manually query the modifiers.
    // This shouldn't be necessary since mouse down will have given us focus if we care about keys.
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LCTRL ) == false )  pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Control;
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LALT ) == false )   pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Alt;
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LSHIFT ) == false ) pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Shift;
    if( PlatformSpecific_CheckKeyState( ' ' ) == false )              pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Space;

    if( mouseAction != -1 )
    {
        if( mouseAction == GCBA_Up )
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
#if !MYFW_OSX
            //LOGInfo( LOGTag, "Request mouse unlock\n" );
            SetMouseLock( false );
#endif //!MYFW_OSX

            pEditorState->m_HasMouseMovedSinceButtonPressed[id] = false;
        }
    }

    pEditorState->m_LastMousePosition = pEditorState->m_CurrentMousePosition;
}

bool EditorInterface::HandleInputForEditorCamera(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    ComponentCamera* pCamera = pEditorState->GetEditorCamera();
    MyMatrix startCamTransform = *pCamera->m_pComponentTransform->GetLocalTransform( false );

    // If mouse message. down, up, dragging or wheel.
    if( mouseAction != -1 )
    {
        unsigned int mods = pEditorState->m_ModifierKeyStates;

        // Get the editor camera's local transform.
        MyMatrix* matCamera = pCamera->m_pComponentTransform->GetLocalTransform( true );

        // Move camera in/out if mousewheel spinning.
        if( mouseAction == GCBA_Wheel )
        {
            // Pressure is also mouse wheel movement rate in editor configurations.
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

        // If left mouse down, reset the transform gizmo tool.
        if( mouseAction == GCBA_Down && id == 0 )
        {
            pEditorState->m_pTransformGizmo->m_LastIntersectResultIsValid = false;
        }

        // Enter/Exit RotatingEditorCamera camera state on right-click.
        {
            // If the right mouse button was clicked, switch to rotating editor camera state.
            if( mouseAction == GCBA_Down && id == 1 )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_RotatingEditorCamera;
            }

            // If we're in EDITORACTIONSTATE_RotatingEditorCamera and the right mouse goes up.
            if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_RotatingEditorCamera &&
                mouseAction == GCBA_Up && id == 1 )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_None;
            }
        }

        // If space is held, left button will pan the camera around.  or just middle button.
        if( ( (mods & MODIFIERKEY_LeftMouse) && (mods & MODIFIERKEY_Space) ) || (mods & MODIFIERKEY_MiddleMouse) )
        {
#if MYFW_OSX
            // TODO: Fix OSX to support locked mouse cursor.
            Vector3 dir = pEditorState->m_LastMousePosition - pEditorState->m_CurrentMousePosition;
#else //MYFW_OSX
            // Try to lock the editor mouse cursor so we can move camera with raw mouse input.
            if( IsMouseLocked() == false )
            {
                //LOGInfo( LOGTag, "Request mouse lock\n" );
                SetMouseLock( true );
            }

            Vector2 dir( 0, 0 );
            if( mouseAction == GCBA_RelativeMovement )
            {
                //LOGInfo( LOGTag, "relative movement.\n" );
                dir.Set( -x, -y );
            }
#endif //MYFW_OSX

            //LOGInfo( LOGTag, "dir (%0.2f, %0.2f)\n", dir.x, dir.y );
            if( dir.LengthSquared() > 0 )
                matCamera->TranslatePreRotScale( dir * 0.05f );
        }
        else if( mouseAction == GCBA_Held &&
                 pEditorState->m_EditorActionState == EDITORACTIONSTATE_GroupSelectingObjects &&
                 (mods & MODIFIERKEY_LeftMouse) )
        {
        }
        // If right mouse is down, rotate the camera around selected object or around it's current position.
        else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_RotatingEditorCamera &&
                 (mods & MODIFIERKEY_RightMouse) )
        {
#if MYFW_OSX
            // TODO: fix OSX to support locked mouse cursor.
            Vector3 dir = (pEditorState->m_LastMousePosition - pEditorState->m_CurrentMousePosition) * -1;
#else //MYFW_OSX
            // Try to lock the editor mouse cursor so we can move camera with raw mouse input.
            if( IsMouseLocked() == false )
            {
                //LOGInfo( LOGTag, "Request mouse lock\n" );
                SetMouseLock( true );
            }

            Vector2 dir( 0, 0 );
            if( mouseAction == GCBA_RelativeMovement )
            {
                //LOGInfo( LOGTag, "Relative Movement\n" );
                dir.Set( x, y );
            }
#endif //MYFW_OSX

            if( dir.LengthSquared() > 0 )
            {
                Vector3 pivot;
                float distancefrompivot;

                // Different pivot and distance from pivot depending if Alt is held.
                if( mods & MODIFIERKEY_Alt && pEditorState->m_pSelectedObjects.size() > 0 && pEditorState->m_pTransformGizmo->m_pTranslate1Axis[0] )
                {
                    // pivot around the transform gizmo
                    pivot = pEditorState->m_pTransformGizmo->m_pTranslate1Axis[0]->GetTransform()->GetWorldTransform()->GetTranslation();
                    distancefrompivot = (matCamera->GetTranslation() - pivot).Length();
                }
                else
                {
                    // Pivot on the camera, just change rotation.
                    pivot = matCamera->GetTranslation();
                    distancefrompivot = 0;
                }

                //LOGInfo( LOGTag, "dir (%0.2f, %0.2f)\n", dir.x, dir.y );

                Vector3 angle = pCamera->m_pComponentTransform->GetLocalRotation();

                // TODO: Make this degrees per inch.
                float degreesperpixel = 0.125f;

#if MYFW_RIGHTHANDED
                angle.y += dir.x * degreesperpixel;
                angle.x -= dir.y * degreesperpixel;
#else
                angle.y -= dir.x * degreesperpixel;
                angle.x += dir.y * degreesperpixel;
#endif
                MyClamp( angle.x, -90.0f, 90.0f );

                // Create a new local transform.
                matCamera->SetIdentity();
#if MYFW_RIGHTHANDED
                matCamera->Translate( 0, 0, distancefrompivot );
#else
                matCamera->Translate( 0, 0, -distancefrompivot );
#endif
                matCamera->Rotate( angle.x, 1, 0, 0 );
                matCamera->Rotate( angle.y, 0, 1, 0 );
                matCamera->Translate( pivot );

                // Update the local scale/rotation/translation from the local transform.
                pCamera->m_pComponentTransform->UpdateLocalSRT();
            }
        }

        // Pull the scale/pos/rot from the local matrix and update the values in the watch window.
        pCamera->m_pComponentTransform->UpdateLocalSRT();
        pCamera->m_pComponentTransform->UpdateTransform();
    }

    // Handle editor keys.
    if( keyAction == GCBA_Held )
    {
        // Get the editor camera's local transform.
        MyMatrix* matCamera = pCamera->m_pComponentTransform->GetLocalTransform( true );

        // WASD to move camera.
        Vector3 dir( 0, 0, 0 );
        if( keyCode == 'W' ) dir.z +=  1;
        if( keyCode == 'A' ) dir.x += -1;
        if( keyCode == 'S' ) dir.z += -1;
        if( keyCode == 'D' ) dir.x +=  1;
        if( keyCode == 'Q' ) dir.y +=  1;
        if( keyCode == 'Z' ) dir.y -=  1;

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
        MyMatrix endCamTransform = *pCamera->m_pComponentTransform->GetLocalTransform( false );
        MyMatrix startCamInverseTransform = startCamTransform.GetInverse();

        MyMatrix changeInCamTransform = startCamInverseTransform * endCamTransform;
        MyMatrix newTransform = pEditorState->m_OffsetFromObject * changeInCamTransform;

        pEditorState->m_OffsetFromObject = newTransform;
    }

    return false;
}

void EditorInterface::RenderObjectIDsToFBO()
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( pEditorState->m_pMousePickerFBO->IsFullyLoaded() == false )
        return;

    // Bind our FBO so we can render the scene to it.
    pEditorState->m_pMousePickerFBO->Bind( true );

    pEditorState->m_pTransformGizmo->ScaleGizmosForMousePickRendering( true );

    MyViewport viewport( 0, 0, pEditorState->m_pMousePickerFBO->GetWidth(), pEditorState->m_pMousePickerFBO->GetHeight() );
    g_pRenderer->EnableViewport( &viewport, true );

    g_pRenderer->SetClearColor( ColorFloat( 0, 0, 0, 0 ) );
    g_pRenderer->ClearBuffers( true, true, false );

    // Draw all editor camera components.
    ComponentCamera* pCamera = nullptr;
    for( unsigned int i=0; i<pEditorState->m_pEditorCamera->GetComponentCount(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->GetComponentByIndex( i ) );
        if( pCamera )
        {
            g_pComponentSystemManager->DrawMousePickerFrame( pCamera, &pCamera->m_Camera3D.m_matProj, &pCamera->m_Camera3D.m_matView, g_pEngineCore->GetShader_TintColor() );
            g_pRenderer->ClearBuffers( false, true, false );
        }
    }

    pEditorState->m_pMousePickerFBO->Unbind( true );

    pEditorState->m_pTransformGizmo->ScaleGizmosForMousePickRendering( false );
}

unsigned int EditorInterface::GetIDAtPixel(unsigned int x, unsigned int y, bool createNewBitmap, bool includeTransformGizmo)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    if( pEditorState->m_pMousePickerFBO->IsFullyLoaded() == false )
        return 0;

    if( x >= pEditorState->m_pMousePickerFBO->GetWidth() ||
        y >= pEditorState->m_pMousePickerFBO->GetHeight() )
        return 0;

    if( createNewBitmap )
    {
        if( includeTransformGizmo == false )
        {
            // Hide the transform gizmo.
            pEditorState->m_pTransformGizmo->Hide();
        }

        RenderObjectIDsToFBO();
    }

    // Bind our FBO so we can render sample from it.
    pEditorState->m_pMousePickerFBO->Bind( true );

    // Find the first camera again.
    ComponentCamera* pCamera = nullptr;
    for( unsigned int i=0; i<pEditorState->m_pEditorCamera->GetComponentCount(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->GetComponentByIndex( i ) );
        break;
    }

    MyAssert( pCamera );
    if( pCamera == nullptr )
        return 0;

    // Get a pixel from the FBO... use m_WindowStartX/m_WindowStartY from any camera component.
    unsigned char pixel[4];
    g_pRenderer->ReadPixels( x - (unsigned int)pCamera->m_Viewport.GetX(), y - (unsigned int)pCamera->m_Viewport.GetY(),
                             1, 1, MyRE::PixelFormat_RGBA, MyRE::PixelDataType_UByte, pixel );

    pEditorState->m_pMousePickerFBO->Unbind( true );

    // Convert the RGB value to an id.
    uint64 id = (uint64)pixel[0] | (uint64)pixel[1]<<8 | (uint64)pixel[2]<<16 | (uint64)pixel[3]<<24;
    if( id != 0 )
        id = UINT_MAX - id;
    id = (((uint64)UINT_MAX+1) * (id % 641) + id) / 641; // 1, 641, 6700417, 4294967297,
    //LOGInfo( LOGTag, "pixel - %d, %d, %d, %d - id - %d\n", pixel[0], pixel[1], pixel[2], pixel[3], id );

    return (unsigned int)id;
}

GameObject* EditorInterface::GetObjectAtPixel(unsigned int x, unsigned int y, bool createNewBitmap, bool includeTransformGizmo)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    unsigned int id = GetIDAtPixel( x, y, createNewBitmap, includeTransformGizmo );

    SceneID sceneID = (SceneID)(id / 100000);
    id = id % 100000;

    // Find the object clicked on.
    GameObject* pGameObject = g_pComponentSystemManager->FindGameObjectByID( sceneID, (unsigned int)id );

    // If we didn't click on something, check if it's the transform gizmo.
    //   Has to be checked manually since they are unmanaged.
    if( pGameObject == nullptr )
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
    unsigned int fboWidth = pEditorState->m_pMousePickerFBO->GetWidth();
    unsigned int fboHeight = pEditorState->m_pMousePickerFBO->GetHeight();

    if( sx >= fboWidth || ex >= fboWidth || sy >= fboHeight || ey >= fboHeight )
        return;

    int smallerx = sx > ex ? ex : sx;
    int biggerx = sx < ex ? ex : sx;

    int smallery = sy > ey ? ey : sy;
    int biggery = sy < ey ? ey : sy;

    //LOGInfo( LOGTag, "group selecting: %d,%d  %d,%d\n", smallerx, smallery, biggerx, biggery );

    // Render to the FBO.
    RenderObjectIDsToFBO();

    if( pEditorState->m_pMousePickerFBO->IsFullyLoaded() == false )
        return;

    // Bind our FBO so we can sample from it.
    pEditorState->m_pMousePickerFBO->Bind( true );

    // Find the first camera again.
    ComponentCamera* pCamera = nullptr;
    for( unsigned int i=0; i<pEditorState->m_pEditorCamera->GetComponentCount(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->GetComponentByIndex( i ) );
        break;
    }

    MyAssert( pCamera );
    if( pCamera == nullptr )
        return;

    // Get pixels from the FBO... use m_WindowStartX/m_WindowStartY from any camera component.
    unsigned char* pixels = MyNew unsigned char[fboWidth * fboHeight * 4];
    g_pRenderer->ReadPixels( 0, 0, fboWidth, fboHeight, MyRE::PixelFormat_RGBA, MyRE::PixelDataType_UByte, pixels );
    pEditorState->m_pMousePickerFBO->Unbind( true );

    bool controlHeld = pEditorState->m_ModifierKeyStates & MODIFIERKEY_Control ? true : false;

    // If user isn't holding control, then clear objects and items selected in tree.
    if( controlHeld == false )
        pEditorState->ClearSelectedObjectsAndComponents();

    //unsigned int pixelsbeingtested = (biggerx - smallerx) * (biggery - smallery);

    // Check whether or not the first object clicked on was selected, we only care if control is held.
    bool firstObjectWasSelected = false;
    if( controlHeld )
    {
        unsigned int offset = (sy*fboWidth + sx)*4;
        uint64 id = (uint64)pixels[offset+0] | (uint64)pixels[offset+1]<<8 | (uint64)pixels[offset+2]<<16 | (uint64)pixels[offset+3]<<24;
        if( id != 0 )
            id = UINT_MAX - id;
        id = (((uint64)UINT_MAX+1) * (id % 641) + id) / 641; // 1, 641, 6700417, 4294967297,

        SceneID sceneid = (SceneID)(id / 100000);
        id = id % 100000;

        // if the object's not already selected, select it.
        GameObject* pObject = g_pComponentSystemManager->FindGameObjectByID( sceneid, (unsigned int)id );

        if( pObject && pEditorState->IsGameObjectSelected( pObject ) )
            firstObjectWasSelected = true;
    }

    for( int y=smallery; y<=biggery; y++ )
    {
        for( int x=smallerx; x<=biggerx; x++ )
        {
            unsigned int offset = (y*fboWidth + x)*4;
            uint64 id = (uint64)pixels[offset+0] | (uint64)pixels[offset+1]<<8 | (uint64)pixels[offset+2]<<16 | (uint64)pixels[offset+3]<<24;
            if( id != 0 )
                id = UINT_MAX - id;
            id = (((uint64_t)UINT_MAX+1) * (id % 641) + id) / 641; // 1, 641, 6700417, 4294967297,

            SceneID sceneid = (SceneID)(id / 100000);
            id = id % 100000;

            // If the object's not already selected, select it.
            GameObject* pObject = g_pComponentSystemManager->FindGameObjectByID( sceneid, (unsigned int)id );

            if( pObject )
            {
                // When selecting with mouse, don't allow selection of subobjects of a prefab, always pick the root of the prefab instance.
                if( pObject->GetPrefabRef()->GetPrefab() != nullptr )
                {
                    pObject = pObject->FindRootGameObjectOfPrefabInstance();
                }

                bool objectselected = pEditorState->IsGameObjectSelected( pObject );

                // If we're selecting objects, then select the unselected objects.
                if( firstObjectWasSelected == false )
                {
                    if( objectselected == false )
                    {
                        // Select the object.
                        pEditorState->SelectGameObject( pObject );
                    }
                }
                else if( controlHeld ) // If the first object was already selected, deselect all dragged if control is held.
                {
                    if( objectselected == true )
                    {
                        pEditorState->UnselectGameObject( pObject );
                    }
                }
            }
        }
    }

    //LOGInfo( LOGTag, "Done selecting objects.\n" );

    delete[] pixels;
}
