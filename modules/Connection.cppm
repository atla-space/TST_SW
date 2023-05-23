module;

#include <CivetServer.h>
#include <fmt/format.h>
#include <optional>
#include <span>
#include <variant>

export module Connection;

export struct ConnectionClosed {};
export struct BytesWritten {
	size_t bytesTotal{};
	size_t bytesWritten{};
};

export class Connection {
public:
	Connection(mg_connection* conn);

	auto write(std::span<const std::byte> data) -> std::variant<BytesWritten, ConnectionClosed, std::string>;
	auto write(std::u8string_view data) -> std::variant<BytesWritten, ConnectionClosed, std::string>;
	auto write(std::string_view data) -> std::variant<BytesWritten, ConnectionClosed, std::string>;

	template<typename... Args>
	auto print(Args&&... args) -> std::variant<BytesWritten, ConnectionClosed, std::string> {
		return write(fmt::format(std::forward<Args>(args)...));
	}

	auto sendOk(std::string_view mimetype = "text/plain", uint64_t contentLen = 0) -> std::variant<BytesWritten, ConnectionClosed, std::string>;
	auto sendError(int status, std::string_view reason) -> std::variant<BytesWritten, ConnectionClosed, std::string>;
	auto sendBadRequest() -> std::variant<BytesWritten, ConnectionClosed, std::string> {
		return sendError(400, "Bad Request");
	}

	auto getQueryParameter(std::string_view name) -> std::optional<std::string>;

private:
	mg_connection* m_conn{};
};
