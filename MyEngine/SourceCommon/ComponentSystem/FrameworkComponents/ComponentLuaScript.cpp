//
// Copyright (c) 2014-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentLuaScript.h"
#include "ComponentSystem/Core/EngineFileManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineComponentTypeManager.h"
#include "Core/EngineCore.h"
#include "../../../SourceEditor/PlatformSpecific/FileOpenDialog.h"

#if MYFW_EDITOR
#include "GUI/ImGuiExtensions.h"
#include "../SourceEditor/EngineEditorCommands.h"
#endif

#if MYFW_USING_LUA

// Component Variable List.
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentLuaScript ); //_VARIABLE_LIST

ComponentLuaScript::ComponentLuaScript(ComponentSystemManager* pComponentSystemManager)
: ComponentUpdateable( pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Updateable;
    m_Type = ComponentType_LuaScript;

    m_LuaGameObjectName[0] = '\0';
    m_pCopyExternsFromThisComponentAfterLoadingScript = nullptr;

    m_pScriptFile = nullptr;
#if !MYFW_EDITOR
    m_pLuaInlineScript_OnPlay = nullptr;
#endif

    m_ExposedVars.AllocateObjects( MAX_EXPOSED_VARS ); // Hard coded nonsense for now, max of 4 exposed vars in a script.

#if MYFW_EDITOR
    m_ValueWhenControlSelected = 0;
    m_ImGuiControlIDForCurrentlySelectedVariable = -1;
    m_LinkNextUndoCommandToPrevious = false;
#endif

    m_pComponentSystemManager->Editor_RegisterFileUpdatedCallback( &StaticOnFileUpdated, this );
}

ComponentLuaScript::~ComponentLuaScript()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    while( m_ExposedVars.Count() )
    {
        ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex( 0 );

        // Unregister gameobject deleted callback, if we registered one.
        if( pVariable->type == ExposedVariableType_GameObject && pVariable->pointer )
            static_cast<GameObject*>( pVariable->pointer )->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

        delete pVariable;
    }

#if !MYFW_EDITOR
    delete[] m_pLuaInlineScript_OnPlay;
#endif

    SAFE_RELEASE( m_pScriptFile );
}

void ComponentLuaScript::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentLuaScript* pThis) //_VARIABLE_LIST
{
    // Just want to make sure these are the same on all compilers.  They should be since this is a simple class.
#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
    MyAssert( offsetof( ComponentLuaScript, m_pScriptFile ) == MyOffsetOf( pThis, &pThis->m_pScriptFile ) );
#if __GNUC__
#pragma GCC diagnostic pop
#endif

    // Script is not automatically saved/loaded.
    ComponentVariable* pVar = AddVar( pList, "Script", ComponentVariableType_FilePtr, MyOffsetOf( pThis, &pThis->m_pScriptFile ), false, true, nullptr, (CVarFunc_ValueChanged)&ComponentLuaScript::OnValueChanged, (CVarFunc_DropTarget)&ComponentLuaScript::OnDrop, nullptr );
#if MYFW_USING_IMGUI
    pVar->AddCallback_OnRightClick( (CVarFunc)&ComponentLuaScript::OnRightClickCallback, nullptr );
#endif

    // m_pLuaInlineScript_OnPlay is not automatically saved/loaded.
    pVar = AddVarPointer( pList, "OnPlay", false, true, nullptr, 
        (CVarFunc_GetPointerValue)&ComponentLuaScript::GetPointerValue, (CVarFunc_SetPointerValue)&ComponentLuaScript::SetPointerValue,
        (CVarFunc_GetPointerDesc)&ComponentLuaScript::GetPointerDesc, (CVarFunc_SetPointerDesc)&ComponentLuaScript::SetPointerDesc,
        (CVarFunc_ValueChanged)&ComponentLuaScript::OnValueChanged, nullptr, nullptr );
}

void ComponentLuaScript::Reset()
{
    ComponentUpdateable::Reset();

    m_pLuaGameState = g_pLuaGameState;

    m_ScriptLoaded = false;
    m_Playing = false;
    m_ErrorInScript = false;
    m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading = false;

    m_pCopyExternsFromThisComponentAfterLoadingScript = nullptr;

    while( m_ExposedVars.Count() )
    {
        ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex( 0 );

        // Unregister gameobject deleted callback, if we registered one.
        if( pVariable->type == ExposedVariableType_GameObject && pVariable->pointer )
            static_cast<GameObject*>( pVariable->pointer )->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

        delete pVariable;
    }
}

#if MYFW_USING_LUA
void ComponentLuaScript::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentLuaScript>( "ComponentLuaScript" )
            .addFunction( "SetScriptFile", &ComponentLuaScript::SetScriptFile ) // void ComponentLuaScript::SetScriptFile(MyFileObject* script)
            .addFunction( "SetExternFloat", &ComponentLuaScript::SetExternFloat ) // void ComponentLuaScript::SetExternFloat(const char* name, float newValue)
        .endClass();
}
#endif //MYFW_USING_LUA

void* ComponentLuaScript::GetPointerValue(ComponentVariable* pVar) //_VARIABLE_LIST
{
    //if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    //{
    //}

    return nullptr;
}

void ComponentLuaScript::SetPointerValue(ComponentVariable* pVar, const void* newvalue) //_VARIABLE_LIST
{
    //if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    //{
    //}
}

const char* ComponentLuaScript::GetPointerDesc(ComponentVariable* pVar) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    {
#if MYFW_EDITOR
        return m_pLuaInlineScript_OnPlay.c_str();
#endif
    }

    return "fix me";
}

void ComponentLuaScript::SetPointerDesc(ComponentVariable* pVar, const char* newdesc) //_VARIABLE_LIST
{
    //if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    //{
    //}
}

