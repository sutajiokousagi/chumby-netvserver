var mNetConfig;
var selectedSSID;
var currentWifiList;
var wifiScanTimer;

var activationState;
var activationTimer;
var activationTime;

var selection_currentX;
var selection_currentY;

function onLoad()
{
	keyboard_init();
	
	mNetConfig = cNetConfig.fGetInstance();
	mNetConfig.fInit();
	mNetConfig.wifiScanCallback = updateWifiList;
	mNetConfig.helloCallback = helloCallback;
	
	selectedSSID = '';
	activationState = '';
	activationTimer = null;
	
	$("#div_loadingMain").fadeIn(0);
	$("#div_wifiListMain").fadeOut(0);
	$("#div_wifiDetailsMain").fadeOut(0);
	$("#div_activationMain").fadeOut(0);

	//Load later
	wifiScanTimer = setTimeout("onLoadLater()", 300);	
}

function initWifiDetails()
{
	selection_currentX = 0;
	selection_currentY = 0;
}

//-----------------------------------------------------------

function onLoadLater()
{
	mNetConfig.WifiScan();
	resetWifiScanTimer();
}

function resetWifiScanTimer()
{
	stopWifiScan();
	wifiScanTimer = setTimeout("onLoadLater()", 30000);
}

function stopWifiScan()
{
	clearTimeout(wifiScanTimer);
}

function startActivation()
{
	activationTime = new Date();
	activationState = 'waiting';
	resetActivationTimer();
}

function stopActivation()
{
	activationState = '';
	clearTimeout(activationTimer);
}

function resetActivationTimer()
{
	clearTimeout(activationTimer);
	activationTimer = setTimeout("onActivationTimer()", 5000);
}


//-----------------------------------------------------------

function updateWifiList(wifiListArray)
{
	if (!wifiListArray) {
		fDbg2("Wifi list is null");
		return;
	}
	currentWifiList = wifiListArray;
	
	/*
	var otherWifi = new Array();
	otherWifi['ssid'] = "Other...";
	otherWifi['ch'] = "";
	otherWifi['encryption'] = "NONE";
	otherWifi['auth'] = "OPEN";
	otherWifi['lvl'] = "";
	otherWifi['qty'] = "";
	currentWifiList['Other...'] = otherWifi;
	*/
	
	refreshWifiList();
	$("#div_loadingMain").fadeOut(800);
	$("#div_wifiListMain").fadeIn(800);
}

function refreshWifiList()
{
	//Clear the list
	$('#div_wifiListMain_list').html('');
	
	for (var objectIndex in currentWifiList)
	{
		var oneWifiData = currentWifiList[objectIndex];
		if (oneWifiData == null)
			continue;
		var ssid = oneWifiData['ssid'];
		
		//Select one by default
		if (selectedSSID == '')
			selectedSSID = ssid;
		
		//The row		
		var div_string = '<div id="wifi_item_'+ssid+'" class="';
		if (ssid == selectedSSID)	div_string += 'div_wifiListItem_selected';
		else						div_string += 'div_wifiListItem_normal';
		
		//title
		div_string += '"><div class="div_wifiListItem_ssid">'+ssid+'&nbsp;</div> ';
		
		//the rest
		div_string += '<div style="float:right">';
		//if (oneWifiData['ch'] != '')				div_string += 'Channel ' + oneWifiData['ch'];	

		//encryption
		if (oneWifiData['encryption'] != 'NONE')	div_string += oneWifiData['encryption'];
		if (oneWifiData['encryption'] != 'NONE')	div_string += '<img src="images/lock.png">';
		
		//signal level (magic number)
		if (oneWifiData['lvl'] != '')
		{
			var lvl = 3 - Math.round( (parseInt(oneWifiData['lvl'])+19) / (-20) );
			if (lvl < 0) 		lvl = 0;
			div_string += '<img src="images/wifi' + lvl + '.png">';
		}
		
		div_string += '</div></div>';
		$("#div_wifiListMain_list").append(div_string);
	}
}

//-----------------------------------------------------------

