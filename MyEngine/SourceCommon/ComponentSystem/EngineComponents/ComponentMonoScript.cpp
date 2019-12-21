//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#if MYFW_USING_MONO

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/attrdefs.h"

#include "ComponentMonoScript.h"
#include "ComponentSystem/Core/EngineFileManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineComponentTypeManager.h"
#include "Core/EngineCore.h"
#include "Mono/Core/MonoGameObject.h"
#include "Mono/Framework/MonoFrameworkClasses.h"
#include "Mono/MonoGameState.h"
#include "../../../SourceEditor/PlatformSpecific/FileOpenDialog.h"

#if MYFW_EDITOR
#include "GUI/ImGuiExtensions.h"
#include "../SourceEditor/Commands/EngineEditorCommands.h"
#endif

// Component Variable List.
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentMonoScript ); //_VARIABLE_LIST

ComponentMonoScript::ComponentMonoScript(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager)
: ComponentScriptBase( pEngineCore, pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Updateable;
    m_Type = ComponentType_LuaScript;

    m_MonoGameObjectName[0] = '\0';
    m_pCopyExternsFromThisComponentAfterLoadingScript = nullptr;

    m_pScriptFile = nullptr;
    m_MonoClassName[0] = '\0';
    m_pMonoObjectInstance = nullptr;

    m_pMonoFuncPtr_OnLoad = nullptr;
    m_pMonoFuncPtr_OnPlay = nullptr;
    m_pMonoFuncPtr_OnStop = nullptr;
    m_pMonoFuncPtr_OnTouch = nullptr;
    m_pMonoFuncPtr_OnButtons = nullptr;
    m_pMonoFuncPtr_Update = nullptr;

#if MYFW_EDITOR
    m_ValueWhenControlSelected = 0;
    m_ImGuiControlIDForCurrentlySelectedVariable = -1;
    m_LinkNextUndoCommandToPrevious = false;

    m_pComponentSystemManager->Editor_RegisterFileUpdatedCallback( &StaticOnFileUpdated, this );
#endif
}

ComponentMonoScript::~ComponentMonoScript()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    while( m_ExposedVars.Count() )
    {
        ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex( 0 );

        // Unregister gameobject deleted callback, if we registered one.
        if( pVariable->value.type == ExposedVariableType::GameObject && pVariable->value.valuePointer )
            static_cast<GameObject*>( pVariable->value.valuePointer )->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

        delete pVariable;
    }

    SAFE_RELEASE( m_pScriptFile );
}

void ComponentMonoScript::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentMonoScript* pThis) //_VARIABLE_LIST
{
    // Just want to make sure these are the same on all compilers.  They should be since this is a simple class.
#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
    MyAssert( offsetof( ComponentMonoScript, m_pScriptFile ) == MyOffsetOf( pThis, &pThis->m_pScriptFile ) );
#if __GNUC__
#pragma GCC diagnostic pop
#endif

    // Script is not automatically saved/loaded.
    ComponentVariable* pVar = AddVar( pList, "Script", ComponentVariableType_FilePtr, MyOffsetOf( pThis, &pThis->m_pScriptFile ), false, true, nullptr, (CVarFunc_ValueChanged)&ComponentMonoScript::OnValueChanged, (CVarFunc_DropTarget)&ComponentMonoScript::OnDrop, nullptr );
#if MYFW_USING_IMGUI
    pVar->AddCallback_OnRightClick( (CVarFunc)&ComponentMonoScript::OnRightClickCallback, nullptr );
#endif
}

void ComponentMonoScript::Reset()
{
    ComponentUpdateable::Reset();

#if MYFW_USING_MONO
    m_pMonoGameState = m_pEngineCore->GetMonoGameState();
#endif //MYFW_USING_MONO

    m_ScriptLoaded = false;
    m_Playing = false;
    m_ErrorInScript = false;
    m_CallMonoOnPlayNextTickOrAfterScriptIsFinishedLoading = false;

    m_pCopyExternsFromThisComponentAfterLoadingScript = nullptr;

    while( m_ExposedVars.Count() )
    {
        ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex( 0 );

        // Unregister gameobject deleted callback, if we registered one.
        if( pVariable->value.type == ExposedVariableType::GameObject && pVariable->value.valuePointer )
            static_cast<GameObject*>( pVariable->value.valuePointer )->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

        delete pVariable;
    }
}

