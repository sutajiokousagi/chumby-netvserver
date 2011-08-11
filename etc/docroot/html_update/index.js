function onLoad()
{
	$("#div_loadingMain").fadeIn(0);
	
	//Call systemupdate.sh script
	xmlhttpPost("", "post", { 'cmd' : 'SystemUpdate' }, updateDoneCallback );
}

function updateDoneCallback( vData )
{
	fDbg2("-------------------------------------------");
	fDbg2("  Upgrade Done");
	fDbg2("-------------------------------------------");
	fDbg2( vData );
	//May not reach here since system reboots itself
	//location.href="http://localhost/";
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
function fUPDATECOUNTEvent( vEventName )
{
	return "command ignored";
}

function fUPDATEREADYEvent( vEventName )
{
	xmlhttpPost("", "post", { 'cmd' : 'SystemUpdate' }, updateDoneCallback );
	return "upgrading...";
}