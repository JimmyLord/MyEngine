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

#define LUA_DEBUG_PORT 19542

LuaGameState* g_pLuaGameState = 0;

// Exposed to Lua, change elsewhere if function signature changes.
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
    m_ListenSocket = 0;
    m_DebugSocket = 0;

    // don't build on init, Rebuild is called after a scene is loaded.
    //Rebuild();
}

LuaGameState::~LuaGameState()
{
    if( g_pLuaGameState == this )
        g_pLuaGameState = 0;

    if( m_pLuaState )
        lua_close( m_pLuaState );

    closesocket( m_ListenSocket );
    closesocket( m_DebugSocket );
}

void SetSocketBlockingState(int socket, bool block)
{
    if( block )
    {
        // Set socket as blocking.
        DWORD value = 0;
        ioctlsocket( socket, FIONBIO, &value );
        //const int flags = fcntl( m_ListenSocket, F_GETFL, 0 );
        //fcntl( m_DebugSocket, F_SETFL, flags & ~O_NONBLOCK );
    }
    else
    {
        // Set socket as non-blocking.
        DWORD value = 1;
        ioctlsocket( socket, FIONBIO, &value );
        //const int flags = fcntl( m_ListenSocket, F_GETFL, 0 );
        //fcntl( m_DebugSocket, F_SETFL, flags & O_NONBLOCK );
    }
}

void DebugHookFunction(lua_State* luastate, lua_Debug* ar)
{
    if( g_pLuaGameState->m_DebugSocket <= 0 )
        return;

    if( ar->event == LUA_HOOKLINE )
    {
        g_pLuaGameState->SendStoppedMessage();

        // Block and wait for debugger messages.
        g_pLuaGameState->CheckForDebugNetworkMessages( true );
    }
}

void LuaGameState::Tick()
{
    CheckForDebugNetworkMessages( false );
}

void LuaGameState::CheckForDebugNetworkMessages(bool block)
{
    if( m_ListenSocket <= 0 )
        return;

    char buffer[1000];
    int buffersize = 1000;

    // Check for incoming connection requests. (Only allow 1 debugger)
    if( m_ListenSocket != 0 && m_DebugSocket == 0 )
    {
        sockaddr_in saddr;
        int fromLength = sizeof( sockaddr_in );
        
        int socket = accept( m_ListenSocket, (sockaddr*)&saddr, &fromLength );
        if( socket != -1 )
        {
            LOGInfo( "LuaDebug", "Received incoming connection (socket:%d) (port:%d)\n", socket, saddr.sin_port );

            // Set the socket for debug communication and set it to non-blocking.
            m_DebugSocket = socket;
            SetSocketBlockingState( m_DebugSocket, false );

            //// For now, immediately break into the current lua script.
            //lua_sethook( m_pLuaState, DebugHookFunction, LUA_MASKLINE, 0 );
        }
    }

    // Kick out if we don't have a debug connection.
    if( m_DebugSocket <= 0 )
        return;

    int bytes = -2;
    while( bytes != -1 )
    {
        if( block )
        {
            SetSocketBlockingState( m_DebugSocket, true );

            //while( 1 )
            {
                //Sleep( 100 );
                ////usleep( 5000 );
                
                bytes = (int)recv( m_DebugSocket, buffer, 1000, 0 );

                //if( bytes != -1 )
                //{
                //    break;
                //}
            }
        }
        else
        {
            SetSocketBlockingState( m_DebugSocket, false );

            bytes = (int)recv( m_DebugSocket, buffer, 1000, 0 );
        }

        // If we were blocking and there was an error, or the connection was closed nicely,
        //    set the socket to 0 and remove the lua hook.
        if( (block && bytes == -1) || bytes == 0 )
        {
            LOGInfo( "LuaDebug", "m_DebugSocket was closed\n" );

            lua_sethook( m_pLuaState, DebugHookFunction, 0, 0 );
            m_DebugSocket = 0;
        }

        if( bytes != -1 )
        {
            // Received a packet.
            if( bytes < 1000 )
                buffer[bytes] = 0;
            else
                buffer[999] = 0;

            LOGInfo( "LuaDebug", "Received a packet (size:%d) (value:%s)\n", bytes, buffer );
            block = DealWithDebugNetworkMessages( buffer );
        }
    }
}

