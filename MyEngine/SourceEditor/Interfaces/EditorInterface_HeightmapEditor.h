//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorInterface_HeightmapEditor_H__
#define __EditorInterface_HeightmapEditor_H__

#include "EditorInterface.h"
#include "../SourceEditor/Documents/EditorDocument.h"

class ComponentHeightmap;
class Job_CalculateNormals;

class EditorInterface_HeightmapEditor : public EditorInterface, public EditorDocument
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
        None,
    };

    enum class ToolState
    {
        Idle,
        Active,
    };

protected:
    ComponentHeightmap* m_pHeightmap;

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

    bool m_AlwaysRecalculateNormals;

public:
    EditorInterface_HeightmapEditor(EngineCore* pEngineCore);
    virtual ~EditorInterface_HeightmapEditor();

    virtual void Initialize() override;

    virtual bool IsBusy() override;

    virtual void OnActivated() override;
    virtual void OnDeactivated() override;
    virtual void OnDrawFrame(unsigned int canvasID) override;
    virtual void AddImGuiOverlayItems() override;

    virtual bool HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure) override;
    virtual bool ExecuteHotkeyAction(HotKeyAction action) override;

    virtual void Update() override; // From EditorDocument.

    void CancelCurrentOperation();

    ComponentHeightmap* GetHeightmapBeingEdited() { return m_pHeightmap; }
    void SetHeightmap(ComponentHeightmap* pHeightmap);

    MaterialDefinition* GetMaterial(MaterialTypes type);

    void ApplyCurrentTool(Vector3 mouseIntersectionPoint, int mouseAction);

protected:
    void Save();
};

#endif //__EditorInterface_HeightmapEditor_H__
