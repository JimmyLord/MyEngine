//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "VisualScriptNodes.h"

#include "MyNodeGraph.h"
#include "MyNode.h"

#include "../../SourceCommon/ComponentSystem/BaseComponents/ComponentBase.h"
#include "../../SourceCommon/ComponentSystem/BaseComponents/ComponentVariable.h"
#include "../../SourceCommon/Core/EngineCore.h"

#undef AddVar
#define AddVar ComponentBase::AddVariable_Base

VisualScriptNodeTypeManager::VisualScriptNodeTypeManager()
: MyNodeTypeManager()
{
}

MyNodeGraph::MyNode* VisualScriptNodeTypeManager::AddCreateNodeItemsToContextMenu(Vector2 pos, MyNodeGraph* pNodeGraph)
{
    if( ImGui::BeginMenu( "Values" ) )
    {
        if( ImGui::MenuItem( "Float" ) )                { ImGui::EndMenu(); return CreateNode( "Value_Float", pos, pNodeGraph ); }
        if( ImGui::MenuItem( "Color" ) )                { ImGui::EndMenu(); return CreateNode( "Value_Color", pos, pNodeGraph ); }
        if( ImGui::MenuItem( "GameObject" ) )           { ImGui::EndMenu(); return CreateNode( "Value_GameObject", pos, pNodeGraph ); }
        if( ImGui::MenuItem( "Component" ) )            { ImGui::EndMenu(); return CreateNode( "Value_Component", pos, pNodeGraph ); }
        ImGui::EndMenu();
    }

    if( ImGui::BeginMenu( "Math Operations" ) )
    {
        if( ImGui::MenuItem( "Add" ) )                  { ImGui::EndMenu(); return CreateNode( "MathOp_Add", pos, pNodeGraph ); }
        ImGui::EndMenu();
    }

    if( ImGui::BeginMenu( "Conditions" ) )
    {
        if( ImGui::MenuItem( "GreaterEqual" ) )         { ImGui::EndMenu(); return CreateNode( "Condition_GreaterEqual", pos, pNodeGraph ); }
        ImGui::EndMenu();
    }

    if( ImGui::BeginMenu( "Events" ) )
    {
        if( ImGui::MenuItem( "KeyPress" ) )             { ImGui::EndMenu(); return CreateNode( "Event_KeyPress", pos, pNodeGraph ); }
        ImGui::EndMenu();
    }

    if( ImGui::BeginMenu( "Actions" ) )
    {
        if( ImGui::MenuItem( "Disable GameObject" ) )   { ImGui::EndMenu(); return CreateNode( "Disable_GameObject", pos, pNodeGraph ); }
        ImGui::EndMenu();
    }

    return nullptr;
}

