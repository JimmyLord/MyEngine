//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentVariable.h"

ComponentVariable::ComponentVariable(const char* label, ComponentVariableTypes type, size_t offset, bool saveload, bool displayinwatch, const char* watchlabel,
    CVarFunc_GetPointerValue pGetPointerValueCallBackFunc, CVarFunc_SetPointerValue pSetPointerValueCallBackFunc,
    CVarFunc_GetPointerDesc pGetPointerDescCallBackFunc, CVarFunc_SetPointerDesc pSetPointerDescCallBackFunc,
    CVarFunc_ValueChanged pOnValueChangedCallBackFunc, CVarFunc_DropTarget pOnDropCallBackFunc, CVarFunc pOnButtonPressedCallBackFunc )
{
    m_Label = label;
    m_Type = type;
    m_Offset = offset;
    m_SaveLoad = saveload;
    m_DisplayInWatch = displayinwatch;
    m_WatchLabel = watchlabel;
    if( m_WatchLabel == 0 )
        m_WatchLabel = label;
    m_NumEnumStrings = 0;
    m_ppEnumStrings = 0;

#if MYFW_EDITOR
    m_FloatLowerLimit = 0;
    m_FloatUpperLimit = 0;

    m_pOnDropCallbackFunc = pOnDropCallBackFunc;
    m_pOnButtonPressedCallbackFunc = pOnButtonPressedCallBackFunc;
    m_pOnValueChangedCallbackFunc = pOnValueChangedCallBackFunc;

    m_pShouldVariableBeAddedCallbackFunc = 0;
    m_pVariableAddedToInterfaceCallbackFunc = 0;
    m_pOnRightClickCallbackFunc = 0;
    m_pOnPopupClickCallbackFunc = 0;
#endif //MYFW_EDITOR

    m_pGetPointerValueCallBackFunc = pGetPointerValueCallBackFunc;
    m_pSetPointerValueCallBackFunc = pSetPointerValueCallBackFunc;
    m_pGetPointerDescCallBackFunc = pGetPointerDescCallBackFunc;
    m_pSetPointerDescCallBackFunc = pSetPointerDescCallBackFunc;

    m_ControlID = -1;
    m_Index = -1;
}

#if MYFW_EDITOR
void ComponentVariable::AddCallback_ShouldVariableBeAdded(CVarFunc_ShouldVariableBeAdded pFunc)
{
    m_pShouldVariableBeAddedCallbackFunc = pFunc;
}

void ComponentVariable::AddCallback_VariableAddedToInterface(CVarFunc_VariableAddedToInterface pFunc)
{
    m_pVariableAddedToInterfaceCallbackFunc = pFunc;
}

void ComponentVariable::AddCallback_OnRightClick(CVarFunc_wxMenu pRightClickFunc, CVarFunc_Int pPopupClickFunc)
{
    m_pOnRightClickCallbackFunc = pRightClickFunc;
    m_pOnPopupClickCallbackFunc = pPopupClickFunc;
}

void ComponentVariable::SetEditorLimits(float lowerlimit, float upperlimit)
{
    m_FloatLowerLimit = lowerlimit;
    m_FloatUpperLimit = upperlimit;
}
#endif //MYFW_EDITOR
