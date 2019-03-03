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
#include "../../SourceCommon/ComponentSystem/Core/GameObject.h"

class ComponentBase;

// Visual Script node types.
class VisualScriptNode_Value_Float;
class VisualScriptNode_Value_Color;
class VisualScriptNode_Value_GameObject;
class VisualScriptNode_Value_Component;
class VisualScriptNode_MathOp_Add;
class VisualScriptNode_Condition_GreaterEqual;
class VisualScriptNode_Event_KeyPress;
class VisualScriptNode_Disable_GameObject;

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

class VisualScriptNode : public MyNodeGraph::MyNode
{
public:
    VisualScriptNode(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, int inputsCount, int outputsCount)
    : MyNode( pNodeGraph, id, name, pos, inputsCount, outputsCount )
    {
    }

    virtual uint32 EmitLua(char* string, uint32 offset, int bytesAllocated) { return 0; }
};

#define VSNAddVar ComponentBase::AddVariable_Base

//====================================================================================================
// VisualScriptNode_Value_Float
//====================================================================================================

class VisualScriptNode_Value_Float : public VisualScriptNode
{
protected:
    float m_Float;

public:
    VisualScriptNode_Value_Float(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, float value)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        m_Float = value;
        VSNAddVar( &m_VariablesList, "Float", ComponentVariableType_Float, MyOffsetOf( this, &this->m_Float ), true, true, "", nullptr, nullptr, nullptr );
    }

    const char* GetType() { return "Value_Float"; }

    float GetValue() { return m_Float; }

    virtual void DrawTitle() override
    {
        if( m_Expanded )
            MyNode::DrawTitle();
        else
            ImGui::Text( "%s: %0.2f", m_Name, m_Float );
    }

    virtual uint32 EmitLua(char* string, uint32 offset, int bytesAllocated)
    {
        int startOffset = offset;

        offset += sprintf_s( &string[offset], bytesAllocated - offset, "(%f)", m_Float );

        return offset - startOffset;
    }
};

//====================================================================================================
// VisualScriptNode_Value_Color
//====================================================================================================

class VisualScriptNode_Value_Color : public VisualScriptNode
{
protected:
    ColorByte m_Color;

public:
    VisualScriptNode_Value_Color(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, const ColorByte& color)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        m_Color = color;
        VSNAddVar( &m_VariablesList, "Color", ComponentVariableType_ColorByte, MyOffsetOf( this, &this->m_Color ), true, true, "", nullptr, nullptr, nullptr );
    }

    const char* GetType() { return "Value_Color"; }

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
// VisualScriptNode_Value_GameObject
//====================================================================================================

class VisualScriptNode_Value_GameObject : public VisualScriptNode
{
protected:
    GameObject* m_pGameObject;

public:
    VisualScriptNode_Value_GameObject(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, GameObject* pGameObject)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        m_pGameObject = pGameObject;
        VSNAddVar( &m_VariablesList, "GameObject", ComponentVariableType_GameObjectPtr, MyOffsetOf( this, &this->m_pGameObject ), true, true, "",
            (CVarFunc_ValueChanged)&VisualScriptNode_Value_GameObject::OnValueChanged,
            (CVarFunc_DropTarget)&VisualScriptNode_Value_GameObject::OnDrop, nullptr );
    }

    const char* GetType() { return "Value_GameObject"; }

    void* OnDrop(ComponentVariable* pVar, int x, int y)
    {
        DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

        if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
        {
            m_pGameObject = (GameObject*)pDropItem->m_Value;
        }

        return nullptr;
    }

    void* OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
    {
        return nullptr;
    }
};

//====================================================================================================
// VisualScriptNode_Value_Component
//====================================================================================================

class VisualScriptNode_Value_Component : public VisualScriptNode
{
protected:
    ComponentBase* m_pComponent;

public:
    VisualScriptNode_Value_Component(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, ComponentBase* pComponent)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        m_pComponent = pComponent;
        VSNAddVar( &m_VariablesList, "Component", ComponentVariableType_ComponentPtr, MyOffsetOf( this, &this->m_pComponent ), true, true, "",
            (CVarFunc_ValueChanged)&VisualScriptNode_Value_Component::OnValueChanged,
            (CVarFunc_DropTarget)&VisualScriptNode_Value_Component::OnDrop, nullptr );
    }

    const char* GetType() { return "Value_Component"; }

    void* OnDrop(ComponentVariable* pVar, int x, int y)
    {
        DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

        if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
        {
            m_pComponent = (ComponentBase*)pDropItem->m_Value;
        }

        return nullptr;
    }

    void* OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
    {
        return nullptr;
    }
};

