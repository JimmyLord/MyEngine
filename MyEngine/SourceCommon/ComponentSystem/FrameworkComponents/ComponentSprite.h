//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentSprite_H__
#define __ComponentSprite_H__

#include "ComponentSystem/BaseComponents/ComponentRenderable.h"

class ComponentTransform;

class ComponentSprite : public ComponentRenderable
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentSprite ); //_VARIABLE_LIST

public:
    bool m_WaitingToAddToRenderGraph;
    RenderGraphObject* m_pRenderGraphObject;

    MySprite* m_pSprite;
    ColorByte m_Tint;
    Vector2 m_Size;

public:
    ComponentSprite();
    virtual ~ComponentSprite();
    SetClassnameWithParent( "SpriteComponent", ComponentRenderable ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentSprite&)*pObject; }
    ComponentSprite& operator=(const ComponentSprite& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    virtual void OnLoad();

    static void StaticOnTransformChanged(void* pObjectPtr, Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor) { ((ComponentSprite*)pObjectPtr)->OnTransformChanged( newpos, newrot, newscale, changedbyuserineditor ); }
    void OnTransformChanged(Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor);

    virtual MaterialDefinition* GetMaterial(int submeshIndex) { if( m_pSprite ) return m_pSprite->GetMaterial(); return 0; }
    virtual void SetMaterial(MaterialDefinition* pMaterial, int submeshIndex);

    virtual void SetVisible(bool visible);

    virtual void AddToRenderGraph();
    virtual void RemoveFromRenderGraph();
    virtual void PushChangesToRenderGraphObjects();

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
    // Exposed to Lua, change elsewhere if function signature changes.
    MySprite* GetSprite() { return m_pSprite; }

    // Runtime component variable callbacks. //_VARIABLE_LIST
    static void* StaticGetPointerValue(void* pObjectPtr, ComponentVariable* pVar) { return ((ComponentSprite*)pObjectPtr)->GetPointerValue(pVar); }
    void* GetPointerValue(ComponentVariable* pVar);

    static void StaticSetPointerValue(void* pObjectPtr, ComponentVariable* pVar, const void* newvalue) { return ((ComponentSprite*)pObjectPtr)->SetPointerValue(pVar, newvalue); }
    void SetPointerValue(ComponentVariable* pVar, const void* newvalue);

    static const char* StaticGetPointerDesc(void* pObjectPtr, ComponentVariable* pVar) { return ((ComponentSprite*)pObjectPtr)->GetPointerDesc( pVar ); }
    const char* GetPointerDesc(ComponentVariable* pVar);

    static void StaticSetPointerDesc(void* pObjectPtr, ComponentVariable* pVar, const char* newdesc) { return ((ComponentSprite*)pObjectPtr)->SetPointerDesc( pVar, newdesc ); }
    void SetPointerDesc(ComponentVariable* pVar, const char* newdesc);

public:
#if MYFW_EDITOR
    virtual ComponentVariable* GetComponentVariableForMaterial(int submeshIndex);

    virtual bool IsReferencingFile(MyFileObject* pFile);

#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentSprite*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
#endif //MYFW_USING_WX

    //virtual bool ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar);
    virtual void VariableAddedToWatchPanel(ComponentVariable* pVar);

    // Component variable callbacks. //_VARIABLE_LIST
    void* OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);

#if _DEBUG && MYFW_WINDOWS
    void TriggerBreakpointOnNextDraw();
#endif //_DEBUG && MYFW_WINDOWS

#endif //MYFW_EDITOR
};

#endif //__ComponentSprite_H__
