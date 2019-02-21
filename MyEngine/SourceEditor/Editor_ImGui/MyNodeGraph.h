//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyNodeGraph_H__
#define __MyNodeGraph_H__

#include "../../Libraries/imgui/imgui.h"
#include "../../Libraries/imgui/imgui_internal.h"

class MyNodeGraph
{
    class Node;
    class NodeLink;

    friend class Node;
    friend class NodeLink;

protected:
    typedef uint32 NodeID;
    typedef uint32 SlotID;

    static const NodeID NodeID_Undefined = UINT_MAX;
    static const NodeID SlotID_Undefined = UINT_MAX;

    enum SlotType
    {
        SlotType_Input,
        SlotType_Output,
        SlotType_Undefined,
    };

    class MouseNodeLinkStartPoint
    {
    public:
        NodeID m_NodeID;
        SlotID m_SlotID;
        SlotType m_SlotType;
        ImU32 m_Color;

        void Set(NodeID nodeID, SlotID slotID, SlotType slotType) { m_NodeID = nodeID; m_SlotID = slotID; m_SlotType = slotType; }
        void Clear() { Set( NodeID_Undefined, SlotID_Undefined, SlotType_Undefined ); }
        bool InUse() { return m_NodeID != NodeID_Undefined; }
    };

protected:
    ImVector<Node> m_Nodes;
    ImVector<NodeLink> m_Links;
    ImVector<NodeID> m_SelectedNodeIDs;
    int m_SelectedNodeLinkIndex;

    Vector2 m_ScrollOffset;
    bool m_GridVisible;

    MouseNodeLinkStartPoint m_MouseNodeLinkStartPoint;

protected:
    void DrawGrid(Vector2 offset);
    int FindNodeIndexByID(NodeID nodeID);
    bool IsNodeSlotInUse(NodeID nodeID, SlotID slotID, SlotType slotType);
    void HandleNodeSlot(ImDrawList* pDrawList, Vector2 slotPos, NodeID nodeID, SlotID slotID, SlotType slotType);
    bool HandleNodeLinkCreation(Vector2 slotPos, NodeID nodeID, SlotID slotID, SlotType slotType);

public:
    MyNodeGraph();
    virtual ~MyNodeGraph();

    void Update();
};

#endif //__MyNodeGraph_H__
