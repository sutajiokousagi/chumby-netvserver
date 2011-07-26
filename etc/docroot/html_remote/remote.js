function sendSingleValueCommand(cmd, dataString)
{
	//Which address to submit to
	var web_address = "../bridge";

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

function sendMultiValueCommand(cmd, dataArray)
{
	//Which address to submit to
	var web_address = "../bridge";
	
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

function Hello()
{
	sendSingleValueCommand('Hello', '');
}

function RemoteControl(buttonName)
{
	if (buttonName != '') 
		sendSingleValueCommand('RemoteControl', buttonName);
}
