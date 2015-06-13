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
class ComponentCamera;
class ComponentLuaScript;
class MenuItem;

class ComponentMenuPage : public ComponentBase
{
    static const int MAX_MENU_ITEMS = 128;
    static const int MAX_MENU_NAME_LEN = 32;

protected:
    MyFileObject* m_pMenuLayoutFile;

    bool m_MenuItemsCreated;

    unsigned int m_MenuItemsUsed;
    MenuItem* m_pMenuItems[MAX_MENU_ITEMS];
    MenuItem* m_pMenuItemHeld;

public:
    ComponentTransform* m_pComponentTransform;
    ComponentCamera* m_pComponentCamera;
    ComponentLuaScript* m_pComponentLuaScript;

    bool m_Visible;
    unsigned int m_LayersThisExistsOn;

public:
    ComponentMenuPage();
    virtual ~ComponentMenuPage();
    SetClassnameBase( "MenuPageComponent" ); // only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jComponent, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentMenuPage&)*pObject; }
    virtual ComponentMenuPage& operator=(const ComponentMenuPage& other);

    void FindLuaScriptComponentPointer();
    virtual void OnLoad();
    virtual void OnPlay();
    //virtual void OnStop();

    // will return true if input is used.
    virtual bool OnTouch(int action, int id, float x, float y, float pressure, float size);
    virtual bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);

    virtual void Tick(double TimePassed);
    virtual void Draw();

    void ClearAllMenuItems();
    void SetMenuLayoutFile(MyFileObject* pFile);

public:
#if MYFW_USING_WX
    enum RightClickOptions
    {
        RightClick_AddButton = 2000,
        RightClick_AddSprite,
        RightClick_AddText,
    };

    int m_ControlID_Filename;
    int m_ControlID_ComponentCamera;
    bool h_RenameInProgress;

    static bool m_PanelWatchBlockVisible;

    void SaveMenuPageToDisk(const char* fullpath);
    void RenameMenuPage(const char* newfullpath);

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentMenuPage*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear);

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((ComponentMenuPage*)pObjectPtr)->OnRightClick(); }
    //virtual void OnRightClick();
    virtual void AppendItemsToRightClickMenu(wxMenu* pMenu);
    void OnPopupClick(wxEvent &evt);

    // Watch panel callbacks.
    static void StaticOnValueChanged(void* pObjectPtr, int controlid, bool finishedchanging) { ((ComponentMenuPage*)pObjectPtr)->OnValueChanged( controlid, finishedchanging ); }
    void OnValueChanged(int controlid, bool finishedchanging);

    static void StaticOnMenuItemDeleted(void* pObjectPtr, MenuItem* pMenuItem) { ((ComponentMenuPage*)pObjectPtr)->OnMenuItemDeleted( pMenuItem ); }
    void OnMenuItemDeleted(MenuItem* pMenuItem);

    static void StaticOnDropComponent(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((ComponentMenuPage*)pObjectPtr)->OnDropComponent(controlid, x, y); }
    void OnDropComponent(int controlid, wxCoord x, wxCoord y);
#endif //MYFW_USING_WX
};

#endif //__ComponentMenuPage_H__
