module;

#include <algorithm>
#include <numeric>
#include <ranges>
#include <span>

export module Interpolation.LinearInterpolation;
export import Interpolation;

export template<typename In, typename Out = In>
class LinearInterpolation : public Interpolation<In, Out> {
public:
	auto interpolate(In input) const -> Out override {
		return static_cast<Out>(input * _slope + _offset);
	}

	auto setCalibration(std::span<typename Interpolation<In, Out>::CalibrationPoint> const& calibration) -> void override {
		const auto x    = calibration | std::views::transform([](auto const& p) { return p.input; });
		const auto y    = calibration | std::views::transform([](auto const& p) { return p.output; });
		const auto n    = calibration.size();
		const auto s_x  = std::accumulate(x.begin(), x.end(), 0.0);
		const auto s_y  = std::accumulate(y.begin(), y.end(), 0.0);
		const auto s_xx = std::inner_product(x.begin(), x.end(), x.begin(), 0.0);
		const auto s_xy = std::inner_product(x.begin(), x.end(), y.begin(), 0.0);
		_slope          = (n * s_xy - s_x * s_y) / (n * s_xx - s_x * s_x);
		_offset         = (s_y - _slope * s_x) / n;
	}

	auto slope() const -> double {
		return _slope;
	}

	auto offset() const -> double {
		return _offset;
	}

private:
	double _slope{};
	double _offset{};
};