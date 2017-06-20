//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_LUA

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

    LOGError( LOGTag, "%s(%d): %s: %s - %s\n", fullpath, linenumber, userdata, &what[errori], what );
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

    // Register some GL functions.
    LuaRegisterGLFunctions( m_pLuaState );

    // register Framework classes.
    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<MyMatrix>( "MyMatrix" )
            .addConstructor<void(*) ()>()
            .addData( "m11", &MyMatrix::m11 )
            .addData( "m12", &MyMatrix::m12 )
            .addData( "m13", &MyMatrix::m13 )
            .addData( "m14", &MyMatrix::m24 )
            .addData( "m21", &MyMatrix::m21 )
            .addData( "m22", &MyMatrix::m22 )
            .addData( "m23", &MyMatrix::m23 )
            .addData( "m24", &MyMatrix::m34 )
            .addData( "m31", &MyMatrix::m31 )
            .addData( "m32", &MyMatrix::m32 )
            .addData( "m33", &MyMatrix::m33 )
            .addData( "m34", &MyMatrix::m44 )
            .addData( "m41", &MyMatrix::m41 )
            .addData( "m42", &MyMatrix::m42 )
            .addData( "m43", &MyMatrix::m43 )
            .addData( "m44", &MyMatrix::m44 )
            .addFunction( "CreateLookAtView", &MyMatrix::CreateLookAtView )
            .addFunction( "CreateLookAtWorld", &MyMatrix::CreateLookAtWorld )
            .addFunction( "Scale", (void (MyMatrix::*)(float angle)) &MyMatrix::Scale )
            .addFunction( "Rotate", (void (MyMatrix::*)(float angle, float x, float y, float z)) &MyMatrix::Rotate )
            .addFunction( "Translate", (void (MyMatrix::*)(Vector3 pos)) &MyMatrix::Translate )
            .addFunction( "SetIdentity", &MyMatrix::SetIdentity )
            .addFunction( "CreateSRT", (void (MyMatrix::*)(Vector3 scale, Vector3 rot, Vector3 pos)) &MyMatrix::CreateSRT )
            .addFunction( "Multiply", (MyMatrix (MyMatrix::*)(const MyMatrix o) const) &MyMatrix::operator* )
            .addFunction( "CopyFrom", (MyMatrix& (MyMatrix::*)(const MyMatrix& o)) &MyMatrix::operator= )            
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<Vector4>( "Vector4" )
            .addConstructor<void(*) (float x, float y, float z, float w)>()
            .addData( "x", &Vector4::x )
            .addData( "y", &Vector4::y )
            .addData( "z", &Vector4::z )
            .addData( "w", &Vector4::w )
            .addFunction( "Set", &Vector4::Set )
            .addFunction( "Add", &Vector4::Add )
            .addFunction( "Sub", &Vector4::Sub )
            .addFunction( "Scale", &Vector4::Scale )
            .addFunction( "Dot", &Vector4::Dot )
            //.addFunction( "Cross", &Vector4::Cross )
            .addFunction( "Length", &Vector4::Length )
            .addFunction( "LengthSquared", &Vector4::LengthSquared )
            .addFunction( "Normalize", &Vector4::Normalize )
        .endClass();

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

    //luabridge::getGlobalNamespace( m_pLuaState )
    //    .beginClass<MySprite>( "MySprite" )
    //        .addFunction( "SetZRotation", &MySprite::SetZRotation )
    //    .endClass();    

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<MyFileObject>( "MyFileObject" )
        .endClass();    

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<FileManager>( "FileManager" )
            .addFunction( "RequestFile", &FileManager::RequestFile )
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<SoundManager>( "SoundManager" )
            .addFunction( "PlayCueByName", &SoundManager::PlayCueByName )
        .endClass();

    // Have some entity/component classes register themselves. // ADDING_NEW_ComponentType
    EngineCore::LuaRegister( m_pLuaState );
    GameObject::LuaRegister( m_pLuaState );
    ComponentBase::LuaRegister( m_pLuaState );
    ComponentTransform::LuaRegister( m_pLuaState );
    ComponentGameObjectProperties::LuaRegister( m_pLuaState );
    ComponentSystemManager::LuaRegister( m_pLuaState );

    //ComponentType_Camera,
    ComponentSprite::LuaRegister( m_pLuaState );
    ComponentMesh::LuaRegister( m_pLuaState );
    //ComponentType_MeshOBJ,
    //ComponentType_MeshPrimitive,
    ComponentVoxelMesh::LuaRegister( m_pLuaState );
    ComponentVoxelWorld::LuaRegister( m_pLuaState );
    //ComponentType_Light,
    //ComponentType_CameraShadow,
    //ComponentType_PostEffect,
    ComponentCollisionObject::LuaRegister( m_pLuaState );
    Component2DCollisionObject::LuaRegister( m_pLuaState );
    //ComponentType_2DJointRevolute,
    //ComponentType_2DJointPrismatic,
    //ComponentType_2DJointWeld,
    //ComponentType_LuaScript,
    ComponentParticleEmitter::LuaRegister( m_pLuaState );
    ComponentAnimationPlayer::LuaRegister( m_pLuaState );
    ComponentAnimationPlayer2D::LuaRegister( m_pLuaState );
    ComponentAudioPlayer::LuaRegister( m_pLuaState );
    ComponentMenuPage::LuaRegister( m_pLuaState );

    // Register the MenuItem types.
    MenuItem::LuaRegister( m_pLuaState );
    MenuButton::LuaRegister( m_pLuaState );
    MenuSprite::LuaRegister( m_pLuaState );
    MenuText::LuaRegister( m_pLuaState );

    // register global managers
    luabridge::setGlobal( m_pLuaState, g_pEngineCore, "g_pEngineCore" );
    luabridge::setGlobal( m_pLuaState, g_pComponentSystemManager, "g_pComponentSystemManager" );
    luabridge::setGlobal( m_pLuaState, g_pFileManager, "g_pFileManager" );
    luabridge::setGlobal( m_pLuaState, g_pGameCore->m_pSoundManager, "g_pSoundManager" );
}

#endif //MYFW_USING_LUA
