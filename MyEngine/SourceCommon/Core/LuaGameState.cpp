//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "Core/EngineCore.h"
#include "ComponentSystem/BaseComponents/ComponentBase.h"
#include "ComponentSystem/BaseComponents/ComponentGameObjectProperties.h"
#include "ComponentSystem/BaseComponents/ComponentMenuPage.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/EngineComponents/ComponentHeightmap.h"
#if MYFW_USING_MONO
#include "ComponentSystem/EngineComponents/ComponentMonoScript.h"
#endif //MYFW_USING_MONO
#include "ComponentSystem/EngineComponents/ComponentObjectPool.h"
#include "ComponentSystem/FrameworkComponents/ComponentAnimationPlayer.h"
#include "ComponentSystem/FrameworkComponents/ComponentAnimationPlayer2D.h"
#include "ComponentSystem/FrameworkComponents/ComponentAudioPlayer.h"
#if MYFW_USING_LUA
#include "ComponentSystem/FrameworkComponents/ComponentLuaScript.h"
#endif //MYFW_USING_LUA
#include "ComponentSystem/FrameworkComponents/ComponentMesh.h"
#include "ComponentSystem/FrameworkComponents/ComponentMeshPrimitive.h"
#include "ComponentSystem/FrameworkComponents/ComponentParticleEmitter.h"
#include "ComponentSystem/FrameworkComponents/ComponentSprite.h"
#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DCollisionObject.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DCollisionObject.h"
#include "Voxels/ComponentVoxelMesh.h"
#include "Voxels/ComponentVoxelWorld.h"
#include "../../../SharedGameCode/Menus/MenuItem.h"
#include "../../../SharedGameCode/Menus/MenuButton.h"
#include "../../../SharedGameCode/Menus/MenuSprite.h"
#include "../../../SharedGameCode/Menus/MenuText.h"

#if MYFW_USING_LUA

#define LUA_DEBUG_PORT 19542

bool g_OutputLuaDebugLog = false;
LuaGameState* g_pLuaGameState = 0;

#if MYFW_WINDOWS
#define close closesocket
#endif

