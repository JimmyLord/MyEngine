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
    if( ImGui::IsMouseHoveringAnyWindow() )
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

        // Are absolute x/y over the game window.
        if( mouseabsx >= m_GameWindowPos.x && mouseabsx < m_GameWindowPos.x + m_GameWindowSize.x &&
            mouseabsy >= m_GameWindowPos.y && mouseabsy < m_GameWindowPos.y + m_GameWindowSize.y )
        {
            //ImGui::Begin( "Debug" );
            //ImGui::Text( "In Game Window" );
            //ImGui::End();
        }

        // Are absolute x/y over the editor window.
        if( mouseabsx >= m_EditorWindowPos.x && mouseabsx < m_EditorWindowPos.x + m_EditorWindowSize.x &&
            mouseabsy >= m_EditorWindowPos.y && mouseabsy < m_EditorWindowPos.y + m_EditorWindowSize.y )
        {
            // If this is a mouse message and not a relative movement,
            //     calculate mouse x/y relative to this window.
            if( mouseaction != -1 && mouseaction != GCBA_RelativeMovement )
            {
                localx = x - m_EditorWindowPos.x;
                localy = y - m_EditorWindowPos.y;
            }

            if( keycode != -1 )
                int bp = 1;

            // First, pass the input into the current editor interface.
            if( g_pEngineCore->GetCurrentEditorInterface()->HandleInput( keyaction, keycode, mouseaction, id, localx, localy, pressure ) )
                return true;

            // If it wasn't used, pass it to the transform gizmo.
            if( g_pEngineCore->GetEditorState()->m_pTransformGizmo->HandleInput( g_pEngineCore, -1, -1, mouseaction, id, localx, localy, pressure ) )
                return true;

            // Clear modifier key and mouse button states.
            g_pEngineCore->GetCurrentEditorInterface()->ClearModifierKeyStates( keyaction, keycode, mouseaction, id, x, y, pressure );
        }
    }

    return false;
}

void EditorImGuiMainFrame::AddEverything()
{
    AddMainMenuBar();
    AddGameAndEditorWindows();

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin( "Stuff" );
    ImGui::Text( "WantCaptureKeyboard %d", io.WantCaptureKeyboard );
    ImGui::Text( "WantCaptureMouse %d", io.WantCaptureMouse );
    ImGui::Text( "WantMoveMouse %d", io.WantMoveMouse );
    ImGui::Text( "WantTextInput %d", io.WantTextInput );
    ImGui::Text( "m_GameWindowFocused %d", m_GameWindowFocused );
    ImGui::Text( "m_EditorWindowFocused %d", m_EditorWindowFocused );
    
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
    if( ImGui::Begin( "Game" ) )
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

        ImGui::End();
    }

    if( ImGui::Begin( "Editor" ) )
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

        ImGui::End();
    }
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
