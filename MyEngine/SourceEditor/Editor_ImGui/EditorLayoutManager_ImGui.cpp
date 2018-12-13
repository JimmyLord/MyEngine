//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "ImGuiStylePrefs.h"

const char* g_DefaultEditorLayoutImGuiIniStrings[EditorLayout_NumLayouts] =
{
    "[Window][Main Dock]\nPos=0,0\nSize=1200,650\nCollapsed=0\n\n[Window][Debug##Default]\nPos=60,60\nSize=400,400\nCollapsed=0\n\n[Window][Game]\nPos=0,317\nSize=261,164\nCollapsed=0\nDockId=0x0000000A,0\n\n[Window][Editor]\nPos=263,19\nSize=656,462\nCollapsed=0\nDockId=0x00000008,0\n\n[Window][Objects]\nPos=0,19\nSize=261,296\nCollapsed=0\nDockId=0x00000009,0\n\n[Window][Watch]\nPos=921,19\nSize=279,438\nCollapsed=0\nDockId=0x00000005,0\n\n[Window][Log]\nPos=0,483\nSize=919,167\nCollapsed=0\nDockId=0x00000002,0\n\n[Window][Resources]\nPos=921,459\nSize=279,191\nCollapsed=0\nDockId=0x00000006,0\n\n[Window][Mouse Picker]\nPos=0,19\nSize=256,289\nCollapsed=0\nDockId=0x00000007,1\n\n[Window][Stuff]\nPos=0,19\nSize=256,289\nCollapsed=0\nDockId=0x00000007,2\n\n[Window][ImGui Demo]\nPos=0,483\nSize=919,167\nCollapsed=0\nDockId=0x00000002,1\n\n[Window][Deferred G-Buffer]\nPos=0,483\nSize=919,167\nCollapsed=0\nDockId=0x00000002,1\n\n[Window][Material Editor]\nPos=921,19\nSize=279,438\nCollapsed=0\nDockId=0x00000005,1\n\n[Window][2D Animation Editor]\nPos=263,19\nSize=656,462\nCollapsed=0\nDockId=0x00000008,1\n\n[Window][Grid Settings]\nPos=442,290\nSize=308,60\nCollapsed=0\n\n[Docking][Data]\nDockSpace         ID=0xD01BFD3D Pos=0,19 Size=1200,631 Split=X\n  DockNode        ID=0x00000003 Parent=0xD01BFD3D SizeRef=919,631 Split=Y\n    DockNode      ID=0x00000001 Parent=0x00000003 SizeRef=1200,462 Split=X\n      DockNode    ID=0x00000007 Parent=0x00000001 SizeRef=261,462 Split=Y SelectedTab=0x7DA7F56F\n        DockNode  ID=0x00000009 Parent=0x00000007 SizeRef=261,296 SelectedTab=0x7DA7F56F\n        DockNode  ID=0x0000000A Parent=0x00000007 SizeRef=261,164 SelectedTab=0x83199EB2\n      DockNode    ID=0x00000008 Parent=0x00000001 SizeRef=656,462 CentralNode=1 SelectedTab=0xCB5DF48C\n    DockNode      ID=0x00000002 Parent=0x00000003 SizeRef=1200,167 SelectedTab=0xB7722E25\n  DockNode        ID=0x00000004 Parent=0xD01BFD3D SizeRef=279,631 Split=Y SelectedTab=0x6D97690D\n    DockNode      ID=0x00000005 Parent=0x00000004 SizeRef=334,438 SelectedTab=0x91CA6522\n    DockNode      ID=0x00000006 Parent=0x00000004 SizeRef=334,191 SelectedTab=0x6D97690D\n\n",
    "[Window][Main Dock]\nPos=0,0\nSize=1200,650\nCollapsed=0\n\n[Window][Debug##Default]\nPos=60,60\nSize=400,400\nCollapsed=0\n\n[Window][Game]\nPos=263,19\nSize=656,462\nCollapsed=0\nDockId=0x00000008,0\n\n[Window][Editor]\nPos=0,324\nSize=261,157\nCollapsed=0\nDockId=0x0000000A,0\n\n[Window][Objects]\nPos=0,19\nSize=261,303\nCollapsed=0\nDockId=0x00000009,0\n\n[Window][Watch]\nPos=921,19\nSize=279,438\nCollapsed=0\nDockId=0x00000005,0\n\n[Window][Log]\nPos=0,483\nSize=919,167\nCollapsed=0\nDockId=0x00000002,0\n\n[Window][Resources]\nPos=921,459\nSize=279,191\nCollapsed=0\nDockId=0x00000006,0\n\n[Window][Mouse Picker]\nPos=0,19\nSize=256,289\nCollapsed=0\nDockId=0x00000007,1\n\n[Window][Stuff]\nPos=0,19\nSize=256,289\nCollapsed=0\nDockId=0x00000007,2\n\n[Window][ImGui Demo]\nPos=0,483\nSize=919,167\nCollapsed=0\nDockId=0x00000002,1\n\n[Window][Deferred G-Buffer]\nPos=0,483\nSize=919,167\nCollapsed=0\nDockId=0x00000002,1\n\n[Window][Material Editor]\nPos=921,19\nSize=279,438\nCollapsed=0\nDockId=0x00000005,1\n\n[Window][2D Animation Editor]\nPos=263,19\nSize=656,462\nCollapsed=0\nDockId=0x00000008,1\n\n[Window][Grid Settings]\nPos=442,290\nSize=308,60\nCollapsed=0\n\n[Docking][Data]\nDockSpace         ID=0xD01BFD3D Pos=0,19 Size=1200,631 Split=X\n  DockNode        ID=0x00000003 Parent=0xD01BFD3D SizeRef=919,631 Split=Y\n    DockNode      ID=0x00000001 Parent=0x00000003 SizeRef=1200,462 Split=X\n      DockNode    ID=0x00000007 Parent=0x00000001 SizeRef=261,462 Split=Y SelectedTab=0x7DA7F56F\n        DockNode  ID=0x00000009 Parent=0x00000007 SizeRef=261,303 SelectedTab=0x7DA7F56F\n        DockNode  ID=0x0000000A Parent=0x00000007 SizeRef=261,157 SelectedTab=0xCB5DF48C\n      DockNode    ID=0x00000008 Parent=0x00000001 SizeRef=656,462 CentralNode=1 SelectedTab=0x83199EB2\n    DockNode      ID=0x00000002 Parent=0x00000003 SizeRef=1200,167 SelectedTab=0xB7722E25\n  DockNode        ID=0x00000004 Parent=0xD01BFD3D SizeRef=279,631 Split=Y SelectedTab=0x6D97690D\n    DockNode      ID=0x00000005 Parent=0x00000004 SizeRef=334,438 SelectedTab=0x91CA6522\n    DockNode      ID=0x00000006 Parent=0x00000004 SizeRef=334,191 SelectedTab=0x6D97690D\n\n",
    "[Window][Main Dock]\nPos=0,0\nSize=1200,650\nCollapsed=0\n\n[Window][Debug##Default]\nPos=60,60\nSize=400,400\nCollapsed=0\n\n[Window][Game]\nPos=0,19\nSize=355,359\nCollapsed=0\nDockId=0x00000009,0\n\n[Window][Editor]\nPos=357,19\nSize=400,359\nCollapsed=0\nDockId=0x0000000A,0\n\n[Window][Objects]\nPos=759,19\nSize=225,438\nCollapsed=0\nDockId=0x00000007,0\n\n[Window][Watch]\nPos=986,19\nSize=214,438\nCollapsed=0\nDockId=0x00000008,0\n\n[Window][Log]\nPos=0,380\nSize=757,270\nCollapsed=0\nDockId=0x00000002,0\n\n[Window][Resources]\nPos=759,459\nSize=441,191\nCollapsed=0\nDockId=0x00000006,0\n\n[Window][Mouse Picker]\nPos=0,19\nSize=256,289\nCollapsed=0\nDockId=0x00000001,1\n\n[Window][Stuff]\nPos=0,19\nSize=256,289\nCollapsed=0\nDockId=0x00000001,2\n\n[Window][ImGui Demo]\nPos=0,483\nSize=919,167\nCollapsed=0\nDockId=0x00000002,1\n\n[Window][Deferred G-Buffer]\nPos=0,483\nSize=919,167\nCollapsed=0\nDockId=0x00000002,1\n\n[Window][Material Editor]\nPos=986,19\nSize=214,438\nCollapsed=0\nDockId=0x00000008,1\n\n[Window][2D Animation Editor]\nPos=0,19\nSize=355,359\nCollapsed=0\nDockId=0x00000009,1\n\n[Window][Grid Settings]\nPos=442,290\nSize=308,60\nCollapsed=0\n\n[Docking][Data]\nDockSpace       ID=0xD01BFD3D Pos=0,19 Size=1200,631 Split=X\n  DockNode      ID=0x00000003 Parent=0xD01BFD3D SizeRef=757,631 Split=Y\n    DockNode    ID=0x00000001 Parent=0x00000003 SizeRef=1200,359 Split=X\n      DockNode  ID=0x00000009 Parent=0x00000001 SizeRef=355,462 SelectedTab=0x83199EB2\n      DockNode  ID=0x0000000A Parent=0x00000001 SizeRef=400,462 CentralNode=1 SelectedTab=0xCB5DF48C\n    DockNode    ID=0x00000002 Parent=0x00000003 SizeRef=1200,270 SelectedTab=0xB7722E25\n  DockNode      ID=0x00000004 Parent=0xD01BFD3D SizeRef=441,631 Split=Y SelectedTab=0x6D97690D\n    DockNode    ID=0x00000005 Parent=0x00000004 SizeRef=334,438 Split=X SelectedTab=0x91CA6522\n      DockNode  ID=0x00000007 Parent=0x00000005 SizeRef=225,438 SelectedTab=0x7DA7F56F\n      DockNode  ID=0x00000008 Parent=0x00000005 SizeRef=214,438 SelectedTab=0x91CA6522\n    DockNode    ID=0x00000006 Parent=0x00000004 SizeRef=334,191 SelectedTab=0x6D97690D\n\n",
    "[Window][Main Dock]\nPos=0,0\nSize=1200,649\nCollapsed=0\n\n[Window][Debug##Default]\nPos=60,60\nSize=400,400\nCollapsed=0\n\n[Window][Game]\nPos=0,19\nSize=1200,630\nCollapsed=0\nDockId=0x00000001,0\n\n[Window][Editor]\nPos=357,19\nSize=400,359\nCollapsed=0\nDockId=0x00000001,0\n\n[Window][Objects]\nPos=759,19\nSize=225,438\nCollapsed=0\nDockId=0x00000007,0\n\n[Window][Watch]\nPos=759,19\nSize=441,438\nCollapsed=0\nDockId=0x00000008,0\n\n[Window][Log]\nPos=0,380\nSize=1200,270\nCollapsed=0\nDockId=0x00000002,0\n\n[Window][Resources]\nPos=759,19\nSize=441,631\nCollapsed=0\nDockId=0x00000006,0\n\n[Window][Mouse Picker]\nPos=0,19\nSize=256,289\nCollapsed=0\nDockId=0x00000001,1\n\n[Window][Stuff]\nPos=0,19\nSize=256,289\nCollapsed=0\nDockId=0x00000001,2\n\n[Window][ImGui Demo]\nPos=0,483\nSize=919,167\nCollapsed=0\nDockId=0x00000002,1\n\n[Window][Deferred G-Buffer]\nPos=0,483\nSize=919,167\nCollapsed=0\nDockId=0x00000002,1\n\n[Window][Material Editor]\nPos=986,19\nSize=214,438\nCollapsed=0\nDockId=0x00000008,1\n\n[Window][2D Animation Editor]\nPos=0,19\nSize=355,359\nCollapsed=0\nDockId=0x00000001,1\n\n[Window][Grid Settings]\nPos=442,290\nSize=308,60\nCollapsed=0\n\n[Docking][Data]\nDockSpace       ID=0xD01BFD3D Pos=0,19 Size=1200,630 Split=X\n  DockNode      ID=0x00000003 Parent=0xD01BFD3D SizeRef=757,631 Split=Y\n    DockNode    ID=0x00000001 Parent=0x00000003 SizeRef=1200,359 CentralNode=1 SelectedTab=0x83199EB2\n    DockNode    ID=0x00000002 Parent=0x00000003 SizeRef=1200,270 SelectedTab=0xB7722E25\n  DockNode      ID=0x00000004 Parent=0xD01BFD3D SizeRef=441,631 Split=Y SelectedTab=0x6D97690D\n    DockNode    ID=0x00000005 Parent=0x00000004 SizeRef=334,438 Split=X SelectedTab=0x91CA6522\n      DockNode  ID=0x00000007 Parent=0x00000005 SizeRef=225,438 SelectedTab=0x7DA7F56F\n      DockNode  ID=0x00000008 Parent=0x00000005 SizeRef=214,438 SelectedTab=0x91CA6522\n    DockNode    ID=0x00000006 Parent=0x00000004 SizeRef=334,191 SelectedTab=0x6D97690D\n\n",
};

