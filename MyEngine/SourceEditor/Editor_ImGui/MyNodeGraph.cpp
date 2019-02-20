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

class Node
{
public:
    MyNodeGraph::NodeID m_ID;
    char m_Name[32];
    ImVec2 m_Pos;
    ImVec2 m_Size;
    int m_InputsCount;
    int m_OutputsCount;

    // Node properties.
    float m_Value;
    ImVec4 m_Color;

    Node(int id, const char* name, const ImVec2& pos, float value, const ImVec4& color, int inputsCount, int outputsCount)
    {
        m_ID = id;
        strncpy_s( m_Name, 32, name, 31 );
        m_Name[31] = '\0';
        m_Pos = pos;
        m_Value = value;
        m_Color = color;
        m_InputsCount = inputsCount;
        m_OutputsCount = outputsCount;
    }

    ImVec2 GetInputSlotPos(MyNodeGraph::SlotID slotID) const
    {
        return ImVec2( m_Pos.x, m_Pos.y + m_Size.y * ((float)slotID + 1) / ((float)m_InputsCount + 1) );
    }
    
    ImVec2 GetOutputSlotPos(MyNodeGraph::SlotID slotID) const
    {
        return ImVec2( m_Pos.x + m_Size.x, m_Pos.y + m_Size.y * ((float)slotID + 1) / ((float)m_OutputsCount + 1) );
    }
};

class NodeLink
{
public:
    MyNodeGraph::NodeID m_OutputNodeID;
    MyNodeGraph::SlotID m_OutputSlotID;
    MyNodeGraph::NodeID m_InputNodeID;
    MyNodeGraph::SlotID m_InputSlotID;

