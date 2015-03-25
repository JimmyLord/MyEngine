//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentTypeManager_H__
#define __ComponentTypeManager_H__

class ComponentBase;
class ComponentTypeManager;

extern ComponentTypeManager* g_pComponentTypeManager;

struct ComponentTypeInfo
{
    const char* category;
    const char* name;
};

class ComponentTypeManager
{
public:

public:
    ComponentTypeManager();
    virtual ~ComponentTypeManager();

    virtual ComponentBase* CreateComponent(int type) = 0;
    virtual unsigned int GetNumberOfComponentTypes() = 0;

    virtual const char* GetTypeCategory(int type) = 0;
    virtual const char* GetTypeName(int type) = 0;

    virtual int GetTypeByName(const char* name) = 0;
};

#endif //__ComponentTypeManager_H__