#if MYFW_USING_LUA
void ComponentMonoScript::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentMonoScript>( "ComponentMonoScript" )
            .addFunction( "SetScriptFile", &ComponentMonoScript::SetScriptFile ) // void ComponentMonoScript::SetScriptFile(MyFileObject* script)
            .addFunction( "SetExternFloat", &ComponentMonoScript::SetExternFloat ) // void ComponentMonoScript::SetExternFloat(const char* name, float newValue)
        .endClass();
}
#endif //MYFW_USING_LUA

void* ComponentMonoScript::GetPointerValue(ComponentVariable* pVar) //_VARIABLE_LIST
{
    //if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    //{
    //}

    return nullptr;
}

void ComponentMonoScript::SetPointerValue(ComponentVariable* pVar, const void* newvalue) //_VARIABLE_LIST
{
    //if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    //{
    //}
}

const char* ComponentMonoScript::GetPointerDesc(ComponentVariable* pVar) //_VARIABLE_LIST
{
    //if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    //{
    //}

    return "fix me";
}

void ComponentMonoScript::SetPointerDesc(ComponentVariable* pVar, const char* newdesc) //_VARIABLE_LIST
{
    //if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    //{
    //}
}

#if MYFW_EDITOR
void ComponentMonoScript::CreateNewScriptFile()
{
    //if( m_pScriptFile == nullptr )
    {
        // Generally offer to create scripts in Scripts folder.
        const char* initialPath = "DataSource\\C#\\";

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

        const char* filename = FileSaveDialog( initialPath, "C# files\0*.cs\0All\0*.*\0" );
        if( filename[0] != '\0' )
        {
            int len = (int)strlen( filename );

            // Append '.cs' to end of filename if it wasn't already there.
            char fullpath[MAX_PATH];
            if( len > 4 && strcmp( &filename[len-4], ".cs" ) == 0 )
            {
                strcpy_s( fullpath, MAX_PATH, filename );
            }
            else
            {
                sprintf_s( fullpath, MAX_PATH, "%s.cs", filename );
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
void ComponentMonoScript::AddAllVariablesToWatchPanel()
{
#if !MYFW_USING_MONO
    ImGui::Text( "Mono is disabled in this build." );
    return;
#endif //!MYFW_USING_MONO

    ComponentBase::AddAllVariablesToWatchPanel();

    // Show list of available Mono classes.
    {
        MonoImage* pMonoImage = m_pMonoGameState->GetImage();
        //ImGui::Text( "Class: %s", m_MonoClassName );

        if( pMonoImage == nullptr )
        {
            ImGui::Indent( 20 );
            ImGui::Text( "Mono dll not loaded" );
            ImGui::Unindent( 20 );
        }
        else
        {
            MonoClass* pClass = mono_class_from_name( pMonoImage, "", m_MonoClassName );
            if( pClass == nullptr )
            {
                ImGui::Indent( 20 );
                ImGui::Text( "Class not found in dll", m_MonoClassName );
                ImGui::Unindent( 20 );
            }

            //ImGui::Text( "List of classes" );

            std::vector<std::string> validClasses;
            int32 currentClassIndex = -1;

            if( pMonoImage )
            {
                const MonoTableInfo* pTableInfo = mono_image_get_table_info( pMonoImage, MONO_TABLE_TYPEDEF );

                int numRows = mono_table_info_get_rows( pTableInfo );
                for( int i=0; i<numRows; i++ )
                {
                    uint32_t cols[MONO_TYPEDEF_SIZE];
                    mono_metadata_decode_row( pTableInfo, i, cols, MONO_TYPEDEF_SIZE );

                    //const char* flags = mono_metadata_string_heap( pMonoImage, cols[MONO_TYPEDEF_FLAGS] );
                    const char* nameSpace = mono_metadata_string_heap( pMonoImage, cols[MONO_TYPEDEF_NAMESPACE] );
                    const char* className = mono_metadata_string_heap( pMonoImage, cols[MONO_TYPEDEF_NAME] );
                    //const char* extends = mono_metadata_string_heap( pMonoImage, cols[MONO_TYPEDEF_EXTENDS] );
                    //const char* fieldList = mono_metadata_string_heap( pMonoImage, cols[MONO_TYPEDEF_FIELD_LIST] );
                    //const char* methodList = mono_metadata_string_heap( pMonoImage, cols[MONO_TYPEDEF_METHOD_LIST] );
            
                    MonoClass* pClass = mono_class_from_name( pMonoImage, nameSpace, className );
                    if( pClass )
                    {
                        //MonoType* pType = mono_class_enum_basetype( pClass );
                        //MonoType* pType = mono_class_get_byref_type( pClass );
                        //MonoClass* pElementClass = mono_class_get_element_class( pClass );
                        uint32 flags = mono_class_get_flags( pClass );
                        //mono_class_get_interfaces
                        MonoClass* pParentClass = mono_class_get_parent( pClass );
                        const char* parentClassName = "";
                        if( pParentClass )
                            parentClassName = mono_class_get_name( pParentClass );

                        // Check if this is a valid class type.
                        if( flags & MONO_TYPE_ATTR_PUBLIC &&
                            strcmp( parentClassName, "MyScriptInterface" ) == 0 )
                        {
                            //ImGui::Text( "%s.%s : %s", nameSpace, className, parentClassName );

                            // If we don't already have a class, just pick the first one.
                            if( m_MonoClassName[0] == '\0' )
                            {
                                strcpy_s( m_MonoClassName, 255, className );
                            }

                            if( strcmp( className, m_MonoClassName ) == 0 )
                                currentClassIndex = (int)validClasses.size();

                            validClasses.push_back( className );
                        }
                    }
                }

                // Get list of methods from a class.
                //void* iter = NULL;
                //MonoMethod* method;
                //while(method = mono_class_get_methods(mono_class, &iter))
                //{
                //    cout << mono_method_full_name(method, 1);
                //}
            }

            if( currentClassIndex != -1 )
            {
                if( ImGui::BeginCombo( "MonoClass", m_MonoClassName ) )
                {
                    for( uint32 n = 0; n < validClasses.size(); n++ )
                    {
                        bool is_selected = (n == (uint32)currentClassIndex);
                        if( ImGui::Selectable( validClasses[n].c_str(), is_selected ) )
                        {
                            strcpy_s( m_MonoClassName, 255, validClasses[n].c_str() );
                            m_ScriptLoaded = false;
                            LoadScript();
                        }
                        if( is_selected )
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            }
        }
    }

    if( ImGui::GetIO().MouseDown[0] == false )
    {
        m_LinkNextUndoCommandToPrevious = false;
    }

    // Add all exposed variables.
    ImGui::Indent( 20 );
    AddExposedVariablesToInterface();
    ImGui::Unindent( 20 );
}
#endif

void ComponentMonoScript::OnFileUpdated(MyFileObject* pFile) // StaticOnFileUpdated
{
    if( pFile == m_pScriptFile )
    {
        m_ScriptLoaded = false;
        m_ErrorInScript = false;

        if( m_Playing )
        {
            m_CallMonoOnPlayNextTickOrAfterScriptIsFinishedLoading = true;
        }
    }
}

void* ComponentMonoScript::OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
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
            if( strcmp( pFile->GetExtensionWithDot(), ".cs" ) == 0 )
            {
                oldPointer = m_pScriptFile;
                ClearExposedVariableList( changedByInterface ? true : false );
                SetScriptFile( pFile );
            }
        }
    }

    return oldPointer;
}

void* ComponentMonoScript::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
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

    //if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    //{
    //}

    return oldpointer;
}

#if MYFW_USING_IMGUI
void ComponentMonoScript::OnRightClickCallback(ComponentVariable* pVar)
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
            m_pEngineCore->GetCommandStack()->Add(
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

void ComponentMonoScript::OnExposedVarValueChanged(ExposedVariableDesc* pVar, int component, bool finishedChanging, ExposedVariableValue oldValue, void* oldPointer) // StaticOnExposedVarValueChanged
{
    // Register/unregister GameObject onDelete callbacks.
    if( pVar->value.type == ExposedVariableType::GameObject )
    {
        GameObject* pOldGameObject = static_cast<GameObject*>( oldPointer );
        GameObject* pGameObject = static_cast<GameObject*>( pVar->value.valuePointer );

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
    UpdateChildrenWithNewValue( pVar, finishedChanging, oldValue.valueDouble, oldPointer );

    // Update the mono state with the new value.
    ProgramVariables( true );
}
#endif //MYFW_EDITOR

cJSON* ComponentMonoScript::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* jComponent = ComponentUpdateable::ExportAsJSONObject( savesceneid, saveid );

    if( m_pScriptFile )
        cJSON_AddStringToObject( jComponent, "Script", m_pScriptFile->GetFullPath() );

    if( m_MonoClassName[0] != '\0' )
        cJSON_AddStringToObject( jComponent, "MonoClassName", m_MonoClassName );

    // Save the array of exposed variables.
    if( m_ExposedVars.Count() > 0 )
    {
        cJSON* jExposedVarArray = ExportExposedVariablesAsJSONObject();
        cJSON_AddItemToObject( jComponent, "ExposedVars", jExposedVarArray );
    }

    return jComponent;
}

void ComponentMonoScript::ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid)
{
    //ComponentUpdateable::ImportFromJSONObject( jsonobj, sceneid );

    cJSON* scriptstringobj = cJSON_GetObjectItem( jsonobj, "Script" );
    if( scriptstringobj )
    {
        EngineFileManager* pEngineFileManager = static_cast<EngineFileManager*>( m_pEngineCore->GetManagers()->GetFileManager() );
        MyFileObject* pFile = pEngineFileManager->RequestFile( scriptstringobj->valuestring, GetSceneID() );
        MyAssert( pFile );
        if( pFile )
        {
            SetScriptFile( pFile );
            pFile->Release(); // Free ref added by RequestFile.
        }
    }

    cJSON* jMonoClassName = cJSON_GetObjectItem( jsonobj, "MonoClassName" );
    if( jMonoClassName )
        strcpy_s( m_MonoClassName, 255, jMonoClassName->valuestring );

    // Load the array of exposed variables.
    cJSON* jExposedVarArray = cJSON_GetObjectItem( jsonobj, "ExposedVars" );
    if( jExposedVarArray )
    {
        ImportExposedVariablesFromJSONObject( jExposedVarArray );
    }
}

ComponentMonoScript& ComponentMonoScript::operator=(const ComponentMonoScript& other)
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

void ComponentMonoScript::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMonoScript, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMonoScript, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMonoScript, Draw );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMonoScript, OnTouch );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMonoScript, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMonoScript, OnKeys );
    }
}

