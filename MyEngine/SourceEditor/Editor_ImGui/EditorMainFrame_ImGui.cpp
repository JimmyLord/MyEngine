//
// Copyright (c) 2018-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EditorMemoryWindow_ImGui.h"

#include "ImGuiStylePrefs.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/FrameworkComponents/ComponentAnimationPlayer2D.h"
#include "ComponentSystem/FrameworkComponents/ComponentAudioPlayer.h"
#include "ComponentSystem/FrameworkComponents/ComponentMeshOBJ.h"
#include "Core/EngineComponentTypeManager.h"
#include "Core/EngineCore.h"
#include "GUI/EditorIcons.h"
#include "GUI/ImGuiExtensions.h"
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/Commands/EditorMenuCommands.h"
#include "../SourceEditor/Commands/EngineCommandStack.h"
#include "../SourceEditor/Commands/EngineEditorCommands.h"
#include "../SourceEditor/Documents/EditorDocument.h"
#include "../SourceEditor/Editor_ImGui/EditorLayoutManager_ImGui.h"
#include "../SourceEditor/Editor_ImGui/EditorLogWindow_ImGui.h"
#include "../SourceEditor/Editor_ImGui/EditorMainFrame_ImGui.h"
#include "../SourceEditor/GameObjectTemplateManager.h"
#include "../SourceEditor/Interfaces/EditorInterface.h"
#include "../SourceEditor/NodeGraph/MyNodeGraph.h"
#include "../SourceEditor/NodeGraph/VisualScriptNodes.h"
#include "../SourceEditor/Prefs/EditorKeyBindings.h"
#include "../SourceEditor/TransformGizmo.h"

//====================================================================================================
// Various enums and matching strings (some unused)
//====================================================================================================

const char* g_DefaultEditorWindowTypeMenuLabels[EditorWindow_NumTypes] =
{
    "Game View",
    "Editor View",
    "Object List",
    "Watch",
    "Resources",
    "Log",
    "Grid Settings",
    "Material Editor",
    "2D Animation Editor",
    "Debug: Mouse Picker",
    "Debug: Stuff",
    "Debug: Memory Allocations",
    "Debug: Undo/Redo Stacks",
    "Debug: ImGui Demo",
};

enum PanelMemoryPages
{
    PanelMemoryPage_Materials,
    PanelMemoryPage_Textures,
    PanelMemoryPage_ShaderGroups,
    PanelMemoryPage_SoundCues,
    PanelMemoryPage_Files,
    PanelMemoryPage_Buffers,
    PanelMemoryPage_DrawCalls,
    PanelMemoryPage_NumTypes
};

const char* g_PanelMemoryPagesMenuLabels[PanelMemoryPage_NumTypes] =
{
    "Materials",
    "Textures",
    "Shaders",
    "Sound Cues",
    "Files",
    "Buffers",
    "Draw Calls",
};

//====================================================================================================
// Public methods
//====================================================================================================

EditorMainFrame_ImGui::EditorMainFrame_ImGui(EngineCore* pEngineCore)
{
    m_pEngineCore = pEngineCore;

    // Documents.
    m_pActiveDocument = nullptr;
    m_pLastActiveDocument = nullptr;

    // Layouts.
    m_pLayoutManager = MyNew EditorLayoutManager_ImGui( this );
    m_pCurrentLayout = nullptr;

    // Warnings.
    m_ShowWarning_NewScene = false;
    m_ShowWarning_LoadScene = false;
    m_ShowWarning_CloseDocument = false;
    m_DocumentIndexToCloseAfterWarning = -1;
    m_ShowWarning_CloseEditor = false;

    // Render surfaces.
    m_pGameFBO = m_pEngineCore->GetManagers()->GetTextureManager()->CreateFBO( 1024, 1024, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, true );
    m_pEditorFBO = m_pEngineCore->GetManagers()->GetTextureManager()->CreateFBO( 1024, 1024, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, true );
    m_pMaterialPreviewFBO = m_pEngineCore->GetManagers()->GetTextureManager()->CreateFBO( 1024, 1024, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, true );

#if MYFW_EDITOR
    m_pGameFBO->MemoryPanel_Hide();
    m_pEditorFBO->MemoryPanel_Hide();
    m_pMaterialPreviewFBO->MemoryPanel_Hide();
#endif

    // Material Preview and Editor.
    m_pMaterialToPreview = nullptr;
    m_pMaterialBeingEdited = nullptr;

    // 2D Animation Editor.
    m_FullPathToLast2DAnimInfoBeingEdited[0] = '\0';
    m_p2DAnimInfoBeingEdited = nullptr;
    m_Current2DAnimationIndex = 0;
    m_pAnimPlayerComponent = MyNew ComponentAnimationPlayer2D( pEngineCore->GetComponentSystemManager() );
    m_pAnimPlayerComponent->SetType( ComponentType_AnimationPlayer2D );
    m_pAnimPlayerComponent->SetSceneID( SCENEID_AllScenes );

    // Log Window.
    m_pLogWindow = MyNew EditorLogWindow_ImGui( m_pEngineCore, true );
    m_pMemoryWindow = MyNew EditorMemoryWindow_ImGui();

    // Object list.
    m_pGameObjectToDrawReorderLineAfter = nullptr;
    m_SetObjectListFilterBoxInFocus = false;
    m_ObjectListFilter[0] = '\0';

    // Memory panel.
    m_CurrentMemoryPanelPage = PanelMemoryPage_Materials;
    m_SetMemoryPanelFilterBoxInFocus = false;
    m_MemoryPanelFilter[0] = '\0';

    // For renaming things.
    m_RenamePressedThisFrame = false;
    m_ConfirmCurrentRenameOp = false;
    m_RenameTimerForSlowDoubleClick = 0.0f;
    m_RenameOp_LastObjectClicked = nullptr;
    m_pGameObjectWhoseNameIsBeingEdited = nullptr;
    m_pMaterialWhoseNameIsBeingEdited = nullptr;
    m_NameBeingEdited[0] = '\0';

    // Draw call debugger.
    m_SelectedDrawCallCanvas = -1;
    m_SelectedDrawCallIndex = -1;

    // Game and Editor windows.
    m_GameWindowPos.Set( -1, -1 );
    m_EditorWindowPos.Set( -1, -1 );
    m_GameWindowSize.Set( 0, 0 );
    m_EditorWindowSize.Set( 0, 0 );

    m_GameWindowFocused = false;
    m_GameWindowVisible = false;
    m_EditorWindowHovered = false;
    m_EditorWindowFocused = false;
    m_EditorWindowVisible = false;

    m_CurrentMouseInEditorWindow_X = -1;
    m_CurrentMouseInEditorWindow_Y = -1;

    // Misc.
    m_pLastGameObjectInteractedWithInObjectPanel = nullptr;
    m_UndoStackDepthAtLastSave = 0;

    // Modifier key states.
    m_KeyDownCtrl = false;
    m_KeyDownAlt = false;
    m_KeyDownShift = false;
    m_KeyDownCommand = false;

    // Master Undo/Redo Stack for ImGui editor builds.
    m_pCommandStack = MyNew EngineCommandStack();
    m_pEngineCore->SetCommandStack( m_pCommandStack );
}

EditorMainFrame_ImGui::~EditorMainFrame_ImGui()
{
    m_pAnimPlayerComponent->SetEnabled( false );
    SAFE_DELETE( m_pAnimPlayerComponent );

    SAFE_DELETE( m_pCommandStack );
    m_pEngineCore->SetCommandStack( nullptr );
    SAFE_DELETE( m_pLogWindow );
    SAFE_DELETE( m_pMemoryWindow );

    SAFE_RELEASE( m_pGameFBO );
    SAFE_RELEASE( m_pEditorFBO );
    SAFE_RELEASE( m_pMaterialPreviewFBO );

    delete m_pLayoutManager;
}

Vector2 EditorMainFrame_ImGui::GetEditorWindowCenterPosition()
{
    return m_EditorWindowPos + m_EditorWindowSize/2;
}

void EditorMainFrame_ImGui::StoreCurrentUndoStackSize()
{
    m_UndoStackDepthAtLastSave = m_pCommandStack->GetUndoStackSize();
}

bool EditorMainFrame_ImGui::HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    ImGuiIO& io = ImGui::GetIO();

    if( keyAction == GCBA_Down )
    {
        if( keyCode == MYKEYCODE_LCTRL || keyCode == MYKEYCODE_RCTRL )
            m_KeyDownCtrl = true;

        if( keyCode == MYKEYCODE_LALT || keyCode == MYKEYCODE_RALT )
            m_KeyDownAlt = true;

        if( keyCode == MYKEYCODE_LSHIFT || keyCode == MYKEYCODE_RSHIFT )
            m_KeyDownShift = true;
    }
    
    if( keyAction == GCBA_Up )
    {
        if( keyCode == MYKEYCODE_LCTRL || keyCode == MYKEYCODE_RCTRL )
            m_KeyDownCtrl = false;

        if( keyCode == MYKEYCODE_LALT || keyCode == MYKEYCODE_RALT )
            m_KeyDownAlt = false;

        if( keyCode == MYKEYCODE_LSHIFT || keyCode == MYKEYCODE_RSHIFT )
            m_KeyDownShift = false;
    }

    if( mouseAction == GCBA_Down && id == 0 )
    {
        // For renaming, if the mouse is clicked anywhere, end/confirm the current rename operation.
        if( m_pGameObjectWhoseNameIsBeingEdited || m_pMaterialWhoseNameIsBeingEdited )
        {
            m_ConfirmCurrentRenameOp = true;
        }

        // When binding new keys, if the mouse is clicked anywhere, cancel the operation.
        EditorKeyBindings* pKeyBindings = m_pEngineCore->GetEditorPrefs()->GetKeyBindings();
        pKeyBindings->CancelBindingAction();
    }

    // For keyboard and other non-mouse events, localx/y will be -1.
    float localx = -1;
    float localy = -1;

    if( mouseAction == GCBA_RelativeMovement )
    {
        // localx/y will hold relative movement in this case.
        localx = x;
        localy = y;
    }

    // Read the absolute x/y from the ImGui structure, since non-mouse messages will have x,y of -1,-1.
    float mouseabsx = io.MousePos.x;
    float mouseabsy = io.MousePos.y;

    // Deal with sending input events to the active document.
    for( uint32 i=0; i<m_pEngineCore->GetEditorState()->m_pOpenDocuments.size(); i++ )
    {
        EditorDocument* pDocument = m_pEngineCore->GetEditorState()->m_pOpenDocuments[i];

        if( pDocument->IsWindowVisible() )
        {
            // If the right or middle mouse buttons were clicked on this window, set it as having focus.
            // Needed since those buttons don't focus ImGui window directly.
            if( pDocument->IsWindowHovered() && mouseAction == GCBA_Down && id != 0 )
            {
                const uint32 tempTitleAllocationSize = MAX_PATH*2+5;
                static char tempTitle[tempTitleAllocationSize];
                pDocument->GetWindowTitle( tempTitle, tempTitleAllocationSize );

                ImGui::SetWindowFocus( tempTitle );
            }

            // Always send keyboard actions to window in focus, only send mouse messages if in focus and hovered.
            // Pass keyboard and mouse events to the editor under various conditions.
            if( ( pDocument->IsWindowFocused() && keyAction != -1 ) ||
                ( pDocument->IsWindowFocused() && (mouseAction == GCBA_Up || mouseAction == GCBA_Held || mouseAction == GCBA_RelativeMovement) ) ||
                ( pDocument->IsWindowHovered() )
              )
            {
                Vector2 pos = pDocument->GetWindowPosition();
                Vector2 size = pDocument->GetWindowSize();

                // If this is an absolute mouse input over the document window.
                if( ( mouseAction != -1 && mouseAction != GCBA_RelativeMovement &&
                    mouseabsx >= pos.x && mouseabsx < pos.x + size.x &&
                    mouseabsy >= pos.y && mouseabsy < pos.y + size.y ) )
                {
                    // Calculate mouse x/y relative to this window.
                    localx = x - pos.x;
                    localy = size.y - (y - pos.y);
                }

                // Set modifier key and mouse button states.
                m_pEngineCore->GetEditorState()->SetModifierKeyStates( keyAction, keyCode, mouseAction, id, localx, localy, pressure );
                //LOGInfo( "Mouse Bug", "Document Set: %0.0f, %0.0f", localx, localy );

                if( pDocument->HandleInput( keyAction, keyCode, mouseAction, id, localx, localy, pressure ) )
                {
                    return true;
                }

                // Clear modifier key and mouse button states.
                m_pEngineCore->GetEditorState()->ClearModifierKeyStates( keyAction, keyCode, mouseAction, id, localx, localy, pressure );
                //LOGInfo( "Mouse Bug", "Document Clear: %0.0f, %0.0f", localx, localy );
            }
        }
    }

    // Deal with sending input events to the game window.
    {
        // If this is a keyAction and the game window is in focus.
        if( keyAction != -1 && m_GameWindowFocused )
        {
            // Check for Ctrl-Space keypress while the game is running and in focus to return to editor mode.
            // TODO: Set this as an option, since some games might actually want to use the ctrl-space combination.
            if( m_KeyDownCtrl && keyCode == ' ' && keyAction == GCBA_Down )
            {
                EditorMenuCommand( EditorMenuCommand_Mode_TogglePlayStop );
                return true;
            }

            return false; // Let event continue to the game window.
        }

        // If this is an absolute mouse input over the game window.
        if( ( mouseAction != -1 && mouseAction != GCBA_RelativeMovement &&
              mouseabsx >= m_GameWindowPos.x && mouseabsx < m_GameWindowPos.x + m_GameWindowSize.x &&
              mouseabsy >= m_GameWindowPos.y && mouseabsy < m_GameWindowPos.y + m_GameWindowSize.y ) )
        {
            // Calculate mouse x/y relative to this window.
            localx = x - m_GameWindowPos.x;
            localy = y - m_GameWindowPos.y;

            if( mouseAction == GCBA_Down )
                m_GameWindowFocused = true;

            m_pEngineCore->OnTouchGameWindow( mouseAction, id, localx, localy, pressure, 1 );

            // Input was used.
            return true;
        }

        // Pass relative x/y to the game window if it's in focus.
        if( mouseAction == GCBA_RelativeMovement && m_GameWindowFocused )
        {
            m_pEngineCore->OnTouchGameWindow( mouseAction, id, localx, localy, pressure, 1 );

            // Input was used.
            return true;
        }
    }

    // Deal with sending input events to the editor window.
    {
        // Pass keyboard and mouse events to the editor under various conditions.
        if( ( m_EditorWindowFocused && keyAction != -1 ) ||
            ( m_EditorWindowFocused && (mouseAction == GCBA_Up || mouseAction == GCBA_Held || mouseAction == GCBA_RelativeMovement) ) ||
            ( m_EditorWindowHovered )
          )
        {
            // If this is a mouse message and not a relative movement,
            //     calculate mouse x/y relative to this window.
            if( mouseAction != -1 && mouseAction != GCBA_RelativeMovement )
            {
                localx = x - m_EditorWindowPos.x;
                localy = m_EditorWindowSize.y - (y - m_EditorWindowPos.y);
            }

            // Set modifier key and mouse button states.
            m_pEngineCore->GetEditorState()->SetModifierKeyStates( keyAction, keyCode, mouseAction, id, localx, localy, pressure );
            //LOGInfo( "Mouse Bug", "Editor Set: %0.0f, %0.0f", localx, localy );

            m_CurrentMouseInEditorWindow_X = (unsigned int)localx;
            m_CurrentMouseInEditorWindow_Y = (unsigned int)localy;

            // If the right or middle mouse buttons were clicked on this window, set it as having focus.
            // Needed since those buttons don't focus ImGui window directly.
            if( mouseAction == GCBA_Down && id != 0 )
            {
                ImGui::SetWindowFocus( "Editor" );
            }

            // First, pass the input into the current editor interface.
            if( m_pEngineCore->GetCurrentEditorInterface()->HandleInput( keyAction, keyCode, mouseAction, id, localx, localy, pressure ) )
                return true;

            // If it wasn't used, pass it to the transform gizmo.
            if( m_pEngineCore->GetEditorState()->m_pTransformGizmo->HandleInput( m_pEngineCore, -1, -1, mouseAction, id, localx, localy, pressure ) )
                return true;

            // Clear modifier key and mouse button states.
            m_pEngineCore->GetEditorState()->ClearModifierKeyStates( keyAction, keyCode, mouseAction, id, localx, localy, pressure );
            //LOGInfo( "Mouse Bug", "Editor Clear: %0.0f, %0.0f", localx, localy );
        }
    }

    // If neither the game or editor windows used the input, then check for global hotkeys or cancel certain ops.
    if( keyAction != -1 )
    {
        EditorKeyBindings* pKeyBindings = m_pEngineCore->GetEditorPrefs()->GetKeyBindings();

        // EditorKeyBindings will catch this input and change a hotkey if it's waiting to register one.
        if( pKeyBindings->HandleInput( keyAction, keyCode ) )
        {
            // If a hotkey was pressed, unset that key so 'held' and 'up' messages won't get sent.
            m_pEngineCore->ForceKeyRelease( keyCode );
            return true;
        }
        else if( io.WantTextInput == false )
        {
            if( CheckForHotkeys( keyAction, keyCode ) )
            {
                // If a hotkey was pressed, unset that key so 'held' and 'up' messages won't get sent.
                m_pEngineCore->ForceKeyRelease( keyCode );
                return true;
            }
        }
        else
        {
            // Cancel rename operation if an ImGui input box has focus and escape is pressed.
            if( keyCode == MYKEYCODE_ESC )
            {
                m_pGameObjectWhoseNameIsBeingEdited = nullptr;
                m_pMaterialWhoseNameIsBeingEdited = nullptr;
                return true;
            }
        }

        // Cancel any drag/drop ops when escape is pressed.
        if( keyCode == MYKEYCODE_ESC )
        {
            ImGuiExt::ClearDragDrop();
        }
    }

    // Absorb the message, even if we didn't do anything with it.
    return true;
}

bool EditorMainFrame_ImGui::CheckForHotkeys(int keyAction, int keyCode)
{
    if( keyAction == GCBA_Down )
    {
        bool N  = !m_KeyDownCtrl && !m_KeyDownAlt && !m_KeyDownShift && !m_KeyDownCommand; // No modifiers held
        bool C  =  m_KeyDownCtrl && !m_KeyDownAlt && !m_KeyDownShift && !m_KeyDownCommand; // Ctrl
        bool A  = !m_KeyDownCtrl &&  m_KeyDownAlt && !m_KeyDownShift && !m_KeyDownCommand; // Alt
        bool S  = !m_KeyDownCtrl && !m_KeyDownAlt &&  m_KeyDownShift && !m_KeyDownCommand; // Shift
        bool CS =  m_KeyDownCtrl && !m_KeyDownAlt &&  m_KeyDownShift && !m_KeyDownCommand; // Ctrl-Shift

        // Handle GameObject renaming, not the best idea to do this here, but okay for a start.
        // TODO: Check if F2 or Enter was pressed when menuitem has focus.
        if( N  && keyCode == VK_F2 || N  && keyCode == MYKEYCODE_ENTER )
        {
            m_RenamePressedThisFrame = true;
            m_ConfirmCurrentRenameOp = false;
            return true;
        }

        if( N  && keyCode == MYKEYCODE_DELETE )
        {
            m_pEngineCore->GetEditorState()->DeleteSelectedObjects();
            return true;
        }

        EditorPrefs* pEditorPrefs = m_pEngineCore->GetEditorPrefs();

        EditorKeyBindings* pKeys = m_pEngineCore->GetEditorPrefs()->GetKeyBindings();
        uint8 modifiers = static_cast<uint8>( m_pEngineCore->GetEditorState()->m_ModifierKeyStates );

        // Check all hotkeys and execute their actions if required.
        for( int i=0; i<(int)HotkeyAction::Num; i++ )
        {
            HotkeyAction action = static_cast<HotkeyAction>( i );
            
            // Check if this is a global hotkey.
            if( pKeys->KeyMatches( action, pKeys->GetModifiersHeld(), keyCode ) )
            {
                if( ExecuteHotkeyAction( action ) )
                {
                    return true;
                }
            }
            else if( m_pActiveDocument )
            {
                if( pKeys->KeyMatches( action, pKeys->GetModifiersHeld(), keyCode, m_pActiveDocument->GetHotkeyContext() ) )
                {
                    if( m_pActiveDocument->ExecuteHotkeyAction( action ) )
                    {
                        return true;
                    }
                }
            }
            else
            {
                //HotkeyContext context = m_pEngineCore->GetCurrentEditorInterface()->GetHotkeyContext();

                //if( pKeys->KeyMatches( action, pKeys->GetModifiersHeld(), keyCode, context ) )
                //{
                //    if( m_pEngineCore->GetCurrentEditorInterface()->ExecuteHotkeyAction( action ) )
                //    {
                //        return true;
                //    }
                //}
            }
        }

#if _DEBUG
        // Dump current layout to output window, so it can be cut & pasted to g_DefaultLayouts in the EditorLayoutManager.
        if( CS && keyCode == 'D'   ) { m_pLayoutManager->DumpCurrentLayoutToOutputWindow();                         return true; }
#endif
    }

    return false;
}

