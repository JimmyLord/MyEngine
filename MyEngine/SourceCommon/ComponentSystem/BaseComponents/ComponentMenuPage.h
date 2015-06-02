//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentMenuPage_H__
#define __ComponentMenuPage_H__

class ComponentTransform;
class MenuItem;

class ComponentMenuPage : public ComponentBase
{
    static const int MAX_MENUITEMS = 128;
    static const int MAX_MENU_NAME_LEN = 32;

protected:
    unsigned int m_MenuItemsUsed;
    MenuItem* m_pMenuItems[MAX_MENUITEMS];
    MenuItem* m_pMenuItemHeld;

public:
    ComponentTransform* m_pComponentTransform;
    ComponentCamera* m_pCamera;

    MyFileObject* m_pMenuLayoutFile;

public:
    ComponentMenuPage();
    virtual ~ComponentMenuPage();
    SetClassnameBase( "MenuPageComponent" ); // only first 8 character count.

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentMenuPage&)*pObject; }
    virtual ComponentMenuPage& operator=(const ComponentMenuPage& other);

    // will return true if input is used.
    virtual bool OnTouch(int action, int id, float x, float y, float pressure, float size);
    virtual bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);

    virtual void Tick(double TimePassed);
    virtual void Draw();

    void ClearAllMenuItems();

public:
#if MYFW_USING_WX
    int m_ControlID_Filename;
    bool h_RenameInProgress;

    static bool m_PanelWatchBlockVisible;

    void SaveMenuPageToDisk(const char* fullpath);
    void RenameMenuPage(const char* newfullpath);

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    static void StaticOnLeftClick(void* pObjectPtr, unsigned int count) { ((ComponentMenuPage*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear);

    static void StaticOnRightClick(void* pObjectPtr) { ((ComponentMenuPage*)pObjectPtr)->OnRightClick(); }
    //virtual void OnRightClick();
    virtual void AppendItemsToRightClickMenu(wxMenu* pMenu);
    void OnPopupClick(wxEvent &evt);

    static void StaticOnValueChanged(void* pObjectPtr, int controlid, bool finishedchanging) { ((ComponentMenuPage*)pObjectPtr)->OnValueChanged( controlid, finishedchanging ); }
    void OnValueChanged(int controlid, bool finishedchanging);

    static void StaticOnMenuItemDeleted(void* pObjectPtr, MenuItem* pMenuItem) { ((ComponentMenuPage*)pObjectPtr)->OnMenuItemDeleted( pMenuItem ); }
    void OnMenuItemDeleted(MenuItem* pMenuItem);
#endif //MYFW_USING_WX
};

#endif //__ComponentMenuPage_H__
