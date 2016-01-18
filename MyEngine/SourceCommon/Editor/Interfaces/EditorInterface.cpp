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
        MyMatrix* matLocalCamera = pCamera->m_pComponentTransform->GetLocalTransform();

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
                matLocalCamera->TranslatePreRotScale( dir * speed * g_pEngineCore->m_TimePassedUnpausedLastFrame );
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
                matLocalCamera->TranslatePreRotScale( dir * 0.05f );
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
        // if left or right mouse is down, rotate the camera.
        else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_None &&
                 //( (mods & MODIFIERKEY_LeftMouse) || (mods & MODIFIERKEY_RightMouse) ) )
                 (mods & MODIFIERKEY_RightMouse) )
        {
            // rotate the camera around selected object or a point 10 units in front of the camera.
            Vector2 dir = pEditorState->m_CurrentMousePosition - pEditorState->m_LastMousePosition;

            Vector3 pivot;
            float distancefrompivot;

            //if( mods & MODIFIERKEY_LeftMouse && pEditorState->m_pSelectedObjects.size() > 0 && pEditorState->m_pTransformGizmo->m_pTransformGizmos[0] )//pEditorState->m_pSelectedObjects[0] )
            if( mods & MODIFIERKEY_Alt && pEditorState->m_pSelectedObjects.size() > 0 && pEditorState->m_pTransformGizmo->m_pTransformGizmos[0] )
            {
                // pivot around the transform gizmo
                pivot = pEditorState->m_pTransformGizmo->m_pTransformGizmos[0]->m_pComponentTransform->m_Transform.GetTranslation();
                //pEditorState->m_pSelectedObjects[0]->m_pComponentTransform->m_Transform.GetTranslation();
                distancefrompivot = (matLocalCamera->GetTranslation() - pivot).Length();
            }
            else
            {
                if( mods & MODIFIERKEY_RightMouse )
                {
                    // pivot on the camera, just change rotation.
                    pivot = matLocalCamera->GetTranslation();
                    distancefrompivot = 0;
                }
                else
                {
                    // TODO: try to pivot from distance of object at mouse
                    MyMatrix mattemp = *matLocalCamera;
#if MYFW_RIGHTHANDED
                    mattemp.TranslatePreRotScale( 0, 0, -10 );
#else
                    mattemp.TranslatePreRotScale( 0, 0, 10 );
#endif
                    pivot = mattemp.GetTranslation();
                    distancefrompivot = 10;
                }
            }

            if( dir.LengthSquared() > 0 )
            {
                //LOGInfo( LOGTag, "dir (%0.2f, %0.2f)\n", dir.x, dir.y );

                //Vector3 pos = matLocalCamera->GetTranslation();
                Vector3 angle = pCamera->m_pComponentTransform->GetLocalRotation();

                // todo: make this degrees per inch
                float degreesperpixel = 1.0f;

                dir.Normalize();

#if MYFW_RIGHTHANDED
                angle.y += dir.x * degreesperpixel;
                angle.x -= dir.y * degreesperpixel;
#else
                angle.y -= dir.x * degreesperpixel;
                angle.x += dir.y * degreesperpixel;
#endif
                MyClamp( angle.x, -90.0f, 90.0f );

                matLocalCamera->SetIdentity();
#if MYFW_RIGHTHANDED
                matLocalCamera->Translate( 0, 0, distancefrompivot );
#else
                matLocalCamera->Translate( 0, 0, -distancefrompivot );
#endif
                matLocalCamera->Rotate( angle.x, 1, 0, 0 );
                matLocalCamera->Rotate( angle.y, 0, 1, 0 );
                matLocalCamera->Translate( pivot );
            }
        }

        // pull the pos/angle from the local matrix and update the values for the watch window.
        pCamera->m_pComponentTransform->UpdatePosAndRotFromLocalMatrix();
    }

    // handle editor keys
    if( keyaction == GCBA_Held )
    {
        // get the editor camera's local transform.
        ComponentCamera* pCamera = pEditorState->GetEditorCamera();
        MyMatrix* matLocalCamera = pCamera->m_pComponentTransform->GetLocalTransform();

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
            matLocalCamera->TranslatePreRotScale( dir * speed * g_pEngineCore->m_TimePassedUnpausedLastFrame );
    }

    return false;
}
