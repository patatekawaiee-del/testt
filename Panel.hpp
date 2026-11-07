#pragma once

#include <Geode/Geode.hpp>

namespace clippy::ui {

    // Opens the NOVA-style panel on top of the current scene (does nothing if it is already open).
    void openPanel();

    // Round face for the small pause-menu button.
    cocos2d::CCNode* panelButtonFace(float diameter);

}
