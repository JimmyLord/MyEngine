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

#ifndef __ComponentSystemManager_H__
#define __ComponentSystemManager_H__

class ComponentSystemManager;
class GameObject;
class ComponentBase;
class ComponentCamera;

extern ComponentSystemManager* g_pComponentSystemManager;

class ComponentSystemManager
#if MYFW_USING_WX
: public wxEvtHandler
#endif
{
public:
    ComponentTypeManager* m_pComponentTypeManager;
    CPPListHead m_GameObjects;

    // a component can only exist in one of these lists ATM
    CPPListHead m_ComponentsCamera;
    CPPListHead m_ComponentsInputHandler;
    CPPListHead m_ComponentsUpdateable;
    CPPListHead m_ComponentsRenderable;
    CPPListHead m_ComponentsData;

    unsigned int m_NextGameObjectID;
    unsigned int m_NextComponentID;

public:
    ComponentSystemManager(ComponentTypeManager* typemanager);
    virtual ~ComponentSystemManager();

    char* SaveSceneToJSON();
    void LoadSceneFromJSON(const char* jsonstr);

    void SyncAllRigidBodiesToObjectTransforms();

    void Clear(bool clearunmanagedcomponents = true);

    GameObject* CreateGameObject(bool manageobject = true);
    void DeleteGameObject(GameObject* pObject);
    GameObject* CopyGameObject(GameObject* pObject);

    GameObject* FindGameObjectByID(unsigned int id);
    GameObject* FindGameObjectByName(const char* name);
    ComponentCamera* GetFirstCamera();

    ComponentBase* AddComponent(ComponentBase* pComponent);
    void DeleteComponent(ComponentBase* pComponent);

    void Tick(double TimePassed);
    void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight);
    void OnDrawFrame();
    void OnDrawFrame(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride);
    void DrawMousePickerFrame(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderGroup);

    void OnPlay();
    void OnStop();

    bool OnTouch(int action, int id, float x, float y, float pressure, float size);
    bool OnButtons(GameCoreButtonActions action, GameCoreButtonIDs id);

public:
#if MYFW_USING_WX
    static void StaticOnLeftClick(void* pObjectPtr) { ((ComponentSystemManager*)pObjectPtr)->OnLeftClick(true); }
    static void StaticOnRightClick(void* pObjectPtr) { ((ComponentSystemManager*)pObjectPtr)->OnRightClick(); }
    void OnLeftClick(bool clear);
    void OnRightClick();
    void OnPopupClick(wxEvent &evt); // used as callback for wxEvtHandler, can't be virtual(will crash, haven't looked into it).
#endif //MYFW_USING_WX
};

#endif //__ComponentSystemManager_H__
