var wifidetails_selectedIndex;
var wifidetails_showingSecurity;

function wifidetails_init()
{
	wifilist_stopWifiScanTimer();

	//Clear previous selection
	/*
	wifidetails_selectedIndex = 0;
	for (var i=0; i<5; i++)
		$("#div_wifiListDetails"+i).removeClass("div_wifiDetailsItem_selected").addClass("div_wifiDetailsItem_normal");
	*/
}

function wifidetails_highlightItem(itemName)
{
	//Clear previous selection
	wifidetails_selectedIndex = 0;
	for (var i=0; i<5; i++)
		$("#div_wifiListDetails"+i).removeClass("div_wifiDetailsItem_selected").addClass("div_wifiDetailsItem_normal");
		
	switch (itemName)
	{
		case "ssid":		wifidetails_selectedIndex = 0;		break;
		case "password":	wifidetails_selectedIndex = 1;		break;
		case "security1":	wifidetails_selectedIndex = 2;		break;
		case "security2":	wifidetails_selectedIndex = 3;		break;
		case "security3":	wifidetails_selectedIndex = 4;		break;
		case "keyboard":	wifidetails_selectedIndex = 5;		break;
		case "next":		wifidetails_selectedIndex = 6;		break;
		case "back":		wifidetails_selectedIndex = 7;		break;
		default:			wifidetails_selectedIndex = 0;		break;
	}
	//Highlight it
	if (itemName != "keyboard")
		wifidetails_onRemoteControl("center");
	return wifidetails_selectedIndex;
}

function wifidetails_setItemValue(itemName, newValue)
{
	var oldValue = "";
	switch (itemName)
	{
		case "ssid":		oldValue = $("#wifi_ssid").val();		$("#wifi_ssid").val(newValue);			break;
		case "password":	oldValue = $("#wifi_password").val();	$("#wifi_password").val(newValue);		break;
		case "security":	return wifidetails_setSecurity(newValue); 										break;
	}
	return oldValue;
}

function wifidetails_setSecurity(securityName)
{
	$("#wifi_security_none").removeAttr("checked");
	$("#wifi_security_wep").removeAttr("checked");
	$("#wifi_security_wpa").removeAttr("checked");
			
	securityName = securityName.toUpperCase();
	if (securityName == null || securityName == "NONE" || securityName == "OPEN" || securityName == "")
		$("#wifi_security_none_wrapper").click();
	else if (securityName == "WEP" || securityName == "WEPAUTO")
		$("#wifi_security_wep_wrapper").click();
	else
		$("#wifi_security_wpa_wrapper").click();
	return "";
}

function wifidetails_showSecurity(isShow)
{
	wifidetails_showingSecurity = isShow;
	if (!isShow)
	{
		$("#div_wifiListDetails2").hide();
		$("#div_wifiListDetails3").hide();
		$("#div_wifiListDetails4").hide();
	}
	else
	{
		$("#div_wifiListDetails2").show();
		$("#div_wifiListDetails3").show();
		$("#div_wifiListDetails4").show();
	}
}

function wifidetails_setSelectedSSID(newSSID)
{
	if (newSSID == "Other...")
		newSSID = "";
	wifidetails_setItemValue("ssid", newSSID);
	wifidetails_setItemValue("password", "");
	wifidetails_setItemValue("security", "");
	wifidetails_highlightItem("ssid");
	keyboard_selectFirstKey();
	wifidetails_highlightItem("keyboard");
	wifidetails_showSecurity(true);
	
	if (newSSID == "")
		return;
	
	var oneWifiData = mNetConfig.getWifiData(newSSID);
	if (oneWifiData == null)
		return;
	
	var security = oneWifiData['auth']
	wifidetails_setItemValue("security", security);
	if (security == null || security.toUpperCase() == "NONE" || security == "OPEN" || security == "")
	{
		wifidetails_highlightItem("next");
		return;
	}
	
	wifidetails_highlightItem("password");
	keyboard_selectFirstKey();
	wifidetails_highlightItem("keyboard");
	wifidetails_showSecurity(false);
}

//-----------------------------------------------------------

