//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
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

ComponentLuaScript::ComponentLuaScript(GameObject* owner)
: ComponentUpdateable( owner )
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
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentLuaScript::StaticFillPropertiesWindow, ComponentBase::StaticOnRightClick, gameobjectid, "Lua script" );
}

void ComponentLuaScript::FillPropertiesWindow(bool clear)
{
    if( clear )
        g_pPanelWatch->ClearAllVariables();

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
            g_pPanelWatch->AddDouble( pVar->name.c_str(), &pVar->value, 0, 500 );
        }
        else if( pVar->type == ExposedVariableType_GameObject )
        {
            // TODO: support drag and drop of game objects.
            char* desc = "no gameobject";
            if( pVar->pointer )
                desc = ((GameObject*)pVar->pointer)->m_Name;
            g_pPanelWatch->AddPointerWithDescription( pVar->name.c_str(), pVar->pointer, desc, this, ComponentLuaScript::StaticOnDrop );
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
        
        m_ExposedVars[id]->pointer = pGameObject;

        // update the panel so new gameobject name shows up.
        // TODO: need a cleaner way to do this.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pGameObject->m_Name;
    }
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
            cJSON_AddStringToObject( jsonvar, "Value", ((GameObject*)pVar->pointer)->m_Name );
        }
    }

    return component;
}

void ComponentLuaScript::ImportFromJSONObject(cJSON* jsonobj)
{
    ComponentUpdateable::ImportFromJSONObject( jsonobj );

    cJSON* scriptstringobj = cJSON_GetObjectItem( jsonobj, "Script" );
    if( scriptstringobj )
    {
        MyFileObject* pFile = g_pFileManager->FindFileByName( scriptstringobj->valuestring );
        assert( pFile );
        SetScriptFile( pFile );
    }

    // load the array of exposed variables
    cJSON* exposedvararray = cJSON_GetObjectItem( jsonobj, "ExposedVars" );
    for( int i=0; i<cJSON_GetArraySize( exposedvararray ); i++ )
    {
        cJSON* jsonvar = cJSON_GetArrayItem( exposedvararray, i );

        ExposedVariableDesc* pVar = MyNew ExposedVariableDesc();

        cJSON* obj = cJSON_GetObjectItem( jsonvar, "Name" );
        if( obj )
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

        m_ExposedVars.Add( pVar );
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
    SAFE_RELEASE( m_pScriptFile );
    m_pScriptFile = script;

    if( script )
        m_pScriptFile->AddRef();
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
            lua_pcall( m_pLuaState, 0, LUA_MULTRET, 0 );

            luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );

            ParseExterns( LuaObject );
        }
        else
        {
            if( loadretcode == LUA_ERRSYNTAX )
                LOGInfo( LOGTag, "Lua Syntax error in script: %s\n", m_pScriptFile->m_FilenameWithoutExtension );
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

            if( variabletype.tostring() == "Float" )
            {
                // if it's a new variable or it changed type, set it to it's initial value.
                if( pVar->inuse == false || pVar->type != ExposedVariableType_GameObject )
                    pVar->value = variableinitialvalue.cast<double>();

                pVar->type = ExposedVariableType_Float;
            }

            if( variabletype.tostring() == "GameObject" )
            {
                // if it's a new variable or it changed type, set it to it's initial value.
                if( pVar->inuse == false || pVar->type != ExposedVariableType_GameObject )
                    pVar->pointer = g_pComponentSystemManager->FindGameObjectByName( variableinitialvalue.tostring().c_str() );

                pVar->type = ExposedVariableType_GameObject;
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

void ComponentLuaScript::ProgramVariables(luabridge::LuaRef LuaObject)
{
    // set "this" to the gameobject holding this component.
    luabridge::setGlobal( m_pLuaState, m_pGameObject, "this" );

    // program the exposed vars
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];

        if( pVar->type == ExposedVariableType_Float )
            LuaObject[pVar->name] = pVar->value;

        if( pVar->type == ExposedVariableType_GameObject )
            LuaObject[pVar->name] = (GameObject*)pVar->pointer;
    }
}

void ComponentLuaScript::OnPlay()
{
    ComponentUpdateable::OnPlay();

#if _DEBUG
    // reload the script when play is hit in debug, for quick script edits.
    if( m_pScriptFile->m_FileReady == true )
    {
        m_ScriptLoaded = false;
        m_pScriptFile->m_FileReady = false; // should force a reload, TODO: setup a proper way to do this.
        while( m_pScriptFile && m_pScriptFile->m_FileReady == false )
        {
            m_pScriptFile->Tick();
        }
    }
    LoadScript();
#endif

    // TODO: prevent "play" if file is not loaded.
    if( m_pScriptFile && m_pScriptFile->m_FileReady )
    {
        // find the OnPlay function and call it, look for a table that matched our filename.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        
        if( LuaObject.isNil() == false )
        {
            if( LuaObject["OnPlay"].isFunction() )
            {
                ProgramVariables( LuaObject );
                LuaObject["OnPlay"]();
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
            ProgramVariables( LuaObject );
            LuaObject["OnStop"]();
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
    }

    // find the Tick function and call it.
    if( m_Playing )
    {
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );

        // call tick
        if( LuaObject["Tick"].isFunction() )
        {
            ProgramVariables( LuaObject );
            try
            {
                LuaObject["Tick"]( TimePassed );
            }
            catch( ... )
            {
                LOGError( LOGTag, "Script error: Tick: %s", m_pScriptFile->m_FilenameWithoutExtension );
            }
        }
    }
}
