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

    m_NextLineToBreakOn = -1;
    m_NextSourceFileToBreakOn[0] = 0;
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
        lua_getinfo( g_pLuaGameState->m_pLuaState, "Sl", ar ); // (S)ource and (l)ine.

        bool breakpoint = false;

        // Check for breakpoints.
        for( unsigned int i=0; i<g_pLuaGameState->m_Breakpoints.size(); i++ )
        {
            if( g_pLuaGameState->m_Breakpoints[i].line == ar->currentline &&
                strcmp( g_pLuaGameState->m_Breakpoints[i].file, &ar->source[1] ) == 0 )
            {
                breakpoint = true;
                LOGInfo( "LuaDebug", "Stopped at breakpoint\n" );
            }
        }
        
        // If this is the next line to break on, then break.
        if( breakpoint ||
            g_pLuaGameState->m_NextLineToBreakOn == -1 ||
            ( g_pLuaGameState->m_NextLineToBreakOn <= ar->currentline &&
              strcmp( g_pLuaGameState->m_NextSourceFileToBreakOn, ar->source ) == 0 ) )
        {
            g_pLuaGameState->SendStoppedMessage();

            // Block and wait for debugger messages.
            g_pLuaGameState->CheckForDebugNetworkMessages( true );
        }
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
        }
    }

    // Kick out if we don't have a debug connection.
    if( m_DebugSocket <= 0 )
        return;

    // Deal with TCP messages, separated by '\n' character.
    char buffer[1000];
    int buffersize = 1000;

    int bytes = -2;
    while( bytes != -1 )
    {
        if( block )
        {
            SetSocketBlockingState( m_DebugSocket, true );
        }
        else
        {
            SetSocketBlockingState( m_DebugSocket, false );
        }

        // Read until a '\n'.
        {
            bytes = (int)recv( m_DebugSocket, buffer, 1000, MSG_PEEK );
                
            int i;
            for( i=0; i<bytes; i++ )
            {
                if( buffer[i] == '\n' )
                    break;
            }

            (int)recv( m_DebugSocket, buffer, i+1, 0 );
            buffer[i] = 0;
        }

        // If we were blocking and there was an error, or the connection was closed nicely,
        //    set the socket to 0 and remove the lua hook.
        if( (block && bytes == -1) || bytes == 0 )
        {
            LOGInfo( "LuaDebug", "m_DebugSocket was closed\n" );

            lua_sethook( m_pLuaState, DebugHookFunction, 0, 0 );
            m_DebugSocket = 0;
            ClearAllBreakpoints();
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

// Returns true if we should continue blocking, false otherwise.
bool LuaGameState::DealWithDebugNetworkMessages(char* message)
{
    if( strcmp( message, "continue" ) == 0 )
    {
        m_NextLineToBreakOn = INT_MAX; // Continue (only stop on Breakpoints).
        //// Remove the lua hook for now on continue command.
        //// TODO: keep the hook and continue to check for breakpoints.
        //lua_sethook( m_pLuaState, DebugHookFunction, 0, 0 );
        return false;
    }
    if( strcmp( message, "stepin" ) == 0 ) // used by step-in, pause and break on entry.
    {
        // Break on whatever the next line is.
        m_NextLineToBreakOn = -1;

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
    if( strcmp( message, "stepover" ) == 0 )
    {
        // Set the lua hook (might already be set).
        lua_sethook( m_pLuaState, DebugHookFunction, LUA_MASKLINE, 0 );

        lua_Debug ar;
        lua_getstack( m_pLuaState, 0, &ar );
        lua_getinfo( g_pLuaGameState->m_pLuaState, "Sl", &ar ); // (S)ource and (l)ine.

        // Break on whatever the next line is.
        if( ar.currentline < ar.lastlinedefined )
            m_NextLineToBreakOn = ar.currentline + 1;
        else
            m_NextLineToBreakOn = -1; // If this was the last line of the function, step to next statement.
        strcpy_s( m_NextSourceFileToBreakOn, MAX_PATH, ar.source );

        return false;
    }
    if( strcmp( message, "stepout" ) == 0 )
    {
        // Set the lua hook (might already be set).
        lua_sethook( m_pLuaState, DebugHookFunction, LUA_MASKLINE, 0 );

        lua_Debug ar;
        int isThereAStackFrame1 = lua_getstack( m_pLuaState, 1, &ar );
        
        if( isThereAStackFrame1 == 0 )
        {
            // If there is no stack frame 1, 'continue'
            m_NextLineToBreakOn = INT_MAX; // Continue (only stop on Breakpoints).
        }
        else
        {
            lua_getinfo( g_pLuaGameState->m_pLuaState, "Sl", &ar ); // (S)ource and (l)ine.

            // Break on the next line of stack frame 1.
            m_NextLineToBreakOn = ar.currentline + 1;
            strcpy_s( m_NextSourceFileToBreakOn, MAX_PATH, ar.source );
        }

        return false;
    }
    if( message[0] == '{' )
    {
        cJSON* jMessage = cJSON_Parse( message );

        cJSON* jCommand = cJSON_GetObjectItem( jMessage, "command" );
        if( jCommand )
        {
            if( strcmp( jCommand->valuestring, "breakpoint_ClearAllFromFile" ) == 0 )
            {
                cJSON* jFile = cJSON_GetObjectItem( jMessage, "file" );
                ClearAllBreakpointsFromFile( jFile->valuestring );
            }
            else if( strcmp( jCommand->valuestring, "breakpoint_Set" ) == 0 )
            {
                cJSON* jFile = cJSON_GetObjectItem( jMessage, "file" );
                cJSON* jLine = cJSON_GetObjectItem( jMessage, "line" );
                AddBreakpoint( jFile->valuestring, jLine->valueint );
            }
        }

        cJSON_Delete( jMessage );
        return false;
    }

    return true;
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
    int numlevels = 0;

    cJSON* jStackArray = 0;
    
    int validstack = 1;
    while( validstack == 1 )
    {
        lua_Debug ar;
        validstack = lua_getstack( m_pLuaState, numlevels, &ar );

        if( validstack )
        {
            lua_getinfo( m_pLuaState, "nSl", &ar ); // Function (n)ame, (S)ource and (l)ine.

            char fullpath[MAX_PATH];
            fullpath[0] = 0;

            if( ar.source[0] == '@' )
            {
                GetFullPath( &ar.source[1], fullpath, MAX_PATH );                
            }

            cJSON* jStack = cJSON_CreateObject();
            if( ar.name )
            {
                cJSON_AddStringToObject( jStack, "FunctionName", ar.name );
            }
            else
            {
                // TODO: functions called directly from C++ aren't getting their 'name' field filled in.
                cJSON_AddStringToObject( jStack, "FunctionName", "?" );
            }
            cJSON_AddStringToObject( jStack, "Source", fullpath );
            cJSON_AddNumberToObject( jStack, "Line", ar.currentline );

            if( jStackArray == 0 )
                jStackArray = cJSON_CreateArray();
            cJSON_AddItemToArray( jStackArray, jStack );

            // Add Local variables to this stack frame.
            //if( numlevels == 0 )
            {
                AddLocalVarsToStackInJSONMessage( jStack, &ar );
            }

            numlevels++;
        }
    }

    cJSON_AddNumberToObject( jMessage, "StackNumLevels", numlevels );
    if( jStackArray )
        cJSON_AddItemToObject( jMessage, "StackFrames", jStackArray );

    return numlevels;
}

void LuaGameState::AddValueAtTopOfStackToJSONObject(cJSON* jObject, const char* name)
{
    cJSON_AddStringToObject( jObject, "Name", name );

    if( lua_isnumber( m_pLuaState, -1 ) )
    {
        double value = lua_tonumber( m_pLuaState, -1 );
        cJSON_AddNumberToObject( jObject, "Value", value );
    }
    else if( lua_isboolean( m_pLuaState, -1 ) )
    {
        bool value = lua_toboolean( m_pLuaState, -1 ) ? true : false;
        cJSON_AddBoolToObject( jObject, "Value", value );
    }
    else if( lua_isstring( m_pLuaState, -1 ) )
    {
        const char* value = lua_tostring( m_pLuaState, -1 );
        cJSON_AddStringToObject( jObject, "Value", value );
    }
    else if( lua_istable( m_pLuaState, -1 ) )
    {
        // TODO:
        cJSON_AddStringToObject( jObject, "Value", "<object>" );
    }
    else
    {
        // TODO:
        cJSON_AddStringToObject( jObject, "Value", "<other>" );
    }
}

int LuaGameState::AddLocalVarsToStackInJSONMessage(cJSON* jStack, lua_Debug* ar)
{
    int i = 0;
    int numlocals = 0;

    // Add all members of "this" to the message.
    cJSON* jThisesArray = 0;
    {
        lua_getglobal( m_pLuaState, "this" );

        lua_pushnil( m_pLuaState ); // Start searching from the start of the "this" table.
        while( lua_next( m_pLuaState, -2 ) != 0 ) // Pops the key from stack, pushes new key and value.
        {
            cJSON* jThis = 0;

            // -2 is key, -1 is value.
            // Only add if -2 is a string type... they should all be.
            if( lua_isstring( m_pLuaState, -2 ) )
            {
                if( jThisesArray == 0 )
                    jThisesArray = cJSON_CreateArray();

                // This will break lua_next if -2 isn't a string type.
                const char* localname = lua_tostring( m_pLuaState, -2 );

                jThis = cJSON_CreateObject();
                AddValueAtTopOfStackToJSONObject( jThis, localname );
            }

            lua_pop( m_pLuaState, 1 ); // Pop the value off the stack.

            if( jThis )
                cJSON_AddItemToArray( jThisesArray, jThis );
        }

        lua_pop( m_pLuaState, 1 ); // Pop "this" off the stack.
    }

    if( jThisesArray )
        cJSON_AddItemToObject( jStack, "This", jThisesArray );

    // Add all locals to the message.
    cJSON* jLocalsArray = 0;
    while( true )
    {
        const char* localname = lua_getlocal( m_pLuaState, ar, i+1 );

        if( localname == 0 )
            break;

        if( localname[0] != '(' )
        {
            if( jLocalsArray == 0 )
                jLocalsArray = cJSON_CreateArray();

            cJSON* jLocal = cJSON_CreateObject();

            AddValueAtTopOfStackToJSONObject( jLocal, localname );

            cJSON_AddItemToArray( jLocalsArray, jLocal );

            numlocals++;
        }

        lua_pop( m_pLuaState, 1 ); // Pop the value off the stack.
        i++;
    }

    if( jLocalsArray )
        cJSON_AddItemToObject( jStack, "Local", jLocalsArray );

    return numlocals;
}

void LuaGameState::ClearAllBreakpoints()
{
    m_Breakpoints.clear();
}

void LuaGameState::ClearAllBreakpointsFromFile(char* fullpath)
{
    const char* relativepath = GetRelativePath( fullpath );
    for( unsigned int i=0; i<m_Breakpoints.size(); i++ )
    {
        if( strcmp( m_Breakpoints[i].file, relativepath ) == 0 )
        {
            m_Breakpoints[i] = m_Breakpoints[m_Breakpoints.size()-1];
            m_Breakpoints.pop_back();
            i--;
        }
    }
}

void LuaGameState::AddBreakpoint(char* fullpath, int line)
{
    const char* relativepath = GetRelativePath( fullpath );

    Breakpoint bp;
    strcpy_s( bp.file, MAX_PATH, relativepath );
    bp.line = line;

    m_Breakpoints.push_back( bp );
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
