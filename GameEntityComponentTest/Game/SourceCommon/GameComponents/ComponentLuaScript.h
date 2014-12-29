//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
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
    lua_State* m_pLuaState; // a reference to a global lua_State managed elsewhere.

protected:
    bool m_ScriptLoaded;
    bool m_Playing;
    bool m_ShouldBePlayingButIsntBecauseScriptFileWasStillLoading;

    MyFileObject* m_pScriptFile;
    MyList<ExposedVariableDesc*> m_ExposedVars;

public:
    ComponentLuaScript();
    virtual ~ComponentLuaScript();

    //virtual void LuaRegister();

    virtual cJSON* ExportAsJSONObject();
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentLuaScript&)*pObject; }
    virtual ComponentLuaScript& operator=(const ComponentLuaScript& other);

    void SetScriptFile(MyFileObject* script);
    void LoadScript();
    void ParseExterns(luabridge::LuaRef LuaObject);
    void ProgramVariables(luabridge::LuaRef LuaObject, bool updateexposedvariables = false);

    virtual void OnPlay();
    virtual void OnStop();
    virtual void Tick(double TimePassed);

    void OnLoad();
    bool OnTouch(int action, int id, float x, float y, float pressure, float size);
    bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);

public:
#if MYFW_USING_WX
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticFillPropertiesWindow(void* pObjectPtr) { ((ComponentLuaScript*)pObjectPtr)->FillPropertiesWindow(true); }
    void FillPropertiesWindow(bool clear);
    static void StaticOnDrop(void* pObjectPtr) { ((ComponentLuaScript*)pObjectPtr)->OnDrop(); }
    void OnDrop();
    static void StaticOnValueChanged(void* pObjectPtr) { ((ComponentLuaScript*)pObjectPtr)->OnValueChanged(); }
    void OnValueChanged();
#endif //MYFW_USING_WX
};

#endif //__ComponentLuaScript_H__
