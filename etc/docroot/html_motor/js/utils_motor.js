var motor_firmware_enabled = false;
var motor_firmware_version = 0;
var motor_states = new Array();
var servo_states = new Array();
var doutput_state = 0;
var dinput_state = 0;
var dinput_interval = 500;
var dinput_timer_id = null;
var ainput_states = new Array();
var ainput_interval = 100;
var ainput_num_history = 100;
var ainput_timer_id = null;
var ainput_data_history = new Array();
var bridgeUrl = getBridgeUrl();
var motor_firmware_callback = null;
var digital_input_ui_callback = null;
var analog_input_ui_callback = null;
var analog_input_graph_callback = null;

//--------------------------------------------------
// Initialization
//--------------------------------------------------

function check_motor_firmware(vCompleteFunc)
{
	motor_firmware_callback = vCompleteFunc;
	xmlHttpPostMotor(bridgeUrl, "post", { cmd : 'NECommand', value : "/usr/bin/mot_ctl V"}, on_motor_firmware_version );
}

function enable_motor_board(vCompleteFunc)
{
	motor_firmware_callback = vCompleteFunc;
	xmlHttpPostMotor(bridgeUrl, "post", { cmd : 'NECommand', value : "/etc/init.d/netv_service motor"}, on_motor_firmware );
}

function on_motor_firmware_version(vData)
{
	vData = clean_vData(vData);
	if (vData == "")
		return;
	/* Expecting a single number, or this long string
	FPGA is not configured to drive a motor board, please follow this procedure:
	1. Make sure the HDMI input is connected to the motor board. Do not connect an HDMI device to the port! We are not liable for damages if you wire this up wrong.
	2. Issue the command 'netv_service motor'; this will switch the NeTV into motor driver mode.
	*/
	if (vData.length < 5)
	{
		motor_firmware_enabled = true;
		motor_firmware_version = vData;
		console.log("Motor firmware version: " + vData);
		stop_motor_all();
	}
	else
	{
		motor_firmware_enabled = false;
		motor_firmware_version = 0;
		console.log("FPGA is not using motor firmware.");
	}
	
	//Returns 0 if failed to enable motor firmware
	if (motor_firmware_callback)
		motor_firmware_callback(motor_firmware_version);
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
	xmlHttpPostMotor(bridgeUrl, "post", { cmd : 'NECommand', value : "/usr/bin/mot_ctl V"}, on_motor_firmware_version );
}

//--------------------------------------------------
// Motors mode
//--------------------------------------------------

function stop_motor_all()
{
	stop_motor("1");
	stop_motor("2");
	stop_motor("3");
	stop_motor("4");
}

function stop_motor(index)
{
	motor_states[index] = 's';
	send_motor_noreply("m " + index + " s");
	if (global_parameters['log_motor'])
		console.log("Stopping motor " + index);
}

function set_motor_forward(index)
{
	motor_states[index] = 'f';
	send_motor_noreply("m " + index + " f");
	if (global_parameters['log_motor'])
		console.log("Forward motor " + index);
}

function set_motor_reverse(index)
{
	motor_states[index] = 'r';
	send_motor_noreply("m " + index + " r");
	if (global_parameters['log_motor'])
		console.log("Reverse motor " + index);
}

function set_motor_speed(index, value)
{
	if (value == 0) {
		stop_motor(index);
	}
	var prev_state = motor_states[index];
	if (value > 0 && prev_state != "f")			set_motor_forward(index);
	else if (value < 0 && prev_state != "r")	set_motor_reverse(index);
	send_motor_noreply("p " + index + " " + Math.abs(value));
}

//--------------------------------------------------
// Motor settings
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

function is_motor_mode(index)
{
	if (index < 1 || index > 2)
		return true;
	return servo_states[index];
}

function set_motor_mode(index, isMotor)
{
	if (index < 1 || index > 2)
		return;
	servo_states[index] = isMotor;
	if (isMotor)	send_motor_noreply("S " + index + " m");
	else			send_motor_noreply("S " + index + " s");
	stop_motor(index);
	center_servo(index);
}

//--------------------------------------------------
// Servo mode
//--------------------------------------------------

function set_servo_angle(index, value)
{
	if (is_motor_mode(index))
		return;
	if (motor_states[index] != 'f')
		set_motor_forward(index);
	send_motor_noreply("s " + index + " " + value);
}

function center_servo(index)
{
	set_servo_angle(index, 90);
}

//--------------------------------------------------
// Digital outputs
//--------------------------------------------------

//Set all 8 bits of digital output port to 'value' in decimal (0-255)
function set_digital_output_all(value)
{
	send_motor_noreply("u " + value);
}

//Set 1 single digital output bit to on/off
function set_digital_output_single(index, isOn)
{
	if (isOn)	send_motor_noreply("u " + index + " 1");
	else		send_motor_noreply("u " + index + " 0");
}

//Update internal digital output flags & send them to motor board
function set_digital_output_flag(index, isOn)
{
	var mask = 1 << index;
	var new_state = doutput_state;
	if (isOn)		new_state |= mask;
	else			new_state &= ~mask;
	
	doutput_state = new_state;	
	set_digital_output_all(doutput_state);
}

