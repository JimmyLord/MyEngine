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

GameMainFrame* g_pGameMainFrame = 0;

GameMainFrame::GameMainFrame()
: MainFrame(0)
{
    g_pGameMainFrame = this;

    m_pGLCanvasEditor = 0;
    m_CurrentSceneName[0] = 0;

    m_File->Insert( 0, myIDGame_LoadScene, wxT("&Load Scene") );
    m_File->Insert( 1, myIDGame_SaveScene, wxT("&Save Scene") );
    m_File->Insert( 2, myIDGame_SaveScene, wxT("Save Scene &As") );

    m_Data = MyNew wxMenu;
    m_MenuBar->Append( m_Data, wxT("&Data") );
    m_Data->Append( myIDGame_AddDatafile, wxT("&Load Datafiles") );

    Connect( myIDGame_LoadScene,   wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
    Connect( myIDGame_SaveScene,   wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
    Connect( myIDGame_SaveSceneAs, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );

    Connect( myIDGame_AddDatafile, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(GameMainFrame::OnGameMenu) );
}

void GameMainFrame::AddPanes()
{
    MainFrame::AddPanes();

    // create the editor opengl canvas
    int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
    m_pGLCanvasEditor = MyNew MainGLCanvas( (wxFrame*)this, args, 1, false );
    m_pGLCanvasEditor->SetSize( 600, 600 );

    m_AUIManager.AddPane( m_pGLCanvasEditor, wxAuiPaneInfo().Name("GLCanvasEditor").Bottom().Caption("GLCanvasEditor") );//.CaptionVisible(false) );
}

void GameMainFrame::OnPostInit()
{
    FILE* file = 0;
    fopen_s( &file, "EditorPrefs.ini", "rb" );
    if( file )
    {
        char* string = MyNew char[10000];
        int len = fread( string, 1, 10000, file );
        string[len] = 0;
        fclose( file );

        cJSON* pPrefs = cJSON_Parse( string );

        cJSON* pLastScene = cJSON_GetObjectItem( pPrefs, "LastSceneLoaded" );

        LoadScene( pLastScene->valuestring );

        delete[] string;
    }
}

void GameMainFrame::OnClose()
{
    FILE* file = 0;
    fopen_s( &file, "EditorPrefs.ini", "wb" );
    if( file )
    {
        cJSON* pPrefs = cJSON_CreateObject();

        cJSON_AddStringToObject( pPrefs, "LastSceneLoaded", m_CurrentSceneName );

        char* string = cJSON_Print( pPrefs );

        fprintf( file, string );
        fclose( file );

        free( string );
    }
}

void GameMainFrame::OnGameMenu(wxCommandEvent& event)
{
    int id = event.GetId();

    switch( id )
    {
    case myIDGame_LoadScene:
        LoadSceneDialog();
        ResizeViewport();
        break;

    case myIDGame_SaveScene:
        SaveScene();
        break;

    case myIDGame_SaveSceneAs:
        SaveSceneAs();
        break;

    case myIDGame_AddDatafile:
        AddDatafileToScene();
        break;
    }
}

void GameMainFrame::ResizeViewport()
{
    MainFrame::ResizeViewport();

    m_pGLCanvasEditor->ResizeViewport();
}

void GameMainFrame::SaveScene()
{
    if( m_CurrentSceneName[0] == 0 )
    {
        SaveSceneAs();
    }
    else
    {
        ((GameEntityComponentTest*)g_pGameCore)->SaveScene( m_CurrentSceneName );
    }
}

void GameMainFrame::SaveSceneAs()
{
    wxFileDialog FileDialog( this, _("Save Scene file"), "", "", "Scene files (*.scene)|*.scene", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if( FileDialog.ShowModal() == wxID_CANCEL )
        return;
    
    // save the current scene
    // TODO: having issues with my new/delete overrides, so doing some manual labour, will fail on multibyte names
    wxString wxpath = FileDialog.GetPath();
    size_t len = wxpath.length();
    for( unsigned int i=0; i<len; i++ )
        m_CurrentSceneName[i] = wxpath[i];
    m_CurrentSceneName[len] = 0;

    ((GameEntityComponentTest*)g_pGameCore)->SaveScene( m_CurrentSceneName );
}

void GameMainFrame::LoadSceneDialog()
{
    //if( scene is dirty )
    //{
    //    if (wxMessageBox(_("Current content has not been saved! Proceed?"), _("Please confirm"),
    //        wxICON_QUESTION | wxYES_NO, this) == wxNO )
    //    return;
    //}

    wxFileDialog FileDialog( this, _("Open Scene"), "./Data/Scenes", "", "Scene files (*.scene)|*.scene", wxFD_OPEN|wxFD_FILE_MUST_EXIST );
    
    if( FileDialog.ShowModal() == wxID_CANCEL )
        return;

    // load the file chosen by the user
    // TODO: having issues with my new/delete overrides, so doing some manual labour, will fail on multibyte names
    wxString wxpath = FileDialog.GetPath();
    size_t len = wxpath.length();
    for( unsigned int i=0; i<len; i++ )
        m_CurrentSceneName[i] = wxpath[i];
    m_CurrentSceneName[len] = 0;

    ((GameEntityComponentTest*)g_pGameCore)->LoadScene( m_CurrentSceneName, 1 );
}

void GameMainFrame::LoadScene(const char* scenename)
{
    assert( scenename != 0 );

    strcpy( m_CurrentSceneName, scenename );

    ((GameEntityComponentTest*)g_pGameCore)->LoadScene( m_CurrentSceneName, 1 );
}

void GameMainFrame::AddDatafileToScene()
{
    // multiple select file open dialog
    wxFileDialog FileDialog( this, _("Open Datafile"), "./Data", "", "All files(*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE );
    
    if( FileDialog.ShowModal() == wxID_CANCEL )
        return;
    
    // load the files chosen by the user
    // TODO: having issues with my new/delete overrides, so doing some manual labour, will fail on multibyte names
    wxArrayString patharray;
    FileDialog.GetPaths( patharray );

    char fullpath[MAX_PATH];
    for( unsigned int filenum=0; filenum<patharray.Count(); filenum++ )
    {
        size_t fullpathlen = patharray[filenum].length();
        for( unsigned int i=0; i<fullpathlen; i++ )
            fullpath[i] = patharray[filenum][i];
        fullpath[fullpathlen] = 0;

        char dirpath[MAX_PATH];
        GetCurrentDirectoryA( MAX_PATH, dirpath );

        // if the datafile is in our working directory, then load it... otherwise TODO: copy it in?
        size_t dirpathlen = strlen(dirpath);
        if( strncmp( dirpath, fullpath, dirpathlen ) == 0 )
        {
            for( unsigned int i=dirpathlen; i<fullpathlen-1; i++ )
            {
                fullpath[i-dirpathlen] = fullpath[i+1];
                if( fullpath[i-dirpathlen] == '\\' )
                    fullpath[i-dirpathlen] = '/';
                fullpath[i-dirpathlen+1] = 0;
            }
        }
        else
        {
            // File is not in our working directory.
            // TODO: copy the file into our data folder?
            assert( false );
        }

        ((GameEntityComponentTest*)g_pGameCore)->m_pComponentSystemManager->LoadDatafile( fullpath, 1 );
    }
}