// Exposed to Lua, change elsewhere if function signature changes.
void LUA_LogInfo(const char* str)
{
    LOGInfo( LOGTag, "%s", str );
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

LuaGameState::LuaGameState(EngineCore* pEngineCore)
{
    g_pLuaGameState = this;

    m_pEngineCore = pEngineCore;

    m_pLuaState = 0;
#if MYFW_ENABLE_LUA_DEBUGGER
    m_ListenSocket = 0;
    m_DebugSocket = 0;

    m_NextLineToBreakOn = INT_MAX; // Only stop on breakpoints.
    m_NextSourceFileToBreakOn[0] = 0;
#endif // MYFW_ENABLE_LUA_DEBUGGER

    // don't build on init, Rebuild is called after a scene is loaded.
    //Rebuild();

    m_RestartOnNextTick = false;
    m_WasPausedBeforeRestart = false;
}

LuaGameState::~LuaGameState()
{
    if( g_pLuaGameState == this )
        g_pLuaGameState = 0;

    if( m_pLuaState )
        lua_close( m_pLuaState );

#if MYFW_ENABLE_LUA_DEBUGGER
    close( m_ListenSocket );
    close( m_DebugSocket );
#endif
}

#if MYFW_ENABLE_LUA_DEBUGGER
void SetSocketBlockingState(int socket, bool block)
{
    if( block )
    {
        // Set socket as blocking.
#if MYFW_WINDOWS
        DWORD value = 0;
        ioctlsocket( socket, FIONBIO, &value );
#else
        const int flags = fcntl( socket, F_GETFL, 0 );
        fcntl( socket, F_SETFL, flags & ~O_NONBLOCK );
#endif
    }
    else
    {
        // Set socket as non-blocking.
#if MYFW_WINDOWS
        DWORD value = 1;
        ioctlsocket( socket, FIONBIO, &value );
#else
        const int flags = fcntl( socket, F_GETFL, 0 );
        fcntl( socket, F_SETFL, flags | O_NONBLOCK );
#endif
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
                if( g_OutputLuaDebugLog )
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
#endif // MYFW_ENABLE_LUA_DEBUGGER

void LuaGameState::Tick()
{
    if( m_RestartOnNextTick )
    {
        m_RestartOnNextTick = false;
        g_pEngineCore->OnModeStop();
        g_pEngineCore->OnModePlay();

#if MYFW_ENABLE_LUA_DEBUGGER
        if( m_WasPausedBeforeRestart )
            m_NextLineToBreakOn = -1; // Stop on the next line we reach in any file.
        else
            m_NextLineToBreakOn = INT_MAX; // Only stop on breakpoints.
#endif // MYFW_ENABLE_LUA_DEBUGGER
    }

#if MYFW_ENABLE_LUA_DEBUGGER
    CheckForDebugNetworkMessages( false );
#endif // MYFW_ENABLE_LUA_DEBUGGER
}

void LuaGameState::RunFile(const char* relativePath)
{
    int loadretcode = luaL_loadfile( m_pLuaState, relativePath );

    if( loadretcode == LUA_OK )
    {
        // Run the code to do initial parsing.
        int exeretcode = lua_pcall( m_pLuaState, 0, LUA_MULTRET, 0 );
        if( exeretcode == LUA_OK )
        {
            int bp = 1;
        }
    }
}

#if MYFW_ENABLE_LUA_DEBUGGER
void LuaGameState::SetIsDebuggerAllowedToStop(bool isallowed)
{
    if( m_DebugSocket == 0 )
        return;

    if( isallowed )
    {
        lua_sethook( m_pLuaState, DebugHookFunction, LUA_MASKLINE, 0 );
    }
    else
    {
        lua_sethook( m_pLuaState, DebugHookFunction, 0, 0 );
    }
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
        
        // Typecast to int for 64-bit: return values might be truncated. TODO: fix this.
        int socket = (int)accept( m_ListenSocket, (sockaddr*)&saddr, (socklen_t*)&fromLength );
        if( socket != -1 )
        {
            if( g_OutputLuaDebugLog )
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

            recv( m_DebugSocket, buffer, i+1, 0 );
            buffer[i] = 0;
        }

        // If we were blocking and there was an error, or the connection was closed nicely,
        //    set the socket to 0 and remove the lua hook.
        if( (block && bytes == -1) || bytes == 0 )
        {
            if( g_OutputLuaDebugLog )
                LOGInfo( "LuaDebug", "m_DebugSocket was closed\n" );

            // Remove the lua hook.
            lua_sethook( m_pLuaState, DebugHookFunction, 0, 0 );
            m_DebugSocket = 0;
            ClearAllBreakpoints();
        }

        if( bytes != -1 )
        {
            // Received a packet.
            if( bytes < buffersize )
                buffer[bytes] = 0;
            else
                buffer[buffersize-1] = 0;

            if( g_OutputLuaDebugLog )
                LOGInfo( "LuaDebug", "Received a packet (size:%d) (value:%s)\n", bytes, buffer );
    
            block = DealWithDebugNetworkMessages( buffer, block );
        }
    }
}

// Returns true if we should continue blocking, false otherwise.
bool LuaGameState::DealWithDebugNetworkMessages(char* message, bool wasblocking)
{
    // On any debug message, check for updated scripts.
    // Scripts changes will be applied on next tick (on reentry to the function changed).
    m_pEngineCore->GetManagers()->GetFileManager()->ReloadAnyUpdatedFiles( m_pEngineCore, OnFileUpdated_CallbackFunction );

    if( strcmp( message, "continue" ) == 0 )
    {
        m_NextLineToBreakOn = INT_MAX; // Only stop on breakpoints.
        return false;
    }
    if( strcmp( message, "stepin" ) == 0 ) // used by step-in, pause and break on entry.
    {
        // Break on whatever the next line is.
        m_NextLineToBreakOn = -1; // Stop on the next line we reach in any file.

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
            m_NextLineToBreakOn = -1; // If this was the last line of the function, step to next statement in calling function.
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
            m_NextLineToBreakOn = INT_MAX; // Only stop on breakpoints.
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
    if( strcmp( message, "restart" ) == 0 )
    {
        // TODO: Stop/Start gameplay, break only on breakpoints or on first command if already stopped.
        m_RestartOnNextTick = true;
        m_NextLineToBreakOn = INT_MAX; // Only stop on breakpoints.
        m_WasPausedBeforeRestart = wasblocking;
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
                // Set the lua hook (might already be set).
                lua_sethook( m_pLuaState, DebugHookFunction, LUA_MASKLINE, 0 );
                m_NextLineToBreakOn = INT_MAX; // Only stop on breakpoints.

                cJSON* jFile = cJSON_GetObjectItem( jMessage, "file" );
                cJSON* jLine = cJSON_GetObjectItem( jMessage, "line" );
                AddBreakpoint( jFile->valuestring, jLine->valueint );
            }
        }

        cJSON_Delete( jMessage );
        
        return wasblocking;
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
    send( m_DebugSocket, jsonstr, (int)strlen(jsonstr), 0 );
    
    if( g_OutputLuaDebugLog )
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
            {
                jStackArray = cJSON_CreateArray();
                cJSON_AddItemToObject( jMessage, "StackFrames", jStackArray );
            }

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

    return numlevels;
}

void LuaGameState::AddUserDataToJSONArray(cJSON* jArray, cJSON* jObject, const char* name)
{
    // Create a LuaRef from the userdata object, this will be used when querying the properties.
    luabridge::LuaRef LuaObject = luabridge::LuaRef::fromStack( m_pLuaState, -1 );

    if( LuaObject.isUserdata() )
    {
        // LuaBridge stores properties for registered classes in the metatable.
        int ret = lua_getmetatable( m_pLuaState, -1 );
        if( ret == 1 )
        {
            lua_rawgetp( m_pLuaState, -1, luabridge::getIdentityKey() );
            if( lua_isboolean( m_pLuaState, -1 ) )
            {
                lua_pop( m_pLuaState, 1 ); // Pop the bool.

                // Get the type registered with LuaBridge. ("MyMatrix", "GameObject", "Vector2", etc...)
                luabridge::rawgetfield( m_pLuaState, -1, "__type" );
                const char* type = lua_tostring( m_pLuaState, -1 );
                lua_pop( m_pLuaState, 1 ); // Pop the __type.

                cJSON_AddStringToObject( jObject, "Value", type );

                // Get the property getter function table.
                luabridge::rawgetfield( m_pLuaState, -1, "__propget" );

                if( lua_type( m_pLuaState, -1 ) == LUA_TTABLE )
                {
                    cJSON* jPropertiesArray = 0;

                    // Walk over the table, adding each property into a properties array.
                    lua_pushnil( m_pLuaState );
                    while( lua_next( m_pLuaState, -2 ) != 0 )
                    {
                        const char* propertyname = lua_tostring( m_pLuaState, -2 );

                        if( lua_type( m_pLuaState, -1 ) == LUA_TFUNCTION )
                        {
                            if( jPropertiesArray == 0 )
                            {
                                jPropertiesArray = cJSON_CreateArray();
                                cJSON_AddItemToObject( jObject, "Properties", jPropertiesArray );
                            }

                            LuaObject.push( m_pLuaState ); // Push the object pointer as the first arg to the function.
                            lua_call( m_pLuaState, 1, 1 ); // Pops the argument and the function.

                            // Add this property to the JSON Array.
                            AddValueAtTopOfStackToJSONArray( jPropertiesArray, propertyname );

                            lua_pop( m_pLuaState, 1 ); // Pop the property value.
                        }
                    }

                    lua_pop( m_pLuaState, 1 ); // Pop the __propget table.

                    // Properties don't come out in the order they were added, so sort alphabetically at least.
                    if( jPropertiesArray )
                    {
                        // Bubble sort
                        int numproperties = cJSON_GetArraySize( jPropertiesArray );
                        for( int i=0; i<numproperties-1; i++ )
                        {
                            for( int j=0; j<numproperties-i-1; j++ )
                            {
                                cJSON* jCurrentObject = cJSON_GetArrayItem( jPropertiesArray, j );
                                char* currentname = cJSON_GetObjectItem( jCurrentObject, "Name" )->valuestring;

                                cJSON* jNextObject = cJSON_GetArrayItem( jPropertiesArray, j+1 );
                                char* nextname = cJSON_GetObjectItem( jNextObject, "Name" )->valuestring;

                                if( _stricmp( currentname, nextname ) > 0 )
                                {
                                    cJSON_DetachItemFromArray( jPropertiesArray, j );
                                    cJSON_InsertItemInArray( jPropertiesArray, j+1, jCurrentObject );
                                }
                            }
                        }
                    }
                }
            }

            lua_pop( m_pLuaState, 1 ); // Pop the metatable.
        }
    }
}

void LuaGameState::AddValueAtTopOfStackToJSONArray(cJSON* jArray, const char* name)
{
    cJSON* jObject = cJSON_CreateObject();
    cJSON_AddItemToArray( jArray, jObject );

    cJSON_AddStringToObject( jObject, "Name", name );

    int type = lua_type( m_pLuaState, -1 );

    if( type == LUA_TNUMBER )
    {
        double value = lua_tonumber( m_pLuaState, -1 );
        cJSON_AddNumberToObject( jObject, "Value", value );
    }
    else if( type == LUA_TBOOLEAN )
    {
        bool value = lua_toboolean( m_pLuaState, -1 ) ? true : false;
        cJSON_AddBoolToObject( jObject, "Value", value );
    }
    else if( lua_isstring( m_pLuaState, -1 ) )
    {
        const char* value = lua_tostring( m_pLuaState, -1 );
        cJSON_AddStringToObject( jObject, "Value", value );
    }
    else if( type == LUA_TTABLE )
    {
        // TODO:
        cJSON_AddStringToObject( jObject, "Value", "<table>" );
    }
    else if( lua_isuserdata( m_pLuaState, -1 ) )
    {
        AddUserDataToJSONArray( jArray, jObject, name );
    }
    else if( type == LUA_TFUNCTION )
    {
        // TODO:
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

        // Make sure "this" has a value.
        if( !lua_isnil( m_pLuaState, -1 ) )
        {
            lua_pushnil( m_pLuaState ); // Start searching from the start of the "this" table.
            while( lua_next( m_pLuaState, -2 ) != 0 ) // Pops the key from stack, pushes new key and value.
            {
                // -2 is key, -1 is value.
                // Only add if -2 is a string type... they should all be.
                if( lua_isstring( m_pLuaState, -2 ) )
                {
                    if( jThisesArray == 0 )
                    {
                        jThisesArray = cJSON_CreateArray();
                        cJSON_AddItemToObject( jStack, "This", jThisesArray );
                    }

                    // This will break lua_next if -2 isn't a string type.
                    const char* localname = lua_tostring( m_pLuaState, -2 );

                    AddValueAtTopOfStackToJSONArray( jThisesArray, localname );
                }

                lua_pop( m_pLuaState, 1 ); // Pop the value off the stack.
            }
        }

        lua_pop( m_pLuaState, 1 ); // Pop "this" off the stack.
    }

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
            {
                jLocalsArray = cJSON_CreateArray();
                cJSON_AddItemToObject( jStack, "Local", jLocalsArray );
            }

            AddValueAtTopOfStackToJSONArray( jLocalsArray, localname );

            numlocals++;
        }

        lua_pop( m_pLuaState, 1 ); // Pop the value off the stack.
        i++;
    }

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
#endif //MYFW_ENABLE_LUA_DEBUGGER

void LuaGameState::Rebuild()
{
    if( m_pLuaState != 0 )
    {
        lua_close( m_pLuaState );
    }

    m_pLuaState = luaL_newstate();
    luaL_openlibs( m_pLuaState );

    RegisterClasses();

#if MYFW_ENABLE_LUA_DEBUGGER
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

        if( g_OutputLuaDebugLog )
            LOGInfo( "LuaDebug", "Lua debug listen socket created (sock:%d) (port:%d)\n", m_ListenSocket, LUA_DEBUG_PORT );
    }
#endif // MYFW_ENABLE_LUA_DEBUGGER
}

