function sendSingleValueCommand(cmd, dataString)
{
	//Which address to submit to
	var web_address = "./bridge";

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

function Hello()
{
	sendSingleValueCommand('Hello', '');
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

function GetBase64()
{
	var dataString = document.form_GetBase64.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('GetBase64', dataString);
	}
}

function HasFlashPlugin()
{
	sendSingleValueCommand('HasFlashPlugin', '');
}

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

function SetBox()
{
	var dataString = document.form_SetBox.inputField.value;
	
	if (dataString == '') 
	{
		alert('Input field is required');
		console.log('Input field is empty');
	}
	else
	{
		sendSingleValueCommand('SetBox', dataString);
	}
}

function SetChromaKeyOnOff()
{
	var dataString = document.form_SetChromaKeyOnOff.inputField.value;
	
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

function SetChromaKeyColor()
{
	var dataString = document.form_SetChromaKeyColor.inputField.value;
	
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

function LongPoll()
{
	sendSingleValueCommand('LongPoll', '');
}

function GetAllParams()
{
	sendSingleValueCommand('GetAllParams', '');
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
	var dataString1 = document.form_GetXML.inputField.value;
	var dataString2 = document.form_GetXML.inputField2.value;
	var dataArray = { myKey1:dataString1, myKey2:dataString2 };
	
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
	// <time> is time formatted in GMT time as "yyyy.mm.dd-hh:mm:ss"
    // <timezone> is standard timezone ID string formated as "Asia/Singapore
		
	var dataString1 = document.form_SetNetwork.inputField.value;
	var dataString2 = document.form_SetNetwork.inputField2.value;
	var dataString3 = document.form_SetNetwork.inputField3.value;
	var dataString4 = document.form_SetNetwork.inputField4.value;
	var dataString5 = document.form_SetNetwork.inputField5.value;
	var dataArray = { wifi_ssid:dataString1, wifi_password:dataString2, wifi_encryption:dataString3, wifi_auth:dataString4, wifi_allocation:dataString5 };
	
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
	// <time> is time formatted in GMT time as "yyyy.mm.dd-hh:mm:ss"
    // <timezone> is standard timezone ID string formated as "Asia/Singapore
		
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