MyNodeGraph::MyNode* VisualScriptNodeTypeManager::CreateNode(const char* typeName, Vector2 pos, MyNodeGraph* pNodeGraph)
{
    // TODO: Fix nonsense rand() for NodeID.
    uint32 newNodeID = rand();

#define TypeIs(name) strcmp( typeName, name ) == 0 )

    if( TypeIs( "Value_Float" )             return MyNew VisualScriptNode_Value_Float(              pNodeGraph, newNodeID, "Float",             pos, 0.5f );
    if( TypeIs( "Value_Color" )             return MyNew VisualScriptNode_Value_Color(              pNodeGraph, newNodeID, "Color",             pos, ColorByte(255, 255, 255, 255) );
    if( TypeIs( "Value_GameObject" )        return MyNew VisualScriptNode_Value_GameObject(         pNodeGraph, newNodeID, "GameObject",        pos, nullptr );
    if( TypeIs( "Value_Component" )         return MyNew VisualScriptNode_Value_Component(          pNodeGraph, newNodeID, "Component",         pos, nullptr );
    if( TypeIs( "MathOp_Add" )              return MyNew VisualScriptNode_MathOp_Add(               pNodeGraph, newNodeID, "Add",               pos );
    if( TypeIs( "Condition_GreaterEqual" )  return MyNew VisualScriptNode_Condition_GreaterEqual(   pNodeGraph, newNodeID, "GreaterEqual",      pos );
    if( TypeIs( "Event_KeyPress" )          return MyNew VisualScriptNode_Event_KeyPress(           pNodeGraph, newNodeID, "KeyPress",          pos, 'Z' );
    if( TypeIs( "Disable_GameObject" )      return MyNew VisualScriptNode_Disable_GameObject(       pNodeGraph, newNodeID, "DisableGameObject", pos, nullptr );

#undef TypeIs

    MyAssert( false );

    return nullptr;
}

//====================================================================================================
// Macros to emit code.
//====================================================================================================

#define Emit(numTabs, ...) \
do \
{ \
    for( int i=0; i<static_cast<int>( numTabs ); i++ ) \
    { \
        offset += sprintf_s( &string[offset], bytesAllocated - offset, "\t" ); \
    } \
    offset += sprintf_s( &string[offset], bytesAllocated - offset, __VA_ARGS__ ); \
} while( false )

#define EmitNode(node, numTabs) \
do \
{ \
    offset += node->EmitLua( string, offset, bytesAllocated, numTabs ); \
} \
while ( false )

char* EmitNodeTemp(VisualScriptNode* pNode)
{
    char* temp = static_cast<char*>( g_pEngineCore->GetSingleFrameMemoryStack()->AllocateBlock( 32 ) );
    temp[0] = '\0';
    pNode->EmitLua( temp, 0, 32, 0 );
    return temp;
};

//====================================================================================================
// Nodes and overrides for emitting Lua script.
//====================================================================================================

uint32 VisualScriptNode_Value_Float::EmitLua(char* string, uint32 offset, uint32 bytesAllocated, uint32 tabDepth)
{
    int startOffset = offset;
    Emit( 0, "(%f)", m_Float );
    return offset - startOffset;
}

//====================================================================================================
//====================================================================================================

uint32 VisualScriptNode_MathOp_Add::EmitLua(char* string, uint32 offset, uint32 bytesAllocated, uint32 tabDepth)
{
    int startOffset = offset;

    VisualScriptNode* pNode1 = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToInput( m_ID, 0 );
    VisualScriptNode* pNode2 = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToInput( m_ID, 1 );

    if( pNode1 == nullptr || pNode2 == nullptr )
    {
        const char* errorMessage = "ERROR_MathOp_Add";
        Emit( 0, errorMessage );
        return strlen( errorMessage );
    }

    char* node1String = EmitNodeTemp( pNode1 );
    char* node2String = EmitNodeTemp( pNode2 );
    Emit( 0, "( %s + %s )", node1String, node2String );

    return offset - startOffset;
}

//====================================================================================================
//====================================================================================================

uint32 VisualScriptNode_Condition_GreaterEqual::EmitLua(char* string, uint32 offset, uint32 bytesAllocated, uint32 tabDepth)
{
    int startOffset = offset;

    VisualScriptNode* pNode1 = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToInput( m_ID, 1 );
    VisualScriptNode* pNode2 = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToInput( m_ID, 2 );

    if( pNode1 == nullptr || pNode2 == nullptr )
    {
        const char* errorMessage = "ERROR_Condition_GreaterEqual";
        Emit( 0, errorMessage );
        return strlen( errorMessage );
    }

    // Always emit an "if( condition ) then" block.
    {
        char* node1String = EmitNodeTemp( pNode1 );
        char* node2String = EmitNodeTemp( pNode2 );
        Emit( tabDepth, "if( %s <= %s ) then\n", node1String, node2String );

        int count = 0;
        while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 0, count++ ) )
        {
            EmitNode( pNode, tabDepth+1 );
        }
    }

    // If there are any nodes connected to the 2nd output, then emit an else block.
    if( m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 0 ) )
    {
        Emit( tabDepth, "else\n" );

        int count = 0;
        while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 1, count++ ) )
        {
            EmitNode( pNode, tabDepth+1 );
        }
    }

    Emit( tabDepth, "end\n" );

    return offset - startOffset;
}

//====================================================================================================
//====================================================================================================

uint32 VisualScriptNode_Event_KeyPress::ExportAsLuaString(char* string, uint32 offset, uint32 bytesAllocated)
{
    int startOffset = offset;

    uint32 tabDepth = 0;

    Emit( tabDepth, "OnButtons = function(action, id)\n" );

    int count = 0;
    while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 0, count++ ) )
    {
        EmitNode( pNode, tabDepth+1 );
    }

    Emit( tabDepth, "end,\n" );

    return offset - startOffset;
}

//====================================================================================================
//====================================================================================================

uint32 VisualScriptNode_Disable_GameObject::EmitLua(char* string, uint32 offset, uint32 bytesAllocated, uint32 tabDepth)
{
    int startOffset = offset;

    if( m_pGameObject == nullptr )
    {
        const char* errorMessage = "ERROR_Disable_GameObject";
        Emit( 0, errorMessage );
        return strlen( errorMessage );
    }

    if( m_pGameObject->IsEnabled() )
    {
        Emit( tabDepth, "%s:SetEnabled( false );\n", m_pGameObject->GetName() );
    }
    else
    {
        Emit( tabDepth, "%s:SetEnabled( true );\n", m_pGameObject->GetName() );
    }

    return offset - startOffset;
}
