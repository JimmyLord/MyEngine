//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentCollisionObject_H__
#define __ComponentCollisionObject_H__

enum PhysicsPrimitiveTypes //ADDING_NEW_PhysicsPrimitiveType - order doesn't matter, saved as string.
{
    PhysicsPrimitiveType_Cube,
    PhysicsPrimitiveType_Sphere,
    PhysicsPrimitiveType_StaticPlane,
    PhysicsPrimitiveType_ConvexHull,
    PhysicsPrimitive_NumTypes,
};

extern const char* PhysicsPrimitiveTypeStrings[PhysicsPrimitive_NumTypes];

class ComponentCollisionObject : public ComponentUpdateable
{
public:
    btRigidBody* m_pBody;

    int m_PrimitiveType;

    float m_Mass;
    Vector3 m_Scale;
    MyMesh* m_pMesh;

public:
    ComponentCollisionObject();
    virtual ~ComponentCollisionObject();
    SetClassnameBase( "CollisionObjectComponent" ); // only first 8 character count.

    static void LuaRegister(lua_State* luastate);

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentCollisionObject&)*pObject; }
    ComponentCollisionObject& operator=(const ComponentCollisionObject& other);

    virtual void RegisterCallbacks() {} // TODO: change this component to use callbacks.
    virtual void UnregisterCallbacks() {} // TODO: change this component to use callbacks.

    void SetMesh(MyMesh* pMesh);

    virtual void OnPlay();
    virtual void OnStop();
    virtual void Tick(double TimePassed);

    void SyncRigidBodyToTransform();

    void ApplyForce(Vector3 force, Vector3 relpos);

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;
    int m_ControlID_PrimitiveType;

    // Object panel callbacks.
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentCollisionObject*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false);

    // Watch panel callbacks.
    static void StaticOnValueChanged(void* pObjectPtr, int controlid, bool finishedchanging, double oldvalue) { ((ComponentCollisionObject*)pObjectPtr)->OnValueChanged( controlid, finishedchanging ); }
    void OnValueChanged(int controlid, bool finishedchanging);

    static void StaticOnDropOBJ(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((ComponentCollisionObject*)pObjectPtr)->OnDropOBJ(controlid, x, y); }
    void OnDropOBJ(int controlid, wxCoord x, wxCoord y);

    static void StaticOnTransformPositionChanged(void* pObjectPtr, Vector3& newpos, bool changedbyeditor) { ((ComponentCollisionObject*)pObjectPtr)->OnTransformPositionChanged( newpos, changedbyeditor ); }
    void OnTransformPositionChanged(Vector3& newpos, bool changedbyeditor);
#endif //MYFW_USING_WX
};

#endif //__ComponentCollisionObject_H__
