//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorDocumentManager.h"

#include "EditorDocument.h"
#include "Core/EngineCore.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/Documents/EditorDocument_Heightmap.h"
#include "../SourceEditor/NodeGraph/MyNodeGraph.h"
//#include "../SourceEditor/NodeGraph/VisualScriptNodes.h"
#include "../SourceEditor/NodeGraph/VisualScriptNodeTypeManager.h"
#include "../SourceEditor/PlatformSpecific/FileOpenDialog.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"

static VisualScriptNodeTypeManager g_VisualScriptNodeTypeManager;

EditorDocumentManager::EditorDocumentManager(EngineCore* pEngineCore)
{
    m_pEngineCore = pEngineCore;
}

EditorDocumentManager::~EditorDocumentManager()
{
}

EditorDocument* EditorDocumentManager::AddDocumentMenu(EngineCore* pEngineCore, EditorDocument* pDocument)
{
    EditorDocument* pNewDocument = nullptr;

    if( ImGui::BeginMenu( "Document" ) )
    {
        if( ImGui::BeginMenu( "New Document" ) )
        {
            if( ImGui::MenuItem( "Visual Script" ) )
            {
                pNewDocument = MyNew MyNodeGraph( pEngineCore, &g_VisualScriptNodeTypeManager );
            }
            if( ImGui::MenuItem( "Heightmap" ) )
            {
                pNewDocument = MyNew EditorDocument_Heightmap( pEngineCore, nullptr );
            }
            ImGui::EndMenu(); // "New Document"
        }

        if( ImGui::MenuItem( "Load Document..." ) )
        {
            pNewDocument = LoadDocument( pEngineCore );
            //pNewDocument = pDocument->EditorDocumentMenuCommand( EditorDocument::EditorDocumentMenuCommand_Load );
        }

        if( ImGui::BeginMenu( "Load Recent Document" ) )
        {
            uint32 numRecentDocuments = pEngineCore->GetEditorPrefs()->Get_Document_NumRecentDocuments();
            if( numRecentDocuments == 0 )
            {
                ImGui::Text( "no recent documents." );
            }

            for( uint32 i=0; i<numRecentDocuments; i++ )
            {
                std::string relativePathStr = pEngineCore->GetEditorPrefs()->Get_Document_RecentDocument( i );
                const char* relativePath = relativePathStr.c_str();

                if( ImGui::MenuItem( relativePath ) )
                {
                    uint32 len = (uint32)strlen( relativePath );

                    if( strcmp( &relativePath[len-strlen(".myvisualscript")], ".myvisualscript" ) == 0 )
                    {
                        pNewDocument = MyNew MyNodeGraph( pEngineCore, &g_VisualScriptNodeTypeManager );
                    }

                    if( strcmp( &relativePath[len-strlen(".myheightmap")], ".myheightmap" ) == 0 )
                    {
                        pNewDocument = MyNew EditorDocument_Heightmap( pEngineCore, nullptr );
                    }

                    if( pNewDocument )
                    {
                        pNewDocument->SetRelativePath( relativePath );
                        pNewDocument->Load();

                        pEngineCore->GetEditorPrefs()->AddRecentDocument( relativePath );
                    }
                    else
                    {
                        LOGError( LOGTag, "Document not found: %s - removing from recent list.", relativePath );
                        pEngineCore->GetEditorPrefs()->RemoveRecentDocument( relativePath );
                        i--;
                        numRecentDocuments--;
                    }
                }

                if( ImGui::BeginPopupContextItem() )
                {
                    if( ImGui::MenuItem( "Remove" ) )
                    {
                        pEngineCore->GetEditorPrefs()->RemoveRecentDocument( relativePath );
                        i--;
                        numRecentDocuments--;
                    }
                    ImGui::EndPopup();
                }
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        // Save.
        {
            char tempstr[MAX_PATH + 10];
            if( pDocument )
            {
                if( pDocument->GetFilename()[0] == '\0' )
                {
                    sprintf_s( tempstr, MAX_PATH + 10, "Save Untitled as..." );
                }
                else
                {
                    sprintf_s( tempstr, MAX_PATH + 10, "Save %s", pDocument->GetFilename() );
                }
            }
            else
            {
                sprintf_s( tempstr, MAX_PATH + 10, "Save" );
            }

            if( ImGui::MenuItem( tempstr, "Ctrl-S", false, pDocument != nullptr ) )
            {
                pDocument->EditorDocumentMenuCommand( EditorDocument::EditorDocumentMenuCommand_Save );
            }
        }

        if( ImGui::MenuItem( "Save As...", nullptr, false, pDocument != nullptr ) )
        {
            pDocument->EditorDocumentMenuCommand( EditorDocument::EditorDocumentMenuCommand_SaveAs );
        }

        //if( ImGui::MenuItem( "Save All", "Ctrl-Shift-S", false, pDocument != nullptr ) )
        //{
        //    pDocument->EditorDocumentMenuCommand( EditorDocument::EditorDocumentMenuCommand_SaveAll );
        //}

        //if( ImGui::MenuItem( "Run", "F5", false, pDocument != nullptr ) )
        //{
        //    pDocument->EditorDocumentMenuCommand( EditorDocument::EditorDocumentMenuCommand_Run );
        //}

        ImGui::EndMenu(); // "Document"
    }

    return pNewDocument;
}

EditorDocument* EditorDocumentManager::LoadDocument(EngineCore* pEngineCore)
{
    char tempFilter[256];
    //sprintf_s( tempFilter, 256, "%s\0All\0*.*\0", GetDefaultFileSaveFilter() );
    sprintf_s( tempFilter, 256, "%s=%s=%s=",
        "All=*.*",
        "VisualScript Files=*.myvisualscript",
        "MyHeightmap Files=*.myheightmap"
    );
    uint32 tempFilterLen = (uint32)strlen( tempFilter );
    for( uint32 i=0; i<tempFilterLen; i++ )
    {
        if( tempFilter[i] == '=' )
        {
            tempFilter[i] = '\0';
        }
    }

    const char* filename = FileOpenDialog( "Data\\", tempFilter );
    if( filename[0] != '\0' )
    {
        char path[MAX_PATH];
        strcpy_s( path, MAX_PATH, filename );
        const char* relativePath = ::GetRelativePath( path );

        int len = (int)strlen( relativePath );
        EditorDocument* pNewDocument = nullptr;
        
        if( strcmp( &relativePath[len-strlen(".myvisualscript")], ".myvisualscript" ) == 0 )
        {
            pNewDocument = MyNew MyNodeGraph( pEngineCore, &g_VisualScriptNodeTypeManager );
        }        
        else if( strcmp( &relativePath[len-strlen(".myheightmap")], ".myheightmap" ) == 0 )
        {
            pNewDocument = MyNew EditorDocument_Heightmap( pEngineCore, nullptr );
        }

        if( pNewDocument == nullptr )
        {
            LOGError( LOGTag, "This filetype is not supported.\n" );
            return nullptr;
        }

        pNewDocument->SetRelativePath( relativePath );
        pNewDocument->Load();

        pEngineCore->GetEditorPrefs()->AddRecentDocument( relativePath );

        return pNewDocument;
    }

    return nullptr;
}

void EditorDocumentManager::RestorePreviouslyOpenDocuments(EngineCore* pEngineCore)
{
    cJSON* jEditorPrefs = pEngineCore->GetEditorPrefs()->GetEditorPrefsJSONString();
    cJSON* jOpenDocumentsArray = cJSON_GetObjectItem( jEditorPrefs, "State_OpenDocuments" );
    if( jOpenDocumentsArray )
    {
        for( int i=0; i<cJSON_GetArraySize( jOpenDocumentsArray ); i++ )
        {
            cJSON* jDocument = cJSON_GetArrayItem( jOpenDocumentsArray, i );
            char* relativePath = jDocument->valuestring;
            int len = (int)strlen( relativePath );

            EditorDocument* pNewDocument = nullptr;

            if( strcmp( &relativePath[len-strlen(".myvisualscript")], ".myvisualscript" ) == 0 )
            {
                pNewDocument = MyNew MyNodeGraph( pEngineCore, &g_VisualScriptNodeTypeManager );
            }        
            else if( strcmp( &relativePath[len-strlen(".myheightmap")], ".myheightmap" ) == 0 )
            {
                pNewDocument = MyNew EditorDocument_Heightmap( pEngineCore, nullptr );
            }

            if( pNewDocument != nullptr )
            {
                pNewDocument->SetRelativePath( relativePath );
                pNewDocument->Load();
                pEngineCore->GetEditorState()->OpenDocument( pNewDocument );
            }
        }
    }
}
