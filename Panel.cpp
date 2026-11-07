#include "Panel.hpp"

#include <Geode/utils/cocos.hpp>

#include <cmath>
#include <cstdio>

#include "Kit.hpp"

using namespace geode::prelude;
using namespace clippy::ui::kit;

namespace clippy::ui {

    namespace {

        constexpr float kSide = 96.f;

        // ------------------------------------------------------------------ page model (placeholders)

        struct RowSpec {
            enum class K { Toggle, Stepper, Dual, Buttons, Text };
            K k = K::Text;
            std::string title, sub, key, unit;
            int minV = 0, maxV = 100, step = 1, def = 0;
            bool on = false;
            std::string title2, key2, unit2;
            int min2 = 0, max2 = 100, step2 = 1, def2 = 0;
            std::vector<std::string> items;
        };

        struct CardSpec {
            int col = 0;
            std::string title;
            std::vector<RowSpec> rows;
        };

        RowSpec toggle(std::string t, std::string sub, std::string key, bool on) {
            RowSpec r; r.k = RowSpec::K::Toggle; r.title = std::move(t); r.sub = std::move(sub); r.key = std::move(key); r.on = on; return r;
        }
        RowSpec stepper(std::string t, std::string key, int mn, int mx, int step, int def, std::string unit = "") {
            RowSpec r; r.k = RowSpec::K::Stepper; r.title = std::move(t); r.key = std::move(key); r.minV = mn; r.maxV = mx; r.step = step; r.def = def; r.unit = std::move(unit); return r;
        }
        RowSpec dual(std::string t1, std::string k1, int d1, std::string t2, std::string k2, int d2) {
            RowSpec r; r.k = RowSpec::K::Dual; r.title = std::move(t1); r.key = std::move(k1); r.def = d1; r.unit = "%";
            r.title2 = std::move(t2); r.key2 = std::move(k2); r.def2 = d2; r.unit2 = "%"; r.step = 5; r.step2 = 5; return r;
        }
        RowSpec buttons(std::vector<std::string> items) {
            RowSpec r; r.k = RowSpec::K::Buttons; r.items = std::move(items); return r;
        }
        RowSpec text(std::vector<std::string> lines) {
            RowSpec r; r.k = RowSpec::K::Text; r.items = std::move(lines); return r;
        }

        struct PageSpec {
            std::string title;
            std::string pill;
            std::vector<CardSpec> cards;
        };

        std::vector<PageSpec> const& pages() {
            static std::vector<PageSpec> p = {
                {"Log", "CUSTOM", {}}, // special page, built by buildLog()
                {"Stage", "PREVIEW", {
                    {0, "Camera", {
                        toggle("Show Hitboxes", "Placeholder: your box and the blocks", "stage-hitboxes", false),
                        toggle("Show Trail", "Placeholder: the path of the player", "stage-trail", true),
                        toggle("Lock Zoom", "Placeholder: keep the camera still", "stage-lock", false),
                        stepper("Zoom", "stage-zoom", 50, 200, 10, 100, "%"),
                    }},
                    {0, "Stage Notes", {
                        text({"Placeholder text. Nothing here is wired yet.", "Lorem ipsum dolor sit amet."}),
                        buttons({"Reset Stage"}),
                    }},
                    {1, "Level", {
                        toggle("Show Percentage", "Placeholder: big percent on screen", "stage-percent", true),
                        toggle("Hide Attempts", "Placeholder: remove the attempts text", "stage-attempts", false),
                        stepper("Start From", "stage-start", 0, 95, 5, 0, "%"),
                    }},
                    {1, "Display", {
                        toggle("Show FPS", "Placeholder: frames per second", "stage-fps", true),
                        stepper("Opacity", "stage-opacity", 10, 100, 10, 100, "%"),
                    }},
                }},
                {"Modes", "PREVIEW", {
                    {0, "Practice", {
                        toggle("Smart Checkpoints", "Placeholder: place them for you", "modes-smart", true),
                        toggle("Show Checkpoints", "Placeholder: draw every checkpoint", "modes-show", false),
                        toggle("Keep Music Synced", "Placeholder: match the song to the run", "modes-sync", true),
                    }},
                    {0, "Speed", {
                        stepper("Speed", "modes-speed", 25, 200, 25, 100, "%"),
                        stepper("Frame Step", "modes-step", 1, 10, 1, 1),
                        buttons({"Slow", "Normal", "Fast"}),
                    }},
                    {1, "Assist Modes", {
                        toggle("Safe Mode", "Placeholder: progress is not saved", "modes-safe", false),
                        toggle("Auto Retry", "Placeholder: instant restart", "modes-retry", true),
                        stepper("Retry Delay", "modes-delay", 0, 20, 1, 3),
                    }},
                    {1, "Presets", {
                        text({"Placeholder presets for your favourite setups."}),
                        buttons({"Load", "Save", "Delete"}),
                    }},
                }},
                {"Tools", "EXTREME", {
                    {0, "Assists", {
                        toggle("Hitboxes", "Your box (spikes) and inner box (blocks)", "tools-hitboxes", false),
                        toggle("Show Rays", "Your hitbox, press / release paths", "tools-rays", false),
                        toggle("Alternate Assist", "Two-finger spam: every tap counts", "tools-alt", true),
                        toggle("Spam Guard", "Spamming: your clicks go in as made", "tools-spam", false),
                        stepper("Spam at (CPS)", "tools-cps", 1, 30, 1, 5),
                    }},
                    {0, "Your Stats", {
                        text({"Wave Challenge: 14 attempts, 10 deaths, best 100%", "All levels: 9173 attempts, 8223 deaths, 49 completions"}),
                        buttons({"Reset Click Guide"}),
                    }},
                    {1, "Autoplay", {
                        buttons({"Start Autoplay", "Show Panel"}),
                        text({"Plays the level for you. Nothing it plays is", "saved as progress."}),
                        text({"Nothing remembered on this level yet"}),
                        text({"Shared brain: placeholder-0000-learned", "Placeholder: learns from every player's play"}),
                        buttons({"Forget Level", "Undo Click"}),
                    }},
                    {1, "More Assists", {
                        toggle("Straight Fly", "Tap in a ship: it keeps your taps level", "tools-fly", true),
                        dual("From", "tools-from", 0, "To", "tools-to", 100),
                    }},
                }},
            };
            return p;
        }

