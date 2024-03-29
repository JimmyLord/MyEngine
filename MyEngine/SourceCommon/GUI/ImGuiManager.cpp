//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"
#include "../../SourceCommon/GUI/EditorIcons.h"

#include <gl/GL.h>
#include "../../../Framework/MyFramework/SourceWindows/OpenGL/GLExtensions.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/OpenGL/GLHelpers.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/OpenGL/Texture_OpenGL.h"

bool ImGuiManager::m_DeviceObjectsAreValid = false;
int ImGuiManager::m_ShaderHandle = 0;
int ImGuiManager::m_VertHandle = 0;
int ImGuiManager::m_FragHandle = 0;
int ImGuiManager::m_UniformLocationTex = -1;
int ImGuiManager::m_UniformLocationProjMtx = -1;
int ImGuiManager::m_AttribLocationPosition = -1;
int ImGuiManager::m_AttribLocationUV = -1;
int ImGuiManager::m_AttribLocationColor = -1;
unsigned int ImGuiManager::m_VboHandle = 0;
unsigned int ImGuiManager::m_VaoHandle = 0;
unsigned int ImGuiManager::m_ElementsHandle = 0;
TextureDefinition* ImGuiManager::m_pFontTexture = nullptr;

ImGuiManager::ImGuiManager()
{
    m_pImGuiContext = 0;

    m_FrameStarted = false;

    m_LastMouseCursor = ImGuiMouseCursor_COUNT;
}

ImGuiManager::~ImGuiManager()
{
    ImGui::SetCurrentContext( m_pImGuiContext );

    Shutdown();
}

void ImGuiManager::Init(float width, float height)
{
    MyAssert( m_pImGuiContext == 0 );

    m_pImGuiContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    //ImGui::StyleColorsClassic();

    if( m_DeviceObjectsAreValid == false )
    {
        // Created a new context, device objects have never been built.
        CreateDeviceObjects();
    }
    else
    {
        // Created a new context with a minimal attempt to preserve existing device objects.

        // Rebuild ImGui's internal font, but use the original GL texture object.
        CreateFont();

        io.Fonts->TexID = (void*)m_pFontTexture;
    }

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_DockingEnable;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.DisplaySize.x = width;
    io.DisplaySize.y = height;
    //io.IniFilename = "imgui.ini";
    
    // Keyboard mapping.  ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_Tab]         = MYKEYCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]   = MYKEYCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow]  = MYKEYCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]     = MYKEYCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow]   = MYKEYCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp]      = MYKEYCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown]    = MYKEYCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home]        = MYKEYCODE_HOME;
    io.KeyMap[ImGuiKey_End]         = MYKEYCODE_END;
    //io.KeyMap[ImGuiKey_Insert]      = MYKEYCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete]      = MYKEYCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace]   = MYKEYCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter]       = MYKEYCODE_ENTER;
    io.KeyMap[ImGuiKey_Escape]      = MYKEYCODE_ESC;
    io.KeyMap[ImGuiKey_Space]       = ' ';
    io.KeyMap[ImGuiKey_A]           = 'A';
    io.KeyMap[ImGuiKey_C]           = 'C';
    io.KeyMap[ImGuiKey_V]           = 'V';
    io.KeyMap[ImGuiKey_X]           = 'X';
    io.KeyMap[ImGuiKey_Y]           = 'Y';
    io.KeyMap[ImGuiKey_Z]           = 'Z';
}

void ImGuiManager::Shutdown(bool invalidateDeviceObjects)
{
    ImGui::SetCurrentContext( m_pImGuiContext );

    if( invalidateDeviceObjects )
    {
        InvalidateDeviceObjects();
        m_DeviceObjectsAreValid = false;
    }

    ImGui::DestroyContext( m_pImGuiContext );
    m_pImGuiContext = 0;
    m_FrameStarted = false;
}

void ImGuiManager::ActivateContext()
{
    ImGui::SetCurrentContext( m_pImGuiContext );
}

