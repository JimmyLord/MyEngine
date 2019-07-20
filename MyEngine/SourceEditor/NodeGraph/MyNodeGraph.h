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

#include "../SourceEditor/Documents/EditorDocument.h"
#include "../../Libraries/imgui/imgui.h"
#include "../../Libraries/imgui/imgui_internal.h"

class MyNodeTypeManager;

const ImU32 COLOR_BG = IM_COL32( 60, 60, 70, 200 );
const ImU32 COLOR_GRID = IM_COL32( 200, 200, 200, 40 );

const ImU32 COLOR_DRAG_SELECTOR = IM_COL32( 60, 200, 60, 75 );

const ImU32 COLOR_LINK_NORMAL = IM_COL32( 200, 200, 100, 255 );
const ImU32 COLOR_LINK_HIGHLIGHTED = IM_COL32( 100, 100, 200, 255 );
const ImU32 COLOR_LINK_SELECTED = IM_COL32( 0, 0, 255, 255 );

const ImU32 COLOR_LINK_IN_PROGRESS_DEFAULT = IM_COL32( 100, 100, 100, 255 );
const ImU32 COLOR_LINK_IN_PROGRESS_INVALID = IM_COL32( 200, 100, 100, 255 );
const ImU32 COLOR_LINK_IN_PROGRESS_VALID = IM_COL32( 100, 200, 100, 255 );

const ImU32 COLOR_SLOT_HOVERED = IM_COL32( 0, 255, 0, 255 );
const ImU32 COLOR_SLOT_DEFAULT = IM_COL32( 150, 150, 150, 255 );

const ImU32 COLOR_NODE_TRIM = IM_COL32( 0, 0, 0, 255 );
const ImU32 COLOR_NODE_BG_TITLE = IM_COL32( 60, 20, 150, 230 );
const ImU32 COLOR_NODE_BG = IM_COL32( 25, 0, 79, 230 );
const ImU32 COLOR_NODE_BG_SELECTED_BORDER = IM_COL32( 245, 142, 0, 128 );

class MyNodeGraph : public EditorDocument
{
public:
    class MyNode;
    class MyNodeLink;

    friend class MyNode;
    friend class MyNodeLink;
    friend class EditorCommand_NodeGraph_AddNode;
    friend class EditorCommand_NodeGraph_DeleteNodes;
    friend class EditorCommand_NodeGraph_CreateLink;
    friend class EditorCommand_NodeGraph_DeleteLink;

    typedef uint32 NodeID;

protected:
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

    class MyNodeGraph::MyNodeLink
    {
    public:
        NodeID m_OutputNodeID;
        SlotID m_OutputSlotID;
        NodeID m_InputNodeID;
        SlotID m_InputSlotID;

        MyNodeLink()
        {
            m_OutputNodeID = NodeID_Undefined;
            m_OutputSlotID = SlotID_Undefined;
            m_InputNodeID = NodeID_Undefined;
            m_InputSlotID = SlotID_Undefined;
        }
        MyNodeLink(NodeID outputNodeID, SlotID outputSlotID, NodeID inputNodeID, SlotID inputSlotID)
        {
            m_OutputNodeID = outputNodeID;
            m_OutputSlotID = outputSlotID;
            m_InputNodeID = inputNodeID;
            m_InputSlotID = inputSlotID;
        }
    };

protected:
    MyNodeTypeManager* m_pNodeTypeManager;

    NodeID m_NextNodeID;

    std::vector<MyNode*> m_Nodes;
    ImVector<MyNodeLink> m_Links;
    ImVector<NodeID> m_SelectedNodeIDs;
    int m_SelectedNodeLinkIndex;

    Vector2 m_ScrollOffset;
    bool m_GridVisible;

    MouseNodeLinkStartPoint m_MouseNodeLinkStartPoint;

    bool m_ShowingLuaString;
    const char* m_pLuaString;

protected:
    void Clear();

    void DrawGrid(Vector2 offset);
    int FindNodeIndexByID(NodeID nodeID);
    bool IsNodeSlotInUse(NodeID nodeID, SlotID slotID, SlotType slotType);
    void SetExpandedForAllSelectedNodes(bool expand);

    virtual void Save() override; // from EditorDocument.
    virtual void Load() override; // from EditorDocument.

    const char* ExportAsLuaString();
    cJSON* ExportAsJSONObject();
    void ImportFromJSONObject(cJSON* jNodeGraph);

    // Used by EditorCommand_NodeGraph_AddNode and EditorCommand_NodeGraph_DeleteNodes for undo/redo.
    void AddExistingNode(MyNode* pNode);
    void RemoveExistingNode(MyNode* pNode);

public:
    MyNodeGraph(EngineCore* pEngineCore, MyNodeTypeManager* pNodeTypeManager);
    virtual ~MyNodeGraph();

    // Returns true if in focus.
    virtual bool CreateWindowAndUpdate(bool* pDocumentStillOpen) override;
    virtual void Update() override;

    // Getters.
    MyNodeLink* FindLinkConnectedToInput(NodeID nodeID, SlotID slotID, int resultIndex = 0);
    MyNodeLink* FindLinkConnectedToOutput(NodeID nodeID, SlotID slotID, int resultIndex = 0);

    MyNode* FindNodeConnectedToInput(NodeID nodeID, SlotID slotID, int resultIndex = 0);
    MyNode* FindNodeConnectedToOutput(NodeID nodeID, SlotID slotID, int resultIndex = 0);

    NodeID GetNextNodeIDAndIncrement();
};

//====================================================================================================

class MyNodeTypeManager
{
protected:
    MyNodeGraph* m_pNodeGraph;

public:
    MyNodeTypeManager() {}

    virtual MyNodeGraph::MyNode* AddCreateNodeItemsToContextMenu(Vector2 pos, MyNodeGraph* pNodeGraph) = 0;
    virtual MyNodeGraph::MyNode* CreateNode(const char* typeName, Vector2 pos, MyNodeGraph* pNodeGraph) = 0;
};

#endif //__MyNodeGraph_H__
