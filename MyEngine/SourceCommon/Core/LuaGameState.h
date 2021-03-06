//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __LuaGameState_H__
#define __LuaGameState_H__

#if MYFW_USING_LUA

// Enable lua debugging in debug and editor builds. TODO: fix lua debugging in NaCl builds
#if (_DEBUG || MYFW_USING_WX) && !MYFW_NACL
#define MYFW_ENABLE_LUA_DEBUGGER 1
#else
#define MYFW_ENABLE_LUA_DEBUGGER 0
#endif

class EngineCore;
class LuaGameState;

extern LuaGameState* g_pLuaGameState;

void LuaBridgeExt_LogExceptionFormattedForVisualStudioOutputWindow(const char* userdata, const char* fullpath, const char* what);

#if _DEBUG
#include <vector>
#endif

class LuaGameState
{
public:
    struct Breakpoint
    {
        char file[MAX_PATH];
        int line;
    };

    lua_State* m_pLuaState;
    EngineCore* m_pEngineCore;

#if MYFW_ENABLE_LUA_DEBUGGER
    int m_ListenSocket;
    int m_DebugSocket;

    // Used by step-in/out/over.
    int m_NextLineToBreakOn;
    char m_NextSourceFileToBreakOn[MAX_PATH];

    // For breakpoints.
    std::vector<Breakpoint> m_Breakpoints;
#endif

    // For Restart.
    bool m_RestartOnNextTick;
    bool m_WasPausedBeforeRestart;

public:
    LuaGameState(EngineCore* pEngineCore);
    virtual ~LuaGameState();

    virtual void Rebuild();
    virtual void RegisterClasses();

    void Tick();

    void RunFile(const char* relativePath);

    // For use to avoid debug breakpoints
#if MYFW_ENABLE_LUA_DEBUGGER
    void SetIsDebuggerAllowedToStop(bool isallowed);

    void CheckForDebugNetworkMessages(bool block);
    bool DealWithDebugNetworkMessages(char* message, bool wasblocking);

    void SendStoppedMessage();
    int AddStackToJSONMessage(cJSON* jMessage);
    void AddUserDataToJSONArray(cJSON* jArray, cJSON* jObject, const char* name);
    void AddValueAtTopOfStackToJSONArray(cJSON* jArray, const char* name);
    int AddLocalVarsToStackInJSONMessage(cJSON* jStack, lua_Debug* ar);

    void ClearAllBreakpoints();
    void ClearAllBreakpointsFromFile(char* fullpath);
    void AddBreakpoint(char* fullpath, int line);
#endif
};

#endif //MYFW_USING_LUA

#endif //__LuaGameState_H__
