//
// Copyright (c) 2014-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentTransform_H__
#define __ComponentTransform_H__

typedef void (*TransformChangedCallbackFunc)(void* pObjectPtr, Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyeditor);
struct TransformChangedCallbackStruct : public CPPListNode
{
    void* pObj;
    TransformChangedCallbackFunc pFunc;
};

extern MySimplePool<TransformChangedCallbackStruct> g_pComponentTransform_TransformChangedCallbackPool;

class ComponentTransform : public ComponentBase
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentTransform );

    static const int CALLBACK_POOL_SIZE = 100;

protected:
    CPPListHead m_TransformChangedCallbackList;

    ComponentTransform* m_pParentTransform; // for slightly faster access to transform, parent GameObject is stored in GameObject

    MyMatrix m_WorldTransform;
    bool m_WorldTransformIsDirty;
    Vector3 m_WorldPosition;
    Vector3 m_WorldRotation; // in degrees
    Vector3 m_WorldScale;

    MyMatrix m_LocalTransform;
    bool m_LocalTransformIsDirty;
    Vector3 m_LocalPosition;
    Vector3 m_LocalRotation; // in degrees
    Vector3 m_LocalScale;

public:
    ComponentTransform();
    virtual ~ComponentTransform();
    SetClassnameBase( "TransformComponent" ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentTransform&)*pObject; }
    ComponentTransform& operator=(const ComponentTransform& other);

    virtual void RegisterCallbacks() {} // TODO: change this component to use callbacks.
    virtual void UnregisterCallbacks() {} // TODO: change this component to use callbacks.

#if MYFW_USING_WX
    void SetPositionByEditor(Vector3 pos);
    void SetScaleByEditor(Vector3 scale);
    void SetRotationByEditor(Vector3 eulerangles);
#endif

    ComponentTransform* GetParentTransform() { return m_pParentTransform; }
    void SetParentTransform(ComponentTransform* pNewParentTransform);

    void SetWorldTransformIsDirty() { m_WorldTransformIsDirty = true; }
    void SetWorldTransform(MyMatrix* mat);
    MyMatrix* GetWorldTransform(bool markdirty = false);
    Vector3 GetWorldPosition();
    Vector3 GetWorldRotation();
    Vector3 GetWorldScale();
    MyMatrix GetWorldRotPosMatrix();

    // recalculate the matrix each time we set any of the 3 properties. // not efficient
    void SetWorldPosition(Vector3 pos);
    void SetWorldRotation(Vector3 rot);
    void SetWorldScale(Vector3 scale);

    void SetLocalTransform(MyMatrix* mat);
    MyMatrix* GetLocalTransform(bool markdirty = false);
    Vector3 GetLocalPosition();
    Vector3 GetLocalRotation();
    Vector3 GetLocalScale();
    MyMatrix GetLocalRotPosMatrix();

    void Scale(MyMatrix* pScaleMatrix, Vector3 pivot);
    void Rotate(MyMatrix* pRotMatrix, Vector3 pivot);
    void LookAt(Vector3 pos);

    // recalculate the matrix each time we set any of the 3 properties. // not efficient
    void SetLocalPosition(Vector3 pos);
    void SetLocalRotation(Vector3 rot);
    void SetLocalScale(Vector3 scale);

    void UpdateLocalSRT();
    void UpdateWorldSRT();
    void UpdateTransform();

    // Callbacks
    void RegisterTransformChangedCallback(void* pObj, TransformChangedCallbackFunc pCallback);
    void UnregisterTransformChangedCallbacks(void* pObj);

    // Parent transform changed
    static void StaticOnParentTransformChanged(void* pObjectPtr, Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyeditor) { ((ComponentTransform*)pObjectPtr)->OnParentTransformChanged( newpos, newrot, newscale, changedbyeditor ); }
    void OnParentTransformChanged(Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyeditor);

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;
    //int m_ControlID_ParentTransform;

    bool IsAnyParentInList(std::vector<GameObject*>& gameobjects);

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    
    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentTransform*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);

    // Component variable callbacks.
    void* OnDropTransform(ComponentVariable* pVar, wxCoord x, wxCoord y);
    void* OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue);
#endif //MYFW_USING_WX
};

#endif //__ComponentTransform_H__
