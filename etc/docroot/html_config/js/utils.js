
// -------------------------------------------------------------------------------------------------
//	console debug
// -------------------------------------------------------------------------------------------------
function fDbg2(v)
{
	console.log("|~|" + v);
}

// -------------------------------------------------------------------------------------------------
//  string utility
// -------------------------------------------------------------------------------------------------

function XmlEscape(s)
{
	var el = document.createElement("div");
	el.innerText = el.textContent = s;
	s = el.innerHTML;
	delete el;
	return s;
}
function XmlUnescape(s)
{
	/*
	s = s.replace("&amp;", "&");
	s = s.replace("&quot;", "\"");
	s = s.replace("&apos;", "'");
	s = s.replace("&lt;", "<");
	s = s.replace("&gt;", ">");
	*/
	var el = document.createElement("div");
	el.innerHTML = s;
	s = el.innerText;
	delete el;
	return s;	
}
function HTMLEncode(str)
{
	var i = str.length;
	var aRet = [];

  while (i--)
  {
    var iC = str[i].charCodeAt();
    if (iC < 65 || iC > 127 || (iC>90 && iC<97)) {
      aRet[i] = '&#'+iC+';';
    } else {
      aRet[i] = str[i];
    }
   }
  return aRet.join('');    
}

//------------------------------------------------------------------------
// Small non-UI utilities
//------------------------------------------------------------------------

function isArray(o)
{
    return Object.prototype.toString.call(o) === '[object Array]';
}

function getObjectType(o)
{
    return Object.prototype.toString.call(o);
}

function myLog(text)
{
    console.log(text)
}

function stringEndsWith(text, searchText)
{
    return (text.match(searchText+"$")==searchText);
}

function stringStartsWith(text, searchText)
{
    return (text.indexOf(searchText) === 0) ? true : false;
}

function stringContains(text, searchText)
{
    return (text.indexOf(searchText) != -1);
}

function truncate(_value)
{
    if (_value<0)   return Math.ceil(_value);
    else            return Math.floor(_value);
}

function isNumber(n)
{
    if (n == null)
        return false;
    return !isNaN(parseFloat(n)) && isFinite(n);
}

function getSign(value)
{
    return value / Math.abs(value);
}

function trim(s)
{
    var l=0; var r=s.length -1;
    while(l < s.length && s[l] == ' ')
    {	l++;    }
    while(r > l && s[r] == ' ')
    {	r-=1;	}
    return s.substring(l, r+1);
}

function trimNewLine(s)
{
    return s.replace(new RegExp("\\n", "g"), "").replace(new RegExp("\\r", "g"), "");
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