#if MYFW_EDITOR
void ComponentLuaScript::CreateNewScriptFile()
{
    //if( m_pScriptFile == nullptr )
    {
        // Generally offer to create scripts in Scripts folder.
        const char* initialPath = "Data\\Scripts\\";

        bool isMenuActionScript = false;
        bool isMeshScript = false;

        // If a ComponentMenuPage is attached to this game object, then offer to make the file in the menus folder.
        if( m_pGameObject->GetFirstComponentOfType( "MenuPageComponent" ) != nullptr )
        {
            isMenuActionScript = true;
            initialPath = "Data\\Menus\\";
        }

        // If a ComponentMesh is attached to this game object, then add a SetupCustomUniforms callback.
        if( m_pGameObject->GetFirstComponentOfType( "MeshComponent" ) != nullptr )
        {
            isMeshScript = true;
        }

        const char* filename = FileSaveDialog( initialPath, "Lua script files\0*.lua\0All\0*.*\0" );
        if( filename[0] != '\0' )
        {
            int len = (int)strlen( filename );

            // Append '.lua' to end of filename if it wasn't already there.
            char fullpath[MAX_PATH];
            if( len > 4 && strcmp( &filename[len-4], ".lua" ) == 0 )
            {
                strcpy_s( fullpath, MAX_PATH, filename );
            }
            else
            {
                sprintf_s( fullpath, MAX_PATH, "%s.lua", filename );
            }

            const char* relativepath = GetRelativePath( fullpath );

            MyFileObject* pScriptFile = m_pComponentSystemManager->LoadDataFile( relativepath, m_pGameObject->GetSceneID(), nullptr, true )->GetFile();
            SetScriptFile( pScriptFile );

            // TODO: Create external template files in the DataEngine folder.
            {
                FILE* file;
#if MYFW_WINDOWS
                fopen_s( &file, fullpath, "wb" );
#else
                file = fopen( fullpath, "wb" );
#endif

                if( file )
                {
                    if( isMenuActionScript )
                    {
                        fprintf( file, "-- Menu Action File\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "%s =\n", m_pScriptFile->GetFilenameWithoutExtension() );
                        fprintf( file, "{\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnVisible = function(visible)\n" );
                        fprintf( file, "end,\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnAction = function(action)\n" );
                        fprintf( file, "--LogInfo( \"OnAction was called: \" .. action .. \"\\n\" );\n" );
                        fprintf( file, "end\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "}\n" );
                    }
                    else
                    {
                        fprintf( file, "-- %s Script\n", m_pScriptFile->GetFilenameWithoutExtension() );
                        fprintf( file, "\n" );
                        fprintf( file, "%s =\n", m_pScriptFile->GetFilenameWithoutExtension() );
                        fprintf( file, "{\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnLoad = function()\n" );
                        fprintf( file, "end,\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnPlay = function()\n" );
                        fprintf( file, "end,\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnStop = function()\n" );
                        fprintf( file, "end,\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnTouch = function(action, id, x, y, pressure, size)\n" );
                        fprintf( file, "end,\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnButtons = function(action, id)\n" );
                        fprintf( file, "end,\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "Tick = function(deltaTime)\n" );

                        if( isMeshScript )
                        {
                            fprintf( file, "end,\n" );
                            fprintf( file, "\n" );
                            fprintf( file, "SetupCustomUniforms = function(programhandle)\n" );
                        }

                        fprintf( file, "end\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "}\n" );
                    }

                    fclose( file );
                }
            }

            m_pScriptFile->OSLaunchFile( true );
        }
    }
}

#if MYFW_USING_IMGUI
void TestForLuaExposedVariableModificationAndCreateUndoCommand(ComponentLuaScript* pComponent, ImGuiID id, bool modified, ExposedVariableDesc* pVar, double newValue)
{
    MyAssert( pComponent != nullptr );

    // If the id passed in is different than the last known value, then assume a new control was selected.
    if( id != pComponent->m_ImGuiControlIDForCurrentlySelectedVariable )
    {
        // If a new control was selected, store the starting value and start a new undo chain.
        pComponent->m_ValueWhenControlSelected = pVar->valuedouble;
        pComponent->m_LinkNextUndoCommandToPrevious = false;
        pComponent->m_ImGuiControlIDForCurrentlySelectedVariable = id;
    }

    // If the control returned true to indicate it was modified, then create an undo command.
    if( modified && id != 0 )
    {
        MyAssert( id == pComponent->m_ImGuiControlIDForCurrentlySelectedVariable );

        // Add an undo action.
        EditorCommand* pCommand = MyNew EditorCommand_LuaExposedVariableFloatChanged(
            newValue, pVar, ComponentLuaScript::StaticOnExposedVarValueChanged, pComponent );

        pComponent->GetComponentSystemManager()->GetEngineCore()->GetCommandStack()->Do( pCommand, pComponent->m_LinkNextUndoCommandToPrevious );

        // Link the next undo command to this one.
        // TODO: Since we're passing in the starting value,
        //       we can actually replace the old command rather than link to it.
        pComponent->m_LinkNextUndoCommandToPrevious = true;
    }
}

void ComponentLuaScript::AddAllVariablesToWatchPanel()
{
    ComponentBase::AddAllVariablesToWatchPanel();

    ImGui::Indent( 20 );

    if( ImGui::GetIO().MouseDown[0] == false )
    {
        m_LinkNextUndoCommandToPrevious = false;
    }

    // Add all component variables.
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];

        switch( pVar->type )
        {
        case ExposedVariableType_Unused:
            MyAssert( false );
            break;

        case ExposedVariableType_Float:
            {
                float tempFloat = (float)pVar->valuedouble;
                bool modified = ImGui::DragFloat( pVar->name.c_str(), &tempFloat, 0.1f );
                if( modified )
                {
                    TestForLuaExposedVariableModificationAndCreateUndoCommand( this, ImGuiExt::GetActiveItemId(), modified, pVar, tempFloat );
                }
            }
            break;

        case ExposedVariableType_Bool:
            {
                ImGui::Text( "(TODO) Bool: %s", pVar->name.c_str() );
                //id = g_pPanelWatch->AddBool( pVar->name.c_str(), &pVar->valuebool, 0, 0, this, ComponentLuaScript::StaticOnPanelWatchExposedVarValueChanged, ComponentLuaScript::StaticOnRightClickExposedVariable );
            }
            break;

        case ExposedVariableType_Vector3:
            {
                ImGui::Text( "(TODO) Vector3: %s", pVar->name.c_str() );
                //id = g_pPanelWatch->AddVector3( pVar->name.c_str(), (Vector3*)&pVar->valuevector3, 0, 0, this, ComponentLuaScript::StaticOnPanelWatchExposedVarValueChanged, ComponentLuaScript::StaticOnRightClickExposedVariable );
            }
            break;

        case ExposedVariableType_GameObject:
            {
                // Group the button and the label into one "control", will make right-click context menu work on button.
                ImGui::BeginGroup();

                GameObject* pGameObject = static_cast<GameObject*>( pVar->pointer );

                const char* pDesc = "none";
                if( pGameObject )
                {
                    pDesc = pGameObject->GetName();
                }

                float width = ImGui::GetWindowWidth() * 0.65f;
                if( ImGui::Button( pDesc, ImVec2( width, 0 ) ) )
                {
                    // TODO: Pop up a GameObject picker window.
                }

                if( ImGui::BeginDragDropTarget() )
                {
                    if( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "GameObject" ) )
                    {
                        // Set the new value.
                        GameObject* pDroppedGameObject = (GameObject*)*(void**)payload->Data;
                        m_pComponentSystemManager->GetEngineCore()->GetCommandStack()->Do( MyNew EditorCommand_LuaExposedVariablePointerChanged( pDroppedGameObject, pVar, ComponentLuaScript::StaticOnExposedVarValueChanged, this ), true );
                    }

                    ImGui::EndDragDropTarget();
                }

                ImGui::SameLine();
                ImGui::Text( pVar->name.c_str() );

                ImGui::EndGroup();

                // Right-click menu on group.
                ImGui::PushID( pVar );
                if( ImGui::BeginPopupContextItem( "ContextPopup", 1 ) )
                {
                    //// Set color to default, since it might be set to divorced color.
                    //Vector4 color = m_pComponentSystemManager->GetEngineCore()->GetEditorPrefs()->GetImGuiStylePrefs()->GetColor( ImGuiStylePrefs::StylePref_Color_Text );
                    //ImGui::PushStyleColor( ImGuiCol_Text, color );

                    if( ImGui::MenuItem( "Clear GameObject" ) )
                    {
                        // Set the new value.
                        GameObject* pNewGameObject = nullptr;
                        m_pComponentSystemManager->GetEngineCore()->GetCommandStack()->Do( MyNew EditorCommand_LuaExposedVariablePointerChanged(
                            pNewGameObject, pVar, ComponentLuaScript::StaticOnExposedVarValueChanged, this ), true );
                    }

                    //ImGui::PopStyleColor( 1 );

                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            break;
        }

        if( pVar->divorced )
        {
        }
    }

    ImGui::Unindent( 20 );
}
#endif

void ComponentLuaScript::OnFileUpdated(MyFileObject* pFile) // StaticOnFileUpdated
{
    if( pFile == m_pScriptFile )
    {
        m_ScriptLoaded = false;
        m_ErrorInScript = false;

        if( m_Playing )
        {
            m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading = true;
        }
    }
}

void* ComponentLuaScript::OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
{
    void* oldPointer = nullptr;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = static_cast<MyFileObject*>( pDropItem->m_Value );

        if( pFile == nullptr )
        {
            oldPointer = m_pScriptFile;
            ClearExposedVariableList( changedByInterface ? true : false );
            SetScriptFile( nullptr );
        }
        else
        {
            if( strcmp( pFile->GetExtensionWithDot(), ".lua" ) == 0 )
            {
                oldPointer = m_pScriptFile;
                ClearExposedVariableList( changedByInterface ? true : false );
                SetScriptFile( pFile );
            }
        }
    }

    return oldPointer;
}