bool EditorMainFrame_ImGui::ExecuteHotkeyAction(HotkeyAction action)
{
    EditorPrefs* pEditorPrefs = m_pEngineCore->GetEditorPrefs();

    switch( action )
    {
    case HotkeyAction::Global_Find:                   { ImGui::SetWindowFocus( "Objects" ); m_SetObjectListFilterBoxInFocus = true;  return true; }
    case HotkeyAction::File_SaveScene:                { EditorMenuCommand( EditorMenuCommand_File_SaveScene );                       return true; }
    case HotkeyAction::File_SaveAll:                  { EditorMenuCommand( EditorMenuCommand_File_SaveAll );                         return true; }
    case HotkeyAction::File_ExportBox2D:              { EditorMenuCommand( EditorMenuCommand_File_Export_Box2DScene );               return true; }
    case HotkeyAction::File_Preferences:              { pEditorPrefs->Display();                                                     return true; }
    case HotkeyAction::Edit_Undo:                     { EditorMenuCommand( EditorMenuCommand_Edit_Undo );                            return true; }
    case HotkeyAction::Edit_Redo:                     { EditorMenuCommand( EditorMenuCommand_Edit_Redo );                            return true; }
    case HotkeyAction::View_ShowEditorCamProperties:  { EditorMenuCommand( EditorMenuCommand_View_ShowEditorCamProperties );         return true; }
    case HotkeyAction::View_ShowEditorIcons:          { EditorMenuCommand( EditorMenuCommand_View_ShowEditorIcons );                 return true; }
    case HotkeyAction::View_ToggleEditorCamDeferred:  { EditorMenuCommand( EditorMenuCommand_View_ToggleEditorCamDeferred );         return true; }
    case HotkeyAction::View_Full:                     { pEditorPrefs->Set_Aspect_GameAspectRatio( GLView_Full );                     return true; }
    case HotkeyAction::View_Tall:                     { pEditorPrefs->Set_Aspect_GameAspectRatio( GLView_Tall );                     return true; }
    case HotkeyAction::View_Square:                   { pEditorPrefs->Set_Aspect_GameAspectRatio( GLView_Square );                   return true; }
    case HotkeyAction::View_Wide:                     { pEditorPrefs->Set_Aspect_GameAspectRatio( GLView_Wide );                     return true; }
    case HotkeyAction::Grid_Visible:                  { EditorMenuCommand( EditorMenuCommand_Grid_Visible );                         return true; }
    case HotkeyAction::Grid_SnapEnabled:              { EditorMenuCommand( EditorMenuCommand_Grid_SnapEnabled );                     return true; }
    case HotkeyAction::Grid_Settings:                 { m_pCurrentLayout->m_IsWindowOpen[EditorWindow_GridSettings] = true;          return true; }
    case HotkeyAction::Mode_TogglePlayStop:           { EditorMenuCommand( EditorMenuCommand_Mode_TogglePlayStop );                  return true; }
    case HotkeyAction::Mode_Pause:                    { EditorMenuCommand( EditorMenuCommand_Mode_Pause );                           return true; }
    case HotkeyAction::Mode_AdvanceOneFrame:          { EditorMenuCommand( EditorMenuCommand_Mode_AdvanceOneFrame );                 return true; }
    case HotkeyAction::Mode_AdvanceOneSecond:         { EditorMenuCommand( EditorMenuCommand_Mode_AdvanceOneSecond );                return true; }
    case HotkeyAction::Mode_LaunchGame:               { EditorMenuCommand( EditorMenuCommand_Mode_LaunchGame );                      return true; }
    case HotkeyAction::Debug_DrawWireframe:           { EditorMenuCommand( EditorMenuCommand_Debug_DrawWireframe );                  return true; }
    case HotkeyAction::Debug_ShowPhysicsShapes:       { EditorMenuCommand( EditorMenuCommand_Debug_ShowPhysicsShapes );              return true; }
    case HotkeyAction::Debug_ShowStats:               { EditorMenuCommand( EditorMenuCommand_Debug_ShowStats );              return true; }
    case HotkeyAction::Lua_RunLuaScript:              { EditorMenuCommand( EditorMenuCommand_Lua_RunLuaScript );                     return true; }
    case HotkeyAction::Objects_MergeIntoFolder:       { EditorMenuCommand( EditorMenuCommand_Objects_MergeIntoFolder );              return true; }

    // Handled elsewhere, and will return false so the key states won't be affected.
    case HotkeyAction::Camera_Forward:                { return false; }
    case HotkeyAction::Camera_Back:                   { return false; }
    case HotkeyAction::Camera_Left:                   { return false; }
    case HotkeyAction::Camera_Right:                  { return false; }
    case HotkeyAction::Camera_Up:                     { return false; }
    case HotkeyAction::Camera_Down:                   { return false; }
    case HotkeyAction::Camera_Focus:                  { return false; }

    case HotkeyAction::HeightmapEditor_Tool_Raise:    { return false; }
    case HotkeyAction::HeightmapEditor_Tool_Lower:    { return false; }
    case HotkeyAction::HeightmapEditor_Tool_Level:    { return false; }

    case HotkeyAction::Num:
        break;
    }

    return false;
}

void EditorMainFrame_ImGui::RequestCloseWindow()
{
    m_ShowWarning_CloseEditor = false;

    // If any Editor Document has unsaved changes, then pop up a warning.
    if( m_pEngineCore->GetEditorState()->DoAnyOpenDocumentsHaveUnsavedChanges() )
    {
        m_ShowWarning_CloseEditor = true;
    }

    // If the Scene command stack isn't clear, then pop up a warning.
    if( m_ShowWarning_CloseEditor || m_pCommandStack->GetUndoStackSize() != m_UndoStackDepthAtLastSave )
    {
        m_ShowWarning_CloseEditor = true;
    }
    else
    {
        g_pGameCore->SetGameConfirmedCloseIsOkay();
    }
}

void EditorMainFrame_ImGui::OnModeTogglePlayStop(bool nowInEditorMode)
{
    if( nowInEditorMode )
    {
        m_pLayoutManager->RequestEditorLayout();
    }
    else
    {
        m_pLayoutManager->RequestGameLayout();
    }
}

void EditorMainFrame_ImGui::Update(float deltaTime)
{
    m_RenameTimerForSlowDoubleClick += deltaTime;

    m_pAnimPlayerComponent->TickCallback( deltaTime );
}

void EditorMainFrame_ImGui::AddEverything()
{
    EditorPrefs* pEditorPrefs = m_pEngineCore->GetEditorPrefs();

    m_pCurrentLayout = m_pLayoutManager->GetCurrentLayout();

    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos( viewport->Pos );
    ImGui::SetNextWindowSize( viewport->Size );
    ImGui::SetNextWindowViewport( viewport->ID );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
    flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f) );
    ImGui::Begin( "Main Dock", nullptr, flags );
    ImGui::PopStyleVar();

    AddMainMenuBar();

    ImGuiID dockspace_id = ImGui::GetID( "MyDockspace" );
    ImGui::DockSpace( dockspace_id );
    ImGui::End();
    ImGui::PopStyleVar();

    AddGameAndEditorWindows();
    AddObjectList();
    AddWatchPanel();
    AddLogWindow();
    AddMemoryWindow();
    AddCommandStacksWindow();
    AddMemoryPanel();

    AddDebug_MousePicker();

    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_MaterialEditor] )
    {
        AddMaterialEditor();
    }

    Add2DAnimationEditor();

    AddLoseChangesWarningPopups();

    pEditorPrefs->AddCustomizationDialog();

#if _DEBUG
    ImGuiIO& io = ImGui::GetIO();
    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Debug_Stuff] )
    {
        if( ImGui::Begin( "Stuff", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Debug_Stuff] ) )
        {
            ImGui::Text( "WantCaptureKeyboard %d", io.WantCaptureKeyboard );
            ImGui::Text( "WantCaptureMouse %d", io.WantCaptureMouse );
            //ImGui::Text( "WantMoveMouse %d", io.WantMoveMouse );
            ImGui::Text( "WantTextInput %d", io.WantTextInput );
            ImGui::Text( "m_GameWindowFocused %d", m_GameWindowFocused );
            ImGui::Text( "m_EditorWindowHovered %d", m_EditorWindowHovered );    
            ImGui::Text( "m_EditorWindowFocused %d", m_EditorWindowFocused );
            ImGui::Text( "MouseWheel %0.2f", io.MouseWheel );
            ImGui::Text( "m_CurrentMouseInEditorWindow_X %d", m_CurrentMouseInEditorWindow_X );
            ImGui::Text( "m_CurrentMouseInEditorWindow_Y %d", m_CurrentMouseInEditorWindow_Y );

            GameObject* pGO = g_pComponentSystemManager->FindGameObjectByName( "Player" );
            if( pGO && pGO->GetTransform() )
            {
                ImGui::Text( "PlayerX %0.2f", pGO->GetTransform()->GetWorldTransform()->m41 );
            }
        }    
        ImGui::End();
    }

    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Debug_ImGuiDemo] )
    {
        ImGui::ShowDemoWindow( &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Debug_ImGuiDemo] );
    }
#endif //_DEBUG

    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_GridSettings] )
    {
        if( ImGui::Begin( "Grid Settings", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_GridSettings], ImVec2(308, 60), 1 ) )
        {
            // Create a context menu only available from the title bar.
            if( ImGui::BeginPopupContextItem() )
            {
                if( ImGui::MenuItem( "Close" ) )
                    m_pCurrentLayout->m_IsWindowOpen[EditorWindow_GridSettings] = false;

                ImGui::EndPopup();
            }

            pEditorPrefs->FillGridSettingsWindow();
        }
        ImGui::End();
    }

    // Clear m_pActiveDocument, a new one will be set if a document is in focus.
    m_pActiveDocument = nullptr;

    std::vector<EditorDocument*>* pOpenDocuments = &m_pEngineCore->GetEditorState()->m_pOpenDocuments;
    for( uint32 i=0; i<pOpenDocuments->size(); i++ )
    {
        EditorDocument* pDocument = (*pOpenDocuments)[i];

        ImGui::SetNextWindowSize( ImVec2(400, 400), ImGuiSetCond_FirstUseEver );
        bool documentStillOpen = true;
        pDocument->CreateWindowAndUpdate( &documentStillOpen );
        if( pDocument->IsWindowFocused() )
        {
            m_pActiveDocument = pDocument;
            m_pLastActiveDocument = pDocument;
        }

        if( documentStillOpen == false )
        {
            if( pDocument->HasUnsavedChanges() )
            {
                m_ShowWarning_CloseDocument = true;
                m_DocumentIndexToCloseAfterWarning = i;
            }
            else
            {
                m_pActiveDocument = nullptr;
                m_pLastActiveDocument = nullptr;
                delete pDocument;
                pOpenDocuments->erase( pOpenDocuments->begin() + i );
            }
        }
    }

    m_RenamePressedThisFrame = false;

    m_pLayoutManager->FinishFocusChangeIfNeeded();
}

void EditorMainFrame_ImGui::DrawGameAndEditorWindows(EngineCore* pEngineCore)
{
    // Only refresh the active document window.
    if( m_pActiveDocument != nullptr )
    {
        m_pActiveDocument->OnDrawFrame();
    }

    if( m_GameWindowVisible && m_GameWindowSize.LengthSquared() != 0 )
    {
        if( m_pGameFBO->GetColorTexture( 0 ) )
        {
            g_GLStats.NewCanvasFrame( 0 );

            // Draw game view.
            uint32 previousFBO = g_GLStats.m_CurrentFramebuffer;
            m_pGameFBO->Bind( false );

            uint32 x = 0;
            uint32 y = 0;
            uint32 w = (uint32)m_GameWindowSize.x;
            uint32 h = (uint32)m_GameWindowSize.y;

            EditorPrefs* pEditorPrefs = m_pEngineCore->GetEditorPrefs();

            if( pEditorPrefs->Get_Aspect_GameAspectRatio() == GLView_Full )
            {
            }
            else if( pEditorPrefs->Get_Aspect_GameAspectRatio() == GLView_Tall )
            {
                if( w - h/1.5f >= 0 )
                {
                    x = (uint32)( (w - h/1.5f) / 2.0f );
                    w = (uint32)( h/1.5f );
                }
                else
                {
                }
            }
            else if( pEditorPrefs->Get_Aspect_GameAspectRatio() == GLView_Square )
            {
                if( w < h )
                {
                    y = (h-w)/2;
                    h = w;
                }
                else
                {
                    x = (w-h)/2;
                    w = h;
                }
            }
            else if( pEditorPrefs->Get_Aspect_GameAspectRatio() == GLView_Wide )
            {
                if( h - w/1.5f >= 0 )
                {
                    y = (uint32)( ( h - w/1.5f ) / 2.0f );
                    h = (uint32)( w/1.5f );
                }
                else
                {
                }
            }

            // Reset the viewport sizes of the game or editor cameras.
            ComponentSystemManager* pComponentSystemManager = pEngineCore->GetComponentSystemManager();
            if( pComponentSystemManager )
            {
                pComponentSystemManager->OnSurfaceChanged( x, y, w, h, (uint32)m_pEngineCore->m_GameWidth, (uint32)m_pEngineCore->m_GameHeight );
                pComponentSystemManager->OnDrawFrame();
            }

            g_pRenderer->BindFramebuffer( previousFBO );

            g_GLStats.EndCanvasFrame();
        }
    }

    if( m_EditorWindowVisible && m_EditorWindowSize.LengthSquared() != 0 )
    {
        if( m_pEditorFBO->GetColorTexture( 0 ) )
        {
            EditorPrefs* pEditorPrefs = m_pEngineCore->GetEditorPrefs();

            g_GLStats.NewCanvasFrame( 1 );

            // Draw editor view.
            g_GLCanvasIDActive = 1;
            pEngineCore->Editor_OnSurfaceChanged( 0, 0, (unsigned int)m_EditorWindowSize.x, (unsigned int)m_EditorWindowSize.y );

            uint32 previousFBO = g_GLStats.m_CurrentFramebuffer;
            m_pEditorFBO->Bind( false );
            m_pEngineCore->GetEditorState()->GetEditorCamera()->SetDeferred( pEditorPrefs->Get_View_EditorCamDeferred() );
            pEngineCore->GetCurrentEditorInterface()->OnDrawFrame( 1 );
            g_pRenderer->BindFramebuffer( previousFBO );

            g_GLCanvasIDActive = 0;

            g_GLStats.EndCanvasFrame();
        }
    }

    // Render material preview onto a sphere, if requested.
    if( m_pMaterialToPreview != nullptr && m_pMaterialToPreview->GetPreviewType() == MaterialDefinition::PreviewType_Sphere )
    {
        if( m_pMaterialPreviewFBO->GetColorTexture( 0 ) )
        {
            MyMesh* pMeshBall = m_pEngineCore->GetMesh_MaterialBall();

            if( pMeshBall )
            {
                m_pMaterialPreviewFBO->Bind( true );

                MyViewport viewport( 0, 0, m_pMaterialPreviewFBO->GetWidth(), m_pMaterialPreviewFBO->GetHeight() );
                g_pRenderer->EnableViewport( &viewport, true );

                g_pRenderer->SetClearColor( ColorFloat( 0.0f, 0.0f, 0.2f, 1.0f ) );
                g_pRenderer->ClearBuffers( true, true, false );

                pMeshBall->SetMaterial( m_pMaterialToPreview, 0 );

                MyMatrix matview;
                matview.SetIdentity();
#if MYFW_RIGHTHANDED
                matview.Translate( 0, 0, -4 );
#else
                matview.Translate( 0, 0, 4 );
#endif

                float aspect = (float)m_pMaterialPreviewFBO->GetWidth() / m_pMaterialPreviewFBO->GetHeight();
                MyMatrix matproj;
                matproj.CreatePerspectiveVFoV( 45, aspect, 0.01f, 100 );

                MyMatrix matviewproj = matproj * matview;
                Vector3 campos = matview.GetTranslation() * -1;
                Vector3 camrot( 0, 0, 0 );

                float time = (float)MyTime_GetRunningTime();

                // Create 2 rotating lights for material render.
                MyLight light1;
                light1.m_Attenuation.Set( 5, 0.1f, 0.01f ); // x is a radius/range, yz not used.
                light1.m_Color.Set( 1, 1, 1, 1 );
                light1.m_LightType = LightType_Point;
                light1.m_Position.Set( 2*cos(time), 1, 2*sin(time) );

                MyLight light2;
                light2.m_Attenuation.Set( 5, 0.1f, 0.01f ); // x is a radius/range, yz not used.
                light2.m_Color.Set( 1, 1, 1, 1 );
                light2.m_LightType = LightType_Point;
                light2.m_Position.Set( 2*cos(PI+time), 1, 2*sin(PI+time) );

                MyLight* lights[] = { &light1, &light2 };
                pMeshBall->Draw( &matproj, &matview, nullptr, &campos, &camrot, lights, 2, nullptr, nullptr, nullptr, nullptr );

                // Unset the material to avoid holding a ref that prevents the material from unloading.
                pMeshBall->SetMaterial( nullptr, 0 );

                m_pMaterialPreviewFBO->Unbind( true );
            }

            // Unset the material to preview, it will be reset every frame by the code that needs it.
            m_pMaterialToPreview = nullptr;
        }
    }
}

void EditorMainFrame_ImGui::EditMaterial(MaterialDefinition* pMaterial)
{
    m_pMaterialBeingEdited = pMaterial;
    ImGui::SetWindowFocus( "Material Editor" );
    m_pCurrentLayout->m_IsWindowOpen[EditorWindow_MaterialEditor] = true;
}

void EditorMainFrame_ImGui::Edit2DAnimInfo(My2DAnimInfo* pAnimInfo)
{
    m_p2DAnimInfoBeingEdited = pAnimInfo;
    ImGui::SetWindowFocus( "2D Animation Editor" );
    m_pCurrentLayout->m_IsWindowOpen[EditorWindow_2DAnimationEditor] = true;
    m_Current2DAnimationIndex = 0;
}

void EditorMainFrame_ImGui::AddInlineMaterial(MaterialDefinition* pMaterial)
{
    // Show Exposed Uniforms.
    if( pMaterial->m_pShaderGroup )
    {
        MyFileObjectShader* pShaderFile = pMaterial->m_pShaderGroup->GetFile();
        if( pShaderFile )
        {
            for( unsigned int i=0; i<pShaderFile->m_NumExposedUniforms; i++ )
            {
                char tempname[32];

                // Hardcoding to remove the 'u_' I like to stick at the start of my uniform names.
                if( pShaderFile->m_ExposedUniforms[i].m_Name[1] == '_' )
                    strcpy_s( tempname, 32, &pShaderFile->m_ExposedUniforms[i].m_Name[2] );
                else
                    strcpy_s( tempname, 32, pShaderFile->m_ExposedUniforms[i].m_Name );

                switch( pShaderFile->m_ExposedUniforms[i].m_Type )
                {
                case ExposedUniformType_Float:
                    ImGui::DragFloat( tempname, &pMaterial->m_UniformValues[i].m_Float, 0.01f, 0, 1 );
                    break;

                case ExposedUniformType_Vec2:
                    ImGui::DragFloat2( tempname, pMaterial->m_UniformValues[i].m_Vec2, 0.01f, 0, 1 );
                    break;

                case ExposedUniformType_Vec3:
                    ImGui::DragFloat3( tempname, pMaterial->m_UniformValues[i].m_Vec3, 0.01f, 0, 1 );
                    break;

                case ExposedUniformType_Vec4:
                    ImGui::DragFloat4( tempname, pMaterial->m_UniformValues[i].m_Vec4, 0.01f, 0, 1 );
                    break;

                case ExposedUniformType_ColorByte:
                    {
                        ColorByte* pColorByte = (ColorByte*)pMaterial->m_UniformValues[i].m_ColorByte;
                        ColorFloat colorFloat = pColorByte->AsColorFloat();
                        if( ImGui::ColorEdit4( "Ambient Color", &colorFloat.r ) )
                        {
                            pColorByte->SetFromColorFloat( colorFloat );
                        }
                    }
                    break;

                case ExposedUniformType_Sampler2D:
                    {
                        const char* desc = "no texture";

                        EditorPrefs* pEditorPrefs = m_pEngineCore->GetEditorPrefs();

                        Vector4 buttonColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_UnsetObjectButton );
                        Vector4 textColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_UnsetObjectText );

                        TextureDefinition* pTextureColor = pMaterial->m_UniformValues[i].m_pTexture;
                        if( pTextureColor )
                        {
                            desc = pTextureColor->GetFilename();

                            buttonColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Button );
                            textColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Text );
                        }

                        ImGui::PushStyleColor( ImGuiCol_Button, buttonColor );
                        ImGui::PushStyleColor( ImGuiCol_Text, textColor );
                        ImGui::Button( desc, ImVec2( ImGui::GetWindowWidth() * 0.65f, 0 ) );
                        ImGui::PopStyleColor( 2 );

                        if( ImGui::BeginDragDropTarget() )
                        {
                            if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "Texture" ) )
                            {
                                TextureDefinition* pNewTexture = (TextureDefinition*)*(void**)payload->Data;
                                if( pNewTexture )
                                    pNewTexture->AddRef();
                                SAFE_RELEASE( pMaterial->m_UniformValues[i].m_pTexture );
                                pMaterial->m_UniformValues[i].m_pTexture = pNewTexture;
                            }
                            ImGui::EndDragDropTarget();
                        }

                        if( ImGui::IsItemHovered() )
                        {
                            if( pTextureColor )
                            {
                                ImGui::BeginTooltip();
                                //ImGui::Text( "%s", pTex->GetFilename() );
                                AddTexturePreview( pTextureColor, false, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 1 ) );
                                ImGui::EndTooltip();
                            }

                            if( ImGui::IsMouseDoubleClicked( 0 ) )
                            {
                                SAFE_RELEASE( pMaterial->m_UniformValues[i].m_pTexture );
                                pMaterial->m_UniformValues[i].m_pTexture = 0;
                            }
                        }

                        ImGui::SameLine();
                        ImGui::Text( tempname );
                    }
                    break;

                case ExposedUniformType_NotSet:
                default:
                    MyAssert( false );
                    break;
                }
            }
        }
    }
}

My2DAnimInfo* EditorMainFrame_ImGui::Get2DAnimInfoBeingEdited()
{
    if( m_pCurrentLayout && m_pCurrentLayout->m_IsWindowOpen[EditorWindow_2DAnimationEditor] )
    {
        return m_p2DAnimInfoBeingEdited;
    }

    return nullptr;
}

void EditorMainFrame_ImGui::SetFullPathToLast2DAnimInfoBeingEdited(const char* fullPath)
{
    strcpy_s( m_FullPathToLast2DAnimInfoBeingEdited, MAX_PATH, fullPath );
}

//====================================================================================================
// Internal methods
//====================================================================================================

void EditorMainFrame_ImGui::StartRenameOp(GameObject* pGameObject, MaterialDefinition* pMaterial, const char* name)
{
    m_pGameObjectWhoseNameIsBeingEdited = pGameObject;
    m_pMaterialWhoseNameIsBeingEdited = pMaterial;
    strncpy_s( m_NameBeingEdited, 100, name, 99 );

    m_ConfirmCurrentRenameOp = false;
}

