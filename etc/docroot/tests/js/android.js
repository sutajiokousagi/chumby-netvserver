// -------------------------------------------------------------------------------------------------
//	Events from Android app
// -------------------------------------------------------------------------------------------------

function fAndroidEvents( vEventName, vEventData )
{
	vEventData = decodeURIComponent(vEventData);
	switch (vEventName)
	{
		case "changeview":			android_changeView(vEventData);			break;
		case "testconsole":			android_testConsole(vEventData);		break;
	}
}

function android_changeView(xmlDataString)
{
	if (xmlDataString == "reset")
	{
		location.href="http://localhost/";
	}
	else
	{
		main_showState(xmlDataString, true);
	}
}

function android_testConsole(console_string)
{
	//Temporary
	console_string = console_string.replace("+", " ");
	
	htmlString = $("#div_androidTestMain_info").html();
	htmlString += console_string + "<br/>";
	$("#div_androidTestMain_info").html( htmlString );
}