void ComponentMonoScript::UnregisterCallbacks()
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

void ComponentMonoScript::SetScriptFile(MyFileObject* script)
{
    if( script )
        script->AddRef();

    SAFE_RELEASE( m_pScriptFile );
    m_pScriptFile = script;

    m_ScriptLoaded = false;
    m_ErrorInScript = false;

    if( m_Playing )
    {
        m_CallMonoOnPlayNextTickOrAfterScriptIsFinishedLoading = true;
    }
}

void ComponentMonoScript::LoadScript(bool forceLoad)
{
#if !MYFW_USING_MONO
    return;
#endif //!MYFW_USING_MONO

    if( m_ScriptLoaded == true && forceLoad == false )
        return;

    m_pMonoFuncPtr_OnLoad = nullptr;
    m_pMonoFuncPtr_OnPlay = nullptr;
    m_pMonoFuncPtr_OnStop = nullptr;
    m_pMonoFuncPtr_OnTouch = nullptr;
    m_pMonoFuncPtr_OnButtons = nullptr;
    m_pMonoFuncPtr_Update = nullptr;

    // Unregister all event callbacks, they will be Registered again based on what the script needs.
    EventManager* pEventManager = m_pEngineCore->GetManagers()->GetEventManager();
    pEventManager->UnregisterForEvents( "Keyboard", this, &ComponentMonoScript::StaticOnEvent );

    // Script is ready, so run it.
    //if( m_pScriptFile != nullptr )
    if( m_MonoClassName[0] != '\0' )
    {
        m_ScriptLoaded = true;

        // Get the class object from mono.
        MonoDomain* pMonoDomain = m_pMonoGameState->GetActiveDomain();
        MonoImage* pMonoImage = m_pMonoGameState->GetImage();
        
        if( pMonoImage )
        {
            MonoClass* pClass = mono_class_from_name( pMonoImage, "", m_MonoClassName );

            // Get pointers to all the interface methods of the object.
            if( pClass )
            {
                {
                    MonoMethod* pMonoMethod;

                    pMonoMethod = mono_class_get_method_from_name( pClass, "OnLoad", 0 );
                    if( pMonoMethod )
                        m_pMonoFuncPtr_OnLoad = (OnLoadFunc*)mono_method_get_unmanaged_thunk( pMonoMethod );

                    pMonoMethod = mono_class_get_method_from_name( pClass, "OnPlay", 0 );
                    if( pMonoMethod )
                        m_pMonoFuncPtr_OnPlay = (OnPlayFunc*)mono_method_get_unmanaged_thunk( pMonoMethod );

                    pMonoMethod = mono_class_get_method_from_name( pClass, "OnStop", 0 );
                    if( pMonoMethod )
                        m_pMonoFuncPtr_OnStop = (OnStopFunc*)mono_method_get_unmanaged_thunk( pMonoMethod );

                    pMonoMethod = mono_class_get_method_from_name( pClass, "OnTouch", 6 );
                    if( pMonoMethod )
                        m_pMonoFuncPtr_OnTouch = (OnTouchFunc*)mono_method_get_unmanaged_thunk( pMonoMethod );

                    pMonoMethod = mono_class_get_method_from_name( pClass, "OnButtons", 2 );
                    if( pMonoMethod )
                        m_pMonoFuncPtr_OnButtons = (OnButtonsFunc*)mono_method_get_unmanaged_thunk( pMonoMethod );

                    pMonoMethod = mono_class_get_method_from_name( pClass, "Update", 1 );
                    if( pMonoMethod )
                        m_pMonoFuncPtr_Update = (UpdateFunc*)mono_method_get_unmanaged_thunk( pMonoMethod );
                }

                // Create an instance of this class type and call the constructor.
                m_pMonoObjectInstance = mono_object_new( pMonoDomain, pClass );
                MonoMethod* pConstructor = mono_class_get_method_from_name( pClass, ".ctor", 0 );
                mono_runtime_invoke( pConstructor, m_pMonoObjectInstance, nullptr, nullptr );

                // Create and setup the GameObject variable in this m_pMonoObjectInstance.
                {
                    MonoClass* pGameObjectClass = mono_class_from_name( pMonoImage, "MyEngine", "GameObject" );
                    if( pGameObjectClass )
                    {
                        MonoObject* pMonoGameObjectInstance = mono_object_new( pMonoDomain, pGameObjectClass );
                        mono_runtime_object_init( pMonoGameObjectInstance );
            
                        MonoClassField* pNativeGameObjectField = mono_class_get_field_from_name( pGameObjectClass, "m_pNativeObject" );
                        mono_field_set_value( pMonoGameObjectInstance, pNativeGameObjectField, &m_pGameObject );

                        MonoClassField* pGameObjectField = mono_class_get_field_from_name( pClass, "m_GameObject" );
                        mono_field_set_value( m_pMonoObjectInstance, pGameObjectField, pMonoGameObjectInstance );
                    }
                }

                // Call the OnLoad method.
                MonoException* pException = nullptr;
                m_pMonoFuncPtr_OnLoad( m_pMonoObjectInstance, &pException );
                if( pException )
                {
                    char* str = mono_string_to_utf8( mono_object_to_string( (MonoObject*)pException, nullptr ) );
                    LOGError( "MonoScript", "Exception thrown calling OnLoad(): %s\n", str );
                }

                ParseExterns( m_pMonoGameState );

        //      // If OnKeys() exists as a lua function, then register for keyboard events.
        //      if( DoesFunctionExist( "OnKeys" ) )
        //      {
        //          EventManager* pEventManager = m_pEngineCore->GetManagers()->GetEventManager();
        //          pEventManager->RegisterForEvents( "Keyboard", this, &ComponentMonoScript::StaticOnEvent );
        //      }
            }
        }
    }
}

