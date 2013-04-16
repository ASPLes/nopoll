/**
 ** Copyright (C) 2013 Advanced Software Production Line, S.L.
 ** See license.txt or http://www.aspl.es/nopoll
 **/

function testTLSRequestReply () {

    var tls_port = Number(this.port) + 1;

    log ("info", "Connecting to " + this.host + ":" + tls_port);
    var socket = new WebSocket ("wss://" + this.host + ":" + tls_port);
    socket.test = this;

    socket.onopen = testTLSRequestReply.Result;
    socket.onclose = function (event) {
	log ("error", "Connection error. Unable to connect to: " + event.target.url + ". Is a server running on that location or the browser is able to resolve that name?" );
    };
    socket.onerror = function (event) {
	log ("error", "Connection error. Unable to connect to: " + event.target.url + ". Is a server running on that location or the browser is able to resolve that name?" );
    };

};

testTLSRequestReply.Result = function (event) {

    var socket = event.target;
    socket.onclose = null;

    /* send content and wait for the reply */
    socket.onmessage = testTLSRequestReply.Reply;

    socket.msg = "This is a test message..";
    socket.send (socket.msg);
};

testTLSRequestReply.Reply = function (event) {

    /* get the socket */
    var socket = event.target;
    var msg    = event.data;

    if (msg != socket.msg) {
	log ("error", "Expected to find a different message, but found: " + msg);
	return;
    } /* end if */

    log ("info", "Message received (TLS), nice!");

    /* close the connection */
    socket.close ();

    var test   = socket.test;
    test.nextTest ();

    return;
};

/******* BEGIN: testTLSConnect ******/
function testTLSConnect () {

    var tls_port = Number(this.port) + 1;

    log ("info", "Connecting to " + this.host + ":" + tls_port);
    var socket = new WebSocket ("wss://" + this.host + ":" + tls_port);

    socket.test = this;

    socket.onopen = testTLSConnect.Result;
    socket.onclose = function (event) {
	log ("error", "Connection error. Unable to connect to: " + event.target.url + ". Is a server running on that location or the browser is able to resolve that name?" );
    };

    return true;
}

testTLSConnect.Result = function (event) {

    log ("info", "Connection TLS received");
    var socket = event.target;
    socket.onclose = null;

    if (socket.readyState != 1) {
	log ("error", "Expected a ready state of 1 (OPEN) but found: " + socket.readyState);
	return false;
    }

    /* close the socket */
    socket.close ();

    /* call for the next test */
    socket.test.nextTest ();

    return true;
};
/******* END: testTLSConnect ******/

function testBigMsg () {
    log ("info", "Connecting to " + this.host + ":" + this.port);
    var socket = new WebSocket ("ws://" + this.host + ":" + this.port);
    socket.test = this;

    socket.onopen = testBigMsg.Result;
    socket.onclose = function (event) {
	log ("error", "Connection error. Unable to connect to: " + event.target.url + ". Is a server running on that location or the browser is able to resolve that name?" );
    };

};

testBigMsg.Result = function (event) {
    /* get socket */
    var socket = event.target;
    socket.onclose = null;

    /* send a really big message */
    socket.onmessage = testBigMsg.Reply;

    console.log ("Sending content with UTF-8");
    socket.msg = " klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw";
    socket.send (socket.msg);
    console.log ("Content sent");

    return;
};

testBigMsg.Reply = function (event) {

    var socket = event.target;

    /* check reply */
    if (event.data != socket.msg) {
	log ("error", "Expected to find same content in the reply, but something different was found");
	return;
    } /* end if */
    log ("info", "Reply content length received: " + event.data.length);

    socket.close ();

    /* next test */
    socket.test.nextTest ();

    return;
};

function testReallyBigMsg () {
    log ("info", "Connecting to " + this.host + ":" + this.port);
    var socket = new WebSocket ("ws://" + this.host + ":" + this.port);
    socket.test = this;

    socket.onopen = testReallyBigMsg.Result;
    socket.onclose = function (event) {
	log ("error", "Connection error. Unable to connect to: " + event.target.url + ". Is a server running on that location or the browser is able to resolve that name?" );
    };

};