bool EditorMainFrame_ImGui::WasItemSlowDoubleClicked(void* pObjectClicked)
{
    MyAssert( pObjectClicked != nullptr );

    if( ImGui::IsItemClicked( 0 ) )
    {
        if( ImGuiExt::IsMouseHoveringOverItemExpansionArrow() )
        {
            // Clear the timer if the arrow was clicked.
            m_RenameTimerForSlowDoubleClick = 9999.0f;
        }
        else if( ImGui::IsMouseDoubleClicked( 0 ) )
        {
            // Clear the timer if there was a double-click.
            m_RenameTimerForSlowDoubleClick = 9999.0f;
        }
        else if( pObjectClicked != m_RenameOp_LastObjectClicked )
        {
            // Restart the timer if a different object was selected.
            m_RenameTimerForSlowDoubleClick = 0.0f;
            m_RenameOp_LastObjectClicked = pObjectClicked;
        }
        else
        {
            // If we're in the slow double click window, return true.
            if( m_RenameTimerForSlowDoubleClick > ImGui::GetIO().MouseDoubleClickTime && m_RenameTimerForSlowDoubleClick < 1.0f )
            {
                m_RenameTimerForSlowDoubleClick = 0.0f;
                return true;
            }

            // Restart the timer.
            m_RenameTimerForSlowDoubleClick = 0.0f;
        }
    }

    return false;
}

void EditorMainFrame_ImGui::AddMenuItemWithHotkeyCheck(const char* string, HotkeyAction action, bool selected)
{
    EditorKeyBindings* pKeys = m_pEngineCore->GetEditorPrefs()->GetKeyBindings();

    if( ImGui::MenuItem( string, pKeys->GetStringForKey( action ), selected ) )
    {
        ExecuteHotkeyAction( action );
    }
}

