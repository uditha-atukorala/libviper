#include "value.h"

#include <array>
#include <cstring>
#include <string>

namespace viper {
value::value() : _node(nullptr), _value(std::nullopt) {}

value::value(const char *s) : _node(nullptr), _value(s) {}

value::value(const char *s, std::size_t count) : _node(nullptr), _value(std::in_place, s, count) {}

value::value(node_t node) : _node(node), _value(std::nullopt) {}

value::value(std::nullopt_t null) : _node(nullptr), _value(null) {}

std::optional<std::string_view> value::data() const noexcept {
	if (!_value && _node != nullptr && _node.is_keyval()) {
		auto v = _node.val();
		_value = {v.data(), v.size()};
	}

	return _value;
}

template <> bool value::get() const noexcept {
	auto d = data();
	if (!d) {
		return false;
	}

	const std::array<const char *, 11> vals = {
		"y",
		"Y",
		"yes",
		"Yes",
		"YES",
		"true",
		"True",
		"TRUE",
		"on",
		"On",
		"ON",
	};

	for (const auto &v : vals) {
		if (std::strcmp(v, d->data()) == 0) {
			return true;
		}
	}

	// No truthy value found, return false
	return false;
}

template <> long value::get() const noexcept {
	auto s = str();
	if (s.empty()) {
		return 0;
	}

	// Note: errors are ignored here due to noexcept
	return std::strtol(s.data(), nullptr, 0);
}

template <> std::string value::get() const noexcept {
	auto s = str();
	return std::string(s.data(), s.size());
}

template <> std::string_view value::get() const noexcept {
	return str();
}

std::string_view value::str() const noexcept {
	auto d = data();
	if (!d) {
		return std::string_view{};
	}

	return d.value();
}
} // namespace viper