void ImGuiManager::ClearInput()
{
    ImGui::SetCurrentContext( m_pImGuiContext );

    ImGuiIO& io = ImGui::GetIO();

    for( int i=0; i<5; i++ )
    {
        io.MouseDown[i] = false;
    }

    for( int i=0; i<512; i++ )
    {
        io.KeysDown[i] = false;
    }

    io.MouseWheel = 0;

    io.KeyCtrl = false;
    io.KeyShift = false;
    io.KeyAlt = false;
    io.KeySuper = false;
}

void ImGuiManager::OnFocusLost()
{
    ImGui::SetCurrentContext( m_pImGuiContext );

    if( m_pImGuiContext )
    {
        ClearInput();
    }
}

bool ImGuiManager::HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure)
{
    ImGui::SetCurrentContext( m_pImGuiContext );

#if MYFW_OPENGLES2
    return false;
#endif

    ImGuiIO& io = ImGui::GetIO();

    //LOGInfo( "ImGui", "HandleEditorInput()\n" );

    if( mouseaction != -1 )
    {
        MyAssert( keyaction == -1 && keycode == -1 );

        // If giving relative movement to the game/editor, then let ImGui think it has the original position.
        if( mouseaction != GCBA_RelativeMovement )
        {
            io.MousePos.x = x;
            io.MousePos.y = y;
        }

        if( mouseaction == GCBA_Down )
            io.MouseDown[id] = true;
        if( mouseaction == GCBA_Up )
            io.MouseDown[id] = false;

        io.MouseWheel += pressure;
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

#if MYFW_USING_IMGUI
    // In ImGui editor mode, we ignore the return value, all inputs will go to EditorMainFrame_ImGui
    return true;
#endif

    // If a window is hovered, don't let mouse events pass through it.
    if( mouseaction != -1 && ImGui::IsWindowHovered( ImGuiHoveredFlags_AnyWindow ) )
    {
        return true;
    }

    // TODO: ignore key messages if we're entering text in an edit box.

    return false;
}

void ImGuiManager::OnChar(unsigned int c)
{
    ImGui::SetCurrentContext( m_pImGuiContext );

    ImGuiIO& io = ImGui::GetIO();

    if( c > 0 && c < 0x10000 )
    {
        io.AddInputCharacter( (unsigned short)c );
    }
}

void ImGuiManager::StartTick(float deltaTime)
{
    ImGui::SetCurrentContext( m_pImGuiContext );

#if MYFW_OPENGLES2
    return;
#endif

    //LOGInfo( "ImGui", "StartFrame()\n" );

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = deltaTime;
}

void ImGuiManager::StartFrame()
{
    if( m_FrameStarted == true )
        return;

#if MYFW_OPENGLES2
    return;
#endif

    ImGui::SetCurrentContext( m_pImGuiContext );

    m_FrameStarted = true;

    // Update OS mouse cursor with the cursor requested by imgui.
    UpdateMouseCursor();

    ImGuiIO& io = ImGui::GetIO();
    io.KeyCtrl = io.KeysDown[MYKEYCODE_LCTRL] || io.KeysDown[MYKEYCODE_RCTRL];
    io.KeyShift = io.KeysDown[MYKEYCODE_LSHIFT] || io.KeysDown[MYKEYCODE_RSHIFT];
    io.KeyAlt = io.KeysDown[MYKEYCODE_LALT] || io.KeysDown[MYKEYCODE_RALT];
    io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];

    ImGui::NewFrame();
    //ImGui::ShowTestWindow();
}

void ImGuiManager::EndFrame(float width, float height, bool draw)
{
    ImGui::SetCurrentContext( m_pImGuiContext );

#if MYFW_OPENGLES2
    return;
#endif

    //LOGInfo( "ImGui", "EndFrame()\n" );

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = width;
    io.DisplaySize.y = height;

    ImGui::Render();

    m_FrameStarted = false;
    ImDrawData* data = ImGui::GetDrawData();
    RenderDrawLists( data );
}

