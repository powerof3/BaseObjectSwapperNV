#pragma once

#include "Defs.h"

enum class CHANCE_TYPE : std::uint32_t
{
	kRandom,
	kRefHash,
	kLocationHash
};

struct Chance
{
	Chance() = default;
	explicit Chance(const std::string& a_str);

	bool PassedChance(const TESObjectREFR* a_ref) const;

	// members
	CHANCE_TYPE   chanceType{ CHANCE_TYPE::kRefHash };
	float         chanceValue{ 100.0f };
	std::size_t seed{ 0 };
};

struct BOS_RNG
{
	BOS_RNG() = default;
	BOS_RNG(const Chance& a_chance, const TESObjectREFR* a_ref);
	BOS_RNG(const Chance& a_chance);

	template <class T>
	T generate(T a_min, T a_max) const
	{
		if (type == CHANCE_TYPE::kRandom && seed == 0) {
			return SeedRNG().generate<T>(a_min, a_max);
		}
		return SeedRNG(seed).generate<T>(a_min, a_max);
	}

	// members
	CHANCE_TYPE   type;
	std::size_t seed{ 0 };

private:
	static std::size_t get_form_seed(const TESForm* a_form);
};
