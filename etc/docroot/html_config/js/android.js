// -------------------------------------------------------------------------------------------------
//	Events from Android app
// -------------------------------------------------------------------------------------------------

function fAndroidEvents( vEventName, vEventData )
{
	vEventData = decodeURIComponent(vEventData);
	switch (vEventName)
	{
		case "activationcancel":	location.href="http://localhost/";		break;
		case "changeview":			android_changeView(vEventData);			break;
		case "wifiscan":			android_wifiScan(vEventData);			break;
		case "wifiselect":			android_selectWifi(vEventData);			break;
		case "wifidetails":			android_setWifiDetails(vEventData);		break;
		case "wificonfiguring":		android_setWifiDetails(vEventData);		break;
		case "accountdetails":		android_setAccountDetails(vEventData);	break;
	}
}

function android_changeView(xmlDataString)
{
	if (xmlDataString == "remote")
	{
		location.href="http://localhost/";
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
	$("#wifi_ssid").val(ssid);
	$("#wifi_password").val(passwordMask);
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