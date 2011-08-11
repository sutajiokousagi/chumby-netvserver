function onLoad()
{
	$("#div_loadingMain").fadeIn(0);
	
	//Call systemupdate.sh script
	xmlhttpPost("http://localhost/bridge", "post", { 'cmd' : 'SystemUpdate', 'value' : '' }, updateDoneCallback );
}

function updateDoneCallback( vData )
{
	fDbg2("-------------------------------------------");
	fDbg2("  Upgrade Done");
	fDbg2("-------------------------------------------");
	fDbg2 ( vData );
	
	//May not reach here since system reboots itself
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
function fUPDATECOUNTEvent( vEventData )
{
	return "command ignored";
}

function fUPDATEREADYEvent( vEventData )
{
	fDbg2("-------------------------------------------");
	fDbg2("  Update Ready (" + vEventData + " packages)");
	fDbg2("-------------------------------------------");
	
	xmlhttpPost("", "post", { 'cmd' : 'SystemUpdate', 'value' : '' }, updateDoneCallback );
	return "upgrading...";
}