//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

LuaGameState* g_pLuaGameState = 0;

void LUA_LogInfo(const char* str)
{
    LOGInfo( LOGTag, str );
}

void LuaBridgeExt_LogExceptionFormattedForVisualStudioOutputWindow(const char* userdata, const char* fullpath, const char* what)
{
    // 'what' looks like this:
        //[string "-- test Player script, moves to where you cli..."]:26: no member named 'id'

    // we want this so we can double click and go to line in file:
        //1>e:\...fullpath...\gamecomponents\componentluascript.cpp(536):

    // far from robust, but find the '26' wrapped in :'s from 'what'.
    // should work unless error message involved :'s
    int len = (int)strlen( what );
    int linenumber = 0;
    int errori = 0;
    for( int i=len-1; i>0; i-- )
    {
        if( what[i] == ':' )
        {
            if( errori == 0 ) // first colon from end.
            {
                errori = i+1;
            }
            else //if( linenumber == 0 ) // second colon from end.
            {
                linenumber = atoi( &what[i+1] );
                break;
            }
        }
    }

    LOGInfo( LOGTag, "%s(%d): %s: %s - %s\n", fullpath, linenumber, userdata, &what[errori], what );
}

LuaGameState::LuaGameState()
{
    g_pLuaGameState = this;

    m_pLuaState = 0;

    // don't build on init, Rebuild is called after a scene is loaded.
    //Rebuild();
}

LuaGameState::~LuaGameState()
{
    if( g_pLuaGameState == this )
        g_pLuaGameState = 0;

    if( m_pLuaState )
        lua_close( m_pLuaState );
}

void LuaGameState::Rebuild()
{
    if( m_pLuaState != 0 )
    {
        lua_close( m_pLuaState );
    }

    m_pLuaState = luaL_newstate();
    luaL_openlibs( m_pLuaState );

    RegisterClasses();
}

void LuaGameState::RegisterClasses()
{
    // register a loginfo function.
    luabridge::getGlobalNamespace( m_pLuaState ).addFunction( "LogInfo", LUA_LogInfo );
    luabridge::getGlobalNamespace( m_pLuaState ).addFunction( "GetSystemTime", MyTime_GetSystemTime );
    luabridge::getGlobalNamespace( m_pLuaState ).addFunction( "GetRunningTime", MyTime_GetRunningTime );
    luabridge::getGlobalNamespace( m_pLuaState ).addFunction( "GetUnpausedTime", MyTime_GetUnpausedTime );

    // register Framework classes.
    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<Vector3>( "Vector3" )
            .addConstructor<void(*) (float x, float y, float z)>()
            .addData( "x", &Vector3::x )
            .addData( "y", &Vector3::y )
            .addData( "z", &Vector3::z )
            .addFunction( "Set", &Vector3::Set )
            .addFunction( "Add", &Vector3::Add )
            .addFunction( "Sub", &Vector3::Sub )
            .addFunction( "Scale", &Vector3::Scale )
            .addFunction( "Dot", &Vector3::Dot )
            .addFunction( "Cross", &Vector3::Cross )
            .addFunction( "Length", &Vector3::Length )
            .addFunction( "LengthSquared", &Vector3::LengthSquared )
            .addFunction( "Normalize", &Vector3::Normalize )
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<Vector2>( "Vector2" )
            .addConstructor<void(*) (float x, float y)>()
            .addData( "x", &Vector2::x )
            .addData( "y", &Vector2::y )
            .addFunction( "Set", &Vector2::Set )
            .addFunction( "Add", &Vector2::Add )
            .addFunction( "Sub", &Vector2::Sub )
            .addFunction( "Scale", &Vector2::Scale )
            .addFunction( "Dot", &Vector2::Dot )
            .addFunction( "Length", &Vector2::Length )
            .addFunction( "LengthSquared", &Vector2::LengthSquared )
            .addFunction( "Normalize", &Vector2::Normalize )
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<MyFileObject>( "MyFileObject" )
        .endClass();    

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<FileManager>( "FileManager" )
            .addFunction( "RequestFile", &FileManager::RequestFile )
        .endClass();

    // Have some entity/component classes register themselves.
    GameObject::LuaRegister( m_pLuaState );
    ComponentTransform::LuaRegister( m_pLuaState );
    ComponentSystemManager::LuaRegister( m_pLuaState );
    ComponentCollisionObject::LuaRegister( m_pLuaState );
    ComponentAnimationPlayer::LuaRegister( m_pLuaState );

    // register global managers
    luabridge::setGlobal( m_pLuaState, g_pComponentSystemManager, "g_pComponentSystemManager" );
    luabridge::setGlobal( m_pLuaState, g_pFileManager, "g_pFileManager" );
}