const char* g_EditorLayoutMenuLabels[EditorLayout_NumLayouts] =
{
    "Center Editor",
    "Center Game",
    "Side by Side",
    "Full Frame Game",
};

EditorLayoutManager_ImGui::EditorLayoutManager_ImGui()
{
    // Initialize the default layouts before loading any customized ones from disk.
    {
        // "Is window open" booleans.
        for( int i=0; i<EditorLayout_NumLayouts; i++ )
        {
            // Set all windows to be closed by default ...
            for( int j=0; j<EditorWindow_NumTypes; j++ )
                m_DefaultLayouts[i].m_IsWindowOpen[j] = false;

            // ... except these.
            m_DefaultLayouts[i].m_IsWindowOpen[EditorWindow_Game] = true;
            m_DefaultLayouts[i].m_IsWindowOpen[EditorWindow_Editor] = true;
            m_DefaultLayouts[i].m_IsWindowOpen[EditorWindow_ObjectList] = true;
            m_DefaultLayouts[i].m_IsWindowOpen[EditorWindow_Watch] = true;
            m_DefaultLayouts[i].m_IsWindowOpen[EditorWindow_Resources] = true;
            m_DefaultLayouts[i].m_IsWindowOpen[EditorWindow_Log] = true;
        }

        // "Full Frame Game" layout only shows game window.
        m_DefaultLayouts[EditorLayout_FullFrameGame].m_IsWindowOpen[EditorWindow_Editor] = false;
        m_DefaultLayouts[EditorLayout_FullFrameGame].m_IsWindowOpen[EditorWindow_ObjectList] = false;
        m_DefaultLayouts[EditorLayout_FullFrameGame].m_IsWindowOpen[EditorWindow_Watch] = false;
        m_DefaultLayouts[EditorLayout_FullFrameGame].m_IsWindowOpen[EditorWindow_Resources] = false;
        m_DefaultLayouts[EditorLayout_FullFrameGame].m_IsWindowOpen[EditorWindow_Log] = false;

        // ImGui Ini strings.
        for( int i=0; i<EditorLayout_NumLayouts; i++ )
        {
            m_DefaultLayouts[i].m_ImGuiIniString = g_DefaultEditorLayoutImGuiIniStrings[i];
        }

        // Copy all layouts to m_CustomLayouts so we don't modify default layouts and can reset to them.
        for( int i=0; i<EditorLayout_NumLayouts; i++ )
        {
            m_CustomLayouts[i] = m_DefaultLayouts[i];
        }
    }

    m_SelectedLayout_EditorMode = EditorLayout_CenterEditor;
    m_SelectedLayout_GameMode = EditorLayout_CenterEditor;

    m_SwitchingToEditorLayout = false;
    m_SwitchingToGameLayout = false;

    // Set intial layout to the selected editor layout.  Set current layout to none to force a reset.
    m_CurrentLayoutIndex = EditorLayout_NumLayouts;
    m_RequestedLayoutIndex = m_SelectedLayout_EditorMode;
}