void* ComponentLuaScript::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = nullptr;

    if( strcmp( pVar->m_Label, "Script" ) == 0 )
    {
        if( changedByInterface )
        {
#if MYFW_USING_WX
            wxString text = g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->GetTextCtrl()->GetValue();
            if( text == "" || text == "none" || text == "no script" )
            {
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "no script" );
                oldpointer = m_pScriptFile;

                // TODO: undo/redo
                this->SetScriptFile( nullptr );
            }
#endif //MYFW_USING_WX
        }
        else
        {
            MyAssert( false );
            // TODO: Implement this block.
        }
    }

    if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    {
#if MYFW_USING_WX
        m_pLuaInlineScript_OnPlay = g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->GetTextCtrl()->GetValue();
#endif //MYFW_USING_WX
    }

    return oldpointer;
}

#if MYFW_USING_IMGUI
void ComponentLuaScript::OnRightClickCallback(ComponentVariable* pVar)
{
    if( m_pScriptFile )
    {
        if( ImGui::MenuItem( "Edit script" ) )
        {
            m_pScriptFile->OSLaunchFile( true );
            ImGui::CloseCurrentPopup();
        }

        if( ImGui::MenuItem( "Remove script" ) )
        {
            bool undoCommandsAdded = ClearExposedVariableList( true );

            // TODO: This undo/redo method is currently losing exposed variable values, here and in regular drag/drop cases.
            //       Fix along with fixing undo/redo for exposed variables in general.
            // Simulate drag/drop of a nullptr for undo/redo.
            m_pComponentSystemManager->GetEngineCore()->GetCommandStack()->Add(
                MyNew EditorCommand_DragAndDropEvent( this, pVar, 0, -1, -1, DragAndDropType_FileObjectPointer, nullptr, m_pScriptFile ),
                undoCommandsAdded ? true : false );

            // Clear script file.
            SetScriptFile( nullptr );
            ImGui::CloseCurrentPopup();
        }
    }

    if( ImGui::MenuItem( "Create new script" ) )
    {
        CreateNewScriptFile();
        ImGui::CloseCurrentPopup();
    }
}
#endif

void ComponentLuaScript::OnExposedVarValueChanged(ExposedVariableDesc* pVar, int component, bool finishedChanging, double oldValue, void* oldPointer)
{
    // Register/unregister GameObject onDelete callbacks.
    if( pVar->type == ExposedVariableType_GameObject )
    {
        GameObject* pOldGameObject = static_cast<GameObject*>( oldPointer );
        GameObject* pGameObject = static_cast<GameObject*>( pVar->pointer );

        // Unregister GameObject deleted callback, if we registered one.
        if( pOldGameObject )
        {
            MyAssert( pOldGameObject->IsA( "GameObject" ) );
            pOldGameObject->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
        }

        // Register new GameObject deleted callback.
        if( pGameObject )
        {
            MyAssert( pGameObject->IsA( "GameObject" ) );
            pGameObject->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
        }
    }

    // Divorce the child value from it's parent, if it no longer matches.
    if( DoesExposedVariableMatchParent( pVar ) == false ) // Returns true if no parent was found.
    {
        pVar->divorced = true;
    }

    // Pass the new value to children.
    UpdateChildrenWithNewValue( pVar, finishedChanging, oldValue, oldPointer );

    // Update the lua state with the new value.
    ProgramVariables( m_pLuaGameState->m_pLuaState, true );
}

#if MYFW_USING_WX
void ComponentLuaScript::OnRightClickExposedVariable(int controlid)
{
    // The only right-click options involve this components gameobject having a parent, so quick early if it doesn't.
    if( m_pGameObject->GetGameObjectThisInheritsFrom() == nullptr )
        return;

    ExposedVariableDesc* pExposedVar = nullptr;

    if( controlid != -1 && m_ControlIDOfFirstExtern != -1 )
    {
        for( unsigned int index=0; index<m_ExposedVars.Count(); index++ )
        {
            if( m_ExposedVars[index]->controlID == controlid )
            {
                pExposedVar = m_ExposedVars[index];
                break;
            }
        }
    }

    if( pExposedVar == nullptr )
    {
        ComponentBase::OnRightClickVariable( controlid );
        return;
    }

    wxMenu menu;
    menu.SetClientData( &m_ComponentLuaScriptEventHandlerForExposedVariables );

    m_ComponentLuaScriptEventHandlerForExposedVariables.pLuaScriptComponent = this;
    m_ComponentLuaScriptEventHandlerForExposedVariables.pExposedVar = pExposedVar;

    // If this game object inherits from another, right-clicking a variable will offer divorce/marry options.
    if( m_pGameObject->GetGameObjectThisInheritsFrom() )
    {
        if( pExposedVar->divorced == false )
        {
            menu.Append( RightClick_DivorceVariable, "Divorce value from parent" );
 	        menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentLuaScriptEventHandlerForExposedVariables::OnPopupClick );
        }
        else
        {
            menu.Append( RightClick_MarryVariable, "Reset value to parent" );
 	        menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentLuaScriptEventHandlerForExposedVariables::OnPopupClick );
        }
    }

    // Blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // There's no reason this is using g_pPanelWatch other than convenience.
}