        float rowHeight(RowSpec const& r) {
            switch (r.k) {
                case RowSpec::K::Toggle: return r.sub.empty() ? 16.f : 18.f;
                case RowSpec::K::Stepper: return 16.f;
                case RowSpec::K::Dual: return 16.f;
                case RowSpec::K::Buttons: return 18.f;
                default: return 8.f * static_cast<float>(r.items.size()) + 3.f;
            }
        }

        float cardHeight(CardSpec const& c) {
            float h = 14.f + 6.f;
            for (auto const& r : c.rows) h += rowHeight(r);
            return h;
        }

        struct Ctl {
            enum class K { Toggle, Minus, Plus, Button };
            K k = K::Button;
            std::string key, label;
            int minV = 0, maxV = 0, step = 1, def = 0;
            std::string unit;
            CCLabelBMFont* value = nullptr;
            CCDrawNode* knob = nullptr;
            CCScale9Sprite* track = nullptr;
            bool on = false;
        };

        // soft glowing disc: stacked translucent layers, premultiplied because CCDrawNode blends with ONE / ONE_MINUS_SRC_ALPHA
        void blob(CCDrawNode* n, float cx, float cy, float r, int hex, float alpha) {
            for (int i = 0; i < 7; ++i) {
                float t = static_cast<float>(i) / 6.f;
                float rr = r * (1.f - 0.62f * t);
                float a = alpha * 0.16f;
                auto c = col4(hex);
                disc(n, ccp(cx, cy), rr, ccc4f(c.r * a, c.g * a, c.b * a, a), 30);
            }
        }

        class NovaPanel : public FLAlertLayer {
        public:
            static NovaPanel* s_open;

            static void open() {
                if (s_open) return;
                auto layer = new NovaPanel();
                if (layer->setup()) {
                    layer->autorelease();
                    s_open = layer;
                    layer->show();
                    layer->animateIn();
                } else {
                    delete layer;
                }
            }

            ~NovaPanel() {
                if (s_open == this) s_open = nullptr;
            }

            void keyBackClicked() override { this->closePanel(); }

        private:
            float m_w = 0.f, m_h = 0.f;
            CCNode* m_root = nullptr;
            CCNode* m_chrome = nullptr;
            CCNode* m_body = nullptr;
            CCLabelBMFont* m_footer = nullptr;
            int m_section = 3; // opens on Tools, like the reference
            bool m_closing = false;
            std::vector<Ctl> m_ctl;

            // ------------------------------------------------------------ setup

            bool setup() {
                if (!FLAlertLayer::init(0)) return false;
                m_noElasticity = true;
                if (!m_mainLayer) {
                    m_mainLayer = CCLayer::create();
                    this->addChild(m_mainLayer);
                }
                this->setKeypadEnabled(true);
                buildAll();
                return true;
            }

