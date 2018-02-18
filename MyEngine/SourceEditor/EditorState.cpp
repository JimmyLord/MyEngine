//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_EDITOR

const char* EditorIconFilenames[EditorIcon_NumIcons] =
{
    "Data/DataEngine/Textures/IconLight.png",
    "Data/DataEngine/Textures/IconCamera.png"
};

EditorState::EditorState()
{
    m_ModifierKeyStates = 0;
    m_EditorActionState = EDITORACTIONSTATE_None;
    for( int i=0; i<3; i++ )
    {
        m_MouseDownLocation[i].Set( -1, -1 );
        m_HasMouseMovedSinceButtonPressed[i] = false;
    }
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

    m_DistanceTranslated = Vector3( 0 );
    m_AmountScaled = Vector3( 1 );
    m_DistanceRotated = Vector3( 0 );
    m_TransformedInLocalSpace = true;

    m_CameraState = EditorCameraState_Default;
    m_pGameObjectCameraIsFollowing = 0;
    m_OffsetFromObject.SetIdentity();

    for( int i=0; i<EditorIcon_NumIcons; i++ )
    {
        // create all icons as 1x1 sprites, with center pivots. Sprites are facing positive z-axis.
        m_pEditorIcons[i] = MyNew MySprite( true );
        m_pEditorIcons[i]->Create( "EditorIcon", 1, 1, 0, 1, 0, 1, Justify_Center, false, true );

        MaterialDefinition* pMaterial = m_pEditorIcons[i]->GetMaterial();

        pMaterial->SetBlendType( MaterialBlendType_On );

        MyFileObject* pFile = g_pEngineFileManager->RequestFile_UntrackedByScene( EditorIconFilenames[i] );
#if MYFW_EDITOR
        pFile->MemoryPanel_Hide();
#endif
        TextureDefinition* pTexture = g_pTextureManager->CreateTexture( pFile );
#if MYFW_EDITOR
        pTexture->MemoryPanel_Hide();
#endif
        pFile->Release();

        pMaterial->SetTextureColor( pTexture );
        pTexture->Release();

        ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByFilename( "Data/DataEngine/Shaders/Shader_TextureTint.glsl" );
        if( pShaderGroup != 0 )
        {
            pMaterial->SetShader( pShaderGroup );
        }
        else
        {
            MyFileObject* pFile = g_pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_TextureTint.glsl" );
#if MYFW_EDITOR
            pFile->MemoryPanel_Hide();
#endif
            MyAssert( pFile->IsA( "MyFileShader" ) );
            if( pFile->IsA( "MyFileShader" ) )
            {
                MyFileObjectShader* pShaderFile = (MyFileObjectShader*)pFile;
                pShaderGroup = MyNew ShaderGroup( pShaderFile );
                pMaterial->SetShader( pShaderGroup );
                pShaderGroup->Release();
            }
            pFile->Release();
        }
    }
}

EditorState::~EditorState()
{
    SAFE_RELEASE( m_pDebugViewFBO );
    SAFE_RELEASE( m_pMousePickerFBO );

    SAFE_DELETE( m_pTransformGizmo );
    SAFE_DELETE( m_p3DGridPlane );
    SAFE_DELETE( m_pEditorCamera );

    for( int i=0; i<EditorIcon_NumIcons; i++ )
    {
        SAFE_RELEASE( m_pEditorIcons[i] );
    }
}

ComponentCamera* EditorState::GetEditorCamera()
{
    MyAssert( m_pEditorCamera );
    return (ComponentCamera*)m_pEditorCamera->GetFirstComponentOfBaseType( BaseComponentType_Camera );
}

void EditorState::ClearEditorState(bool clearselectedobjectandcomponents)
{
    m_pGameObjectCameraIsFollowing = 0;

    if( clearselectedobjectandcomponents )
    {
        m_pSelectedObjects.clear();
        m_pSelectedComponents.clear();
    }

    ClearConstraint();
}

void EditorState::OnFocusLost()
{
    // Cancel whatever operation the transform gizmo was doing.
    m_pTransformGizmo->CancelCurrentOperation( this );
    m_EditorActionState = EDITORACTIONSTATE_None;
}

