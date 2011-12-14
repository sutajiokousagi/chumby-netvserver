var labels = ["Analog 0", "Analog 1", "Analog 2", "Analog 3", "Analog 4", "Analog 5", "Analog 6", "Analog 7"];
var values = [];
var currentIndex = 0;
var clr = [];
var graphData = [];
var label,c,bg;
var graph_width = 1000;
var graph_height = 280;
var graph_points = 100;
var graph_channels = 8;
var graph_interpolation = "R";		//L for linear		//see http://raphaeljs.com/reference.html#Paper.path
	
var raph = Raphael(function () {
	var r = Raphael("analog_input_graphs", graph_width, graph_height);
	var e = [];
	label = r.text(graph_width/2, 27, labels[currentIndex]).attr({fill: "#000000", stroke: "none", "font": '100 18px "Helvetica Neue", Helvetica, "Arial Unicode MS", Arial, sans-serif'});
	var rightc = r.circle(graph_width/4*3, 27, 10).attr({fill: "#808080", stroke: "none"});
	var right = r.path("M"+(graph_width/4*3)+",22l10,5 -10,5z").attr({fill: "#000"});
	var leftc = r.circle(graph_width/4, 27, 10).attr({fill: "#808080", stroke: "none"});
	var left = r.path("M"+(graph_width/4)+",22l-10,5 10,5z").attr({fill: "#000"});
	c = r.path("M0,0").attr({fill: "none", "stroke-width": 2, "stroke-linecap": "square"});
	bg = r.path("M0,0").attr({stroke: "none", opacity: .3});

	//Generate random colors
	for (var i = 0; i < graph_channels; i++)
		clr[i] = Raphael.getColor(1);
		
	graph_update_data(null);
	
	c.attr({path: values[0], stroke: clr[0]});
	bg.attr({path: values[0] + "L"+graph_width+","+graph_height+" 0,"+graph_height+"z", fill: clr[0]});
	rightc.click(next_graph);
	right.click(next_graph);
	leftc.click(previous_graph);
	left.click(previous_graph);
});

function next_graph() 		{	currentIndex++;		graph_update_drawing();		};
function previous_graph() 	{	currentIndex--;		graph_update_drawing();		};

function graph_update_data(array)
{
	if (array == null)
	{
		//Zero all data
		for (var i = 0; i < graph_channels; i++)
			values[i] = randomPath(graph_points, i);
		return;
	}
	
	for (var m = 0; m < graph_channels; m++)
	{
		var path = "";
		var x = 0;
		var	y = 0;
		var point_gap = graph_width / graph_points;
		graphData[m] = graphData[m] || [];
		for (var i = 0; i < graph_points; i++)
		{
			graphData[m][i] = array[m][i];
			if (i) {
				x += point_gap;
				if (x > graph_width)
					x = graph_width;
				y = graph_height - graphData[m][i];
				path += "," + [x, y];
			} else {
				path += "M" + [0, (y = graph_height - graphData[m][i])] + graph_interpolation;
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
	var x = 0;
	var	y = 0;
	var point_gap = graph_width / graph_points;
	graphData[j] = graphData[j] || [];
	
	for (var i = 0; i < length; i++)
	{
		graphData[j][i] = 0;
		if (i) {
			x += point_gap;
			y = graph_height - graphData[j][i];
			path += "," + [x, y];
		} else {
			path += "M" + [0, (y = graph_height - graphData[j][i])] + graph_interpolation;
		}
	}
	return path;
}

function graph_update_drawing()
{
	if (currentIndex == graph_channels)
		currentIndex = 0;
	if (currentIndex == -1)
		currentIndex = graph_channels-1;
	
	c.attr({path: values[currentIndex], stroke: clr[currentIndex]});	
	bg.attr({path: values[currentIndex] + "L"+graph_width+","+graph_height+" 0,"+graph_height+"z", fill: clr[currentIndex]});
	label.attr({text: labels[currentIndex]});
	
	/* Animated
	var time = 100;
	var anim = Raphael.animation({path: values[currentIndex], stroke: clr[currentIndex]}, time, "<>");
	c.animate(anim);
	bg.animateWith(c, anim, {path: values[currentIndex] + "L"+graph_width+","+graph_height+" 0,"+graph_height+"z", fill: clr[currentIndex]}, time, "<>");
	*/
};