module;

#include <functional>

export module Defer;

export class Defer {
public:
	explicit Defer(std::function<void()> func) : _func{std::move(func)} {
	}
	Defer(const Defer&)                    = delete;
	Defer(Defer&&)                         = delete;
	auto operator=(const Defer&) -> Defer& = delete;
	auto operator=(Defer&&) -> Defer&      = delete;

	~Defer() {
		_func();
	}

private:
	std::function<void()> _func;
};
