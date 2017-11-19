/*---------------------------------------------------------
 * Copyright (C) Microsoft Corporation. All rights reserved.
 *--------------------------------------------------------*/

// from vscode-mock-debug
// will be customized to debug lua scripts in MyEngine

'use strict';

import * as vscode from 'vscode';
import { WorkspaceFolder, DebugConfiguration, ProviderResult, CancellationToken } from 'vscode';

export function activate(context: vscode.ExtensionContext)
{
	// register a configuration provider for 'myenginelua' debug type
	context.subscriptions.push(
		vscode.debug.registerDebugConfigurationProvider( 'myenginelua', new MyEngineLuaConfigurationProvider() ) );
}

export function deactivate()
{
	// nothing to do
}

class MyEngineLuaConfigurationProvider implements vscode.DebugConfigurationProvider
{
	/**
	 * Massage a debug configuration just before a debug session is being launched,
	 * e.g. add all missing attributes to the debug configuration.
	 */
	resolveDebugConfiguration( folder: WorkspaceFolder | undefined, config: DebugConfiguration, token?: CancellationToken ): ProviderResult<DebugConfiguration>
		{
			// if launch.json is missing or empty
			if( !config.type && !config.request && !config.name )
			{
				const editor = vscode.window.activeTextEditor;
				if( editor && editor.document.languageId === 'lua' )
				{
					config.type = 'myenginelua';
					config.name = 'Launch';
					config.request = 'launch';
					config.stopOnEntry = true;
				}
			}

			return config;
		}
}