void ComponentMonoScript::ParseExterns(MonoGameState* pMonoGameState)
{
    // Mark all variables unused.
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        m_ExposedVars[i]->inUse = false;
    }

    MonoDomain* pMonoDomain = m_pMonoGameState->GetActiveDomain();
    MonoImage* pMonoImage = m_pMonoGameState->GetImage();

    MonoClass* pClass = mono_class_from_name( pMonoImage, "", m_MonoClassName );

    if( pClass )
    {
        // TODO: Grab parent fields recursively.
        //MonoClass* pParentClass = mono_class_get_parent( pClass );

        MonoClassField* pField;
        void* iter = nullptr;

        while( pField = mono_class_get_fields( pClass, &iter ) )
        {
            // Get the basic properties of each field.
            const char* varName = mono_field_get_name( pField );
            int32 varFlags = mono_field_get_flags( pField );
            MonoType* varType = mono_field_get_type( pField );

            // We need an instance of the object to grab initial values.
            MonoObject* pInstance = m_pMonoObjectInstance;
            MyAssert( pInstance );

            if( varFlags & MONO_FIELD_ATTR_PUBLIC )
            {
                const char* typeName = mono_type_get_name( varType );

                ExposedVariableDesc* pVar = nullptr;

                // Find any old variable with the same name.
                for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
                {
                    if( strcmp( m_ExposedVars[i]->name.c_str(), varName ) == 0 )
                    {
                        pVar = m_ExposedVars[i];
                        pVar->inUse = true; // Was in use.
                        break;
                    }
                }

                // If not found, create a new one and add it to the list.
                if( pVar == nullptr )
                {
                    pVar = MyNew ExposedVariableDesc();
                    m_ExposedVars.Add( pVar );
                    pVar->inUse = false; // Is new, will be marked inUse after being initialized.
                }

                pVar->name = varName;

                if( strcmp( typeName, "System.Single" ) == 0 )
                {
                    // If it's a new variable or it changed type, set it to it's initial value.
                    if( pVar->inUse == false || pVar->value.type != ExposedVariableType::Float )
                    {
                        float value;
                        mono_field_get_value( pInstance, pField, &value );
                        pVar->value.valueDouble = value;
                    }

                    pVar->value.type = ExposedVariableType::Float;
                    pVar->inUse = true;
                }
                else if( strcmp( typeName, "MyEngine.vec3" ) == 0 )
                {
                    // If it's a new variable or it changed type, set it to it's initial value.
                    if( pVar->inUse == false || pVar->value.type != ExposedVariableType::Vector3 )
                    {
                        Vector3 value;
                        mono_field_get_value( pInstance, pField, &value );
                        pVar->value.valueVec3 = value;
                    }

                    pVar->value.type = ExposedVariableType::Vector3;
                    pVar->inUse = true;
                }
                else if( strcmp( typeName, "System.Boolean" ) == 0 )
                {
                    // If it's a new variable or it changed type, set it to it's initial value.
                    if( pVar->inUse == false || pVar->value.type != ExposedVariableType::Bool )
                    {
                        bool value;
                        mono_field_get_value( pInstance, pField, &value );
                        pVar->value.valueBool = value;
                    }

                    pVar->value.type = ExposedVariableType::Bool;
                    pVar->inUse = true;
                }
                else if( strcmp( typeName, "MyEngine.GameObject" ) == 0 )
                {
                    // If it's a new variable or it changed type, set it to it's initial value.
                    if( pVar->inUse == false || pVar->value.type != ExposedVariableType::GameObject )
                    {
                        void* value;
                        mono_field_get_value( pInstance, pField, &value );
                        pVar->value.valuePointer = value;
                    }
                    
                    pVar->value.type = ExposedVariableType::GameObject;
                    pVar->inUse = true;
                }
                else
                {
                    LOGInfo( LOGTag, "Exposed variable type not supported: %s", typeName );
                    pVar->inUse = false; // Will be removed from list below.
                }
            }
        }
    }

    // Delete unused variables.
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        if( m_ExposedVars[i]->inUse == false )
        {
            ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex_MaintainOrder( i );

            // Unregister gameobject deleted callback, if we registered one.
            if( pVariable->value.type == ExposedVariableType::GameObject && pVariable->value.valuePointer )
                static_cast<GameObject*>( pVariable->value.valuePointer )->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

            delete pVariable;
            i--;
        }
    }
}

