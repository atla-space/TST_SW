module;

#include <chrono>
#include <thread>

import Hardware.LedBlinker;

module Hardware.LedBlinker;

LedBlinker::LedBlinker(GPIO::pin_no pin) : _led{pin} {
	_led.setDirection(GPIO::Direction::out);
	setPattern(Pattern::Slow);
}

void LedBlinker::start() {
	_thread = std::jthread([this](std::stop_token stoken) {
		while (!stoken.stop_requested()) {
			switch (_pattern) {
			case Pattern::Slow:
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
				_led.write(!_lastState);
				break;
			case Pattern::Quick:
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				_led.write(!_lastState);
				break;
			case Pattern::VeryQuick:
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				_led.write(!_lastState);
				break;
			}
			_lastState = !_lastState;
		}
	});
}

void LedBlinker::stop() {
	_thread.request_stop();
}

void LedBlinker::setPattern(Pattern pattern) {
	_pattern = pattern;
}
