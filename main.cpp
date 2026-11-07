// Minixx's Clippy v2 - a small button in the pause menu opens the panel.
#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>

#include "Panel.hpp"

using namespace geode::prelude;

class $modify(ClippyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto win = CCDirector::sharedDirector()->getWinSize();
        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});

        auto button = CCMenuItemSpriteExtra::create(
            clippy::ui::panelButtonFace(24.f), nullptr, this, menu_selector(ClippyPauseLayer::onClippyPanel));
        // right column of the pause menu: just left of the third mod button (the one circled in red)
        button->setPosition({win.width - 59.f, win.height - 73.f});
        menu->addChild(button);
        this->addChild(menu, 20);
    }

    void onClippyPanel(CCObject*) {
        clippy::ui::openPanel();
    }
};
