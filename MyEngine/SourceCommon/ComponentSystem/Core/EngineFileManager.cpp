//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "EngineFileManager.h"

EngineFileManager* g_pEngineFileManager = 0;

EngineFileManager::EngineFileManager()
{
    MyAssert( g_pEngineFileManager == 0 );
    g_pEngineFileManager = this;
}

EngineFileManager::~EngineFileManager()
{
}

MyFileObject* EngineFileManager::RequestFile(const char* filename)
{
    // add any files requested to the list of files needed by the (first) scene.
    MyFileObject* pFile = RequestFile( filename, SCENEID_MainScene );

    return pFile;
}

MyFileObject* EngineFileManager::RequestFile(const char* filename, SceneID sceneid)
{
    MyFileObject* pFile = 0;

    //// TODO: Have the ComponentSystemManager add the file to the list of active files for the scene.
    //if( 0 )//g_pComponentSystemManager )
    //{
    //    pFile = g_pComponentSystemManager->LoadDataFile( filename, sceneid, 0, true );

    //    // LoadDataFile doesn't add a ref, so add one.
    //    pFile->AddRef();
    //}
    //else
    {
        // just load the file as normal.
        pFile = FileManager::RequestFile( filename );
    }

    
    return pFile;
}

MyFileObject* EngineFileManager::RequestFile_UntrackedByScene(const char* filename)
{
    // Load the resource, but don't track it as scene data file.
    MyFileObject* pFile = FileManager::RequestFile( filename );

    return pFile;
}