EditorLayoutManager_ImGui::~EditorLayoutManager_ImGui()
{
}

EditorLayout* EditorLayoutManager_ImGui::GetCurrentLayout()
{
    MyAssert( m_CurrentLayoutIndex >= 0 && m_CurrentLayoutIndex < EditorLayout_NumLayouts );
    
    return &m_CustomLayouts[m_CurrentLayoutIndex];
}

void EditorLayoutManager_ImGui::LoadPrefs(cJSON* jPrefs)
{
    cJSON* obj;

    // Load selected layout for each mode.
    obj = cJSON_GetObjectItem( jPrefs, "SelectedLayout_EditorMode" );
    if( obj )
        m_SelectedLayout_EditorMode = (EditorLayoutTypes)obj->valueint;
    
    obj = cJSON_GetObjectItem( jPrefs, "SelectedLayout_GameMode" );
    if( obj )
        m_SelectedLayout_GameMode = (EditorLayoutTypes)obj->valueint;

    if( m_SelectedLayout_EditorMode < 0 || m_SelectedLayout_EditorMode >= EditorLayout_NumLayouts )
        m_SelectedLayout_EditorMode = EditorLayout_CenterEditor;
    if( m_SelectedLayout_GameMode < 0 || m_SelectedLayout_GameMode >= EditorLayout_NumLayouts )
        m_SelectedLayout_GameMode = EditorLayout_CenterEditor;

    // Load all customized layouts.
    cJSON* jLayoutsArray = cJSON_GetObjectItem( jPrefs, "Layouts" );
    int numItems = cJSON_GetArraySize( jLayoutsArray );
    for( int i=0; i<numItems; i++ )
    {
        if( i >= EditorLayout_NumLayouts )
        {
            LOGError( LOGTag, "Too many layouts in prefs file\n" );
            break;
        }

        cJSON* jLayout = cJSON_GetArrayItem( jLayoutsArray, i );

        cJSONExt_GetBoolArray( jLayout, "IsWindowOpen", m_CustomLayouts[i].m_IsWindowOpen, EditorWindow_NumTypes );
        obj = cJSON_GetObjectItem( jLayout, "IniString" );
        if( obj )
            m_CustomLayouts[i].m_ImGuiIniString = obj->valuestring;
    }

    // Request the loaded editor layout.
    RequestEditorLayout();
}