function onRemoteControl(vButtonName)
{
	if ( $("#div_wifiListMain").is(":visible") )				onRemoteControlWifiList(vButtonName);
	else if ( $("#div_wifiDetailsMain").is(":visible") )		onRemoteControlWifiDefails(vButtonName);
	else if ( $("#div_activationMain").is(":visible") )			onRemoteControlActivation(vButtonName);
}

function onRemoteControlWifiList(vButtonName)
{
	if (vButtonName == "up")
	{
		var previousSSID = "";
		for (var objectIndex in currentWifiList)
		{
			var oneWifiData = currentWifiList[objectIndex];
			if (oneWifiData == null)
				continue;
			var ssid = oneWifiData['ssid'];
			
			if (ssid == selectedSSID)
				break;
			previousSSID = ssid;
		}
		if (previousSSID != "")
			selectedSSID = previousSSID;
			
		refreshWifiList();
		resetWifiScanTimer();
	}
	else if (vButtonName == "down")
	{
		var found = false;
		for (var objectIndex in currentWifiList)
		{
			var oneWifiData = currentWifiList[objectIndex];
			if (oneWifiData == null)
				continue;
			var ssid = oneWifiData['ssid'];

			if (found == true) {
				selectedSSID = ssid;
				break;
			}
		
			if (ssid == selectedSSID)
				found = true;
		}
		refreshWifiList();
		resetWifiScanTimer();
	}
	else if (vButtonName == "center")
	{
		if (selectedSSID == "")
			return;
		var oneWifiData = mNetConfig.getWifiData(selectedSSID);
		if (oneWifiData == null)
			return;
			
		stopWifiScan();
				
		$("#div_wifiListMain").fadeOut(800);
		if (selectedSSID == "Other...")
		{
			$("#div_wifiDetailsMain").fadeIn(800);
			$("#wifi_password").val('');
			$("#wifi_ssid").val('');
			selection_currentY = 0;
			onRemoteControlWifiDefails("center");
		}
		else if (oneWifiData['encryption'] == 'NONE')
		{
			$("#div_activationMain").fadeIn(800);
			mNetConfig.SetNetwork(selectedSSID, '');
			startActivation();
		}
		else
		{
			$("#div_wifiDetailsMain").fadeIn(800);
			$("#wifi_ssid").val(selectedSSID);
			$("#wifi_password").val('');
			selection_currentY = 1;
			onRemoteControlWifiDefails("center");
		}
	}
}

function onRemoteControlWifiDefails(vButtonName)
{
	var newY = selection_currentY;
	if (newY == 0)
	{
		if (vButtonName == "up")			newY = 0;
		else if (vButtonName == "down")		newY++;
		else if (vButtonName == "center")
		{
			$("#wifi_password").blur();
			$("#wifi_password").removeClass("input_focus");
			$("#wifi_ssid").focus();
			$("#wifi_ssid").addClass("input_focus");
		}
	}
	else if (newY == 1)
	{
		if (vButtonName == "up")			newY--;
		else if (vButtonName == "down")		newY++;
		else if (vButtonName == "center")
		{
			$("#wifi_ssid").blur();
			$("#wifi_ssid").removeClass("input_focus");
			$("#wifi_password").focus();
			$("#wifi_password").addClass("input_focus");
		}
	}
	else if (newY == 2)
	{
		var focusElement = $("#wifi_ssid").hasClass("input_focus") ? "wifi_ssid" : "wifi_password";
		var returnFocus = keyboard_onRemoteControl(vButtonName, focusElement);
		if (returnFocus)
		{
			if (vButtonName == "down")			newY++;
			else if (vButtonName == "up")		newY--;
		}
	}
	else if (newY == 3)
	{
		if (vButtonName == "down")			newY = 3;
		else if (vButtonName == "up")		newY--;
		else if (vButtonName == "center")
		{
			selectedSSID = $("#wifi_ssid").val();
			var password = $("#wifi_password").val();
			
			//Validate input fields
			if (selectedSSID == "") {
				selection_currentY = 0;
				onRemoteControlWifiDefails("center");
				$("#wifi_password").addClass("input_focus_error");
				return;
			}
			if (password == "") {
				selection_currentY = 1;
				onRemoteControlWifiDefails("center");
				$("#wifi_password").addClass("input_focus_error");
				return;
			}
			var oneWifiData = mNetConfig.getWifiData(selectedSSID);
												
			$("#div_wifiDetailsMain").fadeOut(800);
			$("#div_activationMain").fadeIn(800);
			mNetConfig.SetNetwork(selectedSSID, password);
			startActivation();
		}
	}
	
	//Highlight new item, avoid 2
	$("#div_wifiListDetails"+selection_currentY).removeClass("div_wifiDetailsItem_selected").addClass("div_wifiDetailsItem_normal");
	if (newY != 2)
	{
		$("#div_wifiListDetails"+newY).removeClass("div_wifiDetailsItem_normal").addClass("div_wifiDetailsItem_selected");
	}
	else if (selection_currentY != 2)
	{
		var focusElement = $("#wifi_ssid").hasClass("input_focus") ? "wifi_ssid" : "wifi_password";
		var returnFocus = keyboard_onRemoteControl(vButtonName, focusElement);
	}
	selection_currentY = newY;
}

