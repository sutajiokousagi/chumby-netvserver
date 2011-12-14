var serverUrl = getServerUrl();				//utils_general.js
var serverIP = getServerIPAddress();		//utils_general.js

var global_parameters = new Array();
global_parameters['log_hostinfo'] = true;
global_parameters['log_btn'] = false;
global_parameters['log_sharephoto_btn'] = true;
global_parameters['log_motor'] = true;

//LED demo
var led_demo_interval = 200;
var led_demo_timer_id = null;
var led_demo_timer_counter = 0;

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
	$( "#motor_channel_mode1" ).buttonset();
	$( "#motor_channel_mode2" ).buttonset();
	$( "#digital_outputs" ).buttonset();
	$( "#led_demo_mode" ).buttonset();
	$( "#digital_inputs" ).buttonset({ disabled: true });
	
	$( "div", "#analog_inputs" ).progressbar(50);
	
	$( "#sortable" ).sortable({
								delay: 100,
								placeholder: "ui-state-highlight"
								});
	$( ".portlet" ).addClass( "ui-widget ui-widget-content ui-helper-clearfix ui-corner-all" )
					.find( ".portlet-header" )
					.addClass( "ui-widget-header ui-corner-top" )
					.prepend( "<span class='ui-icon ui-icon-minusthick'></span>")
					.end()
					.find( ".portlet-content" );
	$( ".portlet-header .ui-icon" ).click(function() {
			$( this ).toggleClass( "ui-icon-minusthick" ).toggleClass( "ui-icon-plusthick" );
			$( this ).parents( ".portlet:first" ).find( ".portlet-content" ).toggle();
		});
	$( "#sortable" ).disableSelection();

	if (global_parameters['log_hostinfo'])
	{
		console.log(document.location);
		console.log("Server IP: " + serverIP);
		console.log("Server URL: " + serverUrl);
	}
		
	//Start by checking motor firmware presence after all other stuffs has been fully loaded
	$("#btn_enable_motor").button( "option", "disabled", true );
	$("#btn_enable_motor").button('option', 'icons', {primary:'ui-icon-transferthick-e-w'} );
	setTimeout("check_motor_firmware(on_firmware_version_changed)", 500);
}

function on_firmware_version_changed(fwver)
{
	if (fwver <= 0)
	{
		$("#btn_enable_motor").button('option', 'label', 'Enable motor firmware');
		$("#btn_enable_motor").button( "option", "disabled", false );
		$("#btn_enable_motor").button('option', 'icons', {primary:'ui-icon-circle-arrow-s'} );
		return;
	}
	
	$("#btn_enable_motor").button('option', 'label', 'Motor firmware enabled');
	$("#btn_enable_motor").button( "option", "disabled", true );
	$("#btn_enable_motor").button('option', 'icons', {primary:'ui-icon-circle-check'} );
	commit_digital_output_ui_state();
	start_digital_input_update(1000, set_digital_input_state_all);
	start_analog_input_update(500, set_analog_input_ui_state, graph_update_data);
	set_motor_mode(1, true);		//motor mode
	set_motor_mode(2, true);		//motor mode
}

//-------------------------------------------------------

function on_button_click(btnName)
{
	if (global_parameters['log_btn'])
		log( btnName );
	if (btnName == "btn_enable_motor") {
		enable_motor_board(on_firmware_version_changed);
		return;
	}
	else if (stringStartsWith(btnName, "dinput"))
	{
		return;
	}
	else if (stringStartsWith(btnName, "doutput"))
	{
		set_digital_output_all( get_digital_output_ui_state_all() );
	}
	else if (stringStartsWith(btnName, "led_demo_mode"))
	{
		var index = btnName.replace("led_demo_mode", "");
		if (index <= 0)	{
			stop_led_demo();
			return;
		}
		start_led_demo(index, 1000);
	}
	else if (stringStartsWith(btnName, "pwmfreq"))
	{
		var desired_freq = $("#"+btnName).val();
		if (desired_freq < 0)
			desired_freq = $("#slider_pwm1").slider( "option", "value" );
	
		set_motor_freqency(desired_freq);
		$("#slider_pwm1").slider( "option", "value", desired_freq );
		$("input", "#pwm_frequencies").button("refresh");
		if (desired_freq < 1000)	$("#pwm_title").html("Motor rate (PWM frequency) - " + desired_freq + "Hz");
		else						$("#pwm_title").html("Motor rate (PWM frequency) - " + (desired_freq/1000.0) + "kHz");
	}
	else if (stringStartsWith(btnName, "stop_motor"))
	{
		var index = btnName.replace("stop_motor", "");
		var isMotor = is_motor_mode(index);
		stop_motor(index);
		center_servo(index);
		if (isMotor)
		{
			$("#slider"+index).slider( "option", "value", 0 );
			$("#motor_title"+index).html("Motor "+index+ " - stopped");
		}
		else
		{
			$("#slider"+index).slider( "option", "value", 900 );
			$("#motor_title"+index).html("Servo "+index+ " - centered");
		}
	}
	else if (stringStartsWith(btnName, "servo_mode"))
	{
		var index = btnName.replace("servo_mode", "");
		if (index < 1 || index > 2)
			return;
		set_motor_mode(index,false);
		$("#slider"+index).slider({ min:0, max:1800, value:900, slide:on_motor_slider_slide });
		$("#stop_motor"+index).button('option', 'label', 'Center');
		$("#motor_title"+index).html("Servo "+index+ " - centered");
	}
	else if (stringStartsWith(btnName, "motor_mode"))
	{
		var index = btnName.replace("motor_mode", "");
		if (index < 1 || index > 2)
			return;
		set_motor_mode(index,true);
		$("#slider"+index).slider({ min:-255, max:255, value:0, slide:on_motor_slider_slide });
		$("#stop_motor"+index).button('option', 'label', 'Stop');
		$("#motor_title"+index).html("Motor "+index+ " - stopped");
	}
	else
	{
		console.log("Unhandled button '" + btnName + "'");
	}
}

