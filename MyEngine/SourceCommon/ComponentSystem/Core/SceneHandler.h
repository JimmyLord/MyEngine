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
    wxTreeItemId sceneid;
    char fullpath[MAX_PATH];
};

class SceneHandler
#if MYFW_USING_WX
: public wxEvtHandler
#endif
{
protected:

public:
    SceneHandler();
    virtual ~SceneHandler();

public:
#if MYFW_USING_WX
    static void StaticOnLeftClick(void* pObjectPtr, unsigned int count) { ((SceneHandler*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);

    static void StaticOnRightClick(void* pObjectPtr) { ((SceneHandler*)pObjectPtr)->OnRightClick(); }
    void OnRightClick();
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).

    static void StaticOnDrag(void* pObjectPtr) { ((SceneHandler*)pObjectPtr)->OnDrag(); }
    void OnDrag();

    static void StaticOnDrop(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((SceneHandler*)pObjectPtr)->OnDrop(controlid, x, y); }
    void OnDrop(int controlid, wxCoord x, wxCoord y);

    static void StaticOnLabelEdit(void* pObjectPtr, wxString newlabel) { ((SceneHandler*)pObjectPtr)->OnLabelEdit( newlabel ); }
    void OnLabelEdit(wxString newlabel);
#endif //MYFW_USING_WX
};

#endif //__SceneHandler_H__
