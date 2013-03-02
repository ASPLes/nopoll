/**
 ** Copyright (C) 2013 Advanced Software Production Line, S.L.
 ** See license.txt or http://www.aspl.es/nopoll
 **/


/******* BEGIN: testConnect ******/
function testConnect () {

    log ("info", "Connecting to " + this.host + ":" + this.port);

    return true;
}

testConnect.Result = function (conn) {

    /* call for the next test */
    this.nextTest ();

    return true;
};
/******* END: testConnect ******/

function testWebSocketAvailable () {

    log ("info", "Checking if WebSocket is available on this system");

    if (typeof(WebSocket) == undefined || WebSocket == null) {
	log ("error", "WebSocket not available on this browser");
	return false;
    }

    /* engine available */
    this.nextTest ();
    return true;
};

/**
 * @brief Constructor function that creates a new regression test.
 *
 * @param host The host	location of the regression test listener.
 * @param port The port location of the regression test listener.
 */
function RegressionTest (host, port) {

    /* initialize next test to execute */
    this.nextTestId = -1;

    /* record host and port */
    this.host       = host;
    this.port       = port;

    /* signal if tests must be stoped */
    this.stopTests  = false;

    /* do not return nothing */
};

/**
 * @brief Method that runs the next test.
 */
RegressionTest.prototype.nextTest = function () {

    /* check to stop tests */
    if (this.stopTests)
	return;

    /* drop an ok message to signal test ok */
    if (this.nextTestId != -1) {
	log ("ok", "TEST-" + this.nextTestId + " " + this.tests[this.nextTestId].name + ": OK");
    }

    while (true) {
	/* select next test to execute */
	this.nextTestId++;

	/* check available tests */
	if (this.tests.length == this.nextTestId) {
	    log ("final-ok", "All regression tests finished OK!");
	    return;
	} else if (this.tests.length < this.nextTestId) {
	    log ("error", "regression test is calling to next test too many times: " + this.nextTestId);
	    return;
	}

	/* check if the test is available */
	if (! this.tests[this.nextTestId].runIt) {
	    /* skip test */
	    continue;
	}

	/* call next test */
	log ("info", "Running TEST-" + this.nextTestId + ": " + this.tests[this.nextTestId].name);

	var ref = this;
	setTimeout (function () {
	    ref.tests[ref.nextTestId].testHandler.apply (ref, []);
	}, 10);
	return;
    } /* end while */
};

/* list of regression test available with its
 * associated test to show */
RegressionTest.prototype.tests = [
    {name: "Check if Websocket is available",           testHandler: testWebSocketAvailable},
    {name: "Websocket basic connection test",           testHandler: testConnect}
];


function log (status, logMsg) {

    var timeStamp;
    if (status != 'ok' && status != 'final-ok') {
	var end;
	if (log.start == undefined) {
	    /* create an start */
	    log.start = new Date();
		timeStamp = "Diff 0: ";
	} else {
	    /* create the stop */
	    end = new Date ();

	    /* compare with previous */
	    timeStamp = "Diff " + String ((end.getTime()) - (log.start.getTime ())) + ": ";

	    /* init new start */
	    log.start = end;
	}
    } else {
	timeStamp = "";
    } /* end if */

    /* set a default string if it is undefined */
    if (typeof logMsg == "undefined")
	logMsg = "";

    logMsg = logMsg.replace (/</g, "&lt;");

    /* add timestamp */
    logMsg = timeStamp + logMsg;

    /* create the node that wild hold the content to log */
    var newNode = document.createElement ("pre");
    dojo.addClass (newNode, "log-line");
    dojo.addClass (newNode, status);

    /* configure log line into the node */
    newNode.innerHTML = logMsg;

    /* place the node into the panel */
    var logpanel = dojo.byId ("log-panel");
    dojo.place (newNode, logpanel);

    /* scroll down a content pane */
    logpanel.scrollTop = logpanel.scrollHeight;

    return;
}

