var currentX;
var currentY;
var countX;
var countY;
var keysArray;

function keyboard_init()
{
	var row1 = ['a','b','c','d','e','f','g','h','i'];
	var row2 = ['j','k','l','m','n','o','p','q','r'];
	var row3 = ['s','t','u','v','w','x','y','z','0'];
	var row4 = ['1','2','3','4','5','6','7','8','9'];
	var row5 = ['back','special','shift','space','backspace','next','-','-','-'];
	keysArray = [ row1, row2, row3, row4, row5 ];
	
	currentX = 0;
	currentY = 0;
	keyboard_setHighlightKey(currentX,currentY,true);
}

function keyboard_onRemoteControl(direction)
{
	var newX = currentX;
	var newY = currentY;
	if (direction == 'left')				newX--;
	else if (direction == 'right')			newX++;
	else if (direction == 'up')				newY--;
	else if (direction == 'down')			newY++;
	
	if (newY > keysArray.length)			newY = row1.length;
	else if (newY < 0)						newY = 0;
	if (newX > keysArray[newY].length)		newX = keysArray[newY].length;
	else if (newX < 0)						newX = 0;
	
	keyboard_setHighlightKey(currentX,currentY, false);
	keyboard_setHighlightKey(newX,newY, false);
	currentX = newX;
	currentY = newY;
	return keyboard_getCurrentKey();
}

function keyboard_setHighlightKey(x,y,isHighlight)
{
	var id = "key_" + keysArray[x][y];
	if (isHighlight)		$(id).removeClass("mac_key").addClass("mac_key_selected");
	else					$(id).removeClass("mac_key_selected").addClass("mac_key");
}

function keyboard_getCurrentKey()
{
	return keysArray[currentX][currentY];
}