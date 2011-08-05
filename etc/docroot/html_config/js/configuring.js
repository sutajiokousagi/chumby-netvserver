var activationState;
var activationTimer;
var activationTime;

function configuring_init()
{
	activationState = '';
	activationTime = null;
	activationTimer = null;
	mNetConfig.helloCallback = configuring_helloCallback;
		
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

function configuring_helloCallback(helloData)
{
	//Not performing activation
	if (activationState == '' || helloData == null)
		return;
	
	var internet = helloData['internet'];
	var ip = helloData['ip'];
	var secondsSinceActivation = ((new Date()).getTime() - activationTime.getTime())/1000;

	//Taking too long
	if (secondsSinceActivation > 60)
	{
		stopActivation();
		$("#div_activationStatus").append("<br>Taking too long. Give up!");
		//Revert to ap_mode.
		mNetConfig.StartAP();
		
		//Check error. How?
		
		//Back to wifi list
		main_showState("wifilist", true);
		return;
	}
	
	//Fail to connect or connecting?
	if (ip == '')
	{
		//$("#div_activationStatus").append("<br>still waiting...");
		return;
	}
	
	//Got a valid IP
	if (activationState == "waiting")
	{
		if (internet != 'true' && internet != true)
			return;
			
		$("#div_activationStatus").append("<br>" + ip);
		$("#div_activationStatus").append("<br>Network configured!");
		stopActivation();
		
		setTimeout("doneConfiguring()", 5000);
		setTimeout("main_hideMainPanel()", 3200);
	}
}

//-----------------------------------------------------------

function doneConfiguring()
{
	location.href="http://localhost/";
}