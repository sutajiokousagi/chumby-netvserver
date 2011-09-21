//----------------------------------------------------------------
// Common functions
//----------------------------------------------------------------

var ipaddress = "";

function onLoad()
{
	//Set the current date & time for easy testing
	var now = new Date();
	var unixDateFormat = now.getFullYear()+'.'+(now.getMonth()+1)+'.'+now.getDate(); 
	unixDateFormat += '-'+now.getHours()+':'+now.getMinutes()+':'+now.getSeconds(); 
	initForm("form_SetTime", "inputField", unixDateFormat);
}

function initForm(form_id, element_name, init_txt)
{
	oFormObject = document.forms[form_id];
	frmElement = oFormObject.elements[element_name];
	frmElement.value = init_txt;
}

function sendSingleValueCommand(cmd, dataString)
{
	//Which address to submit to
	var web_address = "./bridge";
	if (ipaddress && ipaddress != "")
		web_address = "http://" + ipaddress + "/bridge";

	//Convert data string to XML format
	var xmlDataString = '<value>' +  dataString + '</value>';
	
	//Create a clean new form and input fields
	var myForm = document.createElement("form");
	myForm.method = 'POST';
	myForm.action = web_address;
	
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

function sendMultiValueCommand(cmd, dataArray)
{
	//Which address to submit to
	var web_address = "./bridge";
	if (ipaddress && ipaddress != "")
		web_address = "http://" + ipaddress + "/bridge";
		
	//Create a clean new form and input fields
	var myForm = document.createElement("form");
	myForm.method = 'POST';
	myForm.action = web_address;
	
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

// return the value of the radio button that is checked
// return an empty string if none are checked, or
// there are no radio buttons
function getCheckedValue(radioObj)
{
	if(!radioObj)
		return "";
	var radioLength = radioObj.length;
	if(radioLength == undefined)
		if(radioObj.checked)		return radioObj.value;
		else						return "";
	
	for(var i = 0; i < radioLength; i++)
		if(radioObj[i].checked)
			return radioObj[i].value;
	
	return "";
}

//----------------------------------------------------------------
// Public API
//----------------------------------------------------------------

function Hello()
{
	sendSingleValueCommand('Hello', '');
}

function WifiScan()
{
	sendSingleValueCommand('WifiScan', '');
}

function RemoteControl()
{
	var dataString = document.form_RemoteControl.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('RemoteControl', dataString);
	}
}

function FileExists()
{
	var dataString = document.form_FileExists.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('FileExists', dataString);
	}
}

function GetParam()
{
	var dataString = document.form_GetParam.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('GetParam', dataString);
	}
}

function SetParam()
{
	var dataString1 = document.form_SetParam.inputField.value;
	var dataString2 = document.form_SetParam.inputField2.value;
	var dataArray = { 'dataxml_cpanel/username':dataString1, 'dataxml_cpanel/password':dataString2 };
	
	if (dataString1 == '' || dataString2 == '') 
	{
		alert('Input field(s) are required');
		console.log('Input field(s) are empty');
	}
	else
	{
		sendMultiValueCommand('SetParam', dataArray);
	}
}

function TickerEvent()
{
	var dataString1 = document.form_TickerEvent.inputField.value;
	var dataString2 = document.form_TickerEvent.inputField2.value;
	var dataString3 = document.form_TickerEvent.inputField3.value;
	var dataString4 = document.form_TickerEvent.inputField4.value;
	var dataString5 = document.form_TickerEvent.inputField5.value;
	var dataArray = { 'dataxml_message':dataString1, 'dataxml_title':dataString2, 'dataxml_image':dataString3, 'dataxml_type':dataString4, 'dataxml_level':dataString5 };
	
	if (dataString2 == '') 
	{
		alert('Title field is required');
		console.log('Title field is empty');
	}
	else
	{
		sendMultiValueCommand('TickerEvent', dataArray);
	}
}

function EnableSSH()
{
	var dataString = getCheckedValue(document.forms['form_EnableSSH'].elements['inputField']);
	sendSingleValueCommand('EnableSSH', dataString);
}

//----------------------------------------------------------------
// Private API
//----------------------------------------------------------------

function SetUrl()
{
	var dataString = document.form_SetUrl.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('SetUrl', dataString);
	}
}

function GetXML()
{
	var dataString = document.form_GetXML.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('GetXML', dataString);
	}
}

