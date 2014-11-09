myframework.Application = function ()
{
  // The native module for the application.  This refers to the module loaded via the <embed> tag.
  // @type {Element}
  // @private
  this.module_ = null;

  // The function objects that get attached as event handlers.  These are cached so that they can be removed when they are no longer needed.
  // @type {function}
  // @private
  this.boundModuleDidLoad_ = null;
}

// The ids used for elements in the DOM.  The Tumlber Application expects these
// elements to exist.
// @enum {string}
// @private
myframework.Application.DomIds_ =
{
    MODULE: 'myframework',      // The <embed> element representing the NaCl module
    VIEW: 'myframework_view'    // The <div> containing the NaCl element.
}

// Called by the module loading function once the module has been loaded.
// @param {?Element} nativeModule The instance of the native module.
myframework.Application.prototype.moduleDidLoad = function ()
{
    this.module_ = document.getElementById(myframework.Application.DomIds_.MODULE);

    // Unbind the load function.
    this.boundModuleDidLoad_ = null;
}

// Asserts that cond is true; issues an alert and throws an Error otherwise.
// @param {bool} cond The condition.
// @param {String} message The error message issued if cond is false.
myframework.Application.prototype.assert = function (cond, message)
{
    if( !cond )
    {
        message = "Assertion failed: " + message;
        alert(message);
        throw new Error(message);
    }
}

// The run() method starts and 'runs' the application.
// @param {?String} opt_contentDivName The id of a DOM element in which to
//     embed the Native Client module.  If unspecified, defaults to
//     VIEW.  The DOM element must exist.
myframework.Application.prototype.run = function (opt_contentDivName)
{
    contentDivName = opt_contentDivName || myframework.Application.DomIds_.VIEW;
    var contentDiv = document.getElementById(contentDivName);
    this.assert(contentDiv, "Missing DOM element '" + contentDivName + "'");

    // Note that the <EMBED> element is wrapped inside a <DIV>, which has a 'load'
    // event listener attached.  This method is used instead of attaching the
    // 'load' event listener directly to the <EMBED> element to ensure that the
    // listener is active before the NaCl module 'load' event fires.
    this.boundModuleDidLoad_ = this.moduleDidLoad.bind( this );

    contentDiv.addEventListener( 'load', this.boundModuleDidLoad_, true );

    // Load the published .nexe.  This includes the 'nacl' attribute which
    // shows how to load multi-architecture modules.  Each entry in the "nexes"
    // object in the  .nmf manifest file is a key-value pair: the key is the
    // runtime ('x86-32', 'x86-64', etc.); the value is a URL for the desired
    // NaCl module.  To load the debug versions of your .nexes, set the 'nacl'
    //  attribute to the _dbg.nmf version of the manifest file.
    contentDiv.innerHTML = '<embed id="' + myframework.Application.DomIds_.MODULE + '"'
                         + ' src=myframework_dbg.nmf'
//                         + ' src=myframework.nmf'
                         + ' type="application/x-nacl"'
                         + ' width="360" height="600" />'
}
