//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

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

EditorImGuiMainFrame::EditorImGuiMainFrame()
{
    m_pGameFBO = g_pTextureManager->CreateFBO( 1024, 1024, GL_NEAREST, GL_NEAREST, true, 32, true );
    m_pEditorFBO = g_pTextureManager->CreateFBO( 1024, 1024, GL_NEAREST, GL_NEAREST, true, 32, true );

    m_GameWindowPos.Set( -1, -1 );
    m_EditorWindowPos.Set( -1, -1 );
    m_GameWindowSize.Set( 0, 0 );
    m_EditorWindowSize.Set( 0, 0 );

    m_GameWindowFocused = false;
    m_EditorWindowFocused = false;
}

EditorImGuiMainFrame::~EditorImGuiMainFrame()
{
    SAFE_RELEASE( m_pGameFBO );
    SAFE_RELEASE( m_pEditorFBO );
}

Vector2 EditorImGuiMainFrame::GetEditorWindowCenterPosition()
{
    return m_EditorWindowPos + m_EditorWindowSize/2;
}

bool EditorImGuiMainFrame::HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
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
        ( mouseaction != -1 &&
            mouseabsx >= m_EditorWindowPos.x && mouseabsx < m_EditorWindowPos.x + m_EditorWindowSize.x &&
            mouseabsy >= m_EditorWindowPos.y && mouseabsy < m_EditorWindowPos.y + m_EditorWindowSize.y ) )
    {
        // If this is a mouse message and not a relative movement,
        //     calculate mouse x/y relative to this window.
        if( mouseaction != -1 && mouseaction != GCBA_RelativeMovement )
        {
            localx = x - m_EditorWindowPos.x;
            localy = y - m_EditorWindowPos.y;
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
        g_pEngineCore->GetCurrentEditorInterface()->ClearModifierKeyStates( keyaction, keycode, mouseaction, id, x, y, pressure );
    }

    return false;
}

void EditorImGuiMainFrame::AddEverything()
{
    AddMainMenuBar();
    AddGameAndEditorWindows();
    AddObjectList();
    AddWatchPanel();

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin( "Stuff" );
    ImGui::Text( "WantCaptureKeyboard %d", io.WantCaptureKeyboard );
    ImGui::Text( "WantCaptureMouse %d", io.WantCaptureMouse );
    ImGui::Text( "WantMoveMouse %d", io.WantMoveMouse );
    ImGui::Text( "WantTextInput %d", io.WantTextInput );
    ImGui::Text( "m_GameWindowFocused %d", m_GameWindowFocused );
    ImGui::Text( "m_EditorWindowFocused %d", m_EditorWindowFocused );
    ImGui::Text( "MouseWheel %0.2f", io.MouseWheel );

    ImGui::ShowDemoWindow();
    
    ImGui::End();
}

void EditorImGuiMainFrame::AddMainMenuBar()
{
    if( ImGui::BeginMainMenuBar() )
    {
        if( ImGui::BeginMenu( "File" ) )
        {
            if( ImGui::MenuItem( "&New", "CTRL+N" ) ) {}
            if( ImGui::MenuItem( "&Open...", "CTRL+O" ) ) {}
            ImGui::Separator();
            if( ImGui::MenuItem( "&Test...", "CTRL+T" ) ) {}
            if( ImGui::MenuItem( "&Quit" ) ) {}

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Edit" ) )
        {
            if( ImGui::MenuItem( "&Undo", "CTRL+Z" ) ) {}
            if( ImGui::MenuItem( "&Redo", "CTRL+Y" ) ) {}

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "View" ) )
        {
            if( ImGui::MenuItem( "&Save window layout" ) ) {}
            if( ImGui::MenuItem( "&Load window layout" ) ) {}
            if( ImGui::MenuItem( "&Reset window layout" ) ) {}

            if( ImGui::BeginMenu( "Editor Windows" ) )
            {
                for( int i=0; i<EditorWindow_NumTypes; i++ )
                {
                    if( ImGui::MenuItem( g_DefaultEditorWindowTypeMenuLabels[i] ) ) {}
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Aspect" ) )
        {
            if( ImGui::MenuItem( "&Fill", "Alt+1" ) ) {}
            if( ImGui::MenuItem( "&Tall", "Alt+2" ) ) {}
            if( ImGui::MenuItem( "&Square", "Alt+3" ) ) {}
            if( ImGui::MenuItem( "&Wide", "Alt+4" ) ) {}

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void EditorImGuiMainFrame::AddGameAndEditorWindows()
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

void EditorImGuiMainFrame::AddObjectList()
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
                    char* scenename = pUnmanagedName;
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

void EditorImGuiMainFrame::AddGameObjectToObjectList(GameObject* pGameObject)
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

void EditorImGuiMainFrame::AddWatchPanel()
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

void EditorImGuiMainFrame::DrawGameAndEditorWindows(EngineCore* pEngineCore)
{
    if( m_GameWindowSize.LengthSquared() != 0 )
    {
        if( m_pGameFBO->m_pColorTexture )
        {
            // Draw game view.
            m_pGameFBO->Bind( false );
            pEngineCore->OnSurfaceChanged( 0, 0, (unsigned int)m_GameWindowSize.x, (unsigned int)m_GameWindowSize.y );

            pEngineCore->GetComponentSystemManager()->OnDrawFrame();
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
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
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );

            g_GLCanvasIDActive = 0;
        }
    }
}
