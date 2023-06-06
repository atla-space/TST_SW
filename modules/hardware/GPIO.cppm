module;

#include <chrono>
#include <cstdint>
#include <expected>
#include <span>

export module Hardware.GPIO;

export class GPIO {
public:
	using pin_no = uint32_t;

	GPIO(pin_no pin);
	~GPIO();

	// Movable
	GPIO(GPIO&& other) noexcept;
	auto operator=(GPIO&& other) noexcept -> GPIO&;

	// But not copyable
	GPIO(const GPIO&)                    = delete;
	auto operator=(const GPIO&) -> GPIO& = delete;

	enum class Pull { up, down, none };
	enum class Direction { in, out };

	auto setPull(Pull pull) -> void;
	auto setDirection(Direction direction) -> void;

	[[nodiscard]] auto read() const -> bool;
	auto               write(bool value) const -> void;

	template<typename Rep, typename Period>
	static auto busyWait(std::chrono::duration<Rep, Period> duration) -> void {
		auto start = std::chrono::steady_clock::now();
		while (std::chrono::steady_clock::now() < start + duration) {
		}
	}

private:
	pin_no    _pin{};
	Direction _direction{Direction::in};
	Pull      _pull{Pull::none};
};
