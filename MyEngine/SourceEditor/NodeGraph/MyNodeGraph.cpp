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
#include "MyNode.h"
#include "NodeGraphEditorCommands.h"
#include "../../SourceCommon/Core/EngineCore.h"

// Based on this: https://gist.github.com/ocornut/7e9b3ec566a333d725d4
//   but rewritten and expanded.

float GetClosestPointToCubicBezier(int iterations, float fx, float fy, float start, float end, int slices, const ImVec2& P0, const ImVec2& P1, const ImVec2& P2, const ImVec2& P3) 
{
    if( iterations <= 0 )
        return (start + end) / 2;

    if( fequal( start, end ) )
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
    int iterations = static_cast<int>( (p0 - p1).LengthSquared() / 129600 );
    if( iterations < 4 )
        iterations = 4;

    Vector2 pos = GetClosestPointToCubicBezier( position, p0, cp0, cp1, p1, 4, iterations );
    if( (position - pos).LengthSquared() < 6*6 )
        return true;

    return false;
}

MyNodeGraph::MyNodeGraph(EngineCore* pEngineCore, MyNodeTypeManager* pNodeTypeManager)
: EditorDocument( pEngineCore )
{
    m_NextNodeID = 0;

    m_pNodeTypeManager = pNodeTypeManager;

    m_ScrollOffset.Set( 0.0f, 0.0f );
    m_GridVisible = true;
    m_SelectedNodeLinkIndexes.clear();

    m_MouseNodeLinkStartPoint.Clear();

    m_ShowingLuaString = false;
    m_pLuaString = nullptr;

    //m_Nodes.push_back( m_pNodeTypeManager->CreateNode( "Float", Vector2(40, 50), this ) );
    //m_Nodes.push_back( m_pNodeTypeManager->CreateNode( "Color", Vector2(40, 150), this ) );
    //m_Nodes.push_back( m_pNodeTypeManager->CreateNode( "GameObject", Vector2(40, 250), this ) );
    //m_Nodes.push_back( m_pNodeTypeManager->CreateNode( "Add", Vector2(270, 80), this ) );
    //m_Nodes.push_back( MyNew MyNode( this, 100, "MainTex", ImVec2(40, 50), 0.5f, ColorByte(255, 100, 100, 255), 1, 1 ) );
    //m_Nodes.push_back( MyNew MyNode( this, 200, "BumpMap", ImVec2(40, 150), 0.42f, ColorByte(200, 100, 200, 255), 1, 1 ) );
    //m_Nodes.push_back( MyNew MyNode( this, 300, "Combine", ImVec2(270, 80), 1.0f, ColorByte(0, 200, 100, 255), 2, 2 ) );
    //m_Links.push_back( MyNodeLink( 100, 0, 300, 0 ) );
    //m_Links.push_back( MyNodeLink( 200, 0, 300, 1 ) );
}

MyNodeGraph::~MyNodeGraph()
{
    Clear();

    delete[] m_pLuaString;
}

void MyNodeGraph::Clear()
{
    for( uint32 nodeIndex = 0; nodeIndex < m_Nodes.size(); nodeIndex++ )
    {
        delete m_Nodes[nodeIndex];
    }
    m_Nodes.clear();
    m_Links.clear();

    m_SelectedNodeIDs.clear();
    m_SelectedNodeLinkIndexes.clear();
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
    for( uint32 nodeIndex = 0; nodeIndex < m_Nodes.size(); nodeIndex++ )
    {
        if( m_Nodes[nodeIndex]->m_ID == nodeID )
            return nodeIndex;
    }

    return -1;
}

bool MyNodeGraph::IsNodeLinkSelected(int nodeLinkIndex)
{
    for( int i = 0; i < m_SelectedNodeLinkIndexes.size(); i++ )
    {
        if( m_SelectedNodeLinkIndexes[i] == nodeLinkIndex )
            return true;
    }

    return false;
}