void EditorState::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height)
{
    m_EditorWindowRect.Set( startx, starty, width, height );

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

void EditorState::SelectGameObject(GameObject* pObject)
{
    m_pSelectedObjects.push_back( pObject );
}

void EditorState::UnselectGameObject(GameObject* pObject)
{
    m_pSelectedObjects.erase( std::remove( m_pSelectedObjects.begin(), m_pSelectedObjects.end(), pObject ) );
}

bool EditorState::IsGameObjectSelected(GameObject* pObject)
{
    if( std::find( m_pSelectedObjects.begin(), m_pSelectedObjects.end(), pObject ) != m_pSelectedObjects.end() )
        return true;

    return false;
}

void EditorState::DeleteSelectedObjects()
{
    if( m_pSelectedObjects.size() > 0 )
        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DeleteObjects( m_pSelectedObjects ) );
}

void EditorState::SelectComponent(ComponentBase* pComponent)
{
    m_pSelectedComponents.push_back( pComponent );
}

void EditorState::UnselectComponent(ComponentBase* pComponent)
{
    m_pSelectedComponents.erase( std::remove( m_pSelectedComponents.begin(), m_pSelectedComponents.end(), pComponent ) );
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

void EditorState::ClearKeyAndActionStates()
{
    m_ModifierKeyStates = 0;
    m_EditorActionState = EDITORACTIONSTATE_None;
}

void EditorState::ClearSelectedObjectsAndComponents()
{
    m_pSelectedObjects.clear();
    m_pSelectedComponents.clear();

#if MYFW_USING_WX
    g_pPanelObjectList->SelectObject( 0 );
    g_pPanelWatch->ClearAllVariables();
#endif
}

void EditorState::LockCameraToGameObject(GameObject* pGameObject)
{
    if( pGameObject == 0 )
    {
        m_CameraState = EditorCameraState_Default;

        LOGInfo( LOGTag, "Unlocked camera\n" );
    }
    else
    {
        m_CameraState = EditorCameraState_LockedToObject;
        m_pGameObjectCameraIsFollowing = pGameObject;

        MyMatrix GOTransform = *m_pGameObjectCameraIsFollowing->GetTransform()->GetWorldTransform();
        GOTransform.Inverse();
        MyMatrix CamTransform = *m_pEditorCamera->GetTransform()->GetWorldTransform();

        m_OffsetFromObject = GOTransform * CamTransform;

        LOGInfo( LOGTag, "Locked camera to %s\n", pGameObject->GetName() );

        //Vector3 offsetpos = m_OffsetFromObject.GetTranslation();
        //Vector3 offsetrot = m_OffsetFromObject.GetEulerAngles();

        //LOGInfo( LOGTag, "Lock: (%0.0f,%0.0f,%0.0f) (%0.2f,%0.2f,%0.2f)\n",
        //         offsetpos.x, offsetpos.y, offsetpos.z,
        //         offsetrot.x, offsetrot.y, offsetrot.z );
    }
}

void EditorState::UpdateCamera(double TimePassed)
{
    if( m_CameraState == EditorCameraState_LockedToObject )
    {
        MyAssert( m_pGameObjectCameraIsFollowing );

        if( m_pGameObjectCameraIsFollowing )
        {
            m_pGameObjectCameraIsFollowing->GetTransform()->UpdateTransform();

            MyMatrix GOTransform = *m_pGameObjectCameraIsFollowing->GetTransform()->GetWorldTransform();
            MyMatrix newtransform = GOTransform * m_OffsetFromObject;

            m_pEditorCamera->GetTransform()->SetWorldTransform( &newtransform );
        }
    }
}

bool EditorState::HasMouseMovedSinceButtonPressed(int buttonid)
{
    MyAssert( buttonid >= 0 && buttonid <= 2 );

    return m_HasMouseMovedSinceButtonPressed[buttonid];
}

// can't really do it this way since the tree can contain different types of objects.
//void EditorState::SyncFromObjectPanelSelectedItems()
//{
//    wxArrayTreeItemIds selecteditems;
//    unsigned int numselected = (unsigned int)g_pPanelObjectList->m_pTree_Objects->GetSelections( selecteditems );
//
//    m_pSelectedObjects.clear();
//    m_pSelectedComponents.clear();
//
//    for( unsigned int i=0; i<numselected; i++ )
//    {
//        wxTreeItemId id = selecteditems[i].GetID();
//        TreeItemDataGenericObjectInfo* pData = (TreeItemDataGenericObjectInfo*)g_pPanelObjectList->m_pTree_Objects->GetItemData( id );
//
//        if( dynamic_cast<GameObject*>( pData->m_pObject ) != 0 )
//            m_pSelectedObjects.push_back( (GameObject*)pData->m_pObject );
//    }
//}

#endif //MYFW_EDITOR
