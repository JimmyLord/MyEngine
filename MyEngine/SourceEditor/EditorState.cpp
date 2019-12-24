//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorState.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/EngineFileManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"
#include "../SourceEditor/Commands/EngineEditorCommands.h"
#include "../SourceEditor/TransformGizmo.h"
#include "../SourceEditor/Documents/EditorDocument.h"

#if MYFW_EDITOR

const char* EditorIconFilenames[EditorIcon_NumIcons] =
{
    "Data/DataEngine/Textures/IconLight.png",
    "Data/DataEngine/Textures/IconCamera.png"
};

EditorState::EditorState(EngineCore* pEngineCore)
{
    m_pEngineCore = pEngineCore;

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

    m_pTransformGizmo = MyNew TransformGizmo( m_pEngineCore );

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
        m_pEditorIcons[i] = nullptr;
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

    for( uint32 i=0; i<m_pOpenDocuments.size(); i++ )
    {
        delete m_pOpenDocuments[i];
    }
}

void EditorState::Init()
{
    ShaderGroup* pShader = m_pEngineCore->GetManagers()->GetShaderGroupManager()->FindShaderGroupByName( "Shader_TintColor" );

    // Load all the editor icons (Lights/Cameras/Etc)
    for( int i=0; i<EditorIcon_NumIcons; i++ )
    {
        MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();
        BufferManager* pBufferManager = m_pEngineCore->GetManagers()->GetBufferManager();

        MaterialDefinition* pMaterial = pMaterialManager->CreateMaterial();
        pMaterial->SetShader( pShader );

        // Create all icons as 1x1 sprites, with center pivots. Sprites are facing positive z-axis.
        m_pEditorIcons[i] = MyNew MySprite();
        m_pEditorIcons[i]->SetMaterial( pMaterial );
        m_pEditorIcons[i]->Create( pBufferManager, "EditorIcon", 1, 1, 0, 1, 0, 1, Justify_Center, false, true );

        // Now that it's on the sprite, release our reference to the material.
        pMaterial->Release();

        // Icons can't have tranparency, shader will 'discard' fragments if alpha is less than 1.
        pMaterial->SetBlendType( MyRE::MaterialBlendType_Off );

        // Load the icon png file, create a texture, set it to the material.  
        EngineFileManager* pEngineFileManager = static_cast<EngineFileManager*>( m_pEngineCore->GetManagers()->GetFileManager() );
        MyFileObject* pFile = pEngineFileManager->RequestFile_UntrackedByScene( EditorIconFilenames[i] );
        TextureDefinition* pTexture = m_pEngineCore->GetManagers()->GetTextureManager()->CreateTexture( pFile );
        pMaterial->SetTextureColor( pTexture );

        // Free the extra ref's from the requests/creates and hide them from the editor front end.
        pFile->Release();
        pTexture->Release();
        pFile->MemoryPanel_Hide();
        pTexture->MemoryPanel_Hide();

        // Assign the shader to the new material, reuse the shader if already loaded.
        ShaderGroup* pShaderGroup = m_pEngineCore->GetManagers()->GetShaderGroupManager()->FindShaderGroupByFilename( "Data/DataEngine/Shaders/Shader_TextureTintDiscard.glsl" );
        if( pShaderGroup != 0 )
        {
            pMaterial->SetShader( pShaderGroup );
        }
        else
        {
            EngineFileManager* pEngineFileManager = static_cast<EngineFileManager*>( m_pEngineCore->GetManagers()->GetFileManager() );
            MyFileObject* pFile = pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_TextureTintDiscard.glsl" );
            pFile->MemoryPanel_Hide();
            MyAssert( pFile->IsA( "MyFileShader" ) );
            if( pFile->IsA( "MyFileShader" ) )
            {
                MyFileObjectShader* pShaderFile = (MyFileObjectShader*)pFile;
                pShaderGroup = MyNew ShaderGroup( m_pEngineCore, pShaderFile, m_pEngineCore->GetManagers()->GetTextureManager()->GetErrorTexture() );
                pMaterial->SetShader( pShaderGroup );
                pShaderGroup->Release();
            }
            pFile->Release();
        }
    }
}

void EditorState::SaveAllMiscFiles()
{
    MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();

    pMaterialManager->SaveAllMaterials();
    m_pEngineCore->GetComponentSystemManager()->m_pPrefabManager->SaveAllPrefabs();
    m_pEngineCore->GetSoundManager()->SaveAllCues();
}

void EditorState::SaveAllOpenDocuments()
{
    for( uint32 i=0; i<m_pOpenDocuments.size(); i++ )
    {
        if( m_pOpenDocuments[i]->HasUnsavedChanges() )
        {
            const char* filename = m_pOpenDocuments[i]->GetFilename();
            if( filename[0] == '\0' )
            {
                LOGInfo( LOGTag, "WARNING: Untitled document not saved.\n" );
            }
            else
            {
                m_pOpenDocuments[i]->Save();
            }
        }
    }
}

