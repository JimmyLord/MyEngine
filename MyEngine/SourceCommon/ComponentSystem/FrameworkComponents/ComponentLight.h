//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentLight_H__
#define __ComponentLight_H__

class ComponentLight : public ComponentData
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentLight );

protected:
    LightTypes m_LightType;

    MyLight* m_pLight;

public:
    ComponentLight();
    virtual ~ComponentLight();
    SetClassnameBase( "LightComponent" ); // only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentLight&)*pObject; }
    ComponentLight& operator=(const ComponentLight& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    virtual void OnGameObjectEnabled();
    virtual void OnGameObjectDisabled();

    static void StaticOnTransformChanged(void* pObjectPtr, Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyeditor) { ((ComponentLight*)pObjectPtr)->OnTransformChanged( newpos, newrot, newscale, changedbyeditor ); }
    void OnTransformChanged(Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyeditor);

    // pre-DrawCallback functions
    virtual bool IsVisible();
    virtual bool ExistsOnLayer(unsigned int layerflags);

protected:
    // Callback functions for various events.
    //MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
#if MYFW_USING_WX
    MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
#endif //MYFW_USING_WX
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentLight*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);

    void* OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue);
    void* OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y);
#endif //MYFW_USING_WX
};

#endif //__ComponentLight_H__
