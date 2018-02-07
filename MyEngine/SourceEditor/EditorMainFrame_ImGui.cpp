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

EditorMainFrame_ImGui::EditorMainFrame_ImGui()
{
    m_pGameFBO = g_pTextureManager->CreateFBO( 1024, 1024, GL_NEAREST, GL_NEAREST, true, 32, true );
    m_pEditorFBO = g_pTextureManager->CreateFBO( 1024, 1024, GL_NEAREST, GL_NEAREST, true, 32, true );

    m_GameWindowPos.Set( -1, -1 );
    m_EditorWindowPos.Set( -1, -1 );
    m_GameWindowSize.Set( 0, 0 );
    m_EditorWindowSize.Set( 0, 0 );

    m_GameWindowFocused = false;
    m_EditorWindowHovered = false;
    m_EditorWindowFocused = false;

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
        if( C  && keycode == ' ' ) { EditorMenuCommand( EditorMenuCommand_TogglePlayStop );         return true; }
        if( C  && keycode == 'Z' ) { EditorMenuCommand( EditorMenuCommand_Undo );                   return true; }
        if( C  && keycode == 'Y' ) { EditorMenuCommand( EditorMenuCommand_Redo );                   return true; }
        if( CS && keycode == 'Z' ) { EditorMenuCommand( EditorMenuCommand_Redo );                   return true; }
    }

    return false;
}

