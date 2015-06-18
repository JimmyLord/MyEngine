//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SceneHandler_H__
#define __SceneHandler_H__

struct SceneInfo
{
#if MYFW_USING_WX
    wxTreeItemId treeid;
#endif
    char fullpath[MAX_PATH];
    unsigned int m_NextGameObjectID;
    unsigned int m_NextComponentID;

    SceneInfo::SceneInfo()
    {
        fullpath[0] = 0;

        m_NextGameObjectID = 1;
        m_NextComponentID = 1;
    }
};

class SceneHandler
#if MYFW_USING_WX
: public wxEvtHandler
#endif
{
protected:
    unsigned int m_SceneIDBeingAffected;

public:
    SceneHandler();
    virtual ~SceneHandler();

public:
#if MYFW_USING_WX
    enum RightClickOptions
    {
        RightClick_UnloadScene = 1000,
        RightClick_AddGameObject = 1001,
    };

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((SceneHandler*)pObjectPtr)->OnLeftClick( id, count, true ); }
    void OnLeftClick(wxTreeItemId id, unsigned int count, bool clear);

    static void StaticOnRightClick(void* pObjectPtr, wxTreeItemId id) { ((SceneHandler*)pObjectPtr)->OnRightClick( id ); }
    void OnRightClick(wxTreeItemId id);
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnDrag(void* pObjectPtr) { ((SceneHandler*)pObjectPtr)->OnDrag(); }
    void OnDrag();

    static void StaticOnDrop(void* pObjectPtr, wxTreeItemId id, int controlid, wxCoord x, wxCoord y) { ((SceneHandler*)pObjectPtr)->OnDrop(id, controlid, x, y); }
    void OnDrop(wxTreeItemId id, int controlid, wxCoord x, wxCoord y);

    static void StaticOnLabelEdit(void* pObjectPtr, wxTreeItemId id, wxString newlabel) { ((SceneHandler*)pObjectPtr)->OnLabelEdit( id, newlabel ); }
    void OnLabelEdit(wxTreeItemId id, wxString newlabel);
#endif //MYFW_USING_WX
};

#endif //__SceneHandler_H__
