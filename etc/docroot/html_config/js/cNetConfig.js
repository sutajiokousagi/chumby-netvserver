// -------------------------------------------------------------------------------------------------
//	cNetConfig class
//
//
//
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
//	constructor
// -------------------------------------------------------------------------------------------------
function cNetConfig()
{
	this.wifiListArray = new Array();
	this.wifiParamNamesArray = new Array('ssid', 'qty', 'lvl', 'ch', 'mode', 'encryption', 'auth');
	this.helloParamNamesArray = new Array('guid', 'hwver', 'fwver', 'internet', 'mac', 'minAndroid', 'minIOS', 'ip');
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
//	callbacks (to be overriden)
// -------------------------------------------------------------------------------------------------

cNetConfig.prototype.wifiScanCallback = function (wifilist) {};
cNetConfig.prototype.helloCallback = function (params) {};


// -------------------------------------------------------------------------------------------------
//	communication
// -------------------------------------------------------------------------------------------------

cNetConfig.prototype.WifiScan = function (useForce)
{
	if (!useForce)
	{
		fDbg2("Scanning for wifi...");
		xmlhttpPost("", "post", { 'cmd' : 'WifiScan', 'value' : '' }, cNetConfig.instance.rawWifiScanCallback );
	}
	else
	{
		fDbg2("Scanning for wifi...[forced]");
		xmlhttpPost("", "post", { 'cmd' : 'WifiScan', 'value' : '-f' }, cNetConfig.instance.rawWifiScanCallback );
	}
}

cNetConfig.prototype.Hello = function ()
{
	fDbg2("Waiting for handshake response...");
	xmlhttpPost("", "post", { 'cmd' : 'Hello' }, cNetConfig.instance.rawHelloCallback );
}

cNetConfig.prototype.StartAP = function ()
{
	fDbg2("Starting Access Point mode...");
	xmlhttpPost("", "post", { 'cmd' : 'start_ap' }, null );
}

cNetConfig.prototype.StopAP = function ()
{
	fDbg2("Stopping Access Point mode...");
	xmlhttpPost("", "post", { 'cmd' : 'stop_ap' }, null );
}

cNetConfig.prototype.SetNetwork = function (ssid, key, security)
{
	var param = [];
	param['cmd'] = 'SetNetwork';
	
	var oneWifiData = cNetConfig.instance.getWifiData(ssid);
	
	//Manual network ssid
	if (oneWifiData == null)
	{
		param['wifi_ssid'] = ssid;
		if (key != null && key != '')					param['wifi_password'] = key;
		if (security)
		{
			if (security.toUpperCase() == "WEP")		{	param['wifi_encryption'] = "WEP";		param['wifi_authentication'] = "WEPAUTO";	}
			else if (security.toUpperCase() == "WPA")	{	param['wifi_encryption'] = "AES";		param['wifi_authentication'] = "WPA2PSK";	}
			else										{	param['wifi_encryption'] = "NONE";		param['wifi_authentication'] = "OPEN";		}
		}
	}
	//Auto
	else
	{
		param['wifi_ssid'] = oneWifiData['ssid'];
		param['wifi_authentication'] = oneWifiData['auth'];
		param['wifi_encryption'] = oneWifiData['encryption'];
		param['wifi_password'] = oneWifiData['ssid'];
		if (key != null && key != '')					param['wifi_password'] = key;
	}
	
	fDbg2("Setting network config...");
	xmlhttpPost("", "post", param, cNetConfig.instance.rawSetNetworkCallback );
}

cNetConfig.prototype.SetWifiScanData = function (vData)
{
	cNetConfig.instance.rawWifiScanCallback(vData, true);
}

// -------------------------------------------------------------------------------------------------
//	internal callbacks
// -------------------------------------------------------------------------------------------------

cNetConfig.prototype.rawWifiScanCallback = function (vData, doNotSort)
{
	if (vData == null || vData.split == undefined)
		return;
		
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
		//We need to decode XML escape characters since we are using responseText
		oneWifiArray['ssid'] = XmlUnescape( oneWifiArray['ssid'] );
		
		count++;
		var index = cNetConfig.instance.getWifiIndex( oneWifiArray['ssid'] );
		if (index >= 0)		cNetConfig.instance.wifiListArray[index] = oneWifiArray;		//update
		else				cNetConfig.instance.wifiListArray.push( oneWifiArray );			//add
	}
	
	fDbg2("" + count + " wifi networks found");
	if (count <= 0 && doNotSort != true && doNotSort != "true") {							//from internal, not Android/iOS
		fDbg2("No wifi found, force rescan!");
		cNetConfig.instance.WifiScan(true);
	}
		
	//Bubbling
	if (doNotSort != true && doNotSort != "true")
		cNetConfig.instance.sortWifiList();
	
	if (cNetConfig.instance.wifiScanCallback)
		cNetConfig.instance.wifiScanCallback(cNetConfig.instance.wifiListArray);
}

cNetConfig.prototype.rawSetNetworkCallback = function (vData)
{
	if (vData.split("</status>")[0].split("<status>")[1] != "1")
		return;
}

cNetConfig.prototype.rawHelloCallback = function (vData)
{
	if (vData.split("</status>")[0].split("<status>")[1] != "1")
	{
		if (cNetConfig.instance.helloCallback)
			cNetConfig.instance.helloCallback(null);
		return;
	}
	
	var helloData = new Array();
	for (var idx in cNetConfig.instance.helloParamNamesArray)
	{
		var paramName = cNetConfig.instance.helloParamNamesArray[idx];
		helloData[paramName] = vData.split("</"+paramName+">")[0].split("<"+paramName+">")[1];
	}

	if (cNetConfig.instance.helloCallback)
		cNetConfig.instance.helloCallback(helloData);
}

//----------------------------------------------------------------------------------
// Helper functions
//----------------------------------------------------------------------------------

// Bubble sort
cNetConfig.prototype.sortWifiList = function ()
{
	var theList = cNetConfig.instance.wifiListArray;
	for (var first = 0; first < theList.length-1; first++)
	{
		for (var i = 0; i < theList.length-first-1; i++)
		{
			var oneWifiData1 = theList[i];
			var oneWifiData2 = theList[i+1];
			if (oneWifiData2['qty'] > oneWifiData1['qty'])
			{
				var temp = oneWifiData1;
				theList[i] = oneWifiData2;
				theList[i+1] = temp;
			}
		}
	}
}

cNetConfig.prototype.getWifiIndex = function (ssid)
{
	if (ssid == null || ssid == '')
		return -1;
	var theList = cNetConfig.instance.wifiListArray;
	var count = cNetConfig.instance.getWifiCount();
	for (var i = 0; i < count; i++)
	{
		var oneWifiData = theList[i];
		if (oneWifiData == null)
			continue;		
		if (oneWifiData['ssid'] == ssid)
			return i;
	}
	return -1;
}

cNetConfig.prototype.getWifiData = function (ssid)
{
	var index = cNetConfig.instance.getWifiIndex(ssid);
	if (index < 0)
		return null;
	return cNetConfig.instance.wifiListArray[index];
}

cNetConfig.prototype.hasSSID = function (ssid)
{
	return cNetConfig.instance.getWifiIndex(ssid) >= 0;
}

cNetConfig.prototype.getWifiCount = function ()
{
	var count = 0;
	for (var objectIndex in cNetConfig.instance.wifiListArray)
		count++;
	return count;
}