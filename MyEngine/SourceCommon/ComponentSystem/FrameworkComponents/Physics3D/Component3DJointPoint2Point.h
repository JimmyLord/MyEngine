//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Component3DJointPoint2Point_H__
#define __Component3DJointPoint2Point_H__

#if MYFW_USING_BULLET

#include "ComponentSystem/FrameworkComponents/Physics3D/Component3DJointBase.h"

class Component3DCollisionObject;

class Component3DJointPoint2Point : public Component3DJointBase
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( Component3DJointPoint2Point );

protected:
    Component3DCollisionObject* m_pSecondCollisionObject;

    Vector3 m_PivotA;
    Vector3 m_PivotB;

    // Runtime vars, filled in OnPlay();
    btPoint2PointConstraint* m_pJoint;
    btRigidBody* m_pBody;
    btRigidBody* m_pSecondBody;

public:
    Component3DJointPoint2Point(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager);
    virtual ~Component3DJointPoint2Point();
    SetClassnameBase( "3DJoint-Point2PointComponent" ); // only first 8 character count. // "3DJoint-"
    //SetClassnameWithParent( "3DPoint2PointComponent", "3DJoint" ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    //virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    //virtual void ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (Component3DJointPoint2Point&)*pObject; }
    Component3DJointPoint2Point& operator=(const Component3DJointPoint2Point& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    virtual void OnPlay();
    virtual void OnStop();

    virtual void RemoveJointFromWorld();

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
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((Component3DJointPoint2Point*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
#endif //MYFW_USING_WX

    // Component variable callbacks.
    void* OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);
#endif //MYFW_EDITOR
};

#endif //MYFW_USING_BULLET

#endif //__Component3DJointPoint2Point_H__
