var motor_firmware_enabled = false;
var motor_firmware_version = 0;
var motor_states = new Array();
var servo_states = new Array();
var doutput_state = 0;
var dinput_state = 0;
var dinput_interval = 500;
var dinput_timer_id = null;
var ainput_states = new Array();
var ainput_interval = 500;
var ainput_timer_id = null;
var ainput_data_history = new Array();

function check_motor_firmware()
{
	xmlhttpPost(serverUrl, "post", { cmd : 'NECommand', value : "/usr/bin/mot_ctl V"}, on_motor_firmware_version );
}

function enable_motor_board()
{
	xmlhttpPost(serverUrl, "post", { cmd : 'NECommand', value : "/etc/init.d/netv_service motor"}, on_motor_firmware );
}

function on_motor_firmware_version(vData)
{
	vData = clean_vData(vData);
	if (vData == "")
		return;
	/* Expecting a single number, or this
	FPGA is not configured to drive a motor board, please follow this procedure:
	1. Make sure the HDMI input is connected to the motor board. Do not connect an HDMI device to the port! We are not liable for damages if you wire this up wrong.
	2. Issue the command 'netv_service motor'; this will switch the NeTV into motor driver mode.
	*/
	if (vData.length < 5)
	{
		motor_firmware_enabled = true;
		motor_firmware_version = vData;
		console.log("Motor firmware version: " + vData);
		motor_stop_all();
	}
	else
	{
		motor_firmware_enabled = false;
		motor_firmware_version = 0;
		console.log("FPGA is not using motor firmware.");
	}
	on_firmware_version_changed(motor_firmware_version);
}

function on_motor_firmware(vData)
{
	vData = clean_vData(vData);
	if (vData == "")
		return;	
	/*
	Expecting this:
		Setting 0xd420b1a8: 0x90000001 -> 0x90000001 ok
		installing chumby xilinx driver
		configuring FPGA
		Note configuration: motor driver on input side, 720p video on output side
		Setting 0xd420b1c8: 0x00000000 -> 0x00000000 ok
		Setting 0xd420b1b8: 0x210ff003 -> 0x210ff003 ok
	But 2nd time execution, it gives this:
		Setting 0xd420b1a8: 0x90000001 -> 0x90000001 ok
		installing chumby xilinx driver
		configuring FPGA
		FPGA is configured to drive a motor board on the HDMI input, you can't use this utility for the current configuration.
		If you have no idea what this message is about, disconnect the device from the HDMI input NOW or else you may permanantly damage it, and reboot the board to get back to a typical consumer-use application configuration.
		Note configuration: motor driver on input side, 720p video on output side
		Setting 0xd420b1c8: 0x00000000 -> 0x00000000 ok
		Setting 0xd420b1b8: 0x210ff003 -> 0x210ff003 ok
	*/
	//Getting firmware version
	xmlhttpPost(serverUrl, "post", { cmd : 'NECommand', value : "/usr/bin/mot_ctl V"}, on_motor_firmware_version );
}

function clean_vData(vData)
{
	if (!vData)
		return "";
	if (!stringContains(vData, "<status>") || !stringContains(vData, "</status>"))
		return "";
	var status = vData.split("<status>")[1].split("</status>")[0];
	//if (status != "1")
	//	return "";
	if (!stringContains(vData, "<value>") || !stringContains(vData, "</value>"))
		return "";
	var value = vData.split("<value>")[1].split("</value>")[0];
	return value;
}

//--------------------------------------------------

function send_motor_noreply(command_string)
{
	if (command_string == "")
		return;
	if (motor_firmware_enabled == false)
		return;
	xmlhttpPost(serverUrl, "post", { cmd : 'Motor', value : command_string }, null );
}

//--------------------------------------------------

function motor_stop_all()
{
	motor_stop("1");
	motor_stop("2");
	motor_stop("3");
	motor_stop("4");
}

function motor_stop(index)
{
	motor_states[index] = 's';
	send_motor_noreply("m " + index + " s");
	if (global_parameters['log_motor'])
		console.log("Stopping motor " + index);
}

function motor_forward(index)
{
	motor_states[index] = 'f';
	send_motor_noreply("m " + index + " f");
	if (global_parameters['log_motor'])
		console.log("Forward motor " + index);
}

