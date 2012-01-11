var main_x;
var main_y;
var updateCount;
var needReboot;
var startConfiguring;
var completedTextString = "Update Completed!";
var fwverTextString = "Firmware Version: ";
var minAndroidTextString = "Minimum Android app version: ";
var minIOSTextString = "Minimum iOS app version: ";
var upgradedPackageColor = "6598EB";
var needRebootTextString = "System will reboot after upgrading";
var rebootingTextString = "System is rebooting...";
var rebootTextColor = "FF0000";
var maxConsoleLines = 60;
var leftMargin = 20;

var timeoutMinCounter = 0;
var timeoutTimerID = setInterval(onTimeoutTimer, 60000);		//1 minute

function onLoad()
{
	onResize();

	//Params passed thru GET
	updateCount = GET('updateCount');
	isContinue = GET('continue');
	pkgList = GET('list');
	needReboot = GET('reboot');
	startConfiguring = false;
	
	//Save initial panel position
	main_y = $("#div_center").css('top').split('px')[0];
	main_x = $("#div_center").css('left').split('px')[0];
	$("#div_loadingMain").fadeIn(0);
	
	//Not continue from an upgrade process
	if (isContinue == "" || isContinue == 0)
	{
		fDebug("Start upgrading UI");
		
		setUpgradePercentage(0, false);
		
		//Hide everything immediately, slide in later
		main_hideMainPanel();
		
		clearConsoleLog();
		if (needReboot == "true" || needReboot == true)
			addConsoleLog("<font color='#" + rebootTextColor + "'>" + needRebootTextString + "</font>");
		addConsoleLog("Starting...");
	
		setTimeout("onLoadLater()", 100);
	}
	
	//This happens when browser restarts half way during an upgrading process
	//Using opkg-chumby-upgrade, the Browser only get killed at the end, so we consider it as done
	else
	{
		fDebug("Continue upgrading UI: " + isContinue + "%");
				
		main_showMainPanel();
		setUpgradePercentage(201, false);
		setUpgradeDone(pkgList);
	}
}

function onLoadLater()
{
	//Gracefully show the update panel
	main_showMainPanel(1650);
}

function onResize()
{
	var width = $("#div_center").css('width').split('px')[0];
	var height = $("#div_center").css('height').split('px')[0];
	var viewportwidth = $(window).width();
	var viewportheight = $(window).height();
	var left = viewportwidth - width - leftMargin;
	var top = (viewportheight - height) / 2;
	
	$("#div_center").css('top', top);
	$("#div_center").css('left', left);
}

//-----------------------------------------------------------

function main_showMainPanel(duration)
{
	$("#div_center").css('visibility', 'visible');
	
	if (!duration || duration == 0)		$("#div_center").css('left', main_x + 'px');
	else								$("#div_center").animate({ left: main_x }, !duration ? 1600 : duration);
}

function main_hideMainPanel(duration)
{
	if (!duration || duration == 0)		$("#div_center").css('left', $(window).width() + 'px');
	else								$("#div_center").animate({ left: $(window).width() }, !duration ? 1600 : duration);
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
		case "done":		setUpgradeDone(vEventData);			break;
	}
	//return "ok";
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
	if (size > 0)	addConsoleLog("<font color='#" + upgradedPackageColor +"'>Checking " + name + "</font>");
	else			addConsoleLog("Checking " + name);
}

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
		addConsoleLog("<font color='#" + upgradedPackageColor + "'>" + "Finalizing updates, please wait..." + "</font>");
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

function setUpgradeDone(xmlList)
{
	//Format: <list>Collon separated list of packages names</list> or without the xml syntax
	if (xmlList)
	{
		xmlList = xmlList.replace("<list>", "").replace("</list>", "");
		var pkgList = xmlList.split(";");
		
		if (pkgList.length > 0)
		{
			clearConsoleLog();
			for (var idx in pkgList)
				addConsoleLog("<font color='#" + upgradedPackageColor +"'>Upgraded " + pkgList[idx] + "</font>");
			addConsoleLog('----');
		}
	}
	
	//Hehe
	setUpgradePercentage(201, true);
	
	//Console text
	if (!isConsoleLogContains(completedTextString))
		addConsoleLog("<font color='#" + upgradedPackageColor + "'>" + completedTextString + "</font>");
	if (needReboot == "true" || needReboot == true)
		addConsoleLog("<font color='#" + rebootTextColor + "'>" + rebootingTextString + "</font>");
		
	//Show firmware version
	Hello();
			
	//Wait for 2.5 seconds then continue
	
	//Gracefully hide the update panel
	setTimeout("main_hideMainPanel(1600);", 2500+1650);
	
	//May not reach here if upgrade script reboots the system automatically
	setTimeout("location.href=\"http://localhost/\"", 2500+1650+1650);
}

