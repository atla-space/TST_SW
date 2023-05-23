module;

export module Filter;
export import Signal;

export template<typename T>
class Filter {
public:
	virtual auto operator()(T value) -> void = 0;
	virtual auto value() const -> T          = 0;

	Zeeno::Signal<T> signalValue;
};
