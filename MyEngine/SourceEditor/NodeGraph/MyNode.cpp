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
#include "../../SourceCommon/ComponentSystem/BaseComponents/ComponentBase.h"
#include "../../SourceCommon/ComponentSystem/BaseComponents/ComponentVariable.h"
#include "../../SourceCommon/Core/EngineCore.h"

const float NODE_SLOT_RADIUS = 4.0f;
const float NODE_SLOT_COLLISION_RADIUS = 6.0f;

const Vector2 NODE_WINDOW_PADDING( 8.0f, 8.0f );

#undef AddVar
#define AddVar ComponentBase::AddVariable_Base

MyNodeGraph::MyNode::MyNode(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, int inputsCount, int outputsCount)
{
    m_pNodeGraph = pNodeGraph;

    m_ID = id;
    strncpy_s( m_Name, 32, name, 31 );
    m_Name[31] = '\0';
    m_Pos = pos;
    m_InputsCount = inputsCount;
    m_OutputsCount = outputsCount;
    m_InputTooltips = nullptr;
    m_OutputTooltips = nullptr;
    m_Expanded = true;

    m_Size.Set( 0, 0 );
    m_TitleWidth = 1; // Initially set to a small width, will expand based on controls.
    m_RenameState = RenameState::Idle;
    m_IsBeingDragged = false;
    m_MouseDownPosition.Set( -1, -1 );
}

MyNodeGraph::MyNode::~MyNode()
{
    ComponentVariable* pNextVar = nullptr;
    for( ComponentVariable* pVar = m_VariablesList.GetHead(); pVar; pVar = pNextVar )
    {
        pNextVar = pVar->GetNext();

        delete pVar;
    }
}

MyNodeGraph* MyNodeGraph::MyNode::GetNodeGraph()
{
    return m_pNodeGraph;
}

uint32 MyNodeGraph::MyNode::GetInputSlotCount() const
{
    return m_InputsCount;
}

uint32 MyNodeGraph::MyNode::GetOutputSlotCount() const
{
    return m_OutputsCount;
}

ImVec2 MyNodeGraph::MyNode::GetInputSlotPos(SlotID slotID) const
{
    MyAssert( slotID < m_InputsCount );
    return ImVec2( m_Pos.x, m_Pos.y + m_Size.y * ((float)slotID + 1) / ((float)m_InputsCount + 1) );
}
    
ImVec2 MyNodeGraph::MyNode::GetOutputSlotPos(SlotID slotID) const
{
    MyAssert( slotID < m_OutputsCount );
    return ImVec2( m_Pos.x + m_Size.x, m_Pos.y + m_Size.y * ((float)slotID + 1) / ((float)m_OutputsCount + 1) );
}

void MyNodeGraph::MyNode::SetTooltipStrings(const char** inputTooltips, const char** outputTooltips)
{
    m_InputTooltips = inputTooltips;
    m_OutputTooltips = outputTooltips;
}

