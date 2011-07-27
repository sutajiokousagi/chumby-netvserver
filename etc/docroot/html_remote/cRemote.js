// -------------------------------------------------------------------------------------------------
//	cRemote class
//
//
//
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
//	constructor
// -------------------------------------------------------------------------------------------------
function cRemote(
)
{
	// members
	this.mJSClassList = [ "./html_remote/cModel.js" ];
		
	this.mCallBackList = [];
	
	// javascript classes
	this.mModel = null;
}

// -------------------------------------------------------------------------------------------------
//	singleton
// -------------------------------------------------------------------------------------------------
cRemote.instance = null;
cRemote.fGetInstance = function()
{
	return cRemote.instance ? cRemote.instance : (cRemote.instance = new cRemote());
}


// -------------------------------------------------------------------------------------------------
//	fInit
// -------------------------------------------------------------------------------------------------
cRemote.prototype.fInit = function()
{
	// load other js classes
	fLoadExtJSScript(this.mJSClassList);
}




// -------------------------------------------------------------------------------------------------
//	callbacks
// -------------------------------------------------------------------------------------------------

cRemote.prototype.helloCallback = function (mModel) {};

// -------------------------------------------------------------------------------------------------
//	communication
// -------------------------------------------------------------------------------------------------

cRemote.prototype.sendSingleValueCommand = function (cmd, dataString)
{
	//Which address to submit to
	var web_address = "./bridge";

	//Convert data string to XML format
	var xmlDataString = '<value>' +  dataString + '</value>';

	//Create a clean new form and input fields
	var myForm = document.createElement("form");
	myForm.method = 'POST';
	myForm.action = web_address;
	myForm.target = 'loadframe';
	
	var myInput1 = document.createElement("input");
	myInput1.setAttribute('name', 'cmd');
	myInput1.setAttribute('value', cmd);
	myForm.appendChild(myInput1);
	
	var myInput2 = document.createElement("input");
	myInput2.setAttribute('name', 'data');
	myInput2.setAttribute('value', xmlDataString);
	myForm.appendChild(myInput2);

	//Submit & clean up
	document.body.appendChild(myForm);
	myForm.submit();
	document.body.removeChild(myForm);
}

cRemote.prototype.sendMultiValueCommand = function (cmd, dataArray)
{
	//Which address to submit to
	var web_address = "./bridge";
	
	//Create a clean new form and input fields
	var myForm = document.createElement("form");
	myForm.method = 'POST';
	myForm.action = web_address;
	myForm.target = 'loadframe';
	
	var myInput1 = document.createElement("input");
	myInput1.setAttribute('name', 'cmd');
	myInput1.setAttribute('value', cmd);
	myForm.appendChild(myInput1);

	for (var key in dataArray)
	{
		var myInput2 = document.createElement("input");
		myInput2.setAttribute('name', key);
		myInput2.setAttribute('value', dataArray[key]);
		myForm.appendChild(myInput2);
	}
	
	//Submit & clean up
	document.body.appendChild(myForm) ;
	myForm.submit() ;
	document.body.removeChild(myForm) ;
}

cRemote.prototype.postNoReturn = function (command, dataString)
{
	var web_address = "./bridge";
	var xmlDataString = '<value>' +  dataString + '</value>';
	
	xmlhttpPost("", "post", { 'cmd': command, 'data': xmlDataString }, null );

	//var dataArray = { 'cmd':command, 'data':xmlDataString };
	//$.ajax({ type: 'POST', url: web_address, data: dataArray });
}

cRemote.prototype.Hello = function ()
{
	var web_address = "./bridge";
	//xmlhttpPost("./bridge?cmd=Hello&type=web&version=1.0", "get", {}, cRemote.instance.rawHelloCallback );
	xmlhttpPost("", "post", { 'cmd' : 'Hello', 
							  'type' : 'web_'+cRemote.instance.getKnownUserAgent(),
							  'version' : '1.0' }, cRemote.instance.rawHelloCallback );
}

cRemote.prototype.rawHelloCallback = function (vData)
{
	if (vData.split("</status>")[0].split("<status>")[1] != "1")
		return;
	
	var o = cModel.fGetInstance();
	o.CHUMBY_GUID = vData.split("</guid>")[0].split("<guid>")[1];
	o.CHUMBY_DCID = vData.split("</dcid>")[0].split("<dcid>")[1];
	o.CHUMBY_HWVERSION = vData.split("</hwver>")[0].split("<hwver>")[1];
	o.CHUMBY_FWVERSION = vData.split("</fwver>")[0].split("<fwver>")[1];
	o.CHUMBY_FLASHPLUGIN = vData.split("</flashplugin>")[0].split("<flashplugin>")[1];
	o.CHUMBY_FLASHPLAYER = vData.split("</flashver>")[0].split("<flashver>")[1];
	o.CHUMBY_NETWORK_MAC = vData.split("</mac>")[0].split("<mac>")[1];
	o.CHUMBY_INTERNET = vData.split("</internet>")[0].split("<internet>")[1];
	
	if (cRemote.instance.helloCallback)
		cRemote.instance.helloCallback(o);
}

cRemote.prototype.RemoteControl = function (buttonName)
{
	//Hmm. why is xmlhttpPost not working on iOS!
	if (cRemote.instance.getKnownUserAgent() == "ios")
		cRemote.instance.sendSingleValueCommand('RemoteControl', buttonName);
	else
		cRemote.instance.postNoReturn("RemoteControl", buttonName);
}

cRemote.prototype.getKnownUserAgent = function ()
{
	var agent = navigator.userAgent.toLowerCase();
		
	//iOS can't be wrong
	if(agent.indexOf('iphone') != -1 || agent.indexOf('ipod') != -1 || agent.indexOf('ipad') != -1 )
	{
		return "ios";
	}
	//Good Android device
	else if(agent.indexOf('android') != -1)
	{
		return "android";
	}
	//Well-known browsers
	else if (agent.indexOf("series60") != -1 || agent.indexOf("symbian") != -1 || agent.indexOf("windows ce") != -1 || agent.indexOf("blackberry") != -1 )
	{
		return "others";
	}
	//Desktop browser
	else if (typeof orientation == 'undefined' ? true : false)
	{
		return "desktop";	
	}
	//China Android devices
	else if(navigator.platform.toLowerCase().indexOf('linux') != -1 && navigator.platform.toLowerCase().indexOf('arm') != -1  )
	{
		return "crap";
	}
	else
	{
		return navigator.userAgent;	
	}
}
