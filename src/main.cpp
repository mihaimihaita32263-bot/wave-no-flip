#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

static void notifyBlocked(char const* how) {
    static int count = 0;
    if (count++ < 40) {
        log::info("[WaveNoFlip] blocked a gravity change via {}", how);
    }
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

    // Way 1: gravity portals never even touch the wave.
    bool collidedWithObject(float dt, GameObject* obj, cocos2d::CCRect rect, bool skipCheck) {
        if (m_isDart && obj) {
            auto t = obj->m_objectType;
            if (t == GameObjectType::InverseGravityPortal || t == GameObjectType::NormalGravityPortal) {
                notifyBlocked("collidedWithObject");
                return false;
            }
        }
        return PlayerObject::collidedWithObject(dt, obj, rect, skipCheck);
    }

    // Way 2: block the game's gravity-flip function while in wave mode.
    void flipGravity(bool flip, bool noEffects) {
        if (m_isDart && !m_fields->bypass) {
            notifyBlocked("flipGravity");
            return;
        }
        PlayerObject::flipGravity(flip, noEffects);
    }

    // Way 3: if gravity changed anyway while in wave mode, put it back.
    void update(float dt) {
        auto f = m_fields.self();

        if (m_isDart) {
            if (!f->wasWave) {
                f->wasWave = true;
                f->lockedUpsideDown = m_isUpsideDown;
                log::info("[WaveNoFlip] wave detected (upsideDown={})", m_isUpsideDown);
            } else if (m_isUpsideDown != f->lockedUpsideDown) {
                notifyBlocked("update-lock");
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
