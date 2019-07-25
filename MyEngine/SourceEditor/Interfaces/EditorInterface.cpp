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
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/TransformGizmo.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"
#include "../SourceEditor/Prefs/EditorKeyBindings.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"
#include "../../../SharedGameCode/Core/MyMeshText.h"

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
    if( m_pEngineCore->GetShader_TintColor() )
    {
        Shader_Base* pShader;

        // No lights, 4 bones. Used by mouse picker.
        pShader = (Shader_Base*)m_pEngineCore->GetShader_TintColor()->GlobalPass( 0, 4 );
        if( pShader->m_Initialized == false )
            pShader->CompileShader();

        // 4 lights, 0 bones. Used by transform gizmo.
        pShader = (Shader_Base*)m_pEngineCore->GetShader_TintColor()->GlobalPass( 4, 0 );
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

    EditorState* pEditorState = m_pEngineCore->GetEditorState();

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

                // Draw overlays for selected objects in editor view.
                if( m_pEngineCore->GetEditorPrefs()->Get_Internal_ShowSpecialEffectsForSelectedItems() )
                {
                    // Only draw selected objects over "main" layer.
                    if( pCamera->m_LayersToRender & Layer_MainScene )
                    {
                        //g_pRenderer->SetDepthTestEnabled( false );
                        g_pRenderer->SetCullingEnabled( false );
                        g_pRenderer->SetDepthFunction( MyRE::DepthFunc_LEqual );

                        ShaderGroup* pShaderOverride = m_pEngineCore->GetShader_SelectedObjects();

                        Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
                        if( pShader->Activate() )
                        {
                            for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
                            {
                                // Draw an outline around the selected object.
                                if( m_pEngineCore->GetEditorPrefs()->Get_View_SelectedObjects_ShowWireframe() )
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
                                if( m_pEngineCore->GetEditorPrefs()->Get_View_SelectedObjects_ShowEffect() )
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

            if( i == 0 && m_pEngineCore->GetEditorPrefs()->Get_Debug_DrawStats() )
            {
                MyMeshText* pDebugTextMesh = m_pEngineCore->GetDebugTextMesh();

                if( pDebugTextMesh )
                {
                    MyRect windowrect( pCamera->m_Viewport.GetX(), pCamera->m_Viewport.GetY(), pCamera->m_Viewport.GetWidth(), pCamera->m_Viewport.GetHeight() );

                    if( pDebugTextMesh->GetMaterial( 0 ) == nullptr )
                    {
                        MaterialDefinition* pMaterial = m_pEngineCore->GetManagers()->GetMaterialManager()->LoadMaterial( "Data/DataEngine/Materials/Nevis60.mymaterial" );
                        MyAssert( pMaterial );
                        if( pMaterial )
                        {
                            pDebugTextMesh->SetMaterial( pMaterial, 0 );
                            pMaterial->Release();
                        }
                    }

                    //pDebugTextMesh->CreateStringWhite( false, 15, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h, Justify_TopRight, Vector2(0,0),
                    //                                     "GLStats - buffers(%0.2fM) - draws(%d) - fps(%d)", pBufferManager->CalculateTotalMemoryUsedByBuffers()/1000000.0f, g_GLStats.GetNumDrawCallsLastFrameForCurrentCanvasID(), (int)m_DebugFPS );
                    pDebugTextMesh->CreateStringWhite( false, 15, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h, Justify_TopRight, Vector2(0,0),
                        "GL - draws(%d) - fps(%d)", g_GLStats.GetNumDrawCallsLastFrameForCurrentCanvasID(), (int)m_pEngineCore->GetDebugFPS() );

                    // Draw Lua memory usage.
                    {
                        int ramUsedThisFrame = m_pEngineCore->GetLuaMemoryUsedThisFrame();
                        int ramUsedLastFrame = m_pEngineCore->GetLuaMemoryUsedLastFrame();

                        int megs = ramUsedThisFrame/1000000;
                        int kilos = (ramUsedThisFrame - megs*1000000)/1000;
                        int bytes = ramUsedThisFrame%1000;

                        int change = ramUsedThisFrame - ramUsedLastFrame;

                        if( megs == 0 )
                        {
                            pDebugTextMesh->CreateStringWhite( true, 15, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h-15, Justify_TopRight, Vector2(0,0),
                                "Lua - memory(%d,%03d) - (%d)", kilos, bytes, change );
                        }
                        else
                        {
                            pDebugTextMesh->CreateStringWhite( true, 15, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h-15, Justify_TopRight, Vector2(0,0),
                                "Lua - memory(%d,%03d,%03d) - (%d)", megs, kilos, bytes, change );
                        }
                    }

#if MYFW_WINDOWS
                    // Draw Main ram memory usage.
                    {
                        //size_t bytesused = MyMemory_GetNumberOfBytesAllocated();
                        //int megs = (int)(bytesused/1000000);
                        //int kilos = (int)((bytesused - megs*1000000)/1000);
                        //int bytes = bytesused%1000;

                        //int change = (int)(bytesused - m_TotalMemoryAllocatedLastFrame);

                        //if( megs == 0 )
                        //{
                        //    pDebugTextMesh->CreateStringWhite( true, 15, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h-30, Justify_TopRight, Vector2(0,0),
                        //        "Memory(%03d,%03d) - (%d)", kilos, bytes, change );
                        //}
                        //else
                        //{
                        //    pDebugTextMesh->CreateStringWhite( true, 15, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h-30, Justify_TopRight, Vector2(0,0),
                        //        "Memory(%d,%03d,%03d) - (%d)", megs, kilos, bytes, change );
                        //}

                        //m_TotalMemoryAllocatedLastFrame = bytesused;
                    }
#endif

                    // Draw single frame stack ram memory usage.
                    {
                        //unsigned int bytesused = m_SingleFrameStackSizeThisFrame;
                        //int megs = bytesused/1000000;
                        //int kilos = (bytesused - megs*1000000)/1000;
                        //int bytes = bytesused%1000;

                        //int change = bytesused - m_SingleFrameStackSizeLastFrame;

                        //if( megs == 0 )
                        //{
                        //    pDebugTextMesh->CreateStringWhite( true, 10, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h-30, Justify_TopRight, Vector2(0,0),
                        //        "Frame Stack(%03d,%03d) - (%d)", kilos, bytes, change );
                        //}
                        //else
                        //{
                        //    pDebugTextMesh->CreateStringWhite( true, 10, (float)windowrect.x+windowrect.w, (float)windowrect.y+windowrect.h-30, Justify_TopRight, Vector2(0,0),
                        //        "Frame Stack(%d,%03d,%03d) - (%d)", megs, kilos, bytes, change );
                        //}
                    }

                    //windowrect.Set( pCamera->m_Viewport.GetX(), pCamera->m_Viewport.GetY(), pCamera->m_Viewport.GetWidth(), pCamera->m_Viewport.GetHeight() );

                    MyMatrix matProj;
                    matProj.CreateOrtho( (float)windowrect.x, (float)windowrect.x+windowrect.w, (float)windowrect.y, (float)windowrect.y+windowrect.h, -1, 1 );
                    g_pRenderer->SetDepthTestEnabled( false );
                    pDebugTextMesh->Draw( &matProj, nullptr, nullptr, nullptr,nullptr,nullptr,0,nullptr,nullptr,nullptr,nullptr );
                    g_pRenderer->SetDepthTestEnabled( true );
                }
            }
        }
    }

    // Draw our mouse picker frame over the screen.
    if( g_pEngineCore->GetDebug_DrawMousePickerFBO() && g_GLCanvasIDActive == 1 )
    {
        MySprite* pDebugQuad = m_pEngineCore->GetSprite_DebugQuad();

        if( pDebugQuad )
        {
            BufferManager* pBufferManager = m_pEngineCore->GetManagers()->GetBufferManager();
            pDebugQuad->CreateInPlace( pBufferManager, "debug", 0.75f, 0.75f, 0.5f, 0.5f, 0, 1, 1, 0, Justify_Center, false );
            m_pEngineCore->GetMaterial_MousePicker()->SetTextureColor( pEditorState->m_pMousePickerFBO->GetColorTexture( 0 ) );
            pDebugQuad->SetMaterial( m_pEngineCore->GetMaterial_MousePicker() );
            pDebugQuad->Draw( nullptr, nullptr, nullptr );
        }
    }
}

//void EditorInterface::SetModifierKeyStates(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
//{
//    EditorState* pEditorState = m_pEngineCore->GetEditorState();
//
//    pEditorState->SetModifierKeyStates( keyAction, keyCode, mouseAction, id, x, y, pressure );
//}

bool EditorInterface::HandleInputForEditorCamera(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    EditorState* pEditorState = m_pEngineCore->GetEditorState();
    ComponentCamera* pCamera = pEditorState->GetEditorCamera();

    return pCamera->HandleInputForEditorCamera( keyAction, keyCode, mouseAction, id, x, y, pressure );
}

bool EditorInterface::ExecuteHotkeyAction(HotkeyAction action)
{
    return false;
}

void EditorInterface::RenderObjectIDsToFBO()
{
    EditorState* pEditorState = m_pEngineCore->GetEditorState();

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
            g_pComponentSystemManager->DrawMousePickerFrame( pCamera, &pCamera->m_Camera3D.m_matProj, &pCamera->m_Camera3D.m_matView, m_pEngineCore->GetShader_TintColor() );
            g_pRenderer->ClearBuffers( false, true, false );
        }
    }

    pEditorState->m_pMousePickerFBO->Unbind( true );

    pEditorState->m_pTransformGizmo->ScaleGizmosForMousePickRendering( false );
}

unsigned int EditorInterface::GetIDAtPixel(unsigned int x, unsigned int y, bool createNewBitmap, bool includeTransformGizmo)
{
    EditorState* pEditorState = m_pEngineCore->GetEditorState();

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
    EditorState* pEditorState = m_pEngineCore->GetEditorState();

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
    EditorState* pEditorState = m_pEngineCore->GetEditorState();
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
