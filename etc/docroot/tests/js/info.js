var helloParamNamesArray = new Array('guid', 'hwver', 'fwver', 'internet', 'mac', 'ip', 'flashplugin', 'flashver');

function Hello()
{
	fDbg("Waiting for handshake response...");
	xmlhttpPost("", "post", { 'cmd' : 'Hello' }, rawHelloCallback );
}

function rawHelloCallback(vData)
{
	if (vData.split("</status>")[0].split("<status>")[1] != "1")
		return;
	
	var helloData = new Array();
	for (var idx in helloParamNamesArray)
	{
		var paramName = helloParamNamesArray[idx];
		helloData[paramName] = vData.split("</"+paramName+">")[0].split("<"+paramName+">")[1];
	}

	helloCallback(helloData);
}

function helloCallback(helloData)
{
	var htmlString = "";
	htmlString += "MAC: <font color='6598EB'>" + helloData['mac'] + "</font><br/><br/>";
	htmlString += "Firmware: " + helloData['fwver'] + "<br/><br/>";
	htmlString += "GUID: " + helloData['guid'] + "<br/><br/>";
	htmlString += "IP: " + helloData['ip'] + "<br/><br/>";
	$("#div_androidTestInfo_info").html(htmlString);
}