// This is the main rendering function.
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGuiManager::RenderDrawLists(ImDrawData* draw_data)
{
    ImGui::SetCurrentContext( m_pImGuiContext );

#if MYFW_OPENGLES2
    return;
#endif

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates).
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if( fb_width == 0 || fb_height == 0 )
        return;

    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Backup GL state.
    GLenum last_active_texture; glGetIntegerv( GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture );
    glActiveTexture( GL_TEXTURE0 );
    GLint last_program; glGetIntegerv( GL_CURRENT_PROGRAM, &last_program );
    GLint last_texture; glGetIntegerv( GL_TEXTURE_BINDING_2D, &last_texture );
    //GLint last_sampler; glGetIntegerv( GL_SAMPLER_BINDING, &last_sampler );
    GLint last_array_buffer; glGetIntegerv( GL_ARRAY_BUFFER_BINDING, &last_array_buffer );
    GLint last_element_array_buffer; glGetIntegerv( GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer );
#if !MYFW_OPENGLES2
    GLint last_vertex_array; glGetIntegerv( GL_VERTEX_ARRAY_BINDING, &last_vertex_array );
    GLint last_polygon_mode[2]; glGetIntegerv( GL_POLYGON_MODE, last_polygon_mode );
#endif
    GLint last_viewport[4]; glGetIntegerv( GL_VIEWPORT, last_viewport );
    GLint last_scissor_box[4]; glGetIntegerv( GL_SCISSOR_BOX, last_scissor_box );
    GLenum last_blend_src_rgb; glGetIntegerv( GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb );
    GLenum last_blend_dst_rgb; glGetIntegerv( GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb );
    GLenum last_blend_src_alpha; glGetIntegerv( GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha );
    GLenum last_blend_dst_alpha; glGetIntegerv( GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha );
    GLenum last_blend_equation_rgb; glGetIntegerv( GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb );
    GLenum last_blend_equation_alpha; glGetIntegerv( GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha );
    GLboolean last_enable_blend = glIsEnabled( GL_BLEND );
    GLboolean last_enable_cull_face = glIsEnabled( GL_CULL_FACE );
    GLboolean last_enable_depth_test = glIsEnabled( GL_DEPTH_TEST );
    GLboolean last_enable_scissor_test = glIsEnabled( GL_SCISSOR_TEST );

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill.
    glEnable( GL_BLEND );
    glBlendEquation( GL_FUNC_ADD );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_SCISSOR_TEST );
#if !MYFW_OPENGLES2
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
#endif

    // Setup viewport, orthographic projection matrix.
    glViewport( 0, 0, (GLsizei)fb_width, (GLsizei)fb_height );
    const float ortho_projection[4][4] =
    {
        { 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
        { 0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f },
        { 0.0f,                  0.0f,                  -1.0f, 0.0f },
        {-1.0f,                  1.0f,                   0.0f, 1.0f },
    };
    glUseProgram( m_ShaderHandle );
    glUniform1i( m_UniformLocationTex, 0 );
    glUniformMatrix4fv( m_UniformLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0] );
    glBindVertexArray( m_VaoHandle );
    //glBindSampler( 0, 0 ); // Rely on combined texture/sampler state.

    for( int n = 0; n < draw_data->CmdListsCount; n++ )
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBindBuffer( GL_ARRAY_BUFFER, m_VboHandle );
        glBufferData( GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW );

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_ElementsHandle );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW );

        for( int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++ )
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if( pcmd->UserCallback )
            {
                pcmd->UserCallback( cmd_list, pcmd );
            }
            else
            {
                Texture_OpenGL* pGLTexture = (Texture_OpenGL*)pcmd->TextureId;
                GLuint texture = 0;
                if( pGLTexture )
                {
                    texture = pGLTexture->GetTextureID();
                }

                glBindTexture( GL_TEXTURE_2D, texture );
                glScissor( (int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y) );
                glDrawElements( GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset );
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }

    // Restore modified GL state.
    glUseProgram( last_program );
    glBindTexture( GL_TEXTURE_2D, last_texture );
    //glBindSampler( 0, last_sampler );
    glActiveTexture( last_active_texture );
#if !MYFW_OPENGLES2
    glBindVertexArray( last_vertex_array );
