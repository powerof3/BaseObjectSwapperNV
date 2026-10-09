#include "Util.h"

extern NVSEScriptInterface* g_script;

class TESFile {
public:
	std::uint32_t	padding[0xFA];
	Bitfield32		flags;
	std::uint32_t	padding2[0x8];
	std::uint8_t	modIndex;
	std::uint16_t	secondModIndex;

	bool IsSmall() const { return flags.Get(0x100); }
	bool IsMedium() const { return flags.Get(0x400); }
	bool IsSpecial() const { return flags.Get(0x700); }
};

class TESDataHandler {
public:
	Bitfield8								flags;

	static TESDataHandler* GetSingleton() { return *reinterpret_cast<TESDataHandler**>(0x11C3F2C); }

	bool SupportsNewFileTypes() const { return flags.Get(0x80); }

	TESFile* GetListFile(const char* fileName) const { return ThisStdCall<TESFile*>(0x462F40, this, fileName); }
};

namespace util
{
	std::vector<std::string> split_with_regex(const std::string& a_str, const srell::regex& a_regex)
	{
		srell::sregex_token_iterator iter(a_str.begin(),
			a_str.end(),
			a_regex,
			-1);
		srell::sregex_token_iterator end{};
		return { iter, end };
	}

	RE::FormID GetFormID(const std::string& a_str)
	{
		constexpr auto lookup_formID = [](std::uint32_t a_refID, const std::string& modName) -> std::uint32_t
			{
				const auto mod = TESDataHandler::GetSingleton()->GetListFile(modName.c_str());
				if (mod) {
					if (TESDataHandler::GetSingleton()->SupportsNewFileTypes() && mod->IsSpecial()) {
						if (mod->IsSmall()) {
							return (a_refID & 0xFFF) | mod->modIndex << 24 | mod->secondModIndex << 12;
						}
						else if (mod->IsMedium()) {
							return (a_refID & 0xFFFF) | mod->modIndex << 24 | mod->secondModIndex << 16;
						}
					}
					
					return (a_refID & 0xFFFFFF) | mod->modIndex << 24;
				}
				else {
					return 0;
				}
			};

		if (const auto splitID = string::split(a_str, "~"); splitID.size() == 2) {
			const auto  formID = string::to_num<std::uint32_t>(splitID[0], true);
			const auto& modName = splitID[1];
			return lookup_formID(formID, modName);
		}
		if (string::is_only_hex(a_str, true))
		{
			auto formID = string::to_num<std::uint32_t>(a_str, true);
			if (const auto form = LookupFormByID(string::to_num<std::uint32_t>(a_str, true)); !form) {
				_ERROR("\t\tFilter [%s] INFO - unable to find form, treating filter as cell formID", a_str.c_str());
			}
			return formID;
		}
		if (const auto form = GetFormByID(a_str.c_str())) {
			return form->refID;
		}
		return static_cast<RE::FormID>(0);
	}

	FormIDOrSet GetSwapFormID(const std::string& a_str)
	{
		if (a_str.contains(",")) {
			FormIDSet  set;
			const auto IDStrs = string::split(a_str, ",");
			set.reserve(IDStrs.size());
			for (auto& IDStr : IDStrs) {
				if (auto formID = GetFormID(IDStr); formID != 0) {
					set.emplace(formID);
				}
				else {
					_ERROR("\t\t\tfailed to process %s (SWAP formID not found)", IDStr.c_str());
				}
			}
			return set;
		}
		else {
			return GetFormID(a_str);
		}
	}

	FormIDOrderedSet GetFormIDOrderedSet(const std::string& a_str)
	{
		FormIDOrderedSet set;
		if (a_str.contains(",")) {
			const auto IDStrs = string::split(a_str, ",");
			for (auto& IDStr : IDStrs) {
				if (auto formID = GetFormID(IDStr); formID != 0) {
					set.emplace(formID);
				}
				else {
					_ERROR("\t\t\tfailed to process %s (formID not found)", IDStr.c_str());
				}
			}
			return set;
		}
		else if (auto formID = GetFormID(a_str); formID != 0) {
			set.emplace(formID);
		}
		return set;
	}

	bool HasKeyword(TESForm* a_form, const std::string& a_keyword)
	{
		if (!HasKeywordScript)
		{
			HasKeywordScript = g_script->CompileScript(
				R"(Begin Function { Ref akForm, string_var asKeyword }
					SetFunctionValue (HasKeyword akForm (asKeyword))
				End)");
		}

		NVSEArrayVarInterface::Element result;
		g_script->CallFunction(HasKeywordScript, nullptr, nullptr, &result, 2, a_form, a_keyword.c_str());

		return result.GetNumber() != 0.0;
	}
}
