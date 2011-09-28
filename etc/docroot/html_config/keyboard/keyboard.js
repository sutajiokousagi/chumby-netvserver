var keyboard_currentX;
var keyboard_currentY;
var keyboard_wrapXMode;		//0: stop at end;  1: wrap around;  2: fall though
var keyboard_wrapYMode;		//0: stop at end;  1: wrap around;  2: fall though
var keyboard_keysIDArray;
var keyboard_keysArray0;
var keyboard_keysArray1;
var keyboard_keysArray2;

// -------------------------------------------------------------------------------------------------
//	Initialization
// -------------------------------------------------------------------------------------------------

function keyboard_init()
{
	var row1 = ['a','b','c','d','e','f','g','h','i'];
	var row2 = ['j','k','l','m','n','o','p','q','r'];
	var row3 = ['s','t','u','v','w','x','y','z','0'];
	var row4 = ['1','2','3','4','5','6','7','8','9'];
	var row5 = ['special','special','shift','shift','space','space','space','backspace','backspace'];
	keyboard_keysIDArray = [ row1, row2, row3, row4, row5 ];
	
	row1 = ['a','b','c','d','e','f','g','h','i'];
	row2 = ['j','k','l','m','n','o','p','q','r'];
	row3 = ['s','t','u','v','w','x','y','z','0'];
	row4 = ['1','2','3','4','5','6','7','8','9'];
	row5 = ['.@#','.@#','shift','shift','space','space','space','<--','<--'];
	keyboard_keysArray0 = [ row1, row2, row3, row4, row5 ];
	
	row1 = ['A','B','C','D','E','F','G','H','I'];
	row2 = ['J','K','L','M','N','O','P','Q','R'];
	row3 = ['S','T','U','V','W','X','Y','Z','0'];
	row4 = ['1','2','3','4','5','6','7','8','9'];
	row5 = ['.@#','.@#','shift','shift','space','space','space','<--','<--'];
	keyboard_keysArray1 = [ row1, row2, row3, row4, row5 ];
	
	row1 = ['$','&amp;','&lt;','&gt;','{','}','~','`','^'];
	row2 = ['#','?','[',']','%','|','+','-',''];
	row3 = ['@','!','(',')',':',';','*','=',''];
	row4 = ['.',',','\\','/','&apos;','&quot;','_','',''];
	row5 = ['abc','abc','shift','shift','space','space','space','<--','<--'];
	keyboard_keysArray2 = [ row1, row2, row3, row4, row5 ];
	
	keyboard_currentX = 0;
	keyboard_currentY = -1;
	keyboard_wrapXMode = 0;
	keyboard_wrapYMode = 2;
	keyboard_clearHighlight();
	keyboard_setLayout(0);
}

function keyboard_selectFirstKey()
{
	keyboard_currentX = 0;
	keyboard_currentY = 0;
	keyboard_clearHighlight();
	keyboard_setHighlightKey(keyboard_currentX, keyboard_currentY, true);
}

// -------------------------------------------------------------------------------------------------
//	Internal use
// -------------------------------------------------------------------------------------------------

function keyboard_clearHighlight()
{
	$(".mac_key_selected").removeClass("mac_key_selected").addClass("mac_key");
}

function keyboard_setHighlightKey(x,y,isHighlight)
{
	if (x<0 || y<0 || y>=keyboard_keysIDArray.length || x>=keyboard_keysIDArray[y].length)
		return keyboard_clearHighlight();
		
	var id = "#key_" + keyboard_keysIDArray[y][x];
	if (isHighlight)		$(id).removeClass("mac_key").addClass("mac_key_selected");
	else					$(id).removeClass("mac_key_selected").addClass("mac_key");
}

