#include "Model.hpp"
#include <fmt/format.h>
#include <iostream>
#include <soci/mysql/soci-mysql.h>
#include <soci/session.h>

template<typename T, typename... Args>
auto fetchCollection(soci::session& db, std::string_view query, Args... params) -> std::vector<T> {
	std::vector<T> collection;
	T              item;

	soci::statement st = ((db.prepare << query, ..., params), soci::into(item));

	st.execute();
	while (st.fetch()) {
		collection.push_back(item);
	}
	return collection;
}

Model::Model(Config config) {
	std::scoped_lock lock{_mutex};
	_db = std::make_unique<soci::session>(soci::mysql, fmt::format("host={} dbname={} user={} password={}", config.host, config.name, config.user, config.pass));

	_db->set_log_stream(&std::clog);

	// check if the table exist and if not, create it
	// experiments, measurements, calibrations, calibration_points
	_db->once << "CREATE TABLE IF NOT EXISTS `calibrations` (`id` INT NOT NULL AUTO_INCREMENT, `timestamp` TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3), "
	             "PRIMARY KEY (`id`));";
	_db->once << "CREATE TABLE IF NOT EXISTS `experiments` (`id` INT NOT NULL AUTO_INCREMENT, `name` VARCHAR(255) NOT NULL, `calibration_id` INT NOT NULL, "
	             "PRIMARY KEY (`id`), FOREIGN KEY(`calibration_id`) REFERENCES `calibrations` (`id`));";
	_db->once
	    << "CREATE TABLE IF NOT EXISTS `measurements` (`id` INT NOT NULL AUTO_INCREMENT, `experiment_id` INT NOT NULL, `value` DOUBLE NOT NULL, `timestamp` "
	       "TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3), PRIMARY KEY (`id`), FOREIGN KEY (`experiment_id`) REFERENCES `experiments` (`id`));";
	_db->once << "CREATE TABLE IF NOT EXISTS `calibration_points` (`id` INT NOT NULL AUTO_INCREMENT, `calibration_id` INT NOT NULL, `measured` DOUBLE NOT "
	             "NULL, `value` DOUBLE NOT NULL, PRIMARY KEY (`id`), FOREIGN KEY (`calibration_id`) REFERENCES `calibrations` (`id`));";
}

auto Model::createExperiment(std::string name) -> Experiment {
	std::scoped_lock lock{_mutex};

	_db->begin();
	int calibration_id{0};
	*_db << "SELECT MAX(`id`) FROM `calibrations`;", soci::into(calibration_id);
	*_db << "INSERT INTO `experiments` (`name`, `calibration_id`) VALUES (:name, :calibration_id);", soci::use(name), soci::use(calibration_id);
	int experiment_id{0};
	*_db << "SELECT LAST_INSERT_ID();", soci::into(experiment_id);
	_db->commit();

	Experiment experiment{experiment_id, name};

	signalExperimentCreated(experiment);

	return experiment;
}

auto Model::listExperiments() -> std::vector<Experiment> {
	std::scoped_lock lock{_mutex};
	return fetchCollection<Experiment>(*_db, "SELECT `id`, `name` FROM `experiments`;");
}

auto Model::lastCalibration() -> Calibration {
	std::scoped_lock lock{_mutex};
	Calibration      calibration;
	*_db << "SELECT `id`, `timestamp` FROM `calibrations` ORDER BY `id` DESC LIMIT 1;", soci::into(calibration.id), soci::into(calibration.timestamp);
	return calibration;
}

auto Model::listCalibrationPoints(Calibration calibration) -> std::vector<CalibrationPoint> {
	std::scoped_lock lock{_mutex};
	return fetchCollection<CalibrationPoint>(
	    *_db,
	    "SELECT `id`, `calibration_id`, `measured`, `value` FROM `calibration_points` WHERE `calibration_id` = :calibration_id;",
	    soci::use(calibration.id));
}

auto Model::hasCalibration() -> bool {
	std::scoped_lock lock{_mutex};
	int              count{0};
	*_db << "SELECT COUNT(*) FROM `calibrations`;", soci::into(count);
	return count > 0;
}

auto Model::createCalibration() -> Calibration {
	std::scoped_lock lock{_mutex};
	_db->begin();
	*_db << "INSERT INTO `calibrations` () VALUES ();";
	Calibration calibration;
	*_db << "SELECT `id`, `timestamp` FROM `calibrations` ORDER BY `id` DESC LIMIT 1;", soci::into(calibration.id), soci::into(calibration.timestamp);
	_db->commit();

	signalCalibrationCreated(calibration);
	return calibration;
}

auto Model::addCalibrationPoint(Calibration calibration, CalibrationPoint point) -> void {
	std::scoped_lock lock{_mutex};
	*_db << "INSERT INTO `calibration_points` (`calibration_id`, `measured`, `value`) VALUES (:calibration_id, :measured, :value);", soci::use(calibration.id),
	    soci::use(point.measured), soci::use(point.value);

	signalCalibrationPointAdded(calibration, point);
}

auto Model::listCalibrations() -> std::vector<Calibration> {
	std::scoped_lock lock{_mutex};
	return fetchCollection<Calibration>(*_db, "SELECT `id`, `timestamp` FROM `calibrations`;");
}

auto Model::addMeasurement(Experiment experiment, double value) -> void {
	std::scoped_lock lock{_mutex};
	*_db << "INSERT INTO `measurements` (`experiment_id`, `value`) VALUES (:experiment_id, :value);", soci::use(experiment.id), soci::use(value);
}

auto Model::lastExperiment() -> Experiment {
	std::scoped_lock lock{_mutex};
	Experiment       experiment;
	*_db << "SELECT * FROM `experiments` ORDER BY `id` DESC LIMIT 1;", soci::into(experiment);
	return experiment;
}

auto Model::getExperiment(int id) -> Experiment {
	std::scoped_lock lock{_mutex};
	Experiment       experiment;
	*_db << "SELECT * FROM `experiments` WHERE `id` = :id;", soci::use(id), soci::into(experiment);
	return experiment;
}

auto Model::listMeasurements(Experiment experiment) -> std::vector<std::pair<std::tm, double>> {
	std::scoped_lock                        lock{_mutex};
	std::vector<std::pair<std::tm, double>> measurements;
	std::pair<std::tm, double>              measurement;

	soci::statement st
	    = (_db->prepare << "SELECT `timestamp`, `value` FROM `measurements` WHERE `experiment_id` = :experiment_id;",
	       soci::use(experiment.id),
	       soci::into(measurement.first),
	       soci::into(measurement.second));

	st.execute();
	while (st.fetch()) {
		measurements.push_back(measurement);
	}
	return measurements;
}