bool EditorState::DoAnyOpenDocumentsHaveUnsavedChanges()
{
    for( uint32 i=0; i<m_pOpenDocuments.size(); i++ )
    {
        if( m_pOpenDocuments[i]->HasUnsavedChanges() )
        {
            return true;
        }
    }

    return false;
}

void EditorState::OpenDocument(EditorDocument* pDocument)
{
    MyAssert( pDocument != nullptr );

    m_pOpenDocuments.push_back( pDocument );
}

ComponentCamera* EditorState::GetEditorCamera()
{
    if( m_pEditorCamera == nullptr )
        return nullptr;

    return (ComponentCamera*)m_pEditorCamera->GetFirstComponentOfBaseType( BaseComponentType_Camera );
}

void EditorState::ClearEditorState(bool clearselectedobjectandcomponents)
{
    if( m_CameraState != EditorCameraState_LockedToObject )
    {
        m_pGameObjectCameraIsFollowing = 0;
    }

    // If the object we're locked on to isn't from a loaded scene, then clear it.
    if( m_CameraState == EditorCameraState_LockedToObject &&
        m_pGameObjectCameraIsFollowing->GetSceneID() >= MAX_SCENES_LOADED )
    {
        m_CameraState = EditorCameraState_Default;
        m_pGameObjectCameraIsFollowing = 0;
    }

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

void EditorState::OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height)
{
    m_EditorWindowRect.Set( x, y, width, height );

    if( m_pDebugViewFBO )
    {
        m_pEngineCore->GetManagers()->GetTextureManager()->ReSetupFBO( m_pDebugViewFBO, width, height, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, false );
    }

    if( m_pMousePickerFBO )
    {
        m_pEngineCore->GetManagers()->GetTextureManager()->ReSetupFBO( m_pMousePickerFBO, width, height, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, false );
    }
}

void EditorState::SetModifierKeyStates(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    if( keyAction == GCBA_Down )
    {
        if( keyCode == MYKEYCODE_LCTRL )    m_ModifierKeyStates |= MODIFIERKEY_Control;
        if( keyCode == MYKEYCODE_LALT )     m_ModifierKeyStates |= MODIFIERKEY_Alt;
        if( keyCode == MYKEYCODE_LSHIFT )   m_ModifierKeyStates |= MODIFIERKEY_Shift;
        if( keyCode == ' ' )                m_ModifierKeyStates |= MODIFIERKEY_Space;
    }

    // Since keys can be pressed while a different window frame has focus, let's just manually query the modifiers.
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LCTRL ) )  m_ModifierKeyStates |= MODIFIERKEY_Control;
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LALT ) )   m_ModifierKeyStates |= MODIFIERKEY_Alt;
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LSHIFT ) ) m_ModifierKeyStates |= MODIFIERKEY_Shift;
    if( PlatformSpecific_CheckKeyState( ' ' ) )              m_ModifierKeyStates |= MODIFIERKEY_Space;

    if( mouseAction != -1 )
    {
        m_CurrentMousePosition.Set( x, y );

        if( mouseAction == GCBA_Down )
        {
            if( id == 0 )
            {
                m_MouseDownLocation[id] = m_CurrentMousePosition;
                m_ModifierKeyStates |= MODIFIERKEY_LeftMouse;
            }
            if( id == 1 )
            {
                m_MouseDownLocation[id] = m_CurrentMousePosition;
                m_ModifierKeyStates |= MODIFIERKEY_RightMouse;
            }
            if( id == 2 )
            {
                m_MouseDownLocation[id] = m_CurrentMousePosition;
                m_ModifierKeyStates |= MODIFIERKEY_MiddleMouse;
            }
        }

        if( mouseAction == GCBA_Held || mouseAction == GCBA_RelativeMovement )
        {
            if( m_ModifierKeyStates & MODIFIERKEY_LeftMouse )
            {
                if( m_MouseDownLocation[0] != m_CurrentMousePosition )
                    m_HasMouseMovedSinceButtonPressed[0] = true;
            }
            if( m_ModifierKeyStates & MODIFIERKEY_RightMouse )
            {
                if( m_MouseDownLocation[1] != m_CurrentMousePosition )
                    m_HasMouseMovedSinceButtonPressed[1] = true;
            }
            if( m_ModifierKeyStates & MODIFIERKEY_MiddleMouse )
            {
                if( m_MouseDownLocation[2] != m_CurrentMousePosition )
                    m_HasMouseMovedSinceButtonPressed[2] = true;
            }
        }
    }
}

