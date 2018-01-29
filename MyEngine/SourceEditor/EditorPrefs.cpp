//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

EditorPrefs* g_pEditorPrefs = 0;

EditorPrefs::EditorPrefs()
{
    g_pEditorPrefs = this;

    m_pSaveFile = 0;

    m_jEditorPrefs = 0;

    // Set default values for prefs.
    m_WindowX = 0;
    m_WindowY = 0;
    m_WindowWidth = 1280;
    m_WindowHeight = 600;
    m_IsWindowMaximized = false;
}

EditorPrefs::~EditorPrefs()
{
    MyAssert( m_pSaveFile == 0 );

    cJSON_Delete( m_jEditorPrefs );
}

void EditorPrefs::Init()
{
    FILE* file = 0;
#if MYFW_USING_WX
    const char* iniFilename = "wxEditorPrefs.ini";
#else
    const char* iniFilename = "EditorPrefs.ini";
#endif

#if MYFW_WINDOWS
    fopen_s( &file, iniFilename, "rb" );
#else
    file = fopen( iniFilename, "rb" );
#endif

    if( file )
    {
        char* string = MyNew char[10000];
        size_t len = fread( string, 1, 10000, file );
        string[len] = 0;
        fclose( file );

        m_jEditorPrefs = cJSON_Parse( string );
        delete[] string;
    }
}

void EditorPrefs::LoadWindowSizePrefs()
{
    if( m_jEditorPrefs == 0 )
        return;

#if MYFW_USING_IMGUI
    // Load in some prefs.
    cJSONExt_GetInt( m_jEditorPrefs, "WindowX", &m_WindowX );
    cJSONExt_GetInt( m_jEditorPrefs, "WindowY", &m_WindowY );
    cJSONExt_GetInt( m_jEditorPrefs, "WindowWidth", &m_WindowWidth );
    cJSONExt_GetInt( m_jEditorPrefs, "WindowHeight", &m_WindowHeight );
    cJSONExt_GetBool( m_jEditorPrefs, "IsMaximized", &m_IsWindowMaximized );

    // Resize window.
    SetWindowPos( g_hWnd, 0, m_WindowX, m_WindowY, m_WindowWidth, m_WindowHeight, 0 );
    if( m_IsWindowMaximized )
    {
        ShowWindow( g_hWnd, SW_MAXIMIZE );
    }
#endif //MYFW_USING_IMGUI
}

void EditorPrefs::LoadPrefs()
{
    if( m_jEditorPrefs == 0 )
        return;

    cJSON* jObject;

    jObject = cJSON_GetObjectItem( m_jEditorPrefs, "EditorCam" );
    if( jObject )
        g_pEngineCore->GetEditorState()->GetEditorCamera()->m_pComponentTransform->ImportFromJSONObject( jObject, EngineCore::ENGINE_SCENE_ID );
}

cJSON* EditorPrefs::SaveStart()
{
    MyAssert( m_pSaveFile == 0 );

#if MYFW_WINDOWS
    fopen_s( &m_pSaveFile, "EditorPrefs.ini", "wb" );
#else
    m_pSaveFile = fopen( "EditorPrefs.ini", "wb" );
#endif
    if( m_pSaveFile )
    {
        cJSON* jPrefs = cJSON_CreateObject();

#if MYFW_USING_IMGUI
        WINDOWINFO info;
        if( GetWindowInfo( g_hWnd, &info ) )
        {
            m_WindowX = info.rcWindow.left;
            m_WindowY = info.rcWindow.top;

            m_WindowWidth = info.rcWindow.right - info.rcWindow.left;
            m_WindowHeight = info.rcWindow.bottom - info.rcWindow.top;

            if( info.dwStyle & WS_MAXIMIZE )
                m_IsWindowMaximized = true;
            else
                m_IsWindowMaximized = false;
        }
#endif //MYFW_USING_IMGUI

        cJSON_AddNumberToObject( jPrefs, "WindowX", m_WindowX );
        cJSON_AddNumberToObject( jPrefs, "WindowY", m_WindowY );
        cJSON_AddNumberToObject( jPrefs, "WindowWidth", m_WindowWidth );
        cJSON_AddNumberToObject( jPrefs, "WindowHeight", m_WindowHeight );
        cJSON_AddNumberToObject( jPrefs, "IsMaximized", m_IsWindowMaximized );

        cJSON_AddItemToObject( jPrefs, "EditorCam", g_pEngineCore->GetEditorState()->GetEditorCamera()->m_pComponentTransform->ExportAsJSONObject( false, true ) );

        return jPrefs;
    }

    return 0;
}

void EditorPrefs::SaveFinish(cJSON* pPrefs)
{
    MyAssert( m_pSaveFile != 0 );
    MyAssert( pPrefs != 0 );

    char* string = cJSON_Print( pPrefs );
    cJSON_Delete( pPrefs );

    fprintf( m_pSaveFile, "%s", string );
    fclose( m_pSaveFile );
    m_pSaveFile = 0;

    cJSONExt_free( string );
}
