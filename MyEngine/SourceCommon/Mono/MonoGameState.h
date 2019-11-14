//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MonoGameState_H__
#define __MonoGameState_H__

#if MYFW_USE_MONO

#include "mono/metadata/object-forward.h"
#include "mono/utils/mono-forward.h"

// Complete hack macros to shift the pointer passed from C# to the start of the data.
//#define MONO_VTABLE_AND_LOCK_BYTES 8
//#define fixPtr(type, x)     ((type*)(((char*)x)+MONO_VTABLE_AND_LOCK_BYTES))
//#define fixVec3(x)       ((Vector3*)(((char*)x)+MONO_VTABLE_AND_LOCK_BYTES))
//#define fixMat4(x)      ((MyMatrix*)(((char*)x)+MONO_VTABLE_AND_LOCK_BYTES))

class MonoGameState
{
public:
    // Globals used by C callback functions when creating new managed objects.
    static MonoDomain* g_pActiveDomain;
    static MonoImage* g_pMonoImage;

protected:
    EngineCore* m_pEngineCore;
    MonoDomain* m_pCoreDomain;

    MyFileObject* m_pDLLFile;

    MonoDomain* m_pActiveDomain;
    MonoImage* m_pMonoImage;

#if MYFW_EDITOR
    bool m_RebuildWhenCompileFinishes;
    JobWithCallbackFunction* m_pJob_RebuildDLL;
#endif

public:
    MonoGameState(EngineCore* pEngineCore);
    ~MonoGameState();

    // Getters.
    MonoDomain* GetActiveDomain() { return m_pActiveDomain; }
    MonoImage* GetImage() { return m_pMonoImage; }

    // Global State.
    // If multiple MonoGameStates exist, any registered C callback functions need to know which state to work with.
    // This means 2 mono functions from different core domains can't run simultaneously on 2 threads.
    void SetAsGlobalState();

#if MYFW_EDITOR
    void CheckForUpdatedScripts();
    void Tick();
    static void CompileDLL(void* pObject) { ((MonoGameState*)pObject)->CompileDLL(); }
    void CompileDLL();

    bool IsRebuilding();
#endif

    bool Rebuild();
};

#endif //MYFW_USE_MONO

#endif //__MonoGameState_H__
