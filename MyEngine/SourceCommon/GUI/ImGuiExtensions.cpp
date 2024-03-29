//
// Copyright (c) 2018-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ImGuiExtensions.h"

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../../Libraries/imgui/imgui_internal.h"

extern IMGUI_API ImGuiContext* GImGui;

namespace ImGuiExt
{

bool IsMouseHoveringOverItemExpansionArrow()
{
    float arrowIconWidthWithSpacing = 20.0f; // TODO: This shouldn't be hard-coded.

    if( ImGui::GetMousePos().x < ImGui::GetItemRectMin().x + arrowIconWidthWithSpacing )
        return true;

    return false;
}

ImGuiID GetActiveItemId()
{
    ImGuiContext& g = *GImGui;
    
    return g.ActiveId;
}

ImGuiID GetLastItemId()
{
    ImGuiContext& g = *GImGui;

    ImGuiWindow* window = g.CurrentWindow;
    if( window )
    {
        assert( false );
        //return window->DC.LastItemId;
    }

    return 0;
}

void SetStyleColorVec4(ImGuiCol idx, ImVec4& color)
{ 
    ImGuiStyle& style = GImGui->Style;
    style.Colors[idx] = color;
}

ImVec2 GetWindowSize(const char* name)
{
    if( ImGuiWindow* window = ImGui::FindWindowByName( name ) )
        return window->Size;

    return ImVec2(-1,-1);
}

ImVec2 GetWindowPos(const char* name)
{
    if( ImGuiWindow* window = ImGui::FindWindowByName( name ) )
        return window->Pos;

    return ImVec2(-1,-1);
}

void ClearDragDrop()
{
    ImGui::ClearDragDrop();
}

// Copied from ImGui::Separator and modified.
void DrawBlock(float offsetX, float offsetY, float sizeX, float sizeY, ImGuiCol colorIndex)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;
    ImGuiContext& g = *GImGui;

    float x1 = window->Pos.x + offsetX;
    float x2 = window->Pos.x + offsetX + sizeX;
    float y1 = window->DC.CursorPos.y + offsetY;
    float y2 = window->DC.CursorPos.y + offsetY + sizeY;

    const ImRect bb( ImVec2(x1, y1), ImVec2(x2, y2) );

    window->DrawList->AddLine( bb.Min, ImVec2(bb.Max.x,bb.Min.y), ImGui::GetColorU32(colorIndex), sizeY );
}

bool ButtonWithTooltip(const char* label, const ImVec2& size, const char* tooltipLabel)
{
    bool wasPressed = ImGui::Button( label, size );
    
    if( ImGui::IsItemHovered() )
    {
        ImGui::BeginTooltip();
        ImGui::Text( tooltipLabel );
        ImGui::EndTooltip();
    }

    return wasPressed;
}

}
