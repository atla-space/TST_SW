#pragma once

#include <mutex>
#include <simple-yaml/simple_yaml.hpp>
#include <soci/session.h>
#include <string>

import Connection;
import Model.Calibration;
import Model.CalibrationPoint;
import Model.Experiment;
import Signal;

class Model {
public:
	struct Config : simple_yaml::Simple {
		using Simple::Simple;

		std::string host = bound("host", "localhost");
		std::string name = bound("name", "test");
		std::string user = bound("user");
		std::string pass = bound("password");
	};

	Model(Config config);

	auto createExperiment(std::string name) -> Experiment;
	auto listExperiments() -> std::vector<Experiment>;
	auto lastExperiment() -> Experiment;
	auto getExperiment(int id) -> Experiment;

	auto hasCalibration() -> bool;
	auto createCalibration() -> Calibration;
	auto lastCalibration() -> Calibration;
	auto listCalibrations() -> std::vector<Calibration>;

	auto addCalibrationPoint(Calibration, CalibrationPoint) -> void;
	auto listCalibrationPoints(Calibration) -> std::vector<CalibrationPoint>;

	auto addMeasurement(Experiment, double) -> void;
	auto listMeasurements(Experiment) -> std::vector<std::pair<std::tm, double>>;

	Zeeno::Signal<Experiment>                    signalExperimentCreated;
	Zeeno::Signal<Calibration>                   signalCalibrationCreated;
	Zeeno::Signal<Calibration, CalibrationPoint> signalCalibrationPointAdded;

private:
	std::unique_ptr<soci::session> _db;
	std::recursive_mutex           _mutex;
};