void MyNodeGraph::MyNode::Draw(ImDrawList* pDrawList, Vector2 offset, bool isSelected, MouseNodeLinkStartPoint* pMouseNodeLink)
{
    ImGui::PushID( m_ID );
    ImVec2 nodeRectMin = offset + m_Pos;

    float titleHeight = 28.0f;
    bool titleOfNodeIsActive = false;
    bool titleOfNodeIsHovered = false;
    bool BGOfNodeIsHovered = false;
    bool ControlInsideNodeIsActive = false;

    // Display node background, no BG on the first frame since we don't know the size of the node yet.
    if( m_Size.x > 0 )
    {
        ImVec2 nodeRectMax = nodeRectMin + m_Size;

        ImGui::SetCursorScreenPos( nodeRectMin );

        // If selected, draw another full color body a bit bigger than the main body to make a "halo".
        if( isSelected )
        {
            ImVec2 thickness = ImVec2( 3, 3 );
            pDrawList->AddRectFilled( nodeRectMin - thickness, nodeRectMax + thickness, COLOR_NODE_BG_SELECTED_BORDER, 4.0f );
        }

        if( m_Expanded )
        {
            ImVec2 nodeRectMidLeft = ImVec2( nodeRectMin.x, nodeRectMin.y + titleHeight );
            ImVec2 nodeRectMidRight = ImVec2( nodeRectMin.x + m_Size.x, nodeRectMin.y + titleHeight );

            // Draw title area a different color than the body.
            pDrawList->AddRectFilled( nodeRectMin, nodeRectMidRight, COLOR_NODE_BG_TITLE, 4.0f, ImDrawCornerFlags_Top );
            pDrawList->AddRectFilled( nodeRectMidLeft, nodeRectMax, COLOR_NODE_BG, 4.0f, ImDrawCornerFlags_Bot );
        }
        else
        {
            // Only draw the title area.
            pDrawList->AddRectFilled( nodeRectMin, nodeRectMax, COLOR_NODE_BG_TITLE, 4.0f );
        }

        // Add a border around the entire node.
        pDrawList->AddRect( nodeRectMin, nodeRectMax, COLOR_NODE_TRIM, 4.0f );
    }

    // Display node contents.
    {
        // Add the node's name along with an arrow to collapse/expand it.
        ImGui::SetCursorScreenPos( nodeRectMin + NODE_WINDOW_PADDING );
        if( ImGui::ArrowButton( "", m_Expanded ? ImGuiDir_Down : ImGuiDir_Right ) )
        {
            if( m_pNodeGraph->m_SelectedNodeIDs.contains( m_ID ) == false )
            {
                m_pNodeGraph->SelectNode( m_ID, true );
            }

            m_pNodeGraph->SetExpandedForAllSelectedNodes( !m_Expanded );
        }
        bool titleArrowIsHovered = ImGui::IsItemHovered();
        ImGui::SameLine();
        DrawTitle();
        ImGui::SameLine();

        float titleWidth = ImGui::GetCursorScreenPos().x - nodeRectMin.x;

        // Add an invisible button over the entire title area, so it can be double-clicked to collapse/expand.
        // Added after ArrowButton and name, so it's overlap checks will happen after.
        ImGui::SetCursorScreenPos( nodeRectMin );
        ImGui::InvisibleButton( "", ImVec2( m_TitleWidth + NODE_WINDOW_PADDING.x, titleHeight ) );
        titleOfNodeIsActive = ImGui::IsItemActive();
        titleOfNodeIsHovered = ImGui::IsItemHovered() && titleArrowIsHovered == false;

        ImGui::SetCursorScreenPos( nodeRectMin + NODE_WINDOW_PADDING );
        
        // Draw the node contents as a group to lock horizontal position.
        ImGui::BeginGroup();

        // If expanded, add this node's controls.
        if( m_Expanded )
        {
            ImGui::SetCursorScreenPos( nodeRectMin + NODE_WINDOW_PADDING + ImVec2( 0, titleHeight ) );

            DrawContents();
        }
        else
        {
            // Otherwise, pad out the space, so size doesn't change.
            ImGui::SetCursorScreenPos( nodeRectMin + ImVec2( m_Size.x, titleHeight ) + NODE_WINDOW_PADDING );
        }

        ImGui::EndGroup();

        // Save the size of what we have emitted and whether any of the widgets are being used.
        if( m_Expanded )
        {
            m_TitleWidth = ImGui::GetItemRectSize().x == 0 ? 1 : ImGui::GetItemRectSize().x;
            m_Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
        }
        else
        {
            m_Size = ImGui::GetItemRectSize();
        }

        if( m_TitleWidth < titleWidth )
            m_TitleWidth = titleWidth;
        if( m_Size.x < m_TitleWidth )
            m_Size.x = m_TitleWidth;
    }

    // Create a large invisible button the size of the node, for selection/dragging purposes.
    // Since it's created after all other widgets on the node, it shouldn't interfere with other widget/button overlap.
    ImGui::SetCursorScreenPos( nodeRectMin );
    ImGui::InvisibleButton( "node", m_Size );

    bool bodyOfNodeIsActive = ImGui::IsItemActive();
    bool bodyOfNodeIsHovered = ImGui::IsItemHovered();

    // Select/Deselect nodes if the title is clicked.
    if( titleOfNodeIsHovered || bodyOfNodeIsHovered )
    {
        // Setup for undo/redo of node movement.
        if( ImGui::IsMouseDown( 0 ) && !ImGui::IsMouseDragging() )
        {
            // Store mouse down position.
            m_MouseDownPosition = ImGui::GetIO().MousePos;
            m_IsBeingDragged = true;
        }
        if( ImGui::IsMouseReleased( 0 ) )
        {
            if( m_IsBeingDragged )
            {
                if( m_pNodeGraph->m_SelectedNodeIDs.size() > 0 )
                {
                    Vector2 amountMoved = ImGui::GetIO().MousePos - m_MouseDownPosition;
                    if( amountMoved.LengthSquared() > 0 )
                    {
                        m_pNodeGraph->m_pCommandStack->Add( MyNew EditorCommand_NodeGraph_MoveNodes( m_pNodeGraph, m_pNodeGraph->m_SelectedNodeIDs, amountMoved ) );
                    }
                }
            }

            m_IsBeingDragged = false;
        }

        // Select on clicks.
        if( ImGui::IsMouseClicked( 0 ) )
        {
            // Select the node only if its not already selected.
            // i.e. Ignore the click if the node is selected.
            if( m_pNodeGraph->m_SelectedNodeIDs.contains( m_ID ) == false )
            {
                m_pNodeGraph->SelectNode( m_ID, true );
            }
            else
            {
                // If it is selected, allow a control click to deselect it.
                if( ImGui::GetIO().KeyCtrl == true )
                {
                    m_pNodeGraph->SelectNode( m_ID, true );
                }
            }
        }
    }

    // Draw a circle for each link slot and check slots for mouse hover/click.
    {
        for( uint32 slotIndex = 0; slotIndex < m_OutputsCount; slotIndex++ )
        {
            Vector2 slotPos = offset + GetOutputSlotPos( slotIndex );
            HandleNodeSlot( pDrawList, slotPos, m_ID, slotIndex, SlotType_Output, pMouseNodeLink );
        }

        for( uint32 slotIndex = 0; slotIndex < m_InputsCount; slotIndex++ )
        {
            Vector2 slotPos = offset + GetInputSlotPos( slotIndex );
            HandleNodeSlot( pDrawList, slotPos, m_ID, slotIndex, SlotType_Input, pMouseNodeLink );
        }
    }

    // Move selected nodes if a selected node is dragged and we're not creating a link.
    bool nodeIsSelected = m_pNodeGraph->m_SelectedNodeIDs.contains( m_ID );
    if( nodeIsSelected && ( titleOfNodeIsActive || bodyOfNodeIsActive ) &&
        ImGui::IsMouseDragging(0) && pMouseNodeLink->InUse() == false )
    {
        for( int i=0; i<m_pNodeGraph->m_SelectedNodeIDs.Size; i++ )
        {
            NodeID nodeID = m_pNodeGraph->m_SelectedNodeIDs[i];
            int nodeIndex = m_pNodeGraph->FindNodeIndexByID( nodeID );
            MyNode* pNode = m_pNodeGraph->m_Nodes[nodeIndex];
            pNode->m_Pos += ImGui::GetIO().MouseDelta;
        }
    }

    ImGui::PopID();
}

