#include "config.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

namespace viper {
config::config(const char *name, const char *path) : _name(name), _path(path) {}

config::config(const char *name, std::filesystem::path path) : _name(name), _path(path) {}

config::config(tree_t t) : _tree(t) {}

std::filesystem::path config::filename() {
	if (!_filename.empty()) {
		return _filename;
	}

	auto fpath = _path / "";
	for (const auto &ext : extensions) {
		auto fname = std::filesystem::path(_name);
		fname += ext;

		fpath.replace_filename(fname);
		auto status = std::filesystem::status(fpath);
		if (std::filesystem::is_regular_file(status)) {
			_filename = std::move(fpath);
			break;
		}
	}

	return _filename;
}

viper::value config::env(const char *path) const noexcept {
	std::string p = std::regex_replace(path, std::regex("[.-]"), "_");
	std::transform(
		p.cbegin(), p.cend(), p.begin(), [](unsigned char c) { return std::toupper(c); });

	const char *v = std::getenv(p.c_str());
	return v == nullptr ? viper::value() : viper::value(v);
}

viper::value config::get(const char *path) const {
	if (auto v = env(path)) {
		return v;
	}

	return val(path);
}

config::node_t config::node(const char *path) const {
	node_t ref = _tree.rootref();
	if (!ref.is_map()) {
		return {nullptr};
	}

	for (auto part : ryml::to_csubstr(path).split('.', 0)) {
		if (!ref.is_map()) {
			// Can't access any children
			ref = node_t{nullptr};
			break;
		}

		ref = ref[part];
		if (ref == nullptr) {
			break;
		}
	}

	return ref;
}

void config::read() {
	auto ec    = std::error_code{};
	auto fname = filename();
	auto size  = std::filesystem::file_size(fname, ec);

	if (ec) {
		throw std::filesystem::filesystem_error("[viper] Failed to get config file size", ec);
	}

	auto buffer = std::vector<char>(size, 0);
	auto f      = std::ifstream(fname, std::ios::binary);
	f.read(buffer.data(), buffer.size());

	try {
		f.exceptions(f.badbit | f.failbit);
	} catch (const std::ios_base::failure &e) {
		throw std::filesystem::filesystem_error("[viper] Failed to read configs", e.code());
	}

	_tree = ryml::parse_in_arena(ryml::to_csubstr(buffer.data()));
}

viper::value config::val(const char *path) const noexcept {
	return viper::value(node(path));
}
} // namespace viper
