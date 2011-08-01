// -------------------------------------------------------------------------------------------------
//	cNetConfig class
//
//
//
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
//	constructor
// -------------------------------------------------------------------------------------------------
function cNetConfig(
)
{
	this.wifiListArray = new Array();
	this.wifiParamNamesArray = new Array('ssid', 'qty', 'lvl', 'ch', 'mode', 'encryption', 'auth');
}

// -------------------------------------------------------------------------------------------------
//	singleton
// -------------------------------------------------------------------------------------------------
cNetConfig.instance = null;
cNetConfig.fGetInstance = function()
{
	return cNetConfig.instance ? cNetConfig.instance : (cNetConfig.instance = new cNetConfig());
}

// -------------------------------------------------------------------------------------------------
//	fInit
// -------------------------------------------------------------------------------------------------
cNetConfig.prototype.fInit = function()
{

}


// -------------------------------------------------------------------------------------------------
//	callbacks
// -------------------------------------------------------------------------------------------------

cNetConfig.prototype.wifiScanCallback = function (wifilist) {};


// -------------------------------------------------------------------------------------------------
//	communication
// -------------------------------------------------------------------------------------------------

cNetConfig.prototype.WifiScan = function ()
{
	var web_address = "./bridge";
	fDbg2("Scanning for wifi...");
	xmlhttpPost("", "post", { 'cmd' : 'WifiScan' }, cNetConfig.instance.rawWifiScanCallback );
}

cNetConfig.prototype.rawWifiScanCallback = function (vData)
{
	if (vData.split("</status>")[0].split("<status>")[1] != "1")
		return;
	
	vData = vData.split("</data>")[0].split("<data>")[1];		
	vRawArray = vData.split("<wifi>");
	var count = 0;
	for (var objectIndex in vRawArray)
	{
		var oneWifiString = vRawArray[objectIndex];
		var oneWifiArray = new Array();
		for (var idx in cNetConfig.instance.wifiParamNamesArray)
		{
			var paramName = cNetConfig.instance.wifiParamNamesArray[idx];
			oneWifiArray[paramName] = oneWifiString.split("</"+paramName+">")[0].split("<"+paramName+">")[1];
		}
		if (oneWifiArray['ssid'] == null || oneWifiArray['ssid'] == '')
			continue;
		
		count++;
		cNetConfig.instance.wifiListArray[oneWifiArray['ssid']] = oneWifiArray;
	}
	
	fDbg2("" + count + " wifi networks found");
	if (cNetConfig.instance.wifiScanCallback)
		cNetConfig.instance.wifiScanCallback(cNetConfig.instance.wifiListArray);
}

cNetConfig.prototype.onRemoteControl = function (vButtonName)
{
	/*
	switch (vButtonName)
	{
		case "cpanel": mNetConfig.fOnSignal(cConst.SIGNAL_TOGGLE_CONTROLPANEL); break;
		case "widget": mNetConfig.fOnSignal(cConst.SIGNAL_TOGGLE_WIDGETENGINE); break;
		case "left": mNetConfig.fOnSignal(cConst.SIGNAL_BUTTON_LEFT); break;
		case "right": mNetConfig.fOnSignal(cConst.SIGNAL_BUTTON_RIGHT); break;
		case "center": mNetConfig.fOnSignal(cConst.SIGNAL_BUTTON_CENTER); break;
		case "up": mNetConfig.fOnSignal(cConst.SIGNAL_BUTTON_UP); break;
		case "down": mNetConfig.fOnSignal(cConst.SIGNAL_BUTTON_DOWN); break;
	}
	*/
}

cNetConfig.prototype.getWifiData = function (ssid)
{
	if (ssid == null || ssid == '')
		return null;
	return wifiListArray[ssid];
}