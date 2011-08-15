var main_x;
var main_y;
var updateCount;
var needReboot;

function onLoad()
{
	//Params passed thru GET
	updateCount = GET('updateCount');
	needReboot = GET('reboot');
	continue_percentage = GET('continue');
	
	//Save initial parameters
	main_y = $("#div_center").offset().top;
	main_x = $("#div_center").offset().left;
	$("#div_loadingMain").fadeIn(0);
	
	setUpgradeProgress(continue_percentage);
	
	//Not continue from an upgrade process
	if (continue_percentage == "" || continue_percentage == 0)
	{
		fDbg2("Start upgrading UI");
		
		//Hide everything immediately, animate in later
		main_hideMainPanel();
	
		setTimeout("onLoadLater()", 100);
	}
	
	//This happens when browser restarts half way during an upgrading process
	else
	{
		fDbg2("Continue upgrading UI: " + continue_percentage + "%");
		main_showMainPanel();
	}
}

function onLoadLater()
{
	//Gracefully show the update panel
	main_showMainPanel(1650);
}

//-----------------------------------------------------------

function main_showMainPanel(duration)
{
	$("#div_center").css('visibility', 'visible');
	
	var offset = $("#div_center").offset();
	offset.top = main_y;
	
	if (!duration || duration == 0)		$("#div_center").offset( offset );
	else								$("#div_center").animate({ top: main_y }, !duration ? 1600 : duration);
}

function main_hideMainPanel(duration)
{
	var offset = $("#div_center").offset();
	offset.top = $(window).height();
	
	if (!duration || duration == 0)		$("#div_center").offset( offset );
	else								$("#div_center").animate({ top: main_y + $(window).height() }, !duration ? 1600 : duration);
}

//-----------------------------------------------------------

function GET( paramName )
{
	paramName = paramName.replace(/[\[]/,"\\\[").replace(/[\]]/,"\\\]");
	var regexS = "[\\?&]"+paramName+"=([^&#]*)";
	var regex = new RegExp( regexS );
	var results = regex.exec( window.location.href );
	if( results == null )    	return "";
	else					   	return results[1];
}

//-----------------------------------------------------------

function fUPDATEEvents( vEventName, vEventData )
{
	switch (vEventName)
	{
		case "progress":	setUpgradeProgress(vEventData);		break;
		case "done":		setUpgradeDone();					break;
	}
}

function setUpgradeProgress(percentage)
{
	if (!percentage || percentage == "" || percentage == 0)
		percentage = 0;
	percentage = parseFloat(""+percentage);
		
	//Set the progress bar (very convenient!)
	$("#progress_bar").width(""+percentage+"%");
	
	//Nicely style the end cap
	if (percentage > 100-(24-8)/2)
	{
		percentage = Math.round(percentage);
		$("#progress_bar").css("border-top-right-radius", 24-(100-percentage)*2);
		$("#progress_bar").css("border-bottom-right-radius", 24-(100-percentage)*2);
	}
	else
	{
		$("#progress_bar").css("border-top-right-radius", 8);
		$("#progress_bar").css("border-bottom-right-radius", 8);
	}
}

function setUpgradeDone()
{
	//Hehe
	setUpgradeProgress(100);
	
	//Gracefully hide the update panel
	main_hideMainPanel(1650);
	
	//May not reach here if upgrade script reboots the system itself
	setTimeout("location.href=\"http://localhost/\"", 1650);
}




// Other system events are totally ignored below




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
