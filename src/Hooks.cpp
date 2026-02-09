#include "Hooks.h"

#include "Manager.h"

namespace BaseObjectSwapper
{
	struct InitItemImpl
	{
		static void __fastcall thunk(TESObjectREFR* a_ref, void* edx)
		{
			if (const auto base = a_ref->baseForm) {
				FormSwap::Manager::GetSingleton()->LoadFormsOnce();

				const auto& [swapBase, objectProperties] = FormSwap::Manager::GetSingleton()->GetSwapData(a_ref, base);

				if (swapBase && swapBase != base) {
					a_ref->InitBaseForm(swapBase);
				}

				if (objectProperties) {
					objectProperties->SetTransform(a_ref);
					objectProperties->SetRecordFlags(a_ref);
				}
			}

			return ThisStdCall<void>(originalAddress, a_ref);
		}
		static inline std::uint32_t originalAddress;

		static void Install()
		{
			originalAddress = DetourVtable(0x102F55C + (4 * 0x22), reinterpret_cast<UInt32>(thunk)); // kVtbl_TESObjectREFR

			_MESSAGE("Installed vtable hook");
		}
	};

	void Install()
	{
		_MESSAGE("-HOOKS-");

		InitItemImpl::Install();
	}
}
