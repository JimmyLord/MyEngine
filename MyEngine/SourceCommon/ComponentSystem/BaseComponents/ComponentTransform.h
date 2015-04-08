//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentTransform_H__
#define __ComponentTransform_H__

typedef void (*TransformPositionChangedCallbackFunc)(void* obj, Vector3& newpos);
struct TransformPositionChangedCallbackStruct
{
    void* pObj;
    TransformPositionChangedCallbackFunc pFunc;
};

class ComponentTransform : public ComponentBase
{
    static const int MAX_REGISTERED_CALLBACKS = 1; // TODO: fix this hardcodedness

public:
    MyMatrix m_Transform;

    ComponentTransform* m_pParentTransform;

protected:
    MyList<TransformPositionChangedCallbackStruct> m_pPositionChangedCallbackList;

    MyMatrix m_LocalTransform;
    Vector3 m_Position;
    Vector3 m_Scale;
    Vector3 m_Rotation; // in degrees

public:
    ComponentTransform();
    virtual ~ComponentTransform();

    static void LuaRegister(lua_State* luastate);

    virtual cJSON* ExportAsJSONObject();
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentTransform&)*pObject; }
    virtual ComponentTransform& operator=(const ComponentTransform& other);

    // recalculate the matrix each time we set any of the 3 properties. // not efficient
    void SetPosition(Vector3 pos);
    void SetScale(Vector3 scale);
    void SetRotation(Vector3 rot);

    Vector3 GetPosition() { return m_Transform.GetTranslation(); }

    Vector3 GetLocalPosition() { return m_Position; }
    Vector3 GetLocalScale() { return m_Scale; }
    Vector3 GetLocalRotation() { return m_Rotation; }
    MyMatrix GetLocalRotPosMatrix();
    MyMatrix* GetLocalTransform() { return &m_LocalTransform; }

    void SetParent(ComponentTransform* pNewParent);
    void UpdatePosAndRotFromLocalMatrix();
    void UpdateMatrix();
    //MyMatrix* GetMatrix();

    // Callbacks
    void RegisterPositionChangedCallback(void* pObj, TransformPositionChangedCallbackFunc pCallback);

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;
    int m_ControlID_ParentTransform;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr) { ((ComponentTransform*)pObjectPtr)->OnLeftClick( true ); }
    void OnLeftClick(bool clear);
    virtual void FillPropertiesWindow(bool clear);
    static void StaticOnDropTransform(void* pObjectPtr) { ((ComponentTransform*)pObjectPtr)->OnDropTransform(); }
    void OnDropTransform();
    static void StaticOnValueChanged(void* pObjectPtr, int id, bool finishedchanging) { ((ComponentTransform*)pObjectPtr)->OnValueChanged( id, finishedchanging ); }
    void OnValueChanged(int id, bool finishedchanging);
#endif //MYFW_USING_WX
};

#endif //__ComponentTransform_H__
