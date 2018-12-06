//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SceneHandler_H__
#define __SceneHandler_H__

class GameObject;

struct SceneInfo
{
public:
#if MYFW_USING_WX
    wxTreeItemId m_TreeID;
#endif
    
    TCPPListHead<GameObject*> m_GameObjects; // scene level game objects, children are stored in a list inside the game object.
    Box2DWorld* m_pBox2DWorld; // each scene has it's own Box2D world, TODO: runtime created physics objects won't work.

    char m_FullPath[MAX_PATH];
    unsigned int m_NextGameObjectID;
    unsigned int m_NextComponentID;
    bool m_InUse;

public:
    SceneInfo();

    void Reset();

    void ChangePath(const char* newfullpath);
};

class SceneHandler
#if MYFW_USING_WX
: public wxEvtHandler
#endif
{
protected:
    SceneID m_SceneIDBeingAffected;

public:
    SceneHandler();
    virtual ~SceneHandler();

public:
#if MYFW_USING_WX
    enum RightClickOptions
    {
        RightClick_UnloadScene = 1000,
        RightClick_AddGameObject,
        RightClick_AddFolder,
        RightClick_AddLogicGameObject,

        RightClick_AddGameObjectFromTemplate,
            // all higher values reserved for different templates.

        // Note: Don't add anything larger than 100000 without changing GameObject.h's RightClickOptions
    };

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId treeid, unsigned int count) { ((SceneHandler*)pObjectPtr)->OnLeftClick( treeid, count, true ); }
    void OnLeftClick(wxTreeItemId treeid, unsigned int count, bool clear);

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId treeid) { ((SceneHandler*)pObjectPtr)->OnRightClick( treeid ); }
    void OnRightClick(wxTreeItemId treeid);
    void AddGameObjectMenuOptionsToMenu(wxMenu* menu, int itemidoffset, SceneID sceneid);
    int AddGameObjectTemplatesToMenu(wxMenu* menu, int itemidoffset, int startindex);
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).
    void HandleRightClickCommand(int id, GameObject* pParentGameObject);

    static void StaticOnDrag(void* pObjectPtr) { ((SceneHandler*)pObjectPtr)->OnDrag(); }
    void OnDrag();

    static void StaticOnDrop(void* pObjectPtr, wxTreeItemId treeid, int controlid, int x, int y) { ((SceneHandler*)pObjectPtr)->OnDrop( treeid, controlid, x, y ); }
    void OnDrop(wxTreeItemId treeid, int controlid, int x, int y);

    static void StaticOnLabelEdit(void* pObjectPtr, wxTreeItemId treeid, wxString newlabel) { ((SceneHandler*)pObjectPtr)->OnLabelEdit( treeid, newlabel ); }
    void OnLabelEdit(wxTreeItemId treeid, wxString newlabel);
#endif //MYFW_USING_WX
};

#endif //__SceneHandler_H__