void ComponentMonoScript::ProgramVariables(bool updateExposedVariables)
{
    if( m_ScriptLoaded == false )
        return;

    // Only program the exposed vars if they change.
    if( updateExposedVariables )
    {
        MonoDomain* pMonoDomain = m_pMonoGameState->GetActiveDomain();
        MonoImage* pMonoImage = m_pMonoGameState->GetImage();

        MonoClass* pClass = mono_class_from_name( pMonoImage, "", m_MonoClassName );

        if( pClass )
        {
            for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
            {
                ExposedVariableDesc* pVar = m_ExposedVars[i];

                switch( pVar->value.type )
                {
                case ExposedVariableType::Float:
                    {
                        MonoClassField* pField = mono_class_get_field_from_name( pClass, pVar->name.c_str() );
                        float value = (float)pVar->value.valueDouble;
                        mono_field_set_value( m_pMonoObjectInstance, pField, &value );
                    }
                    break;

                case ExposedVariableType::Bool:
                    {
                        MonoClassField* pField = mono_class_get_field_from_name( pClass, pVar->name.c_str() );
                        bool value = (bool)pVar->value.valueBool;
                        mono_field_set_value( m_pMonoObjectInstance, pField, &value );
                    }
                    break;

                case ExposedVariableType::Vector3:
                    {
                        MonoClassField* pField = mono_class_get_field_from_name( pClass, pVar->name.c_str() );

                        MonoClass* pVec3Class = mono_class_from_name( pMonoImage, "MyEngine", "vec3" );
                        MonoClassField* pFieldX = mono_class_get_field_from_name( pVec3Class, "x" );
                        MonoClassField* pFieldY = mono_class_get_field_from_name( pVec3Class, "y" );
                        MonoClassField* pFieldZ = mono_class_get_field_from_name( pVec3Class, "z" );

                        MonoObject* pVec3Instance = mono_field_get_value_object( pMonoDomain, pField, m_pMonoObjectInstance );
                        Vector3 value = pVar->value.valueVec3;
                        mono_field_set_value( pVec3Instance, pFieldX, &value.x );
                        mono_field_set_value( pVec3Instance, pFieldY, &value.y );
                        mono_field_set_value( pVec3Instance, pFieldZ, &value.z );
                    }
                    break;

                case ExposedVariableType::GameObject:
                    {
                        MonoClassField* pField = mono_class_get_field_from_name( pClass, pVar->name.c_str() );
                        GameObject* pGameObject = (GameObject*)pVar->value.valuePointer;
                        MonoObject* pMonoGO = Mono_ConstructGameObject( pGameObject );
                        mono_field_set_value( m_pMonoObjectInstance, pField, pMonoGO );
                    }
                    break;
                
                case ExposedVariableType::Unused:
                    MyAssert( false );
                    break;
                }
            }
        }
    }
}

