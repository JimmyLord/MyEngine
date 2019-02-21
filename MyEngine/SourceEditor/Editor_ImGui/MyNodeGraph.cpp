//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "MyNodeGraph.h"

// Based on this: https://gist.github.com/ocornut/7e9b3ec566a333d725d4
//   but rewritten and expanded.

const float NODE_SLOT_RADIUS = 4.0f;
const float NODE_SLOT_COLLISION_RADIUS = 6.0f;
const Vector2 NODE_WINDOW_PADDING( 8.0f, 8.0f );

const ImU32 COLOR_BG = IM_COL32( 60, 60, 70, 200 );
const ImU32 COLOR_GRID = IM_COL32( 200, 200, 200, 40 );

const ImU32 COLOR_LINK_NORMAL = IM_COL32( 200, 200, 100, 255 );
const ImU32 COLOR_LINK_HIGHLIGHTED = IM_COL32( 100, 100, 200, 255 );
const ImU32 COLOR_LINK_SELECTED = IM_COL32( 0, 0, 255, 255 );

const ImU32 COLOR_LINK_IN_PROGRESS_DEFAULT = IM_COL32( 100, 100, 100, 255 );
const ImU32 COLOR_LINK_IN_PROGRESS_INVALID = IM_COL32( 200, 100, 100, 255 );
const ImU32 COLOR_LINK_IN_PROGRESS_VALID = IM_COL32( 100, 200, 100, 255 );

const ImU32 COLOR_SLOT_HOVERED = IM_COL32( 0, 255, 0, 255 );
const ImU32 COLOR_SLOT_DEFAULT = IM_COL32( 150, 150, 150, 255 );

const ImU32 COLOR_NODE_TRIM = IM_COL32( 0, 0, 0, 255 );
const ImU32 COLOR_NODE_BG_TITLE_DEFAULT = IM_COL32( 60, 20, 150, 230 );
const ImU32 COLOR_NODE_BG_TITLE_HOVERED = IM_COL32( 60, 20, 150, 230 );
const ImU32 COLOR_NODE_BG_DEFAULT = IM_COL32( 25, 0, 79, 230 );
const ImU32 COLOR_NODE_BG_HOVERED = IM_COL32( 25, 0, 79, 230 );
const ImU32 COLOR_NODE_BG_SELECTED_BORDER = IM_COL32( 245, 142, 0, 128 );

class MyNodeGraph::Node
{
public:
    NodeID m_ID;
    char m_Name[32];
    Vector2 m_Pos;
    Vector2 m_Size;
    float m_TitleWidth;
    int m_InputsCount;
    int m_OutputsCount;
    bool m_Expanded;

    // Node properties.
    float m_Value;
    ImVec4 m_Color;

    Node(int id, const char* name, const Vector2& pos, float value, const ImVec4& color, int inputsCount, int outputsCount)
    {
        m_ID = id;
        strncpy_s( m_Name, 32, name, 31 );
        m_Name[31] = '\0';
        m_Pos = pos;
        m_Size.Set( 0, 0 );
        m_TitleWidth = 1; // Initially set to a small width, will expand based on controls.
        m_InputsCount = inputsCount;
        m_OutputsCount = outputsCount;
        m_Expanded = true;

        // Node properties.
        m_Value = value;
        m_Color = color;
    }

    ImVec2 GetInputSlotPos(SlotID slotID) const
    {
        return ImVec2( m_Pos.x, m_Pos.y + m_Size.y * ((float)slotID + 1) / ((float)m_InputsCount + 1) );
    }
    
    ImVec2 GetOutputSlotPos(SlotID slotID) const
    {
        return ImVec2( m_Pos.x + m_Size.x, m_Pos.y + m_Size.y * ((float)slotID + 1) / ((float)m_OutputsCount + 1) );
    }
};

class MyNodeGraph::NodeLink
{
public:
    NodeID m_OutputNodeID;
    SlotID m_OutputSlotID;
    NodeID m_InputNodeID;
    SlotID m_InputSlotID;

