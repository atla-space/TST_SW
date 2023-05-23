#pragma once

#include "Model.hpp"
#include <CivetServer.h>
#include <atomic>
#include <memory>
#include <simple-yaml/simple_yaml.hpp>
#include <span>
#include <string>
#include <thread>

import API;
import Hardware.GPIO;
import Hardware.HX711;
import Hardware.LedBlinker;
import Filter.Average;
import Interpolation.LinearInterpolation;

class App {
public:
	struct Config : simple_yaml::Simple {
		using Simple::Simple;

		struct HX711 : simple_yaml::Simple {
			using Simple::Simple;

			GPIO::pin_no         clkpin  = bound("clkpin");
			GPIO::pin_no         dtpin   = bound("dtpin");
			::HX711::ChannelType channel = bound("channel", ::HX711::ChannelType::B32);
		};

		struct Led : simple_yaml::Simple {
			using Simple::Simple;

			GPIO::pin_no pin = bound("pin");
		};

		Model::Config db    = bound("database");
		HX711         hx711 = bound("hx711");
		std::string   bind  = bound("bind", "0.0.0.0:8080");
		Led           led   = bound("led");
	};

	App(std::span<const std::string> args);
	~App();

	auto start() -> void;
	auto stop() -> void;
	auto startMeasuring() -> void;
	auto stopMeasuring() -> void;
	auto startFastMeasuring() -> void;
	auto stopFastMeasuring() -> void;
	auto startRecording() -> void;
	auto stopRecording() -> void;

	/* Raw value measured */
	auto valueMeasured(int32_t value) -> void;
	/* Moving average filtered */
	auto valueFiltered(int32_t value) -> void;
	/* Value after calibration computation */
	auto valueCalibrated(double value) -> void;

private:
	auto loadCalibration() -> void;

private:
	std::unique_ptr<Config>      _config;
	std::shared_ptr<CivetServer> _server;
	std::unique_ptr<API>         _api;
	std::unique_ptr<HX711>       _hx711;
	std::unique_ptr<Model>       _model;
	std::unique_ptr<LedBlinker>  _led;

	std::jthread                         _measuring_thread;
	std::atomic_bool                     _fastMeasuring{false};
	std::atomic_bool                     _recording{false};
	AvgFilter<int64_t>                   _filter{5};
	LinearInterpolation<int64_t, double> _interpolation;
	Calibration                          _calibration;
	Experiment                           _experiment;
};
