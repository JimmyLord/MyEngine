#ifndef __ImGuiManager_H__
#define __ImGuiManager_H__

class ImGuiManager;

extern ImGuiManager* g_pImGuiManager;

class ImGuiManager
{
public:
    ImGuiManager();
    virtual ~ImGuiManager();

    void Init();
    void Shutdown();

    void ClearInput();
    void StartFrame(double TimePassed);
    void EndFrame(float width, float height, bool draw);

    static void RenderDrawLists(ImDrawData* draw_data);
};

#endif //__ImGuiManager_H__
