//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __NodeGraphEditorCommands_H__
#define __NodeGraphEditorCommands_H__

#include "MyNodeGraph.h"

class MyNodeGraph::MyNode;

class EditorCommand_NodeGraph_AddNode;
class EditorCommand_NodeGraph_DeleteNode;

//====================================================================================================
// EditorCommand_NodeGraph_AddNode
//====================================================================================================

class EditorCommand_NodeGraph_AddNode : public EditorCommand
{
protected:
    MyNodeGraph* m_pNodeGraph;
    MyNodeGraph::MyNode* m_pNode;
    bool m_DeleteNodeWhenDestroyed;

public:
    EditorCommand_NodeGraph_AddNode(MyNodeGraph* pNodeGraph, MyNodeGraph::MyNode* pNode);
    virtual ~EditorCommand_NodeGraph_AddNode();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

//====================================================================================================
// EditorCommand_NodeGraph_DeleteNode
//====================================================================================================

class EditorCommand_NodeGraph_DeleteNode : public EditorCommand
{
protected:
    MyNodeGraph* m_pNodeGraph;
    MyNodeGraph::MyNode* m_pNode;
    std::vector<MyNodeGraph::MyNodeLink> m_Links;
    bool m_DeleteNodeWhenDestroyed;

public:
    EditorCommand_NodeGraph_DeleteNode(MyNodeGraph* pNodeGraph, MyNodeGraph::MyNode* pNode);
    virtual ~EditorCommand_NodeGraph_DeleteNode();

    virtual void Do();
    virtual void Undo();
    virtual EditorCommand* Repeat();
};

#endif // __NodeGraphEditorCommands_H__
