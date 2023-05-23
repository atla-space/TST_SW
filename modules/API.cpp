module;

#include <CivetServer.h>
#include <civetweb.h>
#include <fmt/format.h>
#include <functional>
#include <memory>
#include <range/v3/view/map.hpp>
#include <string>
#include <string_view>

import Connection;
import Model.Experiment;
import Model.Calibration;
import Model.CalibrationPoint;

module API;

class CallbackHandler : public CivetHandler {
public:
	CallbackHandler(std::function<bool(Connection)> callback) : _callback{callback} {
	}

	auto handleGet(CivetServer*, mg_connection* conn) -> bool override {
		return _callback(Connection{conn});
	}

private:
	std::function<bool(Connection)> _callback;
};

API::API(std::shared_ptr<CivetServer> server) : _server{std::move(server)} {
	addHandler("/api/start-measuring", [this](Connection connection) {
		signalStartMeasuring();
		connection.sendOk();
		return true;
	});

	addHandler("/api/stop-measuring", [this](Connection connection) {
		signalStopMeasuring();
		connection.sendOk();
		return true;
	});

	addHandler("/api/create-experiment", [this](Connection connection) {
		auto name = connection.getQueryParameter("name");
		if (!name) {
			connection.sendBadRequest();
			return true;
		}

		signalCreateExperiment(*name);
		connection.sendOk();
		return true;
	});

	addHandler("/api/load-measurements", [this](Connection connection) {
		auto experiment = connection.getQueryParameter("experiment");
		if (!experiment) {
			connection.sendBadRequest();
			return true;
		}

		if (!dataLoader) {
			connection.sendBadRequest();
			return true;
		}

		auto        data = dataLoader(std::stoi(*experiment));
		std::string content{"["};
		for (const auto& [time, value] : data) {
			content += fmt::format(R"({{"timestamp": "{:%Y-%m-%d %H:%M:%S}", "value": {} }})", time, value);
			content += ",";
		}
		if (content.back() == ',') {
			content.pop_back();
		}
		content += "]";
		connection.sendOk("application/json", content.size());
		connection.write(content);
		return true;
	});

	addHandler("/api/list-experiments", [this](Connection connection) {
		std::string content{"["};
		for (const auto& e : _experiments) {
			content += e.toJson();
			content += ",";
		}
		if (content.back() == ',') {
			content.pop_back();
		}
		content += "]";
		connection.sendOk("application/json", content.size());
		connection.write(content);
		return true;
	});

	addHandler("/api/list-calibrations", [this](Connection connection) {
		std::string content{"["};
		for (const auto& c : _calibrations) {
			content += c.toJson();
			content += ",";
		}
		if (content.back() == ',') {
			content.pop_back();
		}
		content += "]";
		connection.sendOk("application/json", content.size());
		connection.write(content);
		return true;
	});

	addHandler("/api/value", [this](Connection connection) {
		const double      weight = _slope.load() * _value.load() + _offset.load();
		const std::string jsonContent{
		    fmt::format(R"({{"value": {}, "weight": {}, "slope": {}, "offset": {} }})", _value.load(), weight, _slope.load(), _offset.load())};
		connection.sendOk("application/json", jsonContent.size());
		connection.write(jsonContent);
		return true;
	});

	addHandler("/api/start-calibration", [this](Connection connection) {
		signalStartCalibration();
		connection.sendOk();
		return true;
	});

	addHandler("/api/calibration-point", [this](Connection connection) {
		auto value = connection.getQueryParameter("value");
		if (!value) {
			connection.sendBadRequest();
			return true;
		}

		signalCalibrationPoint(std::stod(*value));
		connection.sendOk();
		return true;
	});

	addHandler("/api/load-calibration", [this](Connection connection) {
		std::string points{"["};
		for (const auto& p : _calibrationPoints) {
			points += p.toJson();
			points += ",";
		}
		if (points.back() == ',') {
			points.pop_back();
		}
		points += "]";

		std::string content{fmt::format(R"({{"points": {}, "slope": {}, "offset": {} }})", points, _slope.load(), _offset.load())};

		connection.sendOk("application/json", content.size());
		connection.write(content);
		return true;
	});

	for (auto& [path, handler] : _handlers) {
		_server->addHandler(path, handler.get());
	}
}

API::~API() {
	for (const auto& path : _handlers | ranges::views::keys) {
		_server->removeHandler(path);
	}
}

auto API::addHandler(std::string_view path, std::unique_ptr<CivetHandler> handler) -> void {
	_handlers.emplace(path, std::move(handler));
}

auto API::addHandler(std::string_view path, std::function<bool(Connection)> callback) -> void {
	addHandler(path, std::make_unique<CallbackHandler>(callback));
}

auto API::setExperiments(std::vector<Experiment> experiments) -> void {
	_experiments = std::move(experiments);
}

auto API::setValue(int32_t value) -> void {
	_value = value;
}

auto API::setCalibrations(std::vector<Calibration> calibrations) -> void {
	_calibrations = std::move(calibrations);
}

auto API::setCalibrationPoints(std::vector<CalibrationPoint> points) -> void {
	_calibrationPoints = std::move(points);
}

auto API::setInterpolationParams(double slope, double offset) -> void {
	_slope  = slope;
	_offset = offset;
}