    NodeLink(NodeID outputNodeID, SlotID outputSlotID, NodeID inputNodeID, SlotID inputSlotID)
    {
        m_OutputNodeID = outputNodeID;
        m_OutputSlotID = outputSlotID;
        m_InputNodeID = inputNodeID;
        m_InputSlotID = inputSlotID;
    }
};

float GetClosestPointToCubicBezier(int iterations, float fx, float fy, float start, float end, int slices, const ImVec2& P0, const ImVec2& P1, const ImVec2& P2, const ImVec2& P3) 
{
    if( iterations <= 0 )
        return (start + end) / 2;

    float tick = (end - start) / float(slices);
    float best = 0;
    float bestDistance = FLT_MAX;
    float t = start;
        
    while( t <= end )
    {
        // B(t) = (1-t)**3 p0 + 3(1 - t)**2 t P1 + 3(1-t)t**2 P2 + t**3 P3
        float x = (1-t)*(1-t)*(1-t)*P0.x + 3*(1-t)*(1-t)*t*P1.x + 3*(1-t)*t*t*P2.x + t*t*t*P3.x;
        float y = (1-t)*(1-t)*(1-t)*P0.y + 3*(1-t)*(1-t)*t*P1.y + 3*(1-t)*t*t*P2.y + t*t*t*P3.y;
        float dx = x - fx;
        float dy = y - fy;
        float currentDistance = dx*dx + dy*dy;
        if( currentDistance < bestDistance )
        {
            bestDistance = currentDistance;
            best = t;
        }
        t += tick;
    }

    return GetClosestPointToCubicBezier( iterations - 1, fx, fy, (best-tick)<0 ? 0.0f : (best-tick), (best+tick)>1 ? 1.0f : (best+tick), slices, P0, P1, P2, P3 );
}

ImVec2 GetClosestPointToCubicBezier(const ImVec2 & pos, const ImVec2 & P0, const ImVec2 & P1, const ImVec2 & P2, const ImVec2 & P3, int slices, int iterations)
{
    float t = GetClosestPointToCubicBezier( iterations, pos.x, pos.y, 0.0, 1.0, slices, P0, P1, P2, P3 );
    float x = (1-t)*(1-t)*(1-t)*P0.x + 3*(1-t)*(1-t)*t*P1.x + 3*(1-t)*t*t*P2.x + t*t*t*P3.x;
    float y = (1-t)*(1-t)*(1-t)*P0.y + 3*(1-t)*(1-t)*t*P1.y + 3*(1-t)*t*t*P2.y + t*t*t*P3.y;
    return ImVec2(x,y);
}

bool IsNearBezierCurve(Vector2 position, Vector2 p0, Vector2 cp0, Vector2 cp1, Vector2 p1)
{
    // Bounds check.
    if( ( position.x < p0.x - 4 && position.x < p1.x - 4 ) ||
        ( position.x > p0.x + 4 && position.x > p1.x + 4 ) ||
        ( position.y < p0.y - 4 && position.y < p1.y - 4 ) ||
        ( position.y > p0.y + 4 && position.y > p1.y + 4 ) )
    {
        return false;
    }

    // More iterations based on distance, with some magic numbers.
    int iterations = (int)( (p0 - p1).LengthSquared() / 129600 );
    if( iterations < 4 )
        iterations = 4;

    Vector2 pos = GetClosestPointToCubicBezier( position, p0, cp0, cp1, p1, 4, iterations );
    if( (position - pos).LengthSquared() < 6*6 )
        return true;

    return false;
}

MyNodeGraph::MyNodeGraph()
{
    m_ScrollOffset.Set( 0.0f, 0.0f );
    m_GridVisible = true;
    m_SelectedNodeLinkIndex = -1;

    m_MouseNodeLinkStartPoint.Clear();

    m_Nodes.push_back( Node( 100, "MainTex", ImVec2(40, 50), 0.5f, ImColor(255, 100, 100), 1, 1 ) );
    m_Nodes.push_back( Node( 200, "BumpMap", ImVec2(40, 150), 0.42f, ImColor(200, 100, 200), 1, 1 ) );
    m_Nodes.push_back( Node( 300, "Combine", ImVec2(270, 80), 1.0f, ImColor(0, 200, 100), 2, 2 ) );
    m_Links.push_back( NodeLink( 100, 0, 300, 1 ) );
    m_Links.push_back( NodeLink( 200, 0, 300, 1 ) );
}

