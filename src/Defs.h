#pragma once

#include <variant>
#include <string>
#include <ranges>
#include <set>

#include "GameObjects.h"
#include "GameData.h"
#include "SafeWrite.h"
#include "Utilities.h"
#include "PluginAPI.h"
#include "GameAPI.h"
#include "GameScript.h"

#include "lib/string.hpp"
#include "lib/rng.hpp"
#include "lib/distribution.hpp"
#include "lib/numeric.hpp"
#include "lib/simpleINI.hpp"

#include "srell.hpp"
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

using namespace std::literals;

using namespace clib_util;
using SeedRNG = clib_util::RNG;

// for visting variants
template <class... Ts>
struct overload : Ts...
{
	using Ts::operator()...;
};

using FormIDStr = std::variant<RE::FormID, std::string>;

template <class K, class D>
using FlatMap = boost::unordered_flat_map<K, D>;
template <class T>
using FlatSet = boost::unordered_flat_set<T>;
template <class T>
using OrderedSet = std::set<T>;

using FormIDSet = FlatSet<RE::FormID>;
using FormIDOrSet = std::variant<RE::FormID, FormIDSet>;
using FormIDOrderedSet = OrderedSet<RE::FormID>;

template <class T>
using FormIDMap = FlatMap<RE::FormID, T>;

namespace RE
{
	constexpr float NI_INFINITY = FLT_MAX;
	constexpr float NI_PI = static_cast<float>(3.1415926535897932);
	constexpr float NI_HALF_PI = 0.5F * NI_PI;
	constexpr float NI_TWO_PI = 2.0F * NI_PI;
}