void MyNodeGraph::MyNode::HandleNodeSlot(ImDrawList* pDrawList, Vector2 slotPos, NodeID nodeID, SlotID slotID, SlotType slotType, MouseNodeLinkStartPoint* pMouseNodeLink)
{
    ImU32 slotColor = COLOR_SLOT_DEFAULT;

    // Check for collisions.
    if( HandleNodeLinkCreation( slotPos, nodeID, slotID, slotType, pMouseNodeLink ) )
    {
        slotColor = pMouseNodeLink->m_Color;
    }

    // Draw the circle.
    if( m_pNodeGraph->IsNodeSlotInUse( nodeID, slotID, slotType ) )
    {
        pDrawList->AddCircleFilled( slotPos, NODE_SLOT_RADIUS, slotColor );
    }
    else
    {
        pDrawList->AddCircle( slotPos, NODE_SLOT_RADIUS, slotColor, 12, 2 );
    }
}

bool MyNodeGraph::MyNode::HandleNodeLinkCreation(Vector2 slotPos, NodeID nodeID, SlotID slotID, SlotType slotType, MouseNodeLinkStartPoint* pMouseNodeLink)
{
    bool isHovering = false;

    Vector2 diff = ImGui::GetIO().MousePos - slotPos;
    
    // Check if mouse is not over circle.
    if( diff.Length() <= NODE_SLOT_COLLISION_RADIUS )
        isHovering = true;

    // Display a tooltip for this link if we're hovering or alt is held.
    if( isHovering || ImGui::GetIO().KeyAlt )
    {
        const char* strToShow = nullptr;

        if( slotType == SlotType_Input && m_InputTooltips != nullptr )
        {
            strToShow = m_InputTooltips[slotID];
        }
        if( slotType == SlotType_Output && m_OutputTooltips != nullptr )
        {
            strToShow = m_OutputTooltips[slotID];
        }

        if( strToShow )
        {
            if( isHovering )
            {
                ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 7, 5 ) );
                ImGui::BeginTooltip();
                ImGui::Text( "%s", strToShow );
                ImGui::EndTooltip();
                ImGui::PopStyleVar();
            }
            else // Alt key is held.
            {
                ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 7, 5 ) );
                char windowName[32];
                ImFormatString( windowName, sizeof(windowName), "##NodeLink_%02d_%02d_%01d", nodeID, slotID, slotType );
                ImGuiWindowFlags flags = ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoDocking;

                ImGui::SetNextWindowPos( slotPos );
                ImGui::SetNextWindowBgAlpha( 0.75f );
                ImGui::Begin( windowName, NULL, flags );
                ImGui::Text( "%s", strToShow );
                ImGui::End();

                ImGui::PopStyleVar();
            }
        }
    }

    // If we're not hovering over the link, return.
    if( isHovering == false )
        return false;

    // If mouse is clicked, then start a new link. // TODO: Have this take precedence over moving a node around with the mouse.
    if( ImGui::IsMouseClicked( 0 ) )
    {
        pMouseNodeLink->Set( nodeID, slotID, slotType );
    }

    // Don't allow link to same node. TODO: Check for circlular links between multiple nodes.
    if( nodeID == pMouseNodeLink->m_NodeID )
    {
        pMouseNodeLink->m_Color = COLOR_LINK_IN_PROGRESS_INVALID;
    }
    else if( pMouseNodeLink->m_SlotType == SlotType_Undefined )
    {
        pMouseNodeLink->m_Color = COLOR_SLOT_HOVERED;
    }
    else
    {
        if( pMouseNodeLink->m_SlotType == SlotType_Input && slotType == SlotType_Output )
        {
            pMouseNodeLink->m_Color = COLOR_LINK_IN_PROGRESS_VALID;

            // If mouse is released, create a link.
            if( ImGui::IsMouseReleased( 0 ) )
            {
                m_pNodeGraph->m_pCommandStack->Do( MyNew EditorCommand_NodeGraph_CreateLink( m_pNodeGraph, MyNodeLink( nodeID, slotID, pMouseNodeLink->m_NodeID, pMouseNodeLink->m_SlotID ) ) );
            }
        }
        else if( pMouseNodeLink->m_SlotType == SlotType_Output && slotType == SlotType_Input )
        {
            pMouseNodeLink->m_Color = COLOR_LINK_IN_PROGRESS_VALID;

            // If mouse is released, create a link.
            if( ImGui::IsMouseReleased( 0 ) )
            {
                m_pNodeGraph->m_pCommandStack->Do( MyNew EditorCommand_NodeGraph_CreateLink( m_pNodeGraph, MyNodeLink( pMouseNodeLink->m_NodeID, pMouseNodeLink->m_SlotID, nodeID, slotID ) ) );
            }
        }
        else
        {
            pMouseNodeLink->m_Color = COLOR_LINK_IN_PROGRESS_INVALID;
        }
    }

    return true; // Mouse is over circle.
}