//--------------------------------------------------
// Digital inputs
//--------------------------------------------------

function start_digital_input_update(interval, vCompleteFunc)
{
	if (!interval)
		dinput_interval = interval;
	digital_input_ui_callback = vCompleteFunc;
	dinput_timer_id = setInterval ( "request_digital_input_state()", dinput_interval );
}

function stop_digital_input_update()
{
	if (dinput_timer_id == null)
		return;
	clearInterval( dinput_timer_id );
	dinput_timer_id = null;
}

function request_digital_input_state()
{
	xmlHttpPostMotor(bridgeUrl, "post", { cmd : 'NECommand', value : "/usr/bin/mot_ctl i"}, on_digital_input_state );
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
	
	//UI callback
	if (digital_input_ui_callback)
		digital_input_ui_callback( parseInt("0x"+dinput_state) );
}

//--------------------------------------------------
// Analog inputs
//--------------------------------------------------

function start_analog_input_update(interval, vCompleteFunc, vGraphFunc)
{
	if (!interval)
		ainput_interval = interval;
	analog_input_ui_callback = vCompleteFunc;
	analog_input_graph_callback = vGraphFunc;
	ainput_timer_id = setInterval ( "request_analog_input_state()", ainput_interval );
}

function stop_analog_input_update()
{
	if (ainput_timer_id == null)
		return;
	clearInterval( ainput_timer_id );
	ainput_timer_id = null;
}

function request_analog_input_state()
{
	xmlHttpPostMotor(bridgeUrl, "post", { cmd : 'NECommand', value : "/usr/bin/mot_ctl a" }, on_analog_input_state );
}

function on_analog_input_state(vData)
{
	vData = clean_vData(vData);
	if (vData == "")
		return;
	//Expected: 0xe0 0xa0 0x60 0x20 0xc1 0x00 0xa0 0xe0
	if (!stringContains(vData, " ") || !stringContains(vData, "0x"))
		return;
	var valuesArray = vData.split(" ");
	if (!valuesArray)
		return;

	for (channel=0;channel < valuesArray.length; channel++)
	{
		ainput_states[channel] = parseInt( valuesArray[channel] );

		//If not created history array
		if (!ainput_data_history[channel]) {
			ainput_data_history[channel] = [];
			for (i=0;i < ainput_num_history; i++)
				ainput_data_history[channel].push(0);
		}
		
		//Push data to history array
		ainput_data_history[channel].unshift( ainput_states[channel] );
		
		//Limit at 70 values
		if (ainput_data_history[channel].length > ainput_num_history)
			ainput_data_history[channel].pop();
	}
			
	//Update history data
	if (!ainput_data_history[0] || ainput_data_history[0].length < ainput_num_history)
		return;
	
	//UI
	if (analog_input_graph_callback)
		analog_input_graph_callback(ainput_data_history);
}

//----------------------------------------------------------------
// XMLHttpRequest helpers
//----------------------------------------------------------------

function send_motor_noreply(command_string)
{
	if (command_string == "")
		return;
	if (motor_firmware_enabled == false)
		return;
	xmlHttpPostMotor(bridgeUrl, "post", { cmd : 'Motor', value : command_string }, null );
}

function getBridgeUrl()
{
	var tempLocaltion = "" + document.location;
	if (tempLocaltion.indexOf("localhost") != -1)
		return "localhost";
	if (tempLocaltion.indexOf("file://") != -1)
		return "";
	return "http://" + document.location.host + "/bridge";
}

//Get an XMLHttpRequest, object
function getXHR()
{
	var req = null;
	if(window.XMLHttpRequest)
	{
		req = new XMLHttpRequest();
		return req;
	}
	else
	{
		try {
			req = new ActiveXObject('Msxml2.XMLHTTP');				//IE
			return req;
		}
		catch(e)
		{
			try {
				req = new ActiveXObject("Microsoft.XMLHTTP");		//IE
				return req;
			}
			catch(e)
			{
				return null;
			}
		}
	}
	return req;
}

function xmlHttpPostMotor(
	vUrl,
	vType,			// "post" | "get"
	vData,
	vCompleteFunc
)
{		
	var xmlHttpReq = getXHR();
	if (xmlHttpReq == null) {
		alert("Failed to get XMLHttpRequest");
		return;
	}
	
	if (vUrl == "")
		vUrl = getBridgeUrl();
	xmlHttpReq.open(vType, vUrl, true);
	xmlHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
	xmlHttpReq.onreadystatechange = function()
	{
		if (xmlHttpReq.readyState == 4)
		{
			switch (xmlHttpReq.status)
			{
				case 200:
					if (vCompleteFunc)
						vCompleteFunc(xmlHttpReq.responseText);
					break;
				case 0:
					if (vCompleteFunc)
						vCompleteFunc(0);
					break;
			}
		}
	}
	
	if (vType.toLowerCase() == "post")
	{
		var parameters = "";
		for (var o in vData)
			parameters += o + "=" + encodeURIComponent(vData[o]) + "&";
		parameters = parameters.substring(0, parameters.length-1);
			
		xmlHttpReq.send(parameters);
	}
	else
	{
		xmlHttpReq.send();
	}
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