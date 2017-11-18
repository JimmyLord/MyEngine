/*---------------------------------------------------------
 * Copyright (C) Microsoft Corporation. All rights reserved.
 *--------------------------------------------------------*/

// from vscode-mock-debug
// will be customized to debug lua scripts in MyEngine

import {
	Logger, logger,
	DebugSession, LoggingDebugSession,
	InitializedEvent, TerminatedEvent, StoppedEvent, BreakpointEvent, //OutputEvent,
	Thread, StackFrame, Scope, Source, Handles, Breakpoint
} from 'vscode-debugadapter';
import { DebugProtocol } from 'vscode-debugprotocol';
import { basename } from 'path';
import { MyEngineLuaRuntime, MyEngineLuaBreakpoint } from './MyEngineLuaRuntime';
import * as net from 'net';

/**
 * This interface describes the myenginelua-debug specific launch attributes
 * (which are not part of the Debug Adapter Protocol).
 * The schema for these attributes lives in the package.json of the myenginelua-debug extension.
 * The interface should always match this schema.
 */
interface LaunchRequestArguments extends DebugProtocol.LaunchRequestArguments {
	/** An absolute path to the "program" to debug. */
	program: string;
	/** Automatically stop target after launch. If not specified, target does not stop. */
	stopOnEntry?: boolean;
	/** enable logging the Debug Adapter Protocol */
	trace?: boolean;
}

class MyEngineLuaDebugSession extends LoggingDebugSession
{
	// we don't support multiple threads, so we can use a hardcoded ID for the default thread
	private static THREAD_ID = 1;

	// a MyEngineLua runtime (or debugger)
	private _runtime: MyEngineLuaRuntime;

	private _variableHandles = new Handles<string>();

	private _socket: net.Socket;

	private _lastStackJSONMessage;

	//private _stackTraceResponse: DebugProtocol.StackTraceResponse;
	//private _waitingToSendStackTraceResponse = false;

	/**
	 * Creates a new debug adapter that is used for one debug session.
	 * We configure the default implementation of a debug adapter here.
	 */
	public constructor()
	{
		super("myenginelua-debug.txt");

		// this debugger uses zero-based lines and columns
		this.setDebuggerLinesStartAt1(false);
		this.setDebuggerColumnsStartAt1(false);

		this._runtime = new MyEngineLuaRuntime();

		// setup event handlers
		// this._runtime.on('stopOnEntry', () => {
		//  	this.sendEvent(new StoppedEvent('entry', MyEngineLuaDebugSession.THREAD_ID));
		// });
		// this._runtime.on('stopOnStep', () => {
		// 	this.sendEvent(new StoppedEvent('step', MyEngineLuaDebugSession.THREAD_ID));
		// });
		// this._runtime.on('stopOnBreakpoint', () => {
		// 	this.sendEvent(new StoppedEvent('breakpoint', MyEngineLuaDebugSession.THREAD_ID));
		// });
		// this._runtime.on('stopOnException', () => {
		// 	this.sendEvent(new StoppedEvent('exception', MyEngineLuaDebugSession.THREAD_ID));
		// });
		this._runtime.on('breakpointValidated', (bp: MyEngineLuaBreakpoint) => {
			this.sendEvent(new BreakpointEvent('changed', <DebugProtocol.Breakpoint>{ verified: bp.verified, id: bp.id }));
		});
		// this._runtime.on('output', (text, filePath, line, column) => {
		// 	const e: DebugProtocol.OutputEvent = new OutputEvent(`${text}\n`);
		// 	e.body.source = this.createSource(filePath);
		// 	e.body.line = this.convertDebuggerLineToClient(line);
		// 	e.body.column = this.convertDebuggerColumnToClient(column);
		// 	this.sendEvent(e);
		// });
	}

