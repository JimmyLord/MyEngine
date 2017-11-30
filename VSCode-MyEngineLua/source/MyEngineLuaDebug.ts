//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

import {
	DebugSession, LoggingDebugSession,
	InitializedEvent, TerminatedEvent, StoppedEvent, OutputEvent,
	Thread, StackFrame, Scope, Source, Handles, Breakpoint
} from 'vscode-debugadapter';
import { DebugProtocol } from 'vscode-debugprotocol';
import { basename } from 'path';
import * as net from 'net';

// This interface describes extension specific launch attributes (which are not part of the Debug Adapter Protocol).
// The schema for these attributes lives in the package.json of the myenginelua-debug extension.
// The interface should always match this schema.
interface LaunchRequestArguments extends DebugProtocol.LaunchRequestArguments
{
	stopOnEntry?: boolean; // Automatically attach to target after launch. If not specified, target does not stop.
	showDebugLog?: boolean; // Output some debug info on network traffic between this debugger and the game.
}

class MyEngineLuaDebugSession extends LoggingDebugSession
{
	// We don't support multiple threads, so we can use a hardcoded ID for the default thread.
	private static THREAD_ID = 1;

	private _variableHandles = new Handles<string>();
	private _socket: net.Socket;
	private _showDebugLog;

	// Nothing is dynamically requested, so this is the current state of the debugger.
	// Currently stores all stack frames/variables(local/this)/properties.
	private _lastJSONMessage;

	// Creates a new debug adapter that is used for one debug session.
	// We configure the default implementation of a debug adapter here.
	public constructor()
	{
		super( "myenginelua-debug.txt" );

		// This debugger uses zero-based lines and columns.
		this.setDebuggerLinesStartAt1( false );
		this.setDebuggerColumnsStartAt1( false );
	}

	// The 'initialize' request is the first request called by the frontend
	//     to interrogate the features the debug adapter provides.
	protected initializeRequest(response: DebugProtocol.InitializeResponse, args: DebugProtocol.InitializeRequestArguments): void
	{
		// Build and return the capabilities of this debug adapter:
		response.body = response.body || {};

		response.body.supportsConfigurationDoneRequest = true; // The adapter implements the configurationDoneRequest.
		response.body.supportsEvaluateForHovers = true;        // Make VS Code to use 'evaluate' when hovering over source.
		response.body.supportsStepBack = false;                // Make VS Code not show a 'step back' button.
		response.body.supportsRestartRequest = true;           // Tell VSCode we support restart requests.

		this.sendResponse(response);
	}

	protected dealWithIncomingData(data)
	{
		// Convert the data to a JSON object.
		var textChunk = data.toString('utf8');
		let jMessage = JSON.parse( textChunk );

		// If the JSON object doesn't have a defined type, ignore it.
		if( typeof jMessage.Type === 'undefined' )
			return;

		// Received a 'Stopped' message, could be on entry/stepin/stepout/stepover/pause/breakpoint.
		if( jMessage.Type == 'Stopped' )
		{
			this.logInfo( "Received 'Stopped'." );

			this.sendEvent( new StoppedEvent( 'breakpoint', MyEngineLuaDebugSession.THREAD_ID ) );

			// Stopped messages will contain the entire relevant lua state, so store a copy of the message.
			// We'll send the stack and variables to VSCode when it asks for it.
			// This also includes locals and vars inside 'this' object.
			this._lastJSONMessage = jMessage;
		}
	}

	protected TerminateDebugger()
	{
		// Close our socket.
		this._socket.end();

		// Terminate debugger.
		this.sendEvent( new TerminatedEvent() );
	}