void LuaGameState::SendStoppedMessage()
{
    // Send a 'Stopped' message to the debugger.
    cJSON* jMessageOut = cJSON_CreateObject();
    cJSON_AddStringToObject( jMessageOut, "Type", "Stopped" );
    int numstackframes = AddStackToJSONMessage( jMessageOut );

    char* jsonstr = cJSON_PrintUnformatted( jMessageOut );
    send( m_DebugSocket, jsonstr, strlen(jsonstr), 0 );
    
    LOGInfo( "LuaDebug", "Sending 'Stopped' (numframes:%d)\n", numstackframes );
    
    cJSON_Delete( jMessageOut );
    cJSONExt_free( jsonstr );
}

// Returns how many stack frames sent.
int LuaGameState::AddStackToJSONMessage(cJSON* jMessage)
{
    int ret = 0;

    lua_Debug ar;
    ret = lua_getstack( m_pLuaState, 0, &ar );

    if( ret == 0 )
    {
        cJSON_AddNumberToObject( jMessage, "StackNumLevels", 0 );
    }
    else if( ret == 1 )
    {
        lua_getinfo( m_pLuaState, "Sl", &ar );

        char fullpath[MAX_PATH];
        fullpath[0] = 0;

        if( ar.source[0] == '@' )
        {
            GetFullPath( &ar.source[1], fullpath, MAX_PATH );                
        }

        cJSON_AddNumberToObject( jMessage, "StackNumLevels", 1 );
        cJSON_AddStringToObject( jMessage, "FunctionName", "functionName" );
        cJSON_AddStringToObject( jMessage, "Source", fullpath );
        cJSON_AddNumberToObject( jMessage, "Line", ar.currentline );
    }

    return ret;
}

// Returns true if we should continue blocking, false otherwise.
bool LuaGameState::DealWithDebugNetworkMessages(char* message)
{
    if( strcmp( message, "continue" ) == 0 )
    {
        // Remove the lua hook for now on continue command.
        // TODO: keep the hook and continue to check for breakpoints.
        lua_sethook( m_pLuaState, DebugHookFunction, 0, 0 );
        return false;
    }
    if( strcmp( message, "step" ) == 0 ) // also used by pause
    {
        // Set the lua hook (might already be set).
        lua_sethook( m_pLuaState, DebugHookFunction, LUA_MASKLINE, 0 );

        // TODO: Only send this if Lua isn't currently running to let debugger know we've paused execution.
        //   'Stopped' will be sent twice when Lua script isn't currently running. (Once here, once in debug hook)
        //   This stack check will prevent 'Stopped' from being sent twice if we're already stepping through Lua code.
        lua_Debug ar;
        if( lua_getstack( m_pLuaState, 0, &ar ) == 0 )
            SendStoppedMessage();

        return false;
    }
    //if( message[0] == '{' )
    //{
    //    cJSON* jMessageIn = cJSON_Parse( message );
    //    cJSON* jStackStart = cJSON_GetObjectItem( jMessageIn, "stackstart" );
    //    cJSON* jStackEnd = cJSON_GetObjectItem( jMessageIn, "stackend" );

    //    if( jStackStart && jStackEnd )
    //    {
    //        int stackstart = jStackStart->valueint;
    //        int stackend = jStackEnd->valueint;

    //        cJSON_Delete( jMessageIn );

    //        // Create a StackInfo message to reply to the stackrequest.
    //        cJSON* jMessageOut = cJSON_CreateObject();
    //        cJSON_AddStringToObject( jMessageOut, "Type", "StackInfo" );
    //        int numstackframes = AddStackToJSONMessage( jMessageOut );
    //        char* jsonstr = cJSON_PrintUnformatted( jMessageOut );
    //        send( g_pLuaGameState->m_DebugSocket, jsonstr, strlen(jsonstr), 0 );

    //        LOGInfo( "LuaDebug", "Sending stack info (numframes:%d)\n", numstackframes );

    //        cJSON_Delete( jMessageOut );
    //        cJSONExt_free( jsonstr );

    //        // If lua isn't running, stop blocking.
    //        if( numstackframes == 0 )
    //            return false;
    //    }
    //}

    return true;
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

    if( m_ListenSocket == 0 )
    {        
        // Create a socket for debugging.
        m_ListenSocket = (int)socket( AF_INET, SOCK_STREAM, 0 );//IPPROTO_TCP );

        // Bind our socket to a local addr/port.
        sockaddr_in localaddr;
        localaddr.sin_family = AF_INET;
        localaddr.sin_addr.s_addr = INADDR_ANY;
        localaddr.sin_port = htons( LUA_DEBUG_PORT );

        bind( m_ListenSocket, (const sockaddr*)&localaddr, sizeof(sockaddr_in) );

        SetSocketBlockingState( m_ListenSocket, false );
        listen( m_ListenSocket, 10 );

        LOGInfo( "LuaDebug", "Lua debug listen socket created (sock:%d) (port:%d)\n", m_ListenSocket, LUA_DEBUG_PORT );
    }
}

