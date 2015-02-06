//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

//using namespace luabridge;

ComponentLuaScript::ComponentLuaScript()
: ComponentUpdateable()
{
    m_BaseType = BaseComponentType_Updateable;
    m_Type = ComponentType_LuaScript;

    m_pScriptFile = 0;

    m_ExposedVars.AllocateObjects( MAX_EXPOSED_VARS ); // hard coded nonsense for now, max of 4 exposed vars in a script.
}

ComponentLuaScript::~ComponentLuaScript()
{
    while( m_ExposedVars.Count() )
    {
        ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex( 0 );
        delete pVariable;
    }

    SAFE_RELEASE( m_pScriptFile );
}

void ComponentLuaScript::Reset()
{
    ComponentUpdateable::Reset();

    m_pLuaState = g_pLuaGameState->m_pLuaState;

    m_ScriptLoaded = false;
    m_Playing = false;
    m_ShouldBePlayingButIsntBecauseScriptFileWasStillLoading = false;

    while( m_ExposedVars.Count() )
    {
        ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex( 0 );
        delete pVariable;
    }
    //m_Mass = 0;
}

#if MYFW_USING_WX
void ComponentLuaScript::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentLuaScript::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Lua script" );
}

void ComponentLuaScript::OnLeftClick(bool clear)
{
    ComponentBase::OnLeftClick( clear );
}

void ComponentLuaScript::FillPropertiesWindow(bool clear)
{
    ComponentBase::FillPropertiesWindow( clear );

    const char* desc = "no script";
    if( m_pScriptFile )
        desc = m_pScriptFile->m_FullPath;
    g_pPanelWatch->AddPointerWithDescription( "Script", 0, desc, this, ComponentLuaScript::StaticOnDrop );

    // warning: if more variables are added above, code in OnDrop() needs to change

    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];

        if( pVar->type == ExposedVariableType_Float )
        {
            g_pPanelWatch->AddDouble( pVar->name.c_str(), &pVar->value, 0, 500, this, ComponentLuaScript::StaticOnValueChanged );
        }
        else if( pVar->type == ExposedVariableType_GameObject )
        {
            // setup name and drag and drop of game objects.
            const char* desc = "no gameobject";
            if( pVar->pointer )
                desc = ((GameObject*)pVar->pointer)->GetName();
            g_pPanelWatch->AddPointerWithDescription( pVar->name.c_str(), pVar->pointer, desc, this, ComponentLuaScript::StaticOnDrop, ComponentLuaScript::StaticOnValueChanged );
        }
    }
}

void ComponentLuaScript::OnDrop()
{
    ComponentUpdateable::OnDrop();

    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        assert( pFile );

        if( strcmp( pFile->m_ExtensionWithDot, ".lua" ) == 0 )
        {
            SetScriptFile( pFile );
        }
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        GameObject* pGameObject = (GameObject*)g_DragAndDropStruct.m_Value;
        assert( pGameObject );

        // warning: the 1 is the number variables in the watch panel before our externs
        int indexoffirstexternvariableinwatch = 1;
        int id = g_DragAndDropStruct.m_ID - indexoffirstexternvariableinwatch;
        
        // TODO: this will make a mess of memory if different types of objects can be dragged in...
        m_ExposedVars[id]->pointer = pGameObject;

        // update the panel so new gameobject name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pGameObject->GetName();
    }
}

void ComponentLuaScript::OnValueChanged(int id, bool finishedchanging)
{
    ProgramVariables( m_pLuaState, true );
}
#endif //MYFW_USING_WX

cJSON* ComponentLuaScript::ExportAsJSONObject()
{
    cJSON* component = ComponentUpdateable::ExportAsJSONObject();

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
            cJSON_AddStringToObject( jsonvar, "Value", ((GameObject*)pVar->pointer)->GetName() );
        }
    }

    return component;
}

void ComponentLuaScript::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentUpdateable::ImportFromJSONObject( jsonobj, sceneid );

    cJSON* scriptstringobj = cJSON_GetObjectItem( jsonobj, "Script" );
    if( scriptstringobj )
    {
        MyFileObject* pFile = g_pFileManager->FindFileByName( scriptstringobj->valuestring );
        assert( pFile );
        SetScriptFile( pFile );
    }

    // load the array of exposed variables
    cJSON* exposedvararray = cJSON_GetObjectItem( jsonobj, "ExposedVars" );
    int numvars = cJSON_GetArraySize( exposedvararray );
    for( int i=0; i<numvars; i++ )
    {
        cJSON* jsonvar = cJSON_GetArrayItem( exposedvararray, i );

        cJSON* obj = cJSON_GetObjectItem( jsonvar, "Name" );
        assert( obj );
        if( obj == 0 )
            continue;

        // by name, check if the variable is already in our list
        ExposedVariableDesc* pVar = 0;
        for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
        {
            assert( m_ExposedVars[i] );
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
                pVar->pointer = g_pComponentSystemManager->FindGameObjectByName( obj->valuestring );
        }
    }
}

