//
// Copyright (c) 2014-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentLuaScript_H__
#define __ComponentLuaScript_H__

enum ExposedVariableTypes
{
    ExposedVariableType_Unused,
    ExposedVariableType_Float,
    ExposedVariableType_GameObject,
};

struct ExposedVariableDesc
{
    std::string name;
    ExposedVariableTypes type;
    union // TODO?: make these values shared between c++ and lua so they can be changed/saved more easily.
    {
        double value;
        void* pointer;
    };

    bool inuse; // used internally when reparsing the file.
    int controlID;

    ExposedVariableDesc()
    {
        name = "";
        type = ExposedVariableType_Unused;
        value = 0;
        inuse = false;
        controlID = -1;
    }
};

class ComponentLuaScript : public ComponentUpdateable
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentLuaScript );

    static const int MAX_EXPOSED_VARS = 4; // TODO: fix this hardcodedness

public:
    LuaGameState* m_pLuaGameState; // a reference to a global lua_State managed elsewhere.

protected:
    bool m_ScriptLoaded;
    bool m_Playing;
    bool m_ErrorInScript;
    bool m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading;

    const ComponentLuaScript* m_pCopyExternsFromThisComponentAfterLoadingScript;

    MyFileObject* m_pScriptFile;
    MyList<ExposedVariableDesc*> m_ExposedVars;

public:
    ComponentLuaScript();
    virtual ~ComponentLuaScript();
    SetClassnameBase( "LuaScriptComponent" ); // only first 8 character count.

    //virtual void LuaRegister();

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentLuaScript&)*pObject; }
    ComponentLuaScript& operator=(const ComponentLuaScript& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    MyFileObject* GetScriptFile() { return m_pScriptFile; }
    void SetScriptFile(MyFileObject* script);
    void LoadScript();
    void ParseExterns(luabridge::LuaRef LuaObject);
    void ProgramVariables(luabridge::LuaRef LuaObject, bool updateexposedvariables = false);

    void HandleLuaError(const char* functionname, const char* errormessage);

    virtual void OnLoad();
    virtual void OnPlay();
    virtual void OnStop();
    virtual void OnGameObjectEnabled();
    virtual void OnGameObjectDisabled();
    virtual void Tick(double TimePassed) {} // TODO: remove when clearing these out from ComponentUpdatable

    bool OnTouch(int action, int id, float x, float y, float pressure, float size) { return false; } // TODO: remove when clearing these out from ComponentUpdatable
    bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id) { return false; } // TODO: remove when clearing these out from ComponentUpdatable

    // GameObject callbacks.
    static void StaticOnGameObjectDeleted(void* pObjectPtr, GameObject* pGameObject) { ((ComponentLuaScript*)pObjectPtr)->OnGameObjectDeleted( pGameObject ); }
    void OnGameObjectDeleted(GameObject* pGameObject);

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
#if MYFW_USING_WX
    enum RightClickOptions
    {
        RightClick_CreateNewScriptFile = 2000,
    };

    void CreateNewScriptFile();

    static void StaticOnFileUpdated(void* pObjectPtr, MyFileObject* pFile) { ((ComponentLuaScript*)pObjectPtr)->OnFileUpdated( pFile ); }
    void OnFileUpdated(MyFileObject* pFile);

    static bool m_PanelWatchBlockVisible;
    int m_ControlIDOfFirstExtern;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    
    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentLuaScript*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);

    virtual void AppendItemsToRightClickMenu(wxMenu* pMenu);
    void OnPopupClick(wxEvent &evt);

    // Component variable callbacks.
    void* OnDropCV(ComponentVariable* pVar, wxCoord x, wxCoord y);
    void* OnValueChangedCV(ComponentVariable* pVar, bool finishedchanging, double oldvalue);

    // Watch panel callbacks.
    static void StaticOnDrop(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((ComponentLuaScript*)pObjectPtr)->OnDrop(controlid, x, y); }
    void OnDrop(int controlid, wxCoord x, wxCoord y);
    void* ProcessOnDrop(int controlid, wxCoord x, wxCoord y);
    
    static void StaticOnValueChanged(void* pObjectPtr, int controlid, bool finishedchanging, double oldvalue) { ((ComponentLuaScript*)pObjectPtr)->OnValueChanged( controlid, finishedchanging, oldvalue ); }
    void OnValueChanged(int controlid, bool finishedchanging, double oldvalue);

    void UpdateChildrenWithNewValue(int controlid, bool finishedchanging, double oldvalue, void* oldpointer);
    void UpdateChildrenInGameObjectListWithNewValue(ExposedVariableDesc* pVar, unsigned int varindex, GameObject* first, int controlid, bool finishedchanging, double oldvalue, void* oldpointer);
    void UpdateChildGameObjectWithNewValue(ExposedVariableDesc* pVar, unsigned int varindex, GameObject* pChildGameObject, int controlid, bool finishedchanging, double oldvalue, void* oldpointer);
#endif //MYFW_USING_WX

public:
    bool CallFunction(const char* pFuncName)
    {
        if( m_ErrorInScript ) return false;
        if( m_Playing == false ) return false;

        // find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        MyAssert( LuaObject.isNil() == false );

        // call pFuncName
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        ProgramVariables( LuaObject, false );
        try { if( LuaObject[pFuncName]() == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }

    template <class P1>
    bool CallFunction(const char* pFuncName, P1 p1)
    {
        if( m_ErrorInScript ) return false;
        if( m_Playing == false ) return false;

        // find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        MyAssert( LuaObject.isNil() == false );

        // call pFuncName
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        ProgramVariables( LuaObject, false );
        try { if( LuaObject[pFuncName]( p1 ) == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }

    template <class P1, class P2>
    bool CallFunction(const char* pFuncName, P1 p1, P2 p2)
    {
        if( m_ErrorInScript ) return false;
        if( m_Playing == false ) return false;

        // find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        MyAssert( LuaObject.isNil() == false );

        // call pFuncName
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        ProgramVariables( LuaObject, false );
        try { if( LuaObject[pFuncName]( p1, p2 ) == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }

    template <class P1, class P2, class P3, class P4, class P5>
    bool CallFunction(const char* pFuncName, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        if( m_ErrorInScript ) return false;
        if( m_Playing == false ) return false;

        // find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        MyAssert( LuaObject.isNil() == false );

        // call pFuncName
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        ProgramVariables( LuaObject, false );
        try { if( LuaObject[pFuncName]( p1, p2, p3, p4, p5 ) == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }

    template <class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
    bool CallFunction(const char* pFuncName, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
    {
        if( m_ErrorInScript ) return false;
        if( m_Playing == false ) return false;

        // find the function and call it.
        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->m_FilenameWithoutExtension );
        MyAssert( LuaObject.isNil() == false );

        // call pFuncName
        if( LuaObject[pFuncName].isFunction() == false ) return false;

        ProgramVariables( LuaObject, false );
        try { if( LuaObject[pFuncName]( p1, p2, p3, p4, p5, p6, p7, p8 ) == LUA_OK ) return true; return false; }
        catch(luabridge::LuaException const& e) { HandleLuaError( pFuncName, e.what() ); }
        return false;
    }
};

#endif //__ComponentLuaScript_H__
