//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorInterface_H__
#define __EditorInterface_H__

#if MYFW_USING_WX
class EditorInterfaceWxEventHandler : public wxEvtHandler
{
public:
    enum RightClickOptions
    {
        RightClick_Placeholder = 1000, // general right-click options, not used yet.
        RightClick_ComponentOps = 10000, // starts at 10000 adds 1000 for each component, if components need more than 1000 right click options change this
    };

public:
    void* m_pPointer;
    int m_ValueInt;

public:
    EditorInterfaceWxEventHandler()
    {
        m_pPointer = 0;
        m_ValueInt = 0;
    };
    void OnPopupClick(wxEvent &evt);
};
#endif //MYFW_USING_WX

class EditorInterface
{
protected:
    bool m_ShowRightClickMenu;
    GameObject* m_pGameObjectRightClicked;
#if MYFW_USING_WX
    EditorInterfaceWxEventHandler m_EditorInterfaceWxEventHandler;
#endif //MYFW_USING_WX

public:
    EditorInterface();
    virtual ~EditorInterface();

    virtual void Initialize() = 0;

    virtual void OnActivated();
    virtual void OnDeactivated();

    virtual void Tick(double TimePassed);
    virtual void OnDrawFrame(unsigned int canvasid);

    virtual bool HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure) = 0;

    virtual void RenderObjectIDsToFBO();
    virtual unsigned int GetIDAtPixel(unsigned int x, unsigned int y, bool createnewbitmap, bool includetransformgizmo);
    virtual GameObject* GetObjectAtPixel(unsigned int x, unsigned int y, bool createnewbitmap, bool includetransformgizmo);
    virtual void SelectObjectsInRectangle(unsigned int sx, unsigned int sy, unsigned int ex, unsigned int ey);

    void SetModifierKeyStates(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure);
    void ClearModifierKeyStates(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure);
    bool HandleInputForEditorCamera(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure);
};

#endif //__EditorInterface_H__
