//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentAudioPlayer_H__
#define __ComponentAudioPlayer_H__

class ComponentAudioPlayer : public ComponentBase
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentAudioPlayer );

protected:
    // TODO: replace with audio cue system.
    char m_SoundCueName[MAX_SOUND_CUE_NAME_LEN];
    SoundCue* m_pSoundCue;
    int m_ChannelSoundIsPlayingOn;

public:
    ComponentAudioPlayer();
    virtual ~ComponentAudioPlayer();
    SetClassnameBase( "AudioPlayerComponent" ); // only first 8 character count.

#if MYFW_USING_LUA
    static void LuaRegister(lua_State* luastate);
#endif //MYFW_USING_LUA

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentAudioPlayer&)*pObject; }
    ComponentAudioPlayer& operator=(const ComponentAudioPlayer& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

protected:
    void SetSoundCue(SoundCue* pCue);

protected:
    // Callback functions for various events.
    //MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentAudioPlayer*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);

    // Component variable callbacks.
    void* OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y);
    void* OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue);

    static void StaticOnButtonPlaySound(void* pObjectPtr) { ((ComponentAudioPlayer*)pObjectPtr)->OnButtonPlaySound(); }
    void OnButtonPlaySound();
#endif //MYFW_USING_WX
};

#endif //__ComponentAudioPlayer_H__
