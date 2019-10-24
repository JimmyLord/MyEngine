//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "NodeGraphEditorCommands.h"
#include "MyNode.h"
#include "MyNodeGraph.h"
#include "VisualScriptNodes.h"

//====================================================================================================
// EditorCommand_NodeGraph_AddNode
//====================================================================================================

EditorCommand_NodeGraph_AddNode::EditorCommand_NodeGraph_AddNode(MyNodeGraph* pNodeGraph, MyNodeGraph::MyNode* pNode)
{
    m_Name = "EditorCommand_NodeGraph_AddNode";

    MyAssert( pNode != nullptr );
    MyAssert( pNode->GetNodeGraph() == pNodeGraph );

    m_pNodeGraph = pNodeGraph;
    m_pNode = pNode;
    m_DeleteNodeWhenDestroyed = false;
}

EditorCommand_NodeGraph_AddNode::~EditorCommand_NodeGraph_AddNode()
{
    if( m_DeleteNodeWhenDestroyed )
    {
        delete m_pNode;
    }
}

void EditorCommand_NodeGraph_AddNode::Do()
{
    m_pNodeGraph->AddExistingNode( m_pNode );
    m_DeleteNodeWhenDestroyed = false;
}

void EditorCommand_NodeGraph_AddNode::Undo()
{
    m_pNodeGraph->RemoveExistingNode( m_pNode );
    m_DeleteNodeWhenDestroyed = true;
}

EditorCommand* EditorCommand_NodeGraph_AddNode::Repeat()
{
    // Do nothing.

    return nullptr;
}

//====================================================================================================
// EditorCommand_NodeGraph_DeleteNodes
//====================================================================================================

EditorCommand_NodeGraph_DeleteNodes::EditorCommand_NodeGraph_DeleteNodes(MyNodeGraph* pNodeGraph, ImVector<MyNodeGraph::NodeID>& selectedNodeIDs)
{
    m_Name = "EditorCommand_NodeGraph_DeleteNodes";

    MyAssert( selectedNodeIDs.size() != 0 );

    m_pNodeGraph = pNodeGraph;
    for( int i=0; i<selectedNodeIDs.Size; i++ )
    {
        MyNodeGraph::NodeID nodeID = selectedNodeIDs[i];

        int nodeIndex = m_pNodeGraph->FindNodeIndexByID( nodeID );
        MyNodeGraph::MyNode* pNode = m_pNodeGraph->m_Nodes[nodeIndex];
        MyAssert( pNode->GetNodeGraph() == m_pNodeGraph );

        m_pNodes.push_back( pNode );
    }

    for( uint32 nodeIndex=0; nodeIndex<m_pNodes.size(); nodeIndex++ )
    {
        MyNodeGraph::MyNode* pNode = m_pNodes[nodeIndex];

        for( uint32 slotIndex=0; slotIndex<pNode->m_InputsCount; slotIndex++ )
        {
            int count = 0;
            while( MyNodeGraph::MyNodeLink* pNodeLink = m_pNodeGraph->FindLinkConnectedToInput( pNode->m_ID, slotIndex, count++ ) )
            {
                m_Links.push_back( *pNodeLink );
            }
        }

        for( uint32 slotIndex=0; slotIndex<pNode->m_OutputsCount; slotIndex++ )
        {
            int count = 0;
            while( MyNodeGraph::MyNodeLink* pNodeLink = m_pNodeGraph->FindLinkConnectedToOutput( pNode->m_ID, slotIndex, count++ ) )
            {
                m_Links.push_back( *pNodeLink );
            }
        }
    }

    m_DeleteNodesWhenDestroyed = true;
}

EditorCommand_NodeGraph_DeleteNodes::~EditorCommand_NodeGraph_DeleteNodes()
{
    if( m_DeleteNodesWhenDestroyed )
    {
        for( uint32 nodeIndex=0; nodeIndex<m_pNodes.size(); nodeIndex++ )
        {
            delete m_pNodes[nodeIndex];
        }
    }
}

void EditorCommand_NodeGraph_DeleteNodes::Do()
{
    // Remove Nodes.
    for( uint32 nodeIndex=0; nodeIndex<m_pNodes.size(); nodeIndex++ )
    {
        MyNodeGraph::MyNode* pNode = m_pNodes[nodeIndex];

        m_pNodeGraph->RemoveExistingNode( pNode );

        // Remove all connections to this node.
        for( int i=0; i<m_pNodeGraph->m_Links.size(); i++ )
        {
            if( m_pNodeGraph->m_Links[i].m_InputNodeID == pNode->m_ID || m_pNodeGraph->m_Links[i].m_OutputNodeID == pNode->m_ID )
            {
                m_pNodeGraph->m_Links.erase_unsorted( m_pNodeGraph->m_Links.Data + i );
                i--;
            }
        }
    }

    m_DeleteNodesWhenDestroyed = true;
}

void EditorCommand_NodeGraph_DeleteNodes::Undo()
{
    // Restore Nodes.
    for( uint32 nodeIndex=0; nodeIndex<m_pNodes.size(); nodeIndex++ )
    {
        m_pNodeGraph->AddExistingNode( m_pNodes[nodeIndex] );
    }

    // Restore all links.
    for( uint32 i=0; i<m_Links.size(); i++ )
    {
        m_pNodeGraph->m_Links.push_back( m_Links[i] );
    }

    m_DeleteNodesWhenDestroyed = false;
}

