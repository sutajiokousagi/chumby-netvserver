
// -------------------------------------------------------------------------------------------------
//	console debug
// -------------------------------------------------------------------------------------------------

var tempLocaltion = "" + document.location;
var isRunningFromLocalhost = (tempLocaltion.indexOf("localhost") != -1 && tempLocaltion.indexOf("file://") == -1) ? true : false;

function log(v)
{
	if (isRunningFromLocalhost)		console.log("|~|" + v);
	else							console.log(v);
}