function motor_reverse(index)
{
	motor_states[index] = 'r';
	send_motor_noreply("m " + index + " r");
	if (global_parameters['log_motor'])
		console.log("Reverse motor " + index);
}

function set_motor_speed(index, value)
{
	if (value == 0) {
		motor_stop(index);
	}
	var prev_state = motor_states[index];
	if (value > 0 && prev_state != "f")			motor_forward(index);
	else if (value < 0 && prev_state != "r")	motor_reverse(index);
	send_motor_noreply("p " + index + " " + Math.abs(value));
}

//--------------------------------------------------

function set_digital_output_all(value)
{
	send_motor_noreply("u " + value);
}

function set_digital_output(index, isOn)
{
	var mask = 1 << index;
	var new_state = doutput_state;
	if (isOn)		new_state |= mask;
	else			new_state &= ~mask;
	
	doutput_state = new_state;	
	set_digital_output_all(doutput_state);
}

//--------------------------------------------------

function update_digital_input()
{
	xmlhttpPost(serverUrl, "post", { cmd : 'NECommand', value : "/usr/bin/mot_ctl i"}, on_digital_input_state );
}

function on_digital_input_state(vData)
{
	vData = clean_vData(vData);
	if (vData == "")
		return;
	//Expected: 0x20: xx
	if (!stringContains(vData, ": "))
		return;
	dinput_state = trim( vData.split(": ")[1] );
	
	//UI
	set_digital_input_state_all(dinput_state);
}

function start_dinput_update(interval)
{
	if (!interval)
		dinput_interval = interval;
	dinput_timer_id = setInterval ( "update_digital_input()", dinput_interval );
}

function stop_dinput_update()
{
	if (dinput_timer_id == null)
		return;
	clearInterval( dinput_timer_id );
	dinput_timer_id = null;
}

//--------------------------------------------------

function update_analog_inputs()
{
	for (i=0; i<8; i++)
		xmlhttpPost(serverUrl, "post", { cmd : 'NECommand', value : "/usr/bin/mot_ctl a " + i}, on_analog_input_state );
}

function on_analog_input_state(vData)
{
	vData = clean_vData(vData);
	if (vData == "")
		return;
	//Expected: 2: 0x2a
	if (!stringContains(vData, ": "))
		return;
	var channel = parseInt( vData.split(": ")[0] );
	var value = parseInt( vData.split(": ")[1] );
	ainput_states[channel] = value;
	
	//UI
	set_analog_input_state(channel, value);
	
	//Push data to history array
	if (!ainput_data_history[channel]) {
		ainput_data_history[channel] = [];
		for (i=0;i < 32; i++)
			ainput_data_history[channel].push(0);
	}
	ainput_data_history[channel].unshift(value);
	
	//Limit at 40 values
	if (ainput_data_history[channel].length > 40)
		ainput_data_history[channel].pop();
		
	//Update graph data
	if (channel < 7 || ainput_data_history[channel].length < 30)
		return;
		
	graph_update_data(ainput_data_history);
	graph_update_drawing();
}

function start_ainput_update(interval)
{
	if (!interval)
		ainput_interval = interval;
	ainput_timer_id = setInterval ( "update_analog_inputs()", ainput_interval );
}

function stop_ainput_update()
{
	if (ainput_timer_id == null)
		return;
	clearInterval( ainput_timer_id );
	ainput_timer_id = null;
}

//--------------------------------------------------

function set_motor_freqency(freq)
{
	//See mot_ctl command for formula
	var div = Math.round( (101498.0 / freq) - 2.0 );
	set_pwm_divider(div)
}

function set_pwm_divider(div)
{
	if (div < 0) 		div = 0;
	if (div > 65535)	div = 65535;
	send_motor_noreply("P " + div);
}

//--------------------------------------------------

function set_motor_mode(index, isMotor)
{
	if (index < 1 || index > 2)
		return;
	servo_states[index] = isMotor;
	if (isMotor)	send_motor_noreply("S " + index + " m");
	else			send_motor_noreply("S " + index + " s");
	motor_stop(index);
	servo_reset(index);
}

function is_motor_mode(index)
{
	return servo_states[index];
}

function set_servo_angle(index, value)
{
	if (is_motor_mode(index))
		return;
	if (motor_states[index] != 'f')
		motor_forward(index);
	send_motor_noreply("s " + index + " " + value);
}

function servo_reset(index)
{
	set_servo_angle(index, 90);
}