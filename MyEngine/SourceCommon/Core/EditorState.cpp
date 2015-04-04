//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

EditorState::EditorState()
{
    m_ModifierKeyStates = 0;
    m_EditorActionState = EDITORACTIONSTATE_None;
    m_MouseLeftDownLocation.Set( -1, -1 );
    m_MouseRightDownLocation.Set( -1, -1 );
    m_MouseMiddleDownLocation.Set( -1, -1 );
    m_LastMousePosition.Set( -1, -1 );
    m_CurrentMousePosition.Set( -1, -1 );

    m_pDebugViewFBO = 0;
    m_pMousePickerFBO = 0;

    m_p3DGridPlane = 0;
    m_pEditorCamera = 0;

    m_pTransformGizmo = MyNew TransformGizmo();

    m_MousePicker_PickedBody = 0;
    m_MousePicker_PickConstraint = 0;
    m_MousePicker_OldPickingDist = 0;
}

EditorState::~EditorState()
{
    SAFE_RELEASE( m_pDebugViewFBO );
    SAFE_RELEASE( m_pMousePickerFBO );

    SAFE_DELETE( m_pTransformGizmo );
    SAFE_DELETE( m_p3DGridPlane );
    SAFE_DELETE( m_pEditorCamera );
}

ComponentCamera* EditorState::GetEditorCamera()
{
    assert( m_pEditorCamera );
    return (ComponentCamera*)m_pEditorCamera->GetFirstComponentOfBaseType( BaseComponentType_Camera );
}

void EditorState::UnloadScene()
{
    m_pSelectedObjects.clear();
    m_pSelectedComponents.clear();
    //m_pSelectedGameObject = 0;
    ClearConstraint();
}

void EditorState::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height)
{
    if( m_pDebugViewFBO )
    {
        if( m_pDebugViewFBO->m_TextureWidth < width || m_pDebugViewFBO->m_TextureHeight < height )
        {
            // the FBO will be recreated during the texturemanager tick.
            g_pTextureManager->InvalidateFBO( m_pDebugViewFBO );
            m_pDebugViewFBO->Setup( width, height, GL_NEAREST, GL_NEAREST, true, 32, false );
        }
        else
        {
            m_pDebugViewFBO->m_Width = width;
            m_pDebugViewFBO->m_Height = height;
        }
    }

    if( m_pMousePickerFBO )
    {
        if( m_pMousePickerFBO->m_TextureWidth < width || m_pMousePickerFBO->m_TextureHeight < height )
        {
            // the FBO will be recreated during the texturemanager tick.
            g_pTextureManager->InvalidateFBO( m_pMousePickerFBO );
            m_pMousePickerFBO->Setup( width, height, GL_NEAREST, GL_NEAREST, true, 32, false );
        }
        else
        {
            m_pMousePickerFBO->m_Width = width;
            m_pMousePickerFBO->m_Height = height;
        }
    }
}

void EditorState::ClearConstraint()
{
    if( m_MousePicker_PickConstraint && g_pBulletWorld->m_pDynamicsWorld )
    {
        g_pBulletWorld->m_pDynamicsWorld->removeConstraint( m_MousePicker_PickConstraint );
        delete m_MousePicker_PickConstraint;
        //printf("removed constraint %i",gPickingConstraintId);
        m_MousePicker_PickConstraint = 0;
        m_MousePicker_PickedBody->forceActivationState( ACTIVE_TAG );
        m_MousePicker_PickedBody->setDeactivationTime( 0.0f );
        m_MousePicker_PickedBody = 0;

        //action = -1;
    }
}

bool EditorState::IsObjectSelected(GameObject* pObject)
{
    for( unsigned int i=0; i<m_pSelectedObjects.size(); i++ )
    {
        if( m_pSelectedObjects[i] == pObject )
            return true;
    }

    return false;
}

bool EditorState::IsComponentSelected(ComponentBase* pComponent)
{
    for( unsigned int i=0; i<m_pSelectedComponents.size(); i++ )
    {
        if( m_pSelectedComponents[i] == pComponent )
            return true;
    }

    return false;
}

void EditorState::ClearSelectedObjectsAndComponents()
{
    m_pSelectedObjects.clear();
    m_pSelectedComponents.clear();

    g_pPanelObjectList->SelectObject( 0 );
    g_pPanelWatch->ClearAllVariables();
}
