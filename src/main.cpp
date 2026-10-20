#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

static int g_logCount = 0;

static void notifyBlocked(char const* how) {
    if (g_logCount++ < 400) {
        log::info("[WaveNoFlip] BLOCKED via {}", how);
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

        // last seen values, for change logging
        bool seen = false;
        bool lUp = false;
        bool lTouch = false;
        int lState = 0;
        float lMod = 0.f;
        bool lDart = false;
    };

    // Way 1: gravity portals never even touch the wave.
    bool collidedWithObject(float dt, GameObject* obj, cocos2d::CCRect rect, bool skipCheck) {
        if (obj) {
            auto t = obj->m_objectType;
            if (t == GameObjectType::InverseGravityPortal || t == GameObjectType::NormalGravityPortal) {
                if (g_logCount++ < 400) {
                    log::info("[WaveNoFlip] gravity portal collision (wave={}, p2={})", m_isDart, m_isSecondPlayer);
                }
                if (m_isDart) {
                    notifyBlocked("collidedWithObject");
                    return false;
                }
            }
        }
        return PlayerObject::collidedWithObject(dt, obj, rect, skipCheck);
    }

    // Way 2: block the game's gravity-flip function while in wave mode.
    void flipGravity(bool flip, bool noEffects) {
        if (g_logCount++ < 400) {
            log::info("[WaveNoFlip] flipGravity called (flip={}, wave={}, p2={})", flip, m_isDart, m_isSecondPlayer);
        }
        if (m_isDart && !m_fields->bypass) {
            notifyBlocked("flipGravity");
            return;
        }
        PlayerObject::flipGravity(flip, noEffects);
    }

    // Way 3 + diagnostics.
    void update(float dt) {
        auto f = m_fields.self();

        // Log whenever any gravity-related value changes.
        if (!f->seen || f->lUp != m_isUpsideDown || f->lTouch != m_touchedGravityPortal ||
            f->lState != m_stateFlipGravity || f->lMod != m_gravityMod || f->lDart != m_isDart) {
            f->seen = true;
            f->lUp = m_isUpsideDown;
            f->lTouch = m_touchedGravityPortal;
            f->lState = m_stateFlipGravity;
            f->lMod = m_gravityMod;
            f->lDart = m_isDart;
            if (g_logCount++ < 400) {
                log::info(
                    "[WaveNoFlip] state: wave={} upsideDown={} touchedGravPortal={} stateFlip={} gravityMod={} p2={}",
                    m_isDart, m_isUpsideDown, m_touchedGravityPortal, m_stateFlipGravity, m_gravityMod, m_isSecondPlayer
                );
            }
        }

        if (m_isDart) {
            if (!f->wasWave) {
                f->wasWave = true;
                f->lockedUpsideDown = m_isUpsideDown;
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
