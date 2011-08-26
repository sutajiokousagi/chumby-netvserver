var main_y;

function onLoad()
{
	main_y = $("#div_center").offset().top;
	main_hideMainPanel(50);
	
	//Init first view
	irremote_init();
	main_showState('androidtest', false);

	setTimeout("onLoadLater()", 100);
}

function onLoadLater()
{
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
	if ( $("#div_irRemoteMain").is(":visible") )				return "irremote";
	else if ( $("#div_androidTestMain").is(":visible") )		return "androidtest";
	else if ( $("#div_resetBtnMain").is(":visible") )			return "resetbtn";
}

function main_showState(vStateName, animated)
{
	var duration = (animated == true) ? 300 : 0;
	if (vStateName == "irremote")
	{
		irremote_init(0);
		$("#div_androidTestMain").fadeOut(duration);
		setTimeout("$('#div_irRemoteMain').fadeIn("+(duration+50)+");", duration);
		$("#div_resetBtnMain").fadeOut(duration);
	}
	if (vStateName == "androidtest")
	{
		setTimeout("$('#div_androidTestMain').fadeIn("+(duration+50)+");", duration);
		$("#div_irRemoteMain").fadeOut(duration);
		$("#div_resetBtnMain").fadeOut(duration);
	}
	if (vStateName == "resetbtn")
	{
		setTimeout("$('#div_androidTestMain').fadeIn("+(duration+50)+");", duration);
		$("#div_irRemoteMain").fadeOut(duration);
		$("#div_resetBtnMain").fadeOut(duration);
	}
	else
	{
		$("#div_androidTestMain").fadeOut(duration);
		$("#div_irRemoteMain").fadeOut(duration);
		$("#div_resetBtnMain").fadeOut(duration);
	}
}

//-----------------------------------------------------------

function main_onRemoteControl(vButtonName)
{
	if ( $("#div_irRemoteMain").is(":visible") )				irremote_onRemoteControl(vButtonName);
	//else if ( $("#div_androidTestMain").is(":visible") )		androidtest_onRemoteControl(vButtonName);
}
