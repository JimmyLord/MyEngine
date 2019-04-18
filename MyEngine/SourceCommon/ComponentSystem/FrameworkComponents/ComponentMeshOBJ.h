//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentMeshOBJ_H__
#define __ComponentMeshOBJ_H__

#include "ComponentMesh.h"

class ComponentTransform;

class ComponentMeshOBJ : public ComponentMesh
{
protected:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentMeshOBJ ); //_VARIABLE_LIST

public:
    ComponentMeshOBJ();
    virtual ~ComponentMeshOBJ();
    SetClassnameWith2Parents( "MeshOBJComponent", ComponentMesh, ComponentRenderable ); // only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentMeshOBJ&)*pObject; }
    virtual ComponentMeshOBJ& operator=(const ComponentMeshOBJ& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    virtual void SetMesh(MyMesh* pMesh);

protected:
    // Callback functions for various events.
    //MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
    // Runtime component variable callbacks. //_VARIABLE_LIST
    //static void* StaticGetPointerValue(void* pObjectPtr, ComponentVariable* pVar) { return ((ComponentMeshOBJ*)pObjectPtr)->GetPointerValue(pVar); }
    void* GetPointerValue(ComponentVariable* pVar);

    //static void StaticSetPointerValue(void* pObjectPtr, ComponentVariable* pVar, const void* newvalue) { return ((ComponentMeshOBJ*)pObjectPtr)->SetPointerValue(pVar, newvalue); }
    void SetPointerValue(ComponentVariable* pVar, const void* newvalue);

    //static const char* StaticGetPointerDesc(void* pObjectPtr, ComponentVariable* pVar) { return ((ComponentMeshOBJ*)pObjectPtr)->GetPointerDesc( pVar ); }
    const char* GetPointerDesc(ComponentVariable* pVar);

    //static void StaticSetPointerDesc(void* pObjectPtr, ComponentVariable* pVar, const char* newdesc) { return ((ComponentMeshOBJ*)pObjectPtr)->SetPointerDesc( pVar, newdesc ); }
    void SetPointerDesc(ComponentVariable* pVar, const char* newdesc);

public:
#if MYFW_EDITOR
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    
    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentMeshOBJ*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
#endif //MYFW_USING_WX
    
    // Component variable callbacks. //_VARIABLE_LIST
    void* OnDropOBJ(ComponentVariable* pVar, bool changedByInterface, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);
#endif //MYFW_EDITOR
};

#endif //__ComponentMeshOBJ_H__
