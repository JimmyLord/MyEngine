//
// Copyright (c) 2014-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentMesh_H__
#define __ComponentMesh_H__

class ComponentTransform;
class SceneGraphObject;

extern const char* OpenGLPrimitiveTypeStrings[7];

class ComponentMesh : public ComponentRenderable
{
protected:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentMesh ); //_VARIABLE_LIST

public:
    static const int MAX_SUBMESHES = 4;

public:
    MyMesh* m_pMesh;

    bool m_WaitingToAddToSceneGraph;
    SceneGraphObject* m_pSceneGraphObjects[MAX_SUBMESHES];
    MaterialDefinition* m_MaterialList[MAX_SUBMESHES];
    int m_GLPrimitiveType;
    int m_PointSize;

public:
    ComponentMesh();
    virtual ~ComponentMesh();
    SetClassnameWithParent( "MeshComponent", ComponentRenderable ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    // ComponentBase overrides
    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jComponentMesh, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentMesh&)*pObject; }
    ComponentMesh& operator=(const ComponentMesh& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    static bool StaticOnEvent(void* pObjectPtr, MyEvent* pEvent) { return ((ComponentMesh*)pObjectPtr)->OnEvent( pEvent ); }
    virtual bool OnEvent(MyEvent* pEvent);

    // ComponentRenderable overrides
    virtual MaterialDefinition* GetMaterial(int submeshindex) { return m_MaterialList[submeshindex]; }
    virtual void SetMaterial(MaterialDefinition* pMaterial, int submeshindex);

    virtual void SetVisible(bool visible);
    //virtual bool IsVisible();

    virtual void AddToSceneGraph();
    virtual void RemoveFromSceneGraph();

protected:
    // Callback functions for various events.
    MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    
    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentMesh*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
    virtual bool ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar);
    
    // Component variable callbacks. //_VARIABLE_LIST
    void* OnDropMaterial(ComponentVariable* pVar, wxCoord x, wxCoord y);
    void* OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue);
#endif //MYFW_USING_WX
};

#endif //__ComponentMesh_H__
