//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __VisualScriptNodeTypeManager_H__
#define __VisualScriptNodeTypeManager_H__

#include "MyNode.h"
#include "MyNodeGraph.h"
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
class VisualScriptNode_Condition_Keyboard;
class VisualScriptNode_Event_Keyboard;
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

#endif //__VisualScriptNodeTypeManager_H__
