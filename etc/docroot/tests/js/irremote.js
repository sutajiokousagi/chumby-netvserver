var buttonPressedArray;
var buttonNameArray = ['cpanel', 'widget', 'up', 'left', 'center', 'right', 'down'];
var irremote_testIndex;

function irremote_init(testIndex)
{
	irremote_testIndex = testIndex;
	
	buttonPressedArray = new Array();
	for (var btnIndex in buttonNameArray)
		buttonPressedArray[buttonNameArray[btnIndex]] = false;
		
	buttonPressedArray['widget'] = false;
		
	fDbg2("Refresh IR Remote view");
	refreshIRRemote();
	
	if (irremote_testIndex == 0)
	{
		$("#div_irRemoteMain_instruction1").show();
		$("#div_irRemoteMain_instruction2").hide();
	}
	else
	{
		$("#div_irRemoteMain_instruction1").hide();
		$("#div_irRemoteMain_instruction2").show();
	}
	
}

//-----------------------------------------------------------

function refreshIRRemote()
{
	for (var btnName in buttonPressedArray)
	{
		var pressed = buttonPressedArray[btnName];
		if (pressed)		$('#div_irRemoteMain_' + btnName).removeClass("div_irRemoteMain_normal").addClass("div_irRemoteMain_selected");
		else				$('#div_irRemoteMain_' + btnName).removeClass("div_irRemoteMain_selected").addClass("div_irRemoteMain_normal");
	}
}

//-----------------------------------------------------------

function irremote_numButton()
{
	return buttonNameArray.length;
}

function irremote_onRemoteControl(vButtonName)
{
	//Ignore 'setup' button here
	if (vButtonName == "setup")
		return;
	
	//Set the flag & refresh whole page
	buttonPressedArray[vButtonName] = true;
	refreshIRRemote();
	
	//Count how many (unique) buttons have been pressed
	var btnCount = 0;
	for (var btnName in buttonPressedArray)
		if (buttonPressedArray[btnName] != null && buttonPressedArray[btnName] == true)
			btnCount++;
	
	if (btnCount < irremote_numButton())
		return;
	
	//Finish first test
	if (irremote_testIndex == 0)		irremote_init(1);
	else								main_showState("resetbtn");
}