    NodeLink(MyNodeGraph::NodeID outputIndex, MyNodeGraph::SlotID outputSlotID, MyNodeGraph::NodeID inputNodeID, MyNodeGraph::SlotID inputSlotID)
    {
        m_OutputNodeID = inputNodeID;
        m_OutputSlotID = inputSlotID;
        m_InputNodeID = outputIndex;
        m_InputSlotID = outputSlotID;
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
    if( ( position.x < p0.x && position.x < p1.x ) ||
        ( position.x > p0.x && position.x > p1.x ) ||
        ( position.y < p0.y && position.y < p1.y ) ||
        ( position.y > p0.y && position.y > p1.y ) )
    {
        return false;
    }

    // More iterations based on distance, with some magic numbers.
    int iterations = (p0 - p1).LengthSquared() / 129600;
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
    m_SelectedNodeID = NodeID_Undefined;

    m_MouseNodeLinkStartPoint.Clear();

    m_Nodes.push_back( Node( 100, "MainTex", ImVec2(40, 50), 0.5f, ImColor(255, 100, 100), 1, 1 ) );
    m_Nodes.push_back( Node( 200, "BumpMap", ImVec2(40, 150), 0.42f, ImColor(200, 100, 200), 1, 1 ) );
    m_Nodes.push_back( Node( 300, "Combine", ImVec2(270, 80), 1.0f, ImColor(0, 200, 100), 2, 2 ) );
    //m_Links.push_back( NodeLink( 100, 0, 300, 0 ) );
    //m_Links.push_back( NodeLink( 200, 0, 300, 1 ) );
}

MyNodeGraph::~MyNodeGraph()
{
}

void MyNodeGraph::DrawGrid(Vector2 offset)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    if( m_GridVisible )
    {
        ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
        float GRID_SZ = 64.0f;
        Vector2 windowPos = ImGui::GetCursorScreenPos();
        Vector2 canvasSize = ImGui::GetWindowSize();
        for( float x = fmodf( m_ScrollOffset.x, GRID_SZ ); x < canvasSize.x; x += GRID_SZ )
        {
            draw_list->AddLine( Vector2(x, 0.0f) + windowPos, Vector2(x, canvasSize.y) + windowPos, GRID_COLOR );
        }
        for( float y = fmodf( m_ScrollOffset.y, GRID_SZ ); y < canvasSize.y; y += GRID_SZ )
        {
            draw_list->AddLine( Vector2(0.0f, y) + windowPos, Vector2(canvasSize.x, y) + windowPos, GRID_COLOR );
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

bool MyNodeGraph::HandleNodeLinkCreation(Vector2 slotPos, NodeID nodeID, SlotID slotID, SlotType slotType)
{
    Vector2 diff = ImGui::GetIO().MousePos - slotPos;
    
    if( diff.Length() > NODE_SLOT_COLLISION_RADIUS )
        return false; // Mouse is not over circle.

    // If mouse is clicked, then start a new link. // TODO: Have this take precedence over moving a node around with the mouse.
    if( ImGui::IsMouseClicked( 0 ) )
    {
        m_MouseNodeLinkStartPoint.Set( nodeID, slotID, slotType );
    }

    // Don't allow link to same node. TODO: Check for circlular links between multiple nodes.
    if( nodeID == m_MouseNodeLinkStartPoint.m_NodeID )
    {
        m_MouseNodeLinkStartPoint.m_Color = IM_COL32( 200, 100, 100, 255 );
    }
    else
    {
        if( m_MouseNodeLinkStartPoint.m_SlotType == SlotType_Input && slotType == SlotType_Output )
        {
            m_MouseNodeLinkStartPoint.m_Color = IM_COL32( 100, 200, 100, 255 );

            // If mouse is released, create a link.
            if( ImGui::IsMouseReleased( 0 ) )
            {
                m_Links.push_back( NodeLink( nodeID, slotID, m_MouseNodeLinkStartPoint.m_NodeID, m_MouseNodeLinkStartPoint.m_SlotID ) );
                LOGInfo( LOGTag, "Created Link" );
            }
        }
        else if( m_MouseNodeLinkStartPoint.m_SlotType == SlotType_Output && slotType == SlotType_Input )
        {
            m_MouseNodeLinkStartPoint.m_Color = IM_COL32( 100, 200, 100, 255 );

            // If mouse is released, create a link.
            if( ImGui::IsMouseReleased( 0 ) )
            {
                m_Links.push_back( NodeLink( m_MouseNodeLinkStartPoint.m_NodeID, m_MouseNodeLinkStartPoint.m_SlotID, nodeID, slotID ) );
                LOGInfo( LOGTag, "Created Link" );
            }
        }
        else
        {
            m_MouseNodeLinkStartPoint.m_Color = IM_COL32( 200, 100, 100, 255 );
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
        Node* node = &m_Nodes[nodeIndex];
        ImGui::PushID( node->m_ID );
        
        bool isSelected = false;
        isSelected = (node->m_ID == m_SelectedNodeID);
        if( ImGui::Selectable( node->m_Name, isSelected ) )
        {
            m_SelectedNodeID = node->m_ID;
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
    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2(1, 1) );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2(0, 0) );
    ImGui::PushStyleColor( ImGuiCol_ChildWindowBg, IM_COL32(60, 60, 70, 200) );
    ImGui::BeginChild( "scrolling region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove );
    ImGui::PushItemWidth( 120.0f );

    Vector2 offset = m_ScrollOffset + ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw grid.
    DrawGrid( offset );

    // Split draw into 2 channels (0 for background, 1 for foreground).
    drawList->ChannelsSplit( 2 );

    // Draw the links between nodes.
    {
        // Draw established links.
        drawList->ChannelsSetCurrent( 0 ); // Background.
        for( int linkIndex = 0; linkIndex < m_Links.Size; linkIndex++ )
        {
            NodeLink* pLink = &m_Links[linkIndex];
            Node* pOutputNode = &m_Nodes[FindNodeIndexByID(pLink->m_OutputNodeID)];
            Node* pInputNode = &m_Nodes[FindNodeIndexByID(pLink->m_InputNodeID)];
            Vector2 p1 = offset + pInputNode->GetOutputSlotPos( pLink->m_InputSlotID );
            Vector2 p2 = offset + pOutputNode->GetInputSlotPos( pLink->m_OutputSlotID );

            ImU32 color = IM_COL32(200, 200, 100, 255);

            if( IsNearBezierCurve( ImGui::GetIO().MousePos, p1, p1 + Vector2(+50, 0), p2 + Vector2(-50, 0), p2 ) )
            {
                color = IM_COL32(100, 100, 200, 255);
            }

            drawList->AddBezierCurve( p1, p1 + Vector2(+50, 0), p2 + Vector2(-50, 0), p2, color, 3.0f );
        }

        // Draw link being dragged by mouse.
        if( m_MouseNodeLinkStartPoint.m_NodeID != NodeID_Undefined )
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

            drawList->AddBezierCurve( p1, p1 + Vector2(+50, 0), p2 + Vector2(-50, 0), p2, m_MouseNodeLinkStartPoint.m_Color, 3.0f );

            m_MouseNodeLinkStartPoint.m_Color = IM_COL32(100, 100, 100, 255);
        }
    }

    // Draw nodes.
    {
        for( int nodeIndex = 0; nodeIndex < m_Nodes.Size; nodeIndex++ )
        {
            Node* pNode = &m_Nodes[nodeIndex];
            ImGui::PushID( pNode->m_ID );
            ImVec2 nodeRectMin = offset + pNode->m_Pos;

            // Display node contents first.
            drawList->ChannelsSetCurrent( 1 ); // Foreground.
            bool wasAnyActive = ImGui::IsAnyItemActive();
            ImGui::SetCursorScreenPos( nodeRectMin + NODE_WINDOW_PADDING );
        
            ImGui::BeginGroup(); // Lock horizontal position.
            ImGui::Text( "%s", pNode->m_Name );
            ImGui::SliderFloat( "##value", &pNode->m_Value, 0.0f, 1.0f, "Alpha %.2f" );
            ImGui::ColorEdit3( "##color", &pNode->m_Color.x );
            ImGui::EndGroup();

            // Save the size of what we have emitted and whether any of the widgets are being used.
            bool widgetOnNodeIsActive = (!wasAnyActive && ImGui::IsAnyItemActive());
            pNode->m_Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
            ImVec2 nodeRectMax = nodeRectMin + pNode->m_Size;

            // Display node box.
            drawList->ChannelsSetCurrent( 0 ); // Background.
            ImGui::SetCursorScreenPos(nodeRectMin);
            ImGui::InvisibleButton( "node", pNode->m_Size );
            if( ImGui::IsItemHovered() )
            {
                nodeIndexHoveredInScene = nodeIndex;
                openContextMenu |= ImGui::IsMouseClicked(1);
            }
            bool BackgroundOfNodeIsActive = ImGui::IsItemActive();
            if( widgetOnNodeIsActive || BackgroundOfNodeIsActive )
            {
                m_SelectedNodeID = pNode->m_ID;
            }
            if( BackgroundOfNodeIsActive && ImGui::IsMouseDragging(0) )
            {
                pNode->m_Pos = pNode->m_Pos + ImGui::GetIO().MouseDelta;
            }

            ImU32 nodeBGColor = ( nodeIndexHoveredInList == nodeIndex ||
                                  nodeIndexHoveredInScene == nodeIndex ||
                                  ( nodeIndexHoveredInList == -1 && m_SelectedNodeID == pNode->m_ID )
                                ) ? IM_COL32(75, 75, 75, 255) : IM_COL32(60, 60, 60, 255);
            drawList->AddRectFilled( nodeRectMin, nodeRectMax, nodeBGColor, 4.0f );
            drawList->AddRect( nodeRectMin, nodeRectMax, IM_COL32(100, 100, 100, 255), 4.0f );

            // Draw a circle for the link slots and check slots for mouse hover/click.
            for( int slotIndex = 0; slotIndex < pNode->m_OutputsCount; slotIndex++ )
            {
                Vector2 slotPos = offset + pNode->GetOutputSlotPos( slotIndex );
                ImU32 slotColor = IM_COL32( 150, 150, 150, 255 );

                // Check for collisions.
                if( HandleNodeLinkCreation( slotPos, m_Nodes[nodeIndex].m_ID, slotIndex, SlotType_Output ) )
                {
                    slotColor = m_MouseNodeLinkStartPoint.m_Color;
                }

                // Draw the circle.
                drawList->AddCircleFilled( slotPos, NODE_SLOT_RADIUS, slotColor );
            }

            for( int slotIndex = 0; slotIndex < pNode->m_InputsCount; slotIndex++ )
            {
                Vector2 slotPos = offset + pNode->GetInputSlotPos( slotIndex );
                ImU32 slotColor = IM_COL32( 150, 150, 150, 255 );

                // Check for collisions.
                if( HandleNodeLinkCreation( slotPos, m_Nodes[nodeIndex].m_ID, slotIndex, SlotType_Input ) )
                {
                    slotColor = IM_COL32( 0, 255, 0, 255 );
                }

                // Draw the circle.
                drawList->AddCircleFilled( slotPos, NODE_SLOT_RADIUS, slotColor );
            }

            ImGui::PopID();
        }
    }

    // Merge the draw list channels.
    drawList->ChannelsMerge();

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
            m_SelectedNodeID = NodeID_Undefined;
            nodeIndexHoveredInList = NodeID_Undefined;
            nodeIndexHoveredInScene = SlotID_Undefined;
            openContextMenu = true;
        }

        if( openContextMenu )
        {
            ImGui::OpenPopup("context menu");

            if( nodeIndexHoveredInList != NodeID_Undefined )
            {
                m_SelectedNodeID = nodeIndexHoveredInList;
            }
            if( nodeIndexHoveredInScene != SlotID_Undefined )
            {
                m_SelectedNodeID = nodeIndexHoveredInScene;
            }
        }

        // Draw context menu.
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2(8, 8) );
        if( ImGui::BeginPopup( "context menu" ) )
        {
            int nodeIndex = FindNodeIndexByID( m_SelectedNodeID );
            Node* pNode = m_SelectedNodeID != NodeID_Undefined ? &m_Nodes[nodeIndex] : NULL;
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
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    // Handle scrolling.
    {
        // Scroll view with middle mouse when undocked, also left mouse if docked and nothing is hovered.
        if( ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging( 2, 0.0f ) )
        {
            m_ScrollOffset = m_ScrollOffset + ImGui::GetIO().MouseDelta;
        }
    }

    // End of child canvas.
    ImGui::PopItemWidth();
    ImGui::EndChild(); // "scrolling region"
    ImGui::PopStyleColor(); // ImGuiCol_ChildWindowBg
    ImGui::PopStyleVar(2); // ImGuiStyleVar_FramePadding and ImGuiStyleVar_WindowPadding
    ImGui::EndGroup();
}