void EditorLayoutManager_ImGui::SavePrefs(cJSON* jPrefs)
{
    // Save the state of the active layout.
    SyncCurrentImGuiIni();

    // Save selected layout for each mode.
    cJSON_AddNumberToObject( jPrefs, "SelectedLayout_EditorMode", m_SelectedLayout_EditorMode );
    cJSON_AddNumberToObject( jPrefs, "SelectedLayout_GameMode", m_SelectedLayout_GameMode );

    // Save all customized layouts.
    cJSON* jLayoutsArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jPrefs, "Layouts", jLayoutsArray );
    for( int i=0; i<EditorLayout_NumLayouts; i++ )
    {
        cJSON* jLayout = cJSON_CreateObject();
        cJSON_AddItemToArray( jLayoutsArray, jLayout );

        cJSONExt_AddBoolArrayToObject( jLayout, "IsWindowOpen", m_CustomLayouts[i].m_IsWindowOpen, EditorWindow_NumTypes );
        cJSON_AddStringToObject( jLayout, "IniString", m_CustomLayouts[i].m_ImGuiIniString.c_str() );
    }
}

void EditorLayoutManager_ImGui::DumpCurrentLayoutToOutputWindow()
{
    std::string newLayout = ImGui::SaveIniSettingsToMemory();
            
    // Replace all newlines with "\\n"
    size_t index = 0;
    while( ( index = newLayout.find( "\n", index ) ) != std::string::npos )
    {
        newLayout.replace( index, 1, "\\n" );
    }

    LOGInfo( LOGTag, newLayout.c_str() );
}

