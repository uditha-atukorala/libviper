#pragma once

#include <iterator>
#include <optional>
#include <string_view>

#include <ryml.hpp>

namespace viper {
class value {
public:
	/// Represents a value node
	using node_t = ryml::NodeRef;

	/// Represents value data
	using data_t = std::optional<std::string_view>;

	struct iterator {
	public:
		using iterator_category = std::forward_iterator_tag;

		iterator(const node_t &node) :
			_node(node.valid() && node.has_children() ? node.first_child() : node) {}

		value operator*() const { return value(_node); }

		iterator &operator++() {
			_node = _node.next_sibling();
			return *this;
		}

		bool operator!=(const iterator &that) const {
			if (that._node == nullptr) {
				return _node != nullptr;
			}

			if (_node == nullptr) {
				return true; // _node != that._node (!nullptr)
			}

			return (_node != that._node);
		}

	private:
		node_t _node;
	};

	value();
	value(const char *s);
	value(const char *s, std::size_t size);
	value(node_t node);
	value(std::nullopt_t null);

	constexpr explicit operator bool() const noexcept { return _data || (_node != nullptr); }

	template <typename T> explicit operator T() const noexcept { return get<T>(); }

	bool operator==(const value &that) const noexcept;

	iterator begin() const noexcept { return iterator(_node); }
	iterator end() const noexcept { return iterator(nullptr); }

	data_t data() const noexcept;

	/**
	 * Retrieve the stored value converted to a given type.
	 *
	 * Specialisations:
	 *   - template <> bool get() const noexcept;
	 *   - template <> double get() const noexcept;
	 *   - template <> float get() const noexcept;
	 *   - template <> long get() const noexcept;
	 *   - template <> std::string get() const noexcept;
	 *   - template <> std::string_view get() const noexcept;
	 */
	template <typename T> T get() const noexcept;

	std::string_view str() const noexcept;

private:
	node_t _node;

	mutable data_t _data;
};
} // namespace viper
