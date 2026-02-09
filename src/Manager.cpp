#include "Manager.h"

#include "Util.h"

namespace FormSwap
{
	void Manager::LoadFormsOnce()
	{
		std::call_once(init, [this] {
			LoadForms();
			});
	}

	void Manager::LoadForms()
	{
		_MESSAGE("-INI-");

		std::error_code ec;
		const std::filesystem::path bosFolder{ R"(Data\BaseObjectSwapper)" };
		if (!exists(bosFolder, ec)) {
			_WARNING("BOS folder not found...");
			return;
		}

		const auto configs = distribution::get_configs(R"(Data\BaseObjectSwapper)");

		if (configs.empty()) {
			_WARNING("No .ini files were found in Data\\BaseObjectSwapper folder, aborting...");
			return;
		}

		_MESSAGE("%u matching inis found...", configs.size());

		for (auto& path : configs) {
			_MESSAGE("INI : %s", path.c_str());

			CSimpleIniA ini;
			ini.SetUnicode();
			ini.SetMultiKey();
			ini.SetAllowKeyOnly();

			if (const auto rc = ini.LoadFile(path.c_str()); rc < 0) {
				_ERROR("\tcouldn't read INI");
				continue;
			}

			CSimpleIniA::TNamesDepend sections;
			ini.GetAllSections(sections);
			sections.sort(CSimpleIniA::Entry::LoadOrder());

			for (auto& [_section, comment, keyOrder] : sections) {
				std::string section = _section;
				if (section.contains('|')) {
					auto splitSection = string::split(section, "|");
					auto conditions = string::split(splitSection[1], ",");  //[Forms|EditorID,EditorID2]

					_MESSAGE("\treading [%s] : %u conditions", splitSection[0].c_str(), conditions.size());

					ConditionFilters processedConditions(path.substr(5) + "|" + splitSection[1], conditions);

					CSimpleIniA::TNamesDepend values;
					ini.GetAllKeys(section.c_str(), values);
					values.sort(CSimpleIniA::Entry::LoadOrder());

					if (!values.empty()) {
						if (splitSection[0] == "Forms") {
							_MESSAGE("\t\t\t%u form swaps found", values.size());
							for (const auto& key : values) {
								SwapFormData::GetForms(path, key.pItem, [&](const RE::FormID a_baseID, const SwapFormData& a_swapData) {
									swapFormsConditional[a_baseID][processedConditions].emplace_back(a_swapData);
									});
							}
						}
						else {
							_MESSAGE("\t\t\t%u ref property overrides found", values.size());
							for (const auto& key : values) {
								ObjectData::GetProperties(path, key.pItem, [&](const RE::FormID a_baseID, const ObjectData& a_objectData) {
									refPropertiesConditional[a_baseID][processedConditions].emplace_back(a_objectData);
									});
							}
						}
					}
				}
				else {
					_MESSAGE("\treading [%s]", section.c_str());

					CSimpleIniA::TNamesDepend values;
					ini.GetAllKeys(section.c_str(), values);
					values.sort(CSimpleIniA::Entry::LoadOrder());

					if (!values.empty()) {
						if (section == "Transforms" || section == "Properties") {
							_MESSAGE("\t\t\t%u ref property overrides found", values.size());
							for (const auto& key : values) {
								ObjectData::GetProperties(path, key.pItem, [&](RE::FormID a_baseID, const ObjectData& a_objectData) {
									refProperties[a_baseID].push_back(a_objectData);
									});
							}
						}
						else {
							_MESSAGE("\t\t\t%u swaps found", values.size());
							auto& map = (section == "Forms") ? swapForms : swapRefs;
							for (const auto& key : values) {
								SwapFormData::GetForms(path, key.pItem, [&](RE::FormID a_baseID, const SwapFormData& a_swapData) {
									map[a_baseID].push_back(a_swapData);
									});
							}
						}
					}
				}
			}
		}

		_MESSAGE("-RESULT-");

		_MESSAGE("%u form-form swaps", swapForms.size());
		_MESSAGE("%u conditional form swaps", swapFormsConditional.size());
		_MESSAGE("%u ref-form swaps", swapRefs.size());
		_MESSAGE("%u ref property overrides", refProperties.size());
		_MESSAGE("%u conditional ref property overrides", refPropertiesConditional.size());

		_MESSAGE("-CONFLICTS-");

		const auto log_conflicts = [&]<typename T>(std::string_view a_type, const FormIDMap<T>&a_map) {
			if (a_map.empty()) {
				return;
			}
			_MESSAGE("[%s]", a_type.data());
			bool conflicts = false;
			for (auto& [baseID, swapDataVec] : a_map) {
				if (swapDataVec.size() > 1) {
					const auto& winningRecord = swapDataVec.back();
					if (winningRecord.chance.chanceValue != 100) {  //ignore if winning record is randomized
						continue;
					}
					conflicts = true;
					auto winningForm = string::split(winningRecord.record, "|");
					_WARNING("\t%s", winningForm[0].c_str());
					_WARNING("\t\twinning swap : %s (%s)", winningForm[1].c_str(), swapDataVec.back().path.c_str());
					_WARNING("\t\t%u conflicts", swapDataVec.size() - 1);
					for (auto it = swapDataVec.rbegin() + 1; it != swapDataVec.rend(); ++it) {
						auto losingRecord = it->record.substr(it->record.find('|') + 1);
						_WARNING("\t\t\t%s (%s)", losingRecord.c_str(), it->path.c_str());
					}
				}
			}
			if (!conflicts) {
				_MESSAGE("\tNo conflicts found");
			}
			else {
				hasConflicts = true;
			}
		};

		log_conflicts("Forms"sv, swapForms);
		log_conflicts("References"sv, swapRefs);
		log_conflicts("Properties"sv, refProperties);

		_MESSAGE("-END-");
	}

