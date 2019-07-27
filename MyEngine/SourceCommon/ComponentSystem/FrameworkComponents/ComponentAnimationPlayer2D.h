//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentAnimationPlayer2D_H__
#define __ComponentAnimationPlayer2D_H__

#include "ComponentSystem/BaseComponents/ComponentBase.h"
#include "ComponentSystem/Core/ComponentSystemManager.h"

class ComponentSprite;

class ComponentAnimationPlayer2D : public ComponentBase
{
    friend class EditorMainFrame_ImGui; // Allows call to TickCallback() for preview in 2D animation editor.
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentAnimationPlayer2D );

protected:
    ComponentSprite* m_pSpriteComponent;

    bool m_AnimationFileLoaded;
    MyFileObject* m_pAnimationFile;
    My2DAnimInfo* m_pAnimInfo;

    uint32 m_AnimationIndex;
    float m_AnimationTime;
    uint32 m_FrameIndex;

public:
    ComponentAnimationPlayer2D(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager);
    virtual ~ComponentAnimationPlayer2D();
    SetClassnameBase( "Anim2DComponent" ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentAnimationPlayer2D&)*pObject; }
    ComponentAnimationPlayer2D& operator=(const ComponentAnimationPlayer2D& other);

    // Getters
    My2DAnimInfo* Get2DAnimInfoObject() { return m_pAnimInfo; }
    uint32 GetCurrentFrameIndex() { return m_FrameIndex; }
    uint32 GetAnimationIndexByName(const char* name);

    // Setters
    void SetAnimationFile(MyFileObject* pFile);
    void SetCurrentAnimation(uint32 anim);

    // Callbacks
    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

protected:
    // Callback functions for various events.
    MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
#if MYFW_EDITOR
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    static void StaticOnFileUpdated(void* pObjectPtr, MyFileObject* pFile) { ((ComponentAnimationPlayer2D*)pObjectPtr)->OnFileUpdated( pFile ); }
    void OnFileUpdated(MyFileObject* pFile);

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentAnimationPlayer2D*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
#endif //MYFW_USING_WX

    // Component variable callbacks.
    void* OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y);
    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue);

#if MYFW_USING_WX
    // Scene right-click options
    virtual void AddRightClickOptionsToMenu(wxMenu* pMenu, int baseid);
    virtual void OnRightClickOptionClicked(wxEvent &evt, int baseid);
#endif //MYFW_USING_WX
#endif //MYFW_EDITOR
};

#endif //__ComponentAnimationPlayer2D_H__
