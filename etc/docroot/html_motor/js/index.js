var serverUrl = getServerUrl();				//utils_general.js
var serverIP = getServerIPAddress();		//utils_general.js

var global_parameters = new Array();
global_parameters['log_hostinfo'] = true;
global_parameters['log_btn'] = false;
global_parameters['log_sharephoto_btn'] = true;
global_parameters['log_motor'] = true;

function onLoad()
{	
	//Initialize jQueryUI
	$("button").button();
	$("button").click(function() { on_button_click( $(this).attr("name") ); });
	
	$("#slider1").slider({ min:-255, max:255, value:0, slide:on_slider_slide });
	$("#slider2").slider({ min:-255, max:255, value:0, slide:on_slider_slide });
	$("#slider3").slider({ min:-255, max:255, value:0, slide:on_slider_slide });
	$("#slider4").slider({ min:-255, max:255, value:0, slide:on_slider_slide });

	setTimeout("onLoadLater()", 500);
	check_motor_firmware();
}

function onLoadLater()
{
	if (global_parameters['log_hostinfo'])
	{
		console.log(document.location);
		console.log("Server IP: " + serverIP);
		console.log("Server URL: " + serverUrl);
	}
}

function on_firmware_version_changed(fwver)
{
	if (fwver > 0)
		$("#btn_enable_motor").hide();
}

//-------------------------------------------------------

function on_button_click(btnName)
{
	if (global_parameters['log_btn'])
		log( btnName );
	if (btnName == "btn_enable_motor") {
		enable_motor_board();
		return;
	}
	else if (btnName == "motor_stop1") {	motor_stop("1");	$("#slider"+btnName.replace("motor_stop","")).slider( "option", "value", 0 );	return;		}
	else if (btnName == "motor_stop2") {	motor_stop("2");	$("#slider"+btnName.replace("motor_stop","")).slider( "option", "value", 0 );	return;		}
	else if (btnName == "motor_stop3") {	motor_stop("3");	$("#slider"+btnName.replace("motor_stop","")).slider( "option", "value", 0 );	return;		}
	else if (btnName == "motor_stop4") {	motor_stop("4");	$("#slider"+btnName.replace("motor_stop","")).slider( "option", "value", 0 );	return;		}
}

function on_slider_slide(event, ui)
{
	var id = $(this).attr("id");
	var value = ui.value;
	motor_speed(id.replace("slider", ""), value);
}