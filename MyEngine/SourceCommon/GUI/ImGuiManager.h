//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ImGuiManager_H__
#define __ImGuiManager_H__

class ImGuiManager;

class ImGuiManager
{
protected:
    // Shared ImGui Resources.
    static bool m_DeviceObjectsAreValid;
    static int m_ShaderHandle;
    static int m_VertHandle;
    static int m_FragHandle;
    static int m_UniformLocationTex;
    static int m_UniformLocationProjMtx;
    static int m_AttribLocationPosition;
    static int m_AttribLocationUV;
    static int m_AttribLocationColor;
    static unsigned int m_VboHandle;
    static unsigned int m_VaoHandle;
    static unsigned int m_ElementsHandle;
    static TextureDefinition* m_pFontTexture;

protected:
    static bool CreateDeviceObjects();
    static void CreateFont();
    static bool CreateFontAndTexture();
    static void InvalidateDeviceObjects();

protected:
    // Instance ImGui Resources.
    void ClearInput();

    ImGuiContext* m_pImGuiContext;
    bool m_FrameStarted;
    ImGuiMouseCursor m_LastMouseCursor;

public:
    ImGuiManager();
    virtual ~ImGuiManager();

    void Init(float width, float height);
    void Shutdown(bool invalidateDeviceObjects = true);

    ImGuiContext* GetImGuiContext() { return m_pImGuiContext; }

    void OnFocusLost();

    bool HandleInput(int keyaction, int keycode, int mouseaction, int id, float x, float y, float pressure);
    void OnChar(unsigned int c);

    void StartTick(float deltaTime);
    void StartFrame();
    void EndFrame(float width, float height, bool draw);

    void RenderDrawLists(ImDrawData* draw_data);

    bool UpdateMouseCursor();
};

#endif //__ImGuiManager_H__
