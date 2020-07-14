//
// Copyright (c) 2019-2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorDocument_Heightmap_H__
#define __EditorDocument_Heightmap_H__

#include "../SourceEditor/Documents/EditorDocument.h"
#include "../SourceEditor/Prefs/EditorKeyBindings.h"

class ComponentHeightmap;
class GameObject;
class Job_CalculateNormals;

class EditorDocument_Heightmap : public EditorDocument
{
    friend class Job_CalculateNormals;
public:
    enum MaterialTypes
    {
        Mat_Point,
        Mat_BrushOverlay,
        Mat_NumMaterials,
    };

    enum class Tool
    {
        Raise,
        Lower,
        Level,
        Erode,
        None,
    };

    enum class ToolState
    {
        Idle,
        Active,
    };

protected:
    ComponentHeightmap* m_pHeightmap;
    bool m_HeightmapOwnedByUs;

    Tool m_CurrentTool;
    ToolState m_CurrentToolState;

    GameObject* m_pPoint;
    int m_IndexOfPointBeingDragged;
    Vector2 m_PositionMouseWentDown;
    bool m_NewMousePress;
    Vector3 m_WorldSpaceMousePosition;
    Vector3 m_WorldSpaceMousePositionWhenToolStarted;
    Vector3 m_WorldSpaceMousePositionAtDesiredHeight; // Used by "level" tool.

    MaterialDefinition* m_pMaterials[Mat_NumMaterials];

    bool m_HeightmapNormalsNeedRebuilding;
    Job_CalculateNormals* m_pJob_CalculateNormals;

    // Warnings.
    bool m_ShowWarning_CloseEditor;

    // Editor settings.
    float m_BrushSoftness;
    float m_BrushRadius;
    float m_RaiseAmount;
    bool m_LevelUseBrushHeight;
    float m_LevelHeight;

    // Noise Settings.
    bool m_Noise_Dirty;
    bool m_Noise_AutoGenerate;
    int m_Noise_Seed;
    float m_Noise_Amplitude;
    Vector2 m_Noise_Frequency;
    Vector2 m_Noise_Offset;
    int m_Noise_Octaves;
    float m_Noise_Persistance;
    float m_Noise_Lacunarity;
    int m_Noise_Debug_Octave;

    // Erosion Settings.
    float m_Erode_Inertia;
    float m_Erode_MaxCapacity;
    float m_Erode_DepositionPerc;
    float m_Erode_Evaporation;
    float m_Erode_MinSlope;
    float m_Erode_Gravity;
    int m_Erode_Radius;
    float m_Erode_ErosionFactor;
    int m_Erode_MaxSteps;

    bool m_AlwaysRecalculateNormals;

protected:
    // File IO.
    virtual const char* GetFileExtension() { return ".myheightmap"; };
    virtual const char* GetDefaultDataFolder() { return "Data\\Meshes\\"; };
    virtual const char* GetDefaultFileSaveFilter() { return "MyHeightmap Files=*.myheightmap"; };

public:
    EditorDocument_Heightmap(EngineCore* pEngineCore, const char* relativePath, ComponentHeightmap* pHeightmap);
    virtual ~EditorDocument_Heightmap();

    // Pure virtual methods.
    virtual ComponentBase* GetComponentBeingEdited() override;

    virtual void Initialize();

    virtual bool IsBusy();

    virtual void OnDrawFrame() override;
    virtual void AddImGuiOverlayItems();

    void GetMouseRay(Vector2 mousepos, Vector3* start, Vector3* end);

    // Input/Hotkey handling
    virtual HotkeyContext GetHotkeyContext() override { return HotkeyContext::HeightmapEditor; }
    virtual bool HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure) override;
    virtual bool ExecuteHotkeyAction(HotkeyAction action) override;

    virtual void Update() override; // From EditorDocument.

    void AddPaintTools();
    void AddNoiseTools();
    void AddErodeTools();

    void CancelCurrentOperation();

    ComponentHeightmap* GetHeightmapBeingEdited() { return m_pHeightmap; }
    void SetHeightmap(ComponentHeightmap* pHeightmap);

    MaterialDefinition* GetMaterial(MaterialTypes type);

    void ApplyCurrentTool(Vector3 mouseIntersectionPoint, int mouseAction);

protected:
    void Save();
    void Load();
};

#endif //__EditorDocument_Heightmap_H__
