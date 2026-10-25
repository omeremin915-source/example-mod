#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

// ---- Ayarlar (kalıcı) ----
static bool opt(char const* key) {
    return Mod::get()->getSavedValue<bool>(key, false);
}

static bool s_holding = false; // ekrana dokunuluyor mu

// ---- Mod menüsü ----
class ModMenu : public geode::Popup<> {
protected:
    bool setup() override {
        this->setTitle("Mod Menu");

        addToggle("Ucma (dokun = yukari)", "fly", 0);
        addToggle("Sinirsiz Ziplama", "infjump", 1);
        addToggle("Olumsuzluk", "god", 2);
        addToggle("Gorunmezlik + icinden gecme", "ghost", 3);
        return true;
    }

    void addToggle(char const* label, char const* key, int row) {
        float y = -50.f - row * 34.f;

        auto toggler = CCMenuItemToggler::createWithStandardSprites(
            this, menu_selector(ModMenu::onToggle), 0.7f
        );
        toggler->toggle(opt(key));
        toggler->setUserObject("key", CCString::create(key));
        m_buttonMenu->addChildAtPosition(toggler, Anchor::TopLeft, {30.f, y});

        auto text = CCLabelBMFont::create(label, "bigFont.fnt");
        text->setScale(0.45f);
        text->setAnchorPoint({0.f, 0.5f});
        m_buttonMenu->addChildAtPosition(text, Anchor::TopLeft, {55.f, y});
    }

    void onToggle(CCObject* sender) {
        auto t = static_cast<CCMenuItemToggler*>(sender);
        auto key = static_cast<CCString*>(t->getUserObject("key"));
        // toggler tıklanınca isToggled() eski değeri verir
        Mod::get()->setSavedValue<bool>(key->getCString(), !t->isToggled());
    }

public:
    static ModMenu* create() {
        auto ret = new ModMenu();
        if (ret && ret->initAnchored(300.f, 210.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ---- Sağ üstte siyah yuvarlak buton ----
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
            circle, nullptr, this, menu_selector(MenuPlayLayer::onModMenu)
        );
        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});
        menu->addChild(btn);
        btn->setPosition({win.width - 30.f, win.height - 30.f});
        this->addChild(menu, 1000);

        return true;
    }

    void onModMenu(CCObject*) {
        ModMenu::create()->show();
    }

    // Görünmezlik
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (m_player1) m_player1->setVisible(!opt("ghost"));
        if (m_player2) m_player2->setVisible(!opt("ghost"));
    }

    // Ölümsüzlük / içinden geçme: ölüm çağrısını yoksay
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if ((opt("god") || opt("ghost")) && obj != m_anticheatSpike) return;
        PlayLayer::destroyPlayer(player, obj);
    }
};

// ---- Zıplama ve uçma ----
class $modify(MenuPlayer, PlayerObject) {
    bool pushButton(PlayerButton btn) {
        s_holding = true;
        if (opt("infjump")) m_isOnGround = true; // havadayken de zıplat
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
