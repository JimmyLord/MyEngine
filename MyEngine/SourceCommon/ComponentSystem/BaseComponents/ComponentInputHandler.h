//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentInputHandler_H__
#define __ComponentInputHandler_H__

#include "ComponentBase.h"

class ComponentTransform;

class ComponentInputHandler : public ComponentBase
{
public:
    ComponentTransform* m_pComponentTransform;

public:
    ComponentInputHandler();
    virtual ~ComponentInputHandler();
    SetClassnameBase( "InputHandlerComponent" ); // only first 8 character count.

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentInputHandler&)*pObject; }
    ComponentInputHandler& operator=(const ComponentInputHandler& other);

    virtual void RegisterCallbacks() {} // TODO: change this component to use callbacks.
    virtual void UnregisterCallbacks() {} // TODO: change this component to use callbacks.

    // will return true if input is used.
    virtual bool OnTouch(int action, int id, float x, float y, float pressure, float size) = 0;
    virtual bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id) = 0;
    virtual bool OnKeys(GameCoreButtonActions action, int keycode, int unicodechar) = 0;

public:
#if MYFW_EDITOR
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentInputHandler*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
#endif //MYFW_USING_WX
#endif //MYFW_EDITOR
};

#endif //__ComponentInputHandler_H__
