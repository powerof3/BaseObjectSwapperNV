#pragma once

#include "Defs.h"

struct ConditionFilters
{
	ConditionFilters() = default;
	ConditionFilters(std::string a_conditionID, std::vector<std::string>& a_conditions);

	bool operator==(const ConditionFilters& a_rhs) const
	{
		return conditionID == a_rhs.conditionID;
	}

	bool operator<(const ConditionFilters& a_rhs) const
	{
		return conditionID < a_rhs.conditionID;
	}

	// members
	std::string            conditionID{};  // path|condition1,condition2
	std::vector<FormIDStr> NOT{};
	std::vector<FormIDStr> MATCH{};
};

template <class T>
using ConditionalData = std::map<ConditionFilters, std::vector<T>>;

struct ConditionalInput
{
	ConditionalInput(TESObjectREFR* a_ref, TESForm* a_form) :
		ref(a_ref),
		base(a_form),
		currentCell(a_ref->GetParentCell())
	{
		if (currentCell)
		{
			currentWorldspace = currentCell->worldSpace;
			if (const auto xRegionList = static_cast<ExtraCellRegionList*>(currentCell->extraDataList.GetByType(kExtraData_RegionList)))
			{
				currentRegionList = xRegionList->regionList;
			}
		}
	}

	[[nodiscard]] bool IsValid(RE::FormID a_formID) const;
	[[nodiscard]] bool IsValid(const std::string& a_edid) const;

	[[nodiscard]] bool IsValid(const FormIDStr& a_data) const;
	[[nodiscard]] bool IsValid(const ConditionFilters& a_filters) const;

	// members
	TESObjectREFR* ref;
	TESForm* base;
	TESObjectCELL* currentCell;
	TESWorldSpace* currentWorldspace;
	TESRegionList* currentRegionList;
};