void ComponentLuaScriptEventHandlerForExposedVariables::OnPopupClick(wxEvent &evt)
{
    ComponentLuaScriptEventHandlerForExposedVariables* pEvtHandler = (ComponentLuaScriptEventHandlerForExposedVariables*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    ComponentLuaScript* pLuaScriptComponent = pEvtHandler->pLuaScriptComponent;
    ExposedVariableDesc* pExposedVar = pEvtHandler->pExposedVar;

    int id = evt.GetId();
    switch( id )
    {
    case ComponentBase::RightClick_DivorceVariable:
        {
            pExposedVar->divorced = true;
            g_pPanelWatch->ChangeStaticTextFontStyle( pExposedVar->controlID, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD );
            g_pPanelWatch->ChangeStaticTextBGColor( pExposedVar->controlID, wxColour( 255, 200, 200, 255 ) );
        }
        break;

    case ComponentBase::RightClick_MarryVariable:
        {
            pExposedVar->divorced = false;
            g_pPanelWatch->ChangeStaticTextFontStyle( pExposedVar->controlID, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
            g_pPanelWatch->ChangeStaticTextBGColor( pExposedVar->controlID, wxNullColour );
            
            // Change the value of this variable to match the parent.
            pLuaScriptComponent->CopyExposedVarValueFromParent( pExposedVar );
        }
        break;
    }
}
#endif // MYFW_USING_WX

bool ComponentLuaScript::DoesExposedVariableMatchParent(ExposedVariableDesc* pVar)
{
    MyAssert( m_pGameObject );
    if( m_pGameObject == nullptr )
        return true; // The object has no parent, we say it matches.
    
    GameObject* pGameObject = m_pGameObject->GetGameObjectThisInheritsFrom();
    if( pGameObject == nullptr )
        return true; // The object has no parent, we say it matches.

    // Found a game object, now find the matching component on it.
    for( unsigned int i=0; i<pGameObject->GetComponentCount(); i++ )
    {
        ComponentBase* pOtherComponent = pGameObject->GetComponentByIndex( i );

        const char* pThisCompClassName = GetClassname();
        const char* pOtherCompClassName = pOtherComponent->GetClassname();

        if( strcmp( pThisCompClassName, pOtherCompClassName ) == 0 )
        {
            ComponentLuaScript* pOtherLuaScript = static_cast<ComponentLuaScript*>( pOtherComponent );

            // Find children of this gameobject and change their vars if needed.
            for( unsigned int varindex=0; varindex<m_ExposedVars.Count(); varindex++ )
            {
                ExposedVariableDesc* pOtherVar = pOtherLuaScript->m_ExposedVars[varindex];
                MyAssert( pOtherVar );

                if( pVar->name == pOtherVar->name )
                {
                    switch( pVar->type )
                    {
                    case ExposedVariableType_Float:
                        return pVar->valuedouble == pOtherVar->valuedouble;

                    case ExposedVariableType_Bool:
                        return pVar->valuebool == pOtherVar->valuebool;

                    case ExposedVariableType_Vector3:
                        return pVar->valuevector3 == pOtherVar->valuevector3;

                    case ExposedVariableType_GameObject:
                        return pVar->pointer == pOtherVar->pointer;

                    case ExposedVariableType_Unused:
                    default:
                        MyAssert( false );
                        break;
                    }
                }
            }
        }
    }

    MyAssert( false ); // Shouldn't get here.
    return true; // The object has no parent, we say it matches.
}

void ComponentLuaScript::UpdateChildrenWithNewValue(ExposedVariableDesc* pVar, bool finishedChanging, double oldValue, void* oldPointer)
{
    MyAssert( pVar );

    // Find children of this gameobject and change their vars if needed.
    for( unsigned int varindex=0; varindex<m_ExposedVars.Count(); varindex++ )
    {
        ExposedVariableDesc* pOtherVar = m_ExposedVars[varindex];
        MyAssert( pOtherVar );

        if( pVar->name == pOtherVar->name )
        {
            for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
            {
                if( m_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse == false )
                    continue;

                SceneInfo* pSceneInfo = &m_pComponentSystemManager->m_pSceneInfoMap[i];

                if( pSceneInfo->m_GameObjects.GetHead() )
                {
                    GameObject* first = pSceneInfo->m_GameObjects.GetHead();
                    UpdateChildrenInGameObjectListWithNewValue( pVar, varindex, first, finishedChanging, oldValue, oldPointer );
                } 
            }
        }
    }
}

void ComponentLuaScript::UpdateChildrenInGameObjectListWithNewValue(ExposedVariableDesc* pVar, unsigned int varindex, GameObject* first, bool finishedChanging, double oldValue, void* oldPointer)
{
    // Find children of this gameobject and change their values as well, if their value matches the old value.
    for( GameObject* pGameObject = first; pGameObject; pGameObject = pGameObject->GetNext() )
    {
        if( pGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject )
        {
            UpdateChildGameObjectWithNewValue( pVar, varindex, pGameObject, finishedChanging, oldValue, oldPointer );
        }

        GameObject* pFirstChild = pGameObject->GetFirstChild();
        if( pFirstChild )
        {
            UpdateChildrenInGameObjectListWithNewValue( pVar, varindex, pFirstChild, finishedChanging, oldValue, oldPointer );
        }
    }
}

void ComponentLuaScript::UpdateChildGameObjectWithNewValue(ExposedVariableDesc* pVar, unsigned int varIndex, GameObject* pChildGameObject, bool finishedChanging, double oldValue, void* oldPointer)
{
    if( pChildGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject )
    {
        // Found a game object, now find the matching component on it.
        for( unsigned int i=0; i<pChildGameObject->GetComponentCount(); i++ )
        {
            ComponentLuaScript* pChildLuaScript = static_cast<ComponentLuaScript*>( pChildGameObject->GetComponentByIndex( i ) );

            const char* pThisCompClassName = GetClassname();
            const char* pChildCompClassName = pChildLuaScript->GetClassname();

            // TODO: This will fail if multiple of the same component are on an object.
            if( strcmp( pThisCompClassName, pChildCompClassName ) == 0 )
            {
                // It's possible the variables are in a different order, so find the correct variable by name.
                ExposedVariableDesc* pChildVar = nullptr;

                // Find the first variable in the other object with the same name.
                for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
                {
                    if( pChildLuaScript->m_ExposedVars.Count() > i &&
                        m_ExposedVars[varIndex]->name == pChildLuaScript->m_ExposedVars[i]->name )
                    {
                        pChildVar = pChildLuaScript->m_ExposedVars[i];
                        break;
                    }
                }

                if( pChildVar )
                {
                    // Found the matching component, now compare the variable.
                    if( pChildVar->type == ExposedVariableType_Float ||
                        pChildVar->type == ExposedVariableType_Bool )
                    {
                        if( fequal( pChildVar->valuedouble, oldValue ) )
                        {
                            pChildVar->valuedouble = pVar->valuedouble;
                            //pChildLuaScript->OnExposedVarValueChanged( controlid, finishedchanging, oldvalue );

                            pChildLuaScript->ProgramVariables( m_pLuaGameState->m_pLuaState, true );
                            pChildLuaScript->UpdateChildrenWithNewValue( pChildVar, finishedChanging, oldValue, oldPointer );
                        }
                    }

                    if( pChildVar->type == ExposedVariableType_Vector3 )
                    {
                        MyAssert( false );
                        //if( fequal( pChildVar->valuevector3[0], oldvalue ) )
                        //{
                        //    pChildVar->valuedouble = pVar->valuedouble;
                        //    pChildLuaScript->OnValueChanged( controlid, finishedchanging, oldvalue );

                        //    pChildLuaScript->ProgramVariables( m_pLuaGameState->m_pLuaState, true );
                        //    pChildLuaScript->UpdateChildrenWithNewValue( controlid, finishedChanging, oldValue, oldPointer );
                        //}
                    }

                    if( pVar->type == ExposedVariableType_GameObject )
                    {
                        if( pChildVar->pointer == oldPointer )
                        {
                            pChildVar->pointer = pVar->pointer;
                            if( pVar->pointer )
                                static_cast<GameObject*>( pVar->pointer )->RegisterOnDeleteCallback( pChildLuaScript, StaticOnGameObjectDeleted );

                            pChildLuaScript->ProgramVariables( m_pLuaGameState->m_pLuaState, true );
                            pChildLuaScript->UpdateChildrenWithNewValue( pChildVar, finishedChanging, oldValue, oldPointer );
                        }
                    }
                }
            }
        }
    }
}

void ComponentLuaScript::CopyExposedVarValueFromParent(ExposedVariableDesc* pVar)
{
    MyAssert( m_pGameObject );
    MyAssert( m_pGameObject->GetGameObjectThisInheritsFrom() );

    GameObject* pParentGO = m_pGameObject->GetGameObjectThisInheritsFrom();
    if( pParentGO == nullptr )
        return;
    
    // Found a game object, now find the matching component on it.
    for( unsigned int i=0; i<pParentGO->GetComponentCount(); i++ )
    {
        ComponentBase* pOtherComponent = pParentGO->GetComponentByIndex( i );

        const char* pThisCompClassName = GetClassname();
        const char* pOtherCompClassName = pOtherComponent->GetClassname();

        if( strcmp( pThisCompClassName, pOtherCompClassName ) == 0 )
        {
            ComponentLuaScript* pOtherLuaScript = static_cast<ComponentLuaScript*>( pOtherComponent );

            // Find children of this gameobject and change their vars if needed.
            for( unsigned int varindex=0; varindex<m_ExposedVars.Count(); varindex++ )
            {
                ExposedVariableDesc* pOtherVar = pOtherLuaScript->m_ExposedVars[varindex];
                MyAssert( pOtherVar );

                if( pVar->name == pOtherVar->name )
                {
                    switch( pVar->type )
                    {
                    case ExposedVariableType_Float:
                        {
                            double oldvalue = pVar->valuedouble;
                            double newvalue = pOtherVar->valuedouble;
                            pVar->valuedouble = pOtherVar->valuedouble;

                            // Notify component and it's children that the value changed.
                            OnExposedVarValueChanged( pVar, 0, true, oldvalue, nullptr );

#if MYFW_USING_WX
                            m_pComponentSystemManager->GetEngineCore()->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue - oldvalue, PanelWatchType_Double, ((char*)&pVar->valuedouble), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ) );
#endif
                        }
                        break;

                    case ExposedVariableType_Bool:
                        {
                            bool oldvalue = pVar->valuebool;
                            bool newvalue = pOtherVar->valuebool;
                            pVar->valuedouble = pOtherVar->valuebool;

                            // Notify component and it's children that the value changed.
                            OnExposedVarValueChanged( pVar, 0, true, oldvalue, nullptr );

#if MYFW_USING_WX
                            m_pComponentSystemManager->GetEngineCore()->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue - oldvalue, PanelWatchType_Bool, ((char*)&pVar->valuebool), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ) );
#endif
                        }
                        break;

                    case ExposedVariableType_Vector3:
                        {
                            Vector3 oldvalue = *(Vector3*)&pVar->valuevector3;
                            Vector3 newvalue = *(Vector3*)&pOtherVar->valuevector3;
                            *(Vector3*)&pVar->valuevector3 = *(Vector3*)&pOtherVar->valuevector3;

                            // Notify component and it's children that the value changed.
                            OnExposedVarValueChanged( pVar, 0, true, oldvalue.x, nullptr );
                            OnExposedVarValueChanged( pVar, 1, true, oldvalue.y, nullptr );
                            OnExposedVarValueChanged( pVar, 2, true, oldvalue.z, nullptr );

#if MYFW_USING_WX
                            m_pComponentSystemManager->GetEngineCore()->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue.x - oldvalue.x, PanelWatchType_Float, ((char*)&pVar->valuevector3[0]), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ) );
                            m_pComponentSystemManager->GetEngineCore()->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue.y - oldvalue.y, PanelWatchType_Float, ((char*)&pVar->valuevector3[1]), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ), true );
                            m_pComponentSystemManager->GetEngineCore()->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue.z - oldvalue.z, PanelWatchType_Float, ((char*)&pVar->valuevector3[2]), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ), true );
