module;

#include <fmt/format.h>
#include <soci/soci.h>
#include <string>

export module Model.Experiment;

export struct Experiment {
	int         id{};
	std::string name;

	constexpr std::string toJson() const {
		return fmt::format(R"({{ "name": "{}","id": {} }})", name, id);
	}
};

namespace soci {
export template<>
struct type_conversion<Experiment> {
	typedef values base_type;

	static void from_base(values const& v, indicator /* ind */, Experiment& p) {
		p.id   = v.get<int>("id");
		p.name = v.get<std::string>("name");
	}

	static void to_base(const Experiment& p, values& v, indicator& ind) {
		v.set("id", p.id);
		v.set("name", p.name);
		ind = i_ok;
	}
};
} // namespace soci