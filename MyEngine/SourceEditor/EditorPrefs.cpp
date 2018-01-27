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
}

EditorPrefs::~EditorPrefs()
{
    cJSON_Delete( m_jEditorPrefs );
}

void EditorPrefs::Init()
{
    FILE* file = 0;
#if MYFW_WINDOWS
    fopen_s( &file, "EditorPrefs.ini", "rb" );
#else
    file = fopen( "EditorPrefs.ini", "rb" );
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
