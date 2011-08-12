var main_y;
var updateCount;
var needReboot;

function onLoad()
{
	//Params passed thru GET
	updateCount = GET('updateCount');
	needReboot = GET('reboot');
	
	//Hide everything immediately, animate in later
	main_y = $("#div_center").offset().top;
	main_hideMainPanel(50);
	
	$("#div_loadingMain").fadeIn(0);
	
	setTimeout("onLoadLater()", 100);
}

function onLoadLater()
{
	//Gracefully show the update panel
	main_showMainPanel();
	
	//Start heavy updating after animation is done
	setTimeout("doUpgrade()", 1650);
}

function doUpgrade()
{
	//Call systemupdate.sh script
	xmlhttpPost("http://localhost/bridge", "post", { 'cmd' : 'SystemUpdate', 'value' : '' }, updateDoneCallback );
}

//-----------------------------------------------------------

function main_showMainPanel(duration)
{
	$("#div_center").css('visibility', 'visible');
	$("#div_center").animate({ top: main_y }, !duration ? 1600 : duration);
}

function main_hideMainPanel(duration)
{
	$("#div_center").animate({ top: main_y + $(window).height() }, !duration ? 1600 : duration);
}

//-----------------------------------------------------------

function GET( paramName )
{
	paramName = paramName.replace(/[\[]/,"\\\[").replace(/[\]]/,"\\\]");
	var regexS = "[\\?&]"+paramName+"=([^&#]*)";
	var regex = new RegExp( regexS );
	var results = regex.exec( window.location.href );
	if( results == null )    return "";
	else					   return results[1];
}

function updateDoneCallback( vData )
{
	fDbg2("-------------------------------------------");
	fDbg2("  Upgrade Done");
	fDbg2("-------------------------------------------");
	//fDbg2( vData );
	
	//Gracefully hide the update panel
	main_hideMainPanel();
	
	//May not reach here if upgrade script reboots the system itself
	setTimeout("location.href=\"http://localhost/\"", 1650);
}

//-----------------------------------------------------------





// System events are totally ignored below




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
}

// -------------------------------------------------------------------------------------------------
//	press button on D-pad / android
// -------------------------------------------------------------------------------------------------
function fButtonPress( vButtonName )
{
	return "command ignored";
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
	return "command ignored";
}