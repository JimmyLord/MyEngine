//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
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
};

class ComponentLuaScript : public ComponentUpdateable
{
    static const int MAX_EXPOSED_VARS = 4; // TODO: fix this hardcodedness

public:
    LuaGameState* m_pLuaGameState; // a reference to a global lua_State managed elsewhere.

protected:
    bool m_ScriptLoaded;
    bool m_Playing;
    bool m_ErrorInScript;
    bool m_ShouldBePlayingButIsntBecauseScriptFileWasStillLoading;

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

    MyFileObject* GetScriptFile() { return m_pScriptFile; }
    void SetScriptFile(MyFileObject* script);
    void LoadScript();
    void ParseExterns(luabridge::LuaRef LuaObject);
    void ProgramVariables(luabridge::LuaRef LuaObject, bool updateexposedvariables = false);

    void HandleLuaError(const char* functionname, const char* errormessage);

    virtual void OnLoad();
    virtual void OnPlay();
    virtual void OnStop();
    virtual void Tick(double TimePassed);

    void OnScriptLoaded();
    bool OnTouch(int action, int id, float x, float y, float pressure, float size);
    bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);

    void CallFunction(const char* pFuncName);

    // GameObject callbacks.
    static void StaticOnGameObjectDeleted(void* pObjectPtr, GameObject* pGameObject) { ((ComponentLuaScript*)pObjectPtr)->OnGameObjectDeleted( pGameObject ); }
    void OnGameObjectDeleted(GameObject* pGameObject);

public:
#if MYFW_USING_WX
    enum RightClickOptions
    {
        RightClick_CreateNewScriptFile = 2000,
    };

    int m_ControlID_Script;

    void CreateNewScriptFile();

    static void StaticOnFileUpdated(void* pObjectPtr, MyFileObject* pFile) { ((ComponentLuaScript*)pObjectPtr)->OnFileUpdated( pFile ); }
    void OnFileUpdated(MyFileObject* pFile);

    static bool m_PanelWatchBlockVisible;
    int m_ControlIDOfFirstExtern;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    
    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentLuaScript*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear);

    virtual void AppendItemsToRightClickMenu(wxMenu* pMenu);
    void OnPopupClick(wxEvent &evt);

    // Watch panel callbacks.
    static void StaticOnDrop(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((ComponentLuaScript*)pObjectPtr)->OnDrop(controlid, x, y); }
    void OnDrop(int controlid, wxCoord x, wxCoord y);
    
    static void StaticOnValueChanged(void* pObjectPtr, int controlid, bool finishedchanging, double oldvalue) { ((ComponentLuaScript*)pObjectPtr)->OnValueChanged( controlid, finishedchanging ); }
    void OnValueChanged(int controlid, bool finishedchanging);
#endif //MYFW_USING_WX
};

#endif //__ComponentLuaScript_H__
