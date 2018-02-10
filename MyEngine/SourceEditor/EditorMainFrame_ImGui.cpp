//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "EditorMenuCommands.h"

enum EditorWindowTypes
{
    EditorWindow_Game,
    EditorWindow_PanelObjectList,
    EditorWindow_PanelWatch,
    EditorWindow_PanelMemory,
    EditorWindow_NumTypes,
};

const char* g_DefaultEditorWindowTypeMenuLabels[EditorWindow_NumTypes] =
{
    "&Game View",
    "&Object List Panel",
    "&Watch Panel",
    "&Files Panel",
};

enum PanelMemoryPages
{
    PanelMemoryPage_Materials,
    PanelMemoryPage_Textures,
    PanelMemoryPage_ShaderGroups,
    PanelMemoryPage_SoundCues,
    PanelMemoryPage_Files,
    PanelMemoryPage_Buffers,
    PanelMemoryPage_DrawCalls,
    PanelMemoryPage_NumTypes
};

const char* g_PanelMemoryPagesMenuLabels[PanelMemoryPage_NumTypes] =
{
    "Materials",
    "Textures",
    "Shaders",
    "Sound Cues",
    "Files",
    "Buffers",
    "Draw Calls",
};

EditorMainFrame_ImGui::EditorMainFrame_ImGui()
{
    m_pGameFBO = g_pTextureManager->CreateFBO( 1024, 1024, GL_NEAREST, GL_NEAREST, true, 32, true );
    m_pEditorFBO = g_pTextureManager->CreateFBO( 1024, 1024, GL_NEAREST, GL_NEAREST, true, 32, true );
    
    m_pMaterialPreviewFBO = g_pTextureManager->CreateFBO( 1024, 1024, GL_NEAREST, GL_NEAREST, true, 32, true );
    m_pMaterialToPreview = 0;

    m_pMaterialBeingEdited = 0;
    m_IsMaterialEditorOpen = false;

    m_GameWindowPos.Set( -1, -1 );
    m_EditorWindowPos.Set( -1, -1 );
    m_GameWindowSize.Set( 0, 0 );
    m_EditorWindowSize.Set( 0, 0 );

    m_GameWindowFocused = false;
    m_EditorWindowHovered = false;
    m_EditorWindowFocused = false;

    m_CurrentMemoryPanelPage = PanelMemoryPage_Materials;

    m_KeyDownCtrl = false;
    m_KeyDownAlt = false;
    m_KeyDownShift = false;
    m_KeyDownCommand = false;

    m_pCommandStack = MyNew CommandStack;
    g_pEngineCore->SetCommandStack( m_pCommandStack );
}

EditorMainFrame_ImGui::~EditorMainFrame_ImGui()
{
    SAFE_DELETE( m_pCommandStack );

    SAFE_RELEASE( m_pGameFBO );
    SAFE_RELEASE( m_pEditorFBO );
    SAFE_RELEASE( m_pMaterialPreviewFBO )
}

Vector2 EditorMainFrame_ImGui::GetEditorWindowCenterPosition()
{
    return m_EditorWindowPos + m_EditorWindowSize/2;
}

bool EditorMainFrame_ImGui::HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    if( keyaction != -1 )
    {
        if( CheckForHotkeys( keyaction, keycode ) )
        {
            // If a hotkey was pressed, unset that key so 'held' and 'up' messages won't get sent.
            g_pEngineCore->ForceKeyRelease( keycode );
            return true;
        }
    }

    // For keyboard and other non-mouse events, localx/y will be -1.
    float localx = -1;
    float localy = -1;

    if( mouseaction == GCBA_RelativeMovement )
    {
        // localx/y will hold relative movement in this case.
        localx = x;
        localy = y;
    }

    // Read the absolute x/y from the ImGui structure, since non-mouse messages will have x,y of -1,-1.
    ImGuiIO& io = ImGui::GetIO();
    float mouseabsx = io.MousePos.x;
    float mouseabsy = io.MousePos.y;

    // Are absolute x/y over the game window or it's a keyaction and the window is in focus.
    if( ( keyaction != -1 && m_GameWindowFocused ) ||
        ( mouseaction != -1 &&
            mouseabsx >= m_GameWindowPos.x && mouseabsx < m_GameWindowPos.x + m_GameWindowSize.x &&
            mouseabsy >= m_GameWindowPos.y && mouseabsy < m_GameWindowPos.y + m_GameWindowSize.y ) )
    {
        //ImGui::Begin( "Debug" );
        //ImGui::Text( "In Game Window" );
        //ImGui::End();
    }

    // Are absolute x/y over the editor window or it's a keyaction and the window is in focus.
    if( ( keyaction != -1 && m_EditorWindowFocused ) ||
        ( ( m_EditorWindowHovered || m_EditorWindowFocused ) && mouseaction != -1 &&
            mouseabsx >= m_EditorWindowPos.x && mouseabsx < m_EditorWindowPos.x + m_EditorWindowSize.x &&
            mouseabsy >= m_EditorWindowPos.y && mouseabsy < m_EditorWindowPos.y + m_EditorWindowSize.y ) )
    {
        // If this is a mouse message and not a relative movement,
        //     calculate mouse x/y relative to this window.
        if( mouseaction != -1 && mouseaction != GCBA_RelativeMovement )
        {
            localx = x - m_EditorWindowPos.x;
            localy = m_EditorWindowSize.y - (y - m_EditorWindowPos.y);
        }

        // If the right or middle mouse buttons were clicked on this window, set it as having focus.
        // Needed since those buttons don't focus ImGui window directly.
        if( mouseaction == GCBA_Down && id != 0 )
        {
            ImGui::SetWindowFocus( "Editor" );
        }

        // First, pass the input into the current editor interface.
        if( g_pEngineCore->GetCurrentEditorInterface()->HandleInput( keyaction, keycode, mouseaction, id, localx, localy, pressure ) )
            return true;

        // If it wasn't used, pass it to the transform gizmo.
        if( g_pEngineCore->GetEditorState()->m_pTransformGizmo->HandleInput( g_pEngineCore, -1, -1, mouseaction, id, localx, localy, pressure ) )
            return true;

        // Clear modifier key and mouse button states.
        g_pEngineCore->GetCurrentEditorInterface()->ClearModifierKeyStates( keyaction, keycode, mouseaction, id, localx, localy, pressure );
    }

    return false;
}

bool EditorMainFrame_ImGui::CheckForHotkeys(int keyaction, int keycode)
{
    if( keyaction == GCBA_Down )
    {
        if( keycode == MYKEYCODE_LCTRL || keycode == MYKEYCODE_RCTRL )
            m_KeyDownCtrl = true;

        if( keycode == MYKEYCODE_LALT || keycode == MYKEYCODE_RALT )
            m_KeyDownAlt = true;

        if( keycode == MYKEYCODE_LSHIFT || keycode == MYKEYCODE_RSHIFT )
            m_KeyDownShift = true;
    }
    
    if( keyaction == GCBA_Up )
    {
        if( keycode == MYKEYCODE_LCTRL || keycode == MYKEYCODE_RCTRL )
            m_KeyDownCtrl = false;

        if( keycode == MYKEYCODE_LALT || keycode == MYKEYCODE_RALT )
            m_KeyDownAlt = false;

        if( keycode == MYKEYCODE_LSHIFT || keycode == MYKEYCODE_RSHIFT )
            m_KeyDownShift = false;
    }

    if( keyaction == GCBA_Down )
    {
        bool C  =  m_KeyDownCtrl && !m_KeyDownAlt && !m_KeyDownShift && !m_KeyDownCommand; // Ctrl
        bool CS =  m_KeyDownCtrl && !m_KeyDownAlt &&  m_KeyDownShift && !m_KeyDownCommand; // Ctrl-Shift

        if( C  && keycode == 'S' ) { EditorMenuCommand( EditorMenuCommand_File_SaveScene );         return true; }
        if( CS && keycode == 'E' ) { EditorMenuCommand( EditorMenuCommand_File_Export_Box2DScene ); return true; }
        if( C  && keycode == ' ' ) { EditorMenuCommand( EditorMenuCommand_Mode_TogglePlayStop );    return true; }
        if( C  && keycode == 'Z' ) { EditorMenuCommand( EditorMenuCommand_Edit_Undo );              return true; }
        if( C  && keycode == 'Y' ) { EditorMenuCommand( EditorMenuCommand_Edit_Redo );              return true; }
        if( CS && keycode == 'Z' ) { EditorMenuCommand( EditorMenuCommand_Edit_Redo );              return true; }
    }

    return false;
}

void EditorMainFrame_ImGui::AddEverything()
{
    AddMainMenuBar();
    AddGameAndEditorWindows();
    AddObjectList();
    AddWatchPanel();
    AddMemoryPanel();
    AddDebug_MousePicker();

    if( m_IsMaterialEditorOpen )
        AddMaterialEditor();

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin( "Stuff" );
    ImGui::Text( "WantCaptureKeyboard %d", io.WantCaptureKeyboard );
    ImGui::Text( "WantCaptureMouse %d", io.WantCaptureMouse );
    ImGui::Text( "WantMoveMouse %d", io.WantMoveMouse );
    ImGui::Text( "WantTextInput %d", io.WantTextInput );
    ImGui::Text( "m_GameWindowFocused %d", m_GameWindowFocused );
    ImGui::Text( "m_EditorWindowHovered %d", m_EditorWindowHovered );    
    ImGui::Text( "m_EditorWindowFocused %d", m_EditorWindowFocused );
    ImGui::Text( "MouseWheel %0.2f", io.MouseWheel );

    GameObject* pGO = g_pComponentSystemManager->FindGameObjectByName( "Player" );
    if( pGO && pGO->GetTransform() )
    {
        ImGui::Text( "PlayerX %0.2f", pGO->GetTransform()->GetWorldTransform()->m41 );
    }

    ImGui::ShowDemoWindow();
    
    ImGui::End();
}

void EditorMainFrame_ImGui::AddMainMenuBar()
{
    if( ImGui::BeginMainMenuBar() )
    {
        if( ImGui::BeginMenu( "File" ) )
        {
            if( ImGui::MenuItem( "New Scene (TODO)" ) ) {  }
            if( ImGui::MenuItem( "Load Scene..." ) ) { EditorMenuCommand( EditorMenuCommand_File_LoadScene ); }
            if( ImGui::MenuItem( "Create Additional Scene (TODO)" ) ) {}
            if( ImGui::MenuItem( "Load Additional Scene... (TODO)" ) ) {}
            if( ImGui::MenuItem( "Save Scene", "Ctrl-S" ) ) { EditorMenuCommand( EditorMenuCommand_File_SaveScene ); }
            if( ImGui::MenuItem( "Save Scene As... (TODO)" ) ) {}

            if( ImGui::BeginMenu( "Export" ) )
            {
                if( ImGui::MenuItem( "Box2D Scene...", "Ctrl-Shift-E" ) ) { EditorMenuCommand( EditorMenuCommand_File_Export_Box2DScene ); }
                ImGui::EndMenu();
            }
            if( ImGui::MenuItem( "Quit (TODO)" ) ) {}

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Edit" ) )
        {
            if( ImGui::MenuItem( "Undo", "CTRL-Z" ) ) { EditorMenuCommand( EditorMenuCommand_Edit_Undo ); }
            if( ImGui::MenuItem( "Redo", "CTRL-Y" ) ) { EditorMenuCommand( EditorMenuCommand_Edit_Redo ); }

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "View" ) )
        {
            if( ImGui::MenuItem( "Save window layout (TODO)" ) ) {} // { EditorMenuCommand( myID_View_SavePerspective ); }
            if( ImGui::MenuItem( "Load window layout (TODO)" ) ) {} // { EditorMenuCommand( myID_View_LoadPerspective ); }
            if( ImGui::MenuItem( "Reset window layout (TODO)" ) ) {} // { EditorMenuCommand( myID_View_ResetPerspective ); }

            if( ImGui::BeginMenu( "Editor Windows (TODO)" ) )
            {
                for( int i=0; i<EditorWindow_NumTypes; i++ )
                {
                    if( ImGui::MenuItem( g_DefaultEditorWindowTypeMenuLabels[i] ) ) {}
                }

                for( int i=0; i<EngineEditorWindow_NumTypes; i++ )
                {
                    if( ImGui::MenuItem( g_DefaultEngineEditorWindowTypeMenuLabels[i] ) ) {}
                }

                ImGui::EndMenu();
            }

            if( ImGui::BeginMenu( "Editor Perspectives (TODO)" ) )
            {
                for( int i=0; i<Perspective_NumPerspectives; i++ )
                {
                    if( ImGui::MenuItem( g_DefaultPerspectiveMenuLabels[i] ) ) {}
                }
                ImGui::EndMenu();
            }

            if( ImGui::BeginMenu( "Gameplay Perspectives (TODO)" ) )
            {
                for( int i=0; i<Perspective_NumPerspectives; i++ )
                {
                    if( ImGui::MenuItem( g_DefaultPerspectiveMenuLabels[i] ) ) {}
                }
                ImGui::EndMenu();
            }

            ImGui::MenuItem( "Show Editor Icons (TODO)", "Shift-F7" );

            if( ImGui::BeginMenu( "Selected Objects (TODO)" ) )
            {
                if( ImGui::MenuItem( "Show Wireframe (TODO)" ) ) {}
                if( ImGui::MenuItem( "Show Effect (TODO)" ) ) {}

                ImGui::EndMenu();
            }

            if( ImGui::BeginMenu( "Editor Camera Layers (TODO)" ) )
            {
                for( int i=0; i<g_NumberOfVisibilityLayers; i++ )
                {
                    char label[100];
                    char hotkey[100];
                    sprintf_s( label, 100, "(%d) %s", i, g_pVisibilityLayerStrings[i] );
                    sprintf_s( hotkey, 100, "Ctrl-Alt-%d", i );
                    if( i < 9 )
                    {
                        if( ImGui::MenuItem( label, hotkey ) ) {}
                    }
                    else
                    {
                        if( ImGui::MenuItem( label ) ) {}
                    }
                }
                ImGui::EndMenu();
            }
            //    {
            //        m_SubMenu_View_EditorCameraLayers = MyNew wxMenu;
            //        for( int i=0; i<g_NumberOfVisibilityLayers; i++ )
            //        {
            //            wxString label;
            //            if( i < 9 )
            //                label << "(&" << i+1 << ") " << g_pVisibilityLayerStrings[i] << "\tCtrl-Alt-" << i+1;
            //            else
            //                label << "(&" << i+1 << ") " << g_pVisibilityLayerStrings[i];
            //            m_MenuItem_View_EditorCameraLayerOptions[i] = m_SubMenu_View_EditorCameraLayers->AppendCheckItem( myIDEngine_View_EditorCameraLayer + i, label, wxEmptyString );
            //            Connect( myIDEngine_View_EditorCameraLayer + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
            //        }
            //    
            //        m_MenuItem_View_EditorCameraLayerOptions[0]->Check();
            //    }

            ImGui::MenuItem( "Fullscreen Editor (TODO)", "F11" );
            ImGui::MenuItem( "Fullscreen Game (TODO)", "Ctrl-F11" );

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Aspect" ) )
        {
            if( ImGui::MenuItem( "Fill (TODO)", "Alt-1" ) ) {}
            if( ImGui::MenuItem( "Tall (TODO)", "Alt-2" ) ) {}
            if( ImGui::MenuItem( "Square (TODO)", "Alt-3" ) ) {}
            if( ImGui::MenuItem( "Wide (TODO)", "Alt-4" ) ) {}

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Grid" ) )
        {
            if( ImGui::MenuItem( "Grid On/Off (TODO)", "Ctrl-Shift-V", true ) ) {} // { EditorMenuCommand( myIDEngine_Grid_VisibleOnOff ); }
            if( ImGui::MenuItem( "Grid Snap On/Off (TODO)", "Ctrl-G", true ) ) {} // { EditorMenuCommand( myIDEngine_Grid_SnapOnOff ); }
            if( ImGui::MenuItem( "Grid Settings (TODO)", "Ctrl-Shift-G" ) ) {}

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Mode" ) )
        {
            if( ImGui::MenuItem( "Switch Focus on Play/Stop (TODO)", 0, true ) ) {} // { EditorMenuCommand( myIDEngine_Mode_SwitchFocusOnPlayStop ); }
            //// Since Command-Space is "Spotlight Search" on OSX, use the actual control key on OSX as well as Windows/Linux.
            if( ImGui::MenuItem( "Play/Stop", "CTRL-SPACE" ) ) { EditorMenuCommand( EditorMenuCommand_Mode_TogglePlayStop ); }
            if( ImGui::MenuItem( "Pause (TODO)", "Ctrl-." ) ) {} // { EditorMenuCommand( myIDEngine_Mode_Pause ); }
            if( ImGui::MenuItem( "Advance 1 Frame (TODO)", "Ctrl-]" ) ) {} // { EditorMenuCommand( myIDEngine_Mode_Advance1Frame ); }
            if( ImGui::MenuItem( "Advance 1 Second (TODO)", "Ctrl-[" ) ) {} // { EditorMenuCommand( myIDEngine_Mode_Advance1Second ); }

            if( ImGui::BeginMenu( "Launch Platforms (TODO)" ) )
            {
                for( int i=0; i<LaunchPlatform_NumPlatforms; i++ )
                {
                    if( ImGui::MenuItem( g_LaunchPlatformsMenuLabels[i], 0, true ) ) {} // { EditorMenuCommand( myIDEngine_Mode_LaunchPlatforms + i ); }
                }

                ImGui::EndMenu();
            }

            if( ImGui::MenuItem( "Launch Game (TODO)", "Ctrl-F5" ) ) {} // { EditorMenuCommand( myIDEngine_Mode_LaunchGame ); }

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Data" ) )
        {
            if( ImGui::MenuItem( "Load Datafiles (TODO)" ) ) {} // { EditorMenuCommand( myIDEngine_Data_AddDatafile ); }
            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Hackery" ) )
        {
            if( ImGui::MenuItem( "Record (TODO)", "Ctrl-R" ) ) {} // { EditorMenuCommand( myIDEngine_Hackery_RecordMacro ); }
            if( ImGui::MenuItem( "Stop recording and Execute (TODO)", "Ctrl-E" ) ) {} // { EditorMenuCommand( myIDEngine_Hackery_ExecuteMacro ); }
            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Debug views" ) )
        {
            if( ImGui::MenuItem( "Show Mouse Picker FBO (TODO)", "F9" ) ) {} // { EditorMenuCommand( myIDEngine_Debug_ShowMousePickerFBO ); }
            if( ImGui::MenuItem( "Show Animated Debug View for Selection (TODO)", "F8" ) ) {} // { EditorMenuCommand( myIDEngine_Debug_ShowSelectedAnimatedMesh ); }
            if( ImGui::MenuItem( "Show GL Stats (TODO)", "Shift-F9" ) ) {} // { EditorMenuCommand( myIDEngine_Debug_ShowGLStats ); }
            if( ImGui::MenuItem( "Draw Wireframe (TODO)", "Ctrl-F9" ) ) {} // { EditorMenuCommand( myIDEngine_Debug_DrawWireframe ); }
            if( ImGui::MenuItem( "Show Physics debug shapes (TODO)", "Shift-F8" ) ) {} // { EditorMenuCommand( myIDEngine_Debug_ShowPhysicsShapes ); }
            if( ImGui::MenuItem( "Show profiling Info (TODO)", "Ctrl-F8" ) ) {} // { EditorMenuCommand( myIDEngine_Debug_ShowProfilingInfo ); }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void EditorMainFrame_ImGui::AddGameAndEditorWindows()
{
    ImGui::SetNextWindowPos( ImVec2(9, 302), ImGuiCond_FirstUseEver );
    if( ImGui::Begin( "Game", 0, ImVec2(256, 171) ) )
    {
        m_GameWindowFocused = ImGui::IsWindowFocused();

        ImVec2 min = ImGui::GetWindowContentRegionMin();
        ImVec2 max = ImGui::GetWindowContentRegionMax();
        float w = max.x - min.x;
        float h = max.y - min.y;

        if( w <= 0 || h <= 0 )
        {
            m_GameWindowPos.Set( -1, -1 );
            m_GameWindowSize.Set( 0, 0 );
        }
        else
        {
            ImVec2 pos = ImGui::GetWindowPos();
            m_GameWindowPos.Set( pos.x + min.x, pos.y + min.y );
            m_GameWindowSize.Set( w, h );

            // Resize our FBO if the window is larger than it ever was.
            if( w > m_pGameFBO->m_TextureWidth || h > m_pGameFBO->m_TextureHeight )
            {
                // The FBO will be recreated during the TextureManager tick.
                g_pTextureManager->InvalidateFBO( m_pGameFBO );
                m_pGameFBO->Setup( (unsigned int)w, (unsigned int)h, GL_NEAREST, GL_NEAREST, true, 32, false );
            }

            if( m_pGameFBO->m_pColorTexture )
            {
                TextureDefinition* tex = m_pGameFBO->m_pColorTexture;
                ImGui::ImageButton( (void*)tex->GetTextureID(), ImVec2( w, h ), ImVec2(0,h/m_pGameFBO->m_TextureHeight), ImVec2(w/m_pGameFBO->m_TextureWidth,0), 0 );
            }
        }
    }
    ImGui::End();

    ImGui::SetNextWindowPos( ImVec2(269, 24), ImGuiCond_FirstUseEver );
    if( ImGui::Begin( "Editor", 0, ImVec2(579, 397) ) )
    {
        m_EditorWindowFocused = ImGui::IsWindowFocused();
        m_EditorWindowHovered = ImGui::IsWindowHovered();

        ImVec2 min = ImGui::GetWindowContentRegionMin();
        ImVec2 max = ImGui::GetWindowContentRegionMax();
        float w = max.x - min.x;
        float h = max.y - min.y;

        if( w <= 0 || h <= 0 )
        {
            m_EditorWindowPos.Set( -1, -1 );
            m_EditorWindowSize.Set( 0, 0 );
        }
        else
        {
            ImVec2 pos = ImGui::GetWindowPos();
            m_EditorWindowPos.Set( pos.x + min.x, pos.y + min.y );
            m_EditorWindowSize.Set( w, h );

            // Resize our FBO if the window is larger than it ever was.
            if( w > m_pEditorFBO->m_TextureWidth || h > m_pEditorFBO->m_TextureHeight )
            {
                // The FBO will be recreated during the TextureManager tick.
                g_pTextureManager->InvalidateFBO( m_pEditorFBO );
                m_pEditorFBO->Setup( (unsigned int)w, (unsigned int)h, GL_NEAREST, GL_NEAREST, true, 32, false );
            }

            if( m_pEditorFBO->m_pColorTexture )
            {
                TextureDefinition* tex = m_pEditorFBO->m_pColorTexture;
                ImGui::ImageButton( (void*)tex->GetTextureID(), ImVec2( w, h ), ImVec2(0,h/m_pEditorFBO->m_TextureHeight), ImVec2(w/m_pEditorFBO->m_TextureWidth,0), 0 );
            }
        }
    }
    ImGui::End();
}

void EditorMainFrame_ImGui::AddObjectList()
{
    ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    ImGui::SetNextWindowPos( ImVec2(4, 28), ImGuiCond_FirstUseEver );
    if( ImGui::Begin( "Objects", 0, ImVec2(258, 266) ) )
    {
        if( ImGui::CollapsingHeader( "All scenes", ImGuiTreeNodeFlags_DefaultOpen ) )
        {
            // Add all active scenes.
            for( int sceneindex=0; sceneindex<ComponentSystemManager::MAX_SCENES_LOADED; sceneindex++ )
            {
                SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( sceneindex );
                if( pSceneInfo->m_InUse == true )
                {
                    static char* pUnmanagedName = "Unmanaged";
                    static char* pUnsavedName = "Unsaved scene";
                    char* scenename = pUnmanagedName;
                    if( sceneindex != 0 )
                        scenename = pUnsavedName;
                    if( pSceneInfo->m_FullPath[0] != 0 )
                    {
                        int i;
                        for( i=(int)strlen(pSceneInfo->m_FullPath)-1; i>=0; i-- )
                        {
                            if( scenename[i] == '\\' || pSceneInfo->m_FullPath[i] == '/' )
                                break;
                        }
                        scenename = &pSceneInfo->m_FullPath[i+1];
                    }

                    ImGuiTreeNodeFlags node_flags = baseNodeFlags;
                    bool treeNodeIsOpen = ImGui::TreeNodeEx( scenename, node_flags );

                    ImGui::PushID( scenename );
                    if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                    {
                        if( ImGui::MenuItem( "Create GameObject (TODO)" ) )    { ImGui::CloseCurrentPopup(); }

                        ImGui::EndPopup();
                    }
                    ImGui::PopID();

                    if( treeNodeIsOpen )
                    {
                        // Add GameObjects that are in root
                        GameObject* pGameObject = (GameObject*)pSceneInfo->m_GameObjects.GetHead();
                        while( pGameObject )
                        {
                            // Add GameObjects, their children and their components
                            AddGameObjectToObjectList( pGameObject );

                            pGameObject = (GameObject*)pGameObject->GetNext();
                        }
                        ImGui::TreePop();
                    }
                }
            }
        }
    }
    ImGui::End();
}

void EditorMainFrame_ImGui::AddGameObjectToObjectList(GameObject* pGameObject)
{
    EditorState* pEditorState = g_pEngineCore->GetEditorState();

    ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    ImGuiTreeNodeFlags node_flags = baseNodeFlags;
    if( pEditorState->IsGameObjectSelected( pGameObject ) )
    {
        node_flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool treeNodeIsOpen = ImGui::TreeNodeEx( pGameObject, node_flags, pGameObject->GetName() );
        
    ImGui::PushID( pGameObject );
    if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
    {
        int numselected = g_pEngineCore->GetEditorState()->m_pSelectedObjects.size();

        if( numselected > 1 )
        {
            if( ImGui::MenuItem( "Duplicate GameObjects (TODO)" ) )    { ImGui::CloseCurrentPopup(); }
            if( ImGui::MenuItem( "Create Child GameObjects (TODO)" ) ) { ImGui::CloseCurrentPopup(); }
            if( ImGui::MenuItem( "Delete GameObjects (TODO)" ) )       { ImGui::CloseCurrentPopup(); }
        }
        else
        {
            //ImGui::Text( pGameObject->GetName() );
            if( ImGui::MenuItem( "Duplicate GameObject (TODO)" ) )    { ImGui::CloseCurrentPopup(); }
            if( ImGui::MenuItem( "Create Child GameObject (TODO)" ) ) { ImGui::CloseCurrentPopup(); }
            if( pGameObject->GetGameObjectThisInheritsFrom() )
            {
                if( ImGui::MenuItem( "Clear Parent (TODO)" ) )        { ImGui::CloseCurrentPopup(); }
            }
            //if( ImGui::MenuItem( "Add Component with submenus... (TODO)" ) )    { ImGui::CloseCurrentPopup(); }
            {
                int first = 0;
                if( pGameObject->GetTransform() != 0 )
                    first = 1;

                const char* lastcategory = 0;
                bool menuopen = false;

                unsigned int numtypes = g_pComponentTypeManager->GetNumberOfComponentTypes();
                for( unsigned int i=first; i<numtypes; i++ )
                {
                    const char* currentcategory = g_pComponentTypeManager->GetTypeCategory( i );
                    const char* nextcategory = 0;
                    if( i < numtypes-1 )
                        nextcategory = g_pComponentTypeManager->GetTypeCategory( i+1 );

                    if( lastcategory != currentcategory )
                    {
                        menuopen = ImGui::BeginMenu( currentcategory );
                    }

                    if( menuopen )
                    {
                        if( i == ComponentType_Mesh )
                        {
                            // don't include ComponentType_Mesh in the right-click menu.
                            // TODO: if more exceptions are made, improve this system.
                        }
                        else
                        {
                            if( ImGui::MenuItem( g_pComponentTypeManager->GetTypeName( i ) ) )
                            {
                                ComponentBase* pComponent = 0;
                                if( g_pEngineCore->IsInEditorMode() )
                                    pComponent = pGameObject->AddNewComponent( i, pGameObject->GetSceneID() );
                                else
                                    pComponent = pGameObject->AddNewComponent( i, 0 );

                                ImGui::CloseCurrentPopup();
                            }
                        }
                    }

                    if( menuopen && currentcategory != nextcategory )
                    {
                        ImGui::EndMenu();
                    }

                    lastcategory = currentcategory;
                }
            }
            if( ImGui::MenuItem( "Prefab Stuff (TODO)" ) )      { ImGui::CloseCurrentPopup(); }
            if( ImGui::MenuItem( "Delete GameObject (TODO)" ) ) { ImGui::CloseCurrentPopup(); }
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();

    if( ImGui::IsItemClicked() )
    {
        if( ImGui::GetIO().KeyCtrl == false )
        {
            pEditorState->ClearSelectedObjectsAndComponents();
        }

        if( ImGui::GetIO().KeyShift == false )
        {
            // TODO: select all GameObjects between last object in list and this one.
        }

        if( pEditorState->IsGameObjectSelected( pGameObject ) )
        {
            pEditorState->UnselectGameObject( pGameObject );
        }
        else
        {
            pEditorState->SelectGameObject( pGameObject );
        }
    }
    if( treeNodeIsOpen )
    {
        // Add Child GameObjects
        GameObject* pChildGameObject = pGameObject->GetFirstChild();
        while( pChildGameObject )
        {
            AddGameObjectToObjectList( pChildGameObject );
            pChildGameObject = (GameObject*)pChildGameObject->GetNext();
        }

        // Add Components
        for( unsigned int ci=0; ci<pGameObject->GetComponentCountIncludingCore(); ci++ )
        {
            ComponentBase* pComponent = pGameObject->GetComponentByIndexIncludingCore( ci );
            if( pComponent )
            {
                ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf;
                if( pEditorState->IsComponentSelected( pComponent ) )
                {
                    nodeFlags |= ImGuiTreeNodeFlags_Selected;
                }

                if( ImGui::TreeNodeEx( pComponent, nodeFlags, pComponent->GetClassname() ) )
                {
                    if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                    {
                        char* label = "Delete Component";
                        if( pEditorState->m_pSelectedComponents.size() > 1 )
                            label = "Delete Selected Components";
                        if( ImGui::MenuItem( label ) )
                        {
                            pComponent->OnRightClickAction( ComponentBase::RightClick_DeleteComponent );
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::TreePop();
                }

                if( ImGui::IsItemClicked() )
                {
                    if( ImGui::GetIO().KeyCtrl == false )
                    {
                        pEditorState->ClearSelectedObjectsAndComponents();
                    }

                    if( ImGui::GetIO().KeyShift == false )
                    {
                        // TODO: select all Components between last object in list and this one.
                    }

                    if( pEditorState->IsComponentSelected( pComponent ) )
                    {
                        pEditorState->UnselectComponent( pComponent );
                    }
                    else
                    {
                        pEditorState->SelectComponent( pComponent );
                    }
                }
            }
        }
        ImGui::TreePop();
    }
}

void EditorMainFrame_ImGui::AddWatchPanel()
{
    ImGui::SetNextWindowPos( ImVec2(852, 25), ImGuiCond_FirstUseEver );
    if( ImGui::Begin( "Watch", 0, ImVec2(333, 395) ) )
    {
        int numselected = g_pEngineCore->GetEditorState()->m_pSelectedObjects.size();

        if( numselected == 1 )
        {
            ImGui::Text( "%d object selected.", numselected );

            GameObject* pGameObject = g_pEngineCore->GetEditorState()->m_pSelectedObjects[0];
            for( unsigned int i=0; i<pGameObject->GetComponentCountIncludingCore(); i++ )
            {
                ComponentBase* pComponent = pGameObject->GetComponentByIndexIncludingCore( i );
                
                if( ImGui::CollapsingHeader( pComponent->GetClassname(), ImGuiTreeNodeFlags_DefaultOpen ) )
                {
                    //ImGui::Text( "TODO: Component Info." );
                    pComponent->AddAllVariablesToWatchPanel();
                }
            }
        }
        else if( numselected > 1 )
        {
            ImGui::Text( "%d objects selected.", numselected );
        }
        else
        {
            // No GameObjects selected, display components.
            int numselected = g_pEngineCore->GetEditorState()->m_pSelectedComponents.size();

            if( numselected == 1 )
            {
                ImGui::Text( "%d component selected.", numselected );
                
                ComponentBase* pComponent = g_pEngineCore->GetEditorState()->m_pSelectedComponents[0];

                if( ImGui::CollapsingHeader( pComponent->GetClassname(), ImGuiTreeNodeFlags_DefaultOpen ) )
                {
                    //ImGui::Text( "TODO: Component Info." );
                    pComponent->AddAllVariablesToWatchPanel();
                }
            }
            else
            {
                ImGui::Text( "%d components selected.", numselected );
            }
        }
    }
    ImGui::End();
}

void EditorMainFrame_ImGui::AddMemoryPanel()
{
    ImGui::SetNextWindowPos( ImVec2(853, 424), ImGuiCond_FirstUseEver );
    if( ImGui::Begin( "Memory", 0, ImVec2(334, 220) ) )
    {
        for( int i=0; i<PanelMemoryPage_NumTypes; i++ )
        {
            if( i > 0 && i != 4 )
                ImGui::SameLine();
            
            if( m_CurrentMemoryPanelPage == i )
            {
                ImGui::PushStyleColor( ImGuiCol_Button, (ImVec4)ImColor::HSV(0.3f, 0.6f, 0.6f) );
                if( ImGui::SmallButton( g_PanelMemoryPagesMenuLabels[i] ) )
                    m_CurrentMemoryPanelPage = i;
                ImGui::PopStyleColor(1);
            }
            else
            {
                if( ImGui::SmallButton( g_PanelMemoryPagesMenuLabels[i] ) )
                    m_CurrentMemoryPanelPage = i;
            }
        }

        switch( m_CurrentMemoryPanelPage )
        {
        case PanelMemoryPage_Materials:
            {
                AddMemoryPanel_Materials();
            }
            break;

        case PanelMemoryPage_Textures:
            {
                AddMemoryPanel_Textures();
            }
            break;

        case PanelMemoryPage_ShaderGroups:
            {
            }
            break;

        case PanelMemoryPage_SoundCues:
            {
            }
            break;

        case PanelMemoryPage_Files:
            {
            }
            break;

        case PanelMemoryPage_Buffers:
            {
            }
            break;

        case PanelMemoryPage_DrawCalls:
            {
            }
            break;

        case PanelMemoryPage_NumTypes:
            MyAssert( 0 );
            break;
        }
    }
    ImGui::End();
}

void EditorMainFrame_ImGui::AddMemoryPanel_Materials()
{
    m_pMaterialToPreview = 0;

    MaterialDefinition* pMat = g_pMaterialManager->GetFirstMaterial();

    ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    ImGuiTreeNodeFlags node_flags = baseNodeFlags;
    if( ImGui::TreeNodeEx( "All Materials", node_flags | ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        // TODO: folders for materials.
        //const char* foldername = pMat->GetFile()->GetNameOfDeepestFolderPath();

        while( pMat )
        {
            if( ImGui::TreeNodeEx( pMat->GetName(), ImGuiTreeNodeFlags_Leaf | node_flags ) )
            {
                if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                {
                    if( ImGui::MenuItem( "Edit Material", 0, &m_IsMaterialEditorOpen ) ) { m_pMaterialBeingEdited = pMat; ImGui::CloseCurrentPopup(); }
                    if( ImGui::MenuItem( "Unload File (TODO)" ) )                        { ImGui::CloseCurrentPopup(); }
                    if( ImGui::MenuItem( "Find References (TODO)" ) )                    { ImGui::CloseCurrentPopup(); } ;// (%d)", pMat->GetRefCount() ) {}
                    ImGui::EndPopup();
                }

                if( ImGui::IsItemHovered() )
                {
                    if( ImGui::IsMouseDoubleClicked( 0 ) )
                    {
                        m_IsMaterialEditorOpen = true;
                        m_pMaterialBeingEdited = pMat;
                    }

                    ImGui::BeginTooltip();
                    m_pMaterialToPreview = pMat;
                    ImGui::Text( "%s", m_pMaterialToPreview->GetName() );
                    AddMaterialPreview( false, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 1 ) );
                    ImGui::EndTooltip();
                }

                if( ImGui::BeginDragDropSource() )
                {
                    ImGui::SetDragDropPayload( "Material", &pMat, sizeof(pMat), ImGuiCond_Once );
                    m_pMaterialToPreview = pMat;
                    ImGui::Text( "%s", m_pMaterialToPreview->GetName() );
                    AddMaterialPreview( false, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 0.5f ) );
                    ImGui::EndDragDropSource();
                }

                ImGui::TreePop();
            }
            pMat = (MaterialDefinition*)pMat->GetNext();
        }
        ImGui::TreePop();
    }
}

void EditorMainFrame_ImGui::AddMemoryPanel_Textures()
{
    bool numtextureshown = 0;

    for( int i=0; i<2; i++ )
    {
        TextureDefinition* pTex = (TextureDefinition*)g_pTextureManager->m_TexturesStillLoading.GetHead();
        if( i == 1 )
            pTex = (TextureDefinition*)g_pTextureManager->m_LoadedTextures.GetHead();

        if( pTex )
        {
            ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

            ImGuiTreeNodeFlags node_flags = baseNodeFlags;
            char* label = "Textures - Loading";
            if( i == 1 )
                label = "Textures";

            if( ImGui::TreeNodeEx( label, node_flags | ImGuiTreeNodeFlags_DefaultOpen ) )
            {
                while( pTex )
                {
                    if( pTex->m_ShowInMemoryPanel )
                    {
                        numtextureshown++;

                        if( ImGui::TreeNodeEx( pTex->GetFilename(), ImGuiTreeNodeFlags_Leaf | node_flags ) )
                        {
                            if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                            {
                                if( ImGui::MenuItem( "Unload File (TODO)" ) )     { ImGui::CloseCurrentPopup(); }
                                if( ImGui::MenuItem( "Find References (TODO)" ) ) { ImGui::CloseCurrentPopup(); } ;// (%d)", pMat->GetRefCount() ) {}
                                ImGui::EndPopup();
                            }

                            if( ImGui::IsItemHovered() )
                            {
                                ImGui::BeginTooltip();
                                //ImGui::Text( "%s", pTex->GetFilename() );
                                AddTexturePreview( false, pTex, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 1 ) );
                                ImGui::EndTooltip();
                            }

                            if( ImGui::BeginDragDropSource() )
                            {
                                ImGui::SetDragDropPayload( "Texture", &pTex, sizeof(pTex), ImGuiCond_Once );
                                //ImGui::Text( "%s", pTex->GetFilename() );
                                AddTexturePreview( false, pTex, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 1 ) );
                                ImGui::EndDragDropSource();
                            }

                            ImGui::TreePop();
                        }
                    }

                    pTex = (TextureDefinition*)pTex->GetNext();
                }
                ImGui::TreePop();
            }
        }
    }

    if( numtextureshown == 0 )
    {
        ImGui::TreeNodeEx( "No textures loaded.", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen );
    }
}

void EditorMainFrame_ImGui::DrawGameAndEditorWindows(EngineCore* pEngineCore)
{
    if( m_GameWindowSize.LengthSquared() != 0 )
    {
        if( m_pGameFBO->m_pColorTexture )
        {
            // Draw game view.
            m_pGameFBO->Bind( false );
            pEngineCore->OnSurfaceChanged( 0, 0, (unsigned int)m_GameWindowSize.x, (unsigned int)m_GameWindowSize.y );

            pEngineCore->GetComponentSystemManager()->OnDrawFrame();
            MyBindFramebuffer( GL_FRAMEBUFFER, 0, 0, 0 );
        }
    }

    if( m_EditorWindowSize.LengthSquared() != 0 )
    {
        if( m_pEditorFBO->m_pColorTexture )
        {
            // Draw editor view.
            g_GLCanvasIDActive = 1;
            pEngineCore->Editor_OnSurfaceChanged( 0, 0, (unsigned int)m_EditorWindowSize.x, (unsigned int)m_EditorWindowSize.y );

            m_pEditorFBO->Bind( false );
            pEngineCore->GetCurrentEditorInterface()->OnDrawFrame( 1 );
            MyBindFramebuffer( GL_FRAMEBUFFER, 0, 0, 0 );

            g_GLCanvasIDActive = 0;
        }
    }

    if( m_pMaterialToPreview != 0 )
    {
        if( m_pMaterialPreviewFBO->m_pColorTexture )
        {
            // Draw game view.
            m_pMaterialPreviewFBO->Bind( false );
            
            MyMesh* pMeshBall = g_pEngineCore->GetMesh_MaterialBall();

            if( pMeshBall )
            {
                m_pMaterialPreviewFBO->Bind( true );

                glDisable( GL_SCISSOR_TEST );
                glViewport( 0, 0, m_pMaterialPreviewFBO->m_Width, m_pMaterialPreviewFBO->m_Height );

                glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );
                glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

                pMeshBall->SetMaterial( m_pMaterialToPreview, 0 );

                MyMatrix matview;
                matview.SetIdentity();
#if MYFW_RIGHTHANDED
                matview.Translate( 0, 0, -4 );
#else
                matview.Translate( 0, 0, 4 );
#endif

                float aspect = (float)m_pMaterialPreviewFBO->m_Width / m_pMaterialPreviewFBO->m_Height;
                MyMatrix matproj;
                matproj.CreatePerspectiveVFoV( 45, aspect, 0.01f, 100 );

                MyMatrix matviewproj = matproj * matview;
                Vector3 campos = matview.GetTranslation() * -1;
                Vector3 camrot( 0, 0, 0 );

                float time = (float)MyTime_GetRunningTime();

                // Create 2 rotating lights for material render.
                MyLight light1;
                light1.m_Attenuation.Set( 1, 0.1f, 0.01f );
                light1.m_Color.Set( 1, 1, 1, 1 );
                light1.m_LightType = LightType_Point;
                light1.m_Position.Set( 2*cos(time), 1, 2*sin(time) );

                MyLight light2;
                light2.m_Attenuation.Set( 1, 0.1f, 0.01f );
                light2.m_Color.Set( 1, 1, 1, 1 );
                light2.m_LightType = LightType_Point;
                light2.m_Position.Set( 2*cos(PI+time), 1, 2*sin(PI+time) );

                MyLight* lights[] = { &light1, &light2 };
                pMeshBall->Draw( 0, &matviewproj, &campos, &camrot, lights, 2, 0, 0, 0, 0 );

                m_pMaterialPreviewFBO->Unbind( true );
            }
        }
    }
}

void EditorMainFrame_ImGui::AddMaterialPreview(bool createWindow, ImVec2 requestedSize, ImVec4 tint)
{
    if( createWindow == false || ImGui::Begin( "Material", 0, ImVec2(150, 150), 1 ) )
    {
        TextureDefinition* pTexture = m_pMaterialPreviewFBO->m_pColorTexture;
        int texw = m_pMaterialPreviewFBO->m_TextureWidth;
        int texh = m_pMaterialPreviewFBO->m_TextureHeight;

        ImVec2 size = requestedSize;
        if( size.x == 0 )
            size = ImGui::GetContentRegionAvail();
        if( size.x > size.y ) size.x = size.y;
        if( size.y > size.x ) size.y = size.x;

        if( pTexture )
        {
            int w = pTexture->GetWidth();
            int h = pTexture->GetHeight();
            //ImGui::ImageButton( (void*)pTexture->GetTextureID(), size, ImVec2(0,(float)h/texh), ImVec2((float)w/texw,0), -1, ImVec4(0,0,0,1) );
            ImGui::Image( (void*)pTexture->GetTextureID(), size, ImVec2(0,(float)h/texh), ImVec2((float)w/texw,0), tint );
        }
    }

    if( createWindow == true )
    {
        ImGui::End(); // ImGui::Begin( "Material"...
    }
}

void EditorMainFrame_ImGui::AddTexturePreview(bool createWindow, TextureDefinition* pTex, ImVec2 requestedSize, ImVec4 tint)
{
    if( createWindow == false || ImGui::Begin( "Texture", 0, ImVec2(150, 150), 1 ) )
    {
        TextureDefinition* pTexture = pTex;
        int texw = pTex->GetWidth();
        int texh = pTex->GetHeight();

        ImVec2 size = requestedSize;
        if( size.x == 0 )
            size = ImGui::GetContentRegionAvail();
        if( size.x > size.y ) size.x = size.y;
        if( size.y > size.x ) size.y = size.x;

        if( pTexture )
        {
            int w = pTexture->GetWidth();
            int h = pTexture->GetHeight();
            //ImGui::ImageButton( (void*)pTex->GetTextureID(), size, ImVec2(0,(float)h/texh), ImVec2((float)w/texw,0), -1, ImVec4(0,0,0,1) );
            ImGui::Image( (void*)pTex->GetTextureID(), size, ImVec2(0,0), ImVec2((float)w/texw,(float)h/texh), tint );
        }
    }

    if( createWindow == true )
    {
        ImGui::End(); // ImGui::Begin( "Texture"...
    }
}

void EditorMainFrame_ImGui::AddDebug_MousePicker()
{
    if( ImGui::Begin( "Mouse Picker", 0, ImVec2(150, 150), 1 ) )
    {
        TextureDefinition* pTexture = g_pEngineCore->GetEditorState()->m_pMousePickerFBO->m_pColorTexture;
        int texw = g_pEngineCore->GetEditorState()->m_pMousePickerFBO->m_TextureWidth;
        int texh = g_pEngineCore->GetEditorState()->m_pMousePickerFBO->m_TextureHeight;

        ImVec2 size = ImGui::GetContentRegionAvail();
        if( size.x > size.y ) size.x = size.y;
        if( size.y > size.x ) size.y = size.x;

        if( pTexture )
        {
            int w = pTexture->GetWidth();
            int h = pTexture->GetHeight();
            ImGui::Image( (void*)pTexture->GetTextureID(), size, ImVec2(0,(float)h/texh), ImVec2((float)w/texw,0) );
        }
    }
    ImGui::End();
}

void EditorMainFrame_ImGui::AddMaterialEditor()
{
    ImGui::SetNextWindowPos( ImVec2(856, 71), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowSize( ImVec2(339, 349), ImGuiCond_FirstUseEver );
    if( !ImGui::Begin( "Material Editor",  &m_IsMaterialEditorOpen ) )
    {
        ImGui::End();
        return;
    }

    // Create a context menu only available from the title bar.
    if( ImGui::BeginPopupContextItem() )
    {
        if( ImGui::MenuItem( "Close" ) )
            m_IsMaterialEditorOpen = false;

        ImGui::EndPopup();
    }

    {
        MaterialDefinition* pMat = m_pMaterialBeingEdited;
        bool showbuiltinuniforms = true;
        bool showexposeduniforms = true;
        ShaderGroup* pShaderGroup = pMat->GetShader();
        ShaderGroup* pShaderGroupInstanced = pMat->GetShaderInstanced();

        ImGui::Text( "WORK IN PROGRESS - NO UNDO - MANUAL SAVE" );
        ImGui::Text( pMat->GetName() );

        if( showbuiltinuniforms )
        {
            //g_pPanelWatch->AddEnum( "Blend", (int*)&m_BlendType, MaterialBlendType_NumTypes, MaterialBlendTypeStrings );
            const char** items = MaterialBlendTypeStrings;
            int currentItem = pMat->m_BlendType;
            const char* currentItemStr = MaterialBlendTypeStrings[currentItem];
            if( ImGui::BeginCombo( "Blend", currentItemStr ) )
            {
                for( int n = 0; n < MaterialBlendType_NumTypes; n++ )
                {
                    bool is_selected = (n == currentItem);
                    if( ImGui::Selectable( items[n], is_selected ) )
                    {
                        //// Store the old value.
                        //ComponentVariableValue oldvalue( this, pVar );

                        //// Change the value.
                        pMat->m_BlendType = (MaterialBlendType)n;

                        //// Store the new value.
                        //ComponentVariableValue newvalue( this, pVar );

                        //g_pEngineCore->GetCommandStack()->Do(
                        //    MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                        //                                this, pVar, newvalue, oldvalue, true ),
                        //    false );
                    }
                    if( is_selected )
                    {
                        // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::DragFloat2( "UVScale", &pMat->m_UVScale.x, 0.01f, 0, 1 );
            ImGui::DragFloat2( "UVOffset", &pMat->m_UVOffset.x, 0.01f, 0, 1 );

            const char* desc = "no shader";
            if( pShaderGroup && pShaderGroup->GetShader( ShaderPass_Main )->m_pFile )
                desc = pShaderGroup->GetShader( ShaderPass_Main )->m_pFile->GetFilenameWithoutExtension();
            ImGui::Button( desc, ImVec2( ImGui::GetWindowWidth() * 0.65f, 0 ) );
            ImGui::SameLine();
            ImGui::Text( "Shader" );
            //m_ControlID_Shader = g_pPanelWatch->AddPointerWithDescription( "Shader", 0, desc, this, MaterialDefinition::StaticOnDropShader, 0, MaterialDefinition::StaticOnRightClickShader );

            desc = "no shader";
            if( pShaderGroupInstanced && pShaderGroupInstanced->GetShader( ShaderPass_Main )->m_pFile )
                desc = pShaderGroupInstanced->GetShader( ShaderPass_Main )->m_pFile->GetFilenameWithoutExtension();
            ImGui::Button( desc, ImVec2( ImGui::GetWindowWidth() * 0.65f, 0 ) );
            ImGui::SameLine();
            ImGui::Text( "Instanced Shader" );
            //m_ControlID_ShaderInstanced = g_pPanelWatch->AddPointerWithDescription( "Shader Instanced", 0, desc, this, MaterialDefinition::StaticOnDropShader, 0, MaterialDefinition::StaticOnRightClickShader );

            TextureDefinition* pTextureColor = pMat->GetTextureColor();
            desc = "no color texture";
            if( pTextureColor )
                desc = pTextureColor->GetFilename();
            ImGui::Button( desc, ImVec2( ImGui::GetWindowWidth() * 0.65f, 0 ) );
            if( ImGui::BeginDragDropTarget() )
            {
                if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "Texture" ) )
                {
                    pMat->SetTextureColor( (TextureDefinition*)*(void**)payload->Data );
                }
                ImGui::EndDragDropTarget();
            }
            if( ImGui::IsItemHovered() )
            {
                if( pTextureColor )
                {
                    ImGui::BeginTooltip();
                    //ImGui::Text( "%s", pTex->GetFilename() );
                    AddTexturePreview( false, pTextureColor, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 1 ) );
                    ImGui::EndTooltip();
                }

                if( ImGui::IsMouseDoubleClicked( 0 ) )
                {
                    pMat->SetTextureColor( 0 );
                }
            }
            ImGui::SameLine();
            ImGui::Text( "Color Texture" );
            //g_pPanelWatch->AddPointerWithDescription( "Color Texture", 0, desc, this, MaterialDefinition::StaticOnDropTexture, 0, MaterialDefinition::StaticOnRightClickTexture );

            // TODO: Copies of these colors are changing, fix that.
            ColorFloat ambientColorFloat = pMat->m_ColorAmbient.AsColorFloat();
            if( ImGui::ColorEdit4( "Ambient Color", &ambientColorFloat.r ) )
            {
                pMat->m_ColorAmbient.SetFromColorFloat( ambientColorFloat );
            }

            ColorFloat diffuseColorFloat = pMat->m_ColorDiffuse.AsColorFloat();
            if( ImGui::ColorEdit4( "Diffuse Color", &diffuseColorFloat.r ) )
            {
                pMat->m_ColorDiffuse.SetFromColorFloat( diffuseColorFloat );
            }

            ColorFloat specularColorFloat = pMat->m_ColorSpecular.AsColorFloat();
            if( ImGui::ColorEdit4( "Specular Color", &specularColorFloat.r ) )
            {
                pMat->m_ColorSpecular.SetFromColorFloat( specularColorFloat );
            }

            ImGui::DragFloat( "Shininess", &pMat->m_Shininess );
        }

        if( showexposeduniforms && pShaderGroup )
        {
            MyFileObjectShader* pShaderFile = pShaderGroup->GetFile();
            if( pShaderFile )
            {
                for( unsigned int i=0; i<pShaderFile->m_NumExposedUniforms; i++ )
                {
                    char tempname[32];

                    // Hardcoding to remove the 'u_' I like to stick at the start of my uniform names.
                    if( pShaderFile->m_ExposedUniforms[i].m_Name[1] == '_' )
                        strcpy_s( tempname, 32, &pShaderFile->m_ExposedUniforms[i].m_Name[2] );
                    else
                        strcpy_s( tempname, 32, pShaderFile->m_ExposedUniforms[i].m_Name );

                    switch( pShaderFile->m_ExposedUniforms[i].m_Type )
                    {
                    case ExposedUniformType_Float:
                        ImGui::DragFloat( tempname, &pMat->m_UniformValues[i].m_Float );
                        //g_pPanelWatch->AddFloat( tempname, &m_UniformValues[i].m_Float, 0, 1 );
                        break;

                    case ExposedUniformType_Vec2:
                        ImGui::DragFloat2( tempname, &pMat->m_UniformValues[i].m_Vec2[0] );
                        //g_pPanelWatch->AddVector2( tempname, (Vector2*)&m_UniformValues[i].m_Vec2, 0, 1 );
                        break;

                    case ExposedUniformType_Vec3:
                        ImGui::DragFloat3( tempname, &pMat->m_UniformValues[i].m_Vec3[0] );
                        //g_pPanelWatch->AddVector3( tempname, (Vector3*)&m_UniformValues[i].m_Vec3, 0, 1 );
                        break;

                    case ExposedUniformType_Vec4:
                        ImGui::DragFloat4( tempname, &pMat->m_UniformValues[i].m_Vec4[0] );
                        //g_pPanelWatch->AddVector4( tempname, (Vector4*)&m_UniformValues[i].m_Vec4, 0, 1 );
                        break;

                    case ExposedUniformType_ColorByte:
                        {
                            ColorFloat color = ((ColorByte*)&pMat->m_UniformValues[i].m_ColorByte)->AsColorFloat();
                            if( ImGui::ColorEdit4( tempname, &color.r ) )
                            {
                                ((ColorByte*)&pMat->m_UniformValues[i].m_ColorByte)->SetFromColorFloat( color );
                            }
                            //g_pPanelWatch->AddColorByte( tempname, (ColorByte*)&m_UniformValues[i].m_ColorByte, 0, 255 );
                        }
                        break;

                    case ExposedUniformType_Sampler2D:
                        ImGui::Text( "%s (TODO) - Sampler2D", tempname );
                        //m_UniformValues[i].m_ControlID = g_pPanelWatch->AddPointerWithDescription(
                        //    tempname, m_UniformValues[i].m_pTexture,
                        //    m_UniformValues[i].m_pTexture ? m_UniformValues[i].m_pTexture->GetFilename() : "Texture Not Set",
                        //    this, MaterialDefinition::StaticOnDropTexture, 0, 0 );                    
                        break;

                    case ExposedUniformType_NotSet:
                    default:
                        MyAssert( false );
                        break;
                    }
                }
            }

            {
                ImGui::Separator();
                ImGui::Text( "MANUAL SAVE" );
                if( ImGui::Button( "Save" ) )
                {
                    pMat->SaveMaterial( 0 );
                }
                ImGui::SameLine();
                ImGui::Text( "<- MANUAL SAVE" );
            }
        }
    }

    ImGui::End();
}