void EditorMainFrame_ImGui::AddMainMenuBar()
{
    EditorPrefs* pEditorPrefs = m_pEngineCore->GetEditorPrefs();

    bool wasInEditorMode = true;

    if( m_pEngineCore->IsInEditorMode() == false )
    {
        Vector4 gameRunningMenuBarColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_GameRunningMenuBarColor );
        ImGui::PushStyleColor( ImGuiCol_MenuBarBg, gameRunningMenuBarColor );

        wasInEditorMode = false;
    }

    if( ImGui::BeginMainMenuBar() )
    {
        if( ImGui::BeginMenu( "Scene" ) )
        {
            if( ImGui::MenuItem( "New Scene" ) )
            {
                if( m_pCommandStack->GetUndoStackSize() != m_UndoStackDepthAtLastSave )
                    m_ShowWarning_NewScene = true;
                else
                    EditorMenuCommand( EditorMenuCommand_File_NewScene );
            }
            if( ImGui::MenuItem( "Load Scene..." ) )
            {
                if( m_pCommandStack->GetUndoStackSize() != m_UndoStackDepthAtLastSave )
                    m_ShowWarning_LoadScene = true;
                else
                    EditorMenuCommand( EditorMenuCommand_File_LoadScene );
            }
            if( ImGui::BeginMenu( "Load Recent Scene..." ) )
            {
                uint32 numRecentScenes = pEditorPrefs->Get_File_NumRecentScenes();
                if( numRecentScenes == 0 )
                {
                    ImGui::Text( "no recent scenes..." );
                }

                for( uint32 i=0; i<numRecentScenes; i++ )
                {
                    std::string recentFilename = pEditorPrefs->Get_File_RecentScene( i );
                    if( ImGui::MenuItem( recentFilename.c_str() ) )
                    {
                        if( m_pCommandStack->GetUndoStackSize() != m_UndoStackDepthAtLastSave )
                        {
                            m_ShowWarning_LoadScene = true;
                            m_SceneToLoadAfterWarning = recentFilename;
                        }
                        else
                        {
                            // TODO: Handle files that no longer exist.
                            EditorMenuCommand( EditorMenuCommand_File_LoadPreselectedScene, recentFilename );
                        }
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if( ImGui::MenuItem( "Create Additional Scene" ) )
            {
                EditorMenuCommand( EditorMenuCommand_File_CreateAdditionalScene );
            }
            if( ImGui::MenuItem( "Load Additional Scene..." ) )
            {
                EditorMenuCommand( EditorMenuCommand_File_LoadAdditionalScene );
            }

            ImGui::Separator();

            AddMenuItemWithHotkeyCheck( "Save Scene", HotkeyAction::File_SaveScene );
            if( ImGui::MenuItem( "Save Scene As..." ) )
            {
                EditorMenuCommand( EditorMenuCommand_File_SaveSceneAs );
            }
            AddMenuItemWithHotkeyCheck( "Save All", HotkeyAction::File_SaveAll );

            ImGui::Separator();

            if( ImGui::BeginMenu( "Export" ) )
            {
                AddMenuItemWithHotkeyCheck( "Box2D Scene...", HotkeyAction::File_ExportBox2D );
                ImGui::EndMenu();
            }

            ImGui::Separator();

            AddMenuItemWithHotkeyCheck( "Preferences...", HotkeyAction::File_Preferences );

            if( ImGui::MenuItem( "Quit" ) ) { RequestCloseWindow(); }

            ImGui::EndMenu(); // "Scene"
        }

        // Add a menu for editor documents.
        EditorDocument* pNewDocument = EditorDocument::AddDocumentMenu( m_pEngineCore, m_pLastActiveDocument );
        if( pNewDocument != nullptr )
        {
            m_pEngineCore->GetEditorState()->OpenDocument( pNewDocument );
        }

        if( ImGui::BeginMenu( "Edit" ) )
        {
            AddMenuItemWithHotkeyCheck( "Undo", HotkeyAction::Edit_Undo );
            AddMenuItemWithHotkeyCheck( "Redo", HotkeyAction::Edit_Redo );

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "View" ) )
        {
            //if( ImGui::MenuItem( "Save window layout (TODO)" ) ) {} // { EditorMenuCommand( myID_View_SavePerspective ); }
            //if( ImGui::MenuItem( "Load window layout (TODO)" ) ) {} // { EditorMenuCommand( myID_View_LoadPerspective ); }
            if( ImGui::MenuItem( "Reset window layout" ) ) { m_pLayoutManager->ResetCurrentLayout(); };

            if( ImGui::BeginMenu( "Editor Windows" ) )
            {
                for( int i=0; i<EditorWindow_NumTypes; i++ )
                {
                    if( ImGui::MenuItem( g_DefaultEditorWindowTypeMenuLabels[i] ) )
                    {
                        m_pCurrentLayout->m_IsWindowOpen[i] = true;
                    }

                    // Add a separator between main window, secondary window and debug window groups.
                    if( i == 5 || i == 8 )
                    {
                        ImGui::Separator();
                    }
                }

                //for( int i=0; i<EngineEditorWindow_NumTypes; i++ )
                //{
                //    if( ImGui::MenuItem( g_DefaultEngineEditorWindowTypeMenuLabels[i] ) ) {}
                //}

                ImGui::EndMenu();
            }

            if( ImGui::BeginMenu( "Editor Window Layouts" ) )
            {
                for( unsigned int i=0; i<EditorLayout_NumLayouts; i++ )
                {
                    if( ImGui::MenuItem( g_EditorLayoutMenuLabels[i], nullptr, m_pLayoutManager->GetSelectedLayout_EditorMode() == (EditorLayoutTypes)i ) )
                    {
                        m_pLayoutManager->SetSelectedLayout_EditorMode( (EditorLayoutTypes)i );

                        if( m_pEngineCore->IsInEditorMode() == true )
                        {
                            m_pLayoutManager->RequestEditorLayout();
                        }
                    }
                }
                ImGui::EndMenu();
            }

            if( ImGui::BeginMenu( "Gameplay Window Layouts" ) )
            {
                for( int i=0; i<EditorLayout_NumLayouts; i++ )
                {
                    if( ImGui::MenuItem( g_EditorLayoutMenuLabels[i], nullptr, m_pLayoutManager->GetSelectedLayout_GameMode() == (EditorLayoutTypes)i ) )
                    {
                        m_pLayoutManager->SetSelectedLayout_GameMode( (EditorLayoutTypes)i );

                        if( m_pEngineCore->IsInEditorMode() == false )
                        {
                            m_pLayoutManager->RequestGameLayout();
                        }
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if( ImGui::BeginMenu( "Selected Objects" ) )
            {
                if( ImGui::MenuItem( "Show Wireframe", 0, pEditorPrefs->Get_View_SelectedObjects_ShowWireframe() ) ) { EditorMenuCommand( EditorMenuCommand_View_SelectedObjects_ShowWireframe ); }
                if( ImGui::MenuItem( "Show Effect", 0, pEditorPrefs->Get_View_SelectedObjects_ShowEffect() ) ) { EditorMenuCommand( EditorMenuCommand_View_SelectedObjects_ShowEffect ); }

                ImGui::EndMenu();
            }

            if( ImGui::BeginMenu( "Editor Camera" ) )
            {
                AddMenuItemWithHotkeyCheck( "Show Properties", HotkeyAction::View_ShowEditorCamProperties );
                AddMenuItemWithHotkeyCheck( "Show Icons", HotkeyAction::View_ShowEditorIcons, pEditorPrefs->Get_View_ShowEditorIcons() );
                AddMenuItemWithHotkeyCheck( "Deferred Render", HotkeyAction::View_ToggleEditorCamDeferred, pEditorPrefs->Get_View_EditorCamDeferred() );
                if( ImGui::MenuItem( "Show Deferred G-Buffer", "" ) ) { EditorMenuCommand( EditorMenuCommand_View_ShowEditorCamDeferredGBuffer ); }

                if( ImGui::BeginMenu( "Editor Camera Layers (TODO)" ) )
                {
                    for( int i=0; i<g_NumberOfVisibilityLayers; i++ )
                    {
                        char label[100];
                        char hotkey[100];
                        sprintf_s( label, 100, "(%d) %s", i, g_pVisibilityLayerStrings[i] );
                        sprintf_s( hotkey, 100, "Ctrl-Alt-%d", i );
                        if( i < 9 )
                        {
                            if( ImGui::MenuItem( label, hotkey ) ) {}
                        }
                        else
                        {
                            if( ImGui::MenuItem( label ) ) {}
                        }
                    }
                    ImGui::EndMenu();
                }
                //    {
                //        m_SubMenu_View_EditorCameraLayers = MyNew wxMenu;
                //        for( int i=0; i<g_NumberOfVisibilityLayers; i++ )
                //        {
                //            wxString label;
                //            if( i < 9 )
                //                label << "(&" << i+1 << ") " << g_pVisibilityLayerStrings[i] << "\tCtrl-Alt-" << i+1;
                //            else
                //                label << "(&" << i+1 << ") " << g_pVisibilityLayerStrings[i];
                //            m_MenuItem_View_EditorCameraLayerOptions[i] = m_SubMenu_View_EditorCameraLayers->AppendCheckItem( EditorMenuCommand_View_EditorCameraLayer + i, label, wxEmptyString );
                //            Connect( EditorMenuCommand_View_EditorCameraLayer + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EngineMainFrame::OnMenu_Engine) );
                //        }
                //    
                //        m_MenuItem_View_EditorCameraLayerOptions[0]->Check();
                //    }

                ImGui::EndMenu();
            }

            ImGui::Separator();

            ImGui::MenuItem( "Fullscreen Editor (TODO)", "F11" );
            ImGui::MenuItem( "Fullscreen Game (TODO)", "Ctrl-F11" );

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Aspect" ) )
        {
            unsigned int w = (unsigned int)m_EditorWindowSize.x;
            unsigned int h = (unsigned int)m_EditorWindowSize.y;

            EditorPrefs* pEditorPrefs = m_pEngineCore->GetEditorPrefs();

            AddMenuItemWithHotkeyCheck( "Fill",   HotkeyAction::View_Full, pEditorPrefs->Get_Aspect_GameAspectRatio() == GLView_Full );
            AddMenuItemWithHotkeyCheck( "Tall",   HotkeyAction::View_Tall, pEditorPrefs->Get_Aspect_GameAspectRatio() == GLView_Tall );
            AddMenuItemWithHotkeyCheck( "Square", HotkeyAction::View_Square, pEditorPrefs->Get_Aspect_GameAspectRatio() == GLView_Square );
            AddMenuItemWithHotkeyCheck( "Wide",   HotkeyAction::View_Wide, pEditorPrefs->Get_Aspect_GameAspectRatio() == GLView_Wide );

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Grid" ) )
        {
            AddMenuItemWithHotkeyCheck( "Grid Visible", HotkeyAction::Grid_Visible, pEditorPrefs->Get_Grid_Visible() );
            AddMenuItemWithHotkeyCheck( "Grid Snap Enabled", HotkeyAction::Grid_SnapEnabled, pEditorPrefs->Get_Grid_SnapEnabled() );
            AddMenuItemWithHotkeyCheck( "Grid Settings", HotkeyAction::Grid_Settings );

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Mode" ) )
        {
            if( ImGui::MenuItem( "Switch Focus on Play/Stop", nullptr, pEditorPrefs->Get_Mode_SwitchFocusOnPlayStop() ) ) { EditorMenuCommand( EditorMenuCommand_Mode_SwitchFocusOnPlayStop ); }
            // Since Command-Space is "Spotlight Search" on OSX, use the actual control key on OSX as well as Windows/Linux.
            AddMenuItemWithHotkeyCheck( "Play/Stop", HotkeyAction::Mode_TogglePlayStop );
            AddMenuItemWithHotkeyCheck( "Pause", HotkeyAction::Mode_Pause );
            AddMenuItemWithHotkeyCheck( "Advance 1 Frame", HotkeyAction::Mode_AdvanceOneFrame );
            AddMenuItemWithHotkeyCheck( "Advance 1 Second", HotkeyAction::Mode_AdvanceOneSecond );

            if( ImGui::BeginMenu( "Launch Platforms" ) )
            {
                for( int i=0; i<LaunchPlatform_NumPlatforms; i++ )
                {
                    bool isSelected = (pEditorPrefs->Get_Mode_LaunchPlatform() == i);
                    if( ImGui::MenuItem( g_LaunchPlatformsMenuLabels[i], nullptr, isSelected ) ) { EditorMenuCommand( (EditorMenuCommands)(EditorMenuCommand_Mode_LaunchPlatforms + i) ); }
                }

                ImGui::EndMenu();
            }

            AddMenuItemWithHotkeyCheck( "Launch Game", HotkeyAction::Mode_LaunchGame );

            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Data" ) )
        {
            if( ImGui::MenuItem( "Load Datafiles" ) ) { EditorMenuCommand( EditorMenuCommand_Data_LoadDatafiles ); }
            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Hackery" ) )
        {
            if( ImGui::MenuItem( "Record (TODO)", "Ctrl-R" ) ) {} // { EditorMenuCommand( EditorMenuCommand_Hackery_RecordMacro ); }
            if( ImGui::MenuItem( "Stop recording and Execute (TODO)", "Ctrl-E" ) ) {} // { EditorMenuCommand( EditorMenuCommand_Hackery_ExecuteMacro ); }
            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Debug views" ) )
        {
            if( ImGui::MenuItem( "Show Animated Debug View for Selection (TODO)", "F8" ) ) {} // { EditorMenuCommand( EditorMenuCommand_Debug_ShowSelectedAnimatedMesh ); }
            if( ImGui::MenuItem( "Show GL Stats (TODO)", "Shift-F9" ) ) {} // { EditorMenuCommand( EditorMenuCommand_Debug_ShowGLStats ); }
            AddMenuItemWithHotkeyCheck( "Draw Wireframe", HotkeyAction::Debug_DrawWireframe, m_pEngineCore->GetDebug_DrawWireframe() );
            AddMenuItemWithHotkeyCheck( "Show Physics Debug Shapes", HotkeyAction::Debug_ShowPhysicsShapes, pEditorPrefs->Get_Debug_DrawPhysicsDebugShapes() );
            AddMenuItemWithHotkeyCheck( "Show Basic Stats", HotkeyAction::Debug_ShowStats, pEditorPrefs->Get_Debug_DrawStats() );
            if( ImGui::MenuItem( "Show profiling Info (TODO)", "Ctrl-F8" ) ) {} // { EditorMenuCommand( EditorMenuCommand_Debug_ShowProfilingInfo ); }
            ImGui::EndMenu();
        }

        if( ImGui::BeginMenu( "Lua" ) )
        {
            AddMenuItemWithHotkeyCheck( "Run Lua Script...", HotkeyAction::Lua_RunLuaScript );
            if( pEditorPrefs->Get_Lua_NumRecentScripts() > 0 )
            {
                int fileIndex = 0;
                if( ImGui::MenuItem( pEditorPrefs->Get_Lua_RecentScript( fileIndex ).c_str() ) ) { EditorMenuCommand( (EditorMenuCommands)(EditorMenuCommand_Lua_RunRecentLuaScript + fileIndex) ); }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if( wasInEditorMode == false )
    {
        ImGui::PopStyleColor();
    }
}

void EditorMainFrame_ImGui::AddLoseChangesWarningPopups()
{
    if( m_ShowWarning_NewScene )
    {
        m_ShowWarning_NewScene = false;
        ImGui::OpenPopup( "New Scene Warning" );
    }
    if( ImGui::BeginPopupModal( "New Scene Warning" ) )
    {
        ImGui::Text( "Some changes aren't saved." );
        ImGui::Dummy( ImVec2( 0, 10 ) );
        
        if( ImGui::Button( "Create new scene anyway / Lose changes" ) )
        {
            EditorMenuCommand( EditorMenuCommand_File_NewScene );
            ImGui::CloseCurrentPopup();
        }

        if( ImGui::Button( "Cancel / Return to editor" ) )
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if( m_ShowWarning_LoadScene )
    {
        m_ShowWarning_LoadScene = false;
        ImGui::OpenPopup( "Load Scene Warning" );
    }
    if( ImGui::BeginPopupModal( "Load Scene Warning" ) )
    {
        ImGui::Text( "Some changes aren't saved." );
        ImGui::Dummy( ImVec2( 0, 10 ) );
        
        if( ImGui::Button( "Load anyway / Lose changes" ) )
        {
            if( m_SceneToLoadAfterWarning.empty() == true )
            {
                EditorMenuCommand( EditorMenuCommand_File_LoadScene );
            }
            else
            {
                EditorMenuCommand( EditorMenuCommand_File_LoadPreselectedScene, m_SceneToLoadAfterWarning );
            }
            ImGui::CloseCurrentPopup();
        }

        if( ImGui::Button( "Cancel / Return to editor" ) )
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if( m_ShowWarning_CloseDocument )
    {
        m_ShowWarning_CloseDocument = false;
        ImGui::OpenPopup( "Close Document Warning" );
    }
    if( ImGui::BeginPopupModal( "Close Document Warning" ) )
    {
        ImGui::Text( "Some changes aren't saved." );
        ImGui::Dummy( ImVec2( 0, 10 ) );
        
        if( ImGui::Button( "Close / Lose changes" ) )
        {
            m_pActiveDocument = nullptr;
            m_pLastActiveDocument = nullptr;

            std::vector<EditorDocument*>* pOpenDocuments = &m_pEngineCore->GetEditorState()->m_pOpenDocuments;
            delete (*pOpenDocuments)[m_DocumentIndexToCloseAfterWarning];
            pOpenDocuments->erase( pOpenDocuments->begin() + m_DocumentIndexToCloseAfterWarning );

            ImGui::CloseCurrentPopup();
        }

        if( ImGui::Button( "Cancel / Return to editor" ) )
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if( m_ShowWarning_CloseEditor )
    {
        m_ShowWarning_CloseEditor = false;
        ImGui::OpenPopup( "Close Editor Warning" );
    }
    if( ImGui::BeginPopupModal( "Close Editor Warning" ) )
    {
        ImGui::Text( "Some changes aren't saved." );
        ImGui::Dummy( ImVec2( 0, 10 ) );
        
        if( ImGui::Button( "Quit / Lose changes" ) )
        {
            g_pGameCore->SetGameConfirmedCloseIsOkay();
        }

        if( ImGui::Button( "Cancel / Return to editor" ) )
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void EditorMainFrame_ImGui::AddGameAndEditorWindows()
{
    m_GameWindowFocused = m_GameWindowVisible = false;

    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Game] )
    {
        ImGui::SetNextWindowPos( ImVec2(9, 302), ImGuiCond_FirstUseEver );

        if( ImGui::Begin( "Game", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Game], ImVec2(256, 171) ) )
        {
            m_GameWindowFocused = ImGui::IsWindowFocused();
            m_GameWindowVisible = true;

            ImVec2 min = ImGui::GetWindowContentRegionMin();
            ImVec2 max = ImGui::GetWindowContentRegionMax();
            float w = max.x - min.x;
            float h = max.y - min.y;

            if( w <= 0 || h <= 0 )
            {
                m_GameWindowPos.Set( -1, -1 );
                m_GameWindowSize.Set( 0, 0 );
            }
            else
            {
                ImVec2 pos = ImGui::GetWindowPos();
                m_GameWindowPos.Set( pos.x + min.x, pos.y + min.y );
                m_GameWindowSize.Set( w, h );

                if( w > 4096 ) w = 4096;
                if( h > 4096 ) h = 4096;

                // This will resize our FBO if the window is larger than it ever was.
                m_pEngineCore->GetManagers()->GetTextureManager()->ReSetupFBO( m_pGameFBO, (unsigned int)w, (unsigned int)h, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, false );

                if( m_pGameFBO->GetColorTexture( 0 ) )
                {
                    TextureDefinition* tex = m_pGameFBO->GetColorTexture( 0 );
                    ImGui::ImageButton( (void*)tex, ImVec2( w, h ), ImVec2(0,h/m_pGameFBO->GetTextureHeight()), ImVec2(w/m_pGameFBO->GetTextureWidth(),0), 0 );
                }
            }
        }
        else
        {
            m_GameWindowVisible = false;
        }
        ImGui::End();
    }

    m_EditorWindowFocused = m_EditorWindowHovered = m_EditorWindowVisible = false;
    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Editor] )
    {
        ImGui::SetNextWindowPos( ImVec2(269, 24), ImGuiCond_FirstUseEver );
        ImGui::SetNextWindowSize( ImVec2(579, 397), ImGuiCond_FirstUseEver );

        if( ImGui::Begin( "Editor", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Editor] ) )
        {
            m_EditorWindowFocused = ImGui::IsWindowFocused();
            m_EditorWindowHovered = ImGui::IsWindowHovered( ImGuiHoveredFlags_AllowWhenBlockedByActiveItem );
            m_EditorWindowVisible = true;

            ImVec2 min = ImGui::GetWindowContentRegionMin();
            ImVec2 max = ImGui::GetWindowContentRegionMax();
            float w = max.x - min.x;
            float h = max.y - min.y;

            if( w <= 0 || h <= 0 )
            {
                m_EditorWindowPos.Set( -1, -1 );
                m_EditorWindowSize.Set( 0, 0 );
            }
            else
            {
                ImVec2 pos = ImGui::GetWindowPos();
                m_EditorWindowPos.Set( pos.x + min.x, pos.y + min.y );
                m_EditorWindowSize.Set( w, h );

                if( w > 4096 ) w = 4096;
                if( h > 4096 ) h = 4096;

                // This will resize our FBO if the window is larger than it ever was.
                m_pEngineCore->GetManagers()->GetTextureManager()->ReSetupFBO( m_pEditorFBO, (unsigned int)w, (unsigned int)h, MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, false );

                if( m_pEditorFBO->GetColorTexture( 0 ) )
                {
                    TextureDefinition* tex = m_pEditorFBO->GetColorTexture( 0 );
                    ImGui::ImageButton( (void*)tex, ImVec2( w, h ), ImVec2(0,h/m_pEditorFBO->GetTextureHeight()), ImVec2(w/m_pEditorFBO->GetTextureWidth(),0), 0 );

                    if( ImGui::BeginDragDropTarget() )
                    {
                        OnDropEditorWindow();
                        ImGui::EndDragDropTarget();
                    }
                }

                // Allow the current editor interface to add some overlay items to the window.
                {
                    ImGui::SetCursorPos( ImVec2( 8, 28 ) );
                    m_pEngineCore->GetCurrentEditorInterface()->AddImGuiOverlayItems();
                }
            }
        }
        else
        {
            m_EditorWindowVisible = false;
        }
        ImGui::End();
    }
}

void EditorMainFrame_ImGui::AddObjectList()
{
    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_ObjectList] == false )
        return;

    ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    ImGui::SetNextWindowPos( ImVec2(4, 28), ImGuiCond_FirstUseEver );
  
    if( ImGui::Begin( "Objects", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_ObjectList], ImVec2(258, 266) ) )
    {
        // Add an input box for object list filter.
        // For now, it will always auto-select the text when given focus.
        // TODO: Only auto-select when Ctrl-F is pressed, not if clicked by mouse.
        {
            ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_AutoSelectAll;
            if( m_SetObjectListFilterBoxInFocus )
            {
                //inputTextFlags |= ImGuiInputTextFlags_AutoSelectAll;
                ImGui::SetKeyboardFocusHere();
                m_SetObjectListFilterBoxInFocus = false;
            }
            ImGui::InputText( "Filter", m_ObjectListFilter, 100, inputTextFlags );
            ImGui::SameLine();
            if( ImGui::Button( "X" ) )
            {
                m_ObjectListFilter[0] = '\0';
            }
        }

        bool forceOpen = false;
        if( m_ObjectListFilter[0] != '\0' )
        {
            ImGui::PushID( "FilteredList" );
            forceOpen = true;
        }

        ImGui::BeginChild( "GameObject List" );

        if( ImGui::CollapsingHeader( "All scenes", ImGuiTreeNodeFlags_DefaultOpen ) || forceOpen )
        {
            // Add all active scenes.
            for( int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
            {
                // Show Unmanaged scene first.
                unsigned int sceneindex;
                if( i == 0 )
                    sceneindex = SCENEID_Unmanaged;
                else
                    sceneindex = i-1;

                SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( (SceneID)sceneindex );

                if( pSceneInfo->m_InUse == true )
                {
                    static char* pUnmanagedName = "Runtime";
                    static char* pUnsavedName = "Unsaved scene";
                    char* scenename = pUnmanagedName;
                    if( sceneindex != SCENEID_Unmanaged )
                        scenename = pUnsavedName;

                    if( pSceneInfo->m_FullPath[0] != '\0' )
                    {
                        int i;
                        for( i=(int)strlen(pSceneInfo->m_FullPath)-1; i>=0; i-- )
                        {
                            if( pSceneInfo->m_FullPath[i] == '\\' || pSceneInfo->m_FullPath[i] == '/' )
                                break;
                        }
                        scenename = &pSceneInfo->m_FullPath[i+1];
                    }

                    ImGuiTreeNodeFlags nodeFlags = baseNodeFlags;
                    //if( sceneindex != SCENEID_Unmanaged )
                        nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

                    if( forceOpen )
                    {
                        ImGui::SetNextTreeNodeOpen( true );
                    }

                    bool treeNodeIsOpen = ImGui::TreeNodeEx( scenename, nodeFlags );

                    // Right-click menu, don't show for the unmanaged scene.
                    if( sceneindex != SCENEID_Unmanaged )
                    {
                        ImGui::PushID( scenename );
                        if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                        {
                            AddContextMenuOptionsForCreatingGameObjects( nullptr, (SceneID)sceneindex );

                            if( ImGui::MenuItem( "Unload scene (TODO)" ) )
                            {
                                ImGui::CloseCurrentPopup();
                            }

                            ImGui::EndPopup();
                        }
                        ImGui::PopID();
                    }

                    if( treeNodeIsOpen )
                    {
                        // Add GameObjects that are in root.
                        GameObject* pGameObject = pSceneInfo->m_GameObjects.GetHead();
                        while( pGameObject )
                        {
                            // Add GameObjects, their children and their components.
                            AddGameObjectToObjectList( pGameObject, nullptr );

                            pGameObject = pGameObject->GetNext();
                        }
                        ImGui::TreePop();
                    }
                }
            }
        }

        AddPrefabFiles( forceOpen );

        float cursorScreenPositionY = ImGui::GetCursorScreenPos().y;
        float mousePositionY = ImGui::GetMousePos().y;

        ImGui::EndChild();

        // Clear the "reorder" line each frame.
        // Done here after all gameobjects and the reorder line were drawn.
        m_pGameObjectToDrawReorderLineAfter = nullptr;

        // If GameObject is moved into blank area of object list, move it as last object in last scene.
        if( mousePositionY > cursorScreenPositionY )
        {
            // Find the last scene.
            int sceneIndex = 0;
            for( sceneIndex=MAX_SCENES_LOADED-1; sceneIndex>=0; sceneIndex-- )
            {
                if( g_pComponentSystemManager->GetSceneInfo( (SceneID)sceneIndex )->m_InUse )
                    break;
            }

            // Check for dropped object and pretend we're hovering over last gameobject in that scene.
            if( sceneIndex >= 0 )
            {
                SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( (SceneID)sceneIndex );
                GameObject* pGameObject = pSceneInfo->m_GameObjects.GetTail();
                if( pGameObject )
                {
                    OnDropObjectList( pGameObject, true );
                }
            }
        }

        if( m_ObjectListFilter[0] != '\0' )
        {
            ImGui::PopID(); // "FilteredList"
        }
    }
    ImGui::End();
}

void EditorMainFrame_ImGui::AddPrefabFiles(bool forceOpen)
{
    ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    unsigned int numPrefabFiles = g_pComponentSystemManager->m_pPrefabManager->GetNumberOfFiles();
    if( numPrefabFiles > 0 )
    {
        if( ImGui::CollapsingHeader( "Prefab Files", ImGuiTreeNodeFlags_DefaultOpen ) || forceOpen )
        {
            for( unsigned int prefabFileIndex=0; prefabFileIndex<numPrefabFiles; prefabFileIndex++ )
            {
                ImGuiTreeNodeFlags nodeFlags = baseNodeFlags;
                nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

                if( forceOpen )
                {
                    ImGui::SetNextTreeNodeOpen( true );
                }

                PrefabFile* pPrefabFile = g_pComponentSystemManager->m_pPrefabManager->GetLoadedPrefabFileByIndex( prefabFileIndex );
                MyAssert( pPrefabFile != nullptr );

                const char* pPrefabFilename = pPrefabFile->GetFile()->GetFilename();
                MyAssert( pPrefabFilename != nullptr );

                bool treeNodeIsOpen = ImGui::TreeNodeEx( pPrefabFilename, nodeFlags );

                ImGui::PushID( pPrefabFilename );
                if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                {
                    //if( ImGui::MenuItem( "Add GameObject" ) )
                    //{
                    //    pGameObjectCreated = g_pComponentSystemManager->CreateGameObject( true, (SceneID)sceneindex );
                    //    pGameObjectCreated->SetName( "New Game Object" );

                    //    ImGui::CloseCurrentPopup();
                    //}

                    ImGui::EndPopup();
                }
                ImGui::PopID();

                if( treeNodeIsOpen )
                {
                    // Add Prefab objects from this file.
                    PrefabObject* pPrefab = pPrefabFile->GetFirstPrefab();
                    while( pPrefab )
                    {
                        GameObject* pGameObject = pPrefab->GetGameObject();
                        
                        if( pGameObject )
                        {
                            // Add GameObjects, their children and their components.
                            AddGameObjectToObjectList( pGameObject, pPrefab );
                        }

                        pPrefab = (PrefabObject*)pPrefab->GetNext();
                    }
                    
                    ImGui::TreePop();
                }
            }
        }
    }
}

void EditorMainFrame_ImGui::AddGameObjectToObjectList(GameObject* pGameObject, PrefabObject* pPrefab)
{
    if( pGameObject->IsManaged() == false && pPrefab == nullptr )
        return;

    float startCursorPositionX = ImGui::GetCursorPosX();

    // If we're renaming the GameObject, show an edit box instead of a tree node.
    if( pGameObject == m_pGameObjectWhoseNameIsBeingEdited )
    {
        ImGui::PushID( pGameObject );
        ImGui::SetKeyboardFocusHere();
        
        bool confirmed = ImGui::InputText( "New name", m_NameBeingEdited, 100, ImGuiInputTextFlags_AutoSelectAll|ImGuiInputTextFlags_EnterReturnsTrue );

        // Any click will confirm the rename op, but cancel the confirmation if the click was in the edit box.
        if( ImGui::IsItemClicked() )
        {
            m_ConfirmCurrentRenameOp = false;
        }

        // If 'enter' was pressed or another mouse click confirmed the change.
        if( confirmed || m_ConfirmCurrentRenameOp )
        {
            m_pGameObjectWhoseNameIsBeingEdited->SetName( m_NameBeingEdited );
            m_pGameObjectWhoseNameIsBeingEdited = nullptr;
            m_RenamePressedThisFrame = false;
        }

        ImGui::PopID();
    }
    else
    {
        // If the object list filter has any characters in it.
        if( m_ObjectListFilter[0] != '\0' )
        {
            // Check if this GameObject's name contains the substrings.
            if( CheckIfMultipleSubstringsAreInString( pGameObject->GetName(), m_ObjectListFilter ) == false )
            {
                // Substrings weren't found, but might be in child GameObjects, so check/add those.
                GameObject* pChildGameObject = pGameObject->GetFirstChild();
                while( pChildGameObject )
                {
                    AddGameObjectToObjectList( pChildGameObject, pPrefab );
                    pChildGameObject = pChildGameObject->GetNext();
                }

                // Substrings weren't found, so kick out.
                return;
            }
        }

        EditorState* pEditorState = m_pEngineCore->GetEditorState();

        ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

        ImGuiTreeNodeFlags nodeFlags = baseNodeFlags;
        if( pEditorState->IsGameObjectSelected( pGameObject ) )
        {
            nodeFlags |= ImGuiTreeNodeFlags_Selected;
        }

        // If the gameobject is derived from a prefab or if it's disabled, change the color of the name.
        int pushedColors = 0;
        if( pGameObject->IsPrefabInstance() )
        {
            // TODO: put these colors into the preferences files.
            if( pGameObject->IsEnabled() )
                ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0, 0.8f, 0, 1 ) );
            else
                ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0, 0.5f, 0, 1 ) );

            pushedColors++;
        }
        else if( pGameObject->IsEnabled() == false )
        {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.5f, 0.5f, 0.5f, 1 ) );

            pushedColors++;
        }

        // Add the GameObject itself to the tree.
        char* icon = EditorIcon_GameObject;
        if( pGameObject->IsFolder() )
            icon = EditorIcon_Folder;
        if( pGameObject->IsPrefabInstance() )
            icon = EditorIcon_Prefab;

        bool treeNodeIsOpen = ImGui::TreeNodeEx( pGameObject, nodeFlags, "%s %s", icon, pGameObject->GetName() );

        ImGui::PopStyleColor( pushedColors );

        ImGui::PushID( pGameObject );

        // Start rename process on the first selected GameObject.
        // Since 'enter' is a rename key, make sure the object wasn't already being renamed.
        if( pEditorState->m_pSelectedObjects.size() > 0 )
        {
            if( ImGui::IsRootWindowOrAnyChildFocused() && m_RenamePressedThisFrame &&
                pEditorState->m_pSelectedObjects[0] != m_pGameObjectWhoseNameIsBeingEdited )
            {
                StartRenameOp( pEditorState->m_pSelectedObjects[0], nullptr, pEditorState->m_pSelectedObjects[0]->GetName() );
            }
        }

        if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
        {
            EditorState* pEditorState = m_pEngineCore->GetEditorState();

            // If the right-clicked object isn't selected, then unselect what is and select just this one.
            if( pEditorState->IsGameObjectSelected( pGameObject ) == false )
            {
                pEditorState->ClearSelectedObjectsAndComponents();
                pEditorState->SelectGameObject( pGameObject );
            }

            if( pPrefab != nullptr )
            {
                int numselected = (int)m_pEngineCore->GetEditorState()->m_pSelectedObjects.size();

                if( numselected > 1 )
                {
                    if( ImGui::MenuItem( "Delete prefabs" ) )
                    {
                        g_pComponentSystemManager->m_pPrefabManager->DeleteSelectedPrefabs();
                    }
                }
                else
                {
                    if( ImGui::MenuItem( "Delete prefab" ) )
                    {
                        g_pComponentSystemManager->m_pPrefabManager->DeleteSelectedPrefabs();
                    }
                }
            }
            else
            {
                int numselected = (int)m_pEngineCore->GetEditorState()->m_pSelectedObjects.size();

                // If multiple objects are selected, show these options first.
                if( numselected > 1 )
                {
                    if( ImGui::MenuItem( "Duplicate GameObjects (TODO)" ) )    { ImGui::CloseCurrentPopup(); }
                    if( ImGui::MenuItem( "Create Child GameObjects (TODO)" ) ) { ImGui::CloseCurrentPopup(); }
                    if( ImGui::MenuItem( "Delete GameObjects (TODO)" ) )       { ImGui::CloseCurrentPopup(); }
                    if( ImGui::MenuItem( "Clear Parents/Prefabs" ) )           { pGameObject->OnPopupClick( pGameObject, GameObject::RightClick_ClearParent );         ImGui::CloseCurrentPopup(); }
                    ImGui::Separator();
                    ImGui::Text( pGameObject->GetName() );
                }

                // Show options for the single item right-clicked.
                {
                    // Menu options to add new components to a GameObject, don't allow components to be added to a folder.
                    if( pGameObject->IsFolder() == false )
                    {
                        if( ImGui::BeginMenu( "Add Component" ) )
                        {
                            AddContextMenuOptionsForAddingComponents( pGameObject );
                            ImGui::EndMenu();
                        }
                    }

                    // Menu options to add child GameObjects.
                    if( ImGui::BeginMenu( "Add Child" ) )
                    {
                        AddContextMenuOptionsForCreatingGameObjects( pGameObject, pGameObject->GetSceneID() );
                        ImGui::EndMenu();
                    }

                    if( ImGui::MenuItem( "Duplicate GameObject" ) )    { pGameObject->OnPopupClick( pGameObject, GameObject::RightClick_DuplicateGameObject ); ImGui::CloseCurrentPopup(); }
                    if( ImGui::MenuItem( "Create Child GameObject" ) ) { pGameObject->OnPopupClick( pGameObject, GameObject::RightClick_CreateChild );         ImGui::CloseCurrentPopup(); }
                    if( pGameObject->GetGameObjectThisInheritsFrom() )
                    {
                        if( ImGui::MenuItem( "Clear Parent/Prefab" ) ) { pGameObject->OnPopupClick( pGameObject, GameObject::RightClick_ClearParent );         ImGui::CloseCurrentPopup(); }
                    }
                    if( ImGui::BeginMenu( "Create prefab in" ) )
                    {
                        unsigned int numprefabfiles = g_pComponentSystemManager->m_pPrefabManager->GetNumberOfFiles();
                        for( unsigned int i=0; i<numprefabfiles; i++ )
                        {
                            PrefabFile* pPrefabFile = g_pComponentSystemManager->m_pPrefabManager->GetLoadedPrefabFileByIndex( i );
                            MyFileObject* pFile = pPrefabFile->GetFile();
                            MyAssert( pFile != nullptr );

                            if( ImGui::MenuItem( pFile->GetFilenameWithoutExtension() ) )
                            {
                                pGameObject->OnPopupClick( pGameObject, GameObject::RightClick_CreatePrefab + i );

                                ImGui::CloseCurrentPopup();
                            }
                        }

                        if( ImGui::MenuItem( "New/Load Prefab file... (TODO)" ) )
                        {
                            //pGameObject->OnPopupClick( pGameObject, GameObject::RightClick_CreatePrefab + numprefabfiles );

                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::EndMenu();
                    }
                    if( ImGui::MenuItem( "Delete GameObject" ) )
                    {
                        // If the object isn't selected, delete just the one object, otherwise delete all selected objects.
                        if( pEditorState->IsGameObjectSelected( pGameObject ) )
                        {
                            pEditorState->DeleteSelectedObjects();
                        }
                        else
                        {
                            // Create a temp vector to pass into command.
                            std::vector<GameObject*> gameobjects;
                            gameobjects.push_back( pGameObject );
                            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_DeleteObjects( gameobjects ) );
                        }
                    }
                    if( ImGui::MenuItem( "Rename" ) )
                    {
                        StartRenameOp( pGameObject, nullptr, pGameObject->GetName() );
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
            ImGui::EndPopup();
        }

        // Handle start of dragging either GameObjects or Prefabs.
        if( ImGui::BeginDragDropSource() )
        {
            if( pPrefab != nullptr )
            {
                ImGui::SetDragDropPayload( "Prefab", &pPrefab, sizeof(pPrefab), ImGuiCond_Once );
            }
            else
            {
                ImGui::SetDragDropPayload( "GameObject", &pGameObject, sizeof(pGameObject), ImGuiCond_Once );
            }
            ImGui::Text( "%s", pGameObject->GetName() );
            ImGui::EndDragDropSource();
        }

        // Handle dropping things on GameObjects or Prefabs.
        bool dragDropPayloadAcceptedOnRelease = OnDropObjectList( pGameObject, false );

        ImGui::PopID(); // ImGui::PushID( pGameObject );

        bool hoveringOverArrow = ImGuiExt::IsMouseHoveringOverItemExpansionArrow();

        // Select the GameObject if it's clicked.
        // The code selects on mouse release so items can be dragged without changing selection.
        // Also handles Ctrl and Shift clicks for multiple selections.
        // Expand the item without selecting it if the arrow is clicked.
        if( ImGui::IsMouseReleased( 0 ) && ImGui::IsItemHovered() &&
            hoveringOverArrow == false && dragDropPayloadAcceptedOnRelease == false )
        {
            if( ImGui::GetIO().KeyShift == true )
            {
                // Select all GameObjects between last interacted object in list and this one.
                pEditorState->ClearSelectedObjectsAndComponents();

                SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( pGameObject->GetSceneID() );
                GameObject* pFirstGameObjectInScene = pSceneInfo->m_GameObjects.GetHead();

                // If there was no item previously interacted with, set it to the first object in the scene.
                if( m_pLastGameObjectInteractedWithInObjectPanel == nullptr )
                {
                    m_pLastGameObjectInteractedWithInObjectPanel = pFirstGameObjectInScene;
                }

                // Loop through the gameobjects moving forward looking for the selected one.
                // TODO: recurse through children (if visible?)
                // TODO: select all object inside folders?
                bool found = false;
                GameObject* pCurrentGameObject = m_pLastGameObjectInteractedWithInObjectPanel;
                while( true )
                {
                    pEditorState->SelectGameObject( pCurrentGameObject );

                    // If we hit the selected object.
                    if( pCurrentGameObject == pGameObject )
                    {
                        found = true;
                        break;
                    }

                    pCurrentGameObject = pCurrentGameObject->GetNext();
                    if( pCurrentGameObject == nullptr )
                        break;
                }

                // If we didn't find it going forward, clear the list and try in reverse.
                if( found == false )
                {
                    pEditorState->ClearSelectedObjectsAndComponents();
                    GameObject* pCurrentGameObject = m_pLastGameObjectInteractedWithInObjectPanel;
                    while( true )
                    {
                        pEditorState->SelectGameObject( pCurrentGameObject );

                        // If we hit the selected object.
                        if( pCurrentGameObject == pGameObject )
                        {
                            found = true;
                            break;
                        }

                        pCurrentGameObject = pCurrentGameObject->GetPrev();
                        if( pCurrentGameObject == nullptr )
                            break;
                    }
                }
            }
            else
            {
                // If control isn't held, clear the selected objects.
                if( ImGui::GetIO().KeyCtrl == false )
                {
                    pEditorState->ClearSelectedObjectsAndComponents();
                }

                // If there are any selected items and there's a mix of gameobjects and prefabs, then unselect all.
                if( pEditorState->m_pSelectedObjects.size() > 0 )
                {
                    bool gameObjectIsMasterPrefab = pGameObject->GetPrefabRef()->IsMasterPrefabGameObject();
                    bool firstSelectedGameObjectIsMasterPrefab = pEditorState->m_pSelectedObjects[0]->GetPrefabRef()->IsMasterPrefabGameObject();

                    if( gameObjectIsMasterPrefab == true && firstSelectedGameObjectIsMasterPrefab == false )
                        pEditorState->ClearSelectedObjectsAndComponents();

                    if( gameObjectIsMasterPrefab == false && firstSelectedGameObjectIsMasterPrefab == true )
                        pEditorState->ClearSelectedObjectsAndComponents();
                }

                if( pEditorState->IsGameObjectSelected( pGameObject ) )
                {
                    m_pLastGameObjectInteractedWithInObjectPanel = pGameObject;
                    pEditorState->UnselectGameObject( pGameObject );
                }
                else
                {
                    m_pLastGameObjectInteractedWithInObjectPanel = pGameObject;
                    pEditorState->SelectGameObject( pGameObject );

                    // If this is a folder and it's the only selection, then select all objects inside the folder as well.
                    if( pGameObject->IsFolder() && pEditorState->m_pSelectedObjects.size() == 1 )
                    {
                        for( GameObject* pChildGameObject = pGameObject->GetChildList()->GetHead(); pChildGameObject; pChildGameObject = pChildGameObject->GetNext() )
                        {
                            if( pChildGameObject->IsManaged() )
                            {
                                pChildGameObject->AddToList( &pEditorState->m_pSelectedObjects );
                            }
                        }
                    }
                }
            }
        }

        // Deal with slow double-click for renaming GameObjects.
        if( WasItemSlowDoubleClicked( pGameObject ) )
        {
            StartRenameOp( pGameObject, nullptr, pGameObject->GetName() );
        }

        bool forceOpen = false;
        if( treeNodeIsOpen == false && pEditorState->IsGameObjectAParentOfASelectedObjectOrComponent( pGameObject ) )
        {
            ImGui::Indent();
            forceOpen = true;
        }
        
        if( treeNodeIsOpen || forceOpen )
        {
            // Add Child GameObjects.
            GameObject* pChildGameObject = pGameObject->GetFirstChild();
            while( pChildGameObject )
            {
                AddGameObjectToObjectList( pChildGameObject, pPrefab );
                pChildGameObject = pChildGameObject->GetNext();
            }

            // Don't show components, deleted prefab children or deleted prefab components if forced open.
            if( forceOpen == false )
            {
                // Add Components.
                for( unsigned int ci=0; ci<pGameObject->GetComponentCountIncludingCore(); ci++ )
                {
                    ComponentBase* pComponent = pGameObject->GetComponentByIndexIncludingCore( ci );
                    if( pComponent )
                    {
                        if( pComponent->IsA( "GameObjectPropertiesComponent" ) )
                            continue;

                        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf;
                        if( pEditorState->IsComponentSelected( pComponent ) )
                        {
                            nodeFlags |= ImGuiTreeNodeFlags_Selected;
                        }

                        if( ImGui::TreeNodeEx( pComponent, nodeFlags, pComponent->GetClassname() ) )
                        {
                            if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                            {
                                // If the right-clicked object isn't selected, then unselect what is and select just this one.
                                if( pEditorState->IsComponentSelected( pComponent ) == false )
                                {
                                    pEditorState->ClearSelectedObjectsAndComponents();
                                    pEditorState->SelectComponent( pComponent );
                                }

                                char* label = "Delete Component";
                                if( pEditorState->m_pSelectedComponents.size() > 1 )
                                {
                                    label = "Delete Selected Components";
                                }

                                if( ImGui::MenuItem( label ) )
                                {
                                    pComponent->OnRightClickAction( ComponentBase::RightClick_DeleteComponent );
                                    ImGui::CloseCurrentPopup();
                                }

                                ImGui::EndPopup();
                            }
                            ImGui::TreePop();
                        }

                        if( ImGui::IsItemClicked() )
                        {
                            if( ImGui::GetIO().KeyCtrl == false )
                            {
                                pEditorState->ClearSelectedObjectsAndComponents();
                            }

                            if( ImGui::GetIO().KeyShift == false )
                            {
                                // TODO: select all Components between last object in list and this one.
                            }

                            if( pEditorState->IsComponentSelected( pComponent ) )
                            {
                                pEditorState->UnselectComponent( pComponent );
                            }
                            else
                            {
                                pEditorState->SelectComponent( pComponent );
                            }
                        }
                    }
                }

                // Add List of deleted Prefab Children.
                for( unsigned int dpci=0; dpci<pGameObject->m_DeletedPrefabChildIDs.size(); dpci++ )
                {
                    uint32 deletedPrefabChildID = pGameObject->m_DeletedPrefabChildIDs[dpci];
                    GameObject* pChildGameObject = pGameObject->GetPrefabRef()->GetPrefab()->FindChildGameObject( deletedPrefabChildID );

                    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf;

                    if( ImGui::TreeNodeEx( pChildGameObject, nodeFlags, "Deleted: %s", pChildGameObject->GetName() ) )
                    {
                    //    if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                    //    {
                    //        char* label = "Restore Component";
                    //        if( pEditorState->m_pSelectedComponents.size() > 1 )
                    //        {
                    //            label = "Restore Selected Components";
                    //        }
                    //        if( ImGui::MenuItem( label ) )
                    //        {
                    //            g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_RestorePrefabComponent( pGameObject, deletedPrefabComponentID ) );
                    //            ImGui::CloseCurrentPopup();
                    //        }
                    //        ImGui::EndPopup();
                    //    }
                        ImGui::TreePop();
                    }
                }

                // Add List of deleted Prefab Components.
                for( unsigned int dpci=0; dpci<pGameObject->m_DeletedPrefabComponentIDs.size(); dpci++ )
                {
                    uint32 deletedPrefabComponentID = pGameObject->m_DeletedPrefabComponentIDs[dpci];
                    ComponentBase* pComponent = pGameObject->GetPrefabRef()->GetGameObject()->FindComponentByPrefabComponentID( deletedPrefabComponentID );

                    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf;

                    if( ImGui::TreeNodeEx( pComponent, nodeFlags, "Deleted: %s", pComponent->GetClassname() ) )
                    {
                        if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                        {
                            char* label = "Restore Component";
                            if( pEditorState->m_pSelectedComponents.size() > 1 )
                            {
                                label = "Restore Selected Components";
                            }
                            if( ImGui::MenuItem( label ) )
                            {
                                g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_RestorePrefabComponent( pGameObject, deletedPrefabComponentID ) );
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::EndPopup();
                        }
                        ImGui::TreePop();
                    }
                }
            }

            if( treeNodeIsOpen )
                ImGui::TreePop();

            if( forceOpen )
                ImGui::Unindent();
        }
    }

    // Draw a horizontal line to indicate the drag/drop will do a reorder and not a reparent.
    if( m_pGameObjectToDrawReorderLineAfter == pGameObject )
    {
        ImGuiExt::DrawBlock( startCursorPositionX - 5.0f, -3.0f, 8888.0f, 2.0f, ImGuiCol_DragDropTarget );
    }
}

void EditorMainFrame_ImGui::AddWatchPanel()
{
    EditorPrefs* pEditorPrefs = m_pEngineCore->GetEditorPrefs();

    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Watch] == false )
        return;

    ImGui::SetNextWindowPos( ImVec2(852, 25), ImGuiCond_FirstUseEver );

    if( ImGui::Begin( "Watch", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Watch], ImVec2(333, 395) ) )
    {
        EditorState* pEditorState = m_pEngineCore->GetEditorState();

        int numGameObjectsSelected = (int)pEditorState->m_pSelectedObjects.size();

        // If multiple objects are selected, show their shared components.
        // If only one object is selected, just show it's components (will show nothing for empty folders).
        if( numGameObjectsSelected > 0 )
        {
            // Pick the first game object, even it it's a folder.
            GameObject* pFirstGameObject = pEditorState->m_pSelectedObjects[0];
            int firstGameObjectIndex = 0;

            // Find the first non-folder object if there is one, since folders shouldn't have components.
            int numNonFoldersSelected = 0;
            for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
            {
                if( pEditorState->m_pSelectedObjects[i]->IsFolder() == false )
                {
                    if( numNonFoldersSelected == 0 )
                    {
                        pFirstGameObject = pEditorState->m_pSelectedObjects[i];
                        firstGameObjectIndex = i;
                    }

                    numNonFoldersSelected++;
                }
            }

            if( numNonFoldersSelected > 1 )
            {
                ImGui::Text( "%d objects selected.", numNonFoldersSelected );
            }
            else
            {
                GameObject* pGameObjectThisInheritsFrom = pFirstGameObject->GetGameObjectThisInheritsFrom();

                if( pGameObjectThisInheritsFrom == nullptr )
                {
                    ImGui::Text( "%s", pFirstGameObject->GetName() );
                }
                else
                {
                    ImGui::Text( "%s (%s)", pFirstGameObject->GetName(), pGameObjectThisInheritsFrom->GetName() );
                }
                ImGui::SameLine();
                if( pFirstGameObject->IsEnabled() )
                {
                    if( ImGui::Button( "Disable" ) )
                        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_EnableObject( pFirstGameObject, false, true ) );
                }
                else
                {
                    if( ImGui::Button( "Enable" ) )
                        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_EnableObject( pFirstGameObject, true, true ) );
                }
            }

            // Only show components for non-folder objects.
            if( pFirstGameObject->IsFolder() == false )
            {
                // Show common components of all selected Gameobjects:
                {
                    // Search all components including GameObject properties and transform.
                    for( unsigned int foci=0; foci<pFirstGameObject->GetComponentCountIncludingCore(); foci++ )
                    {
                        ComponentBase* pComponentToLookFor = pFirstGameObject->GetComponentByIndexIncludingCore( foci );

                        MyAssert( pComponentToLookFor );

                        pComponentToLookFor->m_MultiSelectedComponents.clear();

                        // Loop through selected gameobjects and check if they all have to least one of this component type on them.
                        bool allGameObjectsHaveComponent = true;
                        for( unsigned int soi=firstGameObjectIndex+1; soi<pEditorState->m_pSelectedObjects.size(); soi++ )
                        {
                            GameObject* pGameObject = pEditorState->m_pSelectedObjects[soi];
                            MyAssert( pGameObject != pFirstGameObject );

                            // Skip folders, since they shouldn't have components.
                            if( pGameObject->IsFolder() )
                                continue;

                            bool hasComponent = false;
                            for( unsigned int soci=0; soci<pGameObject->GetComponentCountIncludingCore(); soci++ )
                            {
                                ComponentBase* pOtherComponent = pGameObject->GetComponentByIndexIncludingCore( soci );

                                if( pOtherComponent && pOtherComponent->IsA( pComponentToLookFor->GetClassname() ) == true )
                                {
                                    pComponentToLookFor->m_MultiSelectedComponents.push_back( pOtherComponent );
                                    hasComponent = true;
                                    break;
                                }
                            }

                            if( hasComponent == false )
                            {
                                allGameObjectsHaveComponent = false;
                                break;
                            }
                        }

                        if( allGameObjectsHaveComponent == true )
                        {
                            //ImGui::PushStyleColor( ImGuiCol_Header, (ImVec4)ImColor::ImColor(50,100,0,255) );
                            //ImGui::PushStyleColor( ImGuiCol_HeaderHovered, (ImVec4)ImColor::ImColor(50,70,0,255) );
                            //ImGui::PushStyleColor( ImGuiCol_HeaderActive, (ImVec4)ImColor::ImColor(50,30,0,255) );
                            ImGui::PushID( pComponentToLookFor );

                            Vector4 bgColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Header );
                            Vector4 checkColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_CheckMark );
                            ImGui::PushStyleColor( ImGuiCol_FrameBg, bgColor );
                            ImGui::PushStyleColor( ImGuiCol_CheckMark, checkColor );

                            // Draw the enabled/disabled checkbox.
                            ComponentBase::EnabledState enabledState = pComponentToLookFor->GetEnabledState();
                            bool enabled = true;
                            if( enabledState == ComponentBase::EnabledState_Disabled_ManuallyDisabled )
                            {
                                enabled = false;
                            }
                            if( ImGui::Checkbox( "", &enabled ) )
                            {
                                pComponentToLookFor->SetEnabled( enabled );
                            }
                            ImGui::PopStyleColor( 2 );
                            ImGui::SameLine();
                            ImGui::SetCursorPosX( ImGui::GetCursorPosX() - 5 );

                            // Draw the component name in a collapsable block.
                            bool componentBlockIsOpen = ImGui::CollapsingHeader( pComponentToLookFor->GetClassname(), ImGuiTreeNodeFlags_DefaultOpen );

                            // Add context menu on the collapsingHeader.
                            if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                            {
                                if( pComponentToLookFor->m_MultiSelectedComponents.size() == 0 )
                                {
                                    if( ImGui::MenuItem( "Delete Component" ) )
                                    {
                                        std::vector<ComponentBase*> componentsToDelete;
                                        componentsToDelete.push_back( pComponentToLookFor );
                                        g_pEngineCore->GetCommandStack()->Do( MyNew EditorCommand_DeleteComponents( componentsToDelete ) );
                                        ImGui::CloseCurrentPopup();
                                    }
                                }
                                else
                                {
                                    if( ImGui::MenuItem( "Delete Components" ) )
                                    {
                                        // Add pComponentToLookFor and pComponentToLookFor->m_MultiSelectedComponents to a vector and delete them.
                                        std::vector<ComponentBase*> componentsToDelete;
                                        componentsToDelete.push_back( pComponentToLookFor );
                                        componentsToDelete.insert( componentsToDelete.end(), pComponentToLookFor->m_MultiSelectedComponents.begin(), pComponentToLookFor->m_MultiSelectedComponents.end() );
                                        
                                        g_pEngineCore->GetCommandStack()->Do( MyNew EditorCommand_DeleteComponents( componentsToDelete ) );
                                        ImGui::CloseCurrentPopup();
                                    }
                                }
                                ImGui::EndPopup();
                            }

                            // Add contents of component if collapsingHeader is open.
                            if( componentBlockIsOpen )
                            {
                                pComponentToLookFor->AddAllVariablesToWatchPanel();
                            }

                            ImGui::PopID();
                            //ImGui::PopStyleColor( 3 );
                        }
                    }
                }
            }
        }
        else
        {
            // No GameObjects selected, display components.
            int numselected = (int)m_pEngineCore->GetEditorState()->m_pSelectedComponents.size();

            if( numselected == 1 )
            {
                ImGui::Text( "%d component selected.", numselected );
                
                ComponentBase* pComponent = m_pEngineCore->GetEditorState()->m_pSelectedComponents[0];

                //ImGui::PushStyleColor( ImGuiCol_Header, (ImVec4)ImColor::ImColor(50,100,0,255) );
                //ImGui::PushStyleColor( ImGuiCol_HeaderHovered, (ImVec4)ImColor::ImColor(50,70,0,255) );
                //ImGui::PushStyleColor( ImGuiCol_HeaderActive, (ImVec4)ImColor::ImColor(50,30,0,255) );
                if( ImGui::CollapsingHeader( pComponent->GetClassname(), ImGuiTreeNodeFlags_DefaultOpen ) )
                {
                    //ImGui::Text( "TODO: Component Info." );
                    ImGui::PushID( pComponent );
                    pComponent->AddAllVariablesToWatchPanel();
                    ImGui::PopID();
                }
                //ImGui::PopStyleColor( 3 );
            }
            else
            {
                ImGui::Text( "%d components selected.", numselected );

                // Loop through all selected components:
                for( unsigned int i=0; i<pEditorState->m_pSelectedComponents.size(); i++ )
                {
                    ComponentBase* pInitialComponent = pEditorState->m_pSelectedComponents[i];

                    // Clear this components MultiSelected list.
                    pInitialComponent->m_MultiSelectedComponents.clear();

                    // Have we shown this type yet?
                    bool alreadyShowedThisType = false;
                    for( unsigned int j=0; j<i; j++ )
                    {
                        ComponentBase* pComponent = pEditorState->m_pSelectedComponents[j];
                        if( pComponent && pComponent->IsA( pInitialComponent->GetClassname() ) == true )
                            alreadyShowedThisType = true;
                    }

                    if( alreadyShowedThisType == false )
                    {
                        pInitialComponent->m_MultiSelectedComponents.push_back( pInitialComponent );

                        // Loop through the rest of the selected components again to find common ones:
                        for( unsigned int j=i+1; j<pEditorState->m_pSelectedComponents.size(); j++ )
                        {
                            ComponentBase* pComponent = pEditorState->m_pSelectedComponents[j];
                            MyAssert( pComponent );

                            // If they're the same type, fill the MultiSelected list with the other components. 
                            if( pComponent && pComponent->IsA( pInitialComponent->GetClassname() ) == true )
                            {
                                pInitialComponent->m_MultiSelectedComponents.push_back( pComponent );
                            }
                        }

                        // Show all the components in the watch window.
                        //ImGui::PushStyleColor( ImGuiCol_Header, (ImVec4)ImColor::ImColor(50,100,0,255) );
                        //ImGui::PushStyleColor( ImGuiCol_HeaderHovered, (ImVec4)ImColor::ImColor(50,70,0,255) );
                        //ImGui::PushStyleColor( ImGuiCol_HeaderActive, (ImVec4)ImColor::ImColor(50,30,0,255) );
                        if( ImGui::CollapsingHeader( pInitialComponent->GetClassname(), ImGuiTreeNodeFlags_DefaultOpen ) )
                        {
                            ImGui::PushID( pInitialComponent );
                            pInitialComponent->AddAllVariablesToWatchPanel();
                            ImGui::PopID();
                        }
                        //ImGui::PopStyleColor( 3 );
                    }
                }
            }
        }

        // Create dummy control in empty region to allow for context menu.
        {
            ImVec2 regionAvailable = ImGui::GetContentRegionAvail();
            if( regionAvailable.y < 50 )
                regionAvailable.y = 50;
            ImGui::Dummy( regionAvailable );

            // Only works when a single GameObject is selected.  TODO: Make this work with multiple.
            if( numGameObjectsSelected == 1 )
            {
                // Pick the first game object, even if it's a folder.
                GameObject* pFirstGameObject = pEditorState->m_pSelectedObjects[0];

                ImGui::PushID( pFirstGameObject );
                if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                {
                    AddContextMenuOptionsForAddingComponents( pFirstGameObject );
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
        }
    }
    ImGui::End();
}

void EditorMainFrame_ImGui::AddLogWindow()
{
    ImGui::SetNextWindowSize( ImVec2(842, 167), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowPos( ImVec2(6, 476), ImGuiCond_FirstUseEver );

    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Log] )
    {
        m_pLogWindow->Draw( "Log", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Log] );
    }
}