//====================================================================================================
// VisualScriptNode_MathOp_Add
//====================================================================================================

class VisualScriptNode_MathOp_Add : public VisualScriptNode
{
protected:
public:
    VisualScriptNode_MathOp_Add(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos)
    : VisualScriptNode( pNodeGraph, id, name, pos, 2, 1 ) {}

    const char* GetType() { return "MathOp_Add"; }

    virtual void DrawContents() override
    {
        MyNode::DrawContents();

        ImGui::Text( "+" );
    }
};


//====================================================================================================
// VisualScriptNode_VisualScriptNode_Condition_GreaterEqual
//====================================================================================================

class VisualScriptNode_Condition_GreaterEqual : public VisualScriptNode
{
protected:
public:
    VisualScriptNode_Condition_GreaterEqual(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos)
    : VisualScriptNode( pNodeGraph, id, name, pos, 3, 2 ) {}

    const char* GetType() { return "Condition_GreaterEqual"; }

    virtual void DrawContents() override
    {
        MyNode::DrawContents();

        ImGui::Text( ">=" );
    }

    virtual void Trigger() override
    {
        MyNode* pNode1 = m_pNodeGraph->FindNodeConnectedToInput( m_ID, 1 );
        MyNode* pNode2 = m_pNodeGraph->FindNodeConnectedToInput( m_ID, 2 );

        if( pNode1 == nullptr || pNode2 == nullptr )
            return;

        float value1 = ((VisualScriptNode_Value_Float*)pNode1)->GetValue();
        float value2 = ((VisualScriptNode_Value_Float*)pNode2)->GetValue();

        if( value1 >= value2 )
        {
            int count = 0;
            while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 0, count++ ) )
            {
                pNode->Trigger();
            }
        }
        else
        {
            int count = 0;
            while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 1, count++ ) )
            {
                pNode->Trigger();
            }
        }
    }

    virtual uint32 EmitLua(char* string, uint32 offset, int bytesAllocated)
    {
        int startOffset = offset;

        MyNode* pNode1 = m_pNodeGraph->FindNodeConnectedToInput( m_ID, 1 );
        MyNode* pNode2 = m_pNodeGraph->FindNodeConnectedToInput( m_ID, 2 );

        if( pNode1 == nullptr || pNode2 == nullptr )
            return 0;

        // if( condition ) then
        {
            offset += sprintf_s( &string[offset], bytesAllocated - offset, "\t" ); // TODO: Track indent size.
            offset += sprintf_s( &string[offset], bytesAllocated - offset, "if( " );
            offset += ((VisualScriptNode_Value_Float*)pNode1)->EmitLua( string, offset, bytesAllocated );
            offset += sprintf_s( &string[offset], bytesAllocated - offset," >= " );
            offset += ((VisualScriptNode_Value_Float*)pNode2)->EmitLua( string, offset, bytesAllocated );

            offset += sprintf_s( &string[offset], bytesAllocated - offset," ) then\n" );

            int count = 0;
            while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 0, count++ ) )
            {
                offset += pNode->EmitLua( string, offset, bytesAllocated );
            }
        }

        // else
        {
            offset += sprintf_s( &string[offset], bytesAllocated - offset, "\t" ); // TODO: Track indent size.
            offset += sprintf_s( &string[offset], bytesAllocated - offset, "else\n" );

            int count = 0;
            while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 0, count++ ) )
            {
                offset += pNode->EmitLua( string, offset, bytesAllocated );
            }
        }

        offset += sprintf_s( &string[offset], bytesAllocated - offset, "\t" ); // TODO: Track indent size.
        offset += sprintf_s( &string[offset], bytesAllocated - offset, "end\n" );

        return offset - startOffset;
    }
};

//====================================================================================================
// VisualScriptNode_Event_KeyPress
//====================================================================================================

