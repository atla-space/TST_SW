module;

#include <thread>

export module Hardware.HX711;

export import Signal;
export import Hardware.GPIO;

export class HX711 {
public:
	HX711(GPIO&& dout, GPIO&& pd_sck);
	~HX711();

	enum class ChannelType { A128, B32, A64, Unknown };

	auto read() -> void;
	auto setChannelNext(ChannelType channel) -> void;

	auto detach() -> void;
	auto join() -> void;

	Zeeno::Signal<int32_t> signalChannelA128Read;
	Zeeno::Signal<int32_t> signalChannelB32Read;
	Zeeno::Signal<int32_t> signalChannelA64Read;

private:
	GPIO _dout;
	GPIO _pd_sck;

	ChannelType _currentChannel{ChannelType::Unknown};
	ChannelType _nextChannel{ChannelType::Unknown};

	std::jthread _worker;
};
