//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Component2DCollisionObject_H__
#define __Component2DCollisionObject_H__

enum Physics2DPrimitiveTypes //ADDING_NEW_Physics2DPrimitiveType - order doesn't matter, saved as string.
{
    Physics2DPrimitiveType_Box,
    Physics2DPrimitiveType_Circle,
    Physics2DPrimitiveType_Edge,
    Physics2DPrimitive_NumTypes,
};

extern const char* Physics2DPrimitiveTypeStrings[Physics2DPrimitive_NumTypes];

class Component2DCollisionObject : public ComponentBase
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( Component2DCollisionObject );

public:
    ComponentLuaScript* m_pComponentLuaScript;

    b2Body* m_pBody;

    int m_PrimitiveType;

    Vector3 m_Scale;

    bool m_Static;
    float m_Density;
    bool m_IsSensor;
    float m_Friction;
    float m_Restitution;
    //MyMesh* m_pMesh;

public:
    Component2DCollisionObject();
    virtual ~Component2DCollisionObject();
    SetClassnameBase( "2DCollisionObjectComponent" ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    //virtual cJSON* ExportAsJSONObject(bool savesceneid);
    //virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (Component2DCollisionObject&)*pObject; }
    Component2DCollisionObject& operator=(const Component2DCollisionObject& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    virtual void OnPlay();
    virtual void OnStop();

    void CreateBody();

    void SyncRigidBodyToTransform();

    void ApplyForce(Vector2 force, Vector2 point);
    void ApplyLinearImpulse(Vector2 impulse, Vector2 point);
    Vector2 GetLinearVelocity();
    float GetMass();

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
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((Component2DCollisionObject*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);

    // Component variable callbacks.
    void* OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y);
    void* OnValueChanged(ComponentVariable* pVar, bool finishedchanging, double oldvalue);

    static void StaticOnTransformPositionChanged(void* pObjectPtr, Vector3& newpos, bool changedbyeditor) { ((Component2DCollisionObject*)pObjectPtr)->OnTransformPositionChanged( newpos, changedbyeditor ); }
    void OnTransformPositionChanged(Vector3& newpos, bool changedbyeditor);
#endif //MYFW_USING_WX
};

#endif //__Component2DCollisionObject_H__