void LuaGameState::RegisterClasses()
{
    // Register a loginfo function.
    luabridge::getGlobalNamespace( m_pLuaState ).addFunction( "LogInfo", LUA_LogInfo ); // void LUA_LogInfo(const char* str)
    luabridge::getGlobalNamespace( m_pLuaState ).addFunction( "GetSystemTime", MyTime_GetSystemTime ); // double MyTime_GetSystemTime(bool realtime)
    luabridge::getGlobalNamespace( m_pLuaState ).addFunction( "GetRunningTime", MyTime_GetRunningTime ); // double MyTime_GetRunningTime()
    luabridge::getGlobalNamespace( m_pLuaState ).addFunction( "GetUnpausedTime", MyTime_GetUnpausedTime ); // double MyTime_GetUnpausedTime()

    // Register some GL functions.
    LuaRegisterGLFunctions( m_pLuaState );

    // register Framework classes.
    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<MyMatrix>( "MyMatrix" )
            .addConstructor<void(*) ()>() // MyMatrix()
            .addData( "m11", &MyMatrix::m11 ) // float
            .addData( "m12", &MyMatrix::m12 ) // float
            .addData( "m13", &MyMatrix::m13 ) // float
            .addData( "m14", &MyMatrix::m24 ) // float
            .addData( "m21", &MyMatrix::m21 ) // float
            .addData( "m22", &MyMatrix::m22 ) // float
            .addData( "m23", &MyMatrix::m23 ) // float
            .addData( "m24", &MyMatrix::m34 ) // float
            .addData( "m31", &MyMatrix::m31 ) // float
            .addData( "m32", &MyMatrix::m32 ) // float
            .addData( "m33", &MyMatrix::m33 ) // float
            .addData( "m34", &MyMatrix::m44 ) // float
            .addData( "m41", &MyMatrix::m41 ) // float
            .addData( "m42", &MyMatrix::m42 ) // float
            .addData( "m43", &MyMatrix::m43 ) // float
            .addData( "m44", &MyMatrix::m44 ) // float
            .addFunction( "CreateLookAtView", &MyMatrix::CreateLookAtView ) // void MyMatrix::CreateLookAtView(const Vector3 &eye, const Vector3 &up, const Vector3 &at)
            .addFunction( "CreateLookAtWorld", &MyMatrix::CreateLookAtWorld ) // void MyMatrix::CreateLookAtWorld(const Vector3& objpos, const Vector3& up, const Vector3& at)
            .addFunction( "Scale", (void (MyMatrix::*)(float scale)) &MyMatrix::Scale ) // void MyMatrix::Scale(float scale);
            .addFunction( "Rotate", (void (MyMatrix::*)(float angle, float x, float y, float z)) &MyMatrix::Rotate ) // void MyMatrix::Rotate(float angle, float x, float y, float z);
            .addFunction( "Translate", (void (MyMatrix::*)(Vector3 pos)) &MyMatrix::Translate ) // void MyMatrix::Translate(Vector3 pos);
            .addFunction( "SetIdentity", &MyMatrix::SetIdentity ) // void MyMatrix::SetIdentity();
            .addFunction( "CreateSRT", (void (MyMatrix::*)(Vector3 scale, Vector3 rot, Vector3 pos)) &MyMatrix::CreateSRT ) // void MyMatrix::CreateSRT(float scale, Vector3 rot, Vector3 pos);
            .addFunction( "Multiply", (MyMatrix (MyMatrix::*)(const MyMatrix o) const) &MyMatrix::operator* ) // MyMatrix MyMatrix::operator *(const MyMatrix o) const
            .addFunction( "CopyFrom", (MyMatrix& (MyMatrix::*)(const MyMatrix& o)) &MyMatrix::operator= ) // MyMatrix& MyMatrix::operator =(const MyMatrix& o)
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<Vector4>( "Vector4" )
            .addConstructor<void(*) (float x, float y, float z, float w)>() // Vector4(float nx, float ny, float nz, float nw)
            .addData( "x", &Vector4::x ) // float
            .addData( "y", &Vector4::y ) // float
            .addData( "z", &Vector4::z ) // float
            .addData( "w", &Vector4::w ) // float
            .addFunction( "Set", &Vector4::Set ) // void Vector4::Set(float nx, float ny, float nz, float nw)
            .addFunction( "Add", &Vector4::Add ) // Vector4 Vector4::Add(const Vector4& o) const
            .addFunction( "Sub", &Vector4::Sub ) // Vector4 Vector4::Sub(const Vector4& o) const
            .addFunction( "Scale", &Vector4::Scale ) // Vector4 Vector4::Scale(const float o) const
            .addFunction( "Dot", &Vector4::Dot ) // float Vector4::Dot(const Vector4 &o) const
            //.addFunction( "Cross", &Vector4::Cross )
            .addFunction( "Length", &Vector4::Length ) // float Vector4::Length() const
            .addFunction( "LengthSquared", &Vector4::LengthSquared ) // float Vector4::LengthSquared() const
            .addFunction( "Normalize", &Vector4::Normalize ) // Vector4 Vector4::Normalize()
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<Vector3>( "Vector3" )
            .addConstructor<void(*) (float x, float y, float z)>() // Vector3(float nx, float ny, float nz)
            .addData( "x", &Vector3::x ) // float
            .addData( "y", &Vector3::y ) // float
            .addData( "z", &Vector3::z ) // float
            .addFunction( "Set", &Vector3::Set ) // void Vector3::Set(float nx, float ny, float nz)
            .addFunction( "Add", &Vector3::Add ) // Vector3 Vector3::Add(const Vector3& o) const
            .addFunction( "Sub", &Vector3::Sub ) // Vector3 Vector3::Sub(const Vector3& o) const
            .addFunction( "Scale", &Vector3::Scale ) // Vector3 Vector3::Scale(const float o) const
            .addFunction( "Dot", &Vector3::Dot ) // float Vector3::Dot(const Vector3 &o) const
            .addFunction( "Cross", &Vector3::Cross ) // Vector3 Vector3::Cross(const Vector3& o) const
            .addFunction( "Length", &Vector3::Length ) // float Vector3::Length() const
            .addFunction( "LengthSquared", &Vector3::LengthSquared ) // float Vector3::LengthSquared() const
            .addFunction( "Normalize", &Vector3::Normalize ) // Vector3 Vector3::Normalize()
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<Vector2>( "Vector2" )
            .addConstructor<void(*) (float x, float y)>() // Vector2(float nx, float ny)
            .addData( "x", &Vector2::x ) // float
            .addData( "y", &Vector2::y ) // float
            .addFunction( "Set", &Vector2::Set ) // void Vector2::Set(float nx, float ny)
            .addFunction( "Add", &Vector2::Add ) // Vector2 Vector2::Add(const Vector2& o) const
            .addFunction( "Sub", &Vector2::Sub ) // Vector2 Vector2::Sub(const Vector2& o) const
            .addFunction( "Scale", &Vector2::Scale ) // Vector2 Vector2::Scale(const float o) const
            .addFunction( "Dot", &Vector2::Dot ) // float Vector2::Dot(const Vector2 &o) const
            .addFunction( "Length", &Vector2::Length ) // float Vector2::Length()
            .addFunction( "LengthSquared", &Vector2::LengthSquared ) // float Vector2::LengthSquared()
            .addFunction( "Normalize", &Vector2::Normalize ) // Vector2 Vector2::Normalize()
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
            .addFunction( "RequestFile", &FileManager::RequestFile ) // MyFileObject* FileManager::RequestFile(const char* filename)
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<SoundManager>( "SoundManager" )
            .addFunction( "PlayCueByName", &SoundManager::PlayCueByName ) // int SoundManager::PlayCueByName(const char* name)
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
    luabridge::setGlobal( m_pLuaState, g_pEngineCore, "g_pEngineCore" ); // EngineCore*
    luabridge::setGlobal( m_pLuaState, g_pComponentSystemManager, "g_pComponentSystemManager" ); // ComponentSystemManager*
    luabridge::setGlobal( m_pLuaState, g_pFileManager, "g_pFileManager" ); // FileManager*
    luabridge::setGlobal( m_pLuaState, g_pGameCore->GetSoundManager(), "g_pSoundManager" ); // SoundManager*
}

#endif //MYFW_USING_LUA
