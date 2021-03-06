//
// Copyright (c) 2014-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentLuaScript_H__
#define __ComponentLuaScript_H__

#if MYFW_USING_LUA

#include "ComponentSystem/BaseComponents/ComponentUpdateable.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "ComponentSystem/EngineComponents/ComponentScriptBase.h"

class ComponentLuaScript : public ComponentScriptBase
{
private:
    // Component Variable List.
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentLuaScript );

public:
    LuaGameState* m_pLuaGameState; // A reference to a global lua_State managed elsewhere.

protected:
    bool m_ScriptLoaded;
    bool m_Playing;
    bool m_ErrorInScript;
    bool m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading;

    char m_LuaGameObjectName[100];
    const ComponentLuaScript* m_pCopyExternsFromThisComponentAfterLoadingScript;

    MyFileObject* m_pScriptFile;
#if MYFW_EDITOR
    std::string m_pLuaInlineScript_OnPlay;
#else
    char* m_pLuaInlineScript_OnPlay; // If this isn't nullptr, this string will get run instead of the OnPlay function in the lua file.
#endif

public:
    ComponentLuaScript(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentLuaScript();
    SetClassnameBase( "LuaScriptComponent" ); // Only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentLuaScript&)*pObject; }
    ComponentLuaScript& operator=(const ComponentLuaScript& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    MyFileObject* GetScriptFile() { return m_pScriptFile; }
    void SetScriptFile(MyFileObject* script);
    void LoadScript();
    void LoadInLineScripts();
    void ParseExterns(luabridge::LuaRef LuaObject);
    virtual void ProgramVariables(bool updateExposedVariables = false) override;
    void SetExternFloat(const char* name, float newValue);

    void HandleLuaError(const char* functionname, const char* errormessage);

    virtual void OnLoad();
    virtual void OnPlay();
    virtual void OnStop();
    virtual void OnGameObjectEnabled();
    virtual void OnGameObjectDisabled();
    virtual void Tick(float deltaTime) {} // TODO: Remove when clearing these out from ComponentUpdatable.

    bool OnTouch(int action, int id, float x, float y, float pressure, float size) { return false; } // TODO: Remove when clearing these out from ComponentUpdatable.
    bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id) { return false; } // TODO: Remove when clearing these out from ComponentUpdatable.

    // GameObject callbacks.
    virtual void OnGameObjectDeleted(GameObject* pGameObject) override;

    // Event callbacks.
    static bool StaticOnEvent(void* pObjectPtr, MyEvent* pEvent) { return ((ComponentLuaScript*)pObjectPtr)->OnEvent( pEvent ); }
    bool OnEvent(MyEvent* pEvent);

    bool IsScriptLoaded() { return m_ScriptLoaded; }

protected:
    // Callback functions for various events.
    MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
    // Runtime component variable callbacks. //_VARIABLE_LIST
    void* GetPointerValue(ComponentVariable* pVar);
    void SetPointerValue(ComponentVariable* pVar, const void* newvalue);
    const char* GetPointerDesc(ComponentVariable* pVar);
    void SetPointerDesc(ComponentVariable* pVar, const char* newdesc);

public:
#if MYFW_EDITOR
    enum RightClickOptions
    {
        RightClick_CreateNewScriptFile = 2000,
        RightClick_LaunchScriptEditor = 2001,
    };

    void CreateNewScriptFile();

#if MYFW_USING_IMGUI
    virtual void AddAllVariablesToWatchPanel(CommandStack* pCommandStack) override;
#endif

    static void StaticOnFileUpdated(void* pObjectPtr, MyFileObject* pFile) { ((ComponentLuaScript*)pObjectPtr)->OnFileUpdated( pFile ); }
    void OnFileUpdated(MyFileObject* pFile);

    // Component variable callbacks.
    void* OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);

#if MYFW_USING_IMGUI
    void OnRightClickCallback(ComponentVariable* pVar);
#endif
    
    // Exposed variable changed callback (not from watch panel).
    virtual void OnExposedVarValueChanged(ExposedVariableDesc* pVar, int component, bool finishedChanging, ExposedVariableValue oldValue, void* oldPointer) override;

    static void StaticOnRightClickExposedVariable(void* pObjectPtr, int controlid) { ((ComponentLuaScript*)pObjectPtr)->OnRightClickExposedVariable( controlid ); }
    void OnRightClickExposedVariable(int controlid);
