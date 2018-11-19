//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __EditorIcons_H__
#define __EditorIcons_H__

// UTF8 reference:
// Unicode value     UTF8 byte code
//                   1st byte - 2nd byte - 3rd byte
// U+E000..U+FFFF    EE..EF     80..BF     80..BF

// MyEditorIcons - Created with Inkscape, exported as svgs and ttf built with http://fontello.com/
#define EditorIconData_Filename "Data/DataEngine/Fonts/MyEditorIcons.ttf"
#define EditorIconData_First    0xE800
#define EditorIconData_Last     0xE802

#define EditorIcon_GameObject   "\xEE\xA0\x80" // 0xe800 gameobject
#define EditorIcon_Folder       "\xEE\xA0\x81" // 0xe801 folder
#define EditorIcon_Prefab       "\xEE\xA0\x82" // 0xe802 prefab

#endif //__EditorIcons_H__
