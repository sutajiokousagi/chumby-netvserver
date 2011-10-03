var androidAliveTimer = null;
var androidTimeoutDuration = 5000;
var androidClearTimeout = false;

// -------------------------------------------------------------------------------------------------
//	Events from Android app
// -------------------------------------------------------------------------------------------------

function fAndroidEvents( vEventName, vEventData )
{	
	vEventData = decodeURIComponent(vEventData);
	switch (vEventName)
	{
		case "activationcancel":	android_clearTimeout();		location.href="http://localhost/";		break;
		case "changeview":			android_clearTimeout();		android_changeView(vEventData);			break;
		case "wifiscan":			android_resetTimeout();		android_wifiScan(vEventData);			break;
		case "wifiselect":			android_resetTimeout();		android_selectWifi(vEventData);			break;
		case "wifidetails":			android_resetTimeout();		android_setWifiDetails(vEventData);		break;
		case "wificonfiguring":		android_clearTimeout();		android_setWifiDetails(vEventData);		break;
		case "accountdetails":		android_clearTimeout();		android_setAccountDetails(vEventData);	break;
	}
	if (vEventData.length > 16)		fDbg2("Received Android event: [" + vEventName + "] data length: " + vEventData.length);
	else							fDbg2("Received Android event: [" + vEventName + "] " + vEventData);
}

function android_changeView(xmlDataString)
{
	if (xmlDataString == "remote")
	{
		android_clearTimeout();
		location.href="http://localhost/";
	}
	else if (xmlDataString == "wificonfiguring")
	{
		android_clearTimeout();
		main_showState(xmlDataString, true);
	}
	else
	{
		main_showState(xmlDataString, true);
	}
}

function android_wifiScan(xmlDataString)
{
	//Show WiFi list UI if it's not currently being shown
	var currentState = main_currentState();
	if (currentState != "wifilist")
		main_showState("wifilist", true);
		
	mNetConfig.SetWifiScanData(xmlDataString);
}

function android_selectWifi(xmlDataString)
{	
	//Show WiFi list UI if it's not currently being shown
	var currentState = main_currentState();
	if (currentState != "wifilist")
		main_showState("wifilist", true);
		
	wifilist_setSelectedSSID(xmlDataString);
}

function android_setWifiDetails(xmlDataString)
{
	var ssid = xmlDataString.split("</wifi_ssid>")[0].split("<wifi_ssid>")[1];
	var passwordMask = xmlDataString.split("</wifi_password>")[0].split("<wifi_password>")[1];			//xxxxx mask only, for security reason
	var encryption = xmlDataString.split("</wifi_encryption>")[0].split("<wifi_encryption>")[1];
	var auth = xmlDataString.split("</wifi_authentication>")[0].split("<wifi_authentication>")[1];
	
	//Show WiFi details UI if it's not currently being shown
	var currentState = main_currentState();
	if (currentState != "wifidetails")
		main_showState("wifidetails", true);
		
	//UI
	wifidetails_setItemValue('ssid', ssid);
	wifidetails_setItemValue('password', passwordMask);
	if (auth == null || auth == "" || auth.toUpperCase() == "OPEN")				auth = "OPEN";
	else if (auth.toUpperCase() == "WEP" || auth.toUpperCase() == "WEPAUTO")	auth = "WEP";
	else																		auth = "WPA";
	wifidetails_setItemValue('security', auth);
}

function android_setAccountDetails(xmlDataString)
{
	var username = xmlDataString.split("</chumby_username>")[0].split("<chumby_username>")[1];
	var passwordMask = xmlDataString.split("</chumby_password>")[0].split("<chumby_password>")[1];			//xxxxx mask only, for security reason
	var devicename = xmlDataString.split("</chumby_device_name>")[0].split("<chumby_device_name>")[1];
	
	//Show account details UI if it's not currently being shown
	var currentState = main_currentState();
	if (currentState != "accountdetails")
		main_showState("accountdetails", true);
	
	//UI
	$("#chumby_username").val(ssid);
	$("#chumby_password").val(passwordMask);
	$("#chumby_device_name").val(devicename);
}

function android_clearTimeout()
{
	if (androidAliveTimer != null)
		clearTimeout(androidAliveTimer);
	androidAliveTimer = null;
	androidClearTimeout = true;
}

function android_resetTimeout()
{
	android_clearTimeout();
	androidAliveTimer = setTimeout("android_timeout()", androidTimeoutDuration);
	androidClearTimeout = false;
}
	
function android_timeout()
{
	if (androidClearTimeout == true)
		return;
	fDbg2("Android html_config timeout...");
	location.href="http://localhost/";
}