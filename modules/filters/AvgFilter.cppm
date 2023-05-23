module;

#include <cstdint>
#include <vector>

export module Filter.Average;
export import Filter;

export template<typename T>
class AvgFilter : public Filter<T> {
public:
	AvgFilter(uint32_t size) {
		setWindowSize(size);
	}

	auto operator()(T value) -> void override {
		_sum -= _values[_tail];
		_sum += value;

		if (_head == _tail) {
			_tail = (_tail + 1) % _values.size();
		}

		_values[_head] = value;
		_head          = (_head + 1) % _values.size();

		_lastValue = _sum / _values.size();
		Filter<T>::signalValue(_lastValue);
	}

	auto value() const -> T override {
		return _lastValue;
	}

	auto setWindowSize(uint32_t size) -> void {
		_values.resize(size);
	}

private:
	T              _lastValue{};
	std::vector<T> _values;
	T              _sum{0};
	uint32_t       _head{1};
	uint32_t       _tail{0};
};
