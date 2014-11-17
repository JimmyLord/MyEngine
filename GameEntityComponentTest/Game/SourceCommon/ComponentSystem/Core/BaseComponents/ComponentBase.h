//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentBase_H__
#define __ComponentBase_H__

enum BaseComponentTypes
{
    BaseComponentType_Data,
    BaseComponentType_Transform, // special case of a data type.
    BaseComponentType_InputHandler,
    BaseComponentType_Updateable,
    BaseComponentType_Renderable,
    BaseComponentType_None,
};

class ComponentBase : public CPPListNode
#if MYFW_USING_WX
, public wxEvtHandler
#endif
{
public:
    BaseComponentTypes m_BaseType;
    int m_Type;
    GameObject* m_pGameObject;

    unsigned int m_ID;

public:
    ComponentBase();
    ComponentBase(GameObject* owner);
    virtual ~ComponentBase();

    virtual cJSON* ExportAsJSONObject();
    virtual void ImportFromJSONObject(cJSON* jsonobj);

    virtual void Reset();

public:
#if MYFW_USING_WX
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr) { ((ComponentBase*)pObjectPtr)->OnLeftClick(); }
    static void StaticOnRightClick(void* pObjectPtr) { ((ComponentBase*)pObjectPtr)->OnRightClick(); }
    static void StaticOnDrag(void* pObjectPtr) { ((ComponentBase*)pObjectPtr)->OnDrag(); }
    static void StaticOnDrop(void* pObjectPtr) { ((ComponentBase*)pObjectPtr)->OnDrop(); }
    void OnLeftClick();
    void OnRightClick();
    void OnPopupClick(wxEvent &evt);
    void OnDrag();
    void OnDrop();
#endif //MYFW_USING_WX
};

#endif //__ComponentBase_H__
