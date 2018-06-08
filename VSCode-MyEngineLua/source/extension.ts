/*---------------------------------------------------------
 * Copyright (C) Microsoft Corporation. All rights reserved.
 *--------------------------------------------------------*/

// from vscode-mock-debug
// customized to debug lua scripts in MyEngine

'use strict';

import * as vscode from 'vscode';
import { WorkspaceFolder, DebugConfiguration, ProviderResult, CancellationToken } from 'vscode';
import { MyEngineLuaDebugSession } from './MyEngineLuaDebug';
import * as Net from 'net';

// Set the following compile time flag to true if the debug adapter should run inside the extension host.
// Please note: the test suite no longer works in this mode.
const EMBED_DEBUG_ADAPTER = false;

export function activate(context: vscode.ExtensionContext)
{
	// register a configuration provider for 'myenginelua' debug type
	const provider = new MyEngineLuaConfigurationProvider();
	context.subscriptions.push(	vscode.debug.registerDebugConfigurationProvider( 'myenginelua', provider ) );
	context.subscriptions.push( provider );
}

export function deactivate()
{
	// nothing to do
}

class MyEngineLuaConfigurationProvider implements vscode.DebugConfigurationProvider
{
	private _server?: Net.Server;

	// Massage a debug configuration just before a debug session is being launched,
	// e.g. add all missing attributes to the debug configuration.
	resolveDebugConfiguration( folder: WorkspaceFolder | undefined, config: DebugConfiguration, token?: CancellationToken ): ProviderResult<DebugConfiguration>
	{
		// If launch.json is missing or empty.
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

		if( EMBED_DEBUG_ADAPTER )
		{
			// Start port listener on launch of first debug session.
			if( !this._server )
			{
				// start listening on a random port
				this._server = Net.createServer( socket => {
					const session = new MyEngineLuaDebugSession();
					session.setRunAsServer( true );
					session.start( <NodeJS.ReadableStream>socket, socket );
				} ).listen(0);
			}

			// Make VS Code connect to debug server instead of launching debug adapter.
			config.debugServer = this._server.address().port;
		}

		return config;
	}

	dispose()
	{
		if( this._server )
		{
			this._server.close();
		}
	}
}