#endif //MYFW_EDITOR

public:
    bool DoesFunctionExist(const char* pFuncName)
    {
        if( m_ScriptLoaded == false ) return false;
        if( m_ErrorInScript ) return false;

        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );
        MyAssert( LuaObject.isNil() == false );

        if( LuaObject[pFuncName].isFunction() == false )
            return false;

        return true;
    }

    bool CallFunctionEvenIfGameplayInactive(const char* pFuncName)
    {
        if( m_ScriptLoaded == false ) return false;
        if( m_ErrorInScript ) return false;
        //if( m_Playing == false ) return false;

        // Find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );
        MyAssert( LuaObject.isNil() == false );

        // Call pFuncName.
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );

        ProgramVariables( false );
        try { if( LuaObject[pFuncName]( gameObjectData ) == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }

    bool CallFunction(const char* pFuncName)
    {
        if( m_ScriptLoaded == false ) return false;
        if( m_ErrorInScript ) return false;
        if( m_Playing == false ) return false;

        // Find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );
        MyAssert( LuaObject.isNil() == false );

        // Call pFuncName.
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );

        ProgramVariables( false );
        try { if( LuaObject[pFuncName]( gameObjectData ) == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }

    template <class P1>
    bool CallFunction(const char* pFuncName, P1 p1)
    {
        if( m_ScriptLoaded == false ) return false;
        if( m_ErrorInScript ) return false;
        if( m_Playing == false ) return false;

        // Find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );
        MyAssert( LuaObject.isNil() == false );

        // Call pFuncName.
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );

        ProgramVariables( false );
        try { if( LuaObject[pFuncName]( gameObjectData, p1 ) == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }

    template <class P1, class P2>
    bool CallFunction(const char* pFuncName, P1 p1, P2 p2)
    {
        if( m_ScriptLoaded == false ) return false;
        if( m_ErrorInScript ) return false;
        if( m_Playing == false ) return false;

        // Find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );
        MyAssert( LuaObject.isNil() == false );

        // Call pFuncName.
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );

        ProgramVariables( false );
        try { if( LuaObject[pFuncName]( gameObjectData, p1, p2 ) == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }

    template <class P1, class P2, class P3>
    bool CallFunction(const char* pFuncName, P1 p1, P2 p2, P3 p3)
    {
        if( m_ScriptLoaded == false ) return false;
        if( m_ErrorInScript ) return false;
        if( m_Playing == false ) return false;

        // Find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );
        MyAssert( LuaObject.isNil() == false );

        // Call pFuncName.
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );

        ProgramVariables( false );
        try { if( LuaObject[pFuncName]( gameObjectData, p1, p2, p3 ) == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }

    template <class P1, class P2, class P3, class P4, class P5>
    bool CallFunction(const char* pFuncName, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        if( m_ScriptLoaded == false ) return false;
        if( m_ErrorInScript ) return false;
        if( m_Playing == false ) return false;

        // Find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );
        MyAssert( LuaObject.isNil() == false );

        // Call pFuncName.
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );

        ProgramVariables( false );
        try { if( LuaObject[pFuncName]( gameObjectData, p1, p2, p3, p4, p5 ) == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }

    template <class P1, class P2, class P3, class P4, class P5, class P6>
    bool CallFunction(const char* pFuncName, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
        if( m_ScriptLoaded == false ) return false;
        if( m_ErrorInScript ) return false;
        if( m_Playing == false ) return false;

        // Find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );
        MyAssert( LuaObject.isNil() == false );

        // Call pFuncName.
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );

        ProgramVariables( false );
        try { if( LuaObject[pFuncName]( gameObjectData, p1, p2, p3, p4, p5, p6 ) == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }

    template <class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
    bool CallFunction(const char* pFuncName, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
    {
        if( m_ScriptLoaded == false ) return false;
        if( m_ErrorInScript ) return false;
        if( m_Playing == false ) return false;

        // Find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );
        MyAssert( LuaObject.isNil() == false );

        // Call pFuncName.
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        luabridge::LuaRef gameObjectData = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_LuaGameObjectName );

        ProgramVariables( false );
        try { if( LuaObject[pFuncName]( gameObjectData, p1, p2, p3, p4, p5, p6, p7, p8 ) == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }
};

#endif //MYFW_USING_LUA

#endif //__ComponentLuaScript_H__
