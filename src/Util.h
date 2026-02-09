#pragma once

#include "Defs.h"

namespace regex
{
	inline srell::regex generic{ R"(\((.*?)\))" };                // pos(0,0,100) -> "0,0,100"
	inline srell::regex transform{ R"(\((.*?),(.*?),(.*?)\))" };  // pos(0,0,100) -> 0, 0, 100
	inline srell::regex string{ R"(,\s*(?![^()]*\)))" };          // pos(0, 0, 100), rot(0, 0, 100) -> "pos(0, 0, 100)","rot(0, 0, 100)"
}

namespace util
{
	inline Script* HasKeywordScript;

	std::vector<std::string> split_with_regex(const std::string& a_str, const srell::regex& a_regex);

	RE::FormID  GetFormID(const std::string& a_str);
	FormIDOrSet GetSwapFormID(const std::string& a_str);
	FormIDOrderedSet GetFormIDOrderedSet(const std::string& a_str);
	bool HasKeyword(TESForm* a_form, const std::string& a_keyword);

	inline float deg_to_rad(float a_degrees)
	{
		return a_degrees * (RE::NI_PI / 180.0f);
	}

	inline float rad_to_deg(float a_radians)
	{
		return a_radians * (180.0f / RE::NI_PI);
	}
}
