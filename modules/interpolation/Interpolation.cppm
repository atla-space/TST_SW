module;

#include <span>

export module Interpolation;
// Interface

export template<typename In, typename Out = In>
class Interpolation {
public:
	struct CalibrationPoint {
		In  input;
		Out output;
	};

	virtual auto setCalibration(std::span<CalibrationPoint> const& calibration) -> void = 0;
	virtual auto interpolate(In input) const -> Out                                     = 0;
};