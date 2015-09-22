//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentTemplate_H__
#define __ComponentTemplate_H__

class ComponentTemplate : public ComponentBase
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentTemplate );

public:
    Vector3 m_SampleVector3;

public:
    ComponentTemplate();
    virtual ~ComponentTemplate();
    SetClassnameBase( "TemplateComponent" ); // only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentTemplate&)*pObject; }
    ComponentTemplate& operator=(const ComponentTemplate& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentTemplate*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear);

    // Component variable callbacks.
    static void* StaticOnDrop(void* pObjectPtr, ComponentVariable* pVar, wxCoord x, wxCoord y) { return ((ComponentTemplate*)pObjectPtr)->OnDrop(pVar, x, y); }
    void* OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y);

    static void* StaticOnValueChanged(void* pObjectPtr, ComponentVariable* pVar, bool finishedchanging, double oldvalue) { return ((ComponentTemplate*)pObjectPtr)->OnValueChanged( pVar, finishedchanging ); }
    void* OnValueChanged(ComponentVariable* pVar, bool finishedchanging);
#endif //MYFW_USING_WX
};

#endif //__ComponentTemplate_H__