void EditorMainFrame_ImGui::AddMemoryWindow()
{
    ImGui::SetNextWindowSize( ImVec2(842, 167), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowPos( ImVec2(6, 476), ImGuiCond_FirstUseEver );

    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Debug_MemoryAllocations] )
    {
        m_pMemoryWindow->DrawStart( "Memory Allocations", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Debug_MemoryAllocations] );
        if( ImGui::Button( "Refresh" ) )
        {
            m_pMemoryWindow->Clear();

            for( CPPListNode* pNode = MyMemory_GetFirstMemObject(); pNode; pNode = pNode->GetNext() )
            {
                MemObject* pMem = (MemObject*)pNode;

                m_pMemoryWindow->AddEntry( pMem->m_File, pMem->m_Line, (uint32)pMem->m_Size );
            }
        }
        
        char temp[32];

        PrintNumberWithCommas( temp, 32, MyMemory_GetNumberOfMemoryAllocations() );
        ImGui::Text( "Total Allocations: %s", temp );

        PrintNumberWithCommas( temp, 32, MyMemory_GetNumberOfActiveMemoryAllocations() );
        ImGui::Text( "Active Allocations: %s", temp );

        PrintNumberWithCommas( temp, 32, (uint32)MyMemory_GetNumberOfBytesAllocated() );
        ImGui::Text( "Active Bytes Allocated: %s", temp );

        m_pMemoryWindow->DrawMid();
        m_pMemoryWindow->DrawEnd();
    }
}

void EditorMainFrame_ImGui::AddCommandStacksWindow()
{
    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Debug_CommandStacks] == false )
        return;

    ImGui::SetNextWindowSize( ImVec2(842, 167), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowPos( ImVec2(6, 476), ImGuiCond_FirstUseEver );

    if( ImGui::Begin( "Undo/Redo", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Resources], ImVec2(334, 220) ) )
    {
        CommandStack* pCommandStack = m_pEngineCore->GetCommandStack();
        if( pCommandStack )
        {
            if( ImGui::CollapsingHeader( "Undo Commands", nullptr, ImGuiTreeNodeFlags_DefaultOpen ) )
            {
                for( uint32 i=pCommandStack->GetUndoStackSize(); i>0; i-- )
                {
                    EditorCommand* pCommand = pCommandStack->GetUndoCommandAtIndex( i-1 );

                    ImGui::Text( "%s", pCommand->GetName() );
                }
            }

            if( ImGui::CollapsingHeader( "Redo Commands", nullptr, ImGuiTreeNodeFlags_DefaultOpen ) )
            {
                for( uint32 i=pCommandStack->GetRedoStackSize(); i>0; i-- )
                {
                    EditorCommand* pCommand = pCommandStack->GetRedoCommandAtIndex( i-1 );

                    ImGui::Text( "%s", pCommand->GetName() );
                }
            }
        }        
    }
    ImGui::End();
}

void EditorMainFrame_ImGui::AddMemoryPanel()
{
    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Resources] == false )
        return;

    ImGui::SetNextWindowPos( ImVec2(853, 424), ImGuiCond_FirstUseEver );

    if( ImGui::Begin( "Resources", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Resources], ImVec2(334, 220) ) )
    {
        // Add an input box for memory panel filter.
        // For now, it will always auto-select the text when given focus.
        {
            ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_AutoSelectAll;
            if( m_SetMemoryPanelFilterBoxInFocus )
            {
                //inputTextFlags |= ImGuiInputTextFlags_AutoSelectAll;
                ImGui::SetKeyboardFocusHere();
                m_SetMemoryPanelFilterBoxInFocus = false;
            }
            ImGui::InputText( "Filter", m_MemoryPanelFilter, 100, inputTextFlags );
            ImGui::SameLine();
            if( ImGui::Button( "X" ) )
            {
                m_MemoryPanelFilter[0] = '\0';
            }
        }

        // If a filter is set, show all types.
        if( m_MemoryPanelFilter[0] != '\0' )
        {
            ImGui::BeginChild( "Memory Details" );

            AddMemoryPanel_Materials();
            AddMemoryPanel_Textures();
            AddMemoryPanel_ShaderGroups();
            AddMemoryPanel_Files();

            ImGui::EndChild();
        }
        else
        {
            if( ImGui::BeginTabBar( "MemoryPages" ) )
            {
                if( ImGui::BeginTabItem( "Materials" ) )
                {
                    ImGui::EndTabItem();

                    ImGui::BeginChild( "Memory Details" );
                    AddMemoryPanel_Materials();
                    ImGui::EndChild();
                }

                if( ImGui::BeginTabItem( "Textures" ) )
                {
                    ImGui::EndTabItem();

                    ImGui::BeginChild( "Memory Details" );
                    AddMemoryPanel_Textures();
                    ImGui::EndChild();
                }

                if( ImGui::BeginTabItem( "Shaders" ) )
                {
                    ImGui::EndTabItem();

                    ImGui::BeginChild( "Memory Details" );
                    AddMemoryPanel_ShaderGroups();
                    ImGui::EndChild();
                }

                if( ImGui::BeginTabItem( "Sound Cues" ) )
                {
                    ImGui::EndTabItem();

                    ImGui::BeginChild( "Memory Details" );
                    AddMemoryPanel_SoundCues();
                    ImGui::EndChild();
                }

                if( ImGui::BeginTabItem( "Files" ) )
                {
                    ImGui::EndTabItem();

                    ImGui::BeginChild( "Memory Details" );
                    AddMemoryPanel_Files();
                    ImGui::EndChild();
                }

                if( ImGui::BeginTabItem( "Buffers" ) )
                {
                    ImGui::EndTabItem();

                    ImGui::BeginChild( "Memory Details" );
                    ImGui::Text( "TODO" );
                    ImGui::EndChild();
                }

                if( ImGui::BeginTabItem( "Draw Calls" ) )
                {
                    ImGui::EndTabItem();

                    ImGui::BeginChild( "Memory Details" );
                    AddMemoryPanel_DrawCalls();
                    ImGui::EndChild();
                }

                ImGui::EndTabBar();
            }
        }
    }
    ImGui::End();
}