void EditorState::ClearModifierKeyStates(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    if( keyAction == GCBA_Up )
    {
        if( keyCode == MYKEYCODE_LCTRL )    m_ModifierKeyStates &= ~MODIFIERKEY_Control;
        if( keyCode == MYKEYCODE_LALT )     m_ModifierKeyStates &= ~MODIFIERKEY_Alt;
        if( keyCode == MYKEYCODE_LSHIFT )   m_ModifierKeyStates &= ~MODIFIERKEY_Shift;
        if( keyCode == ' ' )                m_ModifierKeyStates &= ~MODIFIERKEY_Space;
    }

    // Since keys can be pressed while a different window frame has focus, let's just manually query the modifiers.
    // This shouldn't be necessary since mouse down will have given us focus if we care about keys.
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LCTRL ) == false )  m_ModifierKeyStates &= ~MODIFIERKEY_Control;
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LALT ) == false )   m_ModifierKeyStates &= ~MODIFIERKEY_Alt;
    if( PlatformSpecific_CheckKeyState( MYKEYCODE_LSHIFT ) == false ) m_ModifierKeyStates &= ~MODIFIERKEY_Shift;
    if( PlatformSpecific_CheckKeyState( ' ' ) == false )              m_ModifierKeyStates &= ~MODIFIERKEY_Space;

    if( mouseAction != -1 )
    {
        if( mouseAction == GCBA_Up )
        {
            m_MouseDownLocation[id] = Vector2( -1, -1 );

            if( id == 0 )
            {
                m_ModifierKeyStates &= ~MODIFIERKEY_LeftMouse;
            }
            else if( id == 1 )
            {
                m_ModifierKeyStates &= ~MODIFIERKEY_RightMouse;
            }
            else if( id == 2 )
            {
                m_ModifierKeyStates &= ~MODIFIERKEY_MiddleMouse;
            }

            // Unlock the mouse, even if it wasn't locked.
#if !MYFW_OSX
            //LOGInfo( LOGTag, "Request mouse unlock\n" );
            SetMouseLock( false );
#endif //!MYFW_OSX

            m_HasMouseMovedSinceButtonPressed[id] = false;
        }
    }

    m_LastMousePosition = m_CurrentMousePosition;
}

void EditorState::ClearConstraint()
{
#if MYFW_USING_BULLET
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
#endif //MYFW_USING_BULLET
}

void EditorState::SelectGameObject(GameObject* pObject)
{
    // Don't allow null pointers or duplicate objects into our selected list.
    MyAssert( pObject != 0 );
    MyAssert( std::find( m_pSelectedObjects.begin(), m_pSelectedObjects.end(), pObject ) == m_pSelectedObjects.end() );

    m_pSelectedObjects.push_back( pObject );
}

void EditorState::UnselectGameObject(GameObject* pObject)
{
    // The object shouldn't exist in the list more than once, but we'll still remove them all, just in case.
    m_pSelectedObjects.erase( std::remove( m_pSelectedObjects.begin(), m_pSelectedObjects.end(), pObject ) );
}

bool EditorState::IsGameObjectSelected(GameObject* pObject)
{
    if( std::find( m_pSelectedObjects.begin(), m_pSelectedObjects.end(), pObject ) != m_pSelectedObjects.end() )
        return true;

    return false;
}

bool EditorState::IsGameObjectAParentOfASelectedObjectOrComponent(GameObject* pObject)
{
    for( unsigned i=0; i < m_pSelectedObjects.size(); i++ )
    {
        GameObject* pSelectedGameObject = m_pSelectedObjects[i];
        if( pSelectedGameObject == pObject || pSelectedGameObject->IsParentedTo( pObject, false ) )
            return true;
    }

    for( unsigned i=0; i < m_pSelectedComponents.size(); i++ )
    {
        GameObject* pSelectedGameObject = m_pSelectedComponents[i]->GetGameObject();
        if( pSelectedGameObject == pObject || pSelectedGameObject->IsParentedTo( pObject, false ) )
            return true;
    }

    return false;
}

void EditorState::DeleteSelectedObjects()
{
    if( m_pSelectedObjects.size() > 0 )
    {
        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DeleteObjects( m_pSelectedObjects ) );
    }
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
}

void EditorState::ClearSelectedComponents()
{
    m_pSelectedComponents.clear();
}

void EditorState::ClearSelectedObjectsAndComponentsFromScene(SceneID sceneID)
{
    for( uint32 i=0; i<m_pSelectedObjects.size(); i++ )
    {
        if( m_pSelectedObjects[i]->GetSceneID() == sceneID )
        {
            m_pSelectedObjects[i] = m_pSelectedObjects[m_pSelectedObjects.size()-1];
            m_pSelectedObjects.pop_back();
            i--;
        }
    }

    for( uint32 i=0; i<m_pSelectedComponents.size(); i++ )
    {
        if( m_pSelectedComponents[i]->GetSceneID() == sceneID )
        {
            m_pSelectedComponents[i] = m_pSelectedComponents[m_pSelectedComponents.size()-1];
            m_pSelectedComponents.pop_back();
            i--;
        }
    }
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

void EditorState::UpdateCamera(float deltaTime)
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