#endif
    glBindBuffer( GL_ARRAY_BUFFER, last_array_buffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer );
    glBlendEquationSeparate( last_blend_equation_rgb, last_blend_equation_alpha );
    glBlendFuncSeparate( last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha );
    if( last_enable_blend ) glEnable( GL_BLEND ); else glDisable( GL_BLEND );
    if( last_enable_cull_face ) glEnable( GL_CULL_FACE ); else glDisable( GL_CULL_FACE );
    if( last_enable_depth_test ) glEnable( GL_DEPTH_TEST ); else glDisable( GL_DEPTH_TEST );
    if( last_enable_scissor_test ) glEnable( GL_SCISSOR_TEST ); else glDisable( GL_SCISSOR_TEST );
#if !MYFW_OPENGLES2
    glPolygonMode( GL_FRONT_AND_BACK, last_polygon_mode[0] );
#endif
    glViewport( last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3] );
    glScissor( last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3] );
}

void ImGuiManager::CreateFont() // Static.
{
    // TODO: Fix this to better support multiple ImGuiManager instances.
    //ImGui::SetCurrentContext( m_pImGuiContext );

    ImGuiIO& io = ImGui::GetIO();

    ImFont* pFont = io.Fonts->AddFontDefault();

    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
    static const ImWchar icon_ranges[] = { EditorIconData_First, EditorIconData_Last, 0 };
#if MYFW_EDITOR
    if( FileManager::DoesFileExist( EditorIconData_Filename ) )
    {
        io.Fonts->AddFontFromFileTTF( EditorIconData_Filename, 13.0f, &config, icon_ranges );
    }
#endif //MYFW_EDITOR
    io.Fonts->Build();
}

bool ImGuiManager::CreateFontAndTexture() // Static.
{
    // TODO: Fix this to better support multiple ImGuiManager instances.
    //ImGui::SetCurrentContext( m_pImGuiContext );

    CreateFont();

    // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;

    io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height );

    MyAssert( m_pFontTexture == nullptr );

    GLint last_texture;
    glGetIntegerv( GL_TEXTURE_BINDING_2D, &last_texture );

    // Upload texture to graphics system.
    m_pFontTexture = MyNew Texture_OpenGL();
    Texture_OpenGL* pGLTexture = (Texture_OpenGL*)m_pFontTexture;
    pGLTexture->GenerateTexture( pixels, width, height );

    // Set texture filter.
    glBindTexture( GL_TEXTURE_2D, pGLTexture->GetTextureID() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // Store our identifier.
    io.Fonts->TexID = (void*)m_pFontTexture;

    // Restore state.
    glBindTexture( GL_TEXTURE_2D, last_texture );

    return true;
}