	protected launchRequest(response: DebugProtocol.LaunchResponse, args: LaunchRequestArguments): void
	{
		this._socket = new net.Socket();
		this._socket.connect( 19542, '127.0.0.1' );

		this._socket.on( 'connect', () =>
			{
				this.logInfo( 'Connected to Game.' );

				if( !!args.stopOnEntry )
				{
					// Send a 'stepin' command to the Game, which will pause execution.
					this._socket.write( "stepin" + '\n' );
					this.logInfo( "Sending 'stepin'." );
				}
			}
		);
		this._socket.on( 'data', data => { this.dealWithIncomingData( data ); } );
		this._socket.on( 'error', () => { this.TerminateDebugger() } );
		this._socket.on( 'close', () => { this.TerminateDebugger() } );

		this.sendResponse(response);

		this._showDebugLog = !!args.showDebugLog;

		// Since this debug adapter can accept configuration requests like 'setBreakpoint' at any time,
		//     we request them early by sending an 'initializeRequest' to the frontend.
		// The frontend will end the configuration sequence by calling 'configurationDone' request.
		this.sendEvent( new InitializedEvent() );
	}

	protected setBreakPointsRequest(response: DebugProtocol.SetBreakpointsResponse, args: DebugProtocol.SetBreakpointsArguments): void
	{
		this.logInfo( "setBreakPointsRequest." );

		if( typeof this._socket === 'undefined' )
			return;

		const path = <string>args.source.path;
		const clientLines = args.lines || [];

		// Clear all breakpoints for this file.
		var request =  { "command":"breakpoint_ClearAllFromFile", "file":path };
		this._socket.write( JSON.stringify( request ) + '\n' );
		this.logInfo( "Sending 'breakpoint_ClearAllFromFile': " + path + "." );

		// Set and verify breakpoint locations.
		const actualBreakpoints = clientLines.map( line =>
			{
				var request =  { "command":"breakpoint_Set", "file":path, "line":line };
				this._socket.write( JSON.stringify( request ) + '\n' );
				this.logInfo( "Sending 'breakpoint_Set': " + path + "(" + line + ")." );

				let verified = true;
				const bp = <DebugProtocol.Breakpoint>new Breakpoint( verified, line );
				return bp;
			}
		);

		// Send back the actual breakpoint positions.
		response.body = { breakpoints: actualBreakpoints };
		this.sendResponse(response);
	}

	protected threadsRequest(response: DebugProtocol.ThreadsResponse): void
	{
		this.logInfo( "threadsRequest." );

		// No thread support, so just return a default thread.
		response.body = {
			threads: [
				new Thread( MyEngineLuaDebugSession.THREAD_ID, "thread 1" )
			]
		};
		this.sendResponse(response);
	}

	protected stackTraceRequest(response: DebugProtocol.StackTraceResponse, args: DebugProtocol.StackTraceArguments): void
	{
		this.logInfo( "stackTraceRequest." );

		if( typeof this._lastJSONMessage === 'undefined' )
		{
			this.sendErrorResponse( response, 0, "No Lua script running." );
			return;
		}

		let jMessage = this._lastJSONMessage;

		if( jMessage.StackNumLevels == 0 )
		{
			this.sendErrorResponse( response, 0, "No Lua script running." );
		}
		else
		{
			var frames = new Array<any>();
			for( let i=0; i<jMessage.StackNumLevels; i++ )
			{
				let functionname = jMessage.StackFrames[i].FunctionName;
				let source = this.createSource( jMessage.StackFrames[i].Source );
				let line = jMessage.StackFrames[i].Line;

				var stackframe = new StackFrame( i, functionname, source, line, 0 );
				frames.push( stackframe );
			}

			response.body =
			{
				stackFrames: frames,
				totalFrames: 1
			};

			this.sendResponse( response );
		}
	}

	protected scopesRequest(response: DebugProtocol.ScopesResponse, args: DebugProtocol.ScopesArguments): void
	{
		this.logInfo( "scopesRequest." );

		const frameReference = args.frameId;
		const scopes = new Array<Scope>();
		scopes.push( new Scope( "Local", this._variableHandles.create("Local_" + frameReference), false ) );
		scopes.push( new Scope( "This", this._variableHandles.create("This_" + frameReference), false ) );
		scopes.push( new Scope( "Global", this._variableHandles.create("Global_" + frameReference), false ) );

		response.body = { scopes: scopes };
		this.sendResponse( response );
	}