            void buildAll() {
                updatePalette();
                auto const& p = pal();
                auto win = CCDirector::sharedDirector()->getWinSize();
                m_w = std::min(win.width - 50.f, 400.f);
                m_h = std::min(win.height - 60.f, 226.f);

                m_root = CCNode::create();
                m_root->setContentSize({m_w, m_h});
                m_root->setAnchorPoint({0.5f, 0.5f});
                m_root->setPosition({win.width / 2, win.height / 2});
                m_mainLayer->addChild(m_root);

                for (int i = 0; i < 3; ++i) {
                    float g = 9.f - 3.f * static_cast<float>(i);
                    auto glow = box(m_w + g * 2, m_h + g * 2, p.accent, 14 + i * 16);
                    glow->setPosition({m_w / 2, m_h / 2});
                    m_root->addChild(glow, 0);
                }
                auto shadow = box(m_w + 6.f, m_h + 6.f, 0x000000, p.light ? 40 : 120);
                shadow->setPosition({m_w / 2, m_h / 2 - 3.f});
                m_root->addChild(shadow, 1);
                auto edge = box(m_w + 2.f, m_h + 2.f, p.light ? 0xFFFFFF : p.edge, 255);
                edge->setPosition({m_w / 2, m_h / 2});
                m_root->addChild(edge, 2);
                auto base = box(m_w, m_h, p.surface, p.glass * 255 / 100);
                base->setPosition({m_w / 2, m_h / 2});
                m_root->addChild(base, 3);

                // pastel clouds behind the glass
                auto clouds = CCDrawNode::create();
                float a = p.light ? 0.95f : 0.62f;
                blob(clouds, m_w * 0.30f, m_h * 0.30f, 56.f, p.accent, a);
                blob(clouds, m_w * 0.78f, m_h * 0.74f, 60.f, mixHex(p.accent, 0x35D0E8, 0.65f), a);
                blob(clouds, m_w * 0.62f, m_h * 0.22f, 46.f, mixHex(p.accent, 0xFF5C9A, 0.6f), a * 0.9f);
                blob(clouds, m_w * 0.50f, m_h * 0.62f, 40.f, mixHex(p.accent, 0xFFFFFF, 0.5f), a * 0.7f);
                m_root->addChild(clouds, 4);

                m_body = CCNode::create();
                m_body->setContentSize({m_w, m_h});
                m_body->setAnchorPoint({0.f, 0.f});
                m_root->addChild(m_body, 6);
                m_chrome = CCNode::create();
                m_chrome->setContentSize({m_w, m_h});
                m_chrome->setAnchorPoint({0.f, 0.f});
                m_root->addChild(m_chrome, 7);

                buildChrome();
                buildPage();
            }

            // never rebuild from inside the callback of a button that is about to be destroyed
            void defer(SEL_CallFunc fn) {
                this->runAction(CCSequence::create(CCDelayTime::create(0.01f), CCCallFunc::create(this, fn), nullptr));
            }

            void doRebuildAll() {
                if (m_closing) return;
                if (m_root) m_root->removeFromParentAndCleanup(true);
                m_root = m_chrome = m_body = nullptr;
                m_footer = nullptr;
                buildAll();
            }

            void doApplySection() {
                if (m_closing) return;
                buildChrome();
                buildPage();
            }

            // ------------------------------------------------------------ animation / closing

            void animateIn() {
                auto win = CCDirector::sharedDirector()->getWinSize();
                this->setOpacity(0);
                this->runAction(CCFadeTo::create(0.16f, 70));
                m_root->setScale(0.92f);
                m_root->setPosition({win.width / 2, win.height / 2 - 10.f});
                m_root->runAction(CCEaseBackOut::create(CCScaleTo::create(0.26f, 1.f)));
                m_root->runAction(CCEaseExponentialOut::create(CCMoveTo::create(0.26f, ccp(win.width / 2, win.height / 2))));
            }

            void closePanel() {
                if (m_closing) return;
                m_closing = true;
                this->setKeypadEnabled(false);
                this->runAction(CCFadeTo::create(0.12f, 0));
                m_root->runAction(CCEaseIn::create(CCScaleTo::create(0.12f, 0.94f), 2.f));
                this->runAction(CCSequence::create(CCDelayTime::create(0.14f), CCCallFunc::create(this, callfunc_selector(NovaPanel::finishClose)), nullptr));
            }

            void finishClose() { this->removeFromParentAndCleanup(true); }
            void onClose(CCObject*) { closePanel(); }

            // ------------------------------------------------------------ chrome: sidebar, header, footer

            CCNode* navFace(char const* name, bool active) {
                auto const& p = pal();
                float w = kSide - 20.f;
                auto n = CCNode::create();
                n->setContentSize({w, 17.f});
                n->setAnchorPoint({0.5f, 0.5f});
                if (active) {
                    auto b = box(w, 17.f, p.light ? 0xFFFFFF : mixHex(p.card, 0xFFFFFF, 0.14f), 245);
                    b->setPosition({w / 2, 8.5f});
                    n->addChild(b, 0);
                }
                auto dot = CCDrawNode::create();
                disc(dot, ccp(9.f, 8.5f), 2.4f, col4(active ? p.accent : p.dim), 14);
                n->addChild(dot, 1);
                auto l = label(name, "chatFont.fnt", 0.6f, active ? (p.light ? 0x1B1E2E : p.text) : p.dim, {0.f, 0.5f});
                l->setPosition({17.f, 8.8f});
                n->addChild(l, 1);
                return n;
            }