void LuaGameState::RegisterClasses()
{
    // Register all relevant Engine enums/defines.
    const char* definesScript = "\
BUTTONACTION_Down             = 0;\
BUTTONACTION_Up               = 1;\
BUTTONACTION_Held             = 2;\
BUTTONACTION_Wheel            = 3;\
BUTTONACTION_RelativeMovement = 4;\
\
BUTTONID_Back                 = 0;\
BUTTONID_Left                 = 1;\
BUTTONID_Right                = 2;\
BUTTONID_Up                   = 3;\
BUTTONID_Down                 = 4;\
BUTTONID_ButtonA              = 5;\
BUTTONID_ButtonB              = 6;\
BUTTONID_ButtonC              = 7;\
BUTTONID_ButtonD              = 8;\
BUTTONID_NumButtons           = 9;\
\
GL_NEAREST                    = 0x2600;\
GL_LINEAR                     = 0x2601;\
GL_CLAMP                      = 0x2900;\
GL_REPEAT                     = 0x2901;\
";
    int load_stat = luaL_loadbuffer( m_pLuaState, definesScript, strlen(definesScript), "LuaGameState::RegisterClasses -> Defines" );
    lua_pcall( m_pLuaState, 0, LUA_MULTRET, 0 );

    // Register a loginfo function.
    luabridge::getGlobalNamespace( m_pLuaState )
        .addFunction( "LogInfo", LUA_LogInfo ) // void LUA_LogInfo(const char* str)
        .addFunction( "GetSystemTime", MyTime_GetSystemTime ) // double MyTime_GetSystemTime(bool realtime)
        .addFunction( "GetRunningTime", MyTime_GetRunningTime ) // double MyTime_GetRunningTime()
        .addFunction( "GetUnpausedTime", MyTime_GetUnpausedTime ); // double MyTime_GetUnpausedTime()

    // Register some GL functions.
    LuaRegisterGLFunctions( m_pLuaState );

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<SceneID>( "SceneID" )
        .endClass();

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
        .beginClass<TextureDefinition>( "TextureDefinition" )
            .addFunction( "Release", &TextureDefinition::Lua_Release ) // void TextureDefinition::Lua_Release();
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<TextureManager>( "TextureManager" )
            .addFunction( "CreateTexture", (TextureDefinition* (TextureManager::*)(const char* textureFilename, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, MyRE::WrapModes wrapS, MyRE::WrapModes wrapT)) &TextureManager::CreateTexture ) // TextureDefinition* TextureManager::CreateTexture(const char* textureFilename, MyRE::MinFilters minFilter, MyRE::MinFilters magFilter, MyRE::WrapModes wrapS, MyRE::WrapModes wrapT)
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<ShaderGroup>( "ShaderGroup" )
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<ShaderManager>( "ShaderManager" )
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<MaterialDefinition>( "MaterialDefinition" )
            .addFunction( "SetTextureColor", &MaterialDefinition::SetTextureColor ) // void MaterialDefinition::SetTextureColor(TextureDefinition* pTexture)
            .addFunction( "SetShader", &MaterialDefinition::SetShader ) // void SetShader(ShaderGroup* pShader);
            .addFunction( "SetUVScale", &MaterialDefinition::SetUVScale ) // void SetUVScale(Vector2 scale);
            .addFunction( "SetUVOffset", &MaterialDefinition::SetUVOffset ) // void SetUVOffset(Vector2 offset);
            .addFunction( "Release", &MaterialDefinition::Lua_Release ) // void MaterialDefinition::Lua_Release();
        .endClass();

    luabridge::getGlobalNamespace( m_pLuaState )
        .beginClass<MaterialManager>( "MaterialManager" )
            .addFunction( "CreateMaterial", (MaterialDefinition* (MaterialManager::*)(const char* name, const char* relativePath)) &MaterialManager::CreateMaterial ) // MaterialDefinition* MaterialManager::CreateMaterial(const char* name = 0, const char* relativePath = 0)
            //.addFunction( "FindMaterialByFilename", &MaterialManager::FindMaterialByFilename ) // MaterialDefinition* MaterialManager::FindMaterialByFilename(const char* fullpath);
            .addFunction( "LoadMaterial", &MaterialManager::LoadMaterial ) // MaterialDefinition* MaterialManager::LoadMaterial(const char* fullpath)
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
    ComponentMeshPrimitive::LuaRegister( m_pLuaState );
    ComponentHeightmap::LuaRegister( m_pLuaState );
    ComponentVoxelMesh::LuaRegister( m_pLuaState );
    ComponentVoxelWorld::LuaRegister( m_pLuaState );
    //ComponentType_Light,
    //ComponentType_CameraShadow,
    //ComponentType_PostEffect,
    Component3DCollisionObject::LuaRegister( m_pLuaState );
    //ComponentType_3DJointPoint2Point,
    //ComponentType_3DJointHinge,
    //ComponentType_3DJointSlider,
    Component2DCollisionObject::LuaRegister( m_pLuaState );
    //ComponentType_2DJointRevolute,
    //ComponentType_2DJointPrismatic,
    //ComponentType_2DJointWeld,
    ComponentLuaScript::LuaRegister( m_pLuaState );
#if MYFW_USING_MONO
    ComponentMonoScript::LuaRegister( m_pLuaState );
#endif
    ComponentParticleEmitter::LuaRegister( m_pLuaState );
    ComponentAnimationPlayer::LuaRegister( m_pLuaState );
    ComponentAnimationPlayer2D::LuaRegister( m_pLuaState );
    ComponentAudioPlayer::LuaRegister( m_pLuaState );
    ComponentObjectPool::LuaRegister( m_pLuaState );
    ComponentMenuPage::LuaRegister( m_pLuaState );

    // Register the MenuItem types.
    MenuItem::LuaRegister( m_pLuaState );
    MenuButton::LuaRegister( m_pLuaState );
    MenuSprite::LuaRegister( m_pLuaState );
    MenuText::LuaRegister( m_pLuaState );

    // Register global managers.
    luabridge::setGlobal( m_pLuaState, m_pEngineCore, "EngineCore" ); // EngineCore*
    luabridge::setGlobal( m_pLuaState, m_pEngineCore->GetComponentSystemManager(), "ComponentSystemManager" ); // ComponentSystemManager*
    luabridge::setGlobal( m_pLuaState, m_pEngineCore->GetManagers()->GetFileManager(), "FileManager" ); // FileManager*
    luabridge::setGlobal( m_pLuaState, m_pEngineCore->GetManagers()->GetTextureManager(), "TextureManager" ); // TextureManager*
    luabridge::setGlobal( m_pLuaState, m_pEngineCore->GetManagers()->GetMaterialManager(), "MaterialManager" ); // MaterialManager*
    luabridge::setGlobal( m_pLuaState, m_pEngineCore->GetSoundManager(), "SoundManager" ); // SoundManager*
}

#endif //MYFW_USING_LUA
