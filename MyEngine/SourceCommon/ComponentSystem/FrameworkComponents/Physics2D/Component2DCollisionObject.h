//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
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
    Physics2DPrimitiveType_Chain,
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

    Box2DWorld* m_pBox2DWorld;
    b2Body* m_pBody; // only the first of these components on a gameobject will have a body, every other one is an additional fixture
    b2Fixture* m_pFixture;

    int m_PrimitiveType;

    Vector2 m_Offset;
    Vector3 m_Scale;

    // Body properties.
    bool m_Static;
    bool m_FixedRotation;

    // Fixture properties.
    float m_Density;
    bool m_IsSensor;
    float m_Friction;
    float m_Restitution;

#if MYFW_EDITOR
    std::vector<b2Vec2> m_Vertices;
#else
    MyList<b2Vec2> m_Vertices;
#endif

public:
    Component2DCollisionObject();
    virtual ~Component2DCollisionObject();
    SetClassnameBase( "2DCollisionObjectComponent" ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (Component2DCollisionObject&)*pObject; }
    Component2DCollisionObject& operator=(const Component2DCollisionObject& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    virtual void OnPlay();
    virtual void OnStop();

    virtual void SetEnabled(bool enabled);

    void CreateBody();
    b2Body* GetBody() { return m_pBody; }

    void SyncRigidBodyToTransform();

    // Getters.
    Vector2 GetLinearVelocity();
    float GetMass();

    // Setters.
    void SetPositionAndAngle(Vector2 newPosition, float angle);
    void SetSensor(bool isSensor);

    // Forces.
    void ClearVelocity();
    void ApplyForce(Vector2 force, Vector2 localpoint);
    void ApplyLinearImpulse(Vector2 impulse, Vector2 localpoint);

protected:
    // Callback functions for various events.
    MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
#if MYFW_EDITOR
    MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
#endif
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
#if MYFW_EDITOR
#if MYFW_USING_IMGUI
    virtual void AddAllVariablesToWatchPanel();
#endif

#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((Component2DCollisionObject*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
    virtual bool ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar);
#endif //MYFW_USING_WX

    void SetVertices(const luabridge::LuaRef verts, unsigned int count);

    // Component variable callbacks.
    void* OnDrop(ComponentVariable* pVar, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue);

    static void StaticOnTransformChanged(void* pObjectPtr, Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor) { ((Component2DCollisionObject*)pObjectPtr)->OnTransformChanged( newpos, newrot, newscale, changedbyuserineditor ); }
    void OnTransformChanged(Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor);

    static void StaticOnButtonEditChain(void* pObjectPtr, int buttonid) { ((Component2DCollisionObject*)pObjectPtr)->OnButtonEditChain( buttonid ); }
    void OnButtonEditChain(int buttonid);
#endif //MYFW_USING_WX
};

#endif //__Component2DCollisionObject_H__