testReallyBigMsg.Result = function (event) {
    /* get socket */
    var socket = event.target;
    socket.onclose = null;

    /* send a really big message */
    socket.onmessage = testReallyBigMsg.Reply;

    console.log ("Sending content with UTF-8");
    socket.msg = " klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 535355  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 8877  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 9888  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 99 9sdf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 093249354  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 3i354  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf wek342  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 324324  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 32432  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf";
    socket.send (socket.msg);
    console.log ("Content sent");

    return;
};

testReallyBigMsg.Reply = function (event) {

    var socket = event.target;

    /* check reply */
    if (event.data != socket.msg) {
	log ("error", "Expected to find same content in the reply, but something different was found");
	return;
    } /* end if */
    log ("info", "Reply content length received: " + event.data.length);

    socket.close ();

    /* next test */
    socket.test.nextTest ();

    return;
};

function testReallyBigMsg2 () {
    log ("info", "Connecting to " + this.host + ":" + this.port);
    var socket = new WebSocket ("ws://" + this.host + ":" + this.port);
    socket.test = this;

    socket.onopen = testReallyBigMsg2.Result;
    socket.onclose = function (event) {
	log ("error", "Connection error. Unable to connect to: " + event.target.url + ". Is a server running on that location or the browser is able to resolve that name?" );
    };

};

var reallyBigMessage = " klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 535355  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 8877  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 9888  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 99 9sdf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 093249354  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 3i354  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf wek342  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 324324  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 32432  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 535355  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 8877  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 9888  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 99 9sdf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 093249354  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 3i354  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf wek342  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 324324  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 32432  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf   klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 535355  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 8877  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 9888  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 99 9sdf  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 093249354  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 3i354  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf wek342  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 324324  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf 32432  klasdfkla akldfj klafklajetqkljt kjlwergklwejry90246tkgwr kñljwrglkjdfg lksdjglskg slkg camión adsfasdf pruébasdfad España asdfaklsjdflk jasfkjaslfjetql tjñqgkjadgklj aglkjalk jafkjaslfkjaskj asjaslfkjasfklajg klajefñlqkjetrlkqj lqkj ñlskdfjañlk asldfjñlafj añlfj ñdfjkjt4ñqlkjt lkj34tlkjañlgjañlkgjañlkgjw kladsf kladsfalsdf lkafalskj asdfkljasdf kaldfjlakf kljasfklffffffffffffffffalf    asdfkljasdf asdfjlakf";

testReallyBigMsg2.Result = function (event) {
    /* get socket */
    var socket = event.target;
    socket.onclose = null;

    /* send a really big message */
    socket.onmessage = testReallyBigMsg2.Reply;

    console.log ("Sending content with UTF-8");
    socket.msg = reallyBigMessage;

    socket.send (socket.msg);
    console.log ("Content sent");

    return;
};

testReallyBigMsg2.Reply = function (event) {

    var socket = event.target;

    /* check reply */
    if (event.data != socket.msg) {
	log ("error", "Expected to find same content in the reply, but something different was found");
	return;
    } /* end if */
    log ("info", "Reply content length received (really big II): " + event.data.length);

    socket.close ();

    /* next test */
    socket.test.nextTest ();

    return;
};

function testIntensiveDataTransfer () {

    log ("info", "Connecting to " + this.host + ":" + this.port);
    var socket = new WebSocket ("ws://" + this.host + ":" + this.port);
    socket.test = this;

    socket.onopen = testIntensiveDataTransfer.Result;
    socket.onclose = function (event) {
	log ("error", "Connection error. Unable to connect to: " + event.target.url + ". Is a server running on that location or the browser is able to resolve that name?" );
    };

};


