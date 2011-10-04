var main_y;

function onLoad()
{
	main_y = $("#div_center").offset().top;
	main_hideMainPanel(50);
	
	resetbtn_pressed = false;
	
	//Init first view
	irremote_init(0);
	main_showState('androidwait', false);

	setTimeout("onLoadLater()", 100);
}

function onLoadLater()
{
	main_showMainPanel();
	
	Hello();
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
	else if ( $("#div_androidTestInfo").is(":visible") )		return "androidwait";
	else if ( $("#div_androidTestMain").is(":visible") )		return "androidtest";
	else if ( $("#div_resetBtnMain").is(":visible") )			return "resetbtn";
	else if ( $("#div_allDoneMain").is(":visible") )			return "done";
}

function main_showState(vStateName, animated)
{
	var duration = (animated == true) ? 300 : 0;
	if (vStateName == "irremote")
	{
		irremote_init(0);
		$("#div_androidTestInfo").fadeOut(duration);
		$("#div_androidTestMain").fadeOut(duration);
		setTimeout("$('#div_irRemoteMain').fadeIn("+(duration+50)+");", duration);
		$("#div_resetBtnMain").fadeOut(duration);
		$("#div_allDoneMain").fadeOut(duration);
	}
	else if (vStateName == "androidwait")
	{
		setTimeout("$('#div_androidTestInfo').fadeIn("+(duration+50)+");", duration);
		$("#div_androidTestMain").fadeOut(duration);
		$("#div_irRemoteMain").fadeOut(duration);
		$("#div_resetBtnMain").fadeOut(duration);
		$("#div_allDoneMain").fadeOut(duration);
	}
	else if (vStateName == "androidtest")
	{
		$("#div_androidTestInfo").fadeOut(duration);
		setTimeout("$('#div_androidTestMain').fadeIn("+(duration+50)+");", duration);
		$("#div_irRemoteMain").fadeOut(duration);
		$("#div_resetBtnMain").fadeOut(duration);
		$("#div_allDoneMain").fadeOut(duration);
	}
	else if (vStateName == "resetbtn")
	{
		resetbtn_init(0);
		$("#div_androidTestInfo").fadeOut(duration);
		$("#div_androidTestMain").fadeOut(duration);
		$("#div_irRemoteMain").fadeOut(duration);
		setTimeout("$('#div_resetBtnMain').fadeIn("+(duration+50)+");", duration);
		$("#div_allDoneMain").fadeOut(duration);
	}
	else if (vStateName == "done")
	{
		$("#div_androidTestInfo").fadeOut(duration);
		$("#div_androidTestMain").fadeOut(duration);
		$("#div_irRemoteMain").fadeOut(duration);
		$("#div_resetBtnMain").fadeOut(duration);
		setTimeout("$('#div_allDoneMain').fadeIn("+(duration+50)+");", duration);
	}
	else
	{
		$("#div_androidTestInfo").fadeOut(duration);
		$("#div_androidTestMain").fadeOut(duration);
		$("#div_irRemoteMain").fadeOut(duration);
		$("#div_resetBtnMain").fadeOut(duration);
		$("#div_allDoneMain").fadeOut(duration);
	}
}

//-----------------------------------------------------------

function main_onRemoteControl(vButtonName)
{
	if ( $("#div_irRemoteMain").is(":visible") )				irremote_onRemoteControl(vButtonName);
	else if ( $("#div_resetBtnMain").is(":visible") )			resetbtn_onRemoteControl(vButtonName);
}
