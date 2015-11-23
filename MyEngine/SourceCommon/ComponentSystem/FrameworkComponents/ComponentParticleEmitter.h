//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentParticleEmitter_H__
#define __ComponentParticleEmitter_H__

class ComponentTransform;

class ComponentParticleEmitter : public ComponentRenderable
{
public:
    struct Particle
    {
        Vector3 pos;
        float size;
        ColorFloat color;
        Vector3 dir;
        float timealive;
        float timetolive;
    };

public:
    MaterialDefinition* m_pMaterial;

    // renderers could be shared between emitters, but would make culling harder in future
    ParticleRenderer* m_pParticleRenderer;

    bool m_RunInEditor;
    //bool m_RunWhenPaused;

    MyActivePool<Particle*> m_Particles;

    float m_TimeTilNextSpawn;
    float m_BurstTimeLeft;

    float m_BurstDuration;

    float m_SpawnTime;
    float m_SpawnTimeVariation;
    float m_InitialSpeedBoost;
    Vector2 m_Center;
    Vector2 m_CenterVariation;
    float m_Size;
    float m_SizeVariation;
    float m_ScaleSpeed;
    ColorFloat m_Color1;
    ColorFloat m_Color2;
    bool m_UseColorsAsOptions;
    bool m_UseColorsAsRange; // not coded
    float m_ColorTransitionDelay;
    float m_ColorTransitionSpeed;
    Vector3 m_Dir;
    Vector3 m_DirVariation;
    float m_TimeToLive;
    float m_TimeToLiveVariation;

    float m_AlphaModifier;

public:
    ComponentParticleEmitter();
    virtual ~ComponentParticleEmitter();
    SetClassnameWithParent( "ParticleEmitterComponent", ComponentRenderable ); // only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentParticleEmitter&)*pObject; }
    virtual ComponentParticleEmitter& operator=(const ComponentParticleEmitter& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    virtual MaterialDefinition* GetMaterial(int submeshindex) { return m_pMaterial; }
    virtual void SetMaterial(MaterialDefinition* pMaterial, int submeshindex);

    void CreateBurst(int number, Vector3 pos);

protected:
    // Callback functions for various events.
    MYFW_DECLARE_COMPONENT_CALLBACK_TICK(); // TickCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONSURFACECHANGED(); // OnSurfaceChangedCallback
    MYFW_DECLARE_COMPONENT_CALLBACK_DRAW(); // DrawCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONTOUCH(); // OnTouchCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONBUTTONS(); // OnButtonsCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONKEYS(); // OnKeysCallback
    //MYFW_DECLARE_COMPONENT_CALLBACK_ONFILERENAMED(); // OnFileRenamedCallback

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    
    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentParticleEmitter*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);

    // Watch panel callbacks.
    static void StaticOnDropMaterial(void* pObjectPtr, int controlid, wxCoord x, wxCoord y) { ((ComponentParticleEmitter*)pObjectPtr)->OnDropMaterial(controlid, x, y); }
    void OnDropMaterial(int controlid, wxCoord x, wxCoord y);
#endif //MYFW_USING_WX
};

#endif //__ComponentParticleEmitter_H__