ComponentLuaScript& ComponentLuaScript::operator=(const ComponentLuaScript& other)
{
    assert( &other != this );

    ComponentUpdateable::operator=( other );

    //m_Mass = other.m_Mass;
    this->m_pScriptFile = other.m_pScriptFile;
    if( this->m_pScriptFile )
        this->m_pScriptFile->AddRef();

    return *this;
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
    if( m_pScriptFile->m_FileReady )
    {
        // load the string from the file
        int loadretcode = luaL_loadstring( m_pLuaState, m_pScriptFile->m_pBuffer );
        if( loadretcode == LUA_OK )
        {
            // run the code to do initial parsing
            int exeretcode = lua_pcall( m_pLuaState, 0, LUA_MULTRET, 0 );
            if( exeretcode == LUA_OK )
            {
                luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );

                if( LuaObject.isTable() )
                {
                    // Create a table to store local variables unique to this component.
                    luabridge::LuaRef datatable = luabridge::newTable( m_pLuaState );
                    
                    char gameobjectname[100];
                    sprintf_s( gameobjectname, 100, "_GameObject_%d", m_pGameObject->GetID() );
                    luabridge::setGlobal( m_pLuaState, datatable, gameobjectname );

                    datatable["gameobject"] = m_pGameObject;

                    ParseExterns( LuaObject );
                    OnLoad();
                }
            }
            else
            {
                if( loadretcode == LUA_ERRRUN )
                {
                    LOGInfo( LOGTag, "Lua Run error in script: %s\n", m_pScriptFile->m_FilenameWithoutExtension );
                }
                else
                {
                    assert( false ); // assert until I hit this and deal with it better.
                    LOGInfo( LOGTag, "Lua Run error in script: %s\n", m_pScriptFile->m_FilenameWithoutExtension );
                }
            }
        }
        else
        {
            if( loadretcode == LUA_ERRSYNTAX )
                LOGInfo( LOGTag, "Lua Syntax error in script: %s\n", m_pScriptFile->m_FilenameWithoutExtension );
            else
            {
                assert( false );
                LOGInfo( LOGTag, "Lua Syntax error in script: %s\n", m_pScriptFile->m_FilenameWithoutExtension );
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

            luabridge::LuaRef variablename = variabledesc[1];
            luabridge::LuaRef variabletype = variabledesc[2];
            luabridge::LuaRef variableinitialvalue = variabledesc[3];

            std::string varname = variablename.tostring();

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
            std::string vartype = variabletype.tostring();

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
                    pVar->pointer = g_pComponentSystemManager->FindGameObjectByName( variableinitialvalue.tostring().c_str() );

                pVar->type = ExposedVariableType_GameObject;
            }
            else
            {
                assert( false );
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
    luabridge::LuaRef datatable = luabridge::getGlobal( m_pLuaState, gameobjectname );
    luabridge::setGlobal( m_pLuaState, datatable, "this" );

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

void ComponentLuaScript::OnLoad()
{
    //if( m_Playing )
    {
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        
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
                LuaBridgeExt_LogExceptionFormattedForVisualStudioOutputWindow( "OnLoad", m_pScriptFile->m_FullPath, e.what() );
            }
        }
    }
}

void ComponentLuaScript::OnPlay()
{
    ComponentUpdateable::OnPlay();

#if _DEBUG && MYFW_USING_WX
    // reload the script when play is hit in debug, for quick script edits.
    if( m_pScriptFile && m_pScriptFile->m_FileReady == true )
    {
        m_ScriptLoaded = false;
        g_pFileManager->ReloadFile( m_pScriptFile );
        // hard loop until the filemanager is done loading the file.
        while( m_pScriptFile && m_pScriptFile->m_FileReady == false )
        {
            g_pFileManager->Tick();
        }
        LoadScript();
    }
#endif

    // TODO: prevent "play" if file is not loaded.
    //       or test this a bit, Tick will call OnPlay once it's loaded.
    if( m_pScriptFile )
    {
        if( m_pScriptFile->m_FileReady == false )
        {
            m_ShouldBePlayingButIsntBecauseScriptFileWasStillLoading = true;
            LOGInfo( LOGTag, "Script warning: OnPlay, script not loaded: %s\n", m_pScriptFile->m_FilenameWithoutExtension );
        }
        else
        {
            // find the OnPlay function and call it, look for a table that matched our filename.
            luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        
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
                        LuaBridgeExt_LogExceptionFormattedForVisualStudioOutputWindow( "OnPlay", m_pScriptFile->m_FullPath, e.what() );
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

    if( m_Playing )
    {
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        
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
                LuaBridgeExt_LogExceptionFormattedForVisualStudioOutputWindow( "OnStop", m_pScriptFile->m_FullPath, e.what() );
            }
        }
    }

    m_Playing = false;
}

void ComponentLuaScript::Tick(double TimePassed)
{
    //ComponentUpdateable::Tick( TimePassed );

    if( m_ScriptLoaded == false )
    {
        LoadScript();

        if( m_ShouldBePlayingButIsntBecauseScriptFileWasStillLoading )
        {
            m_ShouldBePlayingButIsntBecauseScriptFileWasStillLoading = false;
            OnPlay();
        }
    }

    // find the Tick function and call it.
    if( m_Playing )
    {
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );

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
                LuaBridgeExt_LogExceptionFormattedForVisualStudioOutputWindow( "Tick", m_pScriptFile->m_FullPath, e.what() );
            }
        }
    }
}

bool ComponentLuaScript::OnTouch(int action, int id, float x, float y, float pressure, float size)
{
    // find the OnTouch function and call it.
    if( m_Playing )
    {
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );

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
                LuaBridgeExt_LogExceptionFormattedForVisualStudioOutputWindow( "OnTouch", m_pScriptFile->m_FullPath, e.what() );
            }
        }
    }

    return false;
}

bool ComponentLuaScript::OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    // find the OnButtons function and call it.
    if( m_Playing )
    {
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );

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
                LuaBridgeExt_LogExceptionFormattedForVisualStudioOutputWindow( "OnButtons", m_pScriptFile->m_FullPath, e.what() );
            }
        }
    }

    return false;
}