function GetUrl()
{
	var dataString1 = document.form_GetUrl.inputField.value;
	var dataString2 = document.form_GetUrl.inputField2.value;
	var dataArray = { url:dataString1, post:dataString2 };
	
	if (dataString1 == '' || dataString2 == '') 
	{
		alert('Input field(s) are required');
		console.log('Input field(s) are empty');
	}
	else
	{
		sendMultiValueCommand('GetUrl', dataArray);
	}
}

function GetJPG()
{
	var dataString = document.form_GetJPG.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('GetJPG', dataString);
	}
}

//---------------------------------
//Not in used

function PlayWidget()
{
	var dataString = document.form_PlayWidget.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('PlayWidget', dataString);
	}
}

function PlaySWF()
{
	var dataString = document.form_PlaySWF.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('PlaySWF', dataString);
	}
}

function SetWidgetSize()
{
	var dataString = document.form_SetWidgetSize.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendMultiValueCommand('SetWidgetSize', dataArray);
	}
}

function WidgetEngine()
{
	var dataString = document.form_WidgetEngine.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('WidgetEngine', dataString);
	}
}

//---------------------------------

function SetChromaKeyOnOff()
{
	var dataString = getCheckedValue(document.forms['form_SetChromaKeyOnOff'].elements['inputField']);
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('SetChromaKey', dataString);
	}
}

function ControlPanel()
{
	var dataString = document.form_ControlPanel.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('ControlPanel', dataString);
	}
}

function GetAllParams()
{
	sendSingleValueCommand('GetAllParams', '');
}

function GetFileContents()
{
	var dataString = document.form_GetFileContents.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('GetFileContents', dataString);
	}
}

function SetFileContents()
{
	var dataString1 = document.form_SetFileContents.inputField.value;
	var dataString2 = document.form_SetFileContents.inputField2.value;
	var dataArray = { path:dataString1, content:dataString2 };
	
	if (dataString1 == '' || dataString2 == '') 
	{
		alert('Input field(s) are required');
		console.log('Input field(s) are empty');
	}
	else
	{
		sendMultiValueCommand('SetFileContents', dataArray);
	}
}

function GetFileSize()
{
	var dataString = document.form_GetFileSize.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('GetFileSize', dataString);
	}
}

function UnlinkFile()
{
	var dataString = document.form_UnlinkFile.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('UnlinkFile', dataString);
	}
}

function SetTime()
{
	// <time> is time formatted in GMT time as "yyyy.mm.dd-hh:mm:ss"
    // <timezone> is standard timezone ID string formated as "Asia/Singapore
		
	var dataString1 = document.form_SetTime.inputField.value;
	var dataString2 = document.form_SetTime.inputField2.value;
	var dataArray = { time:dataString1, timezone:dataString2 };
	
	if (dataString1 == '') 
	{
		alert('Input field(s) are required');
		console.log('Input field(s) are empty');
	}
	else
	{
		sendMultiValueCommand('SetTime', dataArray);
	}
}

function SetNetwork()
{
	var dataString1 = document.form_SetNetwork.inputField.value;
	var dataString2 = document.form_SetNetwork.inputField2.value;
	var dataString4 = document.form_SetNetwork.inputField4.value;
	var dataString5 = document.form_SetNetwork.inputField5.value;
	var dataString6 = getCheckedValue(document.forms['form_SetNetwork'].elements['inputField6']);
	var dataArray = { wifi_ssid:dataString1, wifi_password:dataString2, wifi_authentication:dataString4, wifi_allocation:dataString5, test:dataString6 };
	
	if (dataString1 == '')
	{
		alert('Input field(s) are required');
		console.log('Input field(s) are empty');
	}
	else
	{
		sendMultiValueCommand('SetNetwork', dataArray);
	}
}

function SetAccount()
{
	var dataString1 = document.form_SetAccount.inputField.value;
	var dataString2 = document.form_SetAccount.inputField2.value;
	var dataString3 = document.form_SetAccount.inputField3.value;
	var dataArray = { chumby_username:dataString1, chumby_password:dataString2, chumby_device_name:dataString3 };
	
	if (dataString1 == '' || dataString2 == '') 
	{
		alert('Input field(s) are required');
		console.log('Input field(s) are empty');
	}
	else
	{
		sendMultiValueCommand('SetAccount', dataArray);
	}
}

function NeCommand()
{
	var dataString = document.form_NeCommand.inputField.value;

	if (dataString == '') 
	{
		alert('Input field(s) are required');
		console.log('Input field(s) are empty');
	}
	else
	{
		sendSingleValueCommand('NeCommand', dataString);
	}
}