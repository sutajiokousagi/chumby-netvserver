var serverUrl = getServerUrl();				//utils_general.js
var serverIP = getServerIPAddress();		//utils_general.js
var publicIP = "";

var global_parameters = new Array();
global_parameters['log_hostinfo'] = true;
global_parameters['log_remotecontrol_btn'] = false;
global_parameters['log_sharephoto_btn'] = true;

function onLoad()
{
	//Initialize tabs with jQueryUI
	$("#tabs").tabs().removeClass('ui-corner-all').removeClass('ui-widget-content');
    $("#tabs li").removeClass('ui-corner-top');
	$("#tabs ul").removeClass('ui-widget-header').removeClass('ui-corner-all');
	
	$( "button" ).button();
	$( "button" ).click(function() { return false; });
	
	$.getJSON("http://jsonip.appspot.com?callback=?", function(da){ publicIP = da.ip; });
	
	setTimeout("onLoadLater()", 1000);
	
	//Testing Chinese character & UTF-8 encoding
	//xmlhttpPost(serverUrl, "post", { cmd : 'RemoteControl', value : '测试' } );
}

function onLoadLater()
{
	if (global_parameters['log_hostinfo'])
	{
		console.log(document.location);
		console.log("Public IP: " + publicIP);
		console.log("Server IP: " + serverIP);
		console.log("Server URL: " + serverUrl);
	}
}