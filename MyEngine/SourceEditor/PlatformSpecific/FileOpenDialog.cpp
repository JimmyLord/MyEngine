//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include <Commdlg.h>

// To allow the dialog to open multiple files,
//     pass a pointer to a bool that will be set to true if multiple files were selected
//         or false if only 1 file was selected
char* FileOpenDialog(const char* initialDir, const char* filter, bool* openedMultipleFiles)
{
#if MYFW_WINDOWS
    static char fullpath[2048]; // Allow for multiple files to be opened at once.
    fullpath[0] = 0;

    OPENFILENAMEA ofn;
	ZeroMemory( &ofn, sizeof(ofn) );
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = 0;
	ofn.lpstrFile = fullpath;
	ofn.nMaxFile = 2048;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = 0;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if( openedMultipleFiles != 0 )
        ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
                
    GetOpenFileNameA( &ofn );

    if( openedMultipleFiles != 0 && ofn.nFileOffset != 0 )
    {
        // If multiple files were selected, fullpath will look like:
        //     "PathToFile0filename0filename0filename00"
        // If a single file was selected, fullpath will look like:
        //     "PathToFile\filename"

        if( fullpath[ofn.nFileOffset-1] == 0 )
        {
            *openedMultipleFiles = true;
        }
        else
        {
            *openedMultipleFiles = false;
        }
    }
#else
    LOGError( LOGTag, "TODO: Implement me!\n" );
#endif

    return fullpath;
}

char* FileSaveDialog(const char* initialDir, const char* filter)
{
#if MYFW_WINDOWS
    static char fullpath[MAX_PATH];
    fullpath[0] = 0;

    OPENFILENAMEA ofn;
	ZeroMemory( &ofn, sizeof(ofn) );
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = 0;
	ofn.lpstrFile = fullpath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = 0;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
                
    GetOpenFileNameA( &ofn );
#else
    LOGError( LOGTag, "TODO: Implement me!\n" );
#endif

    return fullpath;
}
