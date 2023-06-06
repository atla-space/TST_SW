module;
import Hardware.GPIO;

#include <array>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <magic_enum.hpp>
#include <poll.h>
#include <span>
#include <string>
#include <unistd.h>

#if __has_include(<wiringPi.h>)
#	include <wiringPi.h>
#else
static const constexpr int PUD_UP{0};
static const constexpr int PUD_DOWN{1};
static const constexpr int PUD_OFF{2};
static const constexpr int INPUT{0};
static const constexpr int OUTPUT{1};
static const constexpr int HIGH{1};
static const constexpr int LOW{0};

auto digitalRead([[maybe_unused]] int pin) -> int {
	return 0;
}
auto digitalWrite([[maybe_unused]] int pin, [[maybe_unused]] int value) -> void {
}
auto pinMode([[maybe_unused]] int pin, [[maybe_unused]] int direction) -> void {
}
auto pullUpDnControl([[maybe_unused]] int pin, [[maybe_unused]] int pull) -> void {
}
#endif

module Hardware.GPIO;

namespace {
[[nodiscard]] constexpr auto pullMap(GPIO::Pull pull) -> int {
	switch (pull) {
	case GPIO::Pull::up:
		return PUD_UP;
	case GPIO::Pull::down:
		return PUD_DOWN;
	case GPIO::Pull::none:
		return PUD_OFF;
	}
	assert(false); // "Invalid pull"
	return PUD_OFF;
}

[[nodiscard]] constexpr auto directionMap(GPIO::Direction direction) -> int {
	switch (direction) {
	case GPIO::Direction::in:
		return INPUT;
	case GPIO::Direction::out:
		return OUTPUT;
	}
	assert(false); // "Invalid direction"
	return INPUT;
}
} // namespace

GPIO::GPIO(pin_no pin) : _pin{pin} {
	_direction = Direction::out;
	setDirection(Direction::in);
}

GPIO::~GPIO() {
}

GPIO::GPIO(GPIO&& other) noexcept : _pin{other._pin}, _direction{other._direction} {
	other._pin = 0;
}

auto GPIO::operator=(GPIO&& other) noexcept -> GPIO& {
	_pin       = other._pin;
	_direction = other._direction;
	_pull      = other._pull;
	other._pin = 0;
	return *this;
}

auto GPIO::setPull(Pull pull) -> void {
	if (_pull == pull) {
		return;
	}

	pullUpDnControl(_pin, pullMap(pull));
	_pull = pull;
}

auto GPIO::setDirection(Direction direction) -> void {
	if (_direction == direction) {
		return;
	}

	pinMode(_pin, directionMap(direction));
	_direction = direction;
}

auto GPIO::read() const -> bool {
	return digitalRead(_pin) == HIGH;
}

auto GPIO::write(bool value) const -> void {
	digitalWrite(_pin, value ? HIGH : LOW);
}