MyNodeGraph::~MyNodeGraph()
{
}

void MyNodeGraph::DrawGrid(Vector2 offset)
{
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    
    if( m_GridVisible )
    {
        float GRID_SZ = 64.0f;
        Vector2 windowPos = ImGui::GetCursorScreenPos();
        Vector2 canvasSize = ImGui::GetWindowSize();
        for( float x = fmodf( m_ScrollOffset.x, GRID_SZ ); x < canvasSize.x; x += GRID_SZ )
        {
            pDrawList->AddLine( Vector2(x, 0.0f) + windowPos, Vector2(x, canvasSize.y) + windowPos, COLOR_GRID );
        }
        for( float y = fmodf( m_ScrollOffset.y, GRID_SZ ); y < canvasSize.y; y += GRID_SZ )
        {
            pDrawList->AddLine( Vector2(0.0f, y) + windowPos, Vector2(canvasSize.x, y) + windowPos, COLOR_GRID );
        }
    }
}

int MyNodeGraph::FindNodeIndexByID(NodeID nodeID)
{
    for( int nodeIndex = 0; nodeIndex < m_Nodes.Size; nodeIndex++ )
    {
        if( m_Nodes[nodeIndex].m_ID == nodeID )
            return nodeIndex;
    }

    return -1;
}

bool MyNodeGraph::IsNodeSlotInUse(NodeID nodeID, SlotID slotID, SlotType slotType)
{
    for( int linkIndex = 0; linkIndex < m_Links.Size; linkIndex++ )
    {
        if( ( slotType == SlotType_Input &&
              m_Links[linkIndex].m_InputNodeID == nodeID &&
              m_Links[linkIndex].m_InputSlotID == slotID ) ||
            ( slotType == SlotType_Output &&
              m_Links[linkIndex].m_OutputNodeID == nodeID &&
              m_Links[linkIndex].m_OutputSlotID == slotID ) )
        {
            return true;
        }
    }

    return false;
}

void MyNodeGraph::HandleNodeSlot(ImDrawList* pDrawList, Vector2 slotPos, NodeID nodeID, SlotID slotID, SlotType slotType)
{
    ImU32 slotColor = COLOR_SLOT_DEFAULT;

    // Check for collisions.
    if( HandleNodeLinkCreation( slotPos, nodeID, slotID, slotType ) )
    {
        slotColor = m_MouseNodeLinkStartPoint.m_Color;
    }

    // Draw the circle.
    if( IsNodeSlotInUse( nodeID, slotID, slotType ) )
    {
        pDrawList->AddCircleFilled( slotPos, NODE_SLOT_RADIUS, slotColor );
    }
    else
    {
        pDrawList->AddCircle( slotPos, NODE_SLOT_RADIUS, slotColor, 12, 2 );
    }
}

