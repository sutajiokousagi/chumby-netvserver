var activationState;
var activationTimer;
var activationTime;

function configuring_init()
{
	wifilist_stopWifiScanTimer();
	
	activationState = '';
	activationTime = null;
	activationTimer = null;
	mNetConfig.helloCallback = configuring_helloCallback;
	
	$("#div_activationStatus").html("");
	$("#div_activationStatus").removeClass('div_activationStatus_success').removeClass('div_activationStatus_error');
	startActivation();
}

//-----------------------------------------------------------

function startActivation()
{
	activationTime = new Date();
	activationState = 'waiting';
	resetActivationTimer();
}

function stopActivation()
{
	activationState = '';
	if (clearTimeout != null)
		clearTimeout(activationTimer);
}

function resetActivationTimer(duration)
{
	if (clearTimeout != null)
		clearTimeout(activationTimer);
	activationTimer = setTimeout("configuring_onActivationTimer()", !duration ? 5000 : duration);
}

//-----------------------------------------------------------

function configuring_onRemoteControl(vButtonName)
{
	//Accept no buttons
	return;
}

function configuring_onActivationTimer()
{
	//not performing activation
	if (activationState == '') {
		stopActivation();
		return;
	}
		
	resetActivationTimer();
	if (activationState == "waiting")
		mNetConfig.Hello();
}

function configuring_onNetworkManagerState(vEventName)
{
	//not performing activation, don't care
	if (activationState == '')
		return;
		
	if (activationState == "waiting" && vEventName == "connected")
		mNetConfig.Hello();
}

function configuring_helloCallback(helloData)
{
	//Not performing activation
	if (activationState == '' || helloData == null)
		return;
	
	var internet = helloData['internet'];
	var ip = helloData['ip'];
	var secondsSinceActivation = ((new Date()).getTime() - activationTime.getTime()) / 1000;

	//Taking too long
	if (secondsSinceActivation > 50)
	{
		stopActivation();
		$("#div_activationStatus").removeClass('div_activationStatus_success').addClass('div_activationStatus_error');
		$("#div_activationStatus").html("<br>Error connecting to <br>" + selectedSSID + " network.<br>Please try again.");
		//Revert to ap_mode.
		mNetConfig.StartAP();
		
		//Check error. How?
		
		//Back to wifi list
		setTimeout("main_showState('wifilist', true);", 3200);
		return;
	}
	
	//Failed to connect or trying connecting
	if (ip == '')
	{
		if (secondsSinceActivation > 35)			$("#div_activationStatus").html("Be patient...");
		else if (secondsSinceActivation > 25)		$("#div_activationStatus").html("Hold on...");
		else if (secondsSinceActivation > 15)		$("#div_activationStatus").html("Keep waiting...");
		return;
	}
	
	//Got a valid IP (Note: We should check that we really have Internet here)
	if (activationState == "waiting")
	{
		if (internet != 'true' && internet != true)
			return;
			
		$("#div_activationStatus").removeClass('div_activationStatus_error').addClass('div_activationStatus_success');
		$("#div_activationStatus").html("<br>Network successfully configured!<br><strong>" + ip + "</strong>");
		stopActivation();
		
		setTimeout("doneConfiguring();", 5000);
		setTimeout("main_hideMainPanel();", 3200);
	}
}

//-----------------------------------------------------------

function doneConfiguring()
{
	var tempLocaltion = "" + document.location;
	if (tempLocaltion.indexOf("localhost") != -1)
		location.href = "http://localhost/";
	else 
		location.href = "http://" + document.location.host + "/";
}