class VisualScriptNode_Event_KeyPress : public VisualScriptNode
{
protected:
    int m_KeyCode;

public:
    VisualScriptNode_Event_KeyPress(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, int keyCode)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        m_KeyCode = keyCode;
        VSNAddVar( &m_VariablesList, "KeyCode", ComponentVariableType_Int, MyOffsetOf( this, &this->m_KeyCode ), true, true, "", nullptr, nullptr, nullptr );

        g_pEventManager->RegisterForEvents( "Keyboard", this, &VisualScriptNode_Event_KeyPress::StaticOnEvent );
    }

    const char* GetType() { return "Event_KeyPress"; }

    virtual void DrawTitle() override
    {
        if( m_Expanded )
            MyNode::DrawTitle();
        else
            ImGui::Text( "%s: %c", m_Name, m_KeyCode );
    }

    virtual void DrawContents() override
    {
        MyNode::DrawContents();

        ImGui::Text( "Key: %c", m_KeyCode );
    }

    static bool StaticOnEvent(void* pObjectPtr, MyEvent* pEvent) { return ((VisualScriptNode_Event_KeyPress*)pObjectPtr)->OnEvent( pEvent ); }
    bool OnEvent(MyEvent* pEvent)
    {
        MyAssert( pEvent->IsType( "Keyboard" ) );

        int action = pEvent->GetInt( "Action" );
        int keyCode = pEvent->GetInt( "KeyCode" );

        if( action == GCBA_Down && keyCode == m_KeyCode )
        {
            int count = 0;
            while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 0, count++ ) )
            {
                pNode->Trigger();
            }
        }

        return false;
    }

    virtual uint32 ExportAsLuaString(char* string, uint32 offset, int bytesAllocated)
    {
        int startOffset = offset;

        offset += sprintf_s( &string[offset], bytesAllocated - offset, "OnButtons = function(action, id)\n" );

        int count = 0;
        while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 0, count++ ) )
        {
            offset += pNode->EmitLua( string, offset, bytesAllocated );
        }

        offset += sprintf_s( &string[offset], bytesAllocated - offset,"end,\n" );

        return offset - startOffset;
    }
};

//====================================================================================================
// VisualScriptNode_Disable_GameObject
//====================================================================================================

class VisualScriptNode_Disable_GameObject : public VisualScriptNode
{
protected:
    GameObject* m_pGameObject;

public:
    VisualScriptNode_Disable_GameObject(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, GameObject* pGameObject)
    : VisualScriptNode( pNodeGraph, id, name, pos, 1, 0 )
    {
        m_pGameObject = pGameObject;
        VSNAddVar( &m_VariablesList, "GameObject", ComponentVariableType_GameObjectPtr, MyOffsetOf( this, &this->m_pGameObject ), true, true, "",
            (CVarFunc_ValueChanged)&VisualScriptNode_Disable_GameObject::OnValueChanged,
            (CVarFunc_DropTarget)&VisualScriptNode_Disable_GameObject::OnDrop, nullptr );
    }

    const char* GetType() { return "Disable_GameObject"; }

    virtual void DrawTitle() override
    {
        if( m_Expanded )
        {
            MyNode::DrawTitle();
        }
        else
        {
            if( m_pGameObject )
                ImGui::Text( "%s: %s", m_Name, m_pGameObject->GetName() );
            else
                ImGui::Text( "%s: not set" );
        }
    }

    virtual void Trigger() override
    {
        if( m_pGameObject )
        {
            m_pGameObject->SetEnabled( !m_pGameObject->IsEnabled(), true );
        }
    }

    void* OnDrop(ComponentVariable* pVar, int x, int y)
    {
        DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

        if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
        {
            m_pGameObject = (GameObject*)pDropItem->m_Value;
        }

        return nullptr;
    }

    void* OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
    {
        return nullptr;
    }

    virtual uint32 EmitLua(char* string, uint32 offset, int bytesAllocated)
    {
        int startOffset = offset;

        offset += sprintf_s( &string[offset], bytesAllocated - offset, "\t\t" ); // TODO: Track indent size.

        if( m_pGameObject->IsEnabled() )
            offset += sprintf_s( &string[offset], bytesAllocated - offset, "%s:SetEnabled( false );\n", m_pGameObject->GetName() );
        else
            offset += sprintf_s( &string[offset], bytesAllocated - offset, "%s:SetEnabled( true );\n", m_pGameObject->GetName() );

        return offset - startOffset;
    }
};

#endif //__VisualScriptNodes_H__