void EditorLayoutManager_ImGui::SyncCurrentImGuiIni()
{
    if( m_CurrentLayoutIndex != EditorLayout_NumLayouts )
    {
        const char* newLayout = ImGui::SaveIniSettingsToMemory();
        m_CustomLayouts[m_CurrentLayoutIndex].m_ImGuiIniString = newLayout;
    }
}

void EditorLayoutManager_ImGui::ResetCurrentLayout()
{
    if( m_CurrentLayoutIndex != EditorLayout_NumLayouts )
    {
        m_CustomLayouts[m_CurrentLayoutIndex] = m_DefaultLayouts[m_CurrentLayoutIndex];

        // Force a reset to the same layout.
        RequestLayoutChange( m_CurrentLayoutIndex, true );
    }
}

void EditorLayoutManager_ImGui::RequestLayoutChange(EditorLayoutTypes layout, bool discardModificationsAndForceChange)
{
    m_RequestedLayoutIndex = layout;

    if( discardModificationsAndForceChange )
    {
        // Set current layout to none to force a reset.
        m_CurrentLayoutIndex = EditorLayout_NumLayouts;
    }
}

void EditorLayoutManager_ImGui::RequestEditorLayout()
{
    RequestLayoutChange( m_SelectedLayout_EditorMode, false );
    m_SwitchingToEditorLayout = true;
}