	void Manager::PrintConflicts() const
	{
		if (hasConflicts) {
			Console_Print_Str(std::format("[BOS] Conflicts found, check po3_BaseObjectSwapper.log in %s for more info\n", GetFalloutDirectory()));
		}
	}

	SwapFormResult Manager::GetSwapFormConditional(TESObjectREFR* a_ref, TESForm* a_base)
	{
		if (const auto it = swapFormsConditional.find(a_base->refID); it != swapFormsConditional.end()) {
			const ConditionalInput input(a_ref, a_base);

			for (auto& [filters, swapDataVec] : it->second | std::ranges::views::reverse) {
				if (input.IsValid(filters)) {
					for (auto& swapData : swapDataVec | std::ranges::views::reverse) {
						if (auto swapObject = swapData.GetSwapBase(a_ref)) {
							return { swapObject, swapData.properties };
						}
					}
				}
			}
		}

		return { nullptr, std::nullopt };
	}

	std::optional<ObjectProperties> Manager::GetObjectPropertiesConditional(TESObjectREFR* a_ref, TESForm* a_base)
	{
		if (const auto it = refPropertiesConditional.find(a_base->refID); it != refPropertiesConditional.end()) {
			const ConditionalInput input(a_ref, a_base);

			for (auto& [filters, objectDataVec] : it->second | std::ranges::views::reverse) {
				if (input.IsValid(filters)) {
					for (auto& objectData : objectDataVec | std::ranges::views::reverse) {
						if (objectData.HasValidProperties(a_ref)) {
							return objectData.properties;
						}
					}
				}
			}
		}

		return std::nullopt;
	}

	void Manager::InsertLeveledItemRef(const TESObjectREFR* a_refr)
	{
		swappedLeveledItemRefs.insert(a_refr->refID);
	}

	bool Manager::IsLeveledItemRefSwapped(const TESObjectREFR* a_refr) const
	{
		return swappedLeveledItemRefs.contains(a_refr->refID);
	}

	SwapFormResult Manager::GetSwapData(TESObjectREFR* a_ref, TESForm* a_base)
	{
		SwapFormResult swapData{ nullptr, std::nullopt };

		// get base
		const auto get_swap_base = [a_ref](const TESForm* a_form, const FormIDMap<SwapFormDataVec>& a_map) -> SwapFormResult {
			if (const auto it = a_map.find(a_form->refID); it != a_map.end()) {
				for (auto& data : it->second | std::ranges::views::reverse) {
					if (auto swapBase = data.GetSwapBase(a_ref)) {
						return { swapBase, data.properties };
					}
				}
			}
			return { nullptr, std::nullopt };
			};

		// references -> conditional forms -> forms
		if (!a_ref->IsCreated()) {
			swapData = get_swap_base(a_ref, swapRefs);
		}
		if (!swapData.first) {
			swapData = GetSwapFormConditional(a_ref, a_base);
		}
		if (!swapData.first) {
			swapData = get_swap_base(a_base, swapForms);
		}

		// process leveled swaps. do not swap if leveled item has encounter zone
		/*if (const auto swapLvlBase = swapData.first ? swapData.first->As<TESLevItem>() : nullptr) {
			if (a_ref->GetEncounterZone() == nullptr) {
				RE::BSScrapArray<RE::CALCED_OBJECT> calcedObjects{};
				swapLvlBase->CalculateCurrentFormList(a_ref->GetCalcLevel(false), 1, calcedObjects, 0, true);
				if (!calcedObjects.empty()) {
					swapData.first = static_cast<RE::TESBoundObject*>(calcedObjects.front().form);
				}
			}
			else {
				swapData.first = nullptr;
			}
		}*/

		// get object properties
		const auto get_properties = [&](const TESForm* a_form) -> std::optional<ObjectProperties> {
			if (const auto it = refProperties.find(a_form->refID); it != refProperties.end()) {
				for (auto& objectData : it->second | std::ranges::views::reverse) {
					if (objectData.HasValidProperties(a_ref)) {
						return objectData.properties;
					}
				}
			}
			return std::nullopt;
			};

		constexpr auto has_properties = [](const std::optional<ObjectProperties>& a_result) {
			return a_result && a_result->IsValid();
			};

		// references -> conditional forms -> forms
		if (!has_properties(swapData.second) && !a_ref->IsCreated()) {
			swapData.second = get_properties(a_ref);
		}
		if (!has_properties(swapData.second)) {
			swapData.second = GetObjectPropertiesConditional(a_ref, a_base);
		}
		if (!has_properties(swapData.second)) {
			swapData.second = get_properties(a_base);
		}

		return swapData;
	}
}
