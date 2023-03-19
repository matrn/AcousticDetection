const TIME_TO_HIDE_VALUE = 2000;
const BARPOLAR_WIDTH = 10;

const margin = 20;
const plot_layout = {
	margin: {
		l: margin,
		r: margin,
		b: margin,
		t: margin,
		pad: 10
	},
	polar: {
		// docs: https://plotly.com/javascript/reference/layout/polar/
		radialaxis: {
			visible: false,
		},
		angularaxis: {
			rotation: -90,
			dtick: 10,
			//direction: "counterclockwise"
		},
		sector: [-180, 0],
	},

	showlegend: false

};



function getRndInteger(min, max) {
	return Math.floor(Math.random() * (max - min)) + min;
}

class DataUI {
	constructor(angle_value_callback) {
		this.angle_value_callback = angle_value_callback;   // for showing angle value
		this.slots = {};
		this.data = [
			{ type: "barpolar" }   // to keep chart in polar type, without data and this line chart will be broken
		];

		Plotly.newPlot('chart', this.data, plot_layout, { staticPlot: true, responsive: true });

		this.show_angle(0);
	}


	_create_data(angle) {
		return {
			// docs: https://plotly.com/javascript/reference/barpolar/#barpolar
			type: "barpolar",
			//mode: "lines",
			r: [0, 1],
			theta: [0, angle],//-10, angle+10],
			width: BARPOLAR_WIDTH,
			fill: "toself",
			//fillcolor: color,
			opacity: 0.7,
			// line: {
			// 	color: 'black'
			// }
		}
	}


	_export_and_redraw() {
		let new_data = [{ type: "barpolar" }];      // type: "barpolar" is there to keep chart in polar type, without data and this line chart will be broken

		for (let key of Object.keys(this.slots)) new_data.push(this._create_data(key));
		//console.log(new_data);
		this.data.length = 0;
		this.data.push.apply(this.data, new_data);
		//console.log(this.data);
		Plotly.redraw('chart');
	}


	show_angle(angle) {
		if (angle < -90 || angle > 90 || isNaN(angle)) return;
		
		this.angle_value_callback(angle + "°");
		setTimeout(() => {
			this.angle_value_callback("-");
		}, TIME_TO_HIDE_VALUE);

		if (this.slots.hasOwnProperty(angle)) {
			clearTimeout(this.slots[angle].timer);
		}
		this.slots[angle] = {
			'timer': setTimeout(() => {
				delete this.slots[angle];
				this._export_and_redraw();
			}, TIME_TO_HIDE_VALUE)
		};
		this._export_and_redraw();
	}
}

// setInterval(()=>{
	// 	show_angle(getRndInteger(-90, 90));
	// }, 500);