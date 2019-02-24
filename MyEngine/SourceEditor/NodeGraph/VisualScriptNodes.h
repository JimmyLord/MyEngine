//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __VisualScriptNodes_H__
#define __VisualScriptNodes_H__

#include "MyNode.h"
#include "../../SourceCommon/ComponentSystem/BaseComponents/ComponentBase.h"
#include "../../SourceCommon/ComponentSystem/BaseComponents/ComponentVariable.h"

class ComponentBase;

// Visual Script node types.
class VisualScriptNode_Float;
class VisualScriptNode_Color;
class VisualScriptNode_Component;
class VisualScriptNode_Add;

//====================================================================================================
// VisualScriptNodeTypeManager
//====================================================================================================

class VisualScriptNodeTypeManager : public MyNodeTypeManager
{
protected:

public:
    VisualScriptNodeTypeManager();

    virtual MyNodeGraph::MyNode* AddCreateNodeItemsToContextMenu(Vector2 pos, MyNodeGraph* pNodeGraph) override;
    virtual MyNodeGraph::MyNode* CreateNode(const char* typeName, Vector2 pos, MyNodeGraph* pNodeGraph) override;
};

#define VSNAddVar ComponentBase::AddVariable_Base

//====================================================================================================
// VisualScriptNode_Float
//====================================================================================================

class VisualScriptNode_Float : public MyNodeGraph::MyNode
{
protected:
    float m_Float;

public:
    VisualScriptNode_Float(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, float value)
    : MyNode( pNodeGraph, id, name, pos, 0, 1)
    {
        m_Float = value;
        VSNAddVar( &m_VariablesList, "Value", ComponentVariableType_Float, MyOffsetOf( this, &this->m_Float ), false, true, nullptr, nullptr, nullptr, nullptr );
    }

    virtual void DrawTitle() override
    {
        if( m_Expanded )
            MyNode::DrawTitle();
        else
            ImGui::Text( "%s: %0.2f", m_Name, m_Float );
    }
};

//====================================================================================================
// VisualScriptNode_Color
//====================================================================================================

class VisualScriptNode_Color : public MyNodeGraph::MyNode
{
protected:
    ColorByte m_Color;

public:
    VisualScriptNode_Color(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, const ColorByte& color)
    : MyNode( pNodeGraph, id, name, pos, 0, 1)
    {
        m_Color = color;
        VSNAddVar( &m_VariablesList, "Value", ComponentVariableType_ColorByte, MyOffsetOf( this, &this->m_Color ), false, true, nullptr, nullptr, nullptr, nullptr );
    }

    virtual void DrawTitle() override
    {
        if( m_Expanded )
        {
            MyNode::DrawTitle();
        }
        else
        {
            ImGui::Text( "%s", m_Name );
            ImGui::SameLine();
            ImVec4 color = *(ImVec4*)&m_Color.AsColorFloat();
            ImGui::ColorButton( "", color, ImGuiColorEditFlags_NoPicker );
        }
    }
};

//====================================================================================================
// VisualScriptNode_Component
//====================================================================================================

class VisualScriptNode_Component : public MyNodeGraph::MyNode
{
protected:
    ComponentBase* m_pComponent;

public:
    VisualScriptNode_Component(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, ComponentBase* pComponent)
    : MyNode( pNodeGraph, id, name, pos, 0, 1)
    {
        m_pComponent = pComponent;
        VSNAddVar( &m_VariablesList, "Value", ComponentVariableType_ComponentPtr, MyOffsetOf( this, &this->m_pComponent ), false, true, nullptr, nullptr, nullptr, nullptr );
    }
};

//====================================================================================================
// VisualScriptNode_Add
//====================================================================================================

class VisualScriptNode_Add : public MyNodeGraph::MyNode
{
protected:
public:
    VisualScriptNode_Add(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos)
    : MyNode( pNodeGraph, id, name, pos, 2, 1) { }

    virtual void DrawContents() override
    {
        MyNode::DrawContents();

        ImGui::Text( "+" );
    }
};

#endif //__VisualScriptNodes_H__
