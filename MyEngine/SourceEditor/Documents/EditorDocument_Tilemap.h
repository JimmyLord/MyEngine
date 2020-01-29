//
// Copyright (c) 2020 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorDocument_Tilemap_H__
#define __EditorDocument_Tilemap_H__

#include "../SourceEditor/Documents/EditorDocument.h"
#include "../SourceEditor/Prefs/EditorKeyBindings.h"

class ComponentTilemap;
class GameObject;

class EditorDocument_Tilemap : public EditorDocument
{
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
    ComponentTilemap* m_pTilemap;
    bool m_TilemapOwnedByUs;

    Tool m_CurrentTool;
    ToolState m_CurrentToolState;

    GameObject* m_pPoint;
    int m_IndexOfPointBeingDragged;
    Vector2 m_PositionMouseWentDown;
    bool m_NewMousePress;
    Vector2Int m_MouseTilePos;
    Vector3 m_WorldSpaceMousePosition;
    Vector3 m_WorldSpaceMousePositionWhenToolStarted;

    MaterialDefinition* m_pMaterials[Mat_NumMaterials];

    // Warnings.
    bool m_ShowWarning_CloseEditor;

    // Editor settings.
    int m_TileTypeSelected;

protected:
    // File IO.
    virtual const char* GetFileExtension() { return ".mytilemap"; };
    virtual const char* GetDefaultDataFolder() { return "Data\\Meshes\\"; };
    virtual const char* GetDefaultFileSaveFilter() { return "MyTilemap Files=*.mytilemap"; };

public:
    EditorDocument_Tilemap(EngineCore* pEngineCore, ComponentTilemap* pTilemap);
    virtual ~EditorDocument_Tilemap();

    virtual void Initialize();

    virtual bool IsBusy();

    virtual void OnDrawFrame() override;
    virtual void AddImGuiOverlayItems();

    Vector3 GetWorldSpaceMousePosition(Vector2 mousepos);

    // Input/Hotkey handling
    virtual HotkeyContext GetHotkeyContext() override { return HotkeyContext::TilemapEditor; }
    virtual bool HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure) override;
    virtual bool ExecuteHotkeyAction(HotkeyAction action) override;

    virtual void Update() override; // From EditorDocument.

    void CancelCurrentOperation();

    ComponentTilemap* GetTilemapBeingEdited() { return m_pTilemap; }
    void SetTilemap(ComponentTilemap* pTilemap);

    //MaterialDefinition* GetMaterial(MaterialTypes type);

    void ApplyCurrentTool(Vector3 mouseIntersectionPoint, int mouseAction);

protected:
    void Save();
    void Load();
};

#endif //__EditorDocument_Tilemap_H__