void ComponentMonoScript::SetExternFloat(const char* name, float newValue)
{
//#if MYFW_EDITOR
//    // Make sure the variable exists and the type matches.
//    bool found = false;
//
//    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
//    {
//        ExposedVariableDesc* pVar = m_ExposedVars[i];
//        if( pVar->name == name )
//        {
//            found = true;
//            if( pVar->value.type != ExposedVariableType::Float )
//            {
//                LOGError( LOGTag, "SetExternFloat called on wrong type: %d", pVar->value.type );
//                return;
//            }
//
//            pVar->valuedouble = newValue;
//        }
//    }
//
//    if( found == false )
//    {
//        if( m_ScriptLoaded == true )
//        {
//            LOGError( LOGTag, "Extern not found: %s", name );
//            return;
//        }
//
//        ExposedVariableDesc* pVar = MyNew ExposedVariableDesc();
//        m_ExposedVars.Add( pVar );
//
//        pVar->Reset();
//
//        pVar->name = name;
//        pVar->value.type = ExposedVariableType::Float;
//        pVar->valuedouble = newValue;
//        return;
//    }
//#endif
//
//    // Get the GameObjectData table from the lua state for this object.
//    luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );
//    MyAssert( gameObjectData.isTable() );
//
//    // Set the new value.
//    gameObjectData[name] = newValue;
}