function wifidetails_onRemoteControl(vButtonName)
{
	var newY = wifidetails_selectedIndex;
	if (newY == 0)
	{
		if (vButtonName == "up")			newY = 0;
		else if (vButtonName == "down")		newY++;
		else if (vButtonName == "center")
		{
			$("#wifi_ssid_wrapper").click();
		}
	}
	else if (newY == 1)
	{
		if (vButtonName == "up")			newY--;
		else if (vButtonName == "down")		newY += (wifidetails_showingSecurity == true) ? 1 : 4;
		else if (vButtonName == "center")
		{
			$("#wifi_password_wrapper").click();
		}
	}
	
	else if (newY == 2)
	{
		if (vButtonName == "up")			newY--;
		else if (vButtonName == "down")		newY++;
		else if (vButtonName == "center")
		{
			$("#wifi_security_none_wrapper").click();
		}
	}
	else if (newY == 3)
	{
		if (vButtonName == "up")			newY--;
		else if (vButtonName == "down")		newY++;
		else if (vButtonName == "center")
		{
			$("#wifi_security_wep_wrapper").click();
		}
	}
	else if (newY == 4)
	{
		if (vButtonName == "up")			newY--;
		else if (vButtonName == "down")		newY++;
		else if (vButtonName == "center")
		{
			$("#wifi_security_wpa_wrapper").click();
		}
	}
	
	else if (newY == 5)
	{
		var focusElement = $("#wifi_ssid").hasClass("input_focus") ? "wifi_ssid" : "wifi_password";
		var returnFocus = keyboard_onRemoteControl(vButtonName, focusElement);
		if (returnFocus)
		{
			if (vButtonName == "down")			newY++;
			else if (vButtonName == "up")		newY -= (wifidetails_showingSecurity == true) ? 1 : 4;
		}
	}
	else if (newY == 6)
	{
		if (vButtonName == "down")				newY++;
		else if (vButtonName == "up")			newY--;
		else if (vButtonName == "center")
		{
			$('.input_focus_error').removeClass('input_focus_error');
			selectedSSID = $("#wifi_ssid").val();
			var password = $("#wifi_password").val();
			var security = "";
			if ($("input:radio[name=wifi_security_radio]:checked") != null)
				if ($("input:radio[name=wifi_security_radio]:checked").val() != null)
					security = $("input:radio[name=wifi_security_radio]:checked").val().toUpperCase();
			
			//Validate input fields
			if (selectedSSID == "")
			{
				newY = 0;
				$("#wifi_ssid_wrapper").click();
				$("#wifi_ssid_wrapper").addClass("input_focus_error");
				keyboard_selectFirstKey();
				wifidetails_highlightItem("keyboard");
			}
			else if (password == "" && (security == "NONE" || security == "OPEN" || security == ""))
			{
				newY = 1;
				$("#wifi_password_wrapper").click();
				$("#wifi_password_wrapper").addClass("input_focus_error");
				keyboard_selectFirstKey();
				wifidetails_highlightItem("keyboard");
			}
			else
			{
				main_showState("configuring", true);
				mNetConfig.SetNetwork(selectedSSID, password, security);
			}
		}
	}
	else if (newY == 7)
	{
		if (vButtonName == "down")				newY = 7;
		else if (vButtonName == "up")			newY--;
		else if (vButtonName == "center")
		{
			main_showState("wifilist", true);
		}
	}
	
	//Highlight new item, avoid 5 (keyboard)
	$("#div_wifiListDetails"+wifidetails_selectedIndex).removeClass("div_wifiDetailsItem_selected").addClass("div_wifiDetailsItem_normal");
	if (newY != 5)
	{
		$("#div_wifiListDetails"+newY).removeClass("div_wifiDetailsItem_normal").addClass("div_wifiDetailsItem_selected");
	}
	else if (wifidetails_selectedIndex != 5)
	{
		var keyboardFocusElement = $("#wifi_ssid").hasClass("input_focus") ? "wifi_ssid" : "wifi_password";
		var returnFocus = keyboard_onRemoteControl(vButtonName, keyboardFocusElement);
	}
	wifidetails_selectedIndex = newY;
}
