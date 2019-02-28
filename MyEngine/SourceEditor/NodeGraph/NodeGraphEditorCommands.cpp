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
// EditorCommand_NodeGraph_DeleteNode
//====================================================================================================

EditorCommand_NodeGraph_DeleteNode::EditorCommand_NodeGraph_DeleteNode(MyNodeGraph* pNodeGraph, MyNodeGraph::MyNode* pNode)
{
    m_Name = "EditorCommand_NodeGraph_DeleteNode";

    MyAssert( pNode != nullptr );
    MyAssert( pNode->GetNodeGraph() == pNodeGraph );

    m_pNodeGraph = pNodeGraph;
    m_pNode = pNode;

    for( int slotIndex=0; slotIndex<m_pNode->m_InputsCount; slotIndex++ )
    {
        int count = 0;
        while( MyNodeGraph::MyNodeLink* pNodeLink = m_pNodeGraph->FindLinkConnectedToInput( m_pNode->m_ID, slotIndex, count++ ) )
        {
            m_Links.push_back( *pNodeLink );
        }
    }

    for( int slotIndex=0; slotIndex<m_pNode->m_OutputsCount; slotIndex++ )
    {
        int count = 0;
        while( MyNodeGraph::MyNodeLink* pNodeLink = m_pNodeGraph->FindLinkConnectedToOutput( m_pNode->m_ID, slotIndex, count++ ) )
        {
            m_Links.push_back( *pNodeLink );
        }
    }

    m_DeleteNodeWhenDestroyed = true;
}

EditorCommand_NodeGraph_DeleteNode::~EditorCommand_NodeGraph_DeleteNode()
{
    if( m_DeleteNodeWhenDestroyed )
    {
        delete m_pNode;
    }
}

void EditorCommand_NodeGraph_DeleteNode::Do()
{
    m_pNodeGraph->RemoveExistingNode( m_pNode );

    // Remove all connections to this node.
    for( int i=0; i<m_pNodeGraph->m_Links.size(); i++ )
    {
        if( m_pNodeGraph->m_Links[i].m_InputNodeID == m_pNode->m_ID || m_pNodeGraph->m_Links[i].m_OutputNodeID == m_pNode->m_ID )
        {
            m_pNodeGraph->m_Links.erase_unsorted( m_pNodeGraph->m_Links.Data + i );
            i--;
        }
    }

    m_DeleteNodeWhenDestroyed = true;
}

void EditorCommand_NodeGraph_DeleteNode::Undo()
{
    m_pNodeGraph->AddExistingNode( m_pNode );

    // Restore all links.
    for( uint32 i=0; i<m_Links.size(); i++ )
    {
        m_pNodeGraph->m_Links.push_back( m_Links[i] );
    }

    m_DeleteNodeWhenDestroyed = false;
}

EditorCommand* EditorCommand_NodeGraph_DeleteNode::Repeat()
{
    // Do nothing.

    return nullptr;
}
