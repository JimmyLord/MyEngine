//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentLuaScript::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentLuaScript ); //_VARIABLE_LIST

ComponentLuaScript::ComponentLuaScript()
: ComponentUpdateable()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Updateable;
    m_Type = ComponentType_LuaScript;

    m_pScriptFile = 0;

    m_ExposedVars.AllocateObjects( MAX_EXPOSED_VARS ); // hard coded nonsense for now, max of 4 exposed vars in a script.

#if MYFW_USING_WX
    g_pComponentSystemManager->Editor_RegisterFileUpdatedCallback( &StaticOnFileUpdated, this );
#endif
}

ComponentLuaScript::~ComponentLuaScript()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    while( m_ExposedVars.Count() )
    {
        ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex( 0 );

        // unregister gameobject deleted callback, if we registered one.
        if( pVariable->type == ExposedVariableType_GameObject && pVariable->pointer )
            ((GameObject*)pVariable->pointer)->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

        delete pVariable;
    }

    SAFE_RELEASE( m_pScriptFile );
}

void ComponentLuaScript::RegisterVariables(CPPListHead* pList, ComponentLuaScript* pThis) //_VARIABLE_LIST
{
    // just want to make sure these are the same on all compilers.  They should be since this is a simple class.
    MyAssert( offsetof( ComponentLuaScript, m_pScriptFile ) == MyOffsetOf( pThis, &pThis->m_pScriptFile ) );

    // script is not automatically saved/loaded
    AddVar( pList, "Script", ComponentVariableType_FilePtr, MyOffsetOf( pThis, &pThis->m_pScriptFile ), false, true, 0, (CVarFunc_ValueChanged)&ComponentLuaScript::OnValueChangedCV, (CVarFunc_DropTarget)&ComponentLuaScript::OnDropCV, 0 );
}

