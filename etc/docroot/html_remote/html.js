var mRemote;

function onLoad()
{
	mRemote = cRemote.fGetInstance();
	mRemote.fInit();
	mRemote.helloCallback = updateBasicInfo;
	
	//Touch without delay
	var isTouchDevice = ("ontouchstart" in document.documentElement ? true : false)
	if (isTouchDevice)	initFastButtons();
	else				initNormalButtons();
	
	//Init layout
	updateOrientation();
	document.getElementById("page_wrapper").style.visibility = "visible";
	
	//Load later
	setTimeout("onLoadLater()",200);
	
	//UI
	var agent = mRemote.getKnownUserAgent();
	var elemWidth = document.documentElement.clientWidth;
	var elemHeight = document.documentElement.clientHeight;
		
	//iOS can't be wrong
	if(agent == "ios")
	{
		document.getElementById("recommendation").innerHTML = "For better experience please download <a href='http://itunes.apple.com/us/app/samsung-remote/id359580639?mt=8&uo=4' target='itunes_store'>NeTV app</a>";
	}
	//Good Android device
	else if(agent == "android")
	{
		document.getElementById("recommendation").innerHTML = "For better experience please download <a href='market://details?id=com.samsung.remoteTV'>NeTV app</a>";
	}
	//Well-known browsers
	else if (agent == "others")
	{
		document.getElementById("recommendation").innerHTML = "";
	}
	//Desktop browser
	else if (agent == "desktop")
	{
		document.getElementById("recommendation").innerHTML = "Desktop PC: " + navigator.platform;		
	}
	//China Android devices
	else if(agent == "crap")
	{
		document.getElementById("recommendation").innerHTML = "For better experience please download <a href='market://details?id=com.samsung.remoteTV'>NeTV app</a>";
	}
	else
	{
		document.getElementById("recommendation").innerHTML = navigator.userAgent + " --- " + navigator.platform;		
	}
	
	//document.getElementById("recommendation").innerHTML += " --- " + elemWidth + "," + elemHeight;
}

function onLoadLater()
{
	mRemote.Hello();
}

function initFastButtons()
{
	new FastButton(document.getElementById("btn_cpanel"), function(){  mRemote.RemoteControl('cpanel');  }  );
	new FastButton(document.getElementById("btn_widget"), function(){  mRemote.RemoteControl('widget');  }  );
	new FastButton(document.getElementById("btn_up"), function(){  mRemote.RemoteControl('up');  }  );
	new FastButton(document.getElementById("btn_down"), function(){  mRemote.RemoteControl('down');  }  );
	new FastButton(document.getElementById("btn_left"), function(){  mRemote.RemoteControl('left');  }  );
	new FastButton(document.getElementById("btn_right"), function(){  mRemote.RemoteControl('right');  }  );
	new FastButton(document.getElementById("btn_center"), function(){  mRemote.RemoteControl('center');  }  );
};

function initNormalButtons()
{
	document.getElementById("btn_cpanel").onclick = function(event){  mRemote.RemoteControl('cpanel');  };
	document.getElementById("btn_widget").onclick = function(event){  mRemote.RemoteControl('widget');  };
	document.getElementById("btn_up").onclick = function(event){  mRemote.RemoteControl('up');  };
	document.getElementById("btn_down").onclick = function(event){  mRemote.RemoteControl('down');  };
	document.getElementById("btn_left").onclick = function(event){  mRemote.RemoteControl('left');  };
	document.getElementById("btn_right").onclick = function(event){  mRemote.RemoteControl('right');  };
	document.getElementById("btn_center").onclick = function(event){  mRemote.RemoteControl('center');  };
};

//-----------------------------------------------------------

function blockMove(event)
{
	// Tell Safari not to move the window.
	event.preventDefault() ;
}

//-----------------------------------------------------------

function updateOrientation()
{
	var contentType = "show_";
	switch(window.orientation)
	{
		case -90:
		contentType += "right";
		break;

		case 90:
		contentType += "left";
		break;

		case 180:
		contentType += "flipped";
		break;
		
		default:
		contentType += "normal";
		break;
	}
	document.getElementById("page_wrapper").setAttribute("class", contentType);
	document.getElementById("page_wrapper").setAttribute("width", document.documentElement.clientWidth);
	document.getElementById("page_wrapper").setAttribute("height", document.documentElement.clientHeight);
}

//-----------------------------------------------------------

function updateBasicInfo(mModel)
{
	if (!mModel)
		return;
	document.getElementById("statusinfo").innerHTML = "HW: " + mModel.CHUMBY_HWVERSION + "  " + "FW: " + mModel.CHUMBY_FWVERSION;
}