bool MyNodeGraph::HandleNodeLinkCreation(Vector2 slotPos, NodeID nodeID, SlotID slotID, SlotType slotType)
{
    Vector2 diff = ImGui::GetIO().MousePos - slotPos;
    
    // Check if mouse is not over circle.
    if( diff.Length() > NODE_SLOT_COLLISION_RADIUS )
        return false;

    // If mouse is clicked, then start a new link. // TODO: Have this take precedence over moving a node around with the mouse.
    if( ImGui::IsMouseClicked( 0 ) )
    {
        m_MouseNodeLinkStartPoint.Set( nodeID, slotID, slotType );
    }

    if( slotType == SlotType_Input )
        int a= 1;

    // Don't allow link to same node. TODO: Check for circlular links between multiple nodes.
    if( nodeID == m_MouseNodeLinkStartPoint.m_NodeID )
    {
        m_MouseNodeLinkStartPoint.m_Color = COLOR_LINK_IN_PROGRESS_INVALID;
    }
    else if( m_MouseNodeLinkStartPoint.m_SlotType == SlotType_Undefined )
    {
        m_MouseNodeLinkStartPoint.m_Color = COLOR_SLOT_HOVERED;
    }
    else
    {
        if( m_MouseNodeLinkStartPoint.m_SlotType == SlotType_Input && slotType == SlotType_Output )
        {
            m_MouseNodeLinkStartPoint.m_Color = COLOR_LINK_IN_PROGRESS_VALID;

            // If mouse is released, create a link.
            if( ImGui::IsMouseReleased( 0 ) )
            {
                m_Links.push_back( NodeLink( nodeID, slotID, m_MouseNodeLinkStartPoint.m_NodeID, m_MouseNodeLinkStartPoint.m_SlotID ) );
            }
        }
        else if( m_MouseNodeLinkStartPoint.m_SlotType == SlotType_Output && slotType == SlotType_Input )
        {
            m_MouseNodeLinkStartPoint.m_Color = COLOR_LINK_IN_PROGRESS_VALID;

            // If mouse is released, create a link.
            if( ImGui::IsMouseReleased( 0 ) )
            {
                m_Links.push_back( NodeLink( m_MouseNodeLinkStartPoint.m_NodeID, m_MouseNodeLinkStartPoint.m_SlotID, nodeID, slotID ) );
            }
        }
        else
        {
            m_MouseNodeLinkStartPoint.m_Color = COLOR_LINK_IN_PROGRESS_INVALID;
        }
    }

    return true; // Mouse is over circle.
}

