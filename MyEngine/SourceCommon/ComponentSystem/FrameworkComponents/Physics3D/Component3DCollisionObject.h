//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Component3DCollisionObject_H__
#define __Component3DCollisionObject_H__

#if MYFW_USING_BULLET

#include "ComponentSystem/BaseComponents/ComponentBase.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"

enum PhysicsPrimitiveTypes //ADDING_NEW_PhysicsPrimitiveType - order doesn't matter, saved as string.
{
    PhysicsPrimitiveType_Cube,
    PhysicsPrimitiveType_Sphere,
    PhysicsPrimitiveType_StaticPlane,
    PhysicsPrimitiveType_ConvexHull,
    PhysicsPrimitive_NumTypes,
};

extern const char* PhysicsPrimitiveTypeStrings[PhysicsPrimitive_NumTypes];

class Component3DCollisionObject : public ComponentBase
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( Component3DCollisionObject );

protected:
    btRigidBody* m_pBody;

    int m_PrimitiveType;

    float m_Mass;
    Vector3 m_Scale;
    MyMesh* m_pMesh;

protected:
    // Internal functions
    void CreateBody();

public:
    Component3DCollisionObject(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager);
    virtual ~Component3DCollisionObject();
    SetClassnameBase( "3DCollisionObjectComponent" ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (Component3DCollisionObject&)*pObject; }
    Component3DCollisionObject& operator=(const Component3DCollisionObject& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

protected:
    // Callback functions for various events.
    MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
    btRigidBody* GetBody() { return m_pBody; }

    MyMesh* GetMesh() { return m_pMesh; }
    void SetMesh(MyMesh* pMesh);

    virtual void OnPlay();
    virtual void OnStop();

    void SyncRigidBodyToTransform();

    void ApplyForce(Vector3 force, Vector3 relpos);

public:
    // Runtime component variable callbacks. //_VARIABLE_LIST
    void* GetPointerValue(ComponentVariable* pVar);
    void SetPointerValue(ComponentVariable* pVar, const void* newvalue);
    const char* GetPointerDesc(ComponentVariable* pVar);
    void SetPointerDesc(ComponentVariable* pVar, const char* newdesc);

public:
#if MYFW_EDITOR
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;
    int m_ControlID_PrimitiveType;

    // Object panel callbacks.
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((Component3DCollisionObject*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);

    // Watch panel callbacks.
    //static void StaticOnValueChanged(void* pObjectPtr, int controlid, bool directlychanged, bool finishedchanging, double oldvalue, bool valuewaschangedbydragging) { ((Component3DCollisionObject*)pObjectPtr)->OnValueChanged( controlid, finishedchanging ); }
    //void OnValueChanged(int controlid, bool finishedchanging);

    //static void StaticOnDropOBJ(void* pObjectPtr, int controlid, int x, int y) { ((Component3DCollisionObject*)pObjectPtr)->OnDropOBJ(controlid, x, y); }
    //void OnDropOBJ(int controlid, int x, int y);

    static void StaticOnTransformChanged(void* pObjectPtr, const Vector3& newPos, const Vector3& newRot, const Vector3& newScale, bool changedByUserInEditor) { ((Component3DCollisionObject*)pObjectPtr)->OnTransformChanged( newPos, newRot, newScale, changedByUserInEditor ); }
    void OnTransformChanged(const Vector3& newPos, const Vector3& newRot, const Vector3& newScale, bool changedByUserInEditor);
#endif //MYFW_USING_WX

    // Component variable callbacks. //_VARIABLE_LIST
    void* OnDropOBJ(ComponentVariable* pVar, bool changedByInterface, int x, int y);

    virtual bool ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar);
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);
#endif //MYFW_EDITOR
};

#endif //MYFW_USING_BULLET

#endif //__Component3DCollisionObject_H__