            CCNode* modeFace(bool moon, bool active) {
                auto const& p = pal();
                auto n = CCNode::create();
                n->setContentSize({37.f, 13.f});
                n->setAnchorPoint({0.5f, 0.5f});
                auto b = box(37.f, 13.f, active ? p.accent : p.cardDeep);
                b->setPosition({18.5f, 6.5f});
                n->addChild(b, 0);
                auto d = CCDrawNode::create();
                int ink = active ? p.onAccent : p.text, back = active ? p.accent : p.cardDeep;
                if (moon) {
                    disc(d, ccp(18.5f, 6.5f), 3.6f, col4(ink), 16);
                    disc(d, ccp(20.f, 7.5f), 3.1f, col4(back), 16);
                } else {
                    disc(d, ccp(18.5f, 6.5f), 2.1f, col4(ink), 12);
                    for (int i = 0; i < 8; ++i) {
                        float a = static_cast<float>(i) * 0.7853982f;
                        bar(d, ccp(18.5f + std::cos(a) * 3.2f, 6.5f + std::sin(a) * 3.2f), ccp(18.5f + std::cos(a) * 4.5f, 6.5f + std::sin(a) * 4.5f), 0.9f, col4(ink));
                    }
                }
                n->addChild(d, 1);
                return n;
            }

            CCNode* swatchFace(int hex, bool selected) {
                auto const& p = pal();
                auto n = CCNode::create();
                n->setContentSize({11.f, 11.f});
                n->setAnchorPoint({0.5f, 0.5f});
                auto d = CCDrawNode::create();
                disc(d, ccp(5.5f, 5.5f), selected ? 5.5f : 4.9f, col4(selected ? (p.light ? 0x1B1E2E : 0xFFFFFF) : p.edge), 16);
                disc(d, ccp(5.5f, 5.5f), selected ? 4.2f : 4.4f, col4(hex), 16);
                n->addChild(d);
                return n;
            }

