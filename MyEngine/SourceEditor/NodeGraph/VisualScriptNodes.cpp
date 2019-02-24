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
        if( ImGui::MenuItem( "Float" ) )     { ImGui::EndMenu(); return CreateNode( "Float", pos, pNodeGraph ); }
        if( ImGui::MenuItem( "Color" ) )     { ImGui::EndMenu(); return CreateNode( "Color", pos, pNodeGraph ); }
        if( ImGui::MenuItem( "Component" ) ) { ImGui::EndMenu(); return CreateNode( "Component", pos, pNodeGraph ); }
        ImGui::EndMenu();
    }

    if( ImGui::BeginMenu( "Math Operations" ) )
    {
        if( ImGui::MenuItem( "Add" ) )       { ImGui::EndMenu(); return CreateNode( "Add", pos, pNodeGraph ); }
        ImGui::EndMenu();
    }

    return nullptr;
}

MyNodeGraph::MyNode* VisualScriptNodeTypeManager::CreateNode(const char* typeName, Vector2 pos, MyNodeGraph* pNodeGraph)
{
    if( strcmp( typeName, "Float" ) == 0 )
        return MyNew VisualScriptNode_Float( pNodeGraph, rand(), "Float Holder", pos, 0.5f );
    if( strcmp( typeName, "Color" ) == 0 )
        return MyNew VisualScriptNode_Color( pNodeGraph, rand(), "Color Holder", pos, ColorByte(255, 255, 255, 255) );
    if( strcmp( typeName, "Component" ) == 0 )
        return MyNew VisualScriptNode_Component( pNodeGraph, rand(), "Component Holder", pos, nullptr );
    if( strcmp( typeName, "Add" ) == 0 )
        return MyNew VisualScriptNode_Add( pNodeGraph, rand(), "Add", pos );

    MyAssert( false );

    return nullptr;
}
