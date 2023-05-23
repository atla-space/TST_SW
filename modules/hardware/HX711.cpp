module;

#include <chrono>
#include <cstdint>
#include <map>
#include <thread>

import Defer;
import Hardware.HX711;
module Hardware.HX711;

const std::map<HX711::ChannelType, uint8_t> pulsesPerChannel{
    {HX711::ChannelType::A128, 25},
    {HX711::ChannelType::B32, 26},
    {HX711::ChannelType::A64, 27},
};

HX711::HX711(GPIO&& dout, GPIO&& pd_sck) : _dout{std::move(dout)}, _pd_sck{std::move(pd_sck)} {
	_dout.setDirection(GPIO::Direction::in);
	_pd_sck.setDirection(GPIO::Direction::out);

	_pd_sck.write(false);
}

HX711::~HX711() {
}

auto HX711::read() -> void {
	// Wait for the HX711 to become ready
	_pd_sck.write(false);
	auto started = std::chrono::steady_clock::now();
	while (_dout.read() && std::chrono::steady_clock::now() < started + std::chrono::milliseconds{100}) {
		std::this_thread::sleep_for(std::chrono::milliseconds{1});
	}

	if (std::chrono::steady_clock::now() >= started + std::chrono::milliseconds{100}) {
		return;
	}

	// Read the 24-bit value
	constexpr uint8_t bits{24};
	int32_t           value{0};
	for (int i = 0; i < bits; ++i) {
		_pd_sck.write(true);
		GPIO::busyWait(std::chrono::microseconds{1});
		value <<= 1;
		value |= _dout.read() ? 1 : 0;
		_pd_sck.write(false);
		GPIO::busyWait(std::chrono::microseconds{1});
	}

	const size_t pulses = pulsesPerChannel.at(_nextChannel) - bits;

	for (size_t i = 0; i < pulses; ++i) {
		_pd_sck.write(true);
		GPIO::busyWait(std::chrono::microseconds{1});
		_pd_sck.write(false);
		GPIO::busyWait(std::chrono::microseconds{1});
	}
	std::this_thread::sleep_for(std::chrono::microseconds{60});
	Defer defer{[this] { _currentChannel = _nextChannel; }};

	// The 24-bit value is a two's complement number
	if (value & 0x800000) {
		value |= 0xFF000000;
	}

	if (value == -1) {
		// Sometimes it just shows something like -1
		return;
	}

	switch (_currentChannel) {
		using enum ChannelType;
	case A128:
		signalChannelA128Read(value);
		break;
	case B32:
		signalChannelB32Read(value);
		break;
	case A64:
		signalChannelA64Read(value);
		break;
	case Unknown:
		break;
	}
}

auto HX711::setChannelNext(ChannelType channel) -> void {
	_nextChannel = channel;
}

auto HX711::detach() -> void {
	_worker = std::jthread{[this](std::stop_token token) {
		while (!token.stop_requested()) {
			read();
		}
	}};
}

auto HX711::join() -> void {
	_worker.request_stop();
}