#endif
                        }
                        break;

                    case ExposedVariableType_GameObject:
                        g_DragAndDropStruct.Clear();
                        g_DragAndDropStruct.SetControlID( pVar->controlID );
                        g_DragAndDropStruct.Add( DragAndDropType_GameObjectPointer, pOtherVar->pointer );
#if MYFW_USING_WX
                        OnDropExposedVar( pVar->controlID, 0, 0 );
#endif
                        break;

                    case ExposedVariableType_Unused:
                    default:
                        MyAssert( false );
                        break;
                    }
                }
            }
        }
    }
}

bool ComponentLuaScript::ClearExposedVariableList(bool addUndoCommands)
{
    // Delete all variables.
    if( addUndoCommands )
    {
        // Add clear to undo stack.
        if( m_ExposedVars.size() > 0 )
        {
            m_pComponentSystemManager->GetEngineCore()->GetCommandStack()->Do(
                MyNew EditorCommand_LuaClearExposedVariables( this, m_ExposedVars ) );

            return true;
        }
    }
    else
    {
        while( m_ExposedVars.Count() )
        {
            // Remove the first variable from the list.
            ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex( 0 );

            // Unregister gameobject deleted callback, if we registered one.
            if( pVariable->type == ExposedVariableType_GameObject && pVariable->pointer )
                static_cast<GameObject*>( pVariable->pointer )->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

            delete pVariable;
        }
    }

    return false;
}
#endif //MYFW_EDITOR

cJSON* ComponentLuaScript::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* jComponent = ComponentUpdateable::ExportAsJSONObject( savesceneid, saveid );

    if( m_pScriptFile )
        cJSON_AddStringToObject( jComponent, "Script", m_pScriptFile->GetFullPath() );

#if MYFW_EDITOR
    // Trim white-space before saving.
    size_t left = m_pLuaInlineScript_OnPlay.find_first_not_of( " \t" );
    if( left != std::string::npos )
    {
        size_t right = m_pLuaInlineScript_OnPlay.find_last_not_of( " \t" );
        m_pLuaInlineScript_OnPlay = m_pLuaInlineScript_OnPlay.substr( left, right-left+1 );
        if( m_pLuaInlineScript_OnPlay.length() > 0 )
        {
            cJSON_AddStringToObject( jComponent, "LuaString_OnPlay", m_pLuaInlineScript_OnPlay.c_str() );
        }
    }
#endif

    // Save the array of exposed variables.
    if( m_ExposedVars.Count() > 0 )
    {
        cJSON* exposedvararray = cJSON_CreateArray();
        cJSON_AddItemToObject( jComponent, "ExposedVars", exposedvararray );
        for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
        {
            ExposedVariableDesc* pVar = m_ExposedVars[i];
        
            cJSON* jExposedVar = cJSON_CreateObject();
            cJSON_AddItemToArray( exposedvararray, jExposedVar );

            cJSON_AddStringToObject( jExposedVar, "Name", pVar->name.c_str() );
            cJSON_AddNumberToObject( jExposedVar, "Type", pVar->type );

            if( pVar->type == ExposedVariableType_Float )
            {
                cJSON_AddNumberToObject( jExposedVar, "Value", pVar->valuedouble );
            }
            if( pVar->type == ExposedVariableType_Bool )
            {
                cJSON_AddNumberToObject( jExposedVar, "Value", pVar->valuebool );
            }
            if( pVar->type == ExposedVariableType_Vector3 )
            {
                cJSONExt_AddFloatArrayToObject( jExposedVar, "Value", pVar->valuevector3, 3 );
            }
            else if( pVar->type == ExposedVariableType_GameObject && pVar->pointer )
            {
                cJSON* gameobjectref = static_cast<GameObject*>( pVar->pointer )->ExportReferenceAsJSONObject( m_SceneIDLoadedFrom );
                cJSON_AddItemToObject( jExposedVar, "Value", gameobjectref );

                // TODO: Find a way to uniquely identify a game object...
                //cJSON_AddStringToObject( jExposedVar, "Value", static_cast<GameObject*>( pVar->pointer )->GetName() );
            }

            if( pVar->divorced )
                cJSON_AddNumberToObject( jExposedVar, "Divorced", pVar->divorced );
        }
    }

    return jComponent;
}

