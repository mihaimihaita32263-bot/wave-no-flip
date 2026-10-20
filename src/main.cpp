#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(WaveNoFlipPlayer, PlayerObject) {
    void flipGravity(bool flip, bool noEffects) {
        auto mod = Mod::get();

        bool enabled = mod->getSettingValue<bool>("enabled");
        bool practiceOnly = mod->getSettingValue<bool>("practice-only");

        // m_isDart is true while the player is in wave mode
        if (enabled && m_isDart) {
            auto pl = PlayLayer::get();
            bool inPractice = pl && pl->m_isPracticeMode;

            if (!practiceOnly || inPractice) {
                // Skip the original function: gravity stays unchanged
                return;
            }
        }

        PlayerObject::flipGravity(flip, noEffects);
    }
};
