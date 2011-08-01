var mNetConfig;
var selectedSSID;
var currentWifiList;

function onLoad()
{
	mNetConfig = cNetConfig.fGetInstance();
	mNetConfig.fInit();
	mNetConfig.wifiScanCallback = updateWifiList;
	
	selectedSSID = 'ChumbyTest';
	
	$("#div_loadingMain").fadeIn(0);
	$("#div_wifiListMain").fadeOut(0);
	$("#div_wifiDetailsMain").fadeOut(0);
	$("#div_activationMain").fadeOut(0);

	//Load later
	setTimeout("onLoadLater()", 300);	
}

function onLoadLater()
{
	mNetConfig.WifiScan();
}

//-----------------------------------------------------------

function updateWifiList(wifiListArray)
{
	if (!wifiListArray) {
		fDbg2("Wifi list is null");
		return;
	}
	currentWifiList = wifiListArray;
	
	var otherWifi = new Array();
	otherWifi['ssid'] = "Other...";
	otherWifi['ch'] = "";
	otherWifi['encryption'] = "NONE";
	otherWifi['auth'] = "OPEN";
	currentWifiList['Other...'] = otherWifi;
	
	refreshWifiList();
	$("#div_loadingMain").fadeOut(800);
	$("#div_wifiListMain").fadeIn(800);
}

function refreshWifiList()
{
	//Clear the list
	$('#div_wifiListMain_list').html('');
	
	for (var ssid in currentWifiList)
	{
		var oneWifiData = currentWifiList[ssid];
		if (oneWifiData == null)
			continue;

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
		if (oneWifiData['ch'] != '')				div_string += 'Channel ' + oneWifiData['ch'];
		if (oneWifiData['encryption'] != 'NONE')	div_string += '<img src="html_config/lock.png">';
		if (oneWifiData['encryption'] != 'NONE')	div_string += oneWifiData['encryption'];
		div_string += '</div></div>';
		$("#div_wifiListMain_list").append(div_string);
	}
}

function onRemoteControl(vButtonName)
{
	if (vButtonName == "up")
	{
		var previousSSID = "";
		for (var ssid in currentWifiList)
		{
			var oneWifiData = currentWifiList[ssid];
			if (oneWifiData == null)
				continue;
			if (ssid == selectedSSID)
				break;
			previousSSID = ssid;
		}
		if (previousSSID != "")
			selectedSSID = previousSSID;
			
		refreshWifiList();
	}
	else if (vButtonName == "down")
	{
		var found = false;
		for (var ssid in currentWifiList)
		{
			var oneWifiData = currentWifiList[ssid];
			if (oneWifiData == null)
				continue;

			if (found == true) {
				selectedSSID = ssid;
				//$("#wifi_item_'"+ssid+"'")
				break;
			}
		
			if (ssid == selectedSSID)
				found = true;
		}
		refreshWifiList();
	}
	else if (vButtonName == "center")
	{
		if (selectedSSID == "")
			return;
		var oneWifiData = currentWifiList[selectedSSID];
		if (oneWifiData == null)
			return;
				
		$("#div_wifiListMain").fadeOut(800);
		if (selectedSSID == "Other...")
		{
			$("#div_wifiDetailsMain").fadeIn(800);
			$("#wifi_ssid").val('');
			$("#wifi_password").val('');
			$("#wifi_ssid").focus();
		}
		else if (oneWifiData['encryption'] == 'NONE')
		{
			$("#div_activationMain").fadeIn(800);
		}
		else
		{
			$("#wifi_ssid").val(selectedSSID);
			$("#wifi_password").val('');
			$("#wifi_password").focus();
			$("#div_wifiDetailsMain").fadeIn(800);
		}
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
	mNetConfig.onRemoteControl(vButtonName);
}

function fNetworkEvent(	vEventName )
{
	switch (vEventName)
	{
		case "disconnected": mNetConfig.fOnSignal(cConst.SIGNAL_NETWORKEVENT_DISCONNECTED); break;
	}
}