void ComponentLuaScript::ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid)
{
    //ComponentUpdateable::ImportFromJSONObject( jsonobj, sceneid );

    cJSON* scriptstringobj = cJSON_GetObjectItem( jsonobj, "Script" );
    if( scriptstringobj )
    {
        EngineFileManager* pEngineFileManager = static_cast<EngineFileManager*>( m_pComponentSystemManager->GetEngineCore()->GetManagers()->GetFileManager() );
        MyFileObject* pFile = pEngineFileManager->RequestFile( scriptstringobj->valuestring, GetSceneID() );
        MyAssert( pFile );
        if( pFile )
        {
            SetScriptFile( pFile );
            pFile->Release(); // Free ref added by RequestFile.
        }
    }

    cJSON* string_onplay = cJSON_GetObjectItem( jsonobj, "LuaString_OnPlay" );
    if( string_onplay )
    {
#if MYFW_EDITOR
        m_pLuaInlineScript_OnPlay = string_onplay->valuestring;
#else
        // In stand-alone build, allocate memory and make a copy of the string.
        int len = (int)strlen( string_onplay->valuestring );

        m_pLuaInlineScript_OnPlay = MyNew char[len+1];
        strcpy_s( m_pLuaInlineScript_OnPlay, len+1, string_onplay->valuestring );
#endif
    }

    // Load the array of exposed variables.
    cJSON* exposedvararray = cJSON_GetObjectItem( jsonobj, "ExposedVars" );
    if( exposedvararray )
    {
        int numvars = cJSON_GetArraySize( exposedvararray );
        for( int i=0; i<numvars; i++ )
        {
            cJSON* jsonvar = cJSON_GetArrayItem( exposedvararray, i );

            cJSON* obj = cJSON_GetObjectItem( jsonvar, "Name" );
            MyAssert( obj );
            if( obj == nullptr )
                continue;

            // By name, check if the variable is already in our list.
            ExposedVariableDesc* pVar = nullptr;
            for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
            {
                MyAssert( m_ExposedVars[i] );
                if( m_ExposedVars[i]->name == obj->valuestring )
                {
                    pVar = m_ExposedVars[i];
                    break;
                }
            }

            // If not, create and add it.
            if( pVar == nullptr )
            {
                pVar = MyNew ExposedVariableDesc();
                m_ExposedVars.Add( pVar );
            }

            pVar->Reset();

            pVar->name = obj->valuestring;
            cJSONExt_GetInt( jsonvar, "Type", (int*)&pVar->type );

            if( pVar->type == ExposedVariableType_Float )
            {
                cJSONExt_GetDouble( jsonvar, "Value", &pVar->valuedouble );
            }
            if( pVar->type == ExposedVariableType_Bool )
            {
                cJSONExt_GetBool( jsonvar, "Value", &pVar->valuebool );
            }
            if( pVar->type == ExposedVariableType_Vector3 )
            {
                cJSONExt_GetFloatArray( jsonvar, "Value", pVar->valuevector3, 3 );
            }
            else if( pVar->type == ExposedVariableType_GameObject )
            {
                cJSON* obj = cJSON_GetObjectItem( jsonvar, "Value" );
                if( obj )
                {
                    pVar->pointer = m_pComponentSystemManager->FindGameObjectByJSONRef( obj, m_pGameObject->GetSceneID(), false );

                    // TODO: Handle cases where the Scene containing the GameObject referenced isn't loaded.
                    //MyAssert( pVar->pointer != nullptr );
                    if( pVar->pointer )
                    {
                        static_cast<GameObject*>( pVar->pointer )->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
                    }
                    else
                    {
                        LOGError( LOGTag, "LuaScript component on '%s' lost reference to GameObject.\n", m_pGameObject->GetName() );
                    }
                }
            }

            cJSONExt_GetBool( jsonvar, "Divorced", &pVar->divorced );
        }
    }
}

ComponentLuaScript& ComponentLuaScript::operator=(const ComponentLuaScript& other)
{
    MyAssert( &other != this );

    ComponentUpdateable::operator=( other );

    this->m_pScriptFile = other.m_pScriptFile;
    if( this->m_pScriptFile )
    {
        this->m_pScriptFile->AddRef();

        m_pCopyExternsFromThisComponentAfterLoadingScript = &other;
    }

    return *this;
}

void ComponentLuaScript::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLuaScript, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLuaScript, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLuaScript, Draw );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLuaScript, OnTouch );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLuaScript, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLuaScript, OnKeys );
    }
}

void ComponentLuaScript::UnregisterCallbacks()
{
    MyAssert( m_EnabledState != EnabledState_Enabled );

    if( m_CallbacksRegistered == true )
    {
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );

        m_CallbacksRegistered = false;
    }
}

void ComponentLuaScript::SetScriptFile(MyFileObject* script)
{
    if( script )
        script->AddRef();

    SAFE_RELEASE( m_pScriptFile );
    m_pScriptFile = script;

    m_ScriptLoaded = false;
    m_ErrorInScript = false;

    if( m_Playing )
    {
        m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading = true;
    }
}

void ComponentLuaScript::LoadScript()
{
    if( m_ScriptLoaded == true )
        return;

    // Unregister all event callbacks, they will be Registered again based on what the script needs.
    EventManager* pEventManager = m_pComponentSystemManager->GetEngineCore()->GetManagers()->GetEventManager();
    pEventManager->UnregisterForEvents( "Keyboard", this, &ComponentLuaScript::StaticOnEvent );

    // Script is ready, so run it.
    if( m_pScriptFile != nullptr )
    {
        if( m_pScriptFile->GetFileLoadStatus() == FileLoadStatus_Success )
        {
            //LOGInfo( LOGTag, "luaL_loadstring: %s\n", m_pScriptFile->GetFilenameWithoutExtension() );

            // Mark script as loaded. "OnLoad" can be called if no errors occur otherwise m_ErrorInScript will be set as well.
            m_ScriptLoaded = true;

            const char* filename = m_pScriptFile->GetFullPath();
            char label[MAX_PATH+1];
            sprintf_s( label, MAX_PATH+1, "@%s", filename );

#if MYFW_ENABLE_LUA_DEBUGGER
            // Prevent the debugger from stopping while parsing script files.
            m_pLuaGameState->SetIsDebuggerAllowedToStop( false );
#endif

            // Load the string from the file.
            int fileLength = m_pScriptFile->GetFileLength() - 1;
            int loadReturnCode = luaL_loadbuffer( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetBuffer(), fileLength, label );

            if( loadReturnCode == LUA_OK )
            {
                // Run the code to do initial parsing.
                int exeretcode = lua_pcall( m_pLuaGameState->m_pLuaState, 0, LUA_MULTRET, 0 );
                if( exeretcode == LUA_OK )
                {
                    luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );

                    if( LuaObject.isTable() )
                    {
                        // Create a table to store local variables unique to this component.
                        sprintf_s( m_LuaGameObjectName, 100, "_GameObject_%d_%d", m_pGameObject->GetSceneID(), m_pGameObject->GetID() );
                        luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );

                        if( gameObjectData.isTable() == false )
                        {
                            gameObjectData = luabridge::newTable( m_pLuaGameState->m_pLuaState );
                            luabridge::setGlobal( m_pLuaGameState->m_pLuaState, gameObjectData, m_LuaGameObjectName );
                            gameObjectData["gameobject"] = m_pGameObject;
                        }

                        ParseExterns( LuaObject );

                        // Call the OnLoad function in the Lua script.
                        CallFunctionEvenIfGameplayInactive( "OnLoad" );

                        // If OnKeys() exists as a lua function, then register for keyboard events.
                        if( DoesFunctionExist( "OnKeys" ) )
                        {
                            EventManager* pEventManager = m_pComponentSystemManager->GetEngineCore()->GetManagers()->GetEventManager();
                            pEventManager->RegisterForEvents( "Keyboard", this, &ComponentLuaScript::StaticOnEvent );
                        }
                    }
                    else
                    {
                        LOGInfo( LOGTag, "Object with matching name not found in Lua Script: %s\n", m_pScriptFile->GetFilenameWithoutExtension() );
                    }
                }
                else
                {
                    if( exeretcode == LUA_ERRRUN )
                    {
                        const char* errorstr = lua_tostring( m_pLuaGameState->m_pLuaState, -1 );
                        HandleLuaError( "LUA_ERRRUN", errorstr );
                    }
                    else
                    {
                        MyAssert( false ); // Assert until I hit this and deal with it better.
                        const char* errorstr = lua_tostring( m_pLuaGameState->m_pLuaState, -1 );
                        HandleLuaError( "!LUA_ERRRUN", errorstr );
                    }
                }
            }
            else
            {
                if( loadReturnCode == LUA_ERRSYNTAX )
                {
                    const char* errorstr = lua_tostring( m_pLuaGameState->m_pLuaState, -1 );
                    HandleLuaError( "LUA_ERRSYNTAX", errorstr );
                }
                else
                {
                    MyAssert( false );
                    const char* errorstr = lua_tostring( m_pLuaGameState->m_pLuaState, -1 );
                    HandleLuaError( "!LUA_ERRSYNTAX", errorstr );
                }
            }

#if MYFW_ENABLE_LUA_DEBUGGER
            // Reenable the debugger.
            m_pLuaGameState->SetIsDebuggerAllowedToStop( true );
#endif
        }
    }
}

