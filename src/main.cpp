#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

static bool opt(char const* key) {
    return Mod::get()->getSavedValue<bool>(key, false);
}

static bool s_holding = false;

static char const* KEYS[4] = {"fly", "infjump", "god", "ghost"};
static char const* LABELS[4] = {
    "Ucma (dokun = yukari)",
    "Sinirsiz Ziplama",
    "Olumsuzluk",
    "Gorunmezlik + icinden gecme"
};

// ---- Mod menusu paneli ----
class ModPanel : public CCLayerColor {
public:
    static ModPanel* create() {
        auto ret = new ModPanel();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init() {
        if (!CCLayerColor::initWithColor({0, 0, 0, 170})) return false;
        this->setTouchEnabled(true);

        auto win = CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});
        menu->setTouchPriority(-501);
        this->addChild(menu);

        auto title = CCLabelBMFont::create("Mod Menu", "goldFont.fnt");
        title->setPosition({win.width / 2, win.height / 2 + 100.f});
        this->addChild(title);

        for (int i = 0; i < 4; i++) {
            float y = win.height / 2 + 55.f - i * 38.f;

            auto toggler = CCMenuItemToggler::createWithStandardSprites(
                this, menu_selector(ModPanel::onToggle), 0.7f
            );
            toggler->setTag(i);
            toggler->toggle(opt(KEYS[i]));
            toggler->setPosition({win.width / 2 - 130.f, y});
            menu->addChild(toggler);

            auto text = CCLabelBMFont::create(LABELS[i], "bigFont.fnt");
            text->setScale(0.5f);
            text->setAnchorPoint({0.f, 0.5f});
            text->setPosition({win.width / 2 - 100.f, y});
            this->addChild(text);
        }

        auto closeLbl = CCLabelBMFont::create("Kapat", "goldFont.fnt");
        auto closeBtn = CCMenuItemSpriteExtra::create(
            closeLbl, this, menu_selector(ModPanel::onClose)
        );
        closeBtn->setPosition({win.width / 2, win.height / 2 - 110.f});
        menu->addChild(closeBtn);

        return true;
    }

    void registerWithTouchDispatcher() override {
        CCDirector::sharedDirector()->getTouchDispatcher()
            ->addTargetedDelegate(this, -500, true);
    }

    bool ccTouchBegan(CCTouch*, CCEvent*) override { return true; }

    void onToggle(CCObject* sender) {
        auto t = static_cast<CCMenuItemToggler*>(sender);
        // dokununca isToggled() eski degeri verir
        Mod::get()->setSavedValue<bool>(KEYS[t->getTag()], !t->isToggled());
    }

    void onClose(CCObject*) {
        this->removeFromParentAndCleanup(true);
    }
};

// ---- Sag ustte siyah yuvarlak buton ----
class $modify(MenuPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto win = CCDirector::sharedDirector()->getWinSize();

        auto circle = CCNode::create();
        circle->setContentSize({40.f, 40.f});
        auto dot = CCDrawNode::create();
        dot->drawDot({20.f, 20.f}, 18.f, {0.f, 0.f, 0.f, 0.85f});
        circle->addChild(dot);
        auto lbl = CCLabelBMFont::create("M", "bigFont.fnt");
        lbl->setScale(0.5f);
        lbl->setPosition({20.f, 20.f});
        circle->addChild(lbl);

        auto btn = CCMenuItemSpriteExtra::create(
            circle, this, menu_selector(MenuPlayLayer::onModMenu)
        );
        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});
        menu->addChild(btn);
        btn->setPosition({win.width - 30.f, win.height - 30.f});
        this->addChild(menu, 1000);

        return true;
    }

    void onModMenu(CCObject*) {
        this->addChild(ModPanel::create(), 2000);
    }

    // Gorunmezlik
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (m_player1) m_player1->setVisible(!opt("ghost"));
        if (m_player2) m_player2->setVisible(!opt("ghost"));
    }

    // Olumsuzluk / icinden gecme
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if ((opt("god") || opt("ghost")) && obj != m_anticheatSpike) return;
        PlayLayer::destroyPlayer(player, obj);
    }
};

// ---- Ziplama ve ucma ----
class $modify(MenuPlayer, PlayerObject) {
    bool pushButton(PlayerButton btn) {
        s_holding = true;
        if (opt("infjump")) m_isOnGround = true;
        return PlayerObject::pushButton(btn);
    }

    bool releaseButton(PlayerButton btn) {
        s_holding = false;
        return PlayerObject::releaseButton(btn);
    }

    void update(float dt) {
        PlayerObject::update(dt);
        auto pl = PlayLayer::get();
        if (opt("fly") && pl && this == pl->m_player1) {
            m_yVelocity = s_holding ? 8.0 : -8.0;
        }
    }
};