function keyboard_onRemoteControl(direction, elementID)
{
	var newX = keyboard_currentX;
	var newY = keyboard_currentY;
	if (direction == 'left')				newX--;
	else if (direction == 'right')			newX++;
	else if (direction == 'up')				newY--;
	else if (direction == 'down')			newY++;
	else if (direction == "center")
	{
		keyboard_applyCurrentKey(elementID);
		return false;
	}
	
	var returnFocus = false;
	var bounce = false;
	
	switch (keyboard_wrapYMode)
	{
		case 0:
			if (newY >= keyboard_keysIDArray.length)	{	newY = keyboard_keysIDArray.length-1;		bounce = true;		}
			else if (newY < 0)							{	newY = 0;									bounce = true;		}
			break;
		case 1:
			if (newY >= keyboard_keysIDArray.length)		newY = 0;
			else if (newY < 0)								newY = keyboard_keysIDArray.length-1;
			break;
		default:
			if (newY >= keyboard_keysIDArray.length)		returnFocus = true;
			else if (newY < 0)								returnFocus = true;
			if (newY >= keyboard_keysIDArray.length)		newY = keyboard_keysIDArray.length;
			else if (newY < 0)								newY = -1;
			break;
	}
	if (newY >= 0 && newY < keyboard_keysIDArray.length)
	{
		switch (keyboard_wrapXMode)
		{
			case 0:
				if (newX >= keyboard_keysIDArray[newY].length)	{	newX = keyboard_keysIDArray[newY].length-1;		bounce = true;		}
				else if (newX < 0)								{	newX = 0;										bounce = true;		}
				break;
			case 1:
				if (newX >= keyboard_keysIDArray[newY].length)	newX = 0;
				else if (newX < 0)								newX = keyboard_keysIDArray[newY].length-1;
				break;
			default:
				if (newX >= keyboard_keysIDArray[newY].length)	returnFocus = true;
				else if (newX < 0)								returnFocus = true;
				if (newX >= keyboard_keysIDArray[newY].length)	newX = keyboard_keysIDArray[newY].length;
				else if (newX < 0)								newX = -1;
				break;
		}
	}
	
	keyboard_clearHighlight();
	var oldValue = keyboard_getCurrentKey();
	var newValue = keyboard_getKeyValue(newX,newY);
	keyboard_setHighlightKey(newX,newY, true);
	keyboard_currentX = newX;
	keyboard_currentY = newY;
	if (oldValue != newValue || newValue == "" || oldValue == "" || bounce == true)
		return returnFocus;
		
	return keyboard_onRemoteControl(direction, elementID);
}

// ----------------------------

function keyboard_setLayout(mode)
{
	var id;
	for (var y in keyboard_keysIDArray)
		for (var x in keyboard_keysIDArray[y])
		{
			id = "#key_" + keyboard_keysIDArray[y][x];
			if (mode == 0)				$(id).html( keyboard_keysArray0[y][x] );
			else if (mode == 1)			$(id).html( keyboard_keysArray1[y][x] );
			else if (mode == 2)			$(id).html( keyboard_keysArray2[y][x] );
		}
}

function keyboard_getLayout()
{
	id = "#key_" + keyboard_keysIDArray[0][0];
	if ( $(id).html() == keyboard_keysArray0[0][0] )		return 0;
	if ( $(id).html() == keyboard_keysArray1[0][0] )		return 1;
	if ( $(id).html() == keyboard_keysArray2[0][0] )		return 2;
	return 0;
}

// ------------------------------

function keyboard_getKeyValue(x,y)
{
	if (x<0 || y<0 || y>=keyboard_keysIDArray.length || x>=keyboard_keysIDArray[y].length)
		return "";
	return keyboard_keysIDArray[y][x];
}

function keyboard_getCurrentKey()
{
	return keyboard_getKeyValue(keyboard_currentX,keyboard_currentY);
}

function keyboard_applyCurrentKey(elementID)
{
	var currentKeyID = keyboard_getCurrentKey();
	var currentKeyValue = $("#key_" + currentKeyID).html();
	if (currentKeyID == '' || currentKeyValue == '')
		return;

	if (currentKeyValue == "&amp;")				currentKeyValue = "&";
	else if (currentKeyValue == "&lt;")			currentKeyValue = "<";
	else if (currentKeyValue == "&gt;")			currentKeyValue = ">";
	else if (currentKeyValue == "&quot;")		currentKeyValue = "\"";
	else if (currentKeyValue == "&apos;")		currentKeyValue = "'";
		
	var currentValue = $("#"+elementID).val();
	switch (currentKeyID)
	{
		case "space":			currentValue += " ";												break;
		case "backspace":		currentValue = currentValue.substring(0, currentValue.length-1);	break;
		case "back":
		case "next":
		case "special":
			if (keyboard_getLayout() != 2)		keyboard_setLayout(2);
			else								keyboard_setLayout(0);
			break;
		case "shift":
			if (keyboard_getLayout() == 0) 		keyboard_setLayout(1);
			else								keyboard_setLayout(0);
			break;
		default:				currentValue += currentKeyValue;									break;
	}
	$("#"+elementID).val(currentValue);
	$("#"+elementID).blur();
	$("#"+elementID).focus();
}