            void buildChrome() {
                auto const& p = pal();
                m_chrome->removeAllChildrenWithCleanup(true);
                m_footer = nullptr;

                float sideW = kSide - 8.f;
                auto side = box(sideW, m_h - 12.f, p.card, p.light ? 150 : 120);
                side->setPosition({6.f + sideW / 2, m_h / 2});
                m_chrome->addChild(side, 0);

                // logo
                auto logo = label("CLIPPY", "bigFont.fnt", 0.5f, p.accent, {0.5f, 0.5f});
                fit(logo, sideW - 14.f, 0.5f);
                logo->setPosition({6.f + sideW / 2, m_h - 21.f});
                m_chrome->addChild(logo, 3);
                auto ver = label("v2.0.0", "chatFont.fnt", 0.36f, p.dim, {0.5f, 0.5f});
                ver->setPosition({6.f + sideW / 2, m_h - 33.f});
                m_chrome->addChild(ver, 3);
                auto under = box(sideW - 16.f, 1.6f, p.accent, 200);
                under->setPosition({6.f + sideW / 2, m_h - 39.f});
                m_chrome->addChild(under, 3);

                auto menu = CCMenu::create();
                menu->setPosition({0.f, 0.f});
                m_chrome->addChild(menu, 10);

                char const* names[4] = {"Log", "Stage", "Modes", "Tools"};
                for (int i = 0; i < 4; ++i) {
                    auto item = CCMenuItemSpriteExtra::create(navFace(names[i], i == m_section), nullptr, this, menu_selector(NovaPanel::onNav));
                    item->setTag(i);
                    item->setPosition({6.f + sideW / 2, m_h - 56.f - 21.f * static_cast<float>(i)});
                    menu->addChild(item);
                }

                // theme card: dark / light, accent swatches, background swatches
                float cx = 6.f + sideW / 2, cy = 51.f;
                auto te = box(sideW - 8.f, 70.f, p.edge, 200);
                te->setPosition({cx, cy});
                m_chrome->addChild(te, 1);
                auto tc = box(sideW - 10.f, 68.f, p.light ? 0xFFFFFF : p.cardDeep, 235);
                tc->setPosition({cx, cy});
                m_chrome->addChild(tc, 2);

                auto dark = CCMenuItemSpriteExtra::create(modeFace(true, !p.light), nullptr, this, menu_selector(NovaPanel::onThemeMode));
                dark->setTag(3000);
                dark->setPosition({cx - 19.f, cy + 24.f});
                menu->addChild(dark);
                auto light = CCMenuItemSpriteExtra::create(modeFace(false, p.light), nullptr, this, menu_selector(NovaPanel::onThemeMode));
                light->setTag(3001);
                light->setPosition({cx + 19.f, cy + 24.f});
                menu->addChild(light);

                auto l1 = label("Accent", "chatFont.fnt", 0.32f, p.dim, {0.f, 0.5f});
                l1->setPosition({cx - 36.f, cy + 9.f});
                m_chrome->addChild(l1, 3);
                auto l2 = label("Background", "chatFont.fnt", 0.32f, p.dim, {0.f, 0.5f});
                l2->setPosition({cx - 36.f, cy - 11.f});
                m_chrome->addChild(l2, 3);

                int curA = std::clamp(static_cast<int>(Mod::get()->getSettingValue<int64_t>("theme-accent")), 0, 6);
                int curB = std::clamp(static_cast<int>(Mod::get()->getSettingValue<int64_t>("theme-bg")), 0, 6);
                for (int i = 0; i < 7; ++i) {
                    float x = cx - 33.f + 11.f * static_cast<float>(i);
                    auto a = CCMenuItemSpriteExtra::create(swatchFace(kAccents[i], i == curA), nullptr, this, menu_selector(NovaPanel::onSwatch));
                    a->setTag(1000 + i);
                    a->setPosition({x, cy - 1.f});
                    menu->addChild(a);
                    auto b = CCMenuItemSpriteExtra::create(swatchFace(kBackgrounds[i], i == curB), nullptr, this, menu_selector(NovaPanel::onSwatch));
                    b->setTag(2000 + i);
                    b->setPosition({x, cy - 21.f});
                    menu->addChild(b);
                }
                auto note = label("UI preview", "chatFont.fnt", 0.34f, p.dim, {0.5f, 0.5f});
                note->setPosition({cx, 9.f});
                m_chrome->addChild(note, 3);

                // header: title, accent line, pill, close
                auto const& page = pages()[static_cast<size_t>(m_section)];
                auto title = label(page.title, "bigFont.fnt", 0.4f, p.text, {0.f, 0.5f});
                title->setPosition({kSide + 8.f, m_h - 17.f});
                m_chrome->addChild(title, 3);
                auto line = box(m_w - kSide - 16.f, 1.4f, p.accent, 210);
                line->setPosition({kSide + 8.f + (m_w - kSide - 16.f) / 2, m_h - 28.f});
                m_chrome->addChild(line, 3);

                auto pill = pillFace(page.pill, 46.f, 12.f, p.light ? 0xFFFFFF : mixHex(p.card, 0xFFFFFF, 0.14f), p.text, false, false, 0.34f);
                pill->setPosition({m_w - 48.f, m_h - 17.f});
                m_chrome->addChild(pill, 3);
                auto close = CCMenuItemSpriteExtra::create(circleFace(14.f, p.cardDeep, iconClose(14.f, p.text)), nullptr, this, menu_selector(NovaPanel::onClose));
                close->setPosition({m_w - 14.f, m_h - 17.f});
                menu->addChild(close);

                // footer
                m_footer = label(kFooterText, "chatFont.fnt", 0.3f, p.dim, {0.f, 0.5f});
                m_footer->setPosition({kSide + 8.f, 9.f});
                m_chrome->addChild(m_footer, 3);
                auto gear = circleFace(12.f, p.accent, iconTune(12.f, p.onAccent), false);
                gear->setPosition({m_w - 14.f, 9.f});
                m_chrome->addChild(gear, 3);

                geode::cocos::handleTouchPriority(this);
            }

            static constexpr char const* kFooterText = "TAP X OR BACK TO CLOSE  -  UI PREVIEW";

            void flash(std::string const& msg) {
                if (!m_footer) return;
                m_footer->stopAllActions();
                m_footer->setString(msg.c_str());
                m_footer->runAction(CCSequence::create(CCDelayTime::create(1.4f), CCCallFunc::create(this, callfunc_selector(NovaPanel::resetFooter)), nullptr));
            }

            void resetFooter() {
                if (m_footer) m_footer->setString(kFooterText);
            }

            void onNav(CCObject* s) {
                m_section = static_cast<CCNode*>(s)->getTag();
                defer(callfunc_selector(NovaPanel::doApplySection));
            }

            void onThemeMode(CCObject* s) {
                Mod::get()->setSettingValue<std::string>("theme-mode", static_cast<CCNode*>(s)->getTag() == 3000 ? "Dark" : "Light");
                defer(callfunc_selector(NovaPanel::doRebuildAll));
            }

            void onSwatch(CCObject* s) {
                int tag = static_cast<CCNode*>(s)->getTag();
                if (tag >= 2000) Mod::get()->setSettingValue<int64_t>("theme-bg", tag - 2000);
                else Mod::get()->setSettingValue<int64_t>("theme-accent", tag - 1000);
                defer(callfunc_selector(NovaPanel::doRebuildAll));
            }