void EditorMainFrame_ImGui::AddContextMenuOptionsForAddingComponents(GameObject* pGameObject)
{
    int first = 0;
    if( pGameObject->GetTransform() != nullptr )
        first = 1;

    const char* lastcategory = nullptr;
    bool menuopen = false;

    unsigned int numtypes = g_pComponentTypeManager->GetNumberOfComponentTypes();
    for( unsigned int i=first; i<numtypes; i++ )
    {
        const char* currentcategory = g_pComponentTypeManager->GetTypeCategory( i );
        const char* nextcategory = nullptr;
        if( i < numtypes-1 )
            nextcategory = g_pComponentTypeManager->GetTypeCategory( i+1 );

        if( lastcategory != currentcategory )
        {
            menuopen = ImGui::BeginMenu( currentcategory );
        }

        if( menuopen )
        {
            if( i == ComponentType_Mesh )
            {
                // Don't include ComponentType_Mesh in the right-click menu.
                // TODO: If more exceptions are made, improve this system.
            }
            else
            {
                if( ImGui::MenuItem( g_pComponentTypeManager->GetTypeName( i ) ) )
                {
                    ComponentBase* pComponent = nullptr;
                    if( m_pEngineCore->IsInEditorMode() )
                    {
                        EditorCommand_CreateComponent* pCommand = MyNew EditorCommand_CreateComponent( pGameObject, i );
                        g_pGameCore->GetCommandStack()->Do( pCommand );
                        pComponent = pCommand->GetCreatedObject();
                        pComponent->OnLoad();
                    }
                    else
                    {
                        pComponent = pGameObject->AddNewComponent( i, SCENEID_Unmanaged, g_pComponentSystemManager );
                    }

                    ImGui::CloseCurrentPopup();
                }
            }
        }

        if( menuopen && currentcategory != nextcategory )
        {
            ImGui::EndMenu();
        }

        lastcategory = currentcategory;
    }

}

void EditorMainFrame_ImGui::AddContextMenuOptionsForCreatingGameObjects(GameObject* pParentGameObject, SceneID sceneID)
{
    GameObject* pGameObjectCreated = nullptr;
                            
    if( ImGui::MenuItem( "Add GameObject" ) )
    {
        pGameObjectCreated = g_pComponentSystemManager->CreateGameObject( true, sceneID );
        pGameObjectCreated->SetName( "New Game Object" );

        ImGui::CloseCurrentPopup();
    }

    if( ImGui::BeginMenu( "Add GameObject from Template" ) )
    {
        GameObjectTemplateManager* pManager = g_pComponentSystemManager->m_pGameObjectTemplateManager;
    
        cJSON* jFirstParent = pManager->GetParentTemplateJSONObject( 0 );

        unsigned int i = 0;
        while( i < pManager->GetNumberOfTemplates() )
        {
            bool isFolder = pManager->IsTemplateAFolder( i );
            const char* name = pManager->GetTemplateName( i );

            if( isFolder == false )
            {
                if( ImGui::MenuItem( name ) )
                {
                    pGameObjectCreated = g_pComponentSystemManager->CreateGameObjectFromTemplate( i, sceneID );
                    if( pGameObjectCreated )
                    {
                        EditorState* pEditorState = m_pEngineCore->GetEditorState();
                        pEditorState->ClearSelectedObjectsAndComponents();
                        pEditorState->SelectGameObject( pGameObjectCreated );
                    }
                }
            }

            i++;
        }

        ImGui::EndMenu();
    }

    // Don't allow folders other than at root on as children to other folders.
    if( pParentGameObject == nullptr || pParentGameObject->IsFolder() )
    {
        if( ImGui::MenuItem( "Add Folder" ) )
        {
            pGameObjectCreated = g_pComponentSystemManager->CreateGameObject( true, sceneID, true, false );
            pGameObjectCreated->SetName( "New Folder" );

            ImGui::CloseCurrentPopup();
        }
    }

    if( ImGui::MenuItem( "Add Logical GameObject" ) )
    {
        pGameObjectCreated = g_pComponentSystemManager->CreateGameObject( true, sceneID, false, false );
        pGameObjectCreated->SetName( "New Logical Game Object" );

        ImGui::CloseCurrentPopup();
    }

    if( pGameObjectCreated )
    {
        if( pParentGameObject )
        {
            pGameObjectCreated->SetParentGameObject( pParentGameObject );
        }
        g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_CreateGameObject( pGameObjectCreated ) );
    }
}

void EditorMainFrame_ImGui::AddContextMenuItemsForFiles(MyFileObject* pFile, void* pSelectedObject)
{
    FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();

    const char* extension = pFile->GetExtensionWithDot();

    if( strcmp( extension, ".my2daniminfo" ) == 0 )
    {
        if( ImGui::MenuItem( "Edit 2D Anim Info", nullptr, &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_2DAnimationEditor] ) )
        {
            My2DAnimInfo* pAnim = g_pComponentSystemManager->GetFileInfoIfUsedByScene( pFile, SCENEID_Any )->Get2DAnimInfo();
            Edit2DAnimInfo( pAnim );
            ImGui::CloseCurrentPopup();
        }
    }
    else if( pSelectedObject != nullptr && strcmp( extension, ".glsl" ) == 0 )
    {
        if( ImGui::MenuItem( "Create Material Using Shader" ) )
        {
            MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();
            MaterialDefinition* pMat = pMaterialManager->CreateMaterial( pFile->GetFilenameWithoutExtension(), "Data/Materials" );
            pMat->SetShader( (ShaderGroup*)pSelectedObject );
            ImGui::CloseCurrentPopup();
        }
    }
    else
    {
        if( ImGui::MenuItem( "View in Watch Window (TODO)" ) )   { pFile->OnPopupClick( pFileManager, pFile, MyFileObject::RightClick_ViewInWatchWindow );    ImGui::CloseCurrentPopup(); }
    }
    if( ImGui::MenuItem( "Open File" ) )              { pFile->OnPopupClick( pFileManager, pFile, MyFileObject::RightClick_OpenFile );             ImGui::CloseCurrentPopup(); }
    if( ImGui::MenuItem( "Open Containing Folder" ) ) { pFile->OnPopupClick( pFileManager, pFile, MyFileObject::RightClick_OpenContainingFolder ); ImGui::CloseCurrentPopup(); }
    if( ImGui::MenuItem( "Unload File" ) )            { pFile->OnPopupClick( pFileManager, pFile, MyFileObject::RightClick_UnloadFile );           ImGui::CloseCurrentPopup(); }
    if( ImGui::MenuItem( "Find References" ) )        { pFile->OnPopupClick( pFileManager, pFile, MyFileObject::RightClick_FindAllReferences );    ImGui::CloseCurrentPopup(); } // (%d)", pMat->GetRefCount() ) {}
}

void EditorMainFrame_ImGui::AddContextMenuItemsForMaterials(MaterialDefinition* pMaterial)
{
    // Clear is handled in ComponentBase::AddVariableToWatchPanel.
    
    if( ImGui::MenuItem( "Edit material" ) )
    {
        EditMaterial( pMaterial );
    }
}

void EditorMainFrame_ImGui::AddMemoryPanel_Materials()
{
    // Only show the headers if the filter is blank.
    bool showHeaders = (m_MemoryPanelFilter[0] == '\0');

    bool someMaterialsAreLoaded = false;
    //unsigned int numMaterialsShown = 0;

    for( int i=0; i<2; i++ )
    {
        MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();
        //MaterialDefinition* pMat = pMaterialManager->GetFirstMaterial();
        MaterialDefinition* pMat = pMaterialManager->Editor_GetFirstMaterialStillLoading();
        if( i == 1 )
            pMat = pMaterialManager->Editor_GetFirstMaterialLoaded();

        if( pMat == nullptr )
            continue;

        someMaterialsAreLoaded = true;

        ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

        char* label = "Materials - Loading";
        ImGuiTreeNodeFlags nodeFlags = baseNodeFlags;
        if( i == 1 )
        {
            label = "All Materials";
            nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
        }

        if( showHeaders == false || ImGui::TreeNodeEx( label, nodeFlags ) )
        {
            // TODO: Add folders for materials.
            //const char* foldername = pMat->GetFile()->GetNameOfDeepestFolderPath();

            if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
            {
                if( ImGui::MenuItem( "Create New Material" ) )
                {
                    ImGui::CloseCurrentPopup();
                    MaterialDefinition* pMaterial = pMaterialManager->CreateMaterial( "new" );

                    // TODO: Fix path based on folders.
                    char tempstr[MAX_PATH];
                    sprintf_s( tempstr, MAX_PATH, "Data/Materials" );
                    pMaterial->SaveMaterial( tempstr );
                    pMaterialManager->Editor_MoveMaterialToFrontOfLoadedList( pMaterial );

                    // Essentially, tell the ComponentSystemManager that a new material was loaded.
                    //  This will add it to the scene's file list, which will free the material.
                    pMaterialManager->CallMaterialCreatedCallbacks( pMaterial );

                    // Start a rename op on the new material.
                    StartRenameOp( nullptr, pMaterial, pMaterial->GetName() );
                }
                ImGui::EndPopup();
            }

            while( pMat )
            {
                MaterialDefinition* pNextMat = (MaterialDefinition*)pMat->GetNext();

                if( pMat == m_pMaterialWhoseNameIsBeingEdited )
                {
                    ImGui::SetKeyboardFocusHere();
                    if( ImGui::InputText( "New name", m_NameBeingEdited, 100, ImGuiInputTextFlags_AutoSelectAll|ImGuiInputTextFlags_EnterReturnsTrue ) ||
                        m_ConfirmCurrentRenameOp )
                    {
                        m_pMaterialWhoseNameIsBeingEdited->SetName( m_NameBeingEdited );
                        m_pMaterialWhoseNameIsBeingEdited = nullptr;
                    }
                }
                else
                {
                    MyFileObject* pFile = pMat->GetFile();

                    if( pFile )
                    {
                        const char* matName = pFile->GetFilenameWithoutExtension();

                        //numMaterialsShown++;

                        bool showThisItem = true;

                        if( m_MemoryPanelFilter[0] != '\0' )
                        {
                            if( CheckIfMultipleSubstringsAreInString( matName, m_MemoryPanelFilter ) == false )
                            {
                                showThisItem = false;
                            }
                        }

                        if( showThisItem )
                        {
                            if( ImGui::TreeNodeEx( matName, baseNodeFlags | ImGuiTreeNodeFlags_Leaf ) )
                            {
                                // TODO: Find a better answer than IsItemHovered().
                                if( ImGui::IsItemHovered() && m_RenamePressedThisFrame )
                                {
                                    StartRenameOp( nullptr, pMat, matName );
                                }

                                // Deal with slow double-click for renaming Materials.
                                if( WasItemSlowDoubleClicked( pMat ) )
                                {
                                    StartRenameOp( nullptr, pMat, matName );
                                }

                                if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                                {
                                    if( ImGui::MenuItem( "Edit Material", nullptr, &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_MaterialEditor] ) ) { EditMaterial( pMat ); ImGui::CloseCurrentPopup(); }
                                    if( ImGui::MenuItem( "Unload File" ) )     { pMat->OnPopupClick( pMat, MaterialDefinition::RightClick_UnloadFile ); ImGui::CloseCurrentPopup(); pMat = nullptr; }
                                    if( ImGui::MenuItem( "Find References" ) ) { pMat->OnPopupClick( pMat, MaterialDefinition::RightClick_FindAllReferences ); ImGui::CloseCurrentPopup(); } // (%d)", pMat->GetRefCount() ) {}
                                    if( ImGui::MenuItem( "Rename" ) )
                                    {
                                        StartRenameOp( nullptr, pMat, matName );
                                        ImGui::CloseCurrentPopup();
                                    }
                                    ImGui::EndPopup();
                                }

                                if( ImGui::IsItemHovered() )
                                {
                                    if( ImGui::IsMouseDoubleClicked( 0 ) )
                                    {
                                        EditMaterial( pMat );
                                    }

                                    ImGui::BeginTooltip();
                                    ImGui::Text( "%s", pMat->GetName() );
                                    AddMaterialPreview( pMat, false, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 1 ) );
                                    ImGui::EndTooltip();
                                }

                                if( ImGui::BeginDragDropSource() )
                                {
                                    ImGui::SetDragDropPayload( "Material", &pMat, sizeof(pMat), ImGuiCond_Once );
                                    ImGui::Text( "%s", pMat->GetName() );
                                    AddMaterialPreview( pMat, false, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 0.5f ) );
                                    ImGui::EndDragDropSource();
                                }

                                ImGui::TreePop();
                            }
                        }
                    }
                }

                pMat = pNextMat;
            }

            if( showHeaders )
            {
                ImGui::TreePop();
            }
        }
    }

    if( someMaterialsAreLoaded == false )
    {
        ImGui::TreeNodeEx( "No materials loaded.", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen );
    }
}

void EditorMainFrame_ImGui::AddMemoryPanel_Textures()
{
    // Only show the headers if the filter is blank.
    bool showHeaders = (m_MemoryPanelFilter[0] == '\0');

    bool someTexturesAreLoaded = false;
    //unsigned int numTexturesShown = 0;

    TextureManager* pTextureManager = m_pEngineCore->GetManagers()->GetTextureManager();

    for( int i=0; i<2; i++ )
    {
        TextureDefinition* pTex = (TextureDefinition*)pTextureManager->m_TexturesStillLoading.GetHead();
        if( i == 1 )
            pTex = (TextureDefinition*)pTextureManager->m_LoadedTextures.GetHead();

        if( pTex )
        {
            someTexturesAreLoaded = true;

            ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

            char* label = "Textures - Loading";
            if( i == 1 )
                label = "All Textures";

            if( showHeaders == false || ImGui::TreeNodeEx( label, baseNodeFlags | ImGuiTreeNodeFlags_DefaultOpen ) )
            {
                while( pTex )
                {
                    MyFileObject* pFile = pTex->GetFile();

                    if( pTex->m_ShowInMemoryPanel )
                    {
                        //numTexturesShown++;

                        bool showThisItem = true;

                        if( m_MemoryPanelFilter[0] != '\0' )
                        {
                            if( CheckIfMultipleSubstringsAreInString( pTex->GetFilename(), m_MemoryPanelFilter ) == false )
                            {
                                showThisItem = false;
                            }
                        }

                        if( showThisItem )
                        {
                            const char* filename = pTex->GetFilename();
                            if( filename == nullptr || filename[0] == '\0' )
                                filename = "No filename";
                            if( ImGui::TreeNodeEx( filename, ImGuiTreeNodeFlags_Leaf | baseNodeFlags ) )
                            {
                                if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                                {
                                    FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();

                                    if( ImGui::MenuItem( "Unload File (TODO)" ) )     { ImGui::CloseCurrentPopup(); }
                                    if( ImGui::MenuItem( "Find References" ) ){ if( pFile ) pFileManager->Editor_FindAllReferences( pFile ); ImGui::CloseCurrentPopup(); } ;// (%d)", pMat->GetRefCount() ) {}
                                    ImGui::EndPopup();
                                }

                                if( ImGui::IsItemHovered() )
                                {
                                    ImGui::BeginTooltip();
                                    //ImGui::Text( "%s", pTex->GetFilename() );
                                    AddTexturePreview( pTex, false, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 1 ) );
                                    ImGui::EndTooltip();
                                }

                                if( ImGui::BeginDragDropSource() )
                                {
                                    ImGui::SetDragDropPayload( "Texture", &pTex, sizeof(pTex), ImGuiCond_Once );
                                    //ImGui::Text( "%s", pTex->GetFilename() );
                                    AddTexturePreview( pTex, false, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 1 ) );
                                    ImGui::EndDragDropSource();
                                }

                                ImGui::TreePop();
                            }
                        }
                    }

                    pTex = (TextureDefinition*)pTex->GetNext();
                }

                if( showHeaders )
                {
                    ImGui::TreePop();
                }
            }
        }
    }

    if( someTexturesAreLoaded == false )
    {
        ImGui::TreeNodeEx( "No textures loaded.", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen );
    }

    // Add FBOs
    {
        TextureManager* pTextureManager = m_pEngineCore->GetManagers()->GetTextureManager();

        FBODefinition* pFBO = pTextureManager->m_InitializedFBOs.GetHead();

        if( pFBO )
        {
            ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

            char* label = "FBOs";

            if( showHeaders == false || ImGui::TreeNodeEx( label, baseNodeFlags | ImGuiTreeNodeFlags_DefaultOpen ) )
            {
                while( pFBO )
                {
                    if( pFBO->m_ShowInMemoryPanel )
                    {
                        bool showThisItem = true;

                        //if( m_MemoryPanelFilter[0] != '\0' )
                        //{
                        //    if( CheckIfMultipleSubstringsAreInString( pFBO->GetFilename(), m_MemoryPanelFilter ) == false )
                        //    {
                        //        showThisItem = false;
                        //    }
                        //}

                        if( showThisItem )
                        {
                            TextureDefinition* pTex = pFBO->GetColorTexture( 0 );

                            if( pTex )
                            {
                                const char* filename = "Unnamed FBO";
                                if( ImGui::TreeNodeEx( filename, ImGuiTreeNodeFlags_Leaf | baseNodeFlags ) )
                                {
                                    if( ImGui::IsItemHovered() )
                                    {
                                        ImGui::BeginTooltip();
                                        AddTexturePreview( pTex, false, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 1 ) );
                                        ImGui::EndTooltip();
                                    }

                                    if( ImGui::BeginDragDropSource() )
                                    {
                                        ImGui::SetDragDropPayload( "Texture", &pTex, sizeof(pTex), ImGuiCond_Once );
                                        AddTexturePreview( pTex, false, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 1 ) );
                                        ImGui::EndDragDropSource();
                                    }

                                    ImGui::TreePop();
                                }
                            }
                        }
                    }

                    pFBO = pFBO->GetNext();
                }

                if( showHeaders )
                {
                    ImGui::TreePop();
                }
            }
        }
    }
}

void EditorMainFrame_ImGui::AddMemoryPanel_ShaderGroups()
{
    // Only show the headers if the filter is blank.
    bool showHeaders = (m_MemoryPanelFilter[0] == '\0');

    bool someShadersAreLoaded = false;
    //unsigned int numShadersShown = 0;

    {
        ShaderGroup* pShaderGroup = (ShaderGroup*)m_pEngineCore->GetManagers()->GetShaderGroupManager()->m_ShaderGroupList.GetHead();

        if( pShaderGroup )
        {
            someShadersAreLoaded = true;

            ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

            if( showHeaders == false || ImGui::TreeNodeEx( "All Shaders", baseNodeFlags | ImGuiTreeNodeFlags_DefaultOpen ) )
            {
                while( pShaderGroup )
                {
                    // Store the next shader in case this one gets unloaded.
                    ShaderGroup* pNextShaderGroup = (ShaderGroup*)pShaderGroup->GetNext();

                    MyFileObjectShader* pFile = pShaderGroup->GetFile();

                    if( pFile )
                    {
                        if( pFile->m_ShowInMemoryPanel )
                        {
                            bool showThisItem = true;

                            if( m_MemoryPanelFilter[0] != '\0' )
                            {
                                if( CheckIfMultipleSubstringsAreInString( pFile->GetFilenameWithoutExtension(), m_MemoryPanelFilter ) == false )
                                {
                                    showThisItem = false;
                                }
                            }

                            if( showThisItem )
                            {
                                if( ImGui::TreeNodeEx( pFile->GetFilenameWithoutExtension(), ImGuiTreeNodeFlags_Leaf | baseNodeFlags ) )
                                {
                                    if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                                    {
                                        AddContextMenuItemsForFiles( pFile, pShaderGroup );
                                        ImGui::EndPopup();
                                    }

                                    if( ImGui::BeginDragDropSource() )
                                    {
                                        ImGui::SetDragDropPayload( "ShaderGroup", &pShaderGroup, sizeof(pShaderGroup), ImGuiCond_Once );
                                        ImGui::Text( "%s", pFile->GetFullPath() );
                                        ImGui::EndDragDropSource();
                                    }

                                    ImGui::TreePop();
                                }
                            }
                        }
                    }

                    pShaderGroup = pNextShaderGroup;
                }

                if( showHeaders )
                {
                    ImGui::TreePop();
                }
            }
        }
    }

    if( someShadersAreLoaded == false)
    {
        ImGui::TreeNodeEx( "No shaders loaded.", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen );
    }
}

