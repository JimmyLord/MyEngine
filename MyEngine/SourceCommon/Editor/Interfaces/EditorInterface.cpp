//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

EditorInterface::EditorInterface()
{
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

void EditorInterface::OnDrawFrame(unsigned int canvasid)
{
    MyAssert( canvasid == 1 );

    EditorState* pEditorState = g_pEngineCore->m_pEditorState;

    MyAssert( pEditorState->m_pEditorCamera );
    if( pEditorState->m_pEditorCamera )
    {
        // draw scene from editor camera and editorFG camera
        for( unsigned int i=0; i<pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
        {
            ComponentCamera* pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->m_Components[i] );

            if( pCamera )
                pCamera->OnDrawFrame();
        }
    }

    // Draw our mouse picker frame over the screen
    if( g_pEngineCore->m_Debug_DrawMousePickerFBO && g_GLCanvasIDActive == 1 )
    {
        if( g_pEngineCore->m_pDebugQuadSprite == 0 )
            g_pEngineCore->m_pDebugQuadSprite = MyNew MySprite( false );

        g_pEngineCore->m_pDebugQuadSprite->CreateInPlace( "debug", 0.75f, 0.75f, 0.5f, 0.5f, 0, 1, 1, 0, Justify_Center, false );
        g_pEngineCore->m_pMaterial_MousePicker->SetTextureColor( pEditorState->m_pMousePickerFBO->m_pColorTexture );
        g_pEngineCore->m_pDebugQuadSprite->SetMaterial( g_pEngineCore->m_pMaterial_MousePicker );
        g_pEngineCore->m_pDebugQuadSprite->Draw( 0 );
    }
}

void EditorInterface::SetModifierKeyStates(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->m_pEditorState;

    if( keyaction == GCBA_Down )
    {
        if( keycode == MYKEYCODE_LCTRL )    pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Control;
        if( keycode == MYKEYCODE_LALT )     pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Alt;
        if( keycode == MYKEYCODE_LSHIFT )   pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Shift;
        if( keycode == ' ' )                pEditorState->m_ModifierKeyStates |= MODIFIERKEY_Space;
    }

    if( mouseaction != -1 )
    {
        pEditorState->m_CurrentMousePosition.Set( x, y );
        //pEditorState->m_LastMousePosition.Set( x, y );

        if( mouseaction == GCBA_Down )
        {
            if( id == 0 )
            {
                pEditorState->m_MouseLeftDownLocation = pEditorState->m_CurrentMousePosition;
                pEditorState->m_ModifierKeyStates |= MODIFIERKEY_LeftMouse;
            }
            if( id == 1 )
            {
                pEditorState->m_MouseRightDownLocation = pEditorState->m_CurrentMousePosition;
                pEditorState->m_ModifierKeyStates |= MODIFIERKEY_RightMouse;
            }
            if( id == 2 )
            {
                pEditorState->m_MouseMiddleDownLocation = pEditorState->m_CurrentMousePosition;
                pEditorState->m_ModifierKeyStates |= MODIFIERKEY_MiddleMouse;
            }
        }
    }
}

void EditorInterface::ClearModifierKeyStates(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->m_pEditorState;

    if( keyaction == GCBA_Up )
    {
        if( keycode == MYKEYCODE_LCTRL )    pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Control;
        if( keycode == MYKEYCODE_LALT )     pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Alt;
        if( keycode == MYKEYCODE_LSHIFT )   pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Shift;
        if( keycode == ' ' )                pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_Space;
    }

    if( mouseaction != -1 )
    {
        if( mouseaction == GCBA_Up )
        {
            if( id == 0 )
            {
                pEditorState->m_MouseLeftDownLocation = Vector2( -1, -1 );
                pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_LeftMouse;
            }
            else if( id == 1 )
            {
                pEditorState->m_MouseRightDownLocation = Vector2( -1, -1 );
                pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_RightMouse;
            }
            else if( id == 2 )
            {
                pEditorState->m_MouseMiddleDownLocation = Vector2( -1, -1 );
                pEditorState->m_ModifierKeyStates &= ~MODIFIERKEY_MiddleMouse;
            }
        }
    }

    pEditorState->m_LastMousePosition = pEditorState->m_CurrentMousePosition;
}

bool EditorInterface::HandleInputForEditorCamera(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = g_pEngineCore->m_pEditorState;

    // if mouse message. down, up or dragging.
    if( mouseaction != -1 )
    {
        unsigned int mods = pEditorState->m_ModifierKeyStates;

        // get the editor camera's local transform.
        ComponentCamera* pCamera = pEditorState->GetEditorCamera();
        MyMatrix* matCamera = pCamera->m_pComponentTransform->GetLocalTransform();

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
                matCamera->TranslatePreRotScale( dir * speed * g_pEngineCore->m_TimePassedUnpausedLastFrame );
        }

        // if left mouse down, reset the transform gizmo tool.
        if( mouseaction == GCBA_Down && id == 0 )
        {
            pEditorState->m_pTransformGizmo->m_LastIntersectResultIsValid = false;
        }

        // if space is held, left button will pan the camera around.  or just middle button
        if( ( (mods & MODIFIERKEY_LeftMouse) && (mods & MODIFIERKEY_Space) ) || (mods & MODIFIERKEY_MiddleMouse) )
        {
            Vector2 dir = pEditorState->m_LastMousePosition - pEditorState->m_CurrentMousePosition;

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
            // get the direction the mouse moved
            Vector2 dir = pEditorState->m_CurrentMousePosition - pEditorState->m_LastMousePosition;

            if( dir.LengthSquared() > 0 )
            {
                Vector3 pivot;
                float distancefrompivot;

                // different pivot and distance from pivot depending if Alt is held.
                if( mods & MODIFIERKEY_Alt && pEditorState->m_pSelectedObjects.size() > 0 && pEditorState->m_pTransformGizmo->m_pTransformGizmos[0] )
                {
                    // pivot around the transform gizmo
                    pivot = pEditorState->m_pTransformGizmo->m_pTransformGizmos[0]->m_pComponentTransform->GetWorldTransform()->GetTranslation();
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
                float degreesperpixel = 1.0f;

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

        // pull the pos/angle from the world matrix and update the values for the watch window.
        pCamera->m_pComponentTransform->UpdateTransform();
        pCamera->m_pComponentTransform->UpdateLocalSRT();
    }

    // handle editor keys
    if( keyaction == GCBA_Held )
    {
        // get the editor camera's local transform.
        ComponentCamera* pCamera = pEditorState->GetEditorCamera();
        MyMatrix* matCamera = pCamera->m_pComponentTransform->GetLocalTransform();

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
            matCamera->TranslatePreRotScale( dir * speed * g_pEngineCore->m_TimePassedUnpausedLastFrame );

        pCamera->m_pComponentTransform->UpdateLocalSRT();
        pCamera->m_pComponentTransform->UpdateTransform();
    }

    return false;
}

void EditorInterface::RenderObjectIDsToFBO()
{
    EditorState* pEditorState = g_pEngineCore->m_pEditorState;

    if( pEditorState->m_pMousePickerFBO->m_FullyLoaded == false )
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
    for( unsigned int i=0; i<pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->m_Components[i] );
        if( pCamera )
        {
            g_pComponentSystemManager->DrawMousePickerFrame( pCamera, &pCamera->m_Camera3D.m_matViewProj, g_pEngineCore->m_pShader_TintColor );
            glClear( GL_DEPTH_BUFFER_BIT );
        }
    }

    pEditorState->m_pMousePickerFBO->Unbind( true );

    pEditorState->m_pTransformGizmo->ScaleGizmosForMousePickRendering( false );
}

unsigned int EditorInterface::GetIDAtPixel(unsigned int x, unsigned int y, bool createnewbitmap)
{
    EditorState* pEditorState = g_pEngineCore->m_pEditorState;

    if( pEditorState->m_pMousePickerFBO->m_FullyLoaded == false )
        return 0;

    if( createnewbitmap )
    {
        RenderObjectIDsToFBO();
    }

    // bind our FBO so we can render sample from it.
    pEditorState->m_pMousePickerFBO->Bind( true );

    // Find the first camera again.
    ComponentCamera* pCamera = 0;
    for( unsigned int i=0; i<pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->m_Components[i] );
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

GameObject* EditorInterface::GetObjectAtPixel(unsigned int x, unsigned int y, bool createnewbitmap)
{
    EditorState* pEditorState = g_pEngineCore->m_pEditorState;

    unsigned int id = GetIDAtPixel( x, y, createnewbitmap );

    unsigned int sceneid = (unsigned int)(id / 100000);
    id = id % 100000;

    // find the object clicked on.
    GameObject* pGameObject = g_pComponentSystemManager->FindGameObjectByID( sceneid, (unsigned int)id );

    // if we didn't click on something, check if it's the transform gizmo.
    //   has to be checked manually since they are unmanaged.
    if( pGameObject == 0 )
    {
        for( int i=0; i<3; i++ )
        {
            if( pEditorState->m_pTransformGizmo->m_pTransformGizmos[i]->GetID() == id )
            {
                pGameObject = pEditorState->m_pTransformGizmo->m_pTransformGizmos[i];
            }
        }
    }

    return pGameObject;
}

void EditorInterface::SelectObjectsInRectangle(unsigned int sx, unsigned int sy, unsigned int ex, unsigned int ey)
{
    EditorState* pEditorState = g_pEngineCore->m_pEditorState;

    int smallerx = sx > ex ? ex : sx;
    int biggerx = sx < ex ? ex : sx;

    int smallery = sy > ey ? ey : sy;
    int biggery = sy < ey ? ey : sy;

    //LOGInfo( LOGTag, "group selecting: %d,%d  %d,%d\n", smallerx, smallery, biggerx, biggery );

    // render to the FBO.
    RenderObjectIDsToFBO();

    // bind our FBO so we can sample from it.
    pEditorState->m_pMousePickerFBO->Bind( true );

    // Find the first camera again.
    ComponentCamera* pCamera = 0;
    for( unsigned int i=0; i<pEditorState->m_pEditorCamera->m_Components.Count(); i++ )
    {
        pCamera = dynamic_cast<ComponentCamera*>( pEditorState->m_pEditorCamera->m_Components[i] );
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
    g_pPanelObjectList->m_UpdatePanelWatchOnSelection = false;

    //unsigned int pixelsbeingtested = (biggerx - smallerx) * (biggery - smallery);

    // check whether or not the first object clicked on was selected, we only care if control is held.
    bool firstobjectwasselected = false;
    if( controlheld )
    {
        unsigned int offset = (sy*fbowidth + sx)*4;
        unsigned long long id = pixels[offset+0] + pixels[offset+1]*256 + pixels[offset+2]*256*256 + pixels[offset+3]*256*256*256;
        id = (((uint64_t)UINT_MAX+1) * (id % 641) + id) / 641; // 1, 641, 6700417, 4294967297,

        unsigned int sceneid = (unsigned int)id / 100000;
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

            unsigned int sceneid = (unsigned int)id / 100000;
            id = id % 100000;

            // if the object's not already selected, select it.
            GameObject* pObject = g_pComponentSystemManager->FindGameObjectByID( sceneid, (unsigned int)id );

            if( pObject )
            {
                bool objectselected = pEditorState->IsGameObjectSelected( pObject );

                // if we're selecting objects, then select the unselected objects.
                if( firstobjectwasselected == false )
                {
                    if( objectselected == false )
                    {
                        // select the object
                        pEditorState->SelectGameObject( pObject );

                        // select the object in the object tree.
                        g_pPanelObjectList->SelectObject( pObject );
                    }
                }
                else if( controlheld ) // if the first object was already selected, deselect all dragged if control is held.
                {
                    if( objectselected == true )
                    {
                        pEditorState->UnselectGameObject( pObject );
                        g_pPanelObjectList->UnselectObject( pObject );
                    }
                }
            }
        }
    }

    g_pPanelObjectList->m_UpdatePanelWatchOnSelection = true;
    UpdatePanelWatchWithSelectedItems(); // will reset and update pEditorState->m_pSelectedObjects

    //LOGInfo( LOGTag, "Done selecting objects.\n" );

    delete[] pixels;
}