function runTest (testName) {

    /* run all tests */
    /* document.write ("<h1>jsVortex: running all regression tests..</h1>"); */
    /* testConnect (); */
    var host = dojo.byId ("host").value;
    var port = dojo.byId ("port").value;

    /* not required to check host and port here, already done by the form */
    log ("ok", "Running all tests: " + host + ":" + port);

    /* clear log */
    dojo.byId ("log-panel").innerHTML = "";

    log ("info", "Running tests");

    /* create a regression test */
    var regTest = new RegressionTest (host, port);

    /* run tests */
    regTest.nextTest();

    return;
}

function clearLog () {
    /* clear log panel */
    var logpanel = dojo.byId ("log-panel");
    logpanel.innerHTML = "";
}

/**
 * @internal Function called when a test check box is clicked.
 */
function testClicked (event) {
    var status = event.currentTarget.checked;
    var id     = event.currentTarget.id;
    var tests  = RegressionTest.prototype.tests;
    for (position in tests) {
	if (tests[position].name == id) {
	    /* flag the test to be runned or not */
	    tests[position].runIt = status;
	    break;
	} /* end if */
    }
}

function invertSelection (event) {

    for (iterator in RegressionTest.prototype.tests) {
	/* acquire a reference to the test object */
	var test = RegressionTest.prototype.tests[iterator];

	/* invert its current configuration to be executed */
	test.runIt = ! test.runIt;

	/* flag "checked" with the new state */
	test.checkBox.attr ("checked", test.runIt);
    }

    return;
}

function transportSelected (event) {
    clearLog ();
    log ("info", "TCP Transport selected: " + event);
    if (event == "Java Socket Connector")
	VortexTCPTransport.useTransport = 2;
    else if (event == "Firefox nsISocketTransportService")
	VortexTCPTransport.useTransport = 1;
    return;
}

function resizeTestWindow () {

    /* configure window height */
    var windowHeight = dijit.getViewport ().h;
    var heightValue = (windowHeight - dojo.marginBox (dojo.query (".nopoll-regtest-header")[0]).h - dojo.marginBox (dojo.query ("#control-panel")[0]).h) + "px";
    /* dojo.style(dojo.byId ("test-available-panel"), "height", heightValue);
    dojo.style(dojo.byId ("log-panel"), "height", heightValue); */
    dojo.style(dojo.byId ("global-container"), "height", heightValue);

    /* call to resize */
    dijit.byId ("global-container").resize ();

    return;
}

function prepareTest () {
    /* connect clicked signal */
    dojo.connect (dojo.byId("run-test"), "click", runTest);
    dojo.connect (dojo.byId ("clear-log"), "click", clearLog);
    dojo.connect (dojo.byId ("invert-selection"), "click", invertSelection);
    dojo.connect (dijit.byId ("transportType"), "onChange", transportSelected);

    /* configure default connection values */
    dojo.byId ("host").value = location.hostname;

    /* configure default connection port value */
    dojo.byId ("port").value = "44010";

    /* fill all tests available */
    var tests              = RegressionTest.prototype.tests;
    var testAvailablePanel = dijit.byId("test-available-panel");
    var checkBox;
    for (test in tests) {

	/* flag the test to be runned */
	tests[test].runIt = true;

	/* create the label to be associated to the checkBox */
	var label = document.createElement ("label");

	/* label.for = tests[test].name; */
	dojo.attr (label, 'for', tests[test].name);
	dojo.attr (label, 'htmlFor', tests[test].name);

	label.innerHTML = "TEST-" + test + ": " + tests[test].name;

	/* create check box */
	checkBox = new dijit.form.CheckBox (
	    {id: tests[test].name,
	     checked: true,
	     position: test});

	/* configure onClick handler */
	checkBox.onClick = testClicked;

	/* add a reference on the test */
	tests[test].checkBox = checkBox;

	/* add to the panel */
	dojo.place(checkBox.domNode, testAvailablePanel.domNode);

	/* add to the panel */
	dojo.place (label, testAvailablePanel.domNode);

	/* add to the panel a line break */
	dojo.place (document.createElement ("br"), testAvailablePanel.domNode);
    }

    /* call to resize */
    resizeTestWindow ();

    /* catch resize event */
    dojo.connect (window, "resize", resizeTestWindow);
}

/* register our function in dojo */
dojo.addOnLoad (prepareTest);