MyNodeGraph::MyNodeLink* MyNodeGraph::FindLinkConnectedToInput(NodeID nodeID, SlotID slotID, int resultIndex)
{
    int count = 0;
    for( int i=0; i<m_Links.size(); i++ )
    {
        if( m_Links[i].m_InputNodeID == nodeID && m_Links[i].m_InputSlotID == slotID )
        {
            if( count == resultIndex )
                return &m_Links[i];

            count++;
        }
    }

    return nullptr;
}

MyNodeGraph::MyNodeLink* MyNodeGraph::FindLinkConnectedToOutput(NodeID nodeID, SlotID slotID, int resultIndex)
{
    int count = 0;
    for( int i=0; i<m_Links.size(); i++ )
    {
        if( m_Links[i].m_OutputNodeID == nodeID && m_Links[i].m_OutputSlotID == slotID )
        {
            if( count == resultIndex )
                return &m_Links[i];

            count++;
        }
    }

    return nullptr;
}

MyNodeGraph::MyNode* MyNodeGraph::FindNodeConnectedToInput(NodeID nodeID, SlotID slotID, int resultIndex)
{
    int count = 0;
    for( int i=0; i<m_Links.size(); i++ )
    {
        if( m_Links[i].m_InputNodeID == nodeID && m_Links[i].m_InputSlotID == slotID )
        {
            int nodeIndex = FindNodeIndexByID( m_Links[i].m_OutputNodeID );

            if( count == resultIndex )
                return m_Nodes[nodeIndex];

            count++;
        }
    }

    return nullptr;
}

MyNodeGraph::MyNode* MyNodeGraph::FindNodeConnectedToOutput(NodeID nodeID, SlotID slotID, int resultIndex)
{
    int count = 0;
    for( int i=0; i<m_Links.size(); i++ )
    {
        if( m_Links[i].m_OutputNodeID == nodeID && m_Links[i].m_OutputSlotID == slotID )
        {
            int nodeIndex = FindNodeIndexByID( m_Links[i].m_InputNodeID );

            if( count == resultIndex )
                return m_Nodes[nodeIndex];

            count++;
        }
    }

    return nullptr;
}

uint32 MyNodeGraph::GetNextNodeIDAndIncrement()
{
    return m_NextNodeID++;
}

void MyNodeGraph::AddExistingNode(MyNode* pNode)
{
    m_Nodes.push_back( pNode );
}

void MyNodeGraph::RemoveExistingNode(MyNode* pNode)
{
    m_Nodes.erase( std::find( m_Nodes.begin(), m_Nodes.end(), pNode ) );
}

