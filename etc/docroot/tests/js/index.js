var main_y;

function onLoad()
{
	main_y = $("#div_center").offset().top;
	main_hideMainPanel(50);
	
	//Init first view
	irremote_init();
	main_showState('irremote', false);

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
	if ( $("#div_androidDownloadMain").is(":visible") )		return "androiddownload";
	else if ( $("#div_irRemoteMain").is(":visible") )		return "irremote";
	else if ( $("#div_loadingMain").is(":visible") )		return "loading";
}

function main_showState(vStateName, animated)
{
	var duration = (animated == true) ? 300 : 0;
	if (vStateName == "irremote")
	{
		irremote_init();
		setTimeout("$('#div_irRemoteMain').fadeIn("+(duration+50)+");", duration);
		$("#div_androidDownloadMain").fadeOut(duration);
		//$("#div_accountMain").fadeOut(duration);
		//$("#div_configuringMain").fadeOut(duration);
	}
	if (vStateName == "androiddownload")
	{
		$("#div_irRemoteMain").fadeOut(duration);
		setTimeout("$('#div_androidDownloadMain').fadeIn("+(duration+50)+");", duration);
		//$("#div_accountMain").fadeOut(duration);
		//$("#div_configuringMain").fadeOut(duration);
	}
	else
	{
		$("#div_irRemoteMain").fadeOut(duration);
		$("#div_androidDownloadMain").fadeOut(duration);
		//$("#div_accountMain").fadeOut(duration);
		//$("#div_configuringMain").fadeOut(duration);
	}
}

//-----------------------------------------------------------

function main_onRemoteControl(vButtonName)
{
	if ( $("#div_irRemoteMain").is(":visible") )				irremote_onRemoteControl(vButtonName);
	//else if ( $("#div_androidDownloadMain").is(":visible") )	androiddownload_onRemoteControl(vButtonName);
}
