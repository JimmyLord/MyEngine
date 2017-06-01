//
// Copyright (c) 2016-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "ImGuiManager.h"
#include "../../Libraries/imgui/imgui.h"

ImGuiManager* g_pImGuiManager = 0;

ImGuiManager::ImGuiManager()
{
}

ImGuiManager::~ImGuiManager()
{
    Shutdown();
}

void ImGuiManager::Init()
{
    static GLuint g_FontTexture = 0;

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = 300;
    io.DisplaySize.y = 300;
    //io.IniFilename = "imgui.ini";
    io.RenderDrawListsFn = ImGuiManager::RenderDrawLists;  // Setup a render function, or set to NULL and call GetDrawData() after Render() to access the render data.

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height );
        
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    io.Fonts->TexID = (void*)g_FontTexture;

    io.KeyMap[ImGuiKey_Tab]         = MYKEYCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]   = MYKEYCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow]  = MYKEYCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]     = MYKEYCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow]   = MYKEYCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp]      = MYKEYCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown]    = MYKEYCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home]        = MYKEYCODE_HOME;
    io.KeyMap[ImGuiKey_End]         = MYKEYCODE_END;
    io.KeyMap[ImGuiKey_Delete]      = MYKEYCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace]   = MYKEYCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter]       = MYKEYCODE_ENTER;
    io.KeyMap[ImGuiKey_Escape]      = MYKEYCODE_ESC;
    io.KeyMap[ImGuiKey_A]           = 'A';
    io.KeyMap[ImGuiKey_C]           = 'C';
    io.KeyMap[ImGuiKey_V]           = 'V';
    io.KeyMap[ImGuiKey_X]           = 'X';
    io.KeyMap[ImGuiKey_Y]           = 'Y';
    io.KeyMap[ImGuiKey_Z]           = 'Z';
}

void ImGuiManager::Shutdown()
{
    ImGui::Shutdown();
}

void ImGuiManager::ClearInput()
{
    //LOGInfo( "ImGui", "ClearInput()\n" );

    ImGuiIO& io = ImGui::GetIO();

    //io.MousePos.x = 0;
    //io.MousePos.y = 0;
    //io.MouseWheel = 0;

    for( int i=0; i<5; i++ )
    {
        io.MouseDown[i] = false;
    }

    for( int i=0; i<512; i++ )
    {
        io.KeysDown[i] = false;
    }
}

void ImGuiManager::OnFocusLost()
{
    ClearInput();
}

bool ImGuiManager::HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    ImGuiIO& io = ImGui::GetIO();

    //LOGInfo( "ImGui", "HandleEditorInput()\n" );

    if( mouseaction != -1 )
    {
        MyAssert( keyaction == -1 && keycode == -1 );

        io.MousePos.x = x;
        io.MousePos.y = y;

        if( mouseaction == GCBA_Down )
            io.MouseDown[id] = true;
        if( mouseaction == GCBA_Up )
            io.MouseDown[id] = false;

        if( mouseaction == GCBA_Held || mouseaction == GCBA_Wheel )
        {
            for( int i=0; i<3; i++ )
            {
                if( id & (1 << i) )
                    io.MouseDown[i] = true;
            }
        }

        io.MouseWheel = pressure;
    }
    else
    {
        MyAssert( keyaction != -1 && keycode != -1 );
        MyAssert( mouseaction == -1 && id == -1 );
        
        if( keyaction == GCBA_Down )
            io.KeysDown[keycode] = true;
        if( keyaction == GCBA_Up )
            io.KeysDown[keycode] = false;
    }

    // if a window is hovered, don't let mouse events pass through it.
    if( mouseaction != -1 && ImGui::IsMouseHoveringAnyWindow() )
        return true;

    // TODO: ignore key messages if we're entering text in an edit box.

    return false;
}

void ImGuiManager::OnChar(unsigned int c)
{
    ImGuiIO& io = ImGui::GetIO();

    if( c > 0 && c < 0x10000 )
    {
        io.AddInputCharacter( (unsigned short)c );
    }
}

void ImGuiManager::StartTick(double TimePassed)
{
    //LOGInfo( "ImGui", "StartFrame()\n" );

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = (float)TimePassed;

    io.KeyCtrl = io.KeysDown[MYKEYCODE_LCTRL] || io.KeysDown[MYKEYCODE_RCTRL];
    io.KeyShift = io.KeysDown[MYKEYCODE_LSHIFT] || io.KeysDown[MYKEYCODE_RSHIFT];
    io.KeyAlt = io.KeysDown[MYKEYCODE_LALT] || io.KeysDown[MYKEYCODE_RALT];
    //io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void ImGuiManager::StartFrame()
{
    ImGui::NewFrame();
    //ImGui::ShowTestWindow();
}

void ImGuiManager::EndFrame(float width, float height, bool draw)
{
    //LOGInfo( "ImGui", "EndFrame()\n" );

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = width;
    io.DisplaySize.y = height;

    ImGui::Render();
}

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGuiManager::RenderDrawLists(ImDrawData* draw_data)
{
#if MYFW_NACL || MYFW_BLACKBERRY || MYFW_ANDROID || MYFW_EMSCRIPTEN
    // TODO: fix me
#else
    checkGlError( "Start of ImGuiManager::RenderDrawLists()" );

    MyUseProgram( 0 );
    MyBindBuffer( GL_ARRAY_BUFFER, 0 );
    MyBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // We are using the OpenGL fixed pipeline to make the example code simpler to read!
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_2D);
    //MyUseProgram(0); // You may want this if using this code in an OpenGL 3+ context

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render command lists
    #define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const unsigned char* vtx_buffer = (const unsigned char*)&cmd_list->VtxBuffer.front();
        const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, uv)));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
            }
            idx_buffer += pcmd->ElemCount;
        }
    }
    #undef OFFSETOF

    // Restore modified state
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);

    checkGlError( "End of ImGuiManager::RenderDrawLists()" );
#endif
}
