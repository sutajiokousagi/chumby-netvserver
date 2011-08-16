var buttonPressedArray;
var buttonNameArray = ['cpanel', 'widget', 'up', 'left', 'center', 'right', 'down'];

function irremote_init()
{
	buttonPressedArray = new Array();
	for (var btnIndex in buttonNameArray)
		buttonPressedArray[buttonNameArray[btnIndex]] = false;
		
	buttonPressedArray['widget'] = false;
		
	fDbg2("Refresh IR Remote view");
	refreshIRRemote();
}

//-----------------------------------------------------------

function refreshIRRemote()
{
	fDbg2("Refresh IR Remote view");
	
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
	//Set the flag & refresh whole page
	buttonPressedArray[vButtonName] = true;
	refreshIRRemote(false);
	
	//Count how many (unique) buttons have been pressed
	var btnCount = 0;
	for (var btnName in buttonPressedArray)
		if (buttonPressedArray[btnName] != null && buttonPressedArray[btnName] = true)
			btnCount++;
	
	//Done
	if (count >= irremote_numButton())
	{
	}
}