            // ------------------------------------------------------------ pages

            void buildPage() {
                m_body->removeAllChildrenWithCleanup(true);
                m_ctl.clear();
                auto menu = CCMenu::create();
                menu->setPosition({0.f, 0.f});
                m_body->addChild(menu, 30);

                if (m_section == 0) buildLog(menu);
                else buildCards(menu, pages()[static_cast<size_t>(m_section)]);
                geode::cocos::handleTouchPriority(this);
            }

            CCNode* cardBg(float x, float y, float w, float h) {
                auto const& p = pal();
                auto card = CCNode::create();
                card->setContentSize({w, h});
                card->setAnchorPoint({0.f, 0.f});
                card->setPosition({x, y});
                auto e = box(w + 1.f, h + 1.f, p.light ? 0xFFFFFF : p.edge, p.light ? 200 : 110);
                e->setPosition({w / 2, h / 2});
                card->addChild(e, 0);
                auto b = box(w, h, p.light ? 0xFFFFFF : p.card, p.light ? 170 : 150);
                b->setPosition({w / 2, h / 2});
                card->addChild(b, 1);
                return card;
            }

            void buildCards(CCMenu* menu, PageSpec const& page) {
                auto const& p = pal();
                float x0 = kSide + 8.f;
                float gap = 6.f;
                float colW = std::floor((m_w - kSide - 16.f - gap) / 2.f);
                float top = m_h - 34.f;
                float cursor[2] = {top, top};

                for (auto const& c : page.cards) {
                    int col = std::clamp(c.col, 0, 1);
                    float h = cardHeight(c);
                    float x = x0 + static_cast<float>(col) * (colW + gap);
                    float y = cursor[col] - h;
                    auto card = cardBg(x, y, colW, h);
                    m_body->addChild(card, 5);
                    auto t = label(c.title, "chatFont.fnt", 0.52f, p.text, {0.f, 0.5f});
                    t->setPosition({7.f, h - 8.f});
                    card->addChild(t, 3);

                    float ry = h - 14.f;
                    for (auto const& r : c.rows) {
                        float rh = rowHeight(r);
                        buildRow(card, menu, r, x, y, colW, ry, rh);
                        ry -= rh;
                    }
                    card->setScale(0.96f);
                    card->setPosition({x + colW * 0.02f, y + h * 0.02f});
                    card->runAction(CCEaseExponentialOut::create(CCScaleTo::create(0.28f, 1.f)));
                    card->runAction(CCEaseExponentialOut::create(CCMoveTo::create(0.28f, ccp(x, y))));
                    cursor[col] = y - gap;
                }
            }

