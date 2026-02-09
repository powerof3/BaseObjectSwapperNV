#include "ConditionalData.h"

#include "Util.h"

ConditionFilters::ConditionFilters(std::string a_conditionID, std::vector<std::string>& a_conditions) :
	conditionID(std::move(a_conditionID))
{
	NOT.reserve(a_conditions.size());
	MATCH.reserve(a_conditions.size());

	for (auto& condition : a_conditions) {
		bool negate = false;
		if (!condition.empty() && condition[0] == '-') {
			condition.erase(0, 1);
			negate = true;
		}
		if (const auto processedID = util::GetFormID(condition); processedID != 0) {
			negate ? NOT.emplace_back(processedID) : MATCH.emplace_back(processedID);
		}
		else {
			_ERROR("\t\tFilter [%s] INFO - unable to find form, treating filter as FF keyword or cell editorID", condition.c_str());
			negate ? NOT.emplace_back(condition) : MATCH.emplace_back(condition);
		}
	}
}

bool ConditionalInput::IsValid(RE::FormID a_formID) const
{
	if (const auto form = LookupFormByID(a_formID)) {
		switch (form->typeID) {
		case kFormType_TESRegion:
		{
			if (const auto region = static_cast<TESRegion*>(form)) {
				if (currentRegionList) {
					for (const auto& regionInList : currentRegionList->list)
					{
						if (regionInList == region)
						{
							return true;
						}
					}
				}
			}
			return false;
		}
		case kFormType_TESObjectCELL:
			return currentCell == form;
		case kFormType_TESWorldSpace:
		{
			const auto worldspace = static_cast<TESWorldSpace*>(form);
			return currentWorldspace && (currentWorldspace == worldspace || currentWorldspace->parent == worldspace);
		}
		default:
			break;
		}
	}

	return false;
}

bool ConditionalInput::IsValid(const std::string& a_edid) const
{
	if (currentCell && string::iequals(currentCell->GetEditorID(), a_edid)) {
		return true;
	}

	return util::HasKeyword(base, a_edid) || util::HasKeyword(ref, a_edid);
}

bool ConditionalInput::IsValid(const FormIDStr& a_data) const
{
	bool result = false;

	std::visit(overload{
				   [&](RE::FormID a_formID) {
					   result = IsValid(a_formID);
				   },
				   [&](const std::string& a_edid) {
					   result = IsValid(a_edid);
				   } },
		a_data);

	return result;
}

bool ConditionalInput::IsValid(const ConditionFilters& a_filters) const
{
	if (!a_filters.NOT.empty()) {
		if (std::ranges::any_of(a_filters.NOT, [this](const auto& data) { return IsValid(data); })) {
			return false;
		}
	}

	if (!a_filters.MATCH.empty()) {
		if (std::ranges::none_of(a_filters.MATCH, [this](const auto& data) { return IsValid(data); })) {
			return false;
		}
	}

	return true;
}
