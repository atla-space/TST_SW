module;

#include <CivetServer.h>
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <span>
#include <variant>

export module API;

import Signal;
import Connection;
import Model.Experiment;
import Model.Calibration;
import Model.CalibrationPoint;

export class API {
public:
	API(std::shared_ptr<CivetServer> server);
	~API();

	static auto
	check(decltype(std::declval<Connection>().write(std::span<std::byte>{})) ret) {
		std::visit(
		    [](auto&& arg) {
			    using T = std::decay_t<decltype(arg)>;
			    if constexpr (std::is_same_v<T, ConnectionClosed>) {
				    throw ConnectionClosed{};
			    } else if constexpr (std::is_same_v<T, std::string>) {
				    std::cerr << arg << std::endl;
				    throw std::runtime_error{arg};
			    }
		    },
		    ret);
		return ret;
	}

	Zeeno::Signal<>            signalStartMeasuring;
	Zeeno::Signal<>            signalStopMeasuring;
	Zeeno::Signal<std::string> signalCreateExperiment;
	Zeeno::Signal<>            signalStartCalibration;
	Zeeno::Signal<double>      signalCalibrationPoint;

	std::function<std::vector<std::pair<std::tm, double>>(int)> dataLoader;

	auto setExperiments(std::vector<Experiment> experiments) -> void;
	auto setCalibrations(std::vector<Calibration> calibrations) -> void;
	auto setCalibrationPoints(std::vector<CalibrationPoint> points) -> void;
	auto setValue(int32_t value) -> void;
	auto setInterpolationParams(double slope, double offset) -> void;

private:
	void addHandler(std::string_view path, std::unique_ptr<CivetHandler> handler);
	void addHandler(std::string_view path, std::function<bool(Connection)> callback);

private:
	std::shared_ptr<CivetServer>                         _server;
	std::map<std::string, std::unique_ptr<CivetHandler>> _handlers;

	std::vector<Experiment>       _experiments;
	std::vector<Calibration>      _calibrations;
	std::vector<CalibrationPoint> _calibrationPoints;
	std::atomic_int32_t           _value{0};
	std::atomic<double>           _slope{1.0};
	std::atomic<double>           _offset{0.0};
};
