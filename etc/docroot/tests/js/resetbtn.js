function resetbtn_init(testIndex)
{	
	if (testIndex == 0)
	{
		$("#div_resetBtnMain_instruction1").show();
		$("#div_resetBtnMain_instruction2").hide();
	}
	else
	{
		$("#div_resetBtnMain_instruction1").hide();
		$("#div_resetBtnMain_instruction2").show();
	}
}

function resetbtn_onRemoteControl(vButtonName)
{
	if (vButtonName != "setup")
		return;
		
	resetbtn_init(1);
	
	setTimeout("resetbtn_onComplete()", 2500);
}

function resetbtn_onComplete()
{
	main_showState("done");
}