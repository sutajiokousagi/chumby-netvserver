// -------------------------------------------------------------------------------------------------
//	Events from Android app
// -------------------------------------------------------------------------------------------------

function fAndroidEvents( vEventName, vEventData )
{
	vEventData = decodeURIComponent(vEventData);
	switch (vEventName)
	{
		case "changeview":			android_changeView(vEventData);			break;
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