void ComponentLuaScript::Reset()
{
    ComponentUpdateable::Reset();

    m_pLuaGameState = g_pLuaGameState;

    m_ScriptLoaded = false;
    m_Playing = false;
    m_ErrorInScript = false;
    m_ShouldBePlayingButIsntBecauseScriptFileWasStillLoading = false;

    while( m_ExposedVars.Count() )
    {
        ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex( 0 );
        delete pVariable;
    }

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
    m_ControlIDOfFirstExtern = -1;
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void ComponentLuaScript::CreateNewScriptFile()
{
    //if( m_pScriptFile == 0 )
    {
        // generally offer to create scripts in Scripts folder.
        wxString initialpath = "./Data/Scripts";

        bool ismenuactionscript = false;

        // if a menupage component is attached to this game objects, then offer to make the file in the menus folder.
        if( m_pGameObject->GetFirstComponentOfType( "MenuPageComponent" ) != 0 )
        {
            ismenuactionscript = true;
            initialpath = "./Data/Menus";
        }

        wxFileDialog FileDialog( g_pEngineMainFrame, _("Create Lua script file"), initialpath, "", "Lua script files (*.lua)|*.lua", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
        if( FileDialog.ShowModal() != wxID_CANCEL )
        {
            wxString wxpath = FileDialog.GetPath();
            char fullpath[MAX_PATH];
            sprintf_s( fullpath, MAX_PATH, "%s", (const char*)wxpath );
            const char* relativepath = GetRelativePath( fullpath );

            MyFileObject* pScriptFile = g_pComponentSystemManager->LoadDataFile( relativepath, m_pGameObject->GetSceneID(), 0 );
            SetScriptFile( pScriptFile );

            // update the panel so new filename shows up. // TODO: this won't refresh lua variables, so maybe refresh the whole watch panel.
            int scriptcontrolid = FindVariablesControlIDByLabel( "Script" );
            g_pPanelWatch->m_pVariables[scriptcontrolid].m_Description = m_pScriptFile->m_FullPath;

            // TODO: create a template file.
            {
                FILE* file;
                fopen_s( &file, fullpath, "wb" );

                if( file )
                {
                    if( ismenuactionscript )
                    {
                        fprintf( file, "-- Menu Action File\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "%s =\n", m_pScriptFile->m_FilenameWithoutExtension );
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
                        fprintf( file, "-- %s Script\n", m_pScriptFile->m_FilenameWithoutExtension );
                        fprintf( file, "\n" );
                        fprintf( file, "%s =\n", m_pScriptFile->m_FilenameWithoutExtension );
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
                        fprintf( file, "Tick = function(timepassed)\n" );
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

void ComponentLuaScript::OnFileUpdated(MyFileObject* pFile)
{
    if( pFile == m_pScriptFile )
    {
        m_ScriptLoaded = false;
        m_ErrorInScript = false;

        if( m_Playing )
            m_ShouldBePlayingButIsntBecauseScriptFileWasStillLoading = true;
    }
}

void ComponentLuaScript::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentLuaScript::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Lua script" );
}

void ComponentLuaScript::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentLuaScript::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Lua Script", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    m_ControlIDOfFirstExtern = -1;

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        if( addcomponentvariables )
            FillPropertiesWindowWithVariables(); //_VARIABLE_LIST

        //const char* desc = "no script";
        //if( m_pScriptFile )
        //    desc = m_pScriptFile->m_FullPath;
        //m_ControlID_Script = g_pPanelWatch->AddPointerWithDescription( "Script", 0, desc, this, ComponentLuaScript::StaticOnDrop );

        for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
        {
            int id = -1;

            ExposedVariableDesc* pVar = m_ExposedVars[i];

            if( pVar->type == ExposedVariableType_Float )
            {
                id = g_pPanelWatch->AddDouble( pVar->name.c_str(), &pVar->value, 0, 0, this, ComponentLuaScript::StaticOnValueChanged );
            }
            else if( pVar->type == ExposedVariableType_GameObject )
            {
                // setup name and drag and drop of game objects.
                const char* desc = "no gameobject";
                if( pVar->pointer )
                    desc = ((GameObject*)pVar->pointer)->GetName();
                id = g_pPanelWatch->AddPointerWithDescription( pVar->name.c_str(), pVar->pointer, desc, this, ComponentLuaScript::StaticOnDrop, ComponentLuaScript::StaticOnValueChanged );
            }

            if( m_ControlIDOfFirstExtern == -1 )
                m_ControlIDOfFirstExtern = id;

            pVar->controlID = id;
        }
    }
}

void ComponentLuaScript::AppendItemsToRightClickMenu(wxMenu* pMenu)
{
    ComponentBase::AppendItemsToRightClickMenu( pMenu );

    pMenu->Append( RightClick_CreateNewScriptFile, "Create new script" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentLuaScript::OnPopupClick );
}

void ComponentLuaScript::OnPopupClick(wxEvent &evt)
{
    ComponentBase::OnPopupClick( evt );

    ComponentLuaScript* pComponent = (ComponentLuaScript*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    MyAssert( pComponent->IsA( "LuaScriptComponent" ) );

    int id = evt.GetId();

    switch( id )
    {
    case RightClick_CreateNewScriptFile:    pComponent->CreateNewScriptFile();     break;
    }
}

void* ComponentLuaScript::OnDropCV(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldvalue = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        MyAssert( pFile );

        if( strcmp( pFile->m_ExtensionWithDot, ".lua" ) == 0 )
        {
            oldvalue = m_pScriptFile;
            SetScriptFile( pFile );

            // update the panel so new filename shows up.
            // TODO: this won't refresh lua variables, so maybe refresh the whole watch panel.
            g_pPanelWatch->m_pVariables[pVar->m_ControlID].m_Description = m_pScriptFile->m_FullPath;
        }
    }

    return oldvalue;
}

void* ComponentLuaScript::OnValueChangedCV(ComponentVariable* pVar, bool finishedchanging, double oldvalue)
{
    void* oldpointer = 0;

    if( strcmp( pVar->m_Label, "Script" ) == 0 )
    {
        wxString text = g_pPanelWatch->m_pVariables[pVar->m_ControlID].m_Handle_TextCtrl->GetValue();
        if( text == "" || text == "none" || text == "no script" )
        {
            g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "no script" );
            oldpointer = m_pScriptFile;
            this->SetScriptFile( 0 );
        }
    }

    return oldpointer;
}

void* ComponentLuaScript::ProcessOnDrop(int controlid, wxCoord x, wxCoord y)
{
    void* oldpointer = 0;

    ComponentUpdateable::OnDrop( controlid, x, y );

    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        MyAssert( pFile );

        if( strcmp( pFile->m_ExtensionWithDot, ".lua" ) == 0 )
        {
            oldpointer = m_pScriptFile;
            SetScriptFile( pFile );

            // update the panel so new filename shows up. // TODO: this won't refresh lua variables, so maybe refresh the whole watch panel.
            int scriptcontrolid = FindVariablesControlIDByLabel( "Script" );
            g_pPanelWatch->m_pVariables[scriptcontrolid].m_Description = m_pScriptFile->m_FullPath;
        }
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        GameObject* pGameObject = (GameObject*)g_DragAndDropStruct.m_Value;
        MyAssert( pGameObject );

        MyAssert( m_ControlIDOfFirstExtern != -1 );
        if( m_ControlIDOfFirstExtern != -1 )
        {
            int id = g_DragAndDropStruct.m_ID - m_ControlIDOfFirstExtern;
        
            MyAssert( id < (int)m_ExposedVars.Count() );
            if( id < (int)m_ExposedVars.Count() )
            {
                MyAssert( pGameObject->IsA( "GameObject" ) );
        
                // unregister the old gameobject.
                if( m_ExposedVars[id]->pointer )
                    ((GameObject*)m_ExposedVars[id]->pointer)->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

                oldpointer = m_ExposedVars[id]->pointer;
                m_ExposedVars[id]->pointer = pGameObject;
                ((GameObject*)m_ExposedVars[id]->pointer)->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

                // update the panel so new gameobject name shows up.
                g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pGameObject->GetName();
            }
        }
    }

    return oldpointer;
}

void ComponentLuaScript::OnDrop(int controlid, wxCoord x, wxCoord y)
{
    void* oldpointer = ProcessOnDrop( controlid, x, y );

    UpdateChildrenWithNewValue( controlid, true, 0, oldpointer );
}

void ComponentLuaScript::OnValueChanged(int controlid, bool finishedchanging, double oldvalue)
{
    if( m_ControlIDOfFirstExtern != -1 )
    {
        int id = controlid - m_ControlIDOfFirstExtern;
        
        MyAssert( id < (int)m_ExposedVars.Count() );
        if( id < (int)m_ExposedVars.Count() )
        {
            if( m_ExposedVars[id]->type == ExposedVariableType_GameObject )
            {
                ExposedVariableDesc* pVariable = m_ExposedVars[id];
                GameObject* pGameObject = (GameObject*)pVariable->pointer;

                MyAssert( pGameObject->IsA( "GameObject" ) );

                wxString text = g_pPanelWatch->m_pVariables[controlid].m_Handle_TextCtrl->GetValue();
                if( text == "" || text == "none" || text == "no gameobject" )
                {
                    g_pPanelWatch->ChangeDescriptionForPointerWithDescription( controlid, "no gameobject" );

                    // unregister gameobject deleted callback, if we registered one.
                    if( pVariable->type == ExposedVariableType_GameObject && pGameObject )
                        pGameObject->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

                    void* oldpointer = m_ExposedVars[id]->pointer;

                    m_ExposedVars[id]->pointer = 0;

                    UpdateChildrenWithNewValue( controlid, true, 0, oldpointer );
                    ProgramVariables( m_pLuaGameState->m_pLuaState, true );
                    return;
                }
            }
        }
    }

    ProgramVariables( m_pLuaGameState->m_pLuaState, true );

    UpdateChildrenWithNewValue( controlid, finishedchanging, oldvalue, 0 );
}

void ComponentLuaScript::UpdateChildrenWithNewValue(int controlid, bool finishedchanging, double oldvalue, void* oldpointer)
{
    // find children of this gameobject and change their vars if needed.
    for( unsigned int varindex=0; varindex<m_ExposedVars.Count(); varindex++ )
    {
        ExposedVariableDesc* pVar = (ExposedVariableDesc*)m_ExposedVars[varindex];
        MyAssert( pVar );

        if( pVar->controlID == controlid )
        {
            // find children of this gameobject and change their values as well, if their value matches the old value.
            for( CPPListNode* pCompNode = g_pComponentSystemManager->m_GameObjects.GetHead(); pCompNode; pCompNode = pCompNode->GetNext() )
            {
                GameObject* pGameObject = (GameObject*)pCompNode;

                if( pGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject )
                {
                    // Found a game object, now find the matching component on it.
                    for( unsigned int i=0; i<pGameObject->m_Components.Count(); i++ )
                    {
                        ComponentLuaScript* pOtherLuaScript = (ComponentLuaScript*)pGameObject->m_Components[i];

                        const char* pThisCompClassName = GetClassname();
                        const char* pOtherCompClassName = pOtherLuaScript->GetClassname();

                        // TODO: this will fail if multiple of the same component are on an object.
                        if( strcmp( pThisCompClassName, pOtherCompClassName ) == 0 )
                        {
                            // Found the matching component, now compare the variable.
                            if( pVar->type == ExposedVariableType_Float )
                            {
                                if( pOtherLuaScript->m_ExposedVars[varindex]->value == oldvalue )
                                {
                                    pOtherLuaScript->m_ExposedVars[varindex]->value = pVar->value;
                                    pOtherLuaScript->OnValueChanged( controlid, finishedchanging, oldvalue );
                                    ProgramVariables( m_pLuaGameState->m_pLuaState, true );

                                    pOtherLuaScript->UpdateChildrenWithNewValue( controlid, finishedchanging, oldvalue, oldpointer );
                                }
                            }

                            if( pVar->type == ExposedVariableType_GameObject )
                            {
                                if( pOtherLuaScript->m_ExposedVars[varindex]->pointer == oldpointer )
                                {
                                    pOtherLuaScript->m_ExposedVars[varindex]->pointer = pVar->pointer;
                                    if( pVar->pointer )
                                        ((GameObject*)pVar->pointer)->RegisterOnDeleteCallback( pOtherLuaScript, StaticOnGameObjectDeleted );
                                    ProgramVariables( m_pLuaGameState->m_pLuaState, true );

                                    pOtherLuaScript->UpdateChildrenWithNewValue( controlid, finishedchanging, oldvalue, oldpointer );
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentLuaScript::ExportAsJSONObject(bool savesceneid)
{
    cJSON* component = ComponentUpdateable::ExportAsJSONObject( savesceneid );

    if( m_pScriptFile )
        cJSON_AddStringToObject( component, "Script", m_pScriptFile->m_FullPath );

    // save the array of exposed variables
    cJSON* exposedvararray = cJSON_CreateArray();
    cJSON_AddItemToObject( component, "ExposedVars", exposedvararray );
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];
        
        cJSON* jsonvar = cJSON_CreateObject();
        cJSON_AddItemToArray( exposedvararray, jsonvar );

        cJSON_AddStringToObject( jsonvar, "Name", pVar->name.c_str() );
        cJSON_AddNumberToObject( jsonvar, "Type", pVar->type );

        if( pVar->type == ExposedVariableType_Float )
        {
            cJSON_AddNumberToObject( jsonvar, "Value", pVar->value );
        }
        else if( pVar->type == ExposedVariableType_GameObject && pVar->pointer )
        {
            cJSON* gameobjectref = ((GameObject*)pVar->pointer)->ExportReferenceAsJSONObject( m_SceneIDLoadedFrom );
            cJSON_AddItemToObject( jsonvar, "Value", gameobjectref );

            // TODO: find a way to uniquely identify a game object...
            //cJSON_AddStringToObject( jsonvar, "Value", ((GameObject*)pVar->pointer)->GetName() );
        }
    }

    return component;
}

void ComponentLuaScript::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    //ComponentUpdateable::ImportFromJSONObject( jsonobj, sceneid );

    cJSON* scriptstringobj = cJSON_GetObjectItem( jsonobj, "Script" );
    if( scriptstringobj )
    {
        MyFileObject* pFile = g_pFileManager->RequestFile( scriptstringobj->valuestring );
        MyAssert( pFile );
        if( pFile )
        {
            SetScriptFile( pFile );
            pFile->Release(); // free ref added by RequestFile
        }
    }

    // load the array of exposed variables
    cJSON* exposedvararray = cJSON_GetObjectItem( jsonobj, "ExposedVars" );
    int numvars = cJSON_GetArraySize( exposedvararray );
    for( int i=0; i<numvars; i++ )
    {
        cJSON* jsonvar = cJSON_GetArrayItem( exposedvararray, i );

        cJSON* obj = cJSON_GetObjectItem( jsonvar, "Name" );
        MyAssert( obj );
        if( obj == 0 )
            continue;

        // by name, check if the variable is already in our list
        ExposedVariableDesc* pVar = 0;
        for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
        {
            MyAssert( m_ExposedVars[i] );
            if( m_ExposedVars[i]->name == obj->valuestring )
            {
                pVar = m_ExposedVars[i];
                break;
            }
        }

        // if not, create and add it.
        if( pVar == 0 )
        {
            pVar = MyNew ExposedVariableDesc();
            m_ExposedVars.Add( pVar );
        }

        pVar->name = obj->valuestring;
        cJSONExt_GetInt( jsonvar, "Type", (int*)&pVar->type );

        if( pVar->type == ExposedVariableType_Float )
        {
            cJSONExt_GetDouble( jsonvar, "Value", &pVar->value );
        }
        else if( pVar->type == ExposedVariableType_GameObject )
        {
            cJSON* obj = cJSON_GetObjectItem( jsonvar, "Value" );
            if( obj )
            {
                pVar->pointer = g_pComponentSystemManager->FindGameObjectByJSONRef( obj, m_pGameObject->GetSceneID() );
                ((GameObject*)pVar->pointer)->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
                //pVar->pointer = g_pComponentSystemManager->FindGameObjectByName( obj->valuestring );
            }
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

        // load the script and copy externed variable values
        LoadScript();

        for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
        {
            ExposedVariableDesc* pVar = m_ExposedVars[i];
            ExposedVariableDesc* pOtherVar = other.m_ExposedVars[i];            

            if( pVar->type == ExposedVariableType_Float )
                pVar->value = pOtherVar->value;

            if( pVar->type == ExposedVariableType_GameObject )
                pVar->pointer = pOtherVar->pointer;

            if( pVar->pointer )
                ((GameObject*)pVar->pointer)->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
        }

        ProgramVariables( m_pLuaGameState->m_pLuaState, true );
    }

    return *this;
}

void ComponentLuaScript::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
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
}

void ComponentLuaScript::LoadScript()
{
    if( m_pScriptFile == 0 || m_ScriptLoaded == true )
        return;

    // script is ready, so run it.
    if( m_pScriptFile->m_FileLoadStatus == FileLoadStatus_Success )
    {
        LOGInfo( LOGTag, "luaL_loadstring: %s\n", m_pScriptFile->m_FilenameWithoutExtension );

        // load the string from the file
        int loadretcode = luaL_loadstring( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_pBuffer );
        if( loadretcode == LUA_OK )
        {
            // run the code to do initial parsing
            int exeretcode = lua_pcall( m_pLuaGameState->m_pLuaState, 0, LUA_MULTRET, 0 );
            if( exeretcode == LUA_OK )
            {
                luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );

                if( LuaObject.isTable() )
                {
                    // Create a table to store local variables unique to this component.
                    char gameobjectname[100];
                    sprintf_s( gameobjectname, 100, "_GameObject_%d", m_pGameObject->GetID() );
                    luabridge::LuaRef datatable = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, gameobjectname );

                    if( datatable.isTable() == false )
                    {
                        luabridge::LuaRef newdatatable = luabridge::newTable( m_pLuaGameState->m_pLuaState );
                        luabridge::setGlobal( m_pLuaGameState->m_pLuaState, newdatatable, gameobjectname );
                        newdatatable["gameobject"] = m_pGameObject;
                    }

                    ParseExterns( LuaObject );
                    OnScriptLoaded();
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
                    MyAssert( false ); // assert until I hit this and deal with it better.
                    const char* errorstr = lua_tostring( m_pLuaGameState->m_pLuaState, -1 );
                    HandleLuaError( "!LUA_ERRRUN", errorstr );
                }
            }
        }
        else
        {
            if( loadretcode == LUA_ERRSYNTAX )
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

        m_ScriptLoaded = true;
    }
}

void ComponentLuaScript::ParseExterns(luabridge::LuaRef LuaObject)
{
    // mark all variables unused.
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

            ExposedVariableDesc* pVar = 0;

            // find any old variable with the same name
            for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
            {
                if( m_ExposedVars[i]->name == varname )
                {
                    pVar = m_ExposedVars[i];
                    pVar->inuse = true; // was in use.
                }
            }

            // if not found, create a new one and add it to the list.
            if( pVar == 0 )
            {
                pVar = MyNew ExposedVariableDesc();
                m_ExposedVars.Add( pVar );
                pVar->inuse = false; // is new, will be marked inuse after being initialized
            }

            pVar->name = varname;

            if( vartype == "Float" )
            {
                // if it's a new variable or it changed type, set it to it's initial value.
                if( pVar->inuse == false || pVar->type != ExposedVariableType_Float )
                    pVar->value = variableinitialvalue.cast<double>();

                pVar->type = ExposedVariableType_Float;
            }
            else if( vartype == "GameObject" )
            {
                // if it's a new variable or it changed type, set it to it's initial value.
                if( pVar->inuse == false || pVar->type != ExposedVariableType_GameObject )
                {
                    pVar->pointer = g_pComponentSystemManager->FindGameObjectByName( variableinitialvalue.tostring().c_str() );
                    if( pVar->pointer )
                        ((GameObject*)pVar->pointer)->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
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

    // delete unused variables.
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        if( m_ExposedVars[i]->inuse == false )
        {
            m_ExposedVars.RemoveIndex_MaintainOrder( i );
            i--;
        }
    }
}

void ComponentLuaScript::ProgramVariables(luabridge::LuaRef LuaObject, bool updateexposedvariables)
{
    // set "this" to the data table storing this gameobjects script data "_GameObject_<ID>"
    char gameobjectname[100];
    sprintf_s( gameobjectname, 100, "_GameObject_%d", m_pGameObject->GetID() );
    luabridge::LuaRef datatable = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, gameobjectname );
    luabridge::setGlobal( m_pLuaGameState->m_pLuaState, datatable, "this" );

    // only program the exposed vars if they change.
    if( updateexposedvariables )
    {
        for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
        {
            ExposedVariableDesc* pVar = m_ExposedVars[i];

            if( pVar->type == ExposedVariableType_Float )
                datatable[pVar->name] = pVar->value;

            if( pVar->type == ExposedVariableType_GameObject )
                datatable[pVar->name] = (GameObject*)pVar->pointer;
        }
    }
}

void ComponentLuaScript::HandleLuaError(const char* functionname, const char* errormessage)
{
    m_ErrorInScript = true;
    LuaBridgeExt_LogExceptionFormattedForVisualStudioOutputWindow( functionname, m_pScriptFile->m_FullPath, errormessage );

    // pop the error message off the lua stack
    lua_pop( m_pLuaGameState->m_pLuaState, 1 );
}

void ComponentLuaScript::OnScriptLoaded()
{
    if( m_ErrorInScript )
        return;

    //if( m_Playing )
    {
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        
        // call stop
        if( LuaObject["OnLoad"].isFunction() )
        {
            //ProgramVariables( LuaObject );
            try
            {
                LuaObject["OnLoad"]();
            }
            catch(luabridge::LuaException const& e)
            {
                HandleLuaError( "OnLoad", e.what() );
            }
        }
    }
}

void ComponentLuaScript::OnLoad()
{
    ComponentUpdateable::OnLoad();

    m_ScriptLoaded = false;
    m_ErrorInScript = false;
    LoadScript();
}

void ComponentLuaScript::OnPlay()
{
    ComponentUpdateable::OnPlay();

    if( m_ErrorInScript )
        return;

#if _DEBUG && MYFW_USING_WX
    // reload the script when play is hit in debug, for quick script edits.
    //if( m_pScriptFile && m_pScriptFile->m_FileReady == true )
    //{
    //    m_ScriptLoaded = false;
    //    //g_pFileManager->ReloadFile( m_pScriptFile );
    //    // hard loop until the filemanager is done loading the file.
    //    while( m_pScriptFile && m_pScriptFile->m_FileReady == false )
    //    {
    //        g_pFileManager->Tick();
    //    }
    //    LoadScript();
    //}
#endif

    // TODO: prevent "play" if file is not loaded.
    //       or test this a bit, Tick will call OnPlay once it's loaded.
    if( m_pScriptFile )
    {
        if( m_ScriptLoaded == false )
        {
            LoadScript();
        }

        if( m_pScriptFile->m_FileLoadStatus != FileLoadStatus_Success )
        {
            m_ShouldBePlayingButIsntBecauseScriptFileWasStillLoading = true;
            LOGInfo( LOGTag, "Script warning: OnPlay, script not loaded: %s\n", m_pScriptFile->m_FilenameWithoutExtension );
        }
        else
        {
            // find the OnPlay function and call it, look for a table that matched our filename.
            luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        
            if( LuaObject.isNil() == false )
            {
                if( LuaObject["OnPlay"].isFunction() )
                {
                    ProgramVariables( LuaObject, true );
                    try
                    {
                        LuaObject["OnPlay"]();
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
}

void ComponentLuaScript::OnStop()
{
    ComponentUpdateable::OnStop();

    if( m_Playing && m_ErrorInScript == false )
    {
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        
        // call stop
        if( LuaObject["OnStop"].isFunction() )
        {
            ProgramVariables( LuaObject, false );
            try
            {
                LuaObject["OnStop"]();
            }
            catch(luabridge::LuaException const& e)
            {
                HandleLuaError( "OnStop", e.what() );
            }
        }
    }

    m_ScriptLoaded = false;
    m_ErrorInScript = false;
    //LoadScript();
    m_Playing = false;
}

void ComponentLuaScript::OnGameObjectEnabled()
{
    ComponentBase::OnGameObjectEnabled();
    OnPlay();
}

void ComponentLuaScript::OnGameObjectDisabled()
{
    ComponentBase::OnGameObjectDisabled();
}

void ComponentLuaScript::TickCallback(double TimePassed)
{
    //ComponentBase::TickCallback( TimePassed );
    //ComponentUpdateable::Tick( TimePassed );

    if( m_ErrorInScript )
        return;

    if( m_ScriptLoaded == false && g_pLuaGameState )
    {
        LoadScript();
    }

    if( m_ScriptLoaded && m_ShouldBePlayingButIsntBecauseScriptFileWasStillLoading )
    {
        m_ShouldBePlayingButIsntBecauseScriptFileWasStillLoading = false;
        OnPlay();
    }

    // find the Tick function and call it.
    if( m_ScriptLoaded && m_Playing )
    {
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        MyAssert( LuaObject.isTable() );

        if( LuaObject.isTable() )
        {
            // call tick
            if( LuaObject["Tick"].isFunction() )
            {
                ProgramVariables( LuaObject, false );
                try
                {
                    LuaObject["Tick"]( TimePassed );
                }
                catch(luabridge::LuaException const& e)
                {
                    HandleLuaError( "Tick", e.what() );
                }
            }
        }
    }
}

bool ComponentLuaScript::OnTouchCallback(int action, int id, float x, float y, float pressure, float size)
{
    //ComponentBase::OnTouchCallback( action, id, x, y, pressure, size );

    if( m_ErrorInScript )
        return false;

    // find the OnTouch function and call it.
    if( m_Playing )
    {
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );

        // call OnTouch
        if( LuaObject["OnTouch"].isFunction() )
        {
            ProgramVariables( LuaObject, false );
            try
            {
                if( LuaObject["OnTouch"]( action, id, x, y, pressure, size ) )
                    return true;
            }
            catch(luabridge::LuaException const& e)
            {
                HandleLuaError( "OnTouch", e.what() );
            }
        }
    }

    return false;
}

bool ComponentLuaScript::OnButtonsCallback(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    //ComponentBase::OnButtonsCallback( action, id );

    if( m_ErrorInScript )
        return false;

    // find the OnButtons function and call it.
    if( m_Playing )
    {
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );

        // call OnButtons
        if( LuaObject["OnButtons"].isFunction() )
        {
            ProgramVariables( LuaObject, false );
            try
            {
                int a = action;
                int i = id;
                if( LuaObject["OnButtons"]( a, i ) )
                    return true;
            }
            catch(luabridge::LuaException const& e)
            {
                HandleLuaError( "OnButtons", e.what() );
            }
        }
    }

    return false;
}

void ComponentLuaScript::OnGameObjectDeleted(GameObject* pGameObject)
{
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];

        // if any of our variables is a gameobject, clear it if that gameobject was deleted.
        if( pVar->type == ExposedVariableType_GameObject )
        {
            if( pVar->pointer == pGameObject )
            {
#if MYFW_USING_WX
                if( g_pEngineMainFrame->m_pCommandStack )
                {
                    g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_PanelWatchPointerChanged(
                        0,
                        PanelWatchType_PointerWithDesc, &pVar->pointer, -1,
                        ComponentLuaScript::StaticOnValueChanged, this ), true );
                }
#endif
                pVar->pointer = 0;
            }
        }
    }
}
