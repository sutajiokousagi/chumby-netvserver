var main_x;
var main_y;
var updateCount;
var needReboot;
var startConfiguring;

function onLoad()
{
	//Params passed thru GET
	updateCount = GET('updateCount');
	needReboot = GET('reboot');
	isContinue = GET('continue');
	startConfiguring = false;
	
	//Save initial panel position
	main_y = $("#div_center").offset().top;
	main_x = $("#div_center").offset().left;
	$("#div_loadingMain").fadeIn(0);
	
	//Not continue from an upgrade process
	if (isContinue == "" || isContinue == 0)
	{
		fDbg2("Start upgrading UI");
		
		setUpgradePercentage(0, false);
		
		//Hide everything immediately, slide in later
		main_hideMainPanel();
		
		clearConsoleLog();
		addConsoleLog("Starting...");
	
		setTimeout("onLoadLater()", 100);
	}
	
	//This happens when browser restarts half way during an upgrading process
	//Using opkg-chumby-upgrade, the Browser only get killed at the end, so we consider it as done
	else
	{
		fDbg2("Continue upgrading UI: " + isContinue + "%");
		
		setUpgradePercentage(201, false);
		
		main_showMainPanel();
		
		setUpgradeDone();
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
		case "configuring":	setUpgradeConfiguring(vEventData);	break;
		case "progress":	setUpgradeProgress(vEventData);		break;
		case "done":		setUpgradeDone();					break;
	}
}

function setUpgradeProgress(vData)
{
	//Format: <percentage>%1</percentage><pkgname>%2</pkgname><pkgsize>%3</pkgsize><sizeprogress>%4</sizeprogress>
	vData = decodeURIComponent(vData);
	if (vData.split == undefined)
		return;
					
	var percentage = vData.split("</percentage>")[0].split("<percentage>")[1];
	var name = vData.split("</pkgname>")[0].split("<pkgname>")[1];
	var size = vData.split("</pkgsize>")[0].split("<pkgsize>")[1];
	var sizeprogress = vData.split("</sizeprogress>")[0].split("<sizeprogress>")[1];

	//Progress bar
	setUpgradePercentage(percentage, false);
	
	//Console text
	if (size > 0)	addConsoleLog("<font color='#6598EB'>Upgrading " + name + "</font>");
	else			addConsoleLog("Upgrading " + name);
}

/*
function setUpgradeProgress(vData)
{
	vData = decodeURIComponent(vData);

	//Format: <percentage>%1</percentage><pkgname>%2</pkgname><pkgversion>%3</pkgversion><pkgsize>%4</pkgsize>
	vData = decodeURIComponent(vData);
	if (vData.split == undefined)
		return;
					
	var percentage = vData.split("</percentage>")[0].split("<percentage>")[1];
	var name = vData.split("</pkgname>")[0].split("<pkgname>")[1];
	var version = vData.split("</pkgversion>")[0].split("<pkgversion>")[1];
	var size = vData.split("</pkgsize>")[0].split("<pkgsize>")[1];
	size = Math.round(size/1024*10)/10;
	
	//Progress bar
	setUpgradePercentage(percentage);
	
	//Console text
	addConsoleLog("Upgrading " + name + "<br>Version " + version + " (" + size + "KB)...");
}
*/

function setUpgradeConfiguring(vData)
{
	//Format: <percentage>%1</percentage><pkgname>%2</pkgname>
	vData = decodeURIComponent(vData);
	if (vData.split == undefined)
		return;
					
	var percentage = vData.split("</percentage>")[0].split("<percentage>")[1];
	var name = vData.split("</pkgname>")[0].split("<pkgname>")[1];
	
	//Console text
	if (startConfiguring == false)
		addConsoleLog("Finalizing updates, please wait...");
	startConfiguring = true;
}

function setUpgradePercentage(percentage, animated)
{
	if (!percentage || percentage == "" || percentage == 0)
		percentage = 0;
	percentage = parseFloat(""+percentage);
	
	//Some pretty adjustments
	if (percentage < 3.0)			percentage = 3;
	else if (percentage >= 200)		percentage = 100;
	else if (percentage == 100)		percentage = 97;
	
	//Animate the progress bar
	if (animated == true)		$("#progress_bar").animate({ width: ""+percentage+"%" }, 1000);
	else						$("#progress_bar").width("" + percentage + "%");
	
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
	setUpgradePercentage(201, true);
	
	//Console text
	addConsoleLog("Upgrading completed!");
	
	//Wait for 2.5 seconds then continue
	
	//Gracefully hide the update panel
	setTimeout("main_hideMainPanel(1600);", 2500+1650);
	
	//May not reach here if upgrade script reboots the system itself
	setTimeout("location.href=\"http://localhost/\"", 2500+1650+1650);
}

//-----------------------------------------------------------

function addConsoleLog(text)
{
	var oldValue = $("#consolelog").html();
		
	//Keep only last 14 lines
	var tempArray = oldValue.split("<br>");
	while (tempArray.length > 14)
		tempArray.splice(0,1);
	oldValue = tempArray.join("<br>");
		
	$("#consolelog").html(oldValue + "<br>" + text);
}

function clearConsoleLog(text)
{
	var oldValue = $("#consolelog").html("");
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
