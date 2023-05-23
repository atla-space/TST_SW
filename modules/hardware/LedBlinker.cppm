module;

#include <thread>

export module Hardware.LedBlinker;

export import Hardware.GPIO;

export class LedBlinker {
public:
	LedBlinker(GPIO::pin_no pin);

	void start();
	void stop();

	enum class Pattern {
		Slow,
		Quick,
		VeryQuick,
	};

	void setPattern(Pattern pattern);

private:
	GPIO         _led;
	std::jthread _thread;
	bool         _lastState{false};
	Pattern      _pattern{Pattern::Slow};
};
