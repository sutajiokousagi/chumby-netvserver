function onLoad()
{
	$("#div_loadingMain").fadeIn(0);
	
	//Ask NeTVServer to start upgrading
	fDbg2("Asking NeTVServer to upgrade...");
	xmlhttpPost("", "post", { 'cmd' : 'SystemUpdate' }, rawSystemUpdateCallback );
}

function rawSystemUpdateCallback = function (vData)
{
	if (vData.split("</status>")[0].split("<status>")[1] != "1")
		return;
	
	fwver = vData.split("</fwver>")[0].split("<fwver>")[1];
	vLog = vData.split("</log>")[0].split("<log>")[1];
	
	//Check if update succeeded
	var errorTags = [ 'error', 'failed', 'read-only' ];
	var containsErrorTag = false;
	for (var i=0; i<errorTags.length; i++)
	{
		if (vLog.indexOf(errorTags[i]) == -1)
			continue;
		containsErrorTag = true;
		
		//Do something about it?
	}
	
	fDbg2("--------------------------------------------------------------------");
	fDbg2(vLog);
	fDbg2("--------------------------------------------------------------------");
	
	if (containsErrorTag)		fDbg2("Update failed");
	else						fDbg2("Update successful. FWver: " + fwver);
		
	//Redirect to homepage
	location.href="http://localhost/";
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
//	system API functions
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
//	server reset - due to upgrade
// -------------------------------------------------------------------------------------------------
function fServerReset()
{
	return "command ignored";
	fDbg2("fServerReset()");
}

// -------------------------------------------------------------------------------------------------
//	press button on D-pad / android
// -------------------------------------------------------------------------------------------------
function fButtonPress( vButtonName )
{
	return "command ignored";
	/*
	switch(vButtonName)
	{
		case "up":		selectNextWifi();		break;
		case "down":	selectPreviousWifi();	break;
		case "center":	acceptCurrentWifi();	break;
	}
	*/
}

// -------------------------------------------------------------------------------------------------
//	events from HDMI/FPGA
// -------------------------------------------------------------------------------------------------
function fHDMIEvents( vEventName )
{
	return "command ignored";
}

// -------------------------------------------------------------------------------------------------
//	events from DBus/NetworkManager
// -------------------------------------------------------------------------------------------------
function fNMStateChanged( vEventName )
{
	return "command ignored";
}

function fNMDeviceAdded(  )
{
	return "command ignored";
}

function fNMDeviceRemoved(  )
{
	return "command ignored";
}

// -------------------------------------------------------------------------------------------------
//	events from system
// -------------------------------------------------------------------------------------------------
function fUPDATEEvent( vEventName )
{
	vEventName = vEventName.toLowerCase();
	switch(vEventName)
	{
		case "done":	location.href="http://localhost/";		break;
		case "large":	/* Do nothing, already updating */		break;
	}
}