void EditorMainFrame_ImGui::AddMemoryPanel_SoundCues()
{
    // Only show the headers if the filter is blank.
    bool showHeaders = (m_MemoryPanelFilter[0] == '\0');

    bool someSoundCuesAreLoaded = false;

    for( int i=0; i<2; i++ )
    {
        SoundCue* pSoundCue = (SoundCue*)g_pGameCore->GetSoundManager()->GetCuesStillLoading();
        if( i == 1 )
            pSoundCue = (SoundCue*)g_pGameCore->GetSoundManager()->GetCues();

        if( pSoundCue )
        {
            someSoundCuesAreLoaded = true;

            ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

            char* label = "Sound Cues - Loading";
            if( i == 1 )
                label = "All Sound Cues";

            if( showHeaders == false || ImGui::TreeNodeEx( label, baseNodeFlags | ImGuiTreeNodeFlags_DefaultOpen ) )
            {
                while( pSoundCue )
                {
                    // Store the next shader in case this one gets unloaded.
                    SoundCue* pNextSoundCue = (SoundCue*)pSoundCue->GetNext();

                    MyFileObject* pFile = pSoundCue->GetFile();

                    if( pFile )
                    {
                        if( pFile->m_ShowInMemoryPanel )
                        {
                            bool showThisItem = true;

                            if( m_MemoryPanelFilter[0] != '\0' )
                            {
                                if( CheckIfMultipleSubstringsAreInString( pFile->GetFilenameWithoutExtension(), m_MemoryPanelFilter ) == false )
                                {
                                    showThisItem = false;
                                }
                            }

                            if( showThisItem )
                            {
                                if( ImGui::TreeNodeEx( pFile->GetFilenameWithoutExtension(), ImGuiTreeNodeFlags_Leaf | baseNodeFlags ) )
                                {
                                    if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                                    {
                                        AddContextMenuItemsForFiles( pFile );
                                        ImGui::EndPopup();
                                    }

                                    if( ImGui::BeginDragDropSource() )
                                    {
                                        ImGui::SetDragDropPayload( "SoundCue", &pSoundCue, sizeof(pSoundCue), ImGuiCond_Once );
                                        ImGui::Text( "%s", pFile->GetFullPath() );
                                        ImGui::EndDragDropSource();
                                    }

                                    ImGui::TreePop();
                                }
                            }
                        }
                    }

                    pSoundCue = pNextSoundCue;
                }

                if( showHeaders )
                {
                    ImGui::TreePop();
                }
            }
        }
    }

    if( someSoundCuesAreLoaded == false)
    {
        ImGui::TreeNodeEx( "No sound cues loaded.", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen );
    }
}

void EditorMainFrame_ImGui::AddMemoryPanel_Files()
{
    // Only show the headers if the filter is blank.
    bool showHeaders = (m_MemoryPanelFilter[0] == '\0');

    bool someFilesAreLoaded = false;
    //unsigned int numFilesShown = 0;

    // TODO: Don't do this every frame.
    FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();
    pFileManager->SortFileLists();

    for( int i=0; i<2; i++ )
    {
        MyFileObject* pFile = (MyFileObject*)pFileManager->GetFirstFileStillLoading();
        if( i == 1 )
            pFile = (MyFileObject*)pFileManager->GetFirstFileLoaded();

        if( pFile )
        {
            someFilesAreLoaded = true;

            ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

            char* label = "Files - Loading";
            if( i == 1 )
                label = "All Files";

            if( showHeaders == false || ImGui::TreeNodeEx( label, baseNodeFlags | ImGuiTreeNodeFlags_DefaultOpen ) )
            {
                const char* previousFileType = nullptr;
                bool fileTypeOpen = false;
                while( pFile )
                {
                    if( pFile->m_ShowInMemoryPanel )
                    {
                        //numFilesShown++;

                        if( previousFileType == nullptr || strcmp( previousFileType, pFile->GetExtensionWithDot() ) != 0 )
                        {
                            if( fileTypeOpen && previousFileType != nullptr )
                            {
                                if( showHeaders )
                                {
                                    ImGui::TreePop(); // "File Type"
                                }
                            }

                            previousFileType = pFile->GetExtensionWithDot();

                            if( showHeaders )
                            {
                                fileTypeOpen = ImGui::TreeNodeEx( previousFileType, baseNodeFlags );
                            }
                        }

                        if( fileTypeOpen )
                        {
                            bool showThisItem = true;

                            if( m_MemoryPanelFilter[0] != '\0' )
                            {
                                if( CheckIfMultipleSubstringsAreInString( pFile->GetFilenameWithoutExtension(), m_MemoryPanelFilter ) == false )
                                {
                                    showThisItem = false;
                                }
                            }

                            if( showThisItem )
                            {
                                if( ImGui::TreeNodeEx( pFile->GetFilenameWithoutExtension(), ImGuiTreeNodeFlags_Leaf | baseNodeFlags ) )
                                {
                                    if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                                    {
                                        AddContextMenuItemsForFiles( pFile );
                                        ImGui::EndPopup();
                                    }

                                    if( ImGui::BeginDragDropSource() )
                                    {
                                        ImGui::SetDragDropPayload( "File", &pFile, sizeof(pFile), ImGuiCond_Once );
                                        ImGui::Text( "%s", pFile->GetFullPath() );
                                        ImGui::EndDragDropSource();
                                    }

                                    ImGui::TreePop();
                                }
                            }
                        }
                    }

                    pFile = (MyFileObject*)pFile->GetNext();
                }

                if( fileTypeOpen && previousFileType != nullptr )
                {
                    if( showHeaders )
                    {
                        ImGui::TreePop(); // "File Type"
                    }
                }

                if( showHeaders )
                {
                    ImGui::TreePop(); // "All Files"
                }
            }
        }
    }

    if( someFilesAreLoaded == false )
    {
        ImGui::TreeNodeEx( "No files loaded.", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen );
    }
}

void EditorMainFrame_ImGui::AddMemoryPanel_DrawCalls()
{
    ImGuiTreeNodeFlags baseNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
    
    // Always reset draw call limiter to draw everything if one specific draw call isn't hovered.
    if( ImGui::IsWindowHovered(ImGuiFocusedFlags_RootWindow | ImGuiFocusedFlags_ChildWindows) == false )
    {
        g_GLStats.m_DrawCallLimit_Canvas = -1;
        g_GLStats.m_DrawCallLimit_Index = -1;
        m_SelectedDrawCallCanvas = -1;
        m_SelectedDrawCallIndex = -1;
    }

    ImGui::PushID( "DrawCallTree" );
    if( ImGui::TreeNodeEx( "Draw Calls", baseNodeFlags ) )
    {
        for( int canvasIndex=0; canvasIndex<2; canvasIndex++ )
        {
            char* label = "Game Window";
            if( canvasIndex == 1 )
                label = "Editor Window";

            if( ImGui::TreeNodeEx( label, baseNodeFlags ) )
            {
                for( int callIndex=0; callIndex<g_GLStats.m_NumDrawCallsLastFrame[canvasIndex]; callIndex++ )
                {
                    ImGui::PushID( callIndex );

                    bool selected = (canvasIndex == m_SelectedDrawCallCanvas && callIndex == m_SelectedDrawCallIndex);

                    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf;
                    if( selected )
                    {
                        nodeFlags |= ImGuiTreeNodeFlags_Selected;
                    }

                    if( ImGui::TreeNodeEx( "Draw", nodeFlags ) )
                    {
                        if( ImGui::IsItemHovered() )
                        {
                            m_SelectedDrawCallCanvas = canvasIndex;
                            m_SelectedDrawCallIndex = callIndex;
                            selected = true;
                        }

                        if( selected )
                        {
                            g_GLStats.m_DrawCallLimit_Canvas = canvasIndex;
                            g_GLStats.m_DrawCallLimit_Index = callIndex;
                        }

#if _DEBUG && MYFW_WINDOWS
                        if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                        {
                            if( ImGui::MenuItem( "Trigger breakpoint" ) )
                            {
                                g_GLStats.m_DrawCallLimit_BreakPointIndex = callIndex;
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::EndPopup();
                        }
#endif

                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }

                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }
    ImGui::PopID();
}

void EditorMainFrame_ImGui::AddMaterialEditor()
{
    EditorPrefs* pEditorPrefs = m_pEngineCore->GetEditorPrefs();

    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_MaterialEditor] == false )
        return;

    ImGui::SetNextWindowPos( ImVec2(856, 71), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowSize( ImVec2(339, 349), ImGuiCond_FirstUseEver );
    
    if( ImGui::Begin( "Material Editor", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_MaterialEditor] ) )
    {
        if( m_pMaterialBeingEdited == nullptr )
        {
            ImGui::Text( "No material selected." );
            ImGui::End();
            return;
        }
            
        // Create a context menu only available from the title bar.
        if( ImGui::BeginPopupContextItem() )
        {
            if( ImGui::MenuItem( "Close" ) )
                m_pCurrentLayout->m_IsWindowOpen[EditorWindow_MaterialEditor] = false;

            ImGui::EndPopup();
        }

        AddMaterialPreview( m_pMaterialBeingEdited, false, ImVec2( 100, 100 ), ImVec4(1,1,1,1) );

        {
            MaterialDefinition* pMat = m_pMaterialBeingEdited;
            bool showBuiltInUniforms = true;
            bool showExposedUniforms = true;
            ShaderGroup* pShaderGroup = pMat->GetShader();
            ShaderGroup* pShaderGroupInstanced = pMat->GetShaderInstanced();

            {
                ImGui::Text( "WORK IN PROGRESS - NO UNDO - MANUAL SAVE" );
                if( ImGui::Button( "Save" ) )
                {
                    pMat->SaveMaterial( nullptr );
                }
                ImGui::SameLine();
                ImGui::Text( "<- MANUAL SAVE" );
                ImGui::Separator();
            }

            ImGui::Text( pMat->GetName() );

            if( showBuiltInUniforms )
            {
                //g_pPanelWatch->AddEnum( "Blend", (int*)&m_BlendType, MyRE::MaterialBlendType_NumTypes, MyRE::MaterialBlendTypeStrings );
                const char** items = MyRE::MaterialBlendTypeStrings;
                int currentItem = pMat->GetBlendType();
                const char* currentItemStr = MyRE::MaterialBlendTypeStrings[currentItem];
                if( ImGui::BeginCombo( "Blend", currentItemStr ) )
                {
                    for( int n = 0; n < MyRE::MaterialBlendType_NumTypes; n++ )
                    {
                        bool is_selected = (n == currentItem);
                        if( ImGui::Selectable( items[n], is_selected ) )
                        {
                            //// Store the old value.
                            //ComponentVariableValue oldvalue( this, pVar );

                            //// Change the value.
                            pMat->SetBlendType( (MyRE::MaterialBlendTypes)n );

                            //// Store the new value.
                            //ComponentVariableValue newvalue( this, pVar );

                            //m_pEngineCore->GetCommandStack()->Do(
                            //    MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged(
                            //                                this, pVar, newvalue, oldvalue, true ),
                            //    false );
                        }
                        if( is_selected )
                        {
                            // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                if( ImGui::DragFloat2( "UVScale", &pMat->m_UVScale.x, 0.01f, 0, 1 ) )
                {
                    pMat->MarkDirty();
                }
                if( ImGui::DragFloat2( "UVOffset", &pMat->m_UVOffset.x, 0.01f, 0, 1 ) )
                {
                    pMat->MarkDirty();
                }

                // Deal with the shader attached to the material.
                {
                    ImGui::PushID( "Material_Shader" );

                    const char* desc = "no shader";

                    Vector4 buttonColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_UnsetObjectButton );
                    Vector4 textColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_UnsetObjectText );
                    
                    if( pShaderGroup && pShaderGroup->GetShader( ShaderPass_Main )->m_pFile )
                    {
                        desc = pShaderGroup->GetShader( ShaderPass_Main )->m_pFile->GetFilenameWithoutExtension();
                        buttonColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Button );
                        textColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Text );
                    }
                    
                    ImGui::PushStyleColor( ImGuiCol_Button, buttonColor );
                    ImGui::PushStyleColor( ImGuiCol_Text, textColor );
                    ImGui::Button( desc, ImVec2( ImGui::GetWindowWidth() * 0.65f, 0 ) );
                    ImGui::PopStyleColor( 2 );

                    if( ImGui::BeginDragDropTarget() )
                    {
                        if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "ShaderGroup" ) )
                        {
                            pMat->SetShader( (ShaderGroup*)*(void**)payload->Data );
                        }
                        ImGui::EndDragDropTarget();
                    }

                    if( pShaderGroup )
                    {
                        MyFileObjectShader* pFile = pShaderGroup->GetFile();
                        if( pFile )
                        {
                            if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                            {
                                FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();

                                if( ImGui::MenuItem( "Open File" ) )              { pFile->OnPopupClick( pFileManager, pFile, MyFileObject::RightClick_OpenFile );             ImGui::CloseCurrentPopup(); }
                                if( ImGui::MenuItem( "Open containing folder" ) ) { pFile->OnPopupClick( pFileManager, pFile, MyFileObject::RightClick_OpenContainingFolder ); ImGui::CloseCurrentPopup(); }
                                if( ImGui::MenuItem( "Find References" ) )        { pFile->OnPopupClick( pFileManager, pFile, MyFileObject::RightClick_FindAllReferences );    ImGui::CloseCurrentPopup(); } // (%d)", pMat->GetRefCount() ) {}
                                ImGui::EndPopup();
                            }
                        }
                    }

                    if( ImGui::IsItemHovered() )
                    {
                        if( ImGui::IsMouseDoubleClicked( 0 ) )
                        {
                            pMat->SetShader( nullptr );
                            pShaderGroup = pMat->GetShader();
                        }
                    }

                    if( pShaderGroup && pShaderGroup->GetFile() && pShaderGroup->GetFile()->m_NumExposedUniforms )
                    {
                        ImGui::SameLine();
                        if( ImGui::CollapsingHeader( "Shader" ) )
                        {
                            ImGui::Indent( 20 );
                            AddInlineMaterial( pMat );
                            ImGui::Unindent( 20 );
                        }
                    }
                    else
                    {
                        ImGui::SameLine();
                        ImGui::Text( "Shader" );
                    }

                    ImGui::PopID();
                }

                // Deal with the instanced shader attached to the material.
                {
                    ImGui::PushID( "Material_InstancedShader" );

                    const char* desc = "no instanced shader";

                    Vector4 buttonColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_UnsetObjectButton );
                    Vector4 textColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_UnsetObjectText );

                    if( pShaderGroupInstanced && pShaderGroupInstanced->GetShader( ShaderPass_Main )->m_pFile )
                    {
                        desc = pShaderGroupInstanced->GetShader( ShaderPass_Main )->m_pFile->GetFilenameWithoutExtension();
                        buttonColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Button );
                        textColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Text );
                    }

                    ImGui::PushStyleColor( ImGuiCol_Button, buttonColor );
                    ImGui::PushStyleColor( ImGuiCol_Text, textColor );
                    ImGui::Button( desc, ImVec2( ImGui::GetWindowWidth() * 0.65f, 0 ) );
                    ImGui::PopStyleColor( 2 );

                    if( ImGui::BeginDragDropTarget() )
                    {
                        if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "ShaderGroup" ) )
                        {
                            pMat->SetShaderInstanced( (ShaderGroup*)*(void**)payload->Data );
                        }
                        ImGui::EndDragDropTarget();
                    }

                    if( pShaderGroupInstanced )
                    {
                        MyFileObjectShader* pFile = pShaderGroupInstanced->GetFile();
                        if( pFile )
                        {
                            if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                            {
                                FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();

                                if( ImGui::MenuItem( "Open File" ) )              { pFile->OnPopupClick( pFileManager, pFile, MyFileObject::RightClick_OpenFile );             ImGui::CloseCurrentPopup(); }
                                if( ImGui::MenuItem( "Open containing folder" ) ) { pFile->OnPopupClick( pFileManager, pFile, MyFileObject::RightClick_OpenContainingFolder ); ImGui::CloseCurrentPopup(); }
                                if( ImGui::MenuItem( "Find References" ) )        { pFile->OnPopupClick( pFileManager, pFile, MyFileObject::RightClick_FindAllReferences );    ImGui::CloseCurrentPopup(); } // (%d)", pMat->GetRefCount() ) {}
                                ImGui::EndPopup();
                            }
                        }
                    }

                    if( ImGui::IsItemHovered() )
                    {
                        if( ImGui::IsMouseDoubleClicked( 0 ) )
                        {
                            pMat->SetShaderInstanced( nullptr );
                            pShaderGroupInstanced = pMat->GetShaderInstanced();
                        }
                    }

                    // TODO: Instanced shaders can't use exposed uniforms.
                    //if( pShaderGroupInstanced && pShaderGroupInstanced->GetFile() && pShaderGroupInstanced->GetFile()->m_NumExposedUniforms )
                    //{
                    //    ImGui::SameLine();
                    //    if( ImGui::CollapsingHeader( "InstancedShader" ) )
                    //    {
                    //        ImGui::Indent( 20 );
                    //        AddInlineMaterial( pMat );
                    //        ImGui::Unindent( 20 );
                    //    }
                    //}
                    //else
                    {
                        ImGui::SameLine();
                        ImGui::Text( "InstancedShader" );
                    }

                    ImGui::PopID();
                }

                {
                    const char* desc = "no color texture";

                    Vector4 buttonColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_UnsetObjectButton );
                    Vector4 textColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_UnsetObjectText );

                    TextureDefinition* pTextureColor = pMat->GetTextureColor();
                    if( pTextureColor )
                    {
                        desc = pTextureColor->GetFilename();

                        buttonColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Button );
                        textColor = pEditorPrefs->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Text );
                    }

                    ImGui::PushStyleColor( ImGuiCol_Button, buttonColor );
                    ImGui::PushStyleColor( ImGuiCol_Text, textColor );
                    ImGui::Button( desc, ImVec2( ImGui::GetWindowWidth() * 0.65f, 0 ) );
                    ImGui::PopStyleColor( 2 );

                    if( ImGui::BeginDragDropTarget() )
                    {
                        if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "Texture" ) )
                        {
                            pMat->SetTextureColor( (TextureDefinition*)*(void**)payload->Data );
                        }
                        ImGui::EndDragDropTarget();
                    }

                    if( ImGui::IsItemHovered() )
                    {
                        if( pTextureColor )
                        {
                            ImGui::BeginTooltip();
                            //ImGui::Text( "%s", pTex->GetFilename() );
                            AddTexturePreview( pTextureColor, false, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 1 ) );
                            ImGui::EndTooltip();
                        }

                        if( ImGui::IsMouseDoubleClicked( 0 ) )
                        {
                            pMat->SetTextureColor( nullptr );
                        }
                    }

                    ImGui::SameLine();
                    ImGui::Text( "Color Texture" );
                    //g_pPanelWatch->AddPointerWithDescription( "Color Texture", 0, desc, this, MaterialDefinition::StaticOnDropTexture, 0, MaterialDefinition::StaticOnRightClickTexture );
                }

                // TODO: Copies of these colors are changing, fix that.
                ColorFloat ambientColorFloat = pMat->GetColorAmbient().AsColorFloat();
                if( ImGui::ColorEdit4( "Ambient Color", &ambientColorFloat.r ) )
                {
                    pMat->SetColorAmbient( ambientColorFloat.AsColorByte() );
                }

                ColorFloat diffuseColorFloat = pMat->GetColorDiffuse().AsColorFloat();
                if( ImGui::ColorEdit4( "Diffuse Color", &diffuseColorFloat.r ) )
                {
                    pMat->SetColorDiffuse( diffuseColorFloat.AsColorByte() );
                }

                ColorFloat specularColorFloat = pMat->GetColorSpecular().AsColorFloat();
                if( ImGui::ColorEdit4( "Specular Color", &specularColorFloat.r ) )
                {
                    pMat->SetColorSpecular( specularColorFloat.AsColorByte() );
                }

                if( ImGui::DragFloat( "Shininess", &pMat->m_Shininess ) )
                {
                    pMat->MarkDirty();
                }
            }

            // Preview settings.
            {
                ImGui::Separator();

                MyAssert( MaterialDefinition::PreviewType_NumTypes == 2 );
                const char* items[] = { "Sphere", "Flat" };

                int currentItem = pMat->GetPreviewType();
                const char* currentItemStr = items[currentItem];
                if( ImGui::BeginCombo( "Preview Type", currentItemStr ) )
                {
                    for( int n = 0; n < MaterialDefinition::PreviewType_NumTypes; n++ )
                    {
                        bool isSelected = (n == currentItem);
                        if( ImGui::Selectable( items[n], isSelected ) )
                        {
                            pMat->SetPreviewType( (MaterialDefinition::PreviewType)n );
                        }
                        if( isSelected )
                        {
                            // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            }

            {
                ImGui::Separator();
                ImGui::Text( "MANUAL SAVE" );
                if( ImGui::Button( "Save" ) )
                {
                    pMat->SaveMaterial( nullptr );
                }
                ImGui::SameLine();
                ImGui::Text( "<- MANUAL SAVE" );
            }
        }
    }

    ImGui::End();
}

void EditorMainFrame_ImGui::Add2DAnimationEditor()
{
    if( m_p2DAnimInfoBeingEdited != nullptr )
    {
        m_FullPathToLast2DAnimInfoBeingEdited[0] = '\0';
    }

    if( m_FullPathToLast2DAnimInfoBeingEdited[0] != '\0' )
    {
        MyFileInfo* pFileInfo = g_pComponentSystemManager->GetFileInfoIfUsedByScene( m_FullPathToLast2DAnimInfoBeingEdited, SCENEID_Any );
        if( pFileInfo )
        {
            My2DAnimInfo* pAnimInfo = pFileInfo->Get2DAnimInfo();
            m_pCurrentLayout->m_IsWindowOpen[EditorWindow_2DAnimationEditor] = true;
            m_Current2DAnimationIndex = 0;

            MyFileObject* pFile = pAnimInfo->GetSourceFile();
            if( pFile && pFile->IsFinishedLoading() )
            {
                m_p2DAnimInfoBeingEdited = pAnimInfo;
                m_FullPathToLast2DAnimInfoBeingEdited[0] = '\0';
            }
        }
    }

    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_2DAnimationEditor] == false )
        return;

    ImGui::SetNextWindowPos( ImVec2(556, 71), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowSize( ImVec2(339, 349), ImGuiCond_FirstUseEver );
    
    if( ImGui::Begin( "2D Animation Editor", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_2DAnimationEditor] ) )
    {
        // Create a context menu only available from the title bar.
        if( ImGui::BeginPopupContextItem() )
        {
            if( ImGui::MenuItem( "Close" ) )
            {
                m_p2DAnimInfoBeingEdited = nullptr;
                m_pCurrentLayout->m_IsWindowOpen[EditorWindow_2DAnimationEditor] = false;
            }

            ImGui::EndPopup();
        }

        My2DAnimInfo* pAnimInfo = m_p2DAnimInfoBeingEdited;

        if( pAnimInfo == nullptr )
        {
            if( m_FullPathToLast2DAnimInfoBeingEdited[0] == '\0' )
                ImGui::Text( "No animation selected" );
            else
                ImGui::Text( "File still loading: %s", m_FullPathToLast2DAnimInfoBeingEdited );
        }
        else
        {
            m_pAnimPlayerComponent->SetAnimationFile( pAnimInfo->GetSourceFile() );
            m_pAnimPlayerComponent->SetCurrentAnimation( m_Current2DAnimationIndex );

            ImGui::Columns( 2, nullptr, false );

            {
                ImGui::Text( "WORK IN PROGRESS - NO UNDO - MANUAL SAVE" );
                if( ImGui::Button( "Save" ) )
                {
                    pAnimInfo->SaveAnimationControlFile();
                }
                ImGui::SameLine();
                ImGui::Text( "<- MANUAL SAVE" );
            }

            ImGui::NextColumn();

            if( pAnimInfo->GetNumberOfAnimations() > 0 )
            {
                uint32 frameIndex = m_pAnimPlayerComponent->GetCurrentFrameIndex();
                My2DAnimation* pAnim = pAnimInfo->GetAnimationByIndex( m_Current2DAnimationIndex );
                if( pAnim->GetFrameCount() > 0 )
                {
                    My2DAnimationFrame* pFrame = pAnim->GetFrameByIndex( frameIndex );

                    MaterialDefinition* pMat = pFrame->m_pMaterial;

                    AddMaterialColorTexturePreview( pMat, false, ImVec2( 50, 50 ), ImVec4( 1, 1, 1, 1 ) );

                    ImGui::SameLine();

                    ImGui::Text( "FrameIndex: %d", frameIndex );

                    ImGui::SameLine();
                    
                    if( ImGui::Button( "Remove Animation" ) )
                    {
                        pAnimInfo->OnRemoveAnimationPressed( m_Current2DAnimationIndex );
                        
                        if( m_Current2DAnimationIndex > pAnimInfo->GetNumberOfAnimations() )
                            m_Current2DAnimationIndex--;
                    }
                }
            }

            ImGui::Columns( 1 );

            ImGui::Separator();

            ImGui::BeginChild( "Animation Details" );

            ImGui::Text( "%s Animations:", pAnimInfo->GetSourceFile()->GetFilenameWithoutExtension() );
            ImGui::Columns( 3, nullptr, false );

            // First Column: Animations.
            {
                for( unsigned int animIndex=0; animIndex<pAnimInfo->GetNumberOfAnimations(); animIndex++ )
                {
                    My2DAnimation* pAnim = pAnimInfo->GetAnimationByIndex( animIndex );

                    if( ImGui::Selectable( pAnim->GetName(), m_Current2DAnimationIndex == animIndex ) )
                    {
                        m_Current2DAnimationIndex = animIndex;
                    }
                }

                if( pAnimInfo->GetNumberOfAnimations() < My2DAnimInfo::MAX_ANIMATIONS )
                {
                    if( ImGui::Button( "Add Animation" ) )
                    {
                        pAnimInfo->OnAddAnimationPressed();
                    }
                }
            }

            ImGui::NextColumn();

            // Second Column: Frame durations of currently selected animation.
            if( pAnimInfo->GetNumberOfAnimations() > 0 )
            {
                int animIndex = m_Current2DAnimationIndex;

                My2DAnimation* pAnim = pAnimInfo->GetAnimationByIndex( animIndex );
                //ImGui::Text( pAnim->GetName() );

                unsigned int numframes = pAnim->GetFrameCount();

                for( unsigned int frameIndex=0; frameIndex<numframes; frameIndex++ )
                {
                    My2DAnimationFrame* pFrame = pAnim->GetFrameByIndex( frameIndex );

                    ImGui::PushID( pFrame );

                    ImGui::Text( "Frame %d", frameIndex );
                    ImGui::SliderFloat( "Duration", &pFrame->m_Duration, 0, 1 );

                    if( ImGui::Button( "Remove" ) )
                    {
                        pAnimInfo->OnRemoveFramePressed( animIndex, frameIndex );
                        numframes = pAnim->GetFrameCount();
                    }

                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::Spacing();
                    //ImGui::Text( "---------" );

                    ImGui::PopID();
                }

                if( pAnim->GetFrameCount() < My2DAnimInfo::MAX_FRAMES_IN_ANIMATION )
                {
                    if( ImGui::Button( "Add Frame" ) )
                    {
                        pAnimInfo->OnAddFramePressed( animIndex );
                    }
                }
            }

            ImGui::NextColumn();

            // Third Column: Material texture previews.
            if( pAnimInfo->GetNumberOfAnimations() > 0 )
            {
                int animindex = m_Current2DAnimationIndex;

                My2DAnimation* pAnim = pAnimInfo->GetAnimationByIndex( animindex );

                unsigned int numframes = pAnim->GetFrameCount();

                for( unsigned int frameindex=0; frameindex<numframes; frameindex++ )
                {
                    My2DAnimationFrame* pFrame = pAnim->GetFrameByIndex( frameindex );

                    ImGui::PushID( pFrame );

                    MaterialDefinition* pMat = pFrame->m_pMaterial;

                    if( pMat != nullptr )
                    {
                        ImGui::Text( "%s", pMat->GetName() );
                    }
                    else
                    {
                        ImGui::Text( "No Material Assigned" );
                    }

                    AddMaterialColorTexturePreview( pMat, false, ImVec2( 50, 50 ), ImVec4( 1, 1, 1, 1 ) );

                    if( ImGui::BeginDragDropTarget() )
                    {
                        if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "Material" ) )
                        {
                            pFrame->SetMaterial( (MaterialDefinition*)*(void**)payload->Data );
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::Spacing();

                    ImGui::PopID();
                }
            }

            ImGui::Columns( 1 );

            {
                ImGui::Separator();
                ImGui::Text( "MANUAL SAVE" );
                if( ImGui::Button( "Save" ) )
                {
                    pAnimInfo->SaveAnimationControlFile();
                }
                ImGui::SameLine();
                ImGui::Text( "<- MANUAL SAVE" );
            }

            ImGui::EndChild();
        }
    }

    ImGui::End();
}

