module;

#include <functional>

export module Defer;

export class Defer {
public:
	explicit Defer(std::function<void()> f) : f(f) {
	}
	~Defer() {
		f();
	}

private:
	std::function<void()> f;
};
