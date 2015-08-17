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

class ComponentMenuPage;
class ComponentTransform;
class ComponentCamera;
class ComponentLuaScript;
class MenuItem;
class MenuInputBox;
class MenuButton;

#define LEGACYHACK 0

typedef bool (*MenuPageActionCallbackFunc)(void* pObjectPtr, ComponentMenuPage* pPage, const char* function, const char* action, MenuItem* pMenuItem);
struct MenuPageActionCallbackStruct
{
    void* pObj;
    MenuPageActionCallbackFunc pFunc;
};

typedef void (*MenuPageVisibleCallbackFunc)(void* pObjectPtr, ComponentMenuPage* pPage, bool visible);
struct MenuPageVisibleCallbackStruct
{
    void* pObj;
    MenuPageVisibleCallbackFunc pFunc;
};

typedef void (*MenuPageTickCallbackFunc)(void* pObjectPtr, ComponentMenuPage* pPage, double TimePassed);
struct MenuPageTickCallbackStruct
{
    void* pObj;
    MenuPageTickCallbackFunc pFunc;
};

typedef void (*MenuPageDrawCallbackFunc)(void* pObjectPtr, ComponentMenuPage* pPage, ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride);
struct MenuPageDrawCallbackStruct
{
    void* pObj;
    MenuPageDrawCallbackFunc pFunc;
};

typedef bool (*MenuPageOnTouchCallbackFunc)(void* pObjectPtr, ComponentMenuPage* pPage, int action, int id, float x, float y, float pressure, float size);
struct MenuPageOnTouchCallbackStruct
{
    void* pObj;
    MenuPageOnTouchCallbackFunc pFunc;
};

typedef bool (*MenuPageOnButtonsCallbackFunc)(void* pObjectPtr, ComponentMenuPage* pPage, GameCoreButtonActions action, GameCoreButtonIDs id);
struct MenuPageOnButtonsCallbackStruct
{
    void* pObj;
    MenuPageOnButtonsCallbackFunc pFunc;
};

typedef bool (*MenuPageOnKeysCallbackFunc)(void* pObjectPtr, ComponentMenuPage* pPage, GameCoreButtonActions action, int keycode, int unicodechar);
struct MenuPageOnKeysCallbackStruct
{
    void* pObj;
    MenuPageOnKeysCallbackFunc pFunc;
};

#if MYFW_USING_WX
class ComponentMenuPageEventHandlerForMenuItems : public wxEvtHandler
{
public:
    MenuItem* pMenuItemSelected;
    ComponentMenuPage* pMenuPageSelected;

public:
    void OnPopupClick(wxEvent &evt);
};
#endif

class ComponentMenuPage : public ComponentBase
{
    static const int MAX_MENU_ITEMS = 128;
    static const int MAX_MENU_NAME_LEN = 32;
    static const int MAX_BUTTON_ACTION_LENGTH = 32;

protected:
    MyFileObject* m_pMenuLayoutFile;

    bool m_Visible;

    bool m_MenuItemsCreated;

    bool m_InputEnabled;
    MenuInputBox* m_pInputBoxWithKeyboardFocus;

    unsigned int m_MenuItemsUsed;
    MenuItem* m_pMenuItems[MAX_MENU_ITEMS];
    MenuItem* m_pMenuItemHeld;

    char m_ButtonActions[3][MAX_BUTTON_ACTION_LENGTH]; // for buttons B/C/D
    Vector2 m_RelativeCursorSize; // 0,0 won't resize.

    cJSON* m_MenuLayouts;
    cJSON* m_CurrentLayout;
    unsigned int m_CurrentWidth;
    unsigned int m_CurrentHeight;

    Vector4 m_ExtentsBLTRWhenPageLoaded;
    bool m_ExtentsSetWhenLoaded;

    MenuPageActionCallbackStruct m_MenuPageActionCallbackStruct;
    MenuPageVisibleCallbackStruct m_MenuPageVisibleCallbackStruct;
    MenuPageTickCallbackStruct m_MenuPageTickCallbackStruct;
    MenuPageDrawCallbackStruct m_MenuPageDrawCallbackStruct;
    MenuPageOnTouchCallbackStruct m_MenuPageOnTouchCallbackStruct;
    MenuPageOnButtonsCallbackStruct m_MenuPageOnButtonsCallbackStruct;
    MenuPageOnKeysCallbackStruct m_MenuPageOnKeysCallbackStruct;

    // Runtime vars
    unsigned int m_ItemSelected;

public:
    ComponentTransform* m_pComponentTransform;
    ComponentCamera* m_pComponentCamera;
    ComponentLuaScript* m_pComponentLuaScript;

    unsigned int m_LayersThisExistsOn;

public:
    ComponentMenuPage();
    virtual ~ComponentMenuPage();
    SetClassnameBase( "MenuPageComponent" ); // only first 8 character count.