bool ImGuiManager::CreateDeviceObjects() // Static.
{
#if MYFW_OPENGLES2
    return false;
#endif

    // Backup GL state.
    GLint last_texture, last_array_buffer, last_vertex_array;
    glGetIntegerv( GL_TEXTURE_BINDING_2D, &last_texture );
    glGetIntegerv( GL_ARRAY_BUFFER_BINDING, &last_array_buffer );
#if !MYFW_OPENGLES2
    glGetIntegerv( GL_VERTEX_ARRAY_BINDING, &last_vertex_array );
#endif

    const GLchar *vertex_shader =
        "#version 150\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "	Frag_UV = UV;\n"
        "	Frag_Color = Color;\n"
        "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar* fragment_shader =
        "#version 150\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
        "}\n";

    m_ShaderHandle = glCreateProgram();
    m_VertHandle = glCreateShader( GL_VERTEX_SHADER );
    m_FragHandle = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( m_VertHandle, 1, &vertex_shader, 0 );
    glShaderSource( m_FragHandle, 1, &fragment_shader, 0 );
    glCompileShader( m_VertHandle );
    glCompileShader( m_FragHandle );
    glAttachShader( m_ShaderHandle, m_VertHandle );
    glAttachShader( m_ShaderHandle, m_FragHandle );
    glLinkProgram( m_ShaderHandle );

    m_UniformLocationTex = glGetUniformLocation( m_ShaderHandle, "Texture" );
    m_UniformLocationProjMtx = glGetUniformLocation( m_ShaderHandle, "ProjMtx" );
    m_AttribLocationPosition = glGetAttribLocation( m_ShaderHandle, "Position" );
    m_AttribLocationUV = glGetAttribLocation( m_ShaderHandle, "UV" );
    m_AttribLocationColor = glGetAttribLocation( m_ShaderHandle, "Color" );

    glGenBuffers( 1, &m_VboHandle );
    glGenBuffers( 1, &m_ElementsHandle );

    glGenVertexArrays( 1, &m_VaoHandle );
    glBindVertexArray( m_VaoHandle );
    glBindBuffer( GL_ARRAY_BUFFER, m_VboHandle );
    glEnableVertexAttribArray( m_AttribLocationPosition );
    glEnableVertexAttribArray( m_AttribLocationUV );
    glEnableVertexAttribArray( m_AttribLocationColor );

    glVertexAttribPointer( m_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos) );
    glVertexAttribPointer( m_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv) );
    glVertexAttribPointer( m_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col) );

    CreateFontAndTexture();

    // Restore modified GL state.
    glBindTexture( GL_TEXTURE_2D, last_texture );
    glBindBuffer( GL_ARRAY_BUFFER, last_array_buffer );
    glBindVertexArray( last_vertex_array );

    m_DeviceObjectsAreValid = true;

    return true;
}

void ImGuiManager::InvalidateDeviceObjects() // Static.
{
    if( m_VaoHandle ) glDeleteVertexArrays( 1, &m_VaoHandle );
    if( m_VboHandle ) glDeleteBuffers( 1, &m_VboHandle );
    if( m_ElementsHandle ) glDeleteBuffers( 1, &m_ElementsHandle );

    if( m_ShaderHandle && m_VertHandle ) glDetachShader( m_ShaderHandle, m_VertHandle );
    if( m_VertHandle ) glDeleteShader( m_VertHandle );

    if( m_ShaderHandle && m_FragHandle ) glDetachShader( m_ShaderHandle, m_FragHandle );
    if( m_FragHandle ) glDeleteShader( m_FragHandle );

    if( m_ShaderHandle ) glDeleteProgram( m_ShaderHandle );

    if( m_pFontTexture ) m_pFontTexture->Release();
    ImGui::GetIO().Fonts->TexID = 0;

    m_ShaderHandle = 0;
    m_VertHandle = 0;
    m_FragHandle = 0;
    m_UniformLocationTex = -1;
    m_UniformLocationProjMtx = -1;
    m_AttribLocationPosition = -1;
    m_AttribLocationUV = -1;
    m_AttribLocationColor = -1;
    m_VboHandle = 0;
    m_VaoHandle = 0;
    m_ElementsHandle = 0;
    m_pFontTexture = nullptr;
}

bool ImGuiManager::UpdateMouseCursor()
{
    ImGui::SetCurrentContext( m_pImGuiContext );

    ImGuiIO& io = ImGui::GetIO();
    if( io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange )
        return false;

    ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if( m_LastMouseCursor != mouse_cursor )
    {
        m_LastMouseCursor = mouse_cursor;
    }

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if( imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor )
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor.
        ::SetCursor( 0 );
    }
    else
    {
        // If imgui just wants an arrow cursor, don't change the cursor.
        // This allows regular windows cursor changes to occur for resizing, etc.
        if( imgui_cursor == ImGuiMouseCursor_Arrow )
            return true;

        // Show OS mouse cursor.
        LPTSTR win32_cursor = IDC_ARROW;
        switch( imgui_cursor )
        {
        case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW;    break;
        case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM;    break;
        case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL;  break;
        case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE;   break;
        case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS;   break;
        case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
        case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
        case ImGuiMouseCursor_Hand:         win32_cursor = IDC_HAND;     break;
        }
        ::SetCursor( ::LoadCursor( 0, win32_cursor ) );
    }

    return true;
}