void ComponentMonoScript::HandleMonoError(const char* functionname, const char* errormessage)
{
    //m_ErrorInScript = true;
    //LuaBridgeExt_LogExceptionFormattedForVisualStudioOutputWindow( functionname, m_pScriptFile->GetFullPath(), errormessage );

    //// Pop the error message off the lua stack.
    //lua_pop( m_pLuaGameState->m_pLuaState, 1 );
}

void ComponentMonoScript::OnLoad()
{
    ComponentUpdateable::OnLoad();

    m_ScriptLoaded = false;
    m_ErrorInScript = false;
}

void ComponentMonoScript::OnPlay()
{
    ComponentUpdateable::OnPlay();

    if( m_ErrorInScript )
        return;

    if( m_pScriptFile && m_pScriptFile->GetFileLoadStatus() != FileLoadStatus_Success )
    {
        LOGInfo( LOGTag, "Script warning: OnPlay, script not loaded: %s\n", m_pScriptFile->GetFilenameWithoutExtension() );
    }

    m_CallMonoOnPlayNextTickOrAfterScriptIsFinishedLoading = true;
}

void ComponentMonoScript::OnStop()
{
    ComponentUpdateable::OnStop();

    if( m_Playing && m_ErrorInScript == false )
    {
        MonoException* pException = nullptr;
        m_pMonoFuncPtr_OnStop( m_pMonoObjectInstance, &pException );
        if( pException )
        {
            char* str = mono_string_to_utf8( mono_object_to_string( (MonoObject*)pException, nullptr ) );
            LOGError( "MonoScript", "Exception thrown calling OnStop(): %s\n", str );
        }
    }

    m_ScriptLoaded = false;
    m_ErrorInScript = false;
    m_Playing = false;
}

void ComponentMonoScript::OnGameObjectEnabled()
{
    ComponentBase::OnGameObjectEnabled();
    if( m_pEngineCore->IsInEditorMode() == false )
        OnPlay();
}

void ComponentMonoScript::OnGameObjectDisabled()
{
    ComponentBase::OnGameObjectDisabled();
}