	protected variablesRequest(response: DebugProtocol.VariablesResponse, args: DebugProtocol.VariablesArguments): void
	{
		this.logInfo( "variablesRequest." );

		let jMessage = this._lastJSONMessage;

		const variables = new Array<DebugProtocol.Variable>();

		const id = this._variableHandles.get( args.variablesReference );
		if( id !== null )
		{
			var pieces = id.split( "_" ); // This_0, Local_0, Global_0, etc...
			var scope = pieces[0];
			var frameindex = pieces[1];

			if( scope == 'props' )
			{
				// jsonprefix should look like: StackFrames[frameindex][scope][i].Properties
				let jsonprefix = pieces[1];

				let jPropsArray = this.findJSONObjectByString( jMessage, jsonprefix );

				for( let i=0; i<jPropsArray.length; i++ )
				{
					let varname = jPropsArray[i].Name;
					let varvalue = "" + jPropsArray[i].Value;
					let varprops = jPropsArray[i].Properties;
					let hasprops = (typeof varprops !== 'undefined') ? true : false;
					let varpropsjsonprefix = `${jsonprefix}[${i}].Properties`;

					variables.push(
						{
							name: varname,
							type: "string",
							value: "" + varvalue,
							//evaluateName:
							variablesReference: hasprops ? this._variableHandles.create( `props_${varpropsjsonprefix}` ) : 0
						}
					);
				}
			}
			else if( typeof jMessage.StackFrames[frameindex][scope] !== 'undefined' )
			{
				for( let i=0; i<jMessage.StackFrames[frameindex][scope].length; i++ )
				{
					let varname = jMessage.StackFrames[frameindex][scope][i].Name;
					let varvalue = "" + jMessage.StackFrames[frameindex][scope][i].Value;
					let varprops = jMessage.StackFrames[frameindex][scope][i].Properties;
					let hasprops = (typeof varprops !== 'undefined') ? true : false;
					let varpropsjsonprefix = `StackFrames[${frameindex}][${scope}][${i}].Properties`;

					variables.push(
						{
							name: varname,
							type: "string",
							value: "" + varvalue,
							//evaluateName:
							variablesReference: hasprops ? this._variableHandles.create( `props_${varpropsjsonprefix}` ) : 0
						}
					);
				}
			}
		}

		response.body = { variables: variables };
		this.sendResponse( response );
	}

	protected restartRequest(response: DebugProtocol.RestartResponse, args: DebugProtocol.RestartArguments): void
	{
		this._socket.write( "restart" + '\n' );
		this.logInfo( "Sending 'restart'." );

		this.sendResponse( response );
	}

	protected continueRequest(response: DebugProtocol.ContinueResponse, args: DebugProtocol.ContinueArguments): void
	{
		this._socket.write( "continue" + '\n' );
		this.logInfo( "Sending 'continue'." );

		this.sendResponse( response );
	}

	protected nextRequest(response: DebugProtocol.NextResponse, args: DebugProtocol.NextArguments): void
	{
		this._socket.write( "stepover" + '\n' );
		this.logInfo( "Sending 'stepover'." );

		this.sendResponse(response);
	}

	protected stepInRequest(response: DebugProtocol.StepInResponse, args: DebugProtocol.StepInArguments): void
	{
		this._socket.write( "stepin" + '\n' );
		this.logInfo( "Sending 'stepin'." );

		this.sendResponse(response);
	}

	protected stepOutRequest(response: DebugProtocol.StepOutResponse, args: DebugProtocol.StepOutArguments): void
	{
		this._socket.write( "stepout" + '\n' );
		this.logInfo( "Sending 'stepout'." );

		this.sendResponse(response);
	}