EditorCommand* EditorCommand_NodeGraph_DeleteNodes::Repeat()
{
    // Do nothing.

    return nullptr;
}

//====================================================================================================
// EditorCommand_NodeGraph_MoveNodes
//====================================================================================================

EditorCommand_NodeGraph_MoveNodes::EditorCommand_NodeGraph_MoveNodes(MyNodeGraph* pNodeGraph, ImVector<MyNodeGraph::NodeID>& selectedNodeIDs, Vector2 amountMoved)
{
    m_Name = "EditorCommand_NodeGraph_MoveNodes";

    MyAssert( selectedNodeIDs.size() != 0 );

    m_pNodeGraph = pNodeGraph;
    for( int i=0; i<selectedNodeIDs.Size; i++ )
    {
        MyNodeGraph::NodeID nodeID = selectedNodeIDs[i];

        int nodeIndex = m_pNodeGraph->FindNodeIndexByID( nodeID );
        MyNodeGraph::MyNode* pNode = m_pNodeGraph->m_Nodes[nodeIndex];
        MyAssert( pNode->GetNodeGraph() == m_pNodeGraph );

        m_pNodes.push_back( pNode );
    }

    m_AmountMoved = amountMoved;
}

EditorCommand_NodeGraph_MoveNodes::~EditorCommand_NodeGraph_MoveNodes()
{
}

void EditorCommand_NodeGraph_MoveNodes::Do()
{
    // Move nodes to new positions.
    for( uint32 nodeIndex=0; nodeIndex<m_pNodes.size(); nodeIndex++ )
    {
        m_pNodes[nodeIndex]->m_Pos += m_AmountMoved;
    }
}

void EditorCommand_NodeGraph_MoveNodes::Undo()
{
    // Return nodes to original positions.
    for( uint32 nodeIndex=0; nodeIndex<m_pNodes.size(); nodeIndex++ )
    {
        m_pNodes[nodeIndex]->m_Pos -= m_AmountMoved;
    }
}

EditorCommand* EditorCommand_NodeGraph_MoveNodes::Repeat()
{
    // Do nothing.

    return nullptr;
}

//====================================================================================================
// EditorCommand_NodeGraph_CreateLink
//====================================================================================================

EditorCommand_NodeGraph_CreateLink::EditorCommand_NodeGraph_CreateLink(MyNodeGraph* pNodeGraph, MyNodeGraph::MyNodeLink nodeLink)
{
    m_Name = "EditorCommand_NodeGraph_CreateLink";

    m_pNodeGraph = pNodeGraph;
    m_NodeLink = nodeLink;
    m_NodeLinkIndex = -1;
}

EditorCommand_NodeGraph_CreateLink::~EditorCommand_NodeGraph_CreateLink()
{
}

void EditorCommand_NodeGraph_CreateLink::Do()
{
    m_NodeLinkIndex = m_pNodeGraph->m_Links.size();
    m_pNodeGraph->m_Links.push_back( m_NodeLink );
}

void EditorCommand_NodeGraph_CreateLink::Undo()
{
    m_pNodeGraph->m_Links.erase_unsorted( m_pNodeGraph->m_Links.Data + m_NodeLinkIndex );
}

EditorCommand* EditorCommand_NodeGraph_CreateLink::Repeat()
{
    // Do nothing.

    return nullptr;
}

//====================================================================================================
// EditorCommand_NodeGraph_DeleteLink
//====================================================================================================

EditorCommand_NodeGraph_DeleteLink::EditorCommand_NodeGraph_DeleteLink(MyNodeGraph* pNodeGraph, const ImVector<int>& nodeLinkIndexes)
{
    MyAssert( nodeLinkIndexes.size() > 0 );

    m_Name = "EditorCommand_NodeGraph_DeleteLink";

    m_pNodeGraph = pNodeGraph;
    for( int i=0; i<nodeLinkIndexes.size(); i++ )
    {
        m_NodeLinkIndexes.push_back( nodeLinkIndexes[i] );
        m_NodeLinks.push_back( m_pNodeGraph->m_Links[nodeLinkIndexes[i]] );
    }
}

EditorCommand_NodeGraph_DeleteLink::~EditorCommand_NodeGraph_DeleteLink()
{
}

void EditorCommand_NodeGraph_DeleteLink::Do()
{
    for( int i=(uint32)m_NodeLinkIndexes.size()-1; i>=0; i-- )
    {
        m_pNodeGraph->m_Links.erase_unsorted( m_pNodeGraph->m_Links.Data + m_NodeLinkIndexes[i] );
    }
}

void EditorCommand_NodeGraph_DeleteLink::Undo()
{
    for( uint32 i=0; i<m_NodeLinkIndexes.size(); i++ )
    {
        m_pNodeGraph->m_Links.insert( m_pNodeGraph->m_Links.Data + m_NodeLinkIndexes[i], m_NodeLinks[i] );
    }
}

EditorCommand* EditorCommand_NodeGraph_DeleteLink::Repeat()
{
    // Do nothing.

    return nullptr;
}