void MyNodeGraph::Update()
{
    bool openContextMenu = false;
    int nodeIndexHoveredInList = -1;
    int nodeIndexHoveredInScene = -1;
    
    // Draw a list of nodes on the left side.
    ImGui::BeginChild( "node list", ImVec2(100, 0) );
    ImGui::Text( "Nodes" );
    ImGui::Separator();
    for( int nodeIndex = 0; nodeIndex < m_Nodes.Size; nodeIndex++ )
    {
        Node* pNode = &m_Nodes[nodeIndex];
        ImGui::PushID( pNode->m_ID );
        
        bool isSelected = false;
        isSelected = m_SelectedNodeIDs.contains( pNode->m_ID );
        if( ImGui::Selectable( pNode->m_Name, isSelected ) )
        {
            m_SelectedNodeIDs.clear(); // TODO: Handle control/shift to multi-select.
            m_SelectedNodeIDs.push_back( pNode->m_ID );
        }

        if( ImGui::IsItemHovered() )
        {
            nodeIndexHoveredInList = nodeIndex;
            openContextMenu |= ImGui::IsMouseClicked( 1 );
        }

        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginGroup();

    // Create a child canvas for the node graph.
    ImGui::Text( "Hold middle mouse button to scroll (%.2f,%.2f)", m_ScrollOffset.x, m_ScrollOffset.y );
    ImGui::SameLine( ImGui::GetWindowWidth() - 300 );
    ImGui::Checkbox( "Show grid", &m_GridVisible );
    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 1, 1 ) );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );
    ImGui::PushStyleColor( ImGuiCol_ChildWindowBg, COLOR_BG );
    ImGui::BeginChild( "scrolling region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove );
    ImGui::PushItemWidth( 120.0f );

    Vector2 offset = m_ScrollOffset + ImGui::GetCursorScreenPos();
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();

    // Draw grid.
    DrawGrid( offset );

    // Split draw into 2 channels (0 for background, 1 for foreground).
    pDrawList->ChannelsSplit( 2 );

    int nodeLinkIndexHoveredInScene = -1;

    // Draw the links between nodes.
    {
        // Draw established links.
        pDrawList->ChannelsSetCurrent( 0 ); // Background.
        for( int linkIndex = 0; linkIndex < m_Links.Size; linkIndex++ )
        {
            NodeLink* pLink = &m_Links[linkIndex];
            Node* pOutputNode = &m_Nodes[FindNodeIndexByID(pLink->m_OutputNodeID)];
            Node* pInputNode = &m_Nodes[FindNodeIndexByID(pLink->m_InputNodeID)];
            Vector2 p1 = offset + pOutputNode->GetOutputSlotPos( pLink->m_OutputSlotID );
            Vector2 p2 = offset + pInputNode->GetInputSlotPos( pLink->m_InputSlotID );

            ImU32 color = COLOR_LINK_NORMAL;

            if( nodeLinkIndexHoveredInScene == -1 && IsNearBezierCurve( ImGui::GetIO().MousePos, p1, p1 + Vector2(+50, 0), p2 + Vector2(-50, 0), p2 ) )
            {
                nodeLinkIndexHoveredInScene = linkIndex;
                color = COLOR_LINK_HIGHLIGHTED;
            }

            pDrawList->AddBezierCurve( p1, p1 + Vector2(+50, 0), p2 + Vector2(-50, 0), p2, color, 3.0f );
        }

        // Draw link being dragged by mouse.
        if( m_MouseNodeLinkStartPoint.InUse() )
        {
            Vector2 p1, p2;

            int inputNodeIndex = FindNodeIndexByID( m_MouseNodeLinkStartPoint.m_NodeID );
            Node* pInputNode = &m_Nodes[inputNodeIndex];
            
            if( m_MouseNodeLinkStartPoint.m_SlotType == SlotType_Input )
            {
                p1 = ImGui::GetIO().MousePos;
                p2 = offset + pInputNode->GetInputSlotPos( m_MouseNodeLinkStartPoint.m_SlotID );
            }
            else
            {
                p1 = offset + pInputNode->GetOutputSlotPos( m_MouseNodeLinkStartPoint.m_SlotID );
                p2 = ImGui::GetIO().MousePos;
            }

            pDrawList->AddBezierCurve( p1, p1 + Vector2(+50, 0), p2 + Vector2(-50, 0), p2, m_MouseNodeLinkStartPoint.m_Color, 3.0f );

            m_MouseNodeLinkStartPoint.m_Color = COLOR_LINK_IN_PROGRESS_DEFAULT;
        }
    }

    // Draw nodes.
    {
        for( int nodeIndex = 0; nodeIndex < m_Nodes.Size; nodeIndex++ )
        {
            Node* pNode = &m_Nodes[nodeIndex];
            ImGui::PushID( pNode->m_ID );
            ImVec2 nodeRectMin = offset + pNode->m_Pos;

            bool wasAnyActive = ImGui::IsAnyItemActive();
            bool backgroundOfNodeIsActive = false;
            float titleHeight = 28.0f;
            bool titleOfNodeIsActive = false;

            // Display node contents first to establish a size for the background.
            {
                pDrawList->ChannelsSetCurrent( 1 ); // Foreground.
                ImGui::SetCursorScreenPos( nodeRectMin + NODE_WINDOW_PADDING );
        
                // Draw the node contents as a group to lock horizontal position.
                ImGui::BeginGroup();
            
                // Add the node's name along with an arrow to collapse/expand it.
                ImGui::SetCursorScreenPos( nodeRectMin + NODE_WINDOW_PADDING );
                if( ImGui::ArrowButton( "", pNode->m_Expanded ? ImGuiDir_Down : ImGuiDir_Right ) )
                {
                    pNode->m_Expanded = !pNode->m_Expanded;
                }
                ImGui::SameLine();
                ImGui::Text( pNode->m_Name );

                // Add an invisible button, so entire title area can be double-clicked to collapse/expand.
                ImGui::SetCursorScreenPos( nodeRectMin );
                ImGui::InvisibleButton( "", ImVec2( pNode->m_TitleWidth + NODE_WINDOW_PADDING.x, titleHeight ) );
                titleOfNodeIsActive = ImGui::IsItemActive();
                if( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( 0 ) )
                {
                    pNode->m_Expanded = !pNode->m_Expanded;
                }

                // If expanded, add this node's controls.
                if( pNode->m_Expanded )
                {
                    ImGui::SetCursorScreenPos( nodeRectMin + NODE_WINDOW_PADDING + ImVec2( 0, titleHeight ) );
                    ImGui::SliderFloat( "##value", &pNode->m_Value, 0.0f, 1.0f, "Alpha %.2f" );
                    ImGui::ColorEdit3( "##color", &pNode->m_Color.x );
                }
                else
                {
                    // Otherwise, pad out the space, so size doesn't change.
                    ImGui::SetCursorScreenPos( nodeRectMin + ImVec2( pNode->m_Size.x, titleHeight ) + NODE_WINDOW_PADDING );
                }

                ImGui::EndGroup();
            }

            // Save the size of what we have emitted and whether any of the widgets are being used.
            bool widgetOnNodeIsActive = (!wasAnyActive && ImGui::IsAnyItemActive());
            if( pNode->m_Expanded )
            {
                pNode->m_TitleWidth = ImGui::GetItemRectSize().x == 0 ? 1 : ImGui::GetItemRectSize().x;
                pNode->m_Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
            }
            else
            {
                pNode->m_Size = ImGui::GetItemRectSize();
            }

            // Display node background.
            {
                ImVec2 nodeRectMidLeft = ImVec2( nodeRectMin.x, nodeRectMin.y + titleHeight );
                ImVec2 nodeRectMidRight = ImVec2( nodeRectMin.x + pNode->m_Size.x, nodeRectMin.y + titleHeight );
                ImVec2 nodeRectMax = nodeRectMin + pNode->m_Size;

                pDrawList->ChannelsSetCurrent( 0 ); // Background.
                ImGui::SetCursorScreenPos( nodeRectMin );
                ImGui::InvisibleButton( "node", pNode->m_Size );
                if( ImGui::IsItemHovered() )
                {
                    nodeIndexHoveredInScene = nodeIndex;
                    openContextMenu |= ImGui::IsMouseClicked(1);
                }
                backgroundOfNodeIsActive = ImGui::IsItemActive();
                if( widgetOnNodeIsActive || backgroundOfNodeIsActive )
                {
                    m_SelectedNodeIDs.clear(); // TODO: Handle holding control key to multi-select.
                    m_SelectedNodeIDs.push_back( pNode->m_ID );
                }

                bool hovered = ( nodeIndexHoveredInList == nodeIndex ||
                                 nodeIndexHoveredInScene == nodeIndex );
                ImU32 nodeBGColor = hovered ? COLOR_NODE_BG_HOVERED : COLOR_NODE_BG_DEFAULT;
                ImU32 nodeTitleColor = hovered ? COLOR_NODE_BG_TITLE_HOVERED : COLOR_NODE_BG_TITLE_DEFAULT;
            
                ImVec2 thickness = ImVec2( 3, 3 );
                if( m_SelectedNodeIDs.contains( pNode->m_ID ) )
                {
                    pDrawList->AddRectFilled( nodeRectMin - thickness, nodeRectMax + thickness, COLOR_NODE_BG_SELECTED_BORDER, 4.0f );
                }

                if( pNode->m_Expanded )
                {
                    pDrawList->AddRectFilled( nodeRectMin, nodeRectMidRight, nodeTitleColor, 4.0f, 1<<0 | 1<<1 );
                    pDrawList->AddRectFilled( nodeRectMidLeft, nodeRectMax, nodeBGColor, 4.0f, 1<<2 | 1<<3 );
                }
                else
                {
                    pDrawList->AddRectFilled( nodeRectMin, nodeRectMax, nodeTitleColor, 4.0f );
                }
                pDrawList->AddRect( nodeRectMin, nodeRectMax, COLOR_NODE_TRIM, 4.0f );
            }

            // Draw a circle for each link slot and check slots for mouse hover/click.
            {
                for( int slotIndex = 0; slotIndex < pNode->m_OutputsCount; slotIndex++ )
                {
                    Vector2 slotPos = offset + pNode->GetOutputSlotPos( slotIndex );
                    HandleNodeSlot( pDrawList, slotPos, pNode->m_ID, slotIndex, SlotType_Output );
                }

                for( int slotIndex = 0; slotIndex < pNode->m_InputsCount; slotIndex++ )
                {
                    Vector2 slotPos = offset + pNode->GetInputSlotPos( slotIndex );
                    HandleNodeSlot( pDrawList, slotPos, pNode->m_ID, slotIndex, SlotType_Input );
                }
            }

            // Move window if backgroundOfNodeIsActive and we're not creating a link.
            if( (backgroundOfNodeIsActive || titleOfNodeIsActive) && ImGui::IsMouseDragging(0) && m_MouseNodeLinkStartPoint.InUse() == false )
            {
                pNode->m_Pos = pNode->m_Pos + ImGui::GetIO().MouseDelta;
            }

            ImGui::PopID();
        }
    }

    // Merge the draw list channels.
    pDrawList->ChannelsMerge();

    // Clear the temporary link when the mouse is released.
    if( ImGui::IsMouseReleased( 0 ) )
    {
        m_MouseNodeLinkStartPoint.Clear();
    }

    // Deal with context menu.
    {
        // Open context menu.
        if( !ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked( 1 ) )
        {
            m_SelectedNodeIDs.clear();
            m_SelectedNodeLinkIndex = -1;
            nodeIndexHoveredInList = NodeID_Undefined;
            nodeIndexHoveredInScene = SlotID_Undefined;
            openContextMenu = true;
        }

        if( openContextMenu )
        {
            ImGui::OpenPopup("context menu");

            if( nodeLinkIndexHoveredInScene != -1 )
            {
                m_SelectedNodeLinkIndex = nodeLinkIndexHoveredInScene;
            }
            else if( nodeIndexHoveredInList != -1 )
            {
                m_SelectedNodeIDs.clear();
                m_SelectedNodeLinkIndex = -1;
                m_SelectedNodeIDs.push_back( m_Nodes[nodeIndexHoveredInList].m_ID );
            }
            else if( nodeIndexHoveredInScene != -1 )
            {
                m_SelectedNodeIDs.clear();
                m_SelectedNodeLinkIndex = -1;
                m_SelectedNodeIDs.push_back( m_Nodes[nodeIndexHoveredInScene].m_ID );
            }
        }

        // Draw context menu.
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2(8, 8) );
        if( ImGui::BeginPopup( "context menu" ) )
        {
            if( m_SelectedNodeLinkIndex != -1 )
            {
                // Remove the link.
                if( ImGui::MenuItem( "Remove" ) )
                {
                    m_Links.erase_unsorted( m_Links.Data + m_SelectedNodeLinkIndex );
                }
            }
            else 
            {
                Node* pNode = nullptr;
            
                int nodeIndex = m_SelectedNodeIDs.size() > 0 ? FindNodeIndexByID( m_SelectedNodeIDs[0] ) : -1;
                if( nodeIndex != -1 )
                    pNode = &m_Nodes[nodeIndex];
            
                Vector2 scenePos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
                if( pNode )
                {
                    ImGui::Text( "Node '%s'", pNode->m_Name );
                    ImGui::Separator();
                    if( ImGui::MenuItem( "Rename...", nullptr, false, false ) ) {}
                    if( ImGui::MenuItem( "Delete", nullptr, false, false ) ) {}
                    if( ImGui::MenuItem( "Copy", nullptr, false, false ) ) {}
                }
                else
                {
                    if( ImGui::MenuItem( "Add" ) )
                    {
                        m_Nodes.push_back( Node( m_Nodes.Size, "New node", scenePos, 0.5f, ImColor(100, 100, 200), 2, 2 ) );
                    }
                    if( ImGui::MenuItem( "Paste", nullptr, false, false ) ) {}
                }
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    // Handle scrolling.
    {
        // If the node graph area is hovered, but no items are active:
        if( ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() )
        {
            // Scroll view with middle mouse.
            if( ImGui::IsMouseDragging( 2, 0.0f ) )
            {
                m_ScrollOffset = m_ScrollOffset + ImGui::GetIO().MouseDelta;
            }

            // Clear selected nodes if left-mouse is pressed and we're not dragging a new link.
            if( ImGui::IsMouseClicked( 0 ) && m_MouseNodeLinkStartPoint.InUse() == false )
            {
                m_SelectedNodeIDs.clear();
            }
        }
    }

    // End of child canvas.
    ImGui::PopItemWidth();
    ImGui::EndChild(); // "scrolling region"
    ImGui::PopStyleColor(); // ImGuiCol_ChildWindowBg
    ImGui::PopStyleVar(2); // ImGuiStyleVar_FramePadding and ImGuiStyleVar_WindowPadding
    ImGui::EndGroup();
}
