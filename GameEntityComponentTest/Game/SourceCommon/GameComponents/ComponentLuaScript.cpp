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
    m_ScriptName[0] = 0;
}

ComponentLuaScript::ComponentLuaScript(GameObject* owner)
: ComponentUpdateable( owner )
{
    m_BaseType = BaseComponentType_Updateable;
    m_Type = ComponentType_LuaScript;

    m_pScriptFile = 0;
    m_ScriptName[0] = 0;
}

ComponentLuaScript::~ComponentLuaScript()
{
}

void ComponentLuaScript::Reset()
{
    ComponentUpdateable::Reset();

    m_pLuaState = g_pLuaGameState->m_pLuaState;

    m_Playing = false;

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

    //g_pPanelWatch->AddFloat( "Mass", &m_Mass, 0, 100 );

    const char* desc = "no script";
    if( m_pScriptFile )
        desc = m_pScriptFile->m_FullPath;
    g_pPanelWatch->AddPointerWithDescription( "Script", 0, desc, this, ComponentLuaScript::StaticOnDrop );
}

void ComponentLuaScript::OnDrop()
{
    ComponentUpdateable::OnDrop();

    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        assert( pFile );

        int len = strlen( pFile->m_FullPath );
        const char* filenameext = &pFile->m_FullPath[len-4];

        if( strcmp( filenameext, ".lua" ) == 0 )
        {
            m_pScriptFile = pFile;
        }
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentLuaScript::ExportAsJSONObject()
{
    cJSON* component = ComponentUpdateable::ExportAsJSONObject();

    //cJSON_AddNumberToObject( component, "Mass", m_Mass );
    if( m_pScriptFile )
        cJSON_AddStringToObject( component, "Script", m_pScriptFile->m_FullPath );

    return component;
}

void ComponentLuaScript::ImportFromJSONObject(cJSON* jsonobj)
{
    ComponentUpdateable::ImportFromJSONObject( jsonobj );

    //cJSONExt_GetFloat( jsonobj, "Mass", &m_Mass );
    cJSON* scriptstringobj = cJSON_GetObjectItem( jsonobj, "Script" );
    if( scriptstringobj )
    {
        m_pScriptFile = g_pFileManager->FindFileByName( scriptstringobj->valuestring );
        assert( m_pScriptFile );
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

void ComponentLuaScript::LoadScript()
{
#if _DEBUG
    m_pScriptFile->m_FileReady = false; // should force a reload, todo: setup a proper way to do this.
#endif
    while( m_pScriptFile && m_pScriptFile->m_FileReady == false )
    {
        m_pScriptFile->Tick();
    }

    // script is ready, so run it.
    if( m_pScriptFile && m_pScriptFile->m_FileReady )
    {
        m_ScriptName[0] = 0;

        luabridge::setGlobal( m_pLuaState, m_pGameObject, "this" );

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
    }
}

void ComponentLuaScript::ParseExterns(luabridge::LuaRef LuaObject)
{
    luabridge::LuaRef Externs = LuaObject["Externs"];
    if( Externs.isTable() == true )
    {
        int len = Externs.length();
        for( int i=0; i<len; i++ )
        {
            luabridge::LuaRef variablewanted = Externs[i+1];

            luabridge::LuaRef variablename = variablewanted[1];
            luabridge::LuaRef variabletype = variablewanted[2];

            if( variabletype.tostring() == "Int" )
                LuaObject[variablename.tostring()] = 400;
        }
    }
}

void ComponentLuaScript::OnPlay()
{
    ComponentUpdateable::OnPlay();

    LoadScript();

    // TODO: prevent "play" if file is not loaded.
    if( m_pScriptFile && m_pScriptFile->m_FileReady )
    {
        // find the OnPlay function and call it, look for a table that matched our filename.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        if( LuaObject.isNil() == false )
        {
            if( LuaObject["OnPlay"].isFunction() )
            {
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
        if( LuaObject["OnStop"].isFunction() )
        {
            LuaObject["OnStop"]();
        }
    }

    m_Playing = false;
}

void ComponentLuaScript::Tick(double TimePassed)
{
    //ComponentUpdateable::Tick( TimePassed );

    // find the Tick function and call it.
    if( m_Playing )
    {
        luabridge::setGlobal( m_pLuaState, m_pGameObject, "this" );

        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        if( LuaObject["Tick"].isFunction() )
        {
            LuaObject["Tick"]( TimePassed );
        }
    }
}