	protected pauseRequest(response: DebugProtocol.PauseResponse, args: DebugProtocol.PauseArguments): void
	{
		// No difference between stepin and pause.
		this._socket.write( "stepin" + '\n' );
		this.logInfo( "Sending 'stepin'." );

		this.sendResponse(response);
	}

	protected disconnectRequest(response: DebugProtocol.DisconnectResponse, args: DebugProtocol.DisconnectArguments): void
	{
		this.logInfo( "Terminating debugger." );

		this.TerminateDebugger();
		this.sendResponse(response);
	}

	protected evaluateRequest(response: DebugProtocol.EvaluateResponse, args: DebugProtocol.EvaluateArguments): void
	{
		this.logInfo( "evaluateRequest." );

		let reply: string | undefined = undefined;
		let varRef = 0;

		if( typeof args.frameId !== 'undefined' )
		{
			let jMessage = this._lastJSONMessage;
			let frameindex = args.frameId;
			let scope = 'Local';
			if( typeof jMessage.StackFrames[frameindex][scope] !== 'undefined' )
			{
				for( let i=0; i<jMessage.StackFrames[frameindex][scope].length; i++ )
				{
					if( jMessage.StackFrames[frameindex][scope][i].Name == args.expression )
					{
						let varvalue = "" + jMessage.StackFrames[frameindex][scope][i].Value;
						let varprops = jMessage.StackFrames[frameindex][scope][i].Properties;
						let hasprops = (typeof varprops !== 'undefined') ? true : false;
						let varpropsjsonprefix = `StackFrames[${frameindex}][${scope}][${i}].Properties`;

						reply = varvalue;
						if( hasprops )
						{
							varRef = this._variableHandles.create( `props_${varpropsjsonprefix}` )
						}
					}
				}
			}
			scope = 'This';
			if( typeof jMessage.StackFrames[frameindex][scope] !== 'undefined' )
			{
				for( let i=0; i<jMessage.StackFrames[frameindex][scope].length; i++ )
				{
					if( "this." + jMessage.StackFrames[frameindex][scope][i].Name == args.expression )
					{
						let varvalue = "" + jMessage.StackFrames[frameindex][scope][i].Value;
						let varprops = jMessage.StackFrames[frameindex][scope][i].Properties;
						let hasprops = (typeof varprops !== 'undefined') ? true : false;
						let varpropsjsonprefix = `StackFrames[${frameindex}][${scope}][${i}].Properties`;

						reply = varvalue;
						if( hasprops )
						{
							varRef = this._variableHandles.create( `props_${varpropsjsonprefix}` )
						}
					}
				}
			}
		}

		if( reply )
		{
			response.body =
			{
				result: reply,
				variablesReference: varRef,
			};
		}

		this.sendResponse( response );
	}

	//----------------------------------------------
	//---- Helpers
	//----------------------------------------------

	private createSource(filePath: string): Source
	{
		return new Source( basename(filePath), this.convertDebuggerPathToClient(filePath), undefined, undefined, 'myenginelua-adapter-data' );
	}

	private logInfo(message)
	{
		if( this._showDebugLog )
			this.sendEvent( new OutputEvent( message + '\n' ) );
	}

	// Code found here and reformatted:
	// https://stackoverflow.com/questions/6491463/accessing-nested-javascript-objects-with-string-key
	private findJSONObjectByString = function(o, s)
	{
		s = s.replace( /\[(\w+)\]/g, '.$1' ); // Convert indexes to properties.
		s = s.replace( /^\./, '' );           // Strip a leading dot.
		var a = s.split( '.' );

		for( var i=0, n=a.length; i<n; ++i )
		{
			var k = a[i];

			if( k in o )
				o = o[k];
			else
				return;
		}

		return o;
	}
}

DebugSession.run( MyEngineLuaDebugSession );
