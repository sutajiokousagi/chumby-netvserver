var selectedSSID;
var wifiScanTimer;
var currentWifiList;

function wifilist_init()
{
	selectedSSID = '';
	wifiScanTimer = null;
	currentWifiList = null;
	
	//Start scanning
	onWifiScanTimer();
}

//-----------------------------------------------------------

function onWifiScanTimer()
{
	mNetConfig.WifiScan();
	mNetConfig.wifiScanCallback = wifilist_updateWifiList;
	
	wifiScanTimer = null;
	wifilist_resetWifiScanTimer();
}

function wifilist_resetWifiScanTimer(duration)
{
	wifilist_stopWifiScanTimer();	
	wifiScanTimer = setTimeout("onWifiScanTimer()", !duration ? 30000 : duration);
}

function wifilist_stopWifiScanTimer()
{
	if (wifiScanTimer != null)
		clearTimeout(wifiScanTimer);
}

//-----------------------------------------------------------

function wifilist_updateWifiList(wifiListArray)
{
	if (!wifiListArray) {
		fDbg2("Wifi list is null");
		return;
	}
	currentWifiList = wifiListArray;
	
	//Last item is "Other..."
	var otherWifi = new Array();
	otherWifi['ssid'] = "Other...";
	otherWifi['encryption'] = "NONE";
	otherWifi['auth'] = "OPEN";
	otherWifi['ch'] = "";
	otherWifi['lvl'] = "";
	otherWifi['qty'] = "";
	currentWifiList['Other...'] = otherWifi;
	
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
		if (previousSSID != "")
			selectedSSID = previousSSID;
			
		refreshWifiList();
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
				break;
			}
		
			if (ssid == selectedSSID)
				found = true;
		}
		refreshWifiList();
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
			main_showState("wifidetails", true);
			wifidetails_highlightItem("ssid");
		}
		else if (oneWifiData['encryption'] == 'NONE')
		{
			main_showState("configuring", true);
			mNetConfig.SetNetwork(selectedSSID, '');
		}
		else
		{
			main_showState("wifidetails", true);
			wifidetails_setItemValue("ssid", selectedSSID);
			wifidetails_highlightItem("password");
		}
	}
}