void ComponentLuaScript::LoadInLineScripts()
{
    // Add the OnPlay string to the lua state and parse it.
#if MYFW_EDITOR
    if( m_pLuaInlineScript_OnPlay.length() > 0 )
    {
        const char* pScript = m_pLuaInlineScript_OnPlay.c_str();
#else
    if( m_pLuaInlineScript_OnPlay != nullptr && m_pLuaInlineScript_OnPlay[0] != '\0' )
    {
        const char* pScript = m_pLuaInlineScript_OnPlay;
#endif

        // Generate a unique name for an object/table the function can exist in.
        char inlinescriptname[100];
        sprintf_s( inlinescriptname, 100, "_InlineScript_GameObject_%d_%d", m_pGameObject->GetSceneID(), m_pGameObject->GetID() );

        // Create a small lua script to create the object/table which will contain an OnPlay function.
        char inlinescript[200];
        sprintf_s( inlinescript, 200, "%s = { OnPlay = function() %s end }", inlinescriptname, pScript );

        // Load the new script into the lua state.
        int loadReturnCode = luaL_loadstring( m_pLuaGameState->m_pLuaState, inlinescript );

        if( loadReturnCode == LUA_OK )
        {
            // Run the code to do initial parsing.
            int exeretcode = lua_pcall( m_pLuaGameState->m_pLuaState, 0, LUA_MULTRET, 0 );
            if( exeretcode == LUA_OK )
            {
                // Check if the _InlineScript_GameObject_X_X object exists, otherwise something went wrong.
                luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, inlinescriptname );

                if( LuaObject.isTable() == false )
                {
                    LOGError( LOGTag, "Inline lua script failed to compile: %s", inlinescriptname );
                }

                if( LuaObject["OnPlay"].isFunction() == false )
                {
                    LOGError( LOGTag, "Inline lua script failed to compile: %s", inlinescriptname );
                }
            }
        }
    }
}

void ComponentLuaScript::ParseExterns(luabridge::LuaRef LuaObject)
{
    // Mark all variables unused.
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        m_ExposedVars[i]->inuse = false;
    }

    luabridge::LuaRef Externs = LuaObject["Externs"];

    if( Externs.isTable() == true )
    {
        int len = Externs.length();
        for( int i=0; i<len; i++ )
        {
            luabridge::LuaRef variabledesc = Externs[i+1];

            //if( variabledesc.isTable() == false )
            //    return;

            luabridge::LuaRef variablename = variabledesc[1];
            luabridge::LuaRef variabletype = variabledesc[2];
            luabridge::LuaRef variableinitialvalue = variabledesc[3];

            std::string varname = variablename.tostring();
            std::string vartype = variabletype.tostring();

            ExposedVariableDesc* pVar = nullptr;

            // Find any old variable with the same name.
            for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
            {
                if( m_ExposedVars[i]->name == varname )
                {
                    pVar = m_ExposedVars[i];
                    pVar->inuse = true; // Was in use.
                    break;
                }
            }

            // If not found, create a new one and add it to the list.
            if( pVar == nullptr )
            {
                pVar = MyNew ExposedVariableDesc();
                m_ExposedVars.Add( pVar );
                pVar->inuse = false; // Is new, will be marked inuse after being initialized.
            }

            pVar->name = varname;

            if( vartype == "Float" )
            {
                // If it's a new variable or it changed type, set it to it's initial value.
                if( pVar->inuse == false || pVar->type != ExposedVariableType_Float )
                    pVar->valuedouble = variableinitialvalue.cast<double>();

                pVar->type = ExposedVariableType_Float;
            }
            else if( vartype == "Vector3" )
            {
                // If it's a new variable or it changed type, set it to it's initial value.
                if( pVar->inuse == false || pVar->type != ExposedVariableType_Vector3 )
                {
                    if( variableinitialvalue.isTable() == false ||
                        variableinitialvalue[1].isNil() || 
                        variableinitialvalue[2].isNil() || 
                        variableinitialvalue[3].isNil() )
                    {
                        LOGError( "LuaScript", "Initial value for vector3 isn't an array of 3 values\n" );
                    }
                    else
                    {
                        pVar->valuevector3[0] = variableinitialvalue[1].cast<float>();
                        pVar->valuevector3[1] = variableinitialvalue[2].cast<float>();
                        pVar->valuevector3[2] = variableinitialvalue[3].cast<float>();
                    }
                }

                pVar->type = ExposedVariableType_Vector3;
            }
            else if( vartype == "Bool" )
            {
                // If it's a new variable or it changed type, set it to it's initial value.
                if( pVar->inuse == false || pVar->type != ExposedVariableType_Bool )
                    pVar->valuebool = variableinitialvalue.cast<bool>();

                pVar->type = ExposedVariableType_Bool;
            }
            else if( vartype == "GameObject" )
            {
                // If it's a new variable or it changed type, set it to it's initial value.
                if( pVar->inuse == false || pVar->type != ExposedVariableType_GameObject )
                {
                    pVar->pointer = m_pComponentSystemManager->FindGameObjectByName( variableinitialvalue.tostring().c_str() );
                    if( pVar->pointer )
                        static_cast<GameObject*>( pVar->pointer )->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
                }
                pVar->type = ExposedVariableType_GameObject;
            }
            else
            {
                MyAssert( false );
            }

            pVar->inuse = true;
        }
    }

    // Delete unused variables.
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        if( m_ExposedVars[i]->inuse == false )
        {
            ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex_MaintainOrder( i );

            // Unregister gameobject deleted callback, if we registered one.
            if( pVariable->type == ExposedVariableType_GameObject && pVariable->pointer )
                static_cast<GameObject*>( pVariable->pointer )->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

            delete pVariable;
            i--;
        }
    }
}

void ComponentLuaScript::ProgramVariables(luabridge::LuaRef LuaObject, bool updateExposedVariables)
{
    if( m_ScriptLoaded == false )
        return;

    // Get the Lua data table for this GameObject.
    luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );

    // Only program the exposed vars if they change.
    if( updateExposedVariables )
    {
        for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
        {
            ExposedVariableDesc* pVar = m_ExposedVars[i];

            if( pVar->type == ExposedVariableType_Float )
                gameObjectData[pVar->name] = pVar->valuedouble;

            if( pVar->type == ExposedVariableType_Bool )
                gameObjectData[pVar->name] = pVar->valuebool;

            if( pVar->type == ExposedVariableType_Vector3 )
                gameObjectData[pVar->name] = Vector3( pVar->valuevector3[0], pVar->valuevector3[1], pVar->valuevector3[2] );

            if( pVar->type == ExposedVariableType_GameObject )
                gameObjectData[pVar->name] = static_cast<GameObject*>( pVar->pointer );
        }
    }
}

void ComponentLuaScript::SetExternFloat(const char* name, float newValue)
{
#if MYFW_EDITOR
    // Make sure the variable exists and the type matches.
    bool found = false;

    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];
        if( pVar->name == name )
        {
            found = true;
            if( pVar->type != ExposedVariableType_Float )
            {
                LOGError( LOGTag, "SetExternFloat called on wrong type: %d", pVar->type );
                return;
            }

            pVar->valuedouble = newValue;
        }
    }

    if( found == false )
    {
        if( m_ScriptLoaded == true )
        {
            LOGError( LOGTag, "Extern not found: %s", name );
            return;
        }

        ExposedVariableDesc* pVar = MyNew ExposedVariableDesc();
        m_ExposedVars.Add( pVar );

        pVar->Reset();

        pVar->name = name;
        pVar->type = ExposedVariableType_Float;
        pVar->valuedouble = newValue;
        return;
    }