    static void LuaRegister(lua_State* luastate);

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jComponent, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentMenuPage&)*pObject; }
    virtual ComponentMenuPage& operator=(const ComponentMenuPage& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    void FindLuaScriptComponentPointer();
    virtual void OnLoad();
    virtual void OnPlay();
    //virtual void OnStop();
    virtual void OnGameObjectEnabled();
    virtual void OnGameObjectDisabled();

    virtual void SetEnabled(bool enabled);

    void LoadLayoutBasedOnCurrentAspectRatio();
    void UpdateLayout(cJSON* layout);

    void ClearAllMenuItems();
    void SetMenuLayoutFile(MyFileObject* pFile);

    MenuItem* GetMenuItem(unsigned int index) { return m_pMenuItems[index]; }
    MenuItem* GetMenuItemByName(const char* name);
    MenuButton* GetMenuButton(unsigned int index);
    MenuButton* GetMenuButtonByName(const char* name);

    void SetSelectedItemByName(const char* name);
    MenuItem* GetSelectedItem();

    unsigned int GetNumMenuItemsUsed() { return m_MenuItemsUsed; }

    void RegisterMenuPageActionCallback(void* pObj, MenuPageActionCallbackFunc pFunc);
    void RegisterMenuPageVisibleCallback(void* pObj, MenuPageVisibleCallbackFunc pFunc);
    void RegisterMenuPageTickCallback(void* pObj, MenuPageTickCallbackFunc pFunc);
    void RegisterMenuPageDrawCallback(void* pObj, MenuPageDrawCallbackFunc pFunc);
    void RegisterMenuPageOnTouchCallback(void* pObj, MenuPageOnTouchCallbackFunc pFunc);
    void RegisterMenuPageOnButtonsCallback(void* pObj, MenuPageOnButtonsCallbackFunc pFunc);
    void RegisterMenuPageOnKeysCallback(void* pObj, MenuPageOnKeysCallbackFunc pFunc);

    void SetVisible(bool visible);
    bool IsVisible();
    void SetInputEnabled(bool inputenabled);
    bool IsOnTop(); // tests if input handlers are at the front of the componentsystemmanager's callback list.

    void RenameFileInJSONObject(cJSON* jObject, const char* fullpathbefore, const char* fullpathafter);

    void CreateMenuItems();
    bool ExecuteAction(const char* function, const char* action, MenuItem* pItem);

    void ShowPage();
    void HidePage();

protected:
    // Callback functions for various events.
    MYFW_DECLARE_COMPONENT_CALLBACK_TICK( ComponentMenuPage ); // TickCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED( ComponentMenuPage ); // OnSurfaceChangedCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_DRAW( ComponentMenuPage ); // DrawCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH( ComponentMenuPage ); // OnTouchCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS( ComponentMenuPage ); // OnButtonsCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS( ComponentMenuPage ); // OnKeysCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED( ComponentMenuPage ); // OnFileRenamedCallback

public:
#if MYFW_USING_WX
    enum RightClickOptions
    {
        RightClick_AddButton = 2000,
        RightClick_AddSprite,
        RightClick_AddText,
        RightClick_ForceReload,
        RightClick_SavePage,
        RightClick_CopyToOtherLayouts,
    };

    int m_ControlID_Filename;
    int m_ControlID_ComponentCamera;
    bool h_RenameInProgress;

    static bool m_PanelWatchBlockVisible;

#if LEGACYHACK
    void LEGACYHACK_GrabMenuItemPointersFromCurrentScreen();
#endif //LEGACYHACK

    void SaveMenuPageToDisk(const char* fullpath);
    void SaveMenuPageToDisk();
    void RenameMenuPage(const char* newfullpath);

    void SaveCurrentLayoutToJSON();
    cJSON* SaveLayoutToJSON(const char* layoutname);

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentMenuPage*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear);

    //static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((ComponentMenuPage*)pObjectPtr)->OnRightClick(); }
    //virtual void OnRightClick();
    virtual void AppendItemsToRightClickMenu(wxMenu* pMenu);
    void OnPopupClick(wxEvent &evt);

    MenuItem* AddNewMenuItemToTree(int type);
    void AddMenuItemToTree(unsigned int index, PanelObjectListCallbackLeftClick pLeftClickFunc, const char* desc);
    void CopyUniqueItemsToOtherLayouts();

    // Watch panel callbacks.
    static void StaticOnValueChanged(void* pObjectPtr, int controlid, bool finishedchanging, double oldvalue) { ((ComponentMenuPage*)pObjectPtr)->OnValueChanged( controlid, finishedchanging ); }
    void OnValueChanged(int controlid, bool finishedchanging);

    static void StaticOnMenuItemDeleted(void* pObjectPtr, MenuItem* pMenuItem) { ((ComponentMenuPage*)pObjectPtr)->OnMenuItemDeleted( pMenuItem ); }
    void OnMenuItemDeleted(MenuItem* pMenuItem);

    static void StaticOnDropComponent(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((ComponentMenuPage*)pObjectPtr)->OnDropComponent(controlid, x, y); }
    void OnDropComponent(int controlid, wxCoord x, wxCoord y);

    // Object panel callbacks for menu items in our list.
    MYFW_PANELOBJECTLIST_DEFINE_CALLBACK_ONDROP(OnDropMenuItemOnMenuItem, ComponentMenuPage);
    MYFW_PANELOBJECTLIST_DEFINE_CALLBACK_ONDROP(OnDropMenuItemOnMenuPage, ComponentMenuPage);

    ComponentMenuPageEventHandlerForMenuItems m_MenuPageEventHandlerForMenuItems;
    static void StaticOnMenuItemRightClick(void* pObjectPtr, wxTreeItemId id) { ((ComponentMenuPage*)pObjectPtr)->OnMenuItemRightClick( id ); }
    virtual void OnMenuItemRightClick(wxTreeItemId id);
#endif //MYFW_USING_WX
};

#endif //__ComponentMenuPage_H__