bool MyNodeGraph::IsNodeSlotInUse(NodeID nodeID, SlotID slotID, SlotType slotType)
{
    for( int linkIndex = 0; linkIndex < m_Links.size(); linkIndex++ )
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

void MyNodeGraph::SetExpandedForAllSelectedNodes(bool expand)
{
    for( int i=0; i<m_SelectedNodeIDs.Size; i++ )
    {
        NodeID nodeID = m_SelectedNodeIDs[i];
        int nodeIndex = FindNodeIndexByID( nodeID );
        MyNode* pNode = m_Nodes[nodeIndex];
        pNode->m_Expanded = expand;
    }
}

void MyNodeGraph::ClearSelections()
{
    m_SelectedNodeIDs.clear();
    m_SelectedNodeLinkIndexes.clear();
}

void MyNodeGraph::SelectNode(NodeID nodeID, bool checkControlKey)
{
    // Deal with control key if wanted.
    if( checkControlKey )
    {
        // If control isn't held, clear all selected nodes.
        if( ImGui::GetIO().KeyCtrl == false )
        {
            ClearSelections();
        }
        else // If control is held.
        {
            // If the node is selected, unselect it and return.
            if( m_SelectedNodeIDs.contains( nodeID ) == true )
            {
                m_SelectedNodeIDs.erase( std::remove( m_SelectedNodeIDs.begin(), m_SelectedNodeIDs.end(), nodeID ), m_SelectedNodeIDs.end() );
                return;
            }
        }
    }

    // Select the node if it's not already selected.
    if( m_SelectedNodeIDs.contains( nodeID ) == false )
    {
        m_SelectedNodeIDs.push_back( nodeID );
    }
}

void MyNodeGraph::Update()
{
    EditorDocument::Update();

    bool openContextMenu = false;
    int nodeIndexHoveredInList = -1;
    int nodeIndexHoveredInScene = -1;

    // Pull the selected nodes to the foreground.
    // This breaks the order of the nodes list on the left, but I'm okay with that.
    for( int i=m_SelectedNodeIDs.size()-1; i>=0; i-- )
    {
        MyNode* pNode = m_Nodes[FindNodeIndexByID( m_SelectedNodeIDs[i] )];
        RemoveExistingNode( pNode );
        m_Nodes.insert( m_Nodes.begin(), pNode );
    }

    // Draw a list of nodes on the left side.
    ImGui::BeginChild( "node list", ImVec2(100, 0) );
    ImGui::Text( "Nodes" );
    ImGui::Separator();
    for( uint32 nodeIndex = 0; nodeIndex < m_Nodes.size(); nodeIndex++ )
    {
        MyNode* pNode = m_Nodes[nodeIndex];
        ImGui::PushID( pNode->m_ID );
        
        bool isSelected = false;
        isSelected = m_SelectedNodeIDs.contains( pNode->m_ID );

        if( pNode->m_RenameState != MyNode::RenameState::Idle )
        {
            if( pNode->m_RenameState == MyNode::RenameState::Requested )
            {
                ImGui::SetKeyboardFocusHere();
                pNode->m_RenameState = MyNode::RenameState::BeingRenamed;
            }

            if( ImGui::InputText( "", pNode->m_Name, 32, ImGuiInputTextFlags_EnterReturnsTrue ) )
            {
                pNode->m_RenameState = MyNode::RenameState::Idle;
            }
        }
        else if( ImGui::Selectable( pNode->m_Name, isSelected ) )
        {
            SelectNode( pNode->m_ID, true );
        }

        // Check for right-click context menu.
        if( ImGui::IsMouseReleased( 1 ) && ImGui::IsItemHovered() )
        {
            nodeIndexHoveredInList = nodeIndex;
            openContextMenu = true;
        }

        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginGroup();

    // Create a child canvas for the node graph.
    AddItemsAboveNodeGraphWindow();

    if( m_ShowingLuaString )
    {
        if( m_pLuaString )
        {
            ImGui::Text( m_pLuaString );
        }
    }
    else
    {
        ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 1, 1 ) );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );
        ImGui::PushStyleColor( ImGuiCol_ChildWindowBg, COLOR_BG );
        ImGui::BeginChild( "scrolling region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove );
        ImGui::PushItemWidth( 120.0f );

        Vector2 offset = m_ScrollOffset + ImGui::GetCursorScreenPos();
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();

        // Draw grid.
        DrawGrid( offset );

        // Split draw into many channels (0 for background, 2-(numNodes+2) for nodes).
        uint32 numNodes = (uint32)m_Nodes.size();
        pDrawList->ChannelsSplit( 1 + numNodes );

        int nodeLinkIndexHoveredInScene = -1;
        bool wasDraggingNewLink = m_MouseNodeLinkStartPoint.InUse();

        // Draw nodes into many foreground layers, so they get first crack at input.
        {
            for( uint32 nodeIndex = 0; nodeIndex < m_Nodes.size(); nodeIndex++ )
            {
                pDrawList->ChannelsSetCurrent( 1 + numNodes - 1 - nodeIndex ); // 1-numNodes in Foreground.

                MyNode* pNode = m_Nodes[nodeIndex];
                pNode->Draw( pDrawList, offset, m_SelectedNodeIDs.contains( pNode->m_ID ), &m_MouseNodeLinkStartPoint );

                // Check for right-click context menu.
                if( nodeIndexHoveredInScene == -1 && ImGui::IsItemHovered() )
                {
                    nodeIndexHoveredInScene = nodeIndex;

                    if( ImGui::IsMouseReleased( 1 ) )
                    {
                        openContextMenu = true;
                    }
                }
            }
        }

        // Draw links between nodes into a background layer.
        {
            // Draw established links.
            pDrawList->ChannelsSetCurrent( 0 ); // Background.
            for( int linkIndex = 0; linkIndex < m_Links.Size; linkIndex++ )
            {
                MyNodeLink* pLink = &m_Links[linkIndex];
                MyNode* pOutputNode = m_Nodes[FindNodeIndexByID( pLink->m_OutputNodeID )];
                MyNode* pInputNode = m_Nodes[FindNodeIndexByID( pLink->m_InputNodeID )];
                Vector2 p1 = offset + pOutputNode->GetOutputSlotPos( pLink->m_OutputSlotID );
                Vector2 p2 = offset + pInputNode->GetInputSlotPos( pLink->m_InputSlotID );

                ImU32 color = COLOR_LINK_NORMAL;

                if( nodeIndexHoveredInScene == -1 && nodeLinkIndexHoveredInScene == -1 &&
                    IsNearBezierCurve( ImGui::GetIO().MousePos, p1, p1 + Vector2(+50, 0), p2 + Vector2(-50, 0), p2 ) )
                {
                    nodeLinkIndexHoveredInScene = linkIndex;
                    color = COLOR_LINK_HIGHLIGHTED;
                }

                if( IsNodeLinkSelected( linkIndex ) )
                {
                    color = COLOR_LINK_SELECTED;
                }

                pDrawList->AddBezierCurve( p1, p1 + Vector2(+50, 0), p2 + Vector2(-50, 0), p2, color, 3.0f );
            }

            // Draw link being dragged by mouse.
            if( m_MouseNodeLinkStartPoint.InUse() )
            {
                Vector2 p1, p2;

                int inputNodeIndex = FindNodeIndexByID( m_MouseNodeLinkStartPoint.m_NodeID );
                MyNode* pInputNode = m_Nodes[inputNodeIndex];
            
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

        // Merge the draw list channels.
        pDrawList->ChannelsMerge();

        // Clear the temporary link when the mouse is released.
        if( ImGui::IsMouseReleased( 0 ) )
        {
            m_MouseNodeLinkStartPoint.Clear();
        }

        // Deal with context menus.
        {
            // If we right-click an empty part of the grid, unselect everything and open context menu.
            if( !ImGui::IsAnyItemHovered() &&
                ImGui::IsWindowHovered( ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem )
                && ImGui::IsMouseReleased( 1 ) )
            {
                m_SelectedNodeIDs.clear();
                m_SelectedNodeLinkIndexes.clear();
                nodeIndexHoveredInList = NodeID_Undefined;
                nodeIndexHoveredInScene = SlotID_Undefined;
                openContextMenu = true;
            }

            // If we click a link, select it.
            if( nodeLinkIndexHoveredInScene != -1 &&
                !ImGui::IsAnyItemHovered() &&
                ImGui::IsWindowHovered( ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem )
                && ImGui::IsMouseReleased( 0 ) )
            {
                // Only clear if control isn't held when right-clicking.
                if( ImGui::GetIO().KeyCtrl == false )
                {
                    m_SelectedNodeLinkIndexes.clear();
                    m_SelectedNodeIDs.clear();
                }

                if( wasDraggingNewLink == false )
                {
                    m_SelectedNodeLinkIndexes.push_back( nodeLinkIndexHoveredInScene );
                }
            }

            if( openContextMenu )
            {
                ImGui::OpenPopup("context menu");

                if( nodeLinkIndexHoveredInScene != -1 )
                {
                    m_SelectedNodeLinkIndexes.push_back( nodeLinkIndexHoveredInScene );
                }
                else if( nodeIndexHoveredInList != -1 )
                {
                    SelectNode( m_Nodes[nodeIndexHoveredInList]->m_ID, true );
                }
                else if( nodeIndexHoveredInScene != -1 )
                {
                    SelectNode( m_Nodes[nodeIndexHoveredInScene]->m_ID, true );
                }
            }

            // Draw context menu.
            ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2(8, 8) );
            if( ImGui::BeginPopup( "context menu" ) )
            {
                if( m_SelectedNodeLinkIndexes.size() > 0 )
                {
                    // Remove the link.
                    if( ImGui::MenuItem( "Remove" ) )
                    {
                        m_pCommandStack->Do( MyNew EditorCommand_NodeGraph_DeleteLink( this, m_SelectedNodeLinkIndexes ) );
                    }
                }
                else 
                {
                    MyNode* pNode = nullptr;
            
                    int nodeIndex = m_SelectedNodeIDs.size() > 0 ? FindNodeIndexByID( m_SelectedNodeIDs[0] ) : -1;
                    if( nodeIndex != -1 )
                        pNode = m_Nodes[nodeIndex];
            
                    Vector2 scenePos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
                    if( m_SelectedNodeIDs.size() > 1 )
                    {
                        ImGui::Text( "%d Nodes Selected", m_SelectedNodeIDs.size() );
                        ImGui::Separator();
                        if( ImGui::MenuItem( "Delete Nodes", nullptr, false ) )
                        {
                            m_pCommandStack->Do( MyNew EditorCommand_NodeGraph_DeleteNodes( this, m_SelectedNodeIDs ) );
                            ClearSelections();
                        }
                    }
                    else if( pNode )
                    {
                        ImGui::Text( "Node '%s'", pNode->m_Name );
                        ImGui::Separator();
                        if( ImGui::MenuItem( "Rename...", nullptr, false ) ) { pNode->m_RenameState = MyNode::RenameState::Requested; }
                        if( ImGui::MenuItem( "Delete", nullptr, false ) )
                        {
                            m_pCommandStack->Do( MyNew EditorCommand_NodeGraph_DeleteNodes( this, m_SelectedNodeIDs ) );
                            ClearSelections();
                        }
                        if( ImGui::MenuItem( "Copy", nullptr, false, false ) ) {}

                        AddAdditionalItemsToNodeContextMenu( pNode );
                    }
                    else
                    {
                        MyNode* pNode = m_pNodeTypeManager->AddCreateNodeItemsToContextMenu( scenePos, this );
                        if( pNode )
                        {
                            m_pCommandStack->Do( MyNew EditorCommand_NodeGraph_AddNode( this, pNode ) );
                        }

                        ImGui::Separator();

                        if( ImGui::MenuItem( "Paste", nullptr, false, false ) ) {}
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();
        }

        // Handle scrolling and multi-node box select.
        {
            static bool dragging = false;

            if( dragging )
            {
                // Multi-node box select.
                if( ImGui::IsMouseDragging( 0, 0.0f ) )
                {
                    AABB2D mouseAABB;
                    ImVec2 windowPos = ImGui::GetWindowPos();
                    Vector2 mouse1 = ImGui::GetMousePos() - windowPos - m_ScrollOffset;
                    Vector2 mouse2 = ImGui::GetIO().MouseClickedPos[0] - windowPos - m_ScrollOffset;
                    mouseAABB.SetUnsorted( mouse1, mouse2 );

                    pDrawList->AddRectFilled( ImGui::GetMousePos(), ImGui::GetIO().MouseClickedPos[0], COLOR_DRAG_SELECTOR );

                    if( ImGui::GetIO().KeyCtrl == false )
                    {
                        m_SelectedNodeIDs.clear();
                        m_SelectedNodeLinkIndexes.clear();
                    }

                    for( uint32 nodeIndex = 0; nodeIndex < m_Nodes.size(); nodeIndex++ )
                    {
                        MyNode* pNode = m_Nodes[nodeIndex];

                        AABB2D nodeAABB;
                        nodeAABB.Set( pNode->m_Pos, pNode->m_Pos + pNode->m_Size );

                        if( nodeAABB.IsOverlapped( mouseAABB ) )
                        {
                            SelectNode( pNode->m_ID, false );
                        }
                    }
                }
            }

            if( ImGui::IsMouseReleased( 0 ) || m_MouseNodeLinkStartPoint.InUse() )
            {
                dragging = false;
            }

            // If the node graph area is hovered, but no items are active:
            if( ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() )
            {
                if( ImGui::IsMouseDragging( 0, 0.0f ) )
                {
                    dragging = true;
                }

                // Scroll view with middle mouse.
                if( ImGui::IsMouseDragging( 2, 0.0f ) )
                {
                    m_ScrollOffset = m_ScrollOffset + ImGui::GetIO().MouseDelta;
                }

                // Clear selected nodes if left-mouse is pressed, control isn't held and we're not dragging a new link.
                if( ImGui::IsMouseClicked( 0 ) && ImGui::GetIO().KeyCtrl == false && m_MouseNodeLinkStartPoint.InUse() == false )
                {
                    m_SelectedNodeIDs.clear();
                    m_SelectedNodeLinkIndexes.clear();
                }
            }
        }

        // End of child canvas.
        ImGui::PopItemWidth();
        ImGui::EndChild(); // "scrolling region"
        ImGui::PopStyleColor(); // ImGuiCol_ChildWindowBg
        ImGui::PopStyleVar(2); // ImGuiStyleVar_FramePadding and ImGuiStyleVar_WindowPadding
    }

    ImGui::EndGroup();
}

void MyNodeGraph::AddItemsAboveNodeGraphWindow()
{
    ImGui::Text( "Scroll (%.2f,%.2f)", m_ScrollOffset.x, m_ScrollOffset.y );
    ImGui::SameLine();
    if( ImGui::Checkbox( "Lua", &m_ShowingLuaString ) )
    {
        delete[] m_pLuaString;
        m_pLuaString = ExportAsLuaString();
    }
    ImGui::SameLine( ImGui::GetWindowWidth() - 300 );
    ImGui::Checkbox( "Show grid", &m_GridVisible );
}

bool MyNodeGraph::HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    if( ImGui::GetIO().WantTextInput )
        return false;

    for( int i=0; i<m_SelectedNodeIDs.Size; i++ )
    {
        NodeID nodeID = m_SelectedNodeIDs[i];
        int nodeIndex = FindNodeIndexByID( nodeID );
        MyNode* pNode = m_Nodes[nodeIndex];

        if( pNode->HandleInput( keyAction, keyCode, mouseAction, id, x, y, pressure ) )
        {
            return true;
        }
    }

    // Handle key presses.
    if( keyCode == MYKEYCODE_DELETE )
    {
        if( m_SelectedNodeIDs.size() > 0 )
        {
            m_pCommandStack->Do( MyNew EditorCommand_NodeGraph_DeleteNodes( this, m_SelectedNodeIDs ) );
            m_SelectedNodeIDs.clear();
            m_SelectedNodeLinkIndexes.clear();
        }

        if( m_SelectedNodeLinkIndexes.size() > 0 )
        {
            m_pCommandStack->Do( MyNew EditorCommand_NodeGraph_DeleteLink( this, m_SelectedNodeLinkIndexes ) );
            m_SelectedNodeLinkIndexes.clear();
        }
    }

    return EditorDocument::HandleInput( keyAction, keyCode, mouseAction, id, x, y, pressure );
}

void MyNodeGraph::Save()
{
    EditorDocument::Save();

    // Save NodeGraph as JSON string.
    {
        const char* filename = GetRelativePath();
        if( filename[0] == '\0' )
        {
            return;
        }

        cJSON* jNodeGraph = ExportAsJSONObject();

        char* jsonString = cJSON_Print( jNodeGraph );

        cJSON_Delete( jNodeGraph );

        FILE* pFile = nullptr;
#if MYFW_WINDOWS
        fopen_s( &pFile, filename, "wb" );
#else
        pFile = fopen( filename, "wb" );
#endif
        if( pFile != nullptr )
        {
            fprintf( pFile, "%s", jsonString );
            fclose( pFile );
        }
        else
        {
            LOGError( "File failed to open: %s\n", filename );
        }

        cJSONExt_free( jsonString );
    }

    // Save Lua file.
    {
        //const char* luaString = ExportAsLuaString();
        delete[] m_pLuaString;
        m_pLuaString = ExportAsLuaString();

        const int len = 32;
        char filenameWithLuaExtension[len];
        const char* filenameWithExtension = this->GetFilename();
        int i=0;
        while( filenameWithExtension[i] != '\0' && filenameWithExtension[i] != '.' && i<len-1 )
        {
            filenameWithLuaExtension[i] = filenameWithExtension[i];
            i++;
        }
        filenameWithLuaExtension[i] = '\0';

        char fullPath[MAX_PATH];
        sprintf_s( fullPath, MAX_PATH, "Data/ScriptsGenerated/%s.lua", filenameWithLuaExtension );

        FILE* pFile = nullptr;
#if MYFW_WINDOWS
        fopen_s( &pFile, fullPath, "wb" );
#else
        pFile = fopen( fullPath, "wb" );
#endif
        if( pFile != nullptr )
        {
            fprintf( pFile, "%s", m_pLuaString );
            fclose( pFile );
        }
        else
        {
            LOGError( "File failed to open: %s\n", fullPath );
        }

        //delete[] luaString;
    }
}

void MyNodeGraph::Load()
{
    EditorDocument::Load();

    Clear();

    FILE* fileHandle;
#if MYFW_WINDOWS
    errno_t err = fopen_s( &fileHandle, m_RelativePath, "rb" );
#else
    fileHandle = fopen( fullpath, "rb" );
#endif

    char* jsonString = nullptr;
    cJSON* jNodeGraph = nullptr;

    if( fileHandle )
    {
        fseek( fileHandle, 0, SEEK_END );
        long length = ftell( fileHandle );
        if( length > 0 )
        {
            fseek( fileHandle, 0, SEEK_SET );

            MyStackAllocator::MyStackPointer stackpointer;
            jsonString = static_cast<char*>( g_pEngineCore->GetSingleFrameMemoryStack()->AllocateBlock( length+1, &stackpointer ) );
            fread( jsonString, length, 1, fileHandle );
            jsonString[length] = '\0';

            jNodeGraph = cJSON_Parse( jsonString );

            g_pEngineCore->GetSingleFrameMemoryStack()->RewindStack( stackpointer );
        }

        fclose( fileHandle );
    }

    if( jNodeGraph )
    {
        ImportFromJSONObject( jNodeGraph );
    }

    cJSON_Delete( jNodeGraph );
}

const char* MyNodeGraph::ExportAsLuaString()
{
    uint32 bufferSize = 1000;
    char* string = new char[bufferSize];

    uint32 offset = 0;

    // Class start.
    {
        offset += sprintf_s( &string[offset], bufferSize - offset, "-- This is an auto-generated file.\n" );
        offset += sprintf_s( &string[offset], bufferSize - offset, "-- Source: %s\n\n", this->GetRelativePath() );

        const int len = 32;
        char justTheFilename[len];
        const char* filenameWithExtension = this->GetFilename();
        int i=0;
        while( filenameWithExtension[i] != '\0' && filenameWithExtension[i] != '.' && i<len-1 )
        {
            justTheFilename[i] = filenameWithExtension[i];
            i++;
        }
        justTheFilename[i] = '\0';

        offset += sprintf_s( &string[offset], bufferSize - offset, "%s = \n", justTheFilename );
        offset += sprintf_s( &string[offset], bufferSize - offset, "{\n" );
        offset += sprintf_s( &string[offset], bufferSize - offset, "\n" );

        // Lua Variables in OnPlay().
        {
            offset += sprintf_s( &string[offset], bufferSize - offset, "OnPlay = function()\n" );
            for( uint32 nodeIndex = 0; nodeIndex < m_Nodes.size(); nodeIndex++ )
            {
                offset += m_Nodes[nodeIndex]->ExportAsLuaVariablesString( string, offset, bufferSize );
            }
            string[offset] = '\0';
            offset += sprintf_s( &string[offset], bufferSize - offset, "end,\n\n" );
        }

        // Rest of nodes in Lua.
        {
            for( uint32 nodeIndex = 0; nodeIndex < m_Nodes.size(); nodeIndex++ )
            {
                offset += m_Nodes[nodeIndex]->ExportAsLuaString( string, offset, bufferSize );
            }
        }

        // Class end.
        offset += sprintf_s( &string[offset], bufferSize - offset, "\n" );
        offset += sprintf_s( &string[offset], bufferSize - offset, "}\n" );
    }

    string[offset] = '\0';

    return string;
}

cJSON* MyNodeGraph::ExportAsJSONObject()
{
    cJSON* jNodeGraph = cJSON_CreateObject();

    cJSON* jNodeArray = cJSON_AddArrayToObject( jNodeGraph, "Nodes" );
    for( uint32 nodeIndex = 0; nodeIndex < m_Nodes.size(); nodeIndex++ )
    {
        cJSON_AddItemToArray( jNodeArray, m_Nodes[nodeIndex]->ExportAsJSONObject() );
    }

    cJSON* jLinkArray = cJSON_AddArrayToObject( jNodeGraph, "Links" );
    for( int linkIndex = 0; linkIndex < m_Links.size(); linkIndex++ )
    {
        MyNodeLink* pLink = &m_Links[linkIndex];

        cJSON* jLink = cJSON_CreateObject();
        cJSON_AddItemToArray( jLinkArray, jLink );

        cJSON_AddNumberToObject( jLink, "InputNodeID", pLink->m_InputNodeID );
        cJSON_AddNumberToObject( jLink, "InputSlotID", pLink->m_InputSlotID );
        cJSON_AddNumberToObject( jLink, "OutputNodeID", pLink->m_OutputNodeID );
        cJSON_AddNumberToObject( jLink, "OutputSlotID", pLink->m_OutputSlotID );
    }

    return jNodeGraph;
}

void MyNodeGraph::ImportFromJSONObject(cJSON* jNodeGraph)
{
    cJSON* jNodeArray = cJSON_GetObjectItem( jNodeGraph, "Nodes" );
    for( int nodeIndex = 0; nodeIndex < cJSON_GetArraySize( jNodeArray ); nodeIndex++ )
    {
        cJSON* jNode = cJSON_GetArrayItem( jNodeArray, nodeIndex );
        
        cJSON* jType = cJSON_GetObjectItem( jNode, "Type" );
        if( jType )
        {
            // Create the node, but avoid incrementing m_NextNodeID.
            NodeID oldNextNodeID = m_NextNodeID;
            MyNode* pNode = m_pNodeTypeManager->CreateNode( jType->valuestring, Vector2( 0, 0 ), this );
            m_NextNodeID = oldNextNodeID;

            if( pNode )
            {
                m_Nodes.push_back( pNode );
                pNode->ImportFromJSONObject( jNode );

                if( pNode->GetID() >= m_NextNodeID )
                    m_NextNodeID = pNode->GetID() + 1;
            }
        }
    }

    cJSON* jLinkArray = cJSON_GetObjectItem( jNodeGraph, "Links" );
    for( int linkIndex = 0; linkIndex < cJSON_GetArraySize( jLinkArray ); linkIndex++ )
    {
        cJSON* jLink = cJSON_GetArrayItem( jLinkArray, linkIndex );

        MyNodeLink link;

        cJSONExt_GetEnum( jLink, "InputNodeID", link.m_InputNodeID );
        cJSONExt_GetEnum( jLink, "InputSlotID", link.m_InputSlotID );
        cJSONExt_GetEnum( jLink, "OutputNodeID", link.m_OutputNodeID );
        cJSONExt_GetEnum( jLink, "OutputSlotID", link.m_OutputSlotID );

        int inputNodeIndex = FindNodeIndexByID( link.m_InputNodeID );
        int outputNodeIndex = FindNodeIndexByID( link.m_OutputNodeID );

        if( inputNodeIndex == -1 || outputNodeIndex == -1 )
        {
            LOGError( LOGTag, "Broken link found when loading nodegraph" );
        }
        else if( link.m_InputSlotID >= m_Nodes[inputNodeIndex]->GetInputSlotCount() )
        {
            LOGError( LOGTag, "Too many input links found when loading nodegraph" );
        }
        else if( link.m_OutputSlotID >= m_Nodes[outputNodeIndex]->GetOutputSlotCount() )
        {
            LOGError( LOGTag, "Too many output links found when loading nodegraph" );
        }
        else
        {
            m_Links.push_back( link );
        }
    }
}
