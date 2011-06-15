function sendSingleValueCommand(cmd, dataString)
{
	//Which address to submit to
	var web_address = "http://192.168.1.48/bridge";
	if (document.form_Target.input_target_checkbox.checked)
		web_address = "/bridge";
	else if (document.form_Target.input_target.value != "")
		web_address = "http://" + document.form_Target.input_target.value + "/bridge";

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
	var web_address = "http://192.168.1.48/bridge";
	if (document.form_Target.input_target_checkbox.checked)
		web_address = "/bridge";
	else if (document.form_Target.input_target.value != "")
		web_address = "http://" + document.form_Target.input_target.value + "/bridge";

	//Convert data array to XML format
	var xmlDataString = '';
	for (var key in dataArray)
		xmlDataString += '<' + key + '>' + dataArray[key] + '</' + key + '>';
	
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
	document.body.appendChild(myForm) ;
	myForm.submit() ;
	document.body.removeChild(myForm) ;
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
	/*
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
	*/
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


