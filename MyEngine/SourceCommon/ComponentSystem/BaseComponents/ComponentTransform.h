//
// Copyright (c) 2014-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentTransform_H__
#define __ComponentTransform_H__

#include "ComponentBase.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"

class ComponentTransform : public ComponentBase
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentTransform );

protected:
    CPPListHead m_TransformChangedCallbackList;

    ComponentTransform* m_pParentTransform; // This is redundant for slightly faster access to transform; The parent GameObject is stored in GameObject.

    MyMatrix m_WorldTransform;
    bool m_WorldTransformIsDirty;
    Vector3 m_WorldPosition;
    Vector3 m_WorldRotation; // In degrees.
    Vector3 m_WorldScale;

    MyMatrix m_LocalTransform;
    bool m_LocalTransformIsDirty;
    Vector3 m_LocalPosition;
    Vector3 m_LocalRotation; // In degrees.
    Vector3 m_LocalScale;

public:
    ComponentTransform(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentTransform();
    SetClassnameBase( "TransformComponent" ); // Only first 8 character count.

    // These 2 methods are called by the EngineComponentTypeManager at startup and shutdown.
    static void SystemStartup();
    static void SystemShutdown();

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid);

    // Used by Prefab export for children of base prefab object.
    virtual cJSON* ExportLocalTransformAsJSONObject();
    virtual void ImportLocalTransformFromJSONObject(cJSON* jsonobj);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentTransform&)*pObject; }
    ComponentTransform& operator=(const ComponentTransform& other);

    virtual void RegisterCallbacks() {} // TODO: Change this component to use callbacks.
    virtual void UnregisterCallbacks() {} // TODO: Change this component to use callbacks.

#if MYFW_EDITOR
    void SetPositionByEditor(Vector3 pos);
    void SetScaleByEditor(Vector3 scale);
    void SetRotationByEditor(Vector3 eulerAngles);
#endif

    ComponentTransform* GetParentTransform() { return m_pParentTransform; }
    void SetParentTransform(ComponentTransform* pNewParentTransform);

    void SetWorldTransformIsDirty() { m_WorldTransformIsDirty = true; }
    void SetWorldTransform(const MyMatrix* mat);
    MyMatrix* GetWorldTransform(bool markDirty = false);
    Vector3 GetWorldPosition();
    Vector3 GetWorldRotation();
    Vector3 GetWorldScale();
    MyMatrix GetWorldRotPosMatrix();

    // Recalculate the matrix each time we set any of the 3 properties. // Not efficient.
    void SetWorldPosition(Vector3 pos);
    void SetWorldRotation(Vector3 rot);
    void SetWorldScale(Vector3 scale);

    void SetLocalTransform(MyMatrix* mat);
    MyMatrix* GetLocalTransform(bool markDirty = false);
    Vector3 GetLocalPosition();
    Vector3 GetLocalRotation();
    Vector3 GetLocalScale();
    MyMatrix GetLocalRotPosMatrix();

    void Scale(MyMatrix* pScaleMatrix, Vector3 pivot);
    void Rotate(MyMatrix* pRotMatrix, Vector3 pivot);
    void LookAt(Vector3 pos);

    // Recalculate the matrix each time we set any of the 3 properties. // Not efficient.
    void SetLocalPosition(Vector3 pos);
    void SetLocalRotation(Vector3 rot);
    void SetLocalScale(Vector3 scale);

    void UpdateLocalSRT();
    void UpdateWorldSRT();
    void UpdateTransform();

    // Callbacks.
    void RegisterTransformChangedCallback(void* pObj, TransformChangedCallbackFunc* pCallback);
    void UnregisterTransformChangedCallbacks(void* pObj);

    // Parent transform changed.
    static void StaticOnParentTransformChanged(void* pObjectPtr, const Vector3& newPos, const Vector3& newRot, const Vector3& newScale, bool changedByUserInEditor) { ((ComponentTransform*)pObjectPtr)->OnParentTransformChanged( newPos, newRot, newScale, changedByUserInEditor ); }
    void OnParentTransformChanged(const Vector3& newPos, const Vector3& newRot, const Vector3& newScale, bool changedByUserInEditor);

public:
#if MYFW_EDITOR
    bool IsAnyParentInList(std::vector<GameObject*>& gameobjects);

#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;
    //int m_ControlID_ParentTransform;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    
    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentTransform*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);

#endif
    // Component variable callbacks.
    void* OnDropTransform(ComponentVariable* pVar, bool changedByInterface, int x, int y);

    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);
#endif //MYFW_EDITOR
};

#endif //__ComponentTransform_H__
