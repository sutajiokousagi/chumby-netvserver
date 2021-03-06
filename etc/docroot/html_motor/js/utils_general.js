
function getServerIPAddress()
{
	var tempLocaltion = "" + document.location;
	if (tempLocaltion.indexOf("localhost") != -1)
		return "localhost";
	if (tempLocaltion.indexOf("file://") != -1)
		return "file";
	return "" + document.location.host;
}

function getServerUrl()
{
	var addr = getServerIPAddress();
	if (addr == "file")
		return "";
	return "http://" + addr + "/bridge";
}

// -------------------------------------------------------------------------------------------------
// Get a GET parameter by its name
// -------------------------------------------------------------------------------------------------

function getGETParameterByName(name)
{
	name = name.replace(/[\[]/, "\\\[").replace(/[\]]/, "\\\]");
	var regexS = "[\\?&]" + name + "=([^&#]*)";
	var regex = new RegExp(regexS);
	var results = regex.exec(window.location.href);
	if(results == null)
		return "";
    return decodeURIComponent(results[1].replace(/\+/g, " "));
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
		alert("Failed to get XMLHttpRequest");
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

function xmlhttpUploadFile(
	vUrl,
	vPath,
	vFile,
	vCompleteFunc
)
{
	if (!vFile)
		return;
	
	var xmlHttpReq = getXHR();
	if (xmlHttpReq == null) {
		alert("Failed to get XMLHttpRequest");
		return;
	}
	
	if (vUrl == '')
		vUrl = "../bridge";
	vUrl += "?cmd=UploadFile&path=" + encodeURIComponent(vPath);
	xmlHttpReq.open('POST', vUrl, true);
	xmlHttpReq.setRequestHeader("Cache-Control", "no-cache");
	xmlHttpReq.setRequestHeader('Content-Type', 'multipart/form-data');
	xmlHttpReq.setRequestHeader("X-Requested-With", "XMLHttpRequest");
	xmlHttpReq.setRequestHeader("X-File-Name", "filedata");

	upload = xmlHttpReq.upload;
	upload.addEventListener("progress", function (ev)
	{
		if (ev.lengthComputable)
			console.log( (ev.loaded / ev.total) * 100 + "%" );
	}, false);
	
	upload.addEventListener("load", function (ev) {
		console.log("Upload completed");
	}, false);
	
	upload.addEventListener("error", function (ev) {console.log(ev);}, false);
	
	xmlHttpReq.send(vFile);
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
			req = new ActiveXObject('Msxml2.XMLHTTP');				//IE
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