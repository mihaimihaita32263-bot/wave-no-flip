#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

static int g_logCount = 0;

static void toast() {
    static bool shown = false;
    if (shown) return;
    shown = true;
    Notification::create("Wave No Flip: active", NotificationIcon::Success, 1.5f)->show();
}

class $modify(WaveNoFlipPlayer, PlayerObject) {
    struct Fields {
        bool bypass = false;
        bool wasWave = false;
    };

    // The wave never touches gravity portals.
    bool collidedWithObject(float dt, GameObject* obj, cocos2d::CCRect rect, bool skipCheck) {
        if (m_isDart && obj) {
            auto t = obj->m_objectType;
            if (t == GameObjectType::InverseGravityPortal || t == GameObjectType::NormalGravityPortal) {
                if (g_logCount++ < 60) log::info("[WaveNoFlip] gravity portal ignored (wave)");
                toast();
                return false;
            }
        }
        return PlayerObject::collidedWithObject(dt, obj, rect, skipCheck);
    }

    // Nothing can flip gravity while in wave mode.
    void flipGravity(bool flip, bool noEffects) {
        if (m_isDart && !m_fields->bypass) {
            if (g_logCount++ < 60) log::info("[WaveNoFlip] flipGravity blocked (wave)");
            toast();
            return;
        }
        PlayerObject::flipGravity(flip, noEffects);
    }

    // While in wave mode, gravity is always normal (never upside down).
    void update(float dt) {
        auto f = m_fields.self();

        if (m_isDart) {
            if (!f->wasWave) {
                f->wasWave = true;
                if (PlayLayer::get() && g_logCount++ < 60) {
                    log::info("[WaveNoFlip] wave started (upsideDown={})", m_isUpsideDown);
                }
            }
            if (m_isUpsideDown) {
                if (g_logCount++ < 60) log::info("[WaveNoFlip] forcing normal gravity in wave");
                toast();
                f->bypass = true;
                PlayerObject::flipGravity(false, true);
                f->bypass = false;
                m_isUpsideDown = false;
            }
        } else {
            f->wasWave = false;
        }

        PlayerObject::update(dt);
    }
};