//-----------------------------------------------------------

function addConsoleLog(text)
{
	var oldValue = $("#consolelog").html();
		
	//Keep only last 15 lines, do not delete those with colors
	var tempArray = oldValue.split("<br>");
	var numLines = tempArray.length;
	
	//Calculate the max number of lines
	var bottom = $("#consolelog").offset().top + $("#consolelog").height();
	var colorCount = 0;
	if ( numLines > 10 && bottom > ($(window).height() - 60) )
		maxConsoleLines = numLines - 1;
	
	while (tempArray.length > maxConsoleLines)
	{
		for (var i=0; i<tempArray.length; i++)
		{
			if (stringContains(tempArray[i], "</font>")) {
				tempArray[i] = tempArray[i].replace("Checking", "Upgraded");
				colorCount++;
				continue;
			}
			tempArray.splice(i,1);
			break;
		}
		if (colorCount >= maxConsoleLines)
		{
			for (var i=0; i<tempArray.length; i++)
			{
				//Do not delete the reboot warning
				if (stringContains(tempArray[i], rebootTextColor))
					continue;
				tempArray.splice(i,1);
				break;
			}
		}
	}
	
	//Set the new log list
	oldValue = tempArray.join("<br>");
	$("#consolelog").html(oldValue + "<br>" + text);
}

function clearConsoleLog(text)
{
	var oldValue = $("#consolelog").html("");
}

function isConsoleLogContains(text)
{
	var currentValue = $("#consolelog").html();
	return stringContains(currentValue, text);
}

//-----------------------------------------------------------

function stringEndsWith(text, searchText)
{
    return (text.match(searchText+"$")==searchText);
}

function stringStartsWith(text, searchText)
{
    return (text.indexOf(searchText) === 0) ? true : false;
}

function stringContains(text, searchText)
{
    return (text.indexOf(searchText) != -1);
}

//-----------------------------------------------------------

function onTimeoutTimer()
{
    timeoutMinCounter++;
	if (timeoutMinCounter < 25)		//allow up to 25 minutes for firmware upgrade
		return;
		
	clearInterval(timeoutTimerID);
	timeoutTimerID = 0;
	setUpgradeDone();
}




// -------------------------------------------------------------------------------------------------
//	press button on D-pad / android
// -------------------------------------------------------------------------------------------------
function fButtonPress( vButtonName )
{
	//Allow user to hide the Update UI
	if (vButtonName.toLowerCase() == 'cpanel' || vButtonName.toLowerCase() == 'widget' || vButtonName.toLowerCase() == 'setup') {
		main_hideMainPanel(1600);
		return vButtonName;
	}
	return "button ignored";
}

// -------------------------------------------------------------------------------------------------
//	NeTVBrowser will check this function every 60 seconds and reload the page if the reply is not 'true'
// -------------------------------------------------------------------------------------------------
function fCheckAlive()
{
	return true;
}


// -------------------------------------------------------------------------------------------------
//	Probe system for device info
// -------------------------------------------------------------------------------------------------
function Hello()
{
	xmlhttpPost("", "post", { 'cmd' : 'Hello' }, rawHelloCallback );
}
function rawHelloCallback(vData)
{
	if (!vData || vData.split("</status>")[0].split("<status>")[1] != "1")
		return;
	
	var helloData = new Array();
	var helloParamNamesArray = new Array('guid', 'hwver', 'fwver', 'internet', 'mac', 'minAndroid', 'minIOS', 'ip');
	for (var idx in helloParamNamesArray)
	{
		var paramName = helloParamNamesArray[idx];
		helloData[paramName] = vData.split("</"+paramName+">")[0].split("<"+paramName+">")[1];
	}

	addConsoleLog('----');
	addConsoleLog(fwverTextString + helloData['fwver']);
	addConsoleLog(minAndroidTextString + helloData['minAndroid']);
	addConsoleLog(minIOSTextString + helloData['minIOS']);	
}

// Other system events are totally ignored




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