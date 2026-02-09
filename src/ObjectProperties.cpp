#include "ObjectProperties.h"

#include "Util.h"

RandValueParams::RandValueParams(Chance a_chance, const TESObjectREFR* a_ref) :
	rng(a_chance, a_ref)
{}

FloatRange::FloatRange(const std::string& a_str)
{
	const auto splitRange = string::split(a_str, R"(/)");
	min = string::to_num<float>(splitRange[0]);
	max = splitRange.size() > 1 ? string::to_num<float>(splitRange[1]) : min;
}

bool FloatRange::operator==(const FloatRange& a_rhs) const
{
	return (min == a_rhs.min && max == a_rhs.min);
}

bool FloatRange::operator!=(const FloatRange& a_rhs) const
{
	return !operator==(a_rhs);
}

bool FloatRange::is_exact() const
{
	return min == max;
}

void FloatRange::convert_to_radians()
{
	min = util::deg_to_rad(min);
	max = util::deg_to_rad(max);
}

float FloatRange::GetRandomValue(const RandValueParams& a_params) const
{
	float value = is_exact() ? min : a_params.rng.generate(min, max);

	if (a_params.clamp) {
		value = std::clamp(value, a_params.clampMin, a_params.clampMax);
	}

	return value;
}

ScaleRange::ScaleRange(const std::string& a_str) :
	absolute(a_str.contains('A'))
{
	if (srell::cmatch match; srell::regex_search(a_str.c_str(), match, regex::generic)) {
		value = FloatRange(match[1].str());
	}
}

void ScaleRange::SetScale(TESObjectREFR* a_ref, const RandValueParams& a_params) const
{
	if (absolute) {
		a_ref->scale = value.GetRandomValue(a_params);
	} else {
		a_ref->scale = a_ref->scale * value.GetRandomValue(a_params);
	}
}

Point3Range::Point3Range(const std::string& a_str, bool a_convertToRad) :
	relative(a_str.contains('R'))
{
	if (srell::cmatch match; srell::regex_search(a_str.c_str(), match, regex::transform)) {
		//match[0] gets the whole string
		x = FloatRange(match[1].str());
		y = FloatRange(match[2].str());
		z = FloatRange(match[3].str());
		if (a_convertToRad) {
			x.convert_to_radians();
			y.convert_to_radians();
			z.convert_to_radians();
		}
	}
}

NiPoint3 Point3Range::min() const
{
	return NiPoint3(x.min, y.min, z.min);
}

NiPoint3 Point3Range::max() const
{
	return NiPoint3(x.max, y.max, z.max);
}

bool Point3Range::is_exact() const
{
	auto minV = min();
	auto maxV = max();
	return minV.x == maxV.x && minV.y == maxV.y && minV.z == maxV.z;
}

NiPoint3 Point3Range::GetRandomValue(const RandValueParams& a_params) const
{
	return is_exact() ? min() : NiPoint3{ x.GetRandomValue(a_params), y.GetRandomValue(a_params), z.GetRandomValue(a_params) };
}

void Point3Range::SetTransform(float& a_x, float& a_y, float& a_z, const RandValueParams& a_params) const
{
	auto point = GetRandomValue(a_params);
	if (relative) {
		a_x += point.x;
		a_y += point.y;
		a_z += point.z;
	} else {
		a_x = point.x;
		a_y = point.y;
		a_z = point.z;
	}
}

void ObjectProperties::assign_record_flags(const std::string& a_str, bool a_unsetFlag)
{
	auto& flags = a_unsetFlag ? recordFlagsUnset : recordFlagsSet;

	if (srell::cmatch match; srell::regex_search(a_str.c_str(), match, regex::generic)) {
		for (const auto& str : string::split(match[1].str(), ",")) {
			flags |= string::to_num<std::uint32_t>(str, true);
		}
	}
}

ObjectProperties::ObjectProperties(const std::string& a_str)
{
	if (distribution::is_valid_entry(a_str)) {
		auto split_properties = util::split_with_regex(a_str, regex::string);
		for (const auto& propStr : split_properties) {
			if (propStr.contains("pos")) {
				location = Point3Range(propStr);
			} else if (propStr.contains("rot")) {
				rotation = Point3Range(propStr, true);
			} else if (propStr.contains("scale")) {
				refScale = ScaleRange(propStr);
			} else if (propStr.contains("flags")) {
				assign_record_flags(propStr, propStr.contains("C"));
			}
		}
	}
}

bool ObjectProperties::IsValid() const
{
	return location || rotation || refScale || recordFlagsSet != 0 || recordFlagsUnset != 0;
}

void ObjectProperties::SetChance(Chance a_chance)
{
	chance = a_chance;
}

void ObjectProperties::SetTransform(TESObjectREFR* a_refr) const
{
	if (location || rotation || refScale) {
		RandValueParams params(chance, a_refr);
		if (location) {
			location->SetTransform(a_refr->posX, a_refr->posY, a_refr->posZ, params);
		}
		if (rotation) {
			params.clamp = true;
			params.clampMin = -RE::NI_TWO_PI;
			params.clampMax = RE::NI_TWO_PI;
			rotation->SetTransform(a_refr->rotX, a_refr->rotY, a_refr->rotZ, params);
		}
		if (refScale) {
			params.clamp = true;
			params.clampMin = 0.0f;
			params.clampMax = 1000.0f;
			refScale->SetScale(a_refr, params);
		}
	}
}

void ObjectProperties::SetRecordFlags(TESObjectREFR* a_refr) const
{
	if (recordFlagsUnset != 0) {
		a_refr->flags &= ~recordFlagsUnset;
	}

	if (recordFlagsSet != 0) {
		a_refr->flags |= recordFlagsSet;
	}
}
