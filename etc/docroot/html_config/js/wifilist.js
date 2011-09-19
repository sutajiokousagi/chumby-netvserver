var selectedSSID;
var selectedIndex;
var startingIndex;
var maxWifiDisplay;
var wifiScanTimer;
var currentWifiList;

function wifilist_init()
{
	selectedSSID = '';
	startingIndex = 0;
	selectedIndex = 0;
	maxWifiDisplay = 10;
	wifiScanTimer = null;
	currentWifiList = null;
	
	mNetConfig.wifiScanCallback = wifilist_updateWifiList;
}

//-----------------------------------------------------------

function onWifiScanTimer()
{
	mNetConfig.WifiScan(true);
	mNetConfig.wifiScanCallback = wifilist_updateWifiList;
	
	wifiScanTimer = null;
	wifilist_resetWifiScanTimer();
}

function wifilist_startWifiScan()
{
	onWifiScanTimer();
}

function wifilist_resetWifiScanTimer(duration)
{
	wifilist_stopWifiScanTimer();
	mNetConfig.wifiScanCallback = wifilist_updateWifiList;
	wifiScanTimer = setTimeout("onWifiScanTimer()", !duration ? 30000 : duration);
}

function wifilist_stopWifiScanTimer()
{
	mNetConfig.wifiScanCallback = null;
	if (wifiScanTimer != null)
		clearTimeout(wifiScanTimer);
}

//-----------------------------------------------------------
function wifilist_setSelectedSSID(newSSID)
{
	for (var objectIndex in currentWifiList)
	{
		var oneWifiData = currentWifiList[objectIndex];
		if (oneWifiData == null)
			continue;
		var ssid = oneWifiData['ssid'];
		if (ssid != newSSID)
			continue;
		
		selectedSSID = newSSID;
		selectedIndex = objectIndex;
		break;
	}

	fadeInWifiList(false);
}

function wifilist_updateWifiList(wifiListArray)
{
	if (!wifiListArray) {
		fDbg2("Wifi list is null");
		return;
	}
	currentWifiList = [];
	currentWifiList = wifiListArray;
	
	//Last item is "Other..."
	var otherWifi = new Array();
	otherWifi['ssid'] = "Other...";
	otherWifi['encryption'] = "NONE";
	otherWifi['auth'] = "OPEN";
	otherWifi['ch'] = "";
	otherWifi['lvl'] = "";
	otherWifi['qty'] = "";
	currentWifiList.push( otherWifi );
	
	refreshWifiList(true);
	if (main_currentState() == "loading")
		main_showState("wifilist", true);
}

function refreshWifiList(animated)
{
	//Fade out the current list
	if (animated == true)
	{
		$('#div_wifiListMain_list').fadeOut(350);
		setTimeout("fadeInWifiList(true)", 370);
	}
	else
	{
		fadeInWifiList(false);
	}
}	

function fadeInWifiList(animated)
{
	//Clear the list
	$('#div_wifiListMain_list').html('');
	
	//Adjust the display list
	if (selectedSSID == 'Other...')
		selectedIndex = currentWifiList.length-1;
		
	if (selectedIndex < startingIndex)
		startingIndex = selectedIndex;
	if (selectedIndex >= startingIndex + maxWifiDisplay - 1)
		startingIndex = selectedIndex - maxWifiDisplay + 1;
		
	//more... arrow indicator
	if (startingIndex > 0)												$("#div_wifiListMain_moretop").fadeIn(400);
	else																$("#div_wifiListMain_moretop").fadeOut(400);
	if (startingIndex+maxWifiDisplay < currentWifiList.length-1)		$("#div_wifiListMain_morebottom").fadeIn(400);
	else																$("#div_wifiListMain_morebottom").fadeOut(400);
		
	for (var objectIndex=startingIndex; objectIndex < startingIndex+maxWifiDisplay; objectIndex++)
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
		div_string += '<div class="div_wifiListItem_info">';

		//encryption
		//if (oneWifiData['encryption'] != 'NONE')	div_string += oneWifiData['encryption'];
		if (oneWifiData['encryption'] != 'NONE')	div_string += '<img src="images/lock.png">';
		
		//signal level (magic number)
		if (oneWifiData['lvl'] != '')
		{
			var lvl = Math.round( parseInt(oneWifiData['qty']) / 20.0 );
			if (lvl > 3) 			lvl = 3;
			else if (lvl < 0) 		lvl = 0;
			div_string += '<img src="images/wifi' + lvl + '.png">';
		}
		
		div_string += '</div></div>';
		$("#div_wifiListMain_list").append(div_string);
	}
	$('#div_wifiListMain_list').fadeIn( animated ? 350 : 0);
}

//-----------------------------------------------------------

function wifilist_onRemoteControl(vButtonName)
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
		if (previousSSID != "") {
			selectedSSID = previousSSID;
			selectedIndex = objectIndex-1;
		}
			
		fadeInWifiList(false);
		wifilist_resetWifiScanTimer();
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
				selectedIndex = objectIndex;
				break;
			}
		
			if (ssid == selectedSSID)
				found = true;
		}
		fadeInWifiList(false);
		wifilist_resetWifiScanTimer();
	}
	else if (vButtonName == "center")
	{
		if (selectedSSID == "")
			return;
		var oneWifiData = mNetConfig.getWifiData(selectedSSID);
		if (oneWifiData == null)
			return;
			
		wifilist_stopWifiScanTimer();
				
		if (selectedSSID == "Other...")
		{
			wifidetails_highlightItem("ssid");
			main_showState("wifidetails", true);
		}
		else if (oneWifiData['encryption'] == 'NONE')
		{
			mNetConfig.SetNetwork(selectedSSID, '');
			main_showState("configuring", true);
		}
		else
		{
			main_showState("wifidetails", true);
			wifidetails_setItemValue("ssid", selectedSSID);
			wifidetails_highlightItem("password");
		}
	}
}
