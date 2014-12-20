//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

LuaGameState* g_pLuaGameState = 0;

void LUA_LogInfo(const char* str)
{
    LOGInfo( LOGTag, str );
}

LuaGameState::LuaGameState()
{
    g_pLuaGameState = this;

    m_pLuaState = luaL_newstate();
    luaL_openlibs( m_pLuaState );

    RegisterClasses();
}

LuaGameState::~LuaGameState()
{
    if( g_pLuaGameState == this )
        g_pLuaGameState = 0;

    lua_close( m_pLuaState );
}

void LuaGameState::RegisterClasses()
{
    // register a loginfo function.
    luabridge::getGlobalNamespace( m_pLuaState ).addFunction( "LogInfo", LUA_LogInfo );

    // register Framework classes.
    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<Vector3>( "Vector3" )
            .addData( "x", &Vector3::x )
            .addData( "y", &Vector3::y )
            .addData( "z", &Vector3::z )
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

    // register global managers
    luabridge::setGlobal( m_pLuaState, g_pComponentSystemManager, "g_pComponentSystemManager" );
    luabridge::setGlobal( m_pLuaState, g_pFileManager, "g_pFileManager" );
}