            void buildRow(CCNode* card, CCMenu* menu, RowSpec const& r, float cardX, float cardY, float w, float top, float rh) {
                auto const& p = pal();
                float mid = top - rh / 2;
                // menu items live in the page menu, so convert card-local coordinates to page coordinates
                auto page = [&](float lx, float ly) { return ccp(cardX + lx, cardY + ly); };

                switch (r.k) {
                    case RowSpec::K::Toggle: {
                        bool hasSub = !r.sub.empty();
                        auto t = label(r.title, "chatFont.fnt", 0.46f, p.text, {0.f, 0.5f});
                        fit(t, w - 52.f, 0.46f);
                        t->setPosition({7.f, mid + (hasSub ? 3.5f : 0.f)});
                        card->addChild(t, 3);
                        if (hasSub) {
                            auto s = label(r.sub, "chatFont.fnt", 0.3f, p.dim, {0.f, 0.5f});
                            fit(s, w - 50.f, 0.3f);
                            s->setPosition({7.f, mid - 4.5f});
                            card->addChild(s, 3);
                        }
                        bool on = Mod::get()->getSavedValue<bool>("ph:" + r.key, r.on);
                        auto face = CCNode::create();
                        face->setContentSize({26.f, 13.f});
                        face->setAnchorPoint({0.5f, 0.5f});
                        auto track = box(26.f, 12.f, on ? p.accent : p.cardDeep);
                        track->setPosition({13.f, 6.5f});
                        face->addChild(track, 0);
                        auto knob = CCDrawNode::create();
                        disc(knob, ccp(0, 0), 4.9f, col4(p.light && !on ? 0x8A92B0 : 0xFFFFFF), 14);
                        knob->setPosition({on ? 19.5f : 6.5f, 6.5f});
                        face->addChild(knob, 1);

                        Ctl c; c.k = Ctl::K::Toggle; c.key = r.key; c.knob = knob; c.track = track; c.on = on;
                        m_ctl.push_back(c);
                        auto item = CCMenuItemSpriteExtra::create(face, nullptr, this, menu_selector(NovaPanel::onCtl));
                        item->setTag(static_cast<int>(m_ctl.size()) - 1);
                        item->setPosition(page(w - 7.f - 13.f, mid));
                        menu->addChild(item);
                        break;
                    }
                    case RowSpec::K::Stepper: {
                        auto t = label(r.title, "chatFont.fnt", 0.46f, p.text, {0.f, 0.5f});
                        fit(t, w - 82.f, 0.46f);
                        t->setPosition({7.f, mid});
                        card->addChild(t, 3);
                        float plusX = w - 7.f - 5.5f;
                        float valX = plusX - 5.5f - 3.f - 12.f;
                        float minusX = valX - 12.f - 3.f - 5.5f;
                        addStepper(card, menu, page, r.key, r.minV, r.maxV, r.step, r.def, r.unit, minusX, valX, plusX, mid);
                        break;
                    }
                    case RowSpec::K::Dual: {
                        float cur = 7.f;
                        auto t1 = label(r.title, "chatFont.fnt", 0.44f, p.text, {0.f, 0.5f});
                        t1->setPosition({cur, mid});
                        card->addChild(t1, 3);
                        cur += labelWidth(t1) + 4.f;
                        addStepper(card, menu, page, r.key, r.minV, r.maxV, r.step, r.def, r.unit, cur + 5.f, cur + 5.f + 3.f + 10.f + 5.f, cur + 5.f + 3.f + 20.f + 3.f + 5.f, mid, 20.f);
                        cur += 10.f + 3.f + 20.f + 3.f + 10.f + 5.f;
                        auto t2 = label(r.title2, "chatFont.fnt", 0.44f, p.text, {0.f, 0.5f});
                        t2->setPosition({cur, mid});
                        card->addChild(t2, 3);
                        cur += labelWidth(t2) + 4.f;
                        addStepper(card, menu, page, r.key2, r.min2, r.max2, r.step2, r.def2, r.unit2, cur + 5.f, cur + 5.f + 3.f + 10.f + 5.f, cur + 5.f + 3.f + 20.f + 3.f + 5.f, mid, 20.f);
                        break;
                    }
                    case RowSpec::K::Buttons: {
                        float n = static_cast<float>(r.items.size());
                        float bw = (w - 14.f - 4.f * (n - 1.f)) / n;
                        for (size_t i = 0; i < r.items.size(); ++i) {
                            float cx = 7.f + bw / 2 + static_cast<float>(i) * (bw + 4.f);
                            auto face = pillFace(r.items[i], bw, 14.f, p.light ? 0xFFFFFF : mixHex(p.card, 0xFFFFFF, 0.1f), p.text, false, true, 0.42f);
                            Ctl c; c.k = Ctl::K::Button; c.label = r.items[i];
                            m_ctl.push_back(c);
                            auto item = CCMenuItemSpriteExtra::create(face, nullptr, this, menu_selector(NovaPanel::onCtl));
                            item->setTag(static_cast<int>(m_ctl.size()) - 1);
                            item->setPosition(page(cx, mid));
                            menu->addChild(item);
                        }
                        break;
                    }
                    default: {
                        float ly = top - 5.f;
                        for (auto const& line : r.items) {
                            auto l = label(line, "chatFont.fnt", 0.3f, p.dim, {0.f, 0.5f});
                            fit(l, w - 14.f, 0.3f);
                            l->setPosition({7.f, ly});
                            card->addChild(l, 3);
                            ly -= 8.f;
                        }
                        break;
                    }
                }
            }

            template <class PageFn>
            void addStepper(CCNode* card, CCMenu* menu, PageFn page, std::string const& key, int mn, int mx, int step, int def, std::string const& unit,
                float minusX, float valX, float plusX, float y, float valW = 24.f) {
                auto const& p = pal();
                int v = Mod::get()->getSavedValue<int>("ph:" + key, def);
                auto value = label(std::to_string(v) + unit, "chatFont.fnt", 0.44f, p.accent);
                fit(value, valW, 0.44f);
                value->setPosition({valX, y});
                card->addChild(value, 3);

                for (int plus = 0; plus < 2; ++plus) {
                    auto face = circleFace(11.f, p.cardDeep, iconPlusMinus(11.f, p.text, plus == 1));
                    Ctl c; c.k = plus ? Ctl::K::Plus : Ctl::K::Minus; c.key = key; c.minV = mn; c.maxV = mx; c.step = step; c.def = def; c.unit = unit; c.value = value;
                    m_ctl.push_back(c);
                    auto item = CCMenuItemSpriteExtra::create(face, nullptr, this, menu_selector(NovaPanel::onCtl));
                    item->setTag(static_cast<int>(m_ctl.size()) - 1);
                    item->setPosition(page(plus ? plusX : minusX, y));
                    menu->addChild(item);
                }
            }

