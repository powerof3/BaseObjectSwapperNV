#pragma once

#include "SwapData.h"

namespace FormSwap
{
	class Manager
	{
	public:
		static Manager* GetSingleton()
		{
			static Manager singleton;
			return std::addressof(singleton);
		}
		
		void LoadFormsOnce();

		void PrintConflicts() const;

		SwapFormResult                  GetSwapData(TESObjectREFR* a_ref, TESForm* a_base);
		SwapFormResult                  GetSwapFormConditional(TESObjectREFR* a_ref, TESForm* a_base);
		std::optional<ObjectProperties> GetObjectPropertiesConditional(TESObjectREFR* a_ref, TESForm* a_base);

		void InsertLeveledItemRef(const TESObjectREFR* a_refr);
		bool IsLeveledItemRefSwapped(const TESObjectREFR* a_refr) const;

	protected:
		Manager() = default;
		~Manager() = default;

		Manager(const Manager&) = delete;
		Manager(Manager&&) = delete;

		Manager& operator=(const Manager&) = delete;
		Manager& operator=(Manager&&) = delete;

	private:
		void LoadForms();

		// members
		FormIDMap<SwapFormDataVec>         swapRefs{};
		FormIDMap<SwapFormDataConditional> swapFormsConditional{};
		FormIDMap<SwapFormDataVec>         swapForms{};

		FormIDMap<ObjectDataVec>         refProperties{};
		FormIDMap<ObjectDataConditional> refPropertiesConditional{};

		FlatSet<RE::FormID> swappedLeveledItemRefs{};

		bool           hasConflicts{ false };
		std::once_flag init{};
	};
}
