#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

static void notifyBlocked() {
    static bool shown = false;
    if (shown) return;
    shown = true;
    Notification::create("Wave No Flip: gravity portal blocked", NotificationIcon::Success, 1.5f)->show();
}

class $modify(WaveNoFlipPlayer, PlayerObject) {
    struct Fields {
        bool wasWave = false;
        bool lockedUpsideDown = false;
        bool bypass = false;
    };

    // Way 1: stop gravity portals from flipping gravity while in wave mode.
    void flipGravity(bool flip, bool noEffects) {
        if (m_isDart && !m_fields->bypass) {
            notifyBlocked();
            return;
        }
        PlayerObject::flipGravity(flip, noEffects);
    }

    // Way 2: if gravity changed anyway while in wave mode, put it back.
    void update(float dt) {
        auto f = m_fields.self();

        if (m_isDart) {
            if (!f->wasWave) {
                f->wasWave = true;
                f->lockedUpsideDown = m_isUpsideDown;
            } else if (m_isUpsideDown != f->lockedUpsideDown) {
                notifyBlocked();
                f->bypass = true;
                PlayerObject::flipGravity(f->lockedUpsideDown, true);
                f->bypass = false;
                m_isUpsideDown = f->lockedUpsideDown;
            }
        } else {
            f->wasWave = false;
        }

        PlayerObject::update(dt);
    }
};