void EditorLayoutManager_ImGui::RequestGameLayout()
{
    RequestLayoutChange( m_SelectedLayout_GameMode, false );
    m_SwitchingToGameLayout = true;
}

void EditorLayoutManager_ImGui::ApplyLayoutChange()
{
    MyAssert( m_RequestedLayoutIndex >= 0 && m_RequestedLayoutIndex < EditorLayout_NumLayouts );

    if( m_RequestedLayoutIndex != m_CurrentLayoutIndex )
    {
        // Save the current layout.
        SyncCurrentImGuiIni();

        // Reset the imgui context.
        g_pImGuiManager->Shutdown( false );
        g_pImGuiManager->Init( (float)g_pEditorPrefs->GetWindowWidth(), (float)g_pEditorPrefs->GetWindowHeight() );

        // Reapply current imgui color/etc style.
        g_pEditorPrefs->GetImGuiStylePrefs()->ReapplyCurrentPreset();

        // Store the index to the active layout and have imgui load the ini string.
        m_CurrentLayoutIndex = m_RequestedLayoutIndex;
        ImGui::LoadIniSettingsFromMemory( m_CustomLayouts[m_CurrentLayoutIndex].m_ImGuiIniString.c_str() );
    }
}

void EditorLayoutManager_ImGui::FinishFocusChangeIfNeeded()
{
    if( g_pEditorPrefs->Get_Mode_SwitchFocusOnPlayStop() == false )
        return;

    // Fix focus if we're switching into editor or game mode.
    // This needs to be done after all windows have been rebuild after switching layouts.

    if( m_SwitchingToEditorLayout )
    {
        m_SwitchingToEditorLayout = false;
        ImGui::SetWindowFocus( "Editor" );
    }

    if( m_SwitchingToGameLayout )
    {
        m_SwitchingToGameLayout = false;
        ImGui::SetWindowFocus( "Game" );
    }
}
