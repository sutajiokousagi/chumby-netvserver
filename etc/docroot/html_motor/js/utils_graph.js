var labels = ["Analog 0", "Analog 1", "Analog 2", "Analog 3", "Analog 4", "Analog 5", "Analog 6", "Analog 7"];
var values = [];
var now = 0;
var clr = [];
var graphData = [];
var label,c,bg;
	
var raph = Raphael(function () {
	var r = Raphael("analog_input_graphs", 620, 280);
	var e = [];
	label = r.text(310, 27, labels[now]).attr({fill: "#000000", stroke: "none", "font": '100 18px "Helvetica Neue", Helvetica, "Arial Unicode MS", Arial, sans-serif'});
	var rightc = r.circle(364, 27, 10).attr({fill: "#808080", stroke: "none"});
	var right = r.path("M360,22l10,5 -10,5z").attr({fill: "#000"});
	var leftc = r.circle(256, 27, 10).attr({fill: "#808080", stroke: "none"});
	var left = r.path("M260,22l-10,5 10,5z").attr({fill: "#000"});
	c = r.path("M0,0").attr({fill: "none", "stroke-width": 4, "stroke-linecap": "round"});
	bg = r.path("M0,0").attr({stroke: "none", opacity: .3});

	//Generate random colors
	for (var i = 0; i < 8; i++)
		clr[i] = Raphael.getColor(1);
		
	graph_update_data(null);
	
	c.attr({path: values[0], stroke: clr[0]});
	bg.attr({path: values[0] + "L590,280 10,280z", fill: clr[0]});
	rightc.click(next_graph);
	right.click(next_graph);
	leftc.click(previous_graph);
	left.click(previous_graph);
});

function next_graph() 		{	now++;		graph_update_drawing();		};
function previous_graph() 	{	now--;		graph_update_drawing();		};

function graph_update_data(array)
{
	if (array == null)
	{
		//Zero all data
		for (var i = 0; i < 8; i++)
			values[i] = randomPath(30, i);
		return;
	}
	
	var length = 30;
	for (var m = 0; m < 8; m++)
	{
		var path = "";
		var x = 10;
		var	y = 0;
		graphData[m] = graphData[m] || [];
		for (var i = 0; i < length; i++)
		{
			graphData[m][i] = array[m][i];
			if (i) {
				x += 20;
				y = 280 - graphData[m][i];
				path += "," + [x, y];
			} else {
				path += "M" + [10, (y = 280 - graphData[m][i])] + "R";
			}
		}
		values[m] = path;
	}
	
	graph_update_drawing();
}

//Helper function to generate random data on-demand
function randomPath(length, j)
{
	var path = "";
	var x = 10;
	var	y = 0;
	graphData[j] = graphData[j] || [];
	for (var i = 0; i < length; i++)
	{
		graphData[j][i] = 0;//Math.round(Math.random() * 255);
		if (i) {
			x += 20;
			y = 280 - graphData[j][i];
			path += "," + [x, y];
		} else {
			path += "M" + [10, (y = 280 - graphData[j][i])] + "R";
		}
	}
	return path;
}

function graph_update_drawing()
{
	var time = 100;
	if (now == 8) {
		now = 0;
	}
	if (now == -1) {
		now = 7;
	}
	var anim = Raphael.animation({path: values[now], stroke: clr[now]}, time, "<>");
	c.animate(anim);
	bg.animateWith(c, anim, {path: values[now] + "L590,280 10,280z", fill: clr[now]}, time, "<>");
	label.attr({text: labels[now]});
};