function onActivationTimer()
{
	//not performing activation
	if (activationState == '') {
		stopActivation();
		return;
	}
	
	resetActivationTimer();
	if (activationState == "waiting")
		mNetConfig.Hello();
}

function helloCallback(helloData)
{
	//not performing activation
	if (activationState == '' || helloData == null)
		return;
	
	var internet = helloData['internet'];
	var ip = helloData['ip'];
	var secondsSinceActivation = ((new Date()).getTime() - activationTime.getTime())/1000;
	
	//Taking too long
	if (secondsSinceActivation > 60)
	{
		stopActivation();
		$("#div_activationStatus").append("<br>Taking too long. Give up!");
		//Revert to ap_mode.
		mNetConfig.StartAP();
		
		//Check error. How?
		
		//Back to wifi list
		$("#div_activationMain").fadeOut(800);
		$("#div_wifiListMain").fadeIn(800);
		resetWifiScanTimer();
		return;
	}
	
	if (ip == '')
	{
		$("#div_activationStatus").append("<br>still waiting...");
	}
	else if (activationState == "waiting")
	{
		if (internet != 'true' && internet != true)
			return;
			
		$("#div_activationStatus").append("<br>Network configured!");
		$("#div_activationStatus").append("<br>" + ip);
		stopActivation();
		location.href="http://localhost/";
	}
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
//	system API functions
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
//	server reset - browser start, and sconnected to NeTVServer
// -------------------------------------------------------------------------------------------------
function fServerReset()
{
	fDbg2("fServerReset()");
}

// -------------------------------------------------------------------------------------------------
//	press button on D-pad / android
// -------------------------------------------------------------------------------------------------
function fButtonPress( vButtonName )
{
	onRemoteControl(vButtonName);
	//mNetConfig.onRemoteControl(vButtonName);
}

// -------------------------------------------------------------------------------------------------
//	events from HDMI/FPGA
// -------------------------------------------------------------------------------------------------
function fHDMIEvents( vEventName )
{
	//To be decided
	if (vEventName == "unsupported");
	if (vEventName == "attach");
	if (vEventName == "detach");
	if (vEventName == "trigger");
}

// -------------------------------------------------------------------------------------------------
//	events from DBus/NetworkManager
// -------------------------------------------------------------------------------------------------
function fNMStateChanged( vEventName )
{
	//not performing activation, don't care
	if (activationState == '')
		return;
	$("#div_activationStatus").append("<br>" + vEventName);
	
	switch (vEventName)
	{
		case "unknown":			break;
		case "sleeping":		break;
		case "connecting":		break;
		case "disconnected":	break;
		case "connected":		break;
	}
}

function fNMDeviceAdded(  )
{
	//Switching back FROM Access Point mode
	//not performing activation, don't care
	if (activationState == '')
		return;
}

function fNMDeviceRemoved(  )
{
	//Switching TO Access Point mode
	//not performing activation, don't care
	if (activationState == '')
		return;
}