	/**
	 * The 'initialize' request is the first request called by the frontend
	 * to interrogate the features the debug adapter provides.
	 */
	protected initializeRequest(response: DebugProtocol.InitializeResponse, args: DebugProtocol.InitializeRequestArguments): void
	{
		// since this debug adapter can accept configuration requests like 'setBreakpoint' at any time,
		// we request them early by sending an 'initializeRequest' to the frontend.
		// The frontend will end the configuration sequence by calling 'configurationDone' request.
		this.sendEvent(new InitializedEvent());

		// build and return the capabilities of this debug adapter:
		response.body = response.body || {};

		// the adapter implements the configurationDoneRequest.
		response.body.supportsConfigurationDoneRequest = true;

		// make VS Code to use 'evaluate' when hovering over source
		response.body.supportsEvaluateForHovers = true;

		// Make VS Code not show a 'step back' button.
		response.body.supportsStepBack = false;

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

		// Received a 'Stopped' message, could be on entry or step.
		if( jMessage.Type == 'Stopped' )
		{
			console.log( "Received 'Stopped'." );

			this.sendEvent( new StoppedEvent( 'breakpoint', MyEngineLuaDebugSession.THREAD_ID ) );

			// Stopped messages will contain the entire stack, so store a copy of the message.
			// We'll send the stack to VSCode when it asks for it.
			this._lastStackJSONMessage = jMessage;
		}

		// // Received a StackInfo message, which is a direct answer to a stackTraceRequest
		// if( jMessage.Type == 'StackInfo' )
		// {
		// 	console.log( "Received 'StackInfo'." );

		// 	if( this._waitingToSendStackTraceResponse )
		// 	{
		// 		if( jMessage.StackNumLevels == 0 )
		// 		{
		// 			var frames = new Array<any>();
		// 			frames.push( new StackFrame( 0, "stack not available" ) );

		// 			this._stackTraceResponse.body =
		// 			{
		// 				stackFrames: frames,
		// 				totalFrames: 1
		// 			};

		// 			this.sendResponse( this._stackTraceResponse );
		// 		}
		// 		else
		// 		{
		// 			var stackframe = new StackFrame( 0, "functionName()", this.createSource(jMessage.Source), jMessage.Line, 0 );

		// 			var frames = new Array<any>();
		// 			frames.push( stackframe );

		// 			this._stackTraceResponse.body =
		// 			{
		// 				stackFrames: frames,
		// 				totalFrames: 1
		// 			};

		// 			this.sendResponse( this._stackTraceResponse );
		// 		}

		// 		this._waitingToSendStackTraceResponse = false;
		// 	}
		// }
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
				console.log( 'Connected to Game.' );

				if( args.stopOnEntry )
				{
					// Send a 'step' command to the Game, which will pause execution.
					this._socket.write( "step" );
					console.log( "Sending 'step'." );

					// // Tell VSCode we're stopped,
					// // for the case when the Game isn't currently running Lua code and won't reply with a 'Stopped'
					// this.sendEvent( new StoppedEvent( 'step', MyEngineLuaDebugSession.THREAD_ID ) );
				}
			}
		);
		this._socket.on( 'data', data => { this.dealWithIncomingData( data ); } );
		this._socket.on( 'error', () => { this.TerminateDebugger() } );

		// make sure to 'Stop' the buffered logging if 'trace' is not set
		logger.setup( args.trace ? Logger.LogLevel.Verbose : Logger.LogLevel.Stop, false );

		// start the program in the runtime
		this._runtime.start( args.program, !!args.stopOnEntry );

		this.sendResponse(response);
	}

	protected setBreakPointsRequest(response: DebugProtocol.SetBreakpointsResponse, args: DebugProtocol.SetBreakpointsArguments): void {

		const path = <string>args.source.path;
		const clientLines = args.lines || [];

		// clear all breakpoints for this file
		this._runtime.clearBreakpoints(path);

		// set and verify breakpoint locations
		const actualBreakpoints = clientLines.map(l => {
			let { verified, line, id } = this._runtime.setBreakPoint(path, this.convertClientLineToDebugger(l));
			const bp = <DebugProtocol.Breakpoint> new Breakpoint(verified, this.convertDebuggerLineToClient(line));
			bp.id= id;
			return bp;
		});

		// send back the actual breakpoint positions
		response.body = {
			breakpoints: actualBreakpoints
		};
		this.sendResponse(response);
	}

	protected threadsRequest(response: DebugProtocol.ThreadsResponse): void
	{
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
		if( typeof this._lastStackJSONMessage === 'undefined' )
		{
			this.sendErrorResponse( response, 0, "No Lua script running." );
			return;
		}

		let jMessage = this._lastStackJSONMessage;

		if( jMessage.StackNumLevels == 0 )
		{
			this.sendErrorResponse( response, 0, "No Lua script running." );
		}
		else
		{
			var stackframe = new StackFrame( 0, "functionName()", this.createSource(jMessage.Source), jMessage.Line, 0 );

			var frames = new Array<any>();
			frames.push( stackframe );

			response.body =
			{
				stackFrames: frames,
				totalFrames: 1
			};

			this.sendResponse( response );
		}

		//console.log( "Sending 'stackTraceRequest'." );

		// const startFrame = typeof args.startFrame === 'number' ? args.startFrame : 0;
		// const maxLevels = typeof args.levels === 'number' ? args.levels : 1000;
		// const endFrame = startFrame + maxLevels;

		// var stackrequest =  { "stackstart":startFrame, "stackend":endFrame };
		// this._socket.write( JSON.stringify( stackrequest ) );

		// // Some attempt at waiting for a response from the socket,
		// //  works, but isn't good.
		// this._stackTraceResponse = response;
		// this._waitingToSendStackTraceResponse = true;
	}

	protected scopesRequest(response: DebugProtocol.ScopesResponse, args: DebugProtocol.ScopesArguments): void
	{
		const frameReference = args.frameId;
		const scopes = new Array<Scope>();
		scopes.push(new Scope("Local", this._variableHandles.create("local_" + frameReference), false));
		scopes.push(new Scope("Global", this._variableHandles.create("global_" + frameReference), true));

		response.body = {
			scopes: scopes
		};
		this.sendResponse(response);
	}

	protected variablesRequest(response: DebugProtocol.VariablesResponse, args: DebugProtocol.VariablesArguments): void
	{
		const variables = new Array<DebugProtocol.Variable>();
		const id = this._variableHandles.get(args.variablesReference);
		if (id !== null) {
			variables.push({
				name: id + "_i",
				type: "integer",
				value: "123",
				variablesReference: 0
			});
			variables.push({
				name: id + "_f",
				type: "float",
				value: "3.14",
				variablesReference: 0
			});
			variables.push({
				name: id + "_s",
				type: "string",
				value: "hello world",
				variablesReference: 0
			});
			variables.push({
				name: id + "_o",
				type: "object",
				value: "Object",
				variablesReference: this._variableHandles.create("object_")
			});
		}

		response.body = {
			variables: variables
		};
		this.sendResponse(response);
	}

	protected continueRequest(response: DebugProtocol.ContinueResponse, args: DebugProtocol.ContinueArguments): void
	{
		this._socket.write( "continue" );
		console.log( "Sending 'continue'." );

		this.sendResponse( response );
	}

	protected nextRequest(response: DebugProtocol.NextResponse, args: DebugProtocol.NextArguments): void
	{
		this._socket.write( "step" );
		console.log( "Sending 'step'." );

		//this.sendEvent( new StoppedEvent( 'step', MyEngineLuaDebugSession.THREAD_ID ) );
		this.sendResponse(response);
	}

	protected pauseRequest(response: DebugProtocol.PauseResponse, args: DebugProtocol.PauseArguments): void
	{
		// No difference between step and pause.
		this._socket.write( "step" );
		console.log( "Sending 'step'." );

		//this.sendEvent( new StoppedEvent( 'pause', MyEngineLuaDebugSession.THREAD_ID ) );
		this.sendResponse(response);
	}

	protected disconnectRequest(response: DebugProtocol.DisconnectResponse, args: DebugProtocol.DisconnectArguments): void
	{
		this.TerminateDebugger();
		this.sendResponse(response);
	}

	protected evaluateRequest(response: DebugProtocol.EvaluateResponse, args: DebugProtocol.EvaluateArguments): void {

		let reply: string | undefined = undefined;

		if (args.context === 'repl') {
			// 'evaluate' supports to create and delete breakpoints from the 'repl':
			const matches = /new +([0-9]+)/.exec(args.expression);
			if (matches && matches.length === 2) {
				const mbp = this._runtime.setBreakPoint(this._runtime.sourceFile, this.convertClientLineToDebugger(parseInt(matches[1])));
				const bp = <DebugProtocol.Breakpoint> new Breakpoint(mbp.verified, this.convertDebuggerLineToClient(mbp.line), undefined, this.createSource(this._runtime.sourceFile));
				bp.id= mbp.id;
				this.sendEvent(new BreakpointEvent('new', bp));
				reply = `breakpoint created`;
			} else {
				const matches = /del +([0-9]+)/.exec(args.expression);
				if (matches && matches.length === 2) {
					const mbp = this._runtime.clearBreakPoint(this._runtime.sourceFile, this.convertClientLineToDebugger(parseInt(matches[1])));
					if (mbp) {
						const bp = <DebugProtocol.Breakpoint> new Breakpoint(false);
						bp.id= mbp.id;
						this.sendEvent(new BreakpointEvent('removed', bp));
						reply = `breakpoint deleted`;
					}
				}
			}
		}

		response.body = {
			result: reply ? reply : `evaluate(context: '${args.context}', '${args.expression}')`,
			variablesReference: 0
		};
		this.sendResponse(response);
	}

	//---- helpers
	private createSource(filePath: string): Source
	{
		return new Source( basename(filePath), this.convertDebuggerPathToClient(filePath), undefined, undefined, 'myenginelua-adapter-data' );
	}
}

DebugSession.run(MyEngineLuaDebugSession);
