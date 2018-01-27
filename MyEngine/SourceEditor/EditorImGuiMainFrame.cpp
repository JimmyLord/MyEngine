//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

EditorImGuiMainFrame::EditorImGuiMainFrame()
{
    m_pGameFBO = g_pTextureManager->CreateFBO( 1024, 1024, GL_NEAREST, GL_NEAREST, true, 32, true );
    m_pEditorFBO = g_pTextureManager->CreateFBO( 1024, 1024, GL_NEAREST, GL_NEAREST, true, 32, true );
}

EditorImGuiMainFrame::~EditorImGuiMainFrame()
{
    SAFE_RELEASE( m_pGameFBO );
    SAFE_RELEASE( m_pEditorFBO );
}

void EditorImGuiMainFrame::AddEverything()
{
    AddMainMenuBar();
}

void EditorImGuiMainFrame::AddMainMenuBar()
{
    if( ImGui::BeginMainMenuBar() )
    {
        if( ImGui::BeginMenu( "File" ) )
        {
            if( ImGui::MenuItem( "&New", "CTRL+N" ) )
            {
                int bp = 1;
            }
            if( ImGui::MenuItem( "&Open...", "CTRL+O" ) ) {}
            ImGui::Separator();
            if( ImGui::MenuItem( "&Test...", "CTRL+T" ) ) {}
            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Edit" ) )
        {
            if( ImGui::MenuItem( "Undo", "CTRL+Z" ) ) {}
            if( ImGui::MenuItem( "Redo", "CTRL+Y", false, false ) ) {}  // Disabled item
            ImGui::Separator();
            if( ImGui::MenuItem( "Cut", "CTRL+X" ) ) {}
            if( ImGui::MenuItem( "Copy", "CTRL+C" ) ) {}
            if( ImGui::MenuItem( "Paste", "CTRL+V" ) ) {}
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void EditorImGuiMainFrame::DrawGameAndEditorWindows(EngineCore* pEngineCore)
{
    float windowwidth = pEngineCore->GetWindowWidth();
    float windowheight = pEngineCore->GetWindowHeight();

    if( ImGui::Begin( "Game" ) )
    {
        float w = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
        float h = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;
        if( w > 0 && h > 0 )
        {
            if( w > m_pGameFBO->m_TextureWidth || h > m_pGameFBO->m_TextureHeight )
            {
                // The FBO will be recreated during the TextureManager tick.
                g_pTextureManager->InvalidateFBO( m_pGameFBO );
                m_pGameFBO->Setup( (unsigned int)w, (unsigned int)h, GL_NEAREST, GL_NEAREST, true, 32, false );
            }

            if( m_pGameFBO->m_pColorTexture )
            {
                // Draw game view.
                m_pGameFBO->Bind( false );
                pEngineCore->OnSurfaceChanged( 0, 0, (unsigned int)w, (unsigned int)h );

                pEngineCore->GetComponentSystemManager()->OnDrawFrame();
                glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            }

            if( m_pGameFBO->m_pColorTexture )
            {
                TextureDefinition* tex = m_pGameFBO->m_pColorTexture;
                ImGui::Image( (void*)tex->GetTextureID(), ImVec2( w, h ), ImVec2(0,h/m_pGameFBO->m_TextureHeight), ImVec2(w/m_pGameFBO->m_TextureWidth,0) );
            }
        }
    }
    ImGui::End();

    // Draw the editor window into a texture.
    if( ImGui::Begin( "Editor" ) )
    {
        float w = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
        float h = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;
        if( w > 0 && h > 0 )
        {
            if( w > m_pEditorFBO->m_TextureWidth || h > m_pEditorFBO->m_TextureHeight )
            {
                // The FBO will be recreated during the TextureManager tick.
                g_pTextureManager->InvalidateFBO( m_pEditorFBO );
                m_pEditorFBO->Setup( (unsigned int)w, (unsigned int)h, GL_NEAREST, GL_NEAREST, true, 32, false );
            }

            if( m_pEditorFBO->m_pColorTexture )
            {
                g_GLCanvasIDActive = 1;
                pEngineCore->Editor_OnSurfaceChanged( 0, 0, (unsigned int)w, (unsigned int)h );

                // Draw editor view.
                m_pEditorFBO->Bind( false );
                pEngineCore->GetCurrentEditorInterface()->OnDrawFrame( 1 );
                glBindFramebuffer( GL_FRAMEBUFFER, 0 );

                g_GLCanvasIDActive = 0;
            }

            if( m_pEditorFBO->m_pColorTexture )
            {
                TextureDefinition* tex = m_pEditorFBO->m_pColorTexture;
                ImGui::Image( (void*)tex->GetTextureID(), ImVec2( w, h ), ImVec2(0,h/m_pEditorFBO->m_TextureHeight), ImVec2(w/m_pEditorFBO->m_TextureWidth,0) );
            }
        }
    }
    ImGui::End();

    // Reset to full window size.
    pEngineCore->OnSurfaceChanged( 0, 0, (unsigned int)windowwidth, (unsigned int)windowheight );
}