void MyNodeGraph::MyNode::DrawTitle()
{
    //if( m_RenamedProcess > 0 )
    //{
    //    if( m_RenamedProcess == 1 )
    //    {
    //        ImGui::SetKeyboardFocusHere();
    //        m_RenamedProcess = 2;
    //    }
    //    if( ImGui::InputText( "", m_Name, 32, ImGuiInputTextFlags_EnterReturnsTrue ) )
    //    {
    //        m_RenamedProcess = 0;
    //    }
    //}
    //else
    {
        ImGui::Text( m_Name );
    }
}

void MyNodeGraph::MyNode::DrawContents()
{
    for( ComponentVariable* pVar = m_VariablesList.GetHead(); pVar; pVar = pVar->GetNext() )
    {
        ImGui::PushItemWidth( 120 );
        ComponentBase::AddVariableToWatchPanel( g_pEngineCore, this, pVar, nullptr );
    }
}

cJSON* MyNodeGraph::MyNode::ExportAsJSONObject()
{
    cJSON* jNode = cJSON_CreateObject();

    cJSON_AddStringToObject( jNode, "Type", GetType() );
    cJSON_AddStringToObject( jNode, "Name", m_Name );
    cJSON_AddNumberToObject( jNode, "ID", m_ID );
    cJSONExt_AddFloatArrayToObject( jNode, "Pos", &m_Pos.x, 2 );
    //cJSON_AddNumberToObject( jNode, "InputsCount", m_InputsCount );
    //cJSON_AddNumberToObject( jNode, "OutputsCount", m_OutputsCount );
    cJSON_AddBoolToObject( jNode, "Expanded", m_Expanded );

    ComponentBase::ExportVariablesToJSON( jNode, this, &m_VariablesList, nullptr );

    return jNode;
}

void MyNodeGraph::MyNode::ImportFromJSONObject(cJSON* jNode)
{
    cJSONExt_GetString( jNode, "Name", m_Name, 32 );
    cJSONExt_GetEnum( jNode, "ID", m_ID );
    cJSONExt_GetFloatArray( jNode, "Pos", &m_Pos.x, 2 );
    //cJSONExt_GetUnsignedInt( jNode, "InputsCount", &m_InputsCount );
    //cJSONExt_GetUnsignedInt( jNode, "OutputsCount", &m_OutputsCount );
    cJSONExt_GetBool( jNode, "Expanded", &m_Expanded );

    ComponentBase::ImportVariablesFromJSON( jNode, this, &m_VariablesList, nullptr, SCENEID_MainScene );
}
