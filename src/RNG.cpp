#include "RNG.h"

#include "Util.h"

std::size_t BOS_RNG::get_form_seed(const TESForm* a_form)
{
	if (a_form->IsCreated()) {
		return a_form->refID;
	}

	auto fileA = a_form->mods.GetFirstItem();
	if (!fileA)
	{
		return a_form->refID;
	}

	std::size_t result = 0;
	boost::hash_combine(result, a_form->GetLocalFormID());
	boost::hash_combine(result, fileA->name);

	return result;
}

BOS_RNG::BOS_RNG(const Chance& a_chance, const TESObjectREFR* a_ref) :
	type(a_chance.chanceType)
{
	switch (type) {
	case CHANCE_TYPE::kRefHash:
		seed = get_form_seed(a_ref);
		break;
	case CHANCE_TYPE::kLocationHash:
	{
		const auto base = a_ref->baseForm;

		if (auto cell = a_ref->GetParentCell(); !cell) {
			seed = get_form_seed(a_ref);
		}
		else {
			// generate hash based on location + baseID
			std::size_t result = 0;
			boost::hash_combine(result, get_form_seed(cell));
			boost::hash_combine(result, get_form_seed(base));
			seed = result;
		}
	}
	break;
	case CHANCE_TYPE::kRandom:
		seed = a_chance.seed;
		break;
	default:
		break;
	}
}

BOS_RNG::BOS_RNG(const Chance& a_chance) :
	type(a_chance.chanceType),
	seed(a_chance.seed)
{
}

Chance::Chance(const std::string& a_str)
{
	if (distribution::is_valid_entry(a_str)) {
		if (a_str.contains("chance")) {
			if (a_str.contains("R")) {
				chanceType = CHANCE_TYPE::kRandom;
			}
			else if (a_str.contains("L")) {
				chanceType = CHANCE_TYPE::kLocationHash;
			}
			else {
				chanceType = CHANCE_TYPE::kRefHash;
			}

			if (srell::cmatch match; srell::regex_search(a_str.c_str(), match, regex::generic)) {
				const auto chanceOptions = string::split(match[1].str(), ",");
				chanceValue = string::to_num<float>(chanceOptions[0]);
				seed = chanceOptions.size() > 1 ? string::to_num<std::size_t>(chanceOptions[1]) : 0;
			}
		}
	}
}

bool Chance::PassedChance(const TESObjectREFR* a_ref) const
{
	if (chanceValue < 100.0f) {
		BOS_RNG rng(*this, a_ref);
		if (const auto rngValue = rng.generate<float>(0.0f, 100.0f); rngValue > chanceValue) {
			return false;
		}
	}
	return true;
}