void ComponentMonoScript::TickCallback(float deltaTime)
{
    //ComponentBase::TickCallback( deltaTime );
    //ComponentUpdateable::Tick( deltaTime );

    if( m_ErrorInScript )
        return;

    if( m_ScriptLoaded == false && m_pMonoGameState )
    {
        LoadScript();
    }

    if( m_pMonoGameState == nullptr || m_ScriptLoaded == false )
        return;

    // Copy externed variable values after loading the script.
    if( m_pCopyExternsFromThisComponentAfterLoadingScript )
    {
        const ComponentMonoScript& other = *m_pCopyExternsFromThisComponentAfterLoadingScript;
        m_pCopyExternsFromThisComponentAfterLoadingScript = nullptr;

        CopyExposedVariablesFromOtherComponent( other );

        ProgramVariables( true );
    }

    if( m_CallMonoOnPlayNextTickOrAfterScriptIsFinishedLoading )
    {
        m_CallMonoOnPlayNextTickOrAfterScriptIsFinishedLoading = false;

        // Find the OnPlay function and call it, look for a table that matched our filename.
        if( m_pMonoObjectInstance )
        {
            if( m_pMonoFuncPtr_OnPlay )
            {
                // Program the exposed variable values.
                ProgramVariables( true );

                // Call OnPlay().
                MonoException* pException = nullptr;
                m_pMonoFuncPtr_OnPlay( m_pMonoObjectInstance, &pException );
                if( pException )
                {
                    char* str = mono_string_to_utf8( mono_object_to_string( (MonoObject*)pException, nullptr ) );
                    LOGError( "MonoScript", "Exception thrown calling OnPlay(): %s\n", str );
                }
            }

            m_Playing = true;
        }
    }

    // Call Update().
    if( m_Playing )
    {
        MonoException* pException = nullptr;
        m_pMonoFuncPtr_Update( m_pMonoObjectInstance, deltaTime, &pException );
        if( pException )
        {
            char* str = mono_string_to_utf8( mono_object_to_string( (MonoObject*)pException, nullptr ) );
            LOGError( "MonoScript", "Exception thrown calling Update(): %s\n", str );
        }
    }
}

bool ComponentMonoScript::OnTouchCallback(int action, int id, float x, float y, float pressure, float size)
{
    //ComponentBase::OnTouchCallback( action, id, x, y, pressure, size );

    if( m_ErrorInScript )
        return false;

    // Find the OnTouch function and call it.
    if( m_Playing )
    {
        MonoException* pException = nullptr;
        bool result = m_pMonoFuncPtr_OnTouch( m_pMonoObjectInstance, action, id, x, y, pressure, size, &pException );
        if( pException )
        {
            char* str = mono_string_to_utf8( mono_object_to_string( (MonoObject*)pException, nullptr ) );
            LOGError( "MonoScript", "Exception thrown calling OnTouch(): %s\n", str );
            return false;
        }
        return result;
    }

    return false;
}

bool ComponentMonoScript::OnButtonsCallback(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    //ComponentBase::OnButtonsCallback( action, id );

    if( m_ErrorInScript )
        return false;

    // Find the OnButtons function and call it.
    if( m_Playing )
    {
        int a = action;
        int i = id;
        MonoException* pException = nullptr;
        bool result = m_pMonoFuncPtr_OnButtons( m_pMonoObjectInstance, a, i, &pException );
        if( pException )
        {
            char* str = mono_string_to_utf8( mono_object_to_string( (MonoObject*)pException, nullptr ) );
            LOGError( "MonoScript", "Exception thrown calling OnButtons(): %s\n", str );
            return false;
        }
        return result;
    }

    return false;
}

void ComponentMonoScript::OnGameObjectDeleted(GameObject* pGameObject)
{
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];

        // If any of our variables is a gameobject, clear it if that gameobject was deleted.
        if( pVar->value.type == ExposedVariableType::GameObject )
        {
            if( pVar->value.valuePointer == pGameObject )
            {
#if MYFW_EDITOR
                if( m_pEngineCore->GetCommandStack() )
                {
                    // TODO: Removed when converting to mono.
                    //m_pEngineCore->GetCommandStack()->Do( MyNew EditorCommand_LuaExposedVariablePointerChanged(
                    //    nullptr, pVar, ComponentMonoScript::StaticOnExposedVarValueChanged, this ), true );
                }
#endif
                pVar->value.valuePointer = nullptr;
            }
        }
    }
}

bool ComponentMonoScript::OnEvent(MyEvent* pEvent) // StaticOnEvent
{
    // Only registered for Keyboard events ATM.
    MyAssert( pEvent->IsType( "Keyboard" ) );

    int action = pEvent->GetInt( "Action" );
    int keyCode = pEvent->GetInt( "KeyCode" );

    // TODO: Removed when converting to mono.
    //MonoException* pException = nullptr;
    //bool result = m_pMonoFuncPtr_OnKeys( m_pMonoObjectInstance, action, keyCode, &pException ) )
    //if( pException )
    //{
    //    char* str = mono_string_to_utf8( mono_object_to_string( (MonoObject*)pException, nullptr ) );
    //    LOGError( "MonoScript", "Exception thrown calling OnKeys(): %s\n", str );
    //    return false;
    //}
    // return result;

    return false;
}

#endif //MYFW_USING_MONO
