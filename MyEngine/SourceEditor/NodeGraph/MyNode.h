//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyNode_H__
#define __MyNode_H__

#include "MyNodeGraph.h"
#include "../../SourceCommon/ComponentSystem/BaseComponents/ComponentVariable.h"

class MyNodeGraph::MyNode : public ComponentVariableCallbackInterface
{
public:
    enum class RenameState
    {
        Idle,
        Requested,
        BeingRenamed,
    };

public:
    MyNodeGraph* m_pNodeGraph;

    NodeID m_ID;
    char m_Name[32];
    Vector2 m_Pos;
    uint32 m_InputsCount;
    uint32 m_OutputsCount;
    bool m_Expanded;

    // Node properties.
    TCPPListHead<ComponentVariable*> m_VariablesList;

    // Temp values, no need to save.
    Vector2 m_Size;
    float m_TitleWidth;
    RenameState m_RenameState;
    bool m_IsBeingDragged;       // Used for undo/redo of node movement.
    Vector2 m_MouseDownPosition; // Used for undo/redo of node movement.

public:
    MyNode(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, int inputsCount, int outputsCount);
    virtual ~MyNode();

    virtual const char* GetType() = 0;

    virtual bool HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure) { return false; }

    MyNodeGraph* GetNodeGraph();

    uint32 MyNode::GetInputSlotCount() const;
    uint32 MyNode::GetOutputSlotCount() const;
    ImVec2 GetInputSlotPos(SlotID slotID) const;
    ImVec2 GetOutputSlotPos(SlotID slotID) const;
    NodeID GetID() { return m_ID; }

    void Draw(ImDrawList* pDrawList, Vector2 offset, bool isSelected, MouseNodeLinkStartPoint* pMouseNodeLink);
    void HandleNodeSlot(ImDrawList* pDrawList, Vector2 slotPos, NodeID nodeID, SlotID slotID, SlotType slotType, MouseNodeLinkStartPoint* pMouseNodeLink);
    bool HandleNodeLinkCreation(Vector2 slotPos, NodeID nodeID, SlotID slotID, SlotType slotType, MouseNodeLinkStartPoint* pMouseNodeLink);

    virtual void DrawTitle();
    virtual void DrawContents();

    virtual bool Trigger(MyEvent* pEvent = nullptr) { return false; }

    virtual uint32 ExportAsLuaVariablesString(char* string, uint32 offset, uint32 bytesAllocated) { return 0; }
    virtual uint32 ExportAsLuaString(char* string, uint32 offset, uint32 bytesAllocated) { return 0; }
    virtual cJSON* ExportAsJSONObject();
    virtual void ImportFromJSONObject(cJSON* jNode);
};

#endif //__MyNode_H__
