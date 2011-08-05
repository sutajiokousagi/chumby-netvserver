var mNetConfig;
var main_y;

function onLoad()
{
	main_y = $("#div_center").offset().top;
	main_hideMainPanel(100);
	
	keyboard_init();
	
	//Init loading view
	main_showState('loading', false);
	
	mNetConfig = cNetConfig.fGetInstance();
	mNetConfig.fInit();

	setTimeout("onLoadLater()", 200);
}

function onLoadLater()
{
	//Start wifi list module
	wifilist_init();
	
	main_showMainPanel();
}

//-----------------------------------------------------------

function main_showMainPanel(duration)
{
	$("#div_center").css('visibility', 'visible');
	$("#div_center").animate({ top: main_y }, !duration ? 1600 : duration);
}

function main_hideMainPanel(duration)
{
	$("#div_center").animate({ top: main_y + $(window).height() }, !duration ? 1600 : duration);
}

/*
function slideToLeft(itemName, duration)
{
	var width = $("#div_center_container").width();
	$("#"+itemName).animate({ left: -width * 1.5 }, !duration ? 800 : duration);
}
function slideToRight(itemName, duration)
{
	var width = $("#div_center_container").width();
	$("#"+itemName).animate({ left: width * 1.5 }, !duration ? 800 : duration);
}
function slideInFromLeft(itemName, duration)
{
	var width = $("#div_center_container").width();
	var offset = $("#"+itemName).offset();
	offset.left = -width*1.5;
	$("#"+itemName).offset(offset);
	$("#"+itemName).animate({ left: 0 }, !duration ? 800 : duration);
}
function slideInFromRight(itemName, duration)
{
	var width = $("#div_center_container").width();
	var offset = $("#"+itemName).offset();
	offset.left = width*1.5;
	$("#"+itemName).offset(offset);
	$("#"+itemName).animate({ left: 0 }, !duration ? 800 : duration);
}
*/

//-----------------------------------------------------------

function main_currentState()
{
	if ( $("#div_configuringMain").is(":visible") )			return "configuring";
	else if ( $("#div_accountMain").is(":visible") )		return "account";
	else if ( $("#div_wifiDetailsMain").is(":visible") )	return "wifidetails";
	else if ( $("#div_wifiListMain").is(":visible") )		return "wifilist";
	else if ( $("#div_loadingMain").is(":visible") )		return "loading";
}

function main_showState(vStateName, animated)
{
	var duration = (animated == true) ? 300 : 0;
	if (vStateName == "loading")
	{
		$("#div_wifiListMain").fadeOut(duration);
		$("#div_wifiDetailsMain").fadeOut(duration);
		$("#div_accountMain").fadeOut(duration);
		$("#div_configuringMain").fadeOut(duration);
		setTimeout("$('#div_loadingMain').fadeIn("+(duration+50)+");", duration);
	}
	else if (vStateName == "wifilist")
	{
		wifilist_resetWifiScanTimer();
		$("#div_loadingMain").fadeOut(duration);
		setTimeout("$('#div_wifiListMain').fadeIn("+(duration+50)+");", duration);
		$("#div_wifiDetailsMain").fadeOut(duration);
		$("#div_accountMain").fadeOut(duration);
		$("#div_configuringMain").fadeOut(duration);
	}
	else if (vStateName == "wifidetails")
	{
		wifidetails_init();
		$("#div_loadingMain").fadeOut(duration);
		$("#div_wifiListMain").fadeOut(duration);
		setTimeout("$('#div_wifiDetailsMain').fadeIn("+(duration+50)+");", duration);
		$("#div_accountMain").fadeOut(duration);
		$("#div_configuringMain").fadeOut(duration);
	}
	else if (vStateName == "configuring")
	{
		configuring_init();
		$("#div_loadingMain").fadeOut(duration);
		$("#div_wifiListMain").fadeOut(duration);
		$("#div_wifiDetailsMain").fadeOut(duration);
		$("#div_accountMain").fadeOut(duration);
		setTimeout("$('#div_configuringMain').fadeIn("+(duration+50)+");", duration);
	}
	else
	{
		$("#div_loadingMain").fadeOut(duration);
		$("#div_wifiListMain").fadeOut(duration);
		$("#div_wifiDetailsMain").fadeOut(duration);
		$("#div_accountMain").fadeOut(duration);
		$("#div_configuringMain").fadeOut(duration);
	}
}

//-----------------------------------------------------------

function main_onRemoteControl(vButtonName)
{
	if ( $("#div_wifiListMain").is(":visible") )				wifilist_onRemoteControl(vButtonName);
	else if ( $("#div_wifiDetailsMain").is(":visible") )		wifidetails_onRemoteControl(vButtonName);
	//else if ( $("#div_accountMain").is(":visible") )			account_onRemoteControl(vButtonName);
	else if ( $("#div_configuringMain").is(":visible") )		configuring_onRemoteControl(vButtonName);
}