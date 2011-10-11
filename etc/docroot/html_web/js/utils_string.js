
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
		if (iC < 65 || iC > 127 || (iC>90 && iC<97))
			aRet[i] = '&#'+iC+';';
		else
			aRet[i] = str[i];
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
