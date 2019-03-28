//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorDocument_H__
#define __EditorDocument_H__

class EditorDocument
{
protected:
    enum EditorDocumentMenuCommands
    {
        EditorDocumentMenuCommand_Undo,
        EditorDocumentMenuCommand_Redo,
        EditorDocumentMenuCommand_Load,
        EditorDocumentMenuCommand_Save,
        EditorDocumentMenuCommand_SaveAs,
        //EditorDocumentMenuCommand_SaveAll,
    };

protected:
    EngineCore* m_pEngineCore;

    CommandStack* m_pCommandStack;
    uint32 m_UndoStackDepthAtLastSave;
    bool m_SaveRequested;

    char m_RelativePath[MAX_PATH];
    const char* m_Filename; // Pointer to the end part of m_RelativePath;

protected:
    EditorDocument* EditorDocumentMenuCommand(EditorDocumentMenuCommands command);
    EditorDocument* EditorDocumentMenuCommand(EditorDocumentMenuCommands command, std::string value);

public:
    EditorDocument(EngineCore* pEngineCore);
    virtual ~EditorDocument();

    static EditorDocument* AddDocumentMenu(EngineCore* pEngineCore, EditorDocument* pDocument);
    virtual bool HandleInput(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure);
    virtual void Save();
    virtual void Load();

    EngineCore* GetEngineCore() { return m_pEngineCore; }

    void SetRelativePath(const char* relativePath);
    const char* GetRelativePath();
    const char* GetFilename();

    bool HasUnsavedChanges();
};

#endif //__EditorDocument_H__
