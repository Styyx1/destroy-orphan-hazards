namespace EDID
{
    using _GetFormEditorID = const char *(*)(std::uint32_t);

    /*
    * Given a TESForm, it returns the Editor ID. For forms whose ID is not cached,
    * tries to fetch the ID through PowerOfThree's Tweaks. If this fails, or the form
    * doesn't have an EditorID it returns an empty string.
    * @param a_form Pointer to the form to querry. Must NOT be nullptr.
    * @return The form's EditorID. Empty if the form doesn't have an EditorID or PO3's Tweaks are not installed AND the form's EditorID is not cached.
    */
    inline std::string GetEditorID(const RE::TESForm *a_form)
    {
        switch (a_form->GetFormType())
        {
        case RE::FormType::Keyword:
        case RE::FormType::LocationRefType:
        case RE::FormType::Action:
        case RE::FormType::MenuIcon:
        case RE::FormType::Global:
        case RE::FormType::HeadPart:
        case RE::FormType::Race:
        case RE::FormType::Sound:
        case RE::FormType::Script:
        case RE::FormType::Navigation:
        case RE::FormType::Cell:
        case RE::FormType::WorldSpace:
        case RE::FormType::Land:
        case RE::FormType::NavMesh:
        case RE::FormType::Dialogue:
        case RE::FormType::Quest:
        case RE::FormType::Idle:
        case RE::FormType::AnimatedObject:
        case RE::FormType::ImageAdapter:
        case RE::FormType::VoiceType:
        case RE::FormType::Ragdoll:
        case RE::FormType::DefaultObject:
        case RE::FormType::MusicType:
        case RE::FormType::StoryManagerBranchNode:
        case RE::FormType::StoryManagerQuestNode:
        case RE::FormType::StoryManagerEventNode:
        case RE::FormType::SoundRecord:
            return a_form->GetFormEditorID();
        default:
        {
            static auto tweaks = REX::W32::GetModuleHandleA("po3_Tweaks");
            static auto func = reinterpret_cast<_GetFormEditorID>(REX::W32::GetProcAddress(tweaks, "GetFormEditorID"));
            if (func)
            {
                return func(a_form->formID);
            }
            return {};
        }
        }
    }
};


inline static bool ShouldLoad(RE::Hazard* a_haz) {
    if (a_haz->lifetime == -1.0f && !a_haz->IsPermanent()) {
        return false;
    }
    return true;
}

namespace Hooks {
    struct HazardHook {

        static void LoadGameHook(RE::Hazard* a_haz, RE::BGSLoadGameBuffer* a_buf) {
            auto base = a_haz->GetBaseObject();
            if(base)
                logs::info("Load Game Finish Hook for: {}", EDID::GetEditorID(base));
            if (!ShouldLoad(a_haz)) {
                RE::GarbageCollector::GetSingleton()->Add(a_haz, true);
                return;
            }
            logs::info("should load was true");
            func(a_haz, a_buf);
        }

        static void Install()
        {
            REL::Relocation<std::uintptr_t> hazardVtable{ RE::VTABLE_Hazard[0] };
            func = hazardVtable.write_vfunc(0x11, &LoadGameHook);
            logs::info("Installed LoadGame hook on Hazard");
        }

    private:
        static inline REL::Relocation<decltype(LoadGameHook)> func;
    };
}

void InitListener(SKSE::MessagingInterface::Message *a_msg)
{
    if (a_msg->type == SKSE::MessagingInterface::kDataLoaded)
    {
        Hooks::HazardHook::Install();
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
    SKSE::GetMessagingInterface()->RegisterListener(InitListener);

	return true;
}
