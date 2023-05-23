module;

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <soci/soci.h>
#include <string>

export module Model.Calibration;

export struct Calibration {
	int     id{};
	std::tm timestamp;

	constexpr std::string toJson() const {
		return fmt::format(R"({{"id": {}, "timestamp": "{:%Y-%m-%d %H:%M:%S}"}})", id, timestamp);
	}
};

namespace soci {
export template<>
struct type_conversion<Calibration> {
	typedef values base_type;

	static void from_base(values const& v, indicator /* ind */, Calibration& p) {
		p.id        = v.get<int>("id");
		p.timestamp = v.get<std::tm>("timestamp");
	}

	static void to_base(const Calibration& p, values& v, indicator& ind) {
		v.set("id", p.id);
		v.set("timestamp", p.timestamp);
		ind = i_ok;
	}
};
} // namespace soci
