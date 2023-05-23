#include <iostream>
#include <string>
#include <wiringPi.h>
#include <wiringpi2/wiringPi.h>

#include "App.hpp"

App::App(std::span<const std::string> args) {
	std::string config_path{"./config.yaml"};
	if (args.size() == 2) {
		config_path = args[1];
	}

	_config = std::make_unique<Config>(simple_yaml::fromFile(config_path));

	std::cout << "Config: " << std::endl;
	std::cout << "  Database: " << std::endl;
	std::cout << "    Host: " << _config->db.host << std::endl;
	std::cout << "    Name: " << _config->db.name << std::endl;
	std::cout << "    User: " << _config->db.user << std::endl;
	std::cout << "    Pass: " << _config->db.pass << std::endl;
	std::cout << "  HX711: " << std::endl;
	std::cout << "    ClkPin: " << _config->hx711.clkpin << std::endl;
	std::cout << "    DtPin: " << _config->hx711.dtpin << std::endl;
	std::cout << "    Channel: " << magic_enum::enum_name(_config->hx711.channel) << std::endl;
	std::cout << "  Bind: " << _config->bind << std::endl;
}

App::~App() {
	stop();
}

auto App::start() -> void {
	_model = std::make_unique<Model>(_config->db);

	std::vector<std::string> cpp_options{"document_root", std::filesystem::path("www").string(), "listening_ports", _config->bind, "num_threads", "1"};
	_server = std::make_shared<CivetServer>(cpp_options);
	_api    = std::make_unique<API>(_server);

	_api->signalStartMeasuring.connect([this]() { startRecording(); });
	_api->signalStopMeasuring.connect([this]() { stopRecording(); });
	_api->signalCreateExperiment.connect([this](std::string name) {
		if (!_model->hasCalibration()) {
			return;
		}
		_experiment = _model->createExperiment(name);
	});

	_led = std::make_unique<LedBlinker>(_config->led.pin);
	_led->start();

	_hx711 = std::make_unique<HX711>(GPIO{_config->hx711.dtpin}, GPIO{_config->hx711.clkpin});
	std::cout << "Setting channel to " << magic_enum::enum_name(_config->hx711.channel) << std::endl;
	_hx711->setChannelNext(_config->hx711.channel);

	_hx711->signalChannelA128Read.connect([this](int32_t value) { valueMeasured(value); });
	_hx711->signalChannelA64Read.connect([this](int32_t value) { valueMeasured(2 * value); });
	_hx711->signalChannelB32Read.connect([](int32_t value) { std::cout << "Channel B32: " << value << std::endl; });
	_filter.signalValue.connect([this](int32_t value) { valueFiltered(value); });

	// Connect model changes with API
	_model->signalExperimentCreated.connect([this](Experiment) { _api->setExperiments(_model->listExperiments()); });
	_api->setExperiments(_model->listExperiments());
	_experiment = _model->lastExperiment();

	_api->dataLoader = [this](int id) -> std::vector<std::pair<std::tm, double>> { return _model->listMeasurements(_model->getExperiment(id)); };

	_model->signalCalibrationCreated.connect([this](Calibration) {
		_api->setCalibrations(_model->listCalibrations());
		loadCalibration();
		_api->setCalibrationPoints(_model->listCalibrationPoints(_calibration));
		_api->setInterpolationParams(_interpolation.slope(), _interpolation.offset());
	});
	_model->signalCalibrationPointAdded.connect([this](Calibration, CalibrationPoint) {
		_api->setCalibrations(_model->listCalibrations());
		loadCalibration();
		_api->setCalibrationPoints(_model->listCalibrationPoints(_calibration));
		_api->setInterpolationParams(_interpolation.slope(), _interpolation.offset());
	});
	_api->setCalibrations(_model->listCalibrations());
	loadCalibration();
	_api->setCalibrationPoints(_model->listCalibrationPoints(_calibration));
	_api->setInterpolationParams(_interpolation.slope(), _interpolation.offset());

	// Calibration
	_api->signalStartCalibration.connect([this]() {
		std::cout << "Start Calibration" << std::endl;
		_calibration = _model->createCalibration();
	});
	_api->signalCalibrationPoint.connect([this](double weight) {
		startFastMeasuring();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		stopFastMeasuring();

		const auto measured = _filter.value();
		std::cout << "Calibration Point: " << measured << " = " << weight << std::endl;
		_model->addCalibrationPoint(_calibration, {.measured = static_cast<double>(measured), .value = weight});
	});

	startMeasuring();
}

auto App::stop() -> void {
	stopMeasuring();
	_hx711.reset();
	_api.reset();
	_server.reset();
}

auto App::startMeasuring() -> void {
	_measuring_thread = std::jthread([this](std::stop_token stoken) {
		while (!stoken.stop_requested()) {
			_hx711->read();
			if (!_fastMeasuring) {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
	});
}

auto App::stopMeasuring() -> void {
	_measuring_thread.request_stop();
	if (_measuring_thread.joinable()) {
		_measuring_thread.join();
	}
}

auto App::valueMeasured(int32_t value) -> void {
	std::cout << "Value: " << value << std::endl;
	if (value < 0) {
		std::cout << "Negative value, ignoring" << std::endl;
		return;
	}
	_filter(value);
}

auto App::valueFiltered(int32_t value) -> void {
	std::cout << "Filtered Value: " << value << std::endl;
	const auto calibrated = _interpolation.interpolate(value);
	valueCalibrated(calibrated);
	_api->setValue(value);
}

auto App::loadCalibration() -> void {
	if (!_model->hasCalibration()) {
		return;
	}

	_calibration      = _model->lastCalibration();
	const auto points = _model->listCalibrationPoints(_calibration);

	std::vector<LinearInterpolation<int64_t, double>::CalibrationPoint> matPoints;
	matPoints.reserve(points.size());
	for (const auto& p : points) {
		matPoints.push_back({static_cast<int64_t>(p.measured), p.value});
	}

	_interpolation.setCalibration(matPoints);
};

auto App::valueCalibrated(double value) -> void {
	std::cout << "Calibrated Value: " << value << std::endl;
	if (_recording) {
		_model->addMeasurement(_experiment, value);
	}
}

auto App::startFastMeasuring() -> void {
	_fastMeasuring = true;
	_led->setPattern(LedBlinker::Pattern::VeryQuick);
}

auto App::stopFastMeasuring() -> void {
	_fastMeasuring = false;
	_led->setPattern(LedBlinker::Pattern::Slow);
}

auto App::startRecording() -> void {
	_recording = true;
	startFastMeasuring();
}

auto App::stopRecording() -> void {
	_recording = false;
	stopFastMeasuring();
}