testIntensiveDataTransfer.Result = function (event) {
    var socket = event.target;
    socket.onclose = null;

    /* send content and wait for the reply */
    socket.onmessage = function () {
	/* message received */
    };

    var iterator = 0;
    var flipFlop = false;
    while (iterator < 500) {
	var msg = "This is a test message..with come content modified: " + iterator;
	if (flipFlop)
	    socket.send (msg);
	else 
	    socket.send (reallyBigMessage + iterator);

	/* toggle state */
	flipFlop = !flipFlop;

	/* next iterator */
	iterator++;
    } /* end while */

    /* check here if the socket works */
    setTimeout (function () {
	if (socket.readyState != 1) {
	    log ("error", "Found connection ready State on failure after intensive send, test failing");
	    return;
	} /* end if */

	/* close the connection */
	socket.close ();

	var test   = socket.test;
	test.nextTest ();	
    }, 1000);


};

function testRequestReply () {

    log ("info", "Connecting to " + this.host + ":" + this.port);
    var socket = new WebSocket ("ws://" + this.host + ":" + this.port);
    socket.test = this;

    socket.onopen = testRequestReply.Result;
    socket.onclose = function (event) {
	log ("error", "Connection error. Unable to connect to: " + event.target.url + ". Is a server running on that location or the browser is able to resolve that name?" );
    };

};

testRequestReply.Result = function (event) {

    var socket = event.target;
    socket.onclose = null;

    /* send content and wait for the reply */
    socket.onmessage = testRequestReply.Reply;

    socket.msg = "This is a test message..";
    socket.send (socket.msg);
};

testRequestReply.Reply = function (event) {

    /* get the socket */
    var socket = event.target;
    var msg    = event.data;

    if (msg != socket.msg) {
	log ("error", "Expected to find a different message, but found: " + msg);
	return;
    } /* end if */

    log ("info", "Message received, nice!");

    /* close the connection */
    socket.close ();

    var test   = socket.test;
    test.nextTest ();

    return;
};


/******* BEGIN: testConnect ******/
function testConnect () {

    log ("info", "Connecting to " + this.host + ":" + this.port);
    var socket = new WebSocket ("ws://" + this.host + ":" + this.port);

    socket.test = this;

    socket.onopen = testConnect.Result;
    socket.onclose = function (event) {
	log ("error", "Connection error. Unable to connect to: " + event.target.url + ". Is a server running on that location or the browser is able to resolve that name?" );
    };

    return true;
}

testConnect.Result = function (event) {

    log ("info", "Connection received");
    var socket = event.target;
    socket.onclose = null;

    if (socket.readyState != 1) {
	log ("error", "Expected a ready state of 1 (OPEN) but found: " + socket.readyState);
	return false;
    }

    /* close the socket */
    socket.close ();

    /* call for the next test */
    socket.test.nextTest ();

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

function doConnection (uri, onopen, timeout) {

    var socket = new WebSocket (uri);
    socket.onopen = onopen;
    socket.onclose = function (event) {

    };

    /* default timeout of 3 seconds */
    if (! timeout)
	timeout = 3000;

    socket._interval = setInterval (function () {
	/* clear interval */
	clearInterval (socket._interval);

	/* check if the connection was ok */
	if (socket.readyState == 1)
	    return;

	var result = {};
	onopen (result);
    }, timeout);

    return socket;
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
    {name: "Check if Websocket is available",                    testHandler: testWebSocketAvailable},
    {name: "Websocket basic connection test",                    testHandler: testConnect},
    {name: "Websocket send basic data (request/reply)",          testHandler: testRequestReply},
    {name: "Websocket send more content (big messages)",         testHandler: testBigMsg},
    {name: "Websocket send more content (really big messages)",  testHandler: testReallyBigMsg},
    {name: "Websocket send more content (really big messages II)",  testHandler: testReallyBigMsg2},
    {name: "Websocket intensive data transfer",                  testHandler: testIntensiveDataTransfer},
    {name: "TLS: Websocket basic connect",                       testHandler: testTLSConnect},
    {name: "TLS: Websocket send basic data (request/reply)",     testHandler: testTLSRequestReply}
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
    dojo.byId ("port").value = "1234";

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





