/* functions to resize chart height automatically */
let plot_default_height = undefined;
window.addEventListener("load", (event) => {
	console.log("onload");
	setTimeout(function () {
		plot_default_height = document.getElementById('chart').offsetHeight;
		console.log("plot height: ", plot_default_height);
	}, 1500);
});

function get_body_height() {
	return document.getElementById('chart-card').clientHeight - document.getElementById('chart-card-header').offsetHeight;
}
function recompute_chart_size() {
	if (window.matchMedia("(orientation: portrait)").matches) return;   // protrait
	console.log("Recomputing chart");

	let body_height = get_body_height();
	//console.log(body_height, plot_default_height);
	if (body_height < plot_default_height) {
		document.getElementById("chart-card").style.height = "95vw";
		body_height = get_body_height();
	}

	document.getElementById('chart-card-body').style.height = body_height + "px";

	let chart_height = document.getElementById('chart-card-body').clientHeight - 40;
	document.getElementById('chart').style.height = chart_height + "px";

	Plotly.Plots.resize("chart");
}

//document.getElementById("chart-card").scrollIntoView();
screen.orientation.onchange = function () {
	console.log("ORIENATATION CHANGED");
	recompute_chart_size();
	document.getElementById("chart-card").scrollIntoView();
};

window.addEventListener("load", recompute_chart_size);
window.addEventListener("resize", recompute_chart_size);