function on_motor_slider_slide(event, ui)
{
	var id = $(this).attr("id");
	var index = id.replace("slider", "");
	var value = ui.value;
	var isMotor = is_motor_mode(index);

	if (isMotor)
	{
		set_motor_speed(index, value);
		if (value == 0)		$("#motor_title"+index).html("Motor "+index+ " - stopped");
		else if (value > 0)	$("#motor_title"+index).html("Motor "+index+ " - forward " + Math.abs(value));
		else				$("#motor_title"+index).html("Motor "+index+ " - reverse " + Math.abs(value));
	}
	else
	{
		value = value / 10.0;
		set_servo_angle(index, value);
		if (value > 0)	$("#motor_title"+index).html("Servo "+index+ " - " + value + " degree");
	}
}

function on_pwm_slider_slide(event, ui)
{
	var id = $(this).attr("id");
	var desired_freq = ui.value;
	set_motor_freqency(desired_freq);
	$("#pwmfreq15").click();
	if (desired_freq < 1000)	$("#pwm_title").html("Motor rate (PWM frequency) - " + desired_freq + "Hz");
	else						$("#pwm_title").html("Motor rate (PWM frequency) - " + (desired_freq/1000.0) + "kHz");
}

//-------------------------------------------------------

function get_digital_output_ui_state_all()
{
	var state = 0;
	for (i=0; i<8; i++)
		state += (get_digital_output_ui_state(i) == false) ? 0 : (1 << i);
	return state;
}

function get_digital_output_ui_state(index)
{
	return $("#doutput" + index).is(':checked');
}

function set_digital_output_ui_state(index, isOn)
{
	$("#doutput" + index).prop('checked', isOn);
	$("#doutput" + index).button('refresh');
}

function set_digital_output_ui_state_all(state)
{
	for (i=0; i<8; i++)
		set_digital_output_ui_state(i, state & (1 << i));
}

function commit_digital_output_ui_state()
{
	set_digital_output_all( get_digital_output_ui_state_all() );
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
	$("#dinput" + index).prop('checked', isOn);
	$("#dinput" + index).button('refresh');
}

function set_analog_input_ui_state(index, value)
{
	var percentage = 100.0 * value / 255.0;
	$("#adc" + index).progressbar( "option", "value", percentage );
}

//-------------------------------------------------------

function stop_led_demo()
{
	if (led_demo_timer_id == null)
		return;
	clearInterval( led_demo_timer_id );
	led_demo_timer_id = null;
	console.log("Stopping LED demo");
}

function start_led_demo(index, interval)
{
	if (index <= 0)
		return;
	if (!interval)
		led_demo_interval = interval;
	if (led_demo_timer_id)
		stop_led_demo();
	led_demo_timer_counter = 0;
	led_demo_timer_id = setInterval ( "led_demo_"+index+"()", led_demo_interval );
	console.log("Starting LED demo " + index);
}

function led_demo_1()
{
	set_digital_output_ui_state_all(led_demo_timer_counter);
	commit_digital_output_ui_state();
		
	led_demo_timer_counter++;
	if (led_demo_timer_counter > 15)
		led_demo_timer_counter = 0;
}

function led_demo_2()
{	
	set_digital_output_ui_state_all(led_demo_timer_counter);
	commit_digital_output_ui_state();

	if (led_demo_timer_counter == 0)	led_demo_timer_counter = 1;
	else								led_demo_timer_counter = led_demo_timer_counter * 2;
		
	if (led_demo_timer_counter > 8)
		led_demo_timer_counter = 0;
}