            void onCtl(CCObject* s) {
                int idx = static_cast<CCNode*>(s)->getTag();
                if (idx < 0 || idx >= static_cast<int>(m_ctl.size())) return;
                auto& c = m_ctl[static_cast<size_t>(idx)];
                auto const& p = pal();
                switch (c.k) {
                    case Ctl::K::Toggle: {
                        c.on = !c.on;
                        bool on = c.on;
                        Mod::get()->setSavedValue<bool>("ph:" + c.key, on);
                        c.knob->stopAllActions();
                        c.knob->runAction(CCEaseOut::create(CCMoveTo::create(0.12f, ccp(on ? 19.5f : 6.5f, 6.5f)), 2.f));
                        c.track->setColor(col(on ? p.accent : p.cardDeep));
                        break;
                    }
                    case Ctl::K::Minus:
                    case Ctl::K::Plus: {
                        int v = Mod::get()->getSavedValue<int>("ph:" + c.key, c.def);
                        v = std::clamp(v + (c.k == Ctl::K::Plus ? c.step : -c.step), c.minV, c.maxV);
                        Mod::get()->setSavedValue<int>("ph:" + c.key, v);
                        c.value->setString((std::to_string(v) + c.unit).c_str());
                        break;
                    }
                    case Ctl::K::Button:
                        flash("PLACEHOLDER: \"" + c.label + "\" is not wired yet");
                        break;
                }
            }

            // ------------------------------------------------------------ log page

            void buildLog(CCMenu* menu) {
                auto const& p = pal();
                float x = kSide + 8.f, w = m_w - kSide - 16.f;
                float y = 24.f, h = m_h - 34.f - 24.f;
                auto card = cardBg(x, y, w, h);
                m_body->addChild(card, 5);

                struct Line { char const* t; char const* tag; int color; char const* msg; };
                Line lines[8] = {
                    {"16.2s", "NOTE", 0, "input timing: placeholder text for the first line"},
                    {"16.2s", "FIRED", kMint, "press +1t (placeholder detail)"},
                    {"16.2s", "CHECK", kAmber, "plan placeholder: lorem ipsum dolor sit amet"},
                    {"16.2s", "BUFFER", kMint, "press +2t (8.3ms) [placeholder]"},
                    {"12.3s", "DEATH", kRec, "buffered, fired +1t, still died [placeholder]"},
                    {"11.9s", "FIRED", kMint, "press +1t (placeholder)"},
                    {"11.9s", "CHECK", kAmber, "plan placeholder: consectetur adipiscing elit"},
                    {"11.0s", "BUFFER", kMint, "press +2t (8.3ms) [placeholder]"},
                };
                float ly = h - 13.f;
                float step = (h - 20.f) / 7.f;
                for (auto const& ln : lines) {
                    auto t = label(ln.t, "chatFont.fnt", 0.36f, p.dim, {0.f, 0.5f});
                    t->setPosition({8.f, ly});
                    card->addChild(t, 3);
                    auto tag = label(ln.tag, "chatFont.fnt", 0.34f, ln.color ? ln.color : p.dim, {0.f, 0.5f});
                    tag->setPosition({38.f, ly});
                    card->addChild(tag, 3);
                    auto msg = label(ln.msg, "chatFont.fnt", 0.4f, ln.color == kRec ? kRec : p.text, {0.f, 0.5f});
                    fit(msg, w - 92.f, 0.4f);
                    msg->setPosition({80.f, ly});
                    card->addChild(msg, 3);
                    ly -= step;
                }

                char const* names[3] = {"Orbs", "Wave", "Clear"};
                for (int i = 0; i < 3; ++i) {
                    auto face = pillFace(names[i], 38.f, 13.f, i == 2 ? 0x5A2330 : (p.light ? 0xFFFFFF : mixHex(p.card, 0xFFFFFF, 0.1f)), i == 2 ? kRec : p.text, false, true, 0.4f);
                    Ctl c; c.k = Ctl::K::Button; c.label = names[i];
                    m_ctl.push_back(c);
                    auto item = CCMenuItemSpriteExtra::create(face, nullptr, this, menu_selector(NovaPanel::onCtl));
                    item->setTag(static_cast<int>(m_ctl.size()) - 1);
                    item->setPosition({m_w - 70.f + 42.f * static_cast<float>(i) - 42.f, 11.f});
                    menu->addChild(item);
                }
            }
        };

        NovaPanel* NovaPanel::s_open = nullptr;

    }

    void openPanel() {
        NovaPanel::open();
    }

    CCNode* panelButtonFace(float d) {
        updatePalette();
        auto icon = CCDrawNode::create();
        drawPaperclip(icon, d * 0.62f, 0.f, 0.f, true);
        return circleFace(d, pal().card, icon);
    }

}
