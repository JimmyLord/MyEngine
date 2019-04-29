//
// Copyright (c) 2015-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentTemplate_H__
#define __ComponentTemplate_H__

#include "ComponentSystem/BaseComponents/ComponentBase.h"

// Search for ADDING_NEW_ComponentType to find some changes needed for engine.

class ComponentTemplate : public ComponentBase
{
private:
    // Component Variable List.
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentTemplate );

public:
    Vector3 m_SampleVector3;

public:
    ComponentTemplate(ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentTemplate();
    SetClassnameBase( "TemplateComponent" ); // Only first 8 characters count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luaState);
#endif //MYFW_USING_LUA

    //virtual cJSON* ExportAsJSONObject(bool saveSceneID, bool saveID);
    //virtual void ImportFromJSONObject(cJSON* jComponent, SceneID sceneID);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentTemplate&)*pObject; }
    ComponentTemplate& operator=(const ComponentTemplate& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

protected:
    // Callback functions for various events.
    //MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
#if MYFW_EDITOR

    // Component variable callbacks.
    void* OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);
#endif //MYFW_EDITOR
};

#endif //__ComponentTemplate_H__
