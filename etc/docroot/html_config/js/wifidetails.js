var wifidetails_selectedIndex;

function wifidetails_init()
{
	$("#wifi_password").val('');
	$("#wifi_ssid").val('');
	
	//Clear previous selection
	wifidetails_selectedIndex = 0;
	for (var i=0; i<5; i++)
		$("#div_wifiListDetails"+i).removeClass("div_wifiDetailsItem_selected");
}

function wifidetails_highlightItem(itemName)
{
	switch (itemName)
	{
		case "ssid":		wifidetails_selectedIndex = 0;		break;
		case "password":	wifidetails_selectedIndex = 1;		break;
		case "security":	wifidetails_selectedIndex = 2;		break;
		case "next":		wifidetails_selectedIndex = 3;		break;
		case "back":		wifidetails_selectedIndex = 4;		break;
		default:			wifidetails_selectedIndex = 0;		break;
	}
	//Bring it in focus
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
	}
	return oldValue;
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
			$("#wifi_password").blur();
			$("#wifi_password").removeClass("input_focus");
			$("#wifi_password_wrapper").removeClass("input_wrapper_focus").addClass("input_focus");
			$("#wifi_ssid").focus();
			$("#wifi_ssid").addClass("input_focus");
			$("#wifi_ssid_wrapper").removeClass("input_focus").addClass("input_wrapper_focus");
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
			$("#wifi_ssid_wrapper").removeClass("input_wrapper_focus").addClass("input_focus");
			$("#wifi_password").focus();
			$("#wifi_password").addClass("input_focus");
			$("#wifi_password_wrapper").removeClass("input_focus").addClass("input_wrapper_focus");
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
		if (vButtonName == "down")				newY++;
		else if (vButtonName == "up")			newY--;
		else if (vButtonName == "center")
		{
			selectedSSID = $("#wifi_ssid").val();
			var password = $("#wifi_password").val();
			
			//Validate input fields
			if (selectedSSID == "")
			{
				//wifidetails_highlightItem("ssid");
				newY = 0;
				$("#wifi_ssid").addClass("input_focus_error");
				$("#wifi_ssid_wrapper").addClass("input_focus_error");
			}
			else if (password == "")
			{
				//wifidetails_highlightItem("password");
				newY = 1;
				$("#wifi_password").addClass("input_focus_error");
				$("#wifi_password_wrapper").addClass("input_focus_error");
			}
			else
			{
				main_showState("configuring", true);
				mNetConfig.SetNetwork(selectedSSID, password);
			}
		}
	}
	else if (newY == 4)
	{
		if (vButtonName == "down")				newY = 4;
		else if (vButtonName == "up")			newY--;
		else if (vButtonName == "center")
		{
			main_showState("wifilist", true);
		}
	}
	
	//Highlight new item, avoid 2
	$("#div_wifiListDetails"+wifidetails_selectedIndex).removeClass("div_wifiDetailsItem_selected").addClass("div_wifiDetailsItem_normal");
	if (newY != 2)
	{
		$("#div_wifiListDetails"+newY).removeClass("div_wifiDetailsItem_normal").addClass("div_wifiDetailsItem_selected");
	}
	else if (wifidetails_selectedIndex != 2)
	{
		var focusElement = $("#wifi_ssid").hasClass("input_focus") ? "wifi_ssid" : "wifi_password";
		var returnFocus = keyboard_onRemoteControl(vButtonName, focusElement);
	}
	wifidetails_selectedIndex = newY;
}
