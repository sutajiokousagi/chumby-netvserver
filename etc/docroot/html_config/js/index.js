var mNetConfig;
var main_y;

function onLoad()
{
	main_y = $("#div_center").offset().top;
	main_hideMainPanel(50);
	
	keyboard_init();
	
	//Init loading view
	main_showState('loading', false);
	
	mNetConfig = cNetConfig.fGetInstance();
	mNetConfig.fInit();

	setTimeout("onLoadLater()", 100);
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

//-----------------------------------------------------------

function main_currentState()
{
	if ( $("#div_configuringMain").is(":visible") )			return "configuring";
	else if ( $("#div_wifiDetailsMain").is(":visible") )	return "wifidetails";
	else if ( $("#div_wifiListMain").is(":visible") )		return "wifilist";
	else if ( $("#div_loadingMain").is(":visible") )		return "loading";
}

function main_showState(vStateName, animated)
{
	var duration = (animated == true) ? 600 : 0;
	if (vStateName == "loading")
	{
		$("#div_loadingMain").fadeIn(duration);
		$("#div_wifiListMain").fadeOut(duration);
		$("#div_wifiDetailsMain").fadeOut(duration);
		$("#div_configuringMain").fadeOut(duration);
	}
	else if (vStateName == "wifilist")
	{
		wifilist_resetWifiScanTimer();
		$("#div_loadingMain").fadeOut(duration);
		$("#div_wifiListMain").fadeIn(duration);
		$("#div_wifiDetailsMain").fadeOut(duration);
		$("#div_configuringMain").fadeOut(duration);
	}
	else if (vStateName == "wifidetails")
	{
		wifidetails_init();
		$("#div_loadingMain").fadeOut(duration);
		$("#div_wifiListMain").fadeOut(duration);
		$("#div_wifiDetailsMain").fadeIn(duration);
		$("#div_configuringMain").fadeOut(duration);
	}
	else if (vStateName == "configuring")
	{
		$("#div_loadingMain").fadeOut(duration);
		$("#div_wifiListMain").fadeOut(duration);
		$("#div_wifiDetailsMain").fadeOut(duration);
		$("#div_configuringMain").fadeIn(duration);
		configuring_init();
	}
	else
	{
		$("#div_loadingMain").fadeOut(duration);
		$("#div_wifiListMain").fadeOut(duration);
		$("#div_wifiDetailsMain").fadeOut(duration);
		$("#div_configuringMain").fadeOut(duration);
	}
}

//-----------------------------------------------------------

function main_onRemoteControl(vButtonName)
{
	if ( $("#div_wifiListMain").is(":visible") )				wifilist_onRemoteControl(vButtonName);
	else if ( $("#div_wifiDetailsMain").is(":visible") )		wifidetails_onRemoteControl(vButtonName);
	else if ( $("#div_configuringMain").is(":visible") )		configuring_onRemoteControl(vButtonName);
}
