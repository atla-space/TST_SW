module;

#include <fmt/format.h>
#include <soci/soci.h>
#include <string>

export module Model.CalibrationPoint;

export struct CalibrationPoint {
	int    id{};
	int    calibration_id{};
	double measured{};
	double value{};

	constexpr std::string toJson() const {
		return fmt::format(R"({{"calibration_id": {}, "measured": {}, "value": {}}})", calibration_id, measured, value);
	}
};

namespace soci {
export template<>
struct type_conversion<CalibrationPoint> {
	typedef values base_type;

	static void from_base(values const& v, indicator /* ind */, CalibrationPoint& p) {
		p.id             = v.get<int>("id");
		p.calibration_id = v.get<int>("calibration_id");
		p.measured       = v.get<double>("measured");
		p.value          = v.get<double>("value");
	}

	static void to_base(const CalibrationPoint& p, values& v, indicator& ind) {
		v.set("id", p.id);
		v.set("calibration_id", p.calibration_id);
		v.set("measured", p.measured);
		v.set("value", p.value);
		ind = i_ok;
	}
};
} // namespace soci