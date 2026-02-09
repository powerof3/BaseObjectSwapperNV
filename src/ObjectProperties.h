#pragma once

#include "RNG.h"

struct RandValueParams
{
	RandValueParams(Chance a_chance, const TESObjectREFR* a_ref);

	BOS_RNG rng{};
	bool    clamp{ false };
	float   clampMin{ 0.0f };
	float   clampMax{ 0.0f };
};

struct FloatRange
{
	FloatRange() = default;
	FloatRange(const std::string& a_str);

	bool operator==(const FloatRange& a_rhs) const;
	bool operator!=(const FloatRange& a_rhs) const;

	bool is_exact() const;
	void convert_to_radians();

	float GetRandomValue(const RandValueParams& a_params) const;

	// members
	float min{};
	float max{};
};

struct ScaleRange
{
	ScaleRange() = default;
	ScaleRange(const std::string& a_str);

	void SetScale(TESObjectREFR* a_ref, const RandValueParams& a_params) const;

	// members
	bool       absolute{ false };
	FloatRange value{};
};

struct Point3Range
{
	Point3Range() = default;
	Point3Range(const std::string& a_str, bool a_convertToRad = false);

	NiPoint3 min() const;
	NiPoint3 max() const;
	bool         is_exact() const;

	NiPoint3 GetRandomValue(const RandValueParams& a_params) const;
	void         SetTransform(float& a_x, float& a_y, float& a_z, const RandValueParams& a_params) const;

	// members
	bool       relative;
	FloatRange x;
	FloatRange y;
	FloatRange z;
};

class ObjectProperties
{
public:
	ObjectProperties() = default;
	explicit ObjectProperties(const std::string& a_str);

	bool IsValid() const;

	void SetChance(Chance a_chance);
	void SetTransform(TESObjectREFR* a_refr) const;
	void SetRecordFlags(TESObjectREFR* a_refr) const;

private:
	void assign_record_flags(const std::string& a_str, bool a_unsetFlag);

	// members
	Chance chance{};

	std::optional<Point3Range> location{ std::nullopt };
	std::optional<Point3Range> rotation{ std::nullopt };
	std::optional<ScaleRange>  refScale{ std::nullopt };

	std::uint32_t recordFlagsSet{ 0 };
	std::uint32_t recordFlagsUnset{ 0 };
};
