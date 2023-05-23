
import { Api } from "./api.js";

var globalChart = null;
var currentExperiment = null;

function refreshExperimentsList() {
	Api.getExperiments().then(function(response) {
		var experiments = response.data;
		var list = document.getElementById('experiments-list');
		list.innerHTML = '';
		experiments.forEach(function(experiment) {
			var li = document.createElement('div');
			li.innerHTML = experiment.name;
			li.attributes['data-id'] = experiment.id;
			list.appendChild(li);
		});

		document.querySelectorAll('#experiments-list div').forEach(function(element) {
			element.addEventListener('click', function(event) {
				currentExperiment = event.target.attributes['data-id'];
				loadMeasurements(currentExperiment);
			});
		});
	});
}

function loadMeasurements(experiment) {
	Api.loadMeasurements(experiment).then(function(response) {
		const ctx = document.getElementById('chart');
		if (globalChart) {
			globalChart.destroy();
		}

		const x_data = response.data.map(function(point) {
			return point.timestamp;
		});
		const y_data = response.data.map(function(point) {
			return point.value;
		});

		const data = {
			labels: x_data,
			datasets: [{
				type: 'line',
				label: 'Experiment',
				data: y_data
			}]
		};

		globalChart = new Chart(ctx, {
			data: data,
			options: {
				plugins: {
					zoom: {
						pan: {
							enabled: true,
						},
						limits: {
						// axis limits
						},
						zoom: {
							wheel: {
								enabled: true,
							},
							pinch: {
								enabled: false
							},
							drag: {
								enabled: false
							},
						}
					}
				}
			}
		});
	});
}

function loadCalibration(){
	Api.loadCalibration().then(function(response) {
		const ctx = document.getElementById('chart');
		if (globalChart) {
			globalChart.destroy();
		}

		const points = response.data.points.map(function(point) {
			return {
				x: point.measured,
				y: point.value
			};
		});

		console.log(points);

		const minX = Math.min.apply(null, points.map(function(point) {
			return point.x;
		}));
		const maxX = Math.max.apply(null, points.map(function(point) {
			return point.x;
		}));

		const interpolate = function(x) {
			return response.data.slope * x + response.data.offset;
		};

		const data = {
			datasets: [{
				type: 'scatter',
				label: 'Last Calibration',
				data: points
			}, {
				type: 'line',
				label: 'Interpolation',
				data: [{
					x: minX - (maxX - minX) * 0.3,
					y: interpolate(minX - (maxX - minX) * 0.3)
				}, {
					x: maxX + (maxX - minX) * 0.3,
					y: interpolate(maxX + (maxX - minX) * 0.3)
				}]
			}]
		};

		globalChart = new Chart(ctx, {
			data: data,
			options: {
				plugins: {
					zoom: {
						pan: {
							enabled: true,
						},
						limits: {
						// axis limits
						},
						zoom: {
							wheel: {
								enabled: true,
							},
							pinch: {
								enabled: false
							},
							drag: {
								enabled: false
							},
						}
					}
				}
			}
		});
	});
}

function refreshCalibrationsList() {
	Api.getCalibrations().then(function(response) {
		var calibrations = response.data;
		var list = document.getElementById('calibrations-list');
		list.innerHTML = '';
		calibrations.forEach(function(calibration) {
			var li = document.createElement('div');
			li.innerHTML = calibration.timestamp;
			li.attributes['data-id'] = calibration.id;
			list.appendChild(li);
		});
	});

	document.getElementById('btnLoadCalibration').addEventListener('click', function() {
		loadCalibration();
	});
}

document.getElementById('btnAdd').addEventListener('click', function() {
	var name = prompt('Enter experiment name');
	if (name) {
		Api.createExperiment(name).then(function(response) {
			refreshExperimentsList();
		});
	}
});

document.getElementById('btnAddCalibration').addEventListener('click', function() {
	Api.createCalibration().then(function(response) {
		refreshCalibrationsList();
	});
});

document.getElementById('btnAddCalibrationPoint').addEventListener('click', function() {
	var weight = prompt('Enter weight [kg]');
	if (weight) {
		Api.createCalibrationPoint(weight).then(function(response) {
			loadCalibration();
		});
	}
});

var dataRefresher = null;

document.getElementById('btnStart').addEventListener('click', function() {
	Api.startMeasuring().then(function(response) {
		console.log(response);
		dataRefresher = setInterval(function() {
			loadMeasurements(currentExperiment);
		}, 1000);
	});
});

document.getElementById('btnStop').addEventListener('click', function() {
	Api.stopMeasuring().then(function(response) {
		console.log(response);
		clearInterval(dataRefresher);
		dataRefresher = null;
	});
});

document.getElementById('btnRefresh').addEventListener('click', function() {
	refreshExperimentsList();
	refreshCalibrationsList();
});
refreshExperimentsList();
refreshCalibrationsList();

function updateValue() {
	Api.getValue().then(function(response) {
		const label = response.data.value + ' / ' + response.data.weight.toFixed(3) + ' kg';
		document.getElementById('value').innerHTML = label;
	});
}
setInterval(updateValue, 1000);