void EditorMainFrame_ImGui::AddEverything()
{
    AddMainMenuBar();
    AddGameAndEditorWindows();
    AddObjectList();
    AddWatchPanel();
    AddDebug_MousePicker();

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
    if( pGO )
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
            //if( ImGui::MenuItem( "&New", "CTRL-N" ) ) {}
            //if( ImGui::MenuItem( "&Open...", "CTRL-O" ) ) {}
            //ImGui::Separator();
            //if( ImGui::MenuItem( "&Test...", "CTRL-T" ) ) {}
            if( ImGui::MenuItem( "&New Scene" ) ) {  }
            if( ImGui::MenuItem( "&Load Scene..." ) ) { EditorMenuCommand( EditorMenuCommand_File_LoadScene ); }
            if( ImGui::MenuItem( "&Create Additional Scene" ) ) {}
            if( ImGui::MenuItem( "&Load Additional Scene..." ) ) {}
            if( ImGui::MenuItem( "&Save Scene", "Ctrl-S" ) ) { EditorMenuCommand( EditorMenuCommand_File_SaveScene ); }
            if( ImGui::MenuItem( "Save Scene &As..." ) ) {}

            if( ImGui::BeginMenu( "E&xport" ) )
            {
                if( ImGui::MenuItem( "Box2D Scene...", "Ctrl-Shift-E" ) ) { EditorMenuCommand( EditorMenuCommand_File_Export_Box2DScene ); }
                ImGui::EndMenu();
            }
            if( ImGui::MenuItem( "&Quit" ) ) {}

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Edit" ) )
        {
            if( ImGui::MenuItem( "&Undo", "CTRL-Z" ) ) {}
            if( ImGui::MenuItem( "&Redo", "CTRL-Y" ) ) {}

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "View" ) )
        {
            if( ImGui::MenuItem( "&Save window layout" ) ) {} // { EditorMenuCommand( myID_View_SavePerspective ); }
            if( ImGui::MenuItem( "&Load window layout" ) ) {} // { EditorMenuCommand( myID_View_LoadPerspective ); }
            if( ImGui::MenuItem( "&Reset window layout" ) ) {} // { EditorMenuCommand( myID_View_ResetPerspective ); }

            if( ImGui::BeginMenu( "Editor Windows" ) )
            {
                for( int i=0; i<EditorWindow_NumTypes; i++ )
                {
                    if( ImGui::MenuItem( g_DefaultEditorWindowTypeMenuLabels[i] ) ) {}
                }
                ImGui::EndMenu();
            }

            //if( ImGui::BeginMenu( "Editor Perspectives" ) )
            //{
            //    for( int i=0; i<Perspective_NumPerspectives; i++ )
            //    {
            //        if( ImGui::MenuItem( g_DefaultEditorWindowTypeMenuLabels[i] ) ) {}
            //    }
            //    ImGui::EndMenu();
            //}

            //    m_SubMenu_View_EditorPerspectives = MyNew wxMenu;
            //    for( int i=0; i<Perspective_NumPerspectives; i++ )
            //    {
            //        m_MenuItem_View_EditorPerspectiveOptions[i] = m_SubMenu_View_EditorPerspectives->AppendCheckItem( myIDEngine_View_EditorPerspective + i, g_DefaultPerspectiveMenuLabels[i], wxEmptyString );
            //        Connect( myIDEngine_View_EditorPerspective + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
            //    }

            //    m_SubMenu_View_GameplayPerspectives = MyNew wxMenu;
            //    for( int i=0; i<Perspective_NumPerspectives; i++ )
            //    {
            //        m_MenuItem_View_GameplayPerspectiveOptions[i] = m_SubMenu_View_GameplayPerspectives->AppendCheckItem( myIDEngine_View_GameplayPerspective + i, g_DefaultPerspectiveMenuLabels[i], wxEmptyString );
            //        Connect( myIDEngine_View_GameplayPerspective + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
            //    }

            //    m_MenuItem_View_EditorPerspectiveOptions[0]->Check();
            //    m_MenuItem_View_GameplayPerspectiveOptions[0]->Check();

            //    m_View->Append( myIDEngine_View_EditorPerspectives, "Editor Layouts", m_SubMenu_View_EditorPerspectives );
            //    m_View->Append( myIDEngine_View_GameplayPerspectives, "Gameplay Layouts", m_SubMenu_View_GameplayPerspectives );

            //    m_MenuItem_View_ShowEditorIcons = m_View->AppendCheckItem( myIDEngine_View_ShowEditorIcons, wxT("Show &Editor Icons\tShift-F7") );

            //    wxMenu* pMenuSelectedObjects = MyNew wxMenu;
            //    m_View->AppendSubMenu( pMenuSelectedObjects, "Selected Objects" );
            //    m_MenuItem_View_SelectedObjects_ShowWireframe = pMenuSelectedObjects->AppendCheckItem( myIDEngine_View_SelectedObjects_ShowWireframe, wxT("Show &Wireframe") );
            //    m_MenuItem_View_SelectedObjects_ShowEffect = pMenuSelectedObjects->AppendCheckItem( myIDEngine_View_SelectedObjects_ShowEffect, wxT("Show &Effect") );

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

            //        m_View->Append( myIDEngine_View_EditorCameraLayers, "Editor &Camera Layers", m_SubMenu_View_EditorCameraLayers );
            //    }

            //    m_View->Append( myIDEngine_View_FullScreenEditor, wxT("&Fullscreen Editor\tF11") );
            //    m_View->Append( myIDEngine_View_FullScreenGame, wxT("&Fullscreen Game\tCtrl-F11") );
            //}

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Aspect" ) )
        {
            if( ImGui::MenuItem( "&Fill", "Alt-1" ) ) {}
            if( ImGui::MenuItem( "&Tall", "Alt-2" ) ) {}
            if( ImGui::MenuItem( "&Square", "Alt-3" ) ) {}
            if( ImGui::MenuItem( "&Wide", "Alt-4" ) ) {}

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Grid" ) )
        {
            if( ImGui::MenuItem( "Grid &On/Off", "Ctrl-Shift-V", true ) ) {} // { EditorMenuCommand( myIDEngine_Grid_VisibleOnOff ); }
            if( ImGui::MenuItem( "Grid Snap &On/Off", "Ctrl-G", true ) ) {} // { EditorMenuCommand( myIDEngine_Grid_SnapOnOff ); }
            if( ImGui::MenuItem( "Grid &Settings", "Ctrl-Shift-G" ) ) {}

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Mode" ) )
        {
            if( ImGui::MenuItem( "Switch &Focus on Play/Stop", 0, true ) ) {} // { EditorMenuCommand( myIDEngine_Mode_SwitchFocusOnPlayStop ); }
            //// Since Command-Space is "Spotlight Search" on OSX, use the actual control key on OSX as well as Windows/Linux.
            if( ImGui::MenuItem( "&Play/Stop", "CTRL-SPACE" ) ) {} // { EditorMenuCommand( myIDEngine_Mode_PlayStop ); }
            if( ImGui::MenuItem( "Pause", "Ctrl-." ) ) {} // { EditorMenuCommand( myIDEngine_Mode_Pause ); }
            if( ImGui::MenuItem( "Advance 1 Frame", "Ctrl-]" ) ) {} // { EditorMenuCommand( myIDEngine_Mode_Advance1Frame ); }
            if( ImGui::MenuItem( "Advance 1 Second", "Ctrl-[" ) ) {} // { EditorMenuCommand( myIDEngine_Mode_Advance1Second ); }

            if( ImGui::BeginMenu( "L&aunch Platforms" ) )
            {
                //for( int i=0; i<LaunchPlatform_NumPlatforms; i++ )
                //{
                //    if( ImGui::MenuItem( g_LaunchPlatformsMenuLabels[i], 0, true ) ) {} // { EditorMenuCommand( myIDEngine_Mode_LaunchPlatforms + i ); }
                //}

                ImGui::EndMenu();
            }

            if( ImGui::MenuItem( "&Launch Game", "tCtrl-F5" ) ) {} // { EditorMenuCommand( myIDEngine_Mode_LaunchGame ); }

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Data" ) )
        {
            if( ImGui::MenuItem( "&Load Datafiles" ) ) {} // { EditorMenuCommand( myIDEngine_Data_AddDatafile ); }
            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Hackery" ) )
        {
            if( ImGui::MenuItem( "&Record", "Ctrl-R" ) ) {} // { EditorMenuCommand( myIDEngine_Hackery_RecordMacro ); }
            if( ImGui::MenuItem( "Stop recording and &Execute", "Ctrl-E" ) ) {} // { EditorMenuCommand( myIDEngine_Hackery_ExecuteMacro ); }
            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Debug views" ) )
        {
            if( ImGui::MenuItem( "Show &Mouse Picker FBO", "F9" ) ) {} // { EditorMenuCommand( myIDEngine_Debug_ShowMousePickerFBO ); }
            if( ImGui::MenuItem( "Show &Animated Debug View for Selection", "F8" ) ) {} // { EditorMenuCommand( myIDEngine_Debug_ShowSelectedAnimatedMesh ); }
            if( ImGui::MenuItem( "Show &GL Stats", "Shift-F9" ) ) {} // { EditorMenuCommand( myIDEngine_Debug_ShowGLStats ); }
            if( ImGui::MenuItem( "Draw &Wireframe", "Ctrl-F9" ) ) {} // { EditorMenuCommand( myIDEngine_Debug_DrawWireframe ); }
            if( ImGui::MenuItem( "Show &Physics debug shapes", "Shift-F8" ) ) {} // { EditorMenuCommand( myIDEngine_Debug_ShowPhysicsShapes ); }
            if( ImGui::MenuItem( "Show profiling &Info", "Ctrl-F8" ) ) {} // { EditorMenuCommand( myIDEngine_Debug_ShowProfilingInfo ); }
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
                    if( ImGui::TreeNodeEx( scenename, node_flags ) )
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
                ImGui::TreeNodeEx( pComponent, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, pComponent->GetClassname() );
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
        else
        {
            ImGui::Text( "%d objects selected.", numselected );
        }
    }
    ImGui::End();
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
}

void EditorMainFrame_ImGui::AddDebug_MousePicker()
{
    if( ImGui::Begin( "Mouse Picker", 0, ImVec2(150, 150), 1 ) )
    {
        TextureDefinition* pTexture = g_pEngineCore->GetEditorState()->m_pMousePickerFBO->m_pColorTexture;
        int texw = g_pEngineCore->GetEditorState()->m_pMousePickerFBO->m_TextureWidth;
        int texh = g_pEngineCore->GetEditorState()->m_pMousePickerFBO->m_TextureHeight;

        if( pTexture )
        {
            int w = pTexture->GetWidth();
            int h = pTexture->GetHeight();
            ImGui::Image( (void*)pTexture->GetTextureID(), ImVec2( 150, 150 ), ImVec2(0,(float)h/texh), ImVec2((float)w/texw,0) );
        }
    }
    ImGui::End();
}
