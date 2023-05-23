module;

#include <CivetServer.h>
#include <civetweb.h>
#include <fmt/format.h>
#include <limits>
#include <optional>
#include <span>
#include <variant>

import Connection;
module Connection;

Connection::Connection(mg_connection* conn) : m_conn{conn} {
}

auto Connection::write(std::u8string_view data) -> std::variant<BytesWritten, ConnectionClosed, std::string> {
	return write(std::span{reinterpret_cast<const std::byte*>(data.data()), data.size()});
}

auto Connection::write(std::string_view data) -> std::variant<BytesWritten, ConnectionClosed, std::string> {
	return write(std::span{reinterpret_cast<const std::byte*>(data.data()), data.size()});
}

auto Connection::write(std::span<const std::byte> data) -> std::variant<BytesWritten, ConnectionClosed, std::string> {
	if (data.size() > std::numeric_limits<int>::max()) {
		return fmt::format("Data size {} is too large. Max size is {}", data.size(), std::numeric_limits<int>::max());
	}

	const auto ret = mg_write(m_conn, data.data(), data.size());
	if (ret < 0) {
		return fmt::format("Failed to write to connection: {}", ret);
	}
	if (ret == 0) {
		return ConnectionClosed{};
	}
	return BytesWritten{data.size(), static_cast<size_t>(ret)};
}

auto Connection::sendOk(std::string_view mimetype, uint64_t contentLen) -> std::variant<BytesWritten, ConnectionClosed, std::string> {
	std::string mime_c{mimetype};
	const auto  ret = mg_send_http_ok(m_conn, mime_c.c_str(), contentLen);
	if (ret < 0) {
		return fmt::format("Failed to write to connection: {}", ret);
	}
	if (ret == 0) {
		return ConnectionClosed{};
	}
	return BytesWritten{0, static_cast<size_t>(ret)};
}

auto Connection::sendError(int status, std::string_view reason) -> std::variant<BytesWritten, ConnectionClosed, std::string> {
	const auto ret = mg_send_http_error(m_conn, status, "%s", reason.data());
	if (ret < 0) {
		return fmt::format("Failed to write to connection: {}", ret);
	}
	if (ret == 0) {
		return ConnectionClosed{};
	}
	return BytesWritten{0, static_cast<size_t>(ret)};
}

auto Connection::getQueryParameter(std::string_view name) -> std::optional<std::string> {
	const auto query = mg_get_request_info(m_conn)->query_string;
	if (!query) {
		return std::nullopt;
	}

	const auto query_str = std::string_view{query};
	const auto pos       = query_str.find(name);
	if (pos == std::string_view::npos) {
		return std::nullopt;
	}

	const auto start = pos + name.size() + 1;
	const auto end   = query_str.find('&', start);
	return std::string{query_str.substr(start, end - start)};
}
