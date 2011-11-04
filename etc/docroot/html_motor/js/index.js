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
	$("button").click(function() { on_button_click( $(this).attr("id") ); });
	$("input").click(function() { on_button_click( $(this).attr("id") ); });
	
	$("#slider_pwm1").slider({ min:1, max:101498, value:101498, slide:on_pwm_slider_slide });
	$("#slider1").slider({ min:-255, max:255, value:0, slide:on_motor_slider_slide });
	$("#slider2").slider({ min:-255, max:255, value:0, slide:on_motor_slider_slide });
	$("#slider3").slider({ min:-255, max:255, value:0, slide:on_motor_slider_slide });
	$("#slider4").slider({ min:-255, max:255, value:0, slide:on_motor_slider_slide });
	
	$( "#pwm_frequencies" ).buttonset();
	$( "#digital_outputs" ).buttonset();
	$( "#digital_inputs" ).buttonset({ disabled: true });
	
	$( "div", "#analog_inputs" ).progressbar(50);

	if (global_parameters['log_hostinfo'])
	{
		console.log(document.location);
		console.log("Server IP: " + serverIP);
		console.log("Server URL: " + serverUrl);
	}
	
	setTimeout("check_motor_firmware()", 800);
}

function on_firmware_version_changed(fwver)
{
	if (fwver <= 0)
		return;
	
	$("#btn_enable_motor").hide();
	commit_digital_output_state();
	start_dinput_update(1000);
	start_ainput_update(500);
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
	else if (stringStartsWith(btnName, "dinput"))				return;
	else if (stringStartsWith(btnName, "doutput"))
	{
		set_digital_output_all( get_digital_output_state_all() );
	}
	else if (stringStartsWith(btnName, "pwmfreq"))
	{
		var desired_freq = $("#"+btnName).val();
		set_motor_freqency(desired_freq);
		$("#slider_pwm1").slider( "option", "value", desired_freq );
		$("input", "#pwm_frequencies").button("refresh");
	}
	else
	{
		console.log("Unhandled button '" + btnName + "'");
	}
}

function on_motor_slider_slide(event, ui)
{
	var id = $(this).attr("id");
	var value = ui.value;
	motor_speed(id.replace("slider", ""), value);
}

function on_pwm_slider_slide(event, ui)
{
	var id = $(this).attr("id");
	var desired_freq = ui.value;
	set_motor_freqency(desired_freq);
	console.log(desired_freq);
}

//-------------------------------------------------------

function get_digital_output_state_all()
{
	var state = 0;
	for (i=0; i<8; i++)
		state += (get_digital_output_state(i) == false) ? 0 : (1 << i);
	return state;
}

function get_digital_output_state(index)
{
	return $("#doutput" + index).is(':checked');
}

function commit_digital_output_state()
{
	set_digital_output_all( get_digital_output_state_all() );
}

function set_digital_input_state_all(value)
{
	for (i=0; i<8; i++)
	{
		var isOn = (value & (1 << i)) > 0 ? true : false;
		set_digital_input_state(i, isOn);
	}
}

function set_digital_input_state(index, isOn)
{
	$("#dinput" + index).prop('checked', isOn) ;
}

function set_analog_input_state(index, value)
{
	var percentage = 100.0 * value / 255.0;
	$("#adc" + index).progressbar( "option", "value", percentage );
}