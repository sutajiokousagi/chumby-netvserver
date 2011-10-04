
// -------------------------------------------------------------------------------------------------
//	override fDbg() function
// -------------------------------------------------------------------------------------------------

function fDbg(v)
{
	console.log("|~|" + v);
}

// -------------------------------------------------------------------------------------------------
//	fLoadScript
// -------------------------------------------------------------------------------------------------

function fLoadExtJSScript( vFileList )
{
	var vUrl = vFileList.pop();
	var script = document.createElement("script");
	
	script.type = "text/javascript";
	script.src = vUrl;
	
	script.onload = function()
	{
		if (vFileList.length == 0)
			return;
		fLoadExtJSScript(vFileList);
	};
	
	document.getElementsByTagName("head")[0].appendChild(script);
}


// -------------------------------------------------------------------------------------------------
//	get/post calls
// -------------------------------------------------------------------------------------------------

function xmlhttpPost(
	vUrl,
	vType,			// "post" | "get"
	vData,
	vCompleteFunc
)
{
	var xmlHttpReq = getXHR();
	if (xmlHttpReq == null) {
		aleart("Failed to get XMLHttpRequest");
		return;
	}
	
	if (vUrl == '')
		vUrl = "../bridge";
	xmlHttpReq.open(vType, vUrl, true);
	xmlHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
	xmlHttpReq.onreadystatechange = function()
	{
		if (xmlHttpReq.readyState == 4)
		{
			switch (xmlHttpReq.status)
			{
				case 200:
					if (vCompleteFunc)
						vCompleteFunc(xmlHttpReq.responseText);
					break;
				case 0:
					if (vCompleteFunc)
						vCompleteFunc(0);
					break;
			}
		}
	}
	
	if (vType.toLowerCase() == "post")
	{
		var parameters = "";
		for (var o in vData)
			parameters += o + "=" + encodeURIComponent(vData[o]) + "&";	
		parameters = parameters.substring(0, parameters.length-1);
			
		xmlHttpReq.send(parameters);
	}
	else
	{
		xmlHttpReq.send();
	}
}

//Get an XMLHttpRequest, object
function getXHR()
{
	var req = null;
	if(window.XMLHttpRequest)
	{
		req = new XMLHttpRequest();
		return req;
	}
	else
	{
		try {
			req = new ActiveXObject('Msxml2.XMLHTTP');		//IE
			return req;
		}
		catch(e)
		{
			try {
				req = new ActiveXObject("Microsoft.XMLHTTP");		//IE
				return req;
			}
			catch(e)
			{
				return null;
			}
		}
	}
	return req;
}