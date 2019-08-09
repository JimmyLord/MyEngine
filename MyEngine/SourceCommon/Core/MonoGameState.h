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

#include "mono/metadata/object-forward.h"
#include "mono/utils/mono-forward.h"

class MonoGameState
{
protected:
    EngineCore* m_pEngineCore;
    MonoDomain* m_pCoreDomain;

    MyFileObject* m_pDLLFile;

    MonoDomain* m_pActiveDomain;
    MonoImage* m_pMonoImage;

public:
    MonoGameState(EngineCore* pEngineCore);
    ~MonoGameState();

    // Getters.
    MonoDomain* GetActiveDomain() { return m_pActiveDomain; }
    MonoImage* GetImage() { return m_pMonoImage; }

#if MYFW_EDITOR
    void CheckForUpdatedScripts();
    void CompileDLL();
#endif

    void Rebuild();
};

#endif //__MonoGameState_H__