void EditorMainFrame_ImGui::AddMaterialPreview(MaterialDefinition* pMaterial, bool createWindow, ImVec2 requestedSize, ImVec4 tint)
{
    m_pMaterialToPreview = pMaterial;

    if( createWindow == false || ImGui::Begin( "Material", nullptr, ImVec2(requestedSize.x+50, requestedSize.y+50), 1 ) )
    {
        if( m_pMaterialToPreview->GetPreviewType() == MaterialDefinition::PreviewType_Sphere )
        {
            TextureDefinition* pTexture = pTexture = m_pMaterialPreviewFBO->GetColorTexture( 0 );

            int texw = m_pMaterialPreviewFBO->GetTextureWidth();
            int texh = m_pMaterialPreviewFBO->GetTextureHeight();

            ImVec2 size = requestedSize;
            if( size.x == 0 )
                size = ImGui::GetContentRegionAvail();
            if( size.x > size.y ) size.x = size.y;
            if( size.y > size.x ) size.y = size.x;

            if( pTexture )
            {
                int w = pTexture->GetWidth();
                int h = pTexture->GetHeight();
                //ImGui::ImageButton( (void*)pTexture, size, ImVec2(0,(float)h/texh), ImVec2((float)w/texw,0), -1, ImVec4(0,0,0,1) );
                ImGui::Image( (void*)pTexture, size, ImVec2(0,(float)h/texh), ImVec2((float)w/texw,0), tint );
            }
        }
        else //if( m_pMaterialToPreview->GetPreviewType() == MaterialDefinition::PreviewType_Flat )
        {
            AddMaterialColorTexturePreview( m_pMaterialToPreview, false, requestedSize, tint );
        }
    }

    if( createWindow == true )
    {
        ImGui::End(); // ImGui::Begin( "Material"...
    }
}

void EditorMainFrame_ImGui::AddMaterialColorTexturePreview(MaterialDefinition* pMaterial, bool createWindow, ImVec2 requestedSize, ImVec4 tint)
{
    if( pMaterial == nullptr )
    {
        AddTexturePreview( nullptr, false, requestedSize, ImVec4( 1, 1, 1, 1 ), ImVec2( 0, 0 ), ImVec2( 0, 0 ) );
    }
    else
    {
        ImVec2 startUV( pMaterial->GetUVOffset() );
        ImVec2 endUV( pMaterial->GetUVOffset() + pMaterial->GetUVScale() );
        AddTexturePreview( pMaterial->GetTextureColor(), false, requestedSize, ImVec4( 1, 1, 1, 1 ), startUV, endUV );
    }
}

void EditorMainFrame_ImGui::AddTexturePreview(TextureDefinition* pTexture, bool createWindow, ImVec2 requestedSize, ImVec4 tint, ImVec2 startUV, ImVec2 endUV)
{
    if( createWindow == false || ImGui::Begin( "Texture", nullptr, ImVec2(150, 150), 1 ) )
    {
        ImVec2 size = requestedSize;
        if( size.x == 0 )
            size = ImGui::GetContentRegionAvail();
        if( size.x > size.y ) size.x = size.y;
        if( size.y > size.x ) size.y = size.x;

        if( pTexture != nullptr )
        {
            ImGui::Image( (void*)pTexture, size, startUV, endUV, tint );
        }
        else
        {
            ImGui::Image( 0, size, startUV, endUV, tint );
        }
    }

    if( createWindow == true )
    {
        ImGui::End(); // ImGui::Begin( "Texture"...
    }
}

void EditorMainFrame_ImGui::AddDebug_MousePicker()
{
    if( m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Debug_MousePicker] == false )
        return;

    if( ImGui::Begin( "Mouse Picker", &m_pCurrentLayout->m_IsWindowOpen[EditorWindow_Debug_MousePicker], ImVec2(150, 150), 1 ) )
    {
        TextureDefinition* pTexture = m_pEngineCore->GetEditorState()->m_pMousePickerFBO->GetColorTexture( 0 );
        int texw = m_pEngineCore->GetEditorState()->m_pMousePickerFBO->GetTextureWidth();
        int texh = m_pEngineCore->GetEditorState()->m_pMousePickerFBO->GetTextureHeight();

        ImVec2 size = ImGui::GetContentRegionAvail();
        if( size.x > size.y ) size.x = size.y;
        if( size.y > size.x ) size.y = size.x;

        if( pTexture )
        {
            //int w = pTexture->GetWidth();
            //int h = pTexture->GetHeight();
            int w = m_pEngineCore->GetEditorState()->m_pMousePickerFBO->GetWidth();
            int h = m_pEngineCore->GetEditorState()->m_pMousePickerFBO->GetHeight();
            ImGui::Image( (void*)pTexture, size, ImVec2(0,(float)h/texh), ImVec2((float)w/texw,0) );
        }
    }
    ImGui::End();
}

void OnDropSoundCueOnEditorWindow(EngineCore* pEngineCore, SoundCue* pSoundCue)
{
    if( pSoundCue )
    {
        // Create a new GameObject with an audio player component.
        GameObject* pGameObjectCreated = g_pComponentSystemManager->CreateGameObject( false, SCENEID_MainScene );
        pGameObjectCreated->SetName( pSoundCue->GetName() );
        ComponentAudioPlayer* pComponent = (ComponentAudioPlayer*)pGameObjectCreated->AddNewComponent( ComponentType_AudioPlayer, SCENEID_MainScene, g_pComponentSystemManager );
        pComponent->SetSoundCue( pSoundCue );

        // Add it to the undo stack.
        g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_CreateGameObject( pGameObjectCreated ) );

        // Clear the selected objects and select the new one.
        pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();
        pEngineCore->GetEditorState()->SelectGameObject( pGameObjectCreated );
    }
}

bool EditorMainFrame_ImGui::OnDropObjectList(GameObject* pGameObject, bool forceReorder)
{
    bool dragDropPayloadAcceptedOnRelease = false;
    bool dragDropOfItemWillResultInAReorder = false;

    EditorState* pEditorState = m_pEngineCore->GetEditorState();

    if( ImGui::BeginDragDropTarget() )
    {
        if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "GameObject", ImGuiDragDropFlags_AcceptPeekOnly ) )
        {
            // Clear the "reorder" line.
            m_pGameObjectToDrawReorderLineAfter = nullptr;

            // If there's a drag/drop payload and it's a gameobject, then:
            //     if we're hovering over the top half of the item, reparent the dropped item.
            //     if we're hovering over the bottom half, reorder the dropped item after the hovered item.
            if( forceReorder || (ImGui::GetMousePos().y > ImGui::GetItemRectMin().y + (ImGui::GetItemRectSize().y * 0.5f)) )
            {
                dragDropOfItemWillResultInAReorder = true;
                m_pGameObjectToDrawReorderLineAfter = pGameObject;
            }
        }

        ImGuiDragDropFlags dropFlags = 0;
        if( dragDropOfItemWillResultInAReorder )
            dropFlags = ImGuiDragDropFlags_AcceptNoDrawDefaultRect;

        if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "GameObject", dropFlags ) )
        {
            // Releasing the mouse will drop the payload, but we need prevent this from selecting the item.
            dragDropPayloadAcceptedOnRelease = true;

            g_DragAndDropStruct.Clear();

            GameObject* pDroppedGO = (GameObject*)*(void**)payload->Data;
            if( pEditorState->IsGameObjectSelected( pDroppedGO ) == false )
            {
                // If this GameObject wasn't selected, then only move this one.
                g_DragAndDropStruct.Add( DragAndDropType_GameObjectPointer, pDroppedGO );
            }
            else
            {
                // If it was selected, move all selected objects.
                for( unsigned int i=0; i<pEditorState->m_pSelectedObjects.size(); i++ )
                {
                    g_DragAndDropStruct.Add( DragAndDropType_GameObjectPointer, pEditorState->m_pSelectedObjects[i] );
                }
            }

            if( dragDropOfItemWillResultInAReorder )
            {
                MyAssert( pGameObject != nullptr );

                pGameObject->OnDrop( -1, -1, -1, GameObject::GameObjectOnDropAction_Reorder );
                m_pGameObjectToDrawReorderLineAfter = nullptr;
            }
            else
            {
                pGameObject->OnDrop( -1, -1, -1, GameObject::GameObjectOnDropAction_Reparent );
            }
        }

        if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "SoundCue", 0 ) )
        {
            SoundCue* pSoundCue = (SoundCue*)*(void**)payload->Data;
            MyAssert( pSoundCue != nullptr );

            if( pSoundCue )
            {
                // Create an audio player component.
                EditorCommand_CreateComponent* pCommand = MyNew EditorCommand_CreateComponent( pGameObject, ComponentType_AudioPlayer );
                g_pGameCore->GetCommandStack()->Do( pCommand );
                ComponentAudioPlayer* pComponent = (ComponentAudioPlayer*)pCommand->GetCreatedObject();

                // Attach the correct sound cue.
                pComponent->SetSoundCue( pSoundCue );
            }
        }

        ImGui::EndDragDropTarget();
    }

    return dragDropPayloadAcceptedOnRelease;
}

void EditorMainFrame_ImGui::OnDropEditorWindow()
{
    unsigned int x = m_CurrentMouseInEditorWindow_X;
    unsigned int y = m_CurrentMouseInEditorWindow_Y;

    // Get the GameObject the mouse was hovering over.
    ComponentCamera* pCamera = m_pEngineCore->GetEditorState()->GetEditorCamera();
    //y = pCamera->m_WindowHeight - y; // prefer 0,0 at bottom left.
    GameObject* pObjectDroppedOn = m_pEngineCore->GetCurrentEditorInterface()->GetObjectAtPixel( x, y, true, false );

    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "Material" ) )
    {
        //DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

        //if( pDropItem->m_Type == DragAndDropType_MaterialDefinitionPointer )
        {
            MaterialDefinition* pMaterial = (MaterialDefinition*)*(void**)payload->Data; //pDropItem->m_Value;

            if( pMaterial && pObjectDroppedOn )
            {
                pObjectDroppedOn->Editor_SetMaterial( pMaterial );

#if MYFW_USING_WX
                pObjectDroppedOn->Editor_SetMaterial( pMaterial );
                g_pPanelWatch->SetNeedsRefresh();
#endif
            }
        }
    }

    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "Texture" ) )
    {
        //if( pDropItem->m_Type == DragAndDropType_TextureDefinitionPointer )
        {
            TextureDefinition* pTexture = (TextureDefinition*)*(void**)payload->Data; //pDropItem->m_Value;

            if( pTexture && pObjectDroppedOn && pObjectDroppedOn->GetMaterial() )
            {
                g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ChangeTextureOnMaterial( pObjectDroppedOn->GetMaterial(), pTexture ) );
            }
        }
    }

    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "ShaderGroup" ) )
    {
        //if( pDropItem->m_Type == DragAndDropType_ShaderGroupPointer )
        {
            ShaderGroup* pShader = (ShaderGroup*)*(void**)payload->Data; //pDropItem->m_Value;

            if( pShader && pObjectDroppedOn && pObjectDroppedOn->GetMaterial() )
            {
                g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ChangeShaderOnMaterial( pObjectDroppedOn->GetMaterial(), pShader ) );
            }
        }
    }

    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "SoundCue" ) )
    {
        SoundCue* pSoundCue = (SoundCue*)*(void**)payload->Data;
        OnDropSoundCueOnEditorWindow( m_pEngineCore, pSoundCue );
    }

    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "File" ) )
    {
        //if( pDropItem->m_Type == DragAndDropType_FileObjectPointer )
        {
            MyFileObject* pFile = (MyFileObject*)*(void**)payload->Data; //pDropItem->m_Value;
            MyAssert( pFile );

            if( pFile && strcmp( pFile->GetExtensionWithDot(), ".lua" ) == 0 )
            {
                if( pObjectDroppedOn )
                {
                    g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ChangeAllScriptsOnGameObject( pObjectDroppedOn, pFile ) );
                }
            }

            if( pFile && strcmp( pFile->GetExtensionWithDot(), ".glsl" ) == 0 )
            {
                if( pObjectDroppedOn && pObjectDroppedOn->GetMaterial() )
                {
                    ShaderGroup* pShader = m_pEngineCore->GetManagers()->GetShaderGroupManager()->FindShaderGroupByFile( pFile );
                    g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ChangeShaderOnMaterial( pObjectDroppedOn->GetMaterial(), pShader ) );
                }
            }

            if( pFile && strcmp( pFile->GetExtensionWithDot(), ".mycue" ) == 0 )
            {
                SoundCue* pSoundCue = g_pGameCore->GetSoundManager()->FindCueByFilename( pFile->GetFullPath() );
                OnDropSoundCueOnEditorWindow( m_pEngineCore, pSoundCue );
            }

            if( pFile &&
                ( strcmp( pFile->GetExtensionWithDot(), ".obj" ) == 0 || strcmp( pFile->GetExtensionWithDot(), ".mymesh" ) == 0 )
              )
            {
                // Create a new gameobject using this obj.
                MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();
                MeshManager* pMeshManager = m_pEngineCore->GetManagers()->GetMeshManager();
                MyMesh* pMesh = pMeshManager->FindMeshBySourceFile( pFile );

                GameObject* pGameObject = g_pComponentSystemManager->CreateGameObject( true, SCENEID_MainScene );
                pGameObject->SetName( "New mesh" );
                ComponentMeshOBJ* pComponentMeshOBJ = (ComponentMeshOBJ*)pGameObject->AddNewComponent( ComponentType_MeshOBJ, SCENEID_MainScene, g_pComponentSystemManager );
                pComponentMeshOBJ->SetSceneID( SCENEID_MainScene );
                pComponentMeshOBJ->SetMaterial( pMaterialManager->GetFirstMaterial(), 0 );
                pComponentMeshOBJ->SetMesh( pMesh );
                pComponentMeshOBJ->SetLayersThisExistsOn( Layer_MainScene );

                if( pObjectDroppedOn && pObjectDroppedOn->GetMaterial() )
                {
                    // Place it just above of the object selected otherwise place it at 0,0,0... for now.
                    Vector3 pos = pObjectDroppedOn->GetTransform()->GetWorldPosition();

                    ComponentRenderable* pComponentMeshDroppedOn = (ComponentRenderable*)pObjectDroppedOn->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
                    if( pComponentMeshDroppedOn && pComponentMeshDroppedOn->GetBounds() != nullptr )
                    {
                        pos.y += pComponentMeshDroppedOn->GetBounds()->GetHalfSize().y;
                        pos.y += pMesh->GetBounds()->GetHalfSize().y;
                    }

                    pGameObject->GetTransform()->SetWorldPosition( pos );
                    pGameObject->GetTransform()->UpdateTransform();
                }

                // Undo/redo.
                g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_CreateGameObject( pGameObject ) );
            }
        }
    }

    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "Prefab" ) )
    {
        PrefabObject* pPrefab = (PrefabObject*)*(void**)payload->Data;

        // Default to drop into main scene, but prefer putting in same scene as the object dropped on.
        SceneID sceneid = SCENEID_MainScene;
        if( pObjectDroppedOn )
        {
            sceneid = pObjectDroppedOn->GetSceneID();
        }

        // Create the game object.
        GameObject* pGameObjectCreated = g_pComponentSystemManager->CreateGameObjectFromPrefab( pPrefab, true, sceneid );

        if( pGameObjectCreated )
        {
            // Undo/Redo.
            g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_CreateGameObject( pGameObjectCreated ) );

            // Select the object dropped.
            m_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();
            m_pEngineCore->GetEditorState()->SelectGameObject( pGameObjectCreated );

            // Move the new object to the same spot as the one it was dropped on.
            if( pObjectDroppedOn )
            {
                std::vector<GameObject*> selectedobjects;
                selectedobjects.push_back( pGameObjectCreated );
                Vector3 worldPos = pObjectDroppedOn->GetTransform()->GetWorldPosition();
                g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_MoveObjects( worldPos, selectedobjects ), true );
            }
        }
    }

    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "GameObject" ) )
    {
        //if( (int)pDropItem->m_Type == (int)DragAndDropType_GameObjectPointer )
        //{
        //    GameObject* pGameObject = (GameObject*)*(void**)payload->Data; //pDropItem->m_Value;
        //    MyAssert( pGameObject );

        //    int id = g_DragAndDropStruct.GetControlID() - m_ControlIDOfFirstExtern;
        //    
        //    // TODO: this will make a mess of memory if different types of objects can be dragged in...
        //    m_ExposedVars[id]->pointer = pGameObject;

        //    // update the panel so new gameobject name shows up.
        //    g_pPanelWatch->GetVariableProperties( g_DragAndDropStruct.GetControlID() )->m_Description = pGameObject->GetName();
        //}
    }
}