#endif

    // Get the GameObjectData table from the lua state for this object.
    luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );
    MyAssert( gameObjectData.isTable() );

    // Set the new value.
    gameObjectData[name] = newValue;
}

void ComponentLuaScript::HandleLuaError(const char* functionname, const char* errormessage)
{
    m_ErrorInScript = true;
    LuaBridgeExt_LogExceptionFormattedForVisualStudioOutputWindow( functionname, m_pScriptFile->GetFullPath(), errormessage );

    // Pop the error message off the lua stack.
    lua_pop( m_pLuaGameState->m_pLuaState, 1 );
}

void ComponentLuaScript::OnLoad()
{
    ComponentUpdateable::OnLoad();

    m_ScriptLoaded = false;
    m_ErrorInScript = false;
}

void ComponentLuaScript::OnPlay()
{
    ComponentUpdateable::OnPlay();

    if( m_ErrorInScript )
        return;

    if( m_pScriptFile && m_pScriptFile->GetFileLoadStatus() != FileLoadStatus_Success )
    {
        LOGInfo( LOGTag, "Script warning: OnPlay, script not loaded: %s\n", m_pScriptFile->GetFilenameWithoutExtension() );
    }

    m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading = true;

    LoadInLineScripts();

    // Check for an inline OnPlay function and call it.
    {
        // Generate the name for the object/table that contains our inline scripts.
        char inlineScriptName[100];
        sprintf_s( inlineScriptName, 100, "_InlineScript_GameObject_%d_%d", m_pGameObject->GetSceneID(), m_pGameObject->GetID() );

        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, inlineScriptName );
        
        if( LuaObject.isTable() )
        {
            if( LuaObject["OnPlay"].isFunction() )
            {
                try
                {
                    luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );
                    LuaObject["OnPlay"]( gameObjectData );
                }
                catch(luabridge::LuaException const& e)
                {
                    HandleLuaError( "OnPlay - inline", e.what() );
                }
            }
        }
    }
}

void ComponentLuaScript::OnStop()
{
    ComponentUpdateable::OnStop();

    if( m_Playing && m_ErrorInScript == false )
    {
        CallFunction( "OnStop" );
    }

    m_ScriptLoaded = false;
    m_ErrorInScript = false;
    m_Playing = false;
}

void ComponentLuaScript::OnGameObjectEnabled()
{
    ComponentBase::OnGameObjectEnabled();
    if( m_pComponentSystemManager->GetEngineCore()->IsInEditorMode() == false )
        OnPlay();
}

void ComponentLuaScript::OnGameObjectDisabled()
{
    ComponentBase::OnGameObjectDisabled();
}

void ComponentLuaScript::TickCallback(float deltaTime)
{
    //ComponentBase::TickCallback( deltaTime );
    //ComponentUpdateable::Tick( deltaTime );

    if( m_ErrorInScript )
        return;

    if( m_ScriptLoaded == false && m_pLuaGameState )
    {
        LoadScript();
    }

    if( m_pLuaGameState == nullptr || m_ScriptLoaded == false )
        return;

    // Copy externed variable values after loading the script.
    {
        if( m_pCopyExternsFromThisComponentAfterLoadingScript )
        {
            const ComponentLuaScript& other = *m_pCopyExternsFromThisComponentAfterLoadingScript;
            m_pCopyExternsFromThisComponentAfterLoadingScript = nullptr;

            for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
            {
                ExposedVariableDesc* pVar = m_ExposedVars[i];
                ExposedVariableDesc* pOtherVar = nullptr;// = other.m_ExposedVars[i];            

                // Find the first variable in the other object with the same name.
                for( unsigned int oi=0; oi<m_ExposedVars.Count(); oi++ )
                {
                    if( pVar->name == other.m_ExposedVars[oi]->name )
                    {
                        pOtherVar = other.m_ExposedVars[oi];
                        break;
                    }
                }

                if( pOtherVar != nullptr )
                {
                    if( pVar->type == ExposedVariableType_Float )
                    {
                        pVar->valuedouble = pOtherVar->valuedouble;
                    }

                    if( pVar->type == ExposedVariableType_Bool )
                    {
                        pVar->valuebool = pOtherVar->valuebool;
                    }

                    if( pVar->type == ExposedVariableType_Vector3 )
                    {
                        for( int i=0; i<3; i++ )
                            pVar->valuevector3[0] = pOtherVar->valuevector3[0];
                    }

                    if( pVar->type == ExposedVariableType_GameObject )
                    {
                        pVar->pointer = pOtherVar->pointer;

                        if( pVar->pointer )
                            static_cast<GameObject*>( pVar->pointer )->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
                    }
                }
            }

            ProgramVariables( m_pLuaGameState->m_pLuaState, true );
        }
    }

    if( m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading )
    {
        m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading = false;

        // Find the OnPlay function and call it, look for a table that matched our filename.
        if( m_pScriptFile )
        {
            luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );
        
            if( LuaObject.isNil() == false )
            {
                if( LuaObject["OnPlay"].isFunction() )
                {
                    // Program the exposed variable values in the table, don't just set the table to be active.
                    ProgramVariables( LuaObject, true );
                    try
                    {
                        luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );
                        LuaObject["OnPlay"]( gameObjectData );
                    }
                    catch(luabridge::LuaException const& e)
                    {
                        HandleLuaError( "OnPlay", e.what() );
                    }
                }

                m_Playing = true;
            }
        }
    }

    // Find the Tick function and call it.
    if( m_Playing )
    {
        CallFunction( "Tick", deltaTime );
    }
}

bool ComponentLuaScript::OnTouchCallback(int action, int id, float x, float y, float pressure, float size)
{
    //ComponentBase::OnTouchCallback( action, id, x, y, pressure, size );

    if( m_ErrorInScript )
        return false;

    // Find the OnTouch function and call it.
    if( m_Playing )
    {
        if( CallFunction( "OnTouch", action, id, x, y, pressure, size ) )
            return true;
    }

    return false;
}

bool ComponentLuaScript::OnButtonsCallback(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    //ComponentBase::OnButtonsCallback( action, id );

    if( m_ErrorInScript )
        return false;

    // Find the OnButtons function and call it.
    if( m_Playing )
    {
        int a = action;
        int i = id;
        if( CallFunction( "OnButtons", a, i ) )
            return true;
    }

    return false;
}

void ComponentLuaScript::OnGameObjectDeleted(GameObject* pGameObject)
{
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];

        // If any of our variables is a gameobject, clear it if that gameobject was deleted.
        if( pVar->type == ExposedVariableType_GameObject )
        {
            if( pVar->pointer == pGameObject )
            {
                if( m_pComponentSystemManager->GetEngineCore()->GetCommandStack() )
                {
                    m_pComponentSystemManager->GetEngineCore()->GetCommandStack()->Do( MyNew EditorCommand_LuaExposedVariablePointerChanged(
                        nullptr, pVar, ComponentLuaScript::StaticOnExposedVarValueChanged, this ), true );
                }
                pVar->pointer = nullptr;
            }
        }
    }
}

bool ComponentLuaScript::OnEvent(MyEvent* pEvent) // StaticOnEvent
{
    // Only registered for Keyboard events ATM.
    MyAssert( pEvent->IsType( "Keyboard" ) );

    int action = pEvent->GetInt( "Action" );
    int keyCode = pEvent->GetInt( "KeyCode" );

    if( CallFunction( "OnKeys", action, keyCode ) )
        return true;

    return false;
}

#endif //MYFW_USING_LUA
