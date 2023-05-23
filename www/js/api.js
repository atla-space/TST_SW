
import "./axios.min.js";

// REST Api object
export var Api = {
	// Base URL
	baseUrl: 'http://' + window.location.hostname + ':8080/api',

	// Start the measurement
	startMeasuring: function() {
		return axios.get(this.baseUrl + '/start-measuring');
	},

	// Stop the measurement
	stopMeasuring: function() {
		return axios.get(this.baseUrl + '/stop-measuring');
	},

	createExperiment: function(experiment) {
		return axios.get(this.baseUrl + '/create-experiment', {
			params: {
				name: experiment
			}
		});
	},

	createCalibration: function() {
		return axios.get(this.baseUrl + '/start-calibration');
	},

	getExperiments: function() {
		return axios.get(this.baseUrl + '/list-experiments');
	},

	getCalibrations: function() {
		return axios.get(this.baseUrl + '/list-calibrations');
	},

	loadCalibration: function() {
		return axios.get(this.baseUrl + '/load-calibration');
	},

	createCalibrationPoint: function(weight) {
		return axios.get(this.baseUrl + '/calibration-point', {
			params: {
				value: weight
			}
		});
	},

	loadMeasurements: function(experiment) {
		console.log('Loading measurements for experiment ' + experiment);
		return axios.get(this.baseUrl + '/load-measurements', {
			params: {
				experiment: experiment
			}
		});
	},


	getValue: function() {
		return axios.get(this.baseUrl + '/value');
	}

};
