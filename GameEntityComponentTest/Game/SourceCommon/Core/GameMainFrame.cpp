//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

#include "../../Framework/MyFramework/SourceWindows/MYFWWinMainWx.h"
#include "GameMainFrame.h"

enum GameMenuIDs
{
    myIDGame_LoadScene = myID_NumIDs,
    myIDGame_SaveScene,
};

GameMainFrame::GameMainFrame()
: MainFrame(0)
{
    m_File->Insert( 0, myIDGame_LoadScene, wxT("&Load Scene") );
    m_File->Insert( 1, myIDGame_SaveScene, wxT("&Save Scene") );

    Connect( myIDGame_LoadScene, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
    Connect( myIDGame_SaveScene, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
}

void GameMainFrame::OnGameMenu(wxCommandEvent& event)
{
    int id = event.GetId();

    switch( id )
    {
    case myIDGame_SaveScene:
        g_pComponentSystemManager->SaveSceneToJSON();
        break;

    case myIDGame_LoadScene:
        {
            FILE* filehandle;
            errno_t err = fopen_s( &filehandle, "test.scene", "rb" );

            char* jsonstr;

            if( filehandle )
            {
                fseek( filehandle, 0, SEEK_END );
                long length = ftell( filehandle );
                if( length > 0 )
                {
                    fseek( filehandle, 0, SEEK_SET );

                    jsonstr = MyNew char[length+1];
                    fread( jsonstr, length, 1, filehandle );
                    jsonstr[length] = 0;
                }

                fclose( filehandle );

                g_pComponentSystemManager->LoadSceneFromJSON( jsonstr );

                delete[] jsonstr;
            }

            m_pGLCanvas->ResizeViewport();
        }
        break;
    }
}
