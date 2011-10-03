var mNetConfig;
var main_x;
var main_y;
var leftMargin = 20;

function onLoad()
{
	onResize();

	//Save initial panel position
	main_y = $("#div_center").css('top').split('px')[0];
	main_x = $("#div_center").css('left').split('px')[0];
	$("#div_loadingMain").fadeIn(0);
	
	//Hide everything immediately, slide in later
	main_hideMainPanel();
	
	keyboard_init();
	
	//Init loading view
	main_showState('loading', false);
	
	mNetConfig = cNetConfig.fGetInstance();
	mNetConfig.fInit();

	setTimeout("onLoadLater()", 200);
	
	//support native keyboard events
	$(document).keydown(function(e)
	{
		if (e.keyCode == 37)		{		main_onRemoteControl('left');		return false;		}
		else if (e.keyCode == 39)	{		main_onRemoteControl('right');		return false;		}
		else if (e.keyCode == 38)	{		main_onRemoteControl('up');			return false;		}
		else if (e.keyCode == 40)	{		main_onRemoteControl('down');		return false;		}
		else if (e.keyCode == 13)	{		main_onRemoteControl('center');		return false;		}
		else if (e.keyCode == 33)	{		main_onRemoteControl('cpanel');		return false;		}
		else if (e.keyCode == 34)	{		main_onRemoteControl('widget');		return false;		}
		return true;
	});
	
	//support native mouse events
	$("#wifi_security_none_wrapper").click(onRadioFieldClick);
	$("#wifi_security_wep_wrapper").click(onRadioFieldClick);
	$("#wifi_security_wpa_wrapper").click(onRadioFieldClick);
	$("#wifi_ssid_wrapper").click(onInputFieldClick);
	$("#wifi_password_wrapper").click(onInputFieldClick);
}

function onLoadLater()
{
	//Start wifi list module
	wifilist_init();
	wifilist_startWifiScan();
	
	//Gracefully show the update panel
	main_showMainPanel(1650);
}

function onResize()
{
	var width = $("#div_center").css('width').split('px')[0];
	var height = $("#div_center").css('height').split('px')[0];
	var viewportwidth = $(window).width();
	var viewportheight = $(window).height();
	var left = viewportwidth - width - leftMargin;
	var top = (viewportheight - height) / 2;
	
	$("#div_center").css('top', top);
	$("#div_center").css('left', left);
}

//-----------------------------------------------------------

function main_showMainPanel(duration)
{
	$("#div_center").css('visibility', 'visible');
	
	if (!duration || duration == 0)		$("#div_center").css('left', main_x + 'px');
	else								$("#div_center").animate({ left: main_x }, !duration ? 1600 : duration);
}

function main_hideMainPanel(duration)
{
	if (!duration || duration == 0)		$("#div_center").css('left', $(window).width() + 'px');
	else								$("#div_center").animate({ left: $(window).width() }, !duration ? 1600 : duration);
}

//-----------------------------------------------------------

function GET( paramName )
{
	paramName = paramName.replace(/[\[]/,"\\\[").replace(/[\]]/,"\\\]");
	var regexS = "[\\?&]"+paramName+"=([^&#]*)";
	var regex = new RegExp( regexS );
	var results = regex.exec( window.location.href );
	if( results == null )    	return "";
	else					   	return results[1];
}

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
	else if (vStateName == "accountdetails")
	{
		//accountdetails_init();
		$("#div_loadingMain").fadeOut(duration);
		$("#div_wifiListMain").fadeOut(duration);
		$("#div_wifiDetailsMain").fadeOut(duration)
		setTimeout("$('#div_accountMain').fadeIn("+(duration+50)+");", duration);
		$("#div_configuringMain").fadeOut(duration);
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
	else if ( $("#div_configuringMain").is(":visible") )		configuring_onRemoteControl(vButtonName);
	//else if ( $("#div_accountMain").is(":visible") )			account_onRemoteControl(vButtonName);
}

function onRadioFieldClick()
{
	$('.radio_wrapper_selected').removeClass('radio_wrapper_selected');
	$(this).addClass('radio_wrapper_selected');
	$(':radio').attr('checked', false);
	$(this).children(':radio').attr('checked', true);
}

function onInputFieldClick()
{
	$(':input').blur();
	$(':input').removeClass('input_focus');
	$(this).children(':input').focus();
	$(this).children(':input').addClass('input_focus');
	$('.input_wrapper_focus').removeClass('input_wrapper_focus');
	$(this).addClass('input_wrapper_focus');
}