#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

const float windowWidth = 800, windowHeight = 1000;
int totalEnemyEntities = 0;
sf::Texture* buttonTexture;
sf::Texture* heartTexture;
sf::Texture* bulletTexture2, * bulletTexture3, * bulletTexture4, * bulletTexture5,* laserTexture;
sf::Font* font;

struct EntityInfo {
    sf::Color color = sf::Color::White;
    sf::Vector2f pos = { 400, -100 };
    sf::Vector2f destination = { 400, 500 }, center = { 400, 500 };

    float scale = 1;
    float cooldown = 0.6;

    int hp = 100;
    int damage = 25;
    int level = 10;
    int type = 0;
    int xp = 10;

};

struct Animation {
    sf::Vector2u imageCount;
    sf::Vector2u currentImage;

    float totalTime;
    float switchTime;

    sf::IntRect uvRect;

    void create(sf::Texture* texture, sf::Vector2u imageCount, float switchTime) {
        this->imageCount = imageCount;
        this->switchTime = switchTime;
        totalTime = 0.0f;
        currentImage.x = 0;

        uvRect.width = texture->getSize().x / float(imageCount.x);
        uvRect.height = texture->getSize().y / float(imageCount.y);
    } 

    void updateCycle(int row, float deltaTime) {
        currentImage.y = row;
        totalTime += deltaTime;

        if (totalTime >= switchTime) {
            totalTime -= switchTime;
            ++(currentImage.x);

            if (currentImage.x >= imageCount.x) {
                currentImage.x = 0;
                ++currentImage.y;
                if (currentImage.y >= imageCount.y) {
                    currentImage.y = 0;
                }
            }
        }

        uvRect.left = currentImage.x * uvRect.width;
        uvRect.top = currentImage.y * uvRect.height;
    }

    void updateMove(int row, float deltaTime, int direction) {
        currentImage.y = row;
        totalTime += deltaTime;

        if (totalTime >= switchTime) {
            totalTime -= switchTime;
            if (direction == 0) {
                if (currentImage.x >= imageCount.x / 2 + 1) {
                    --(currentImage.x);
                }
                if (currentImage.x <= imageCount.x / 2 - 1) {
                    ++(currentImage.x);
                }
            }
            if (direction == 1) {
                if (currentImage.x < imageCount.x - 1) {
                    ++(currentImage.x);
                }
            }
            if (direction == -1) {
                if (currentImage.x > 0) {
                    --(currentImage.x);
                }
            }
        }

        uvRect.left = currentImage.x * uvRect.width;
        uvRect.top = currentImage.y * uvRect.height;
    }
};

struct Player {
    sf::Texture* texture;
    sf::Sprite sprite;

    int hp = 10;

    void loadTexture() {
        this->texture = new sf::Texture;
        if (!this->texture->loadFromFile("TEXTURES/player_animation.png", sf::IntRect(0, 0, 220, 42))) {
            std::cout << "FAILED TO OPEN PLAYER'S TEXTURE\n";
        }
        sprite.setTexture(*texture);
        sprite.setOrigin(22, 21);
    }

    void pos(float x, float y) {
        sf::Vector2f vect(x, y);
        this->sprite.setPosition(vect);
    }

    void applyInfo(EntityInfo info) {
        hp = info.hp;
        this->pos(info.pos.x, info.pos.y);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(this->sprite);
    }

    sf::Vector2f getPos() {
        return sprite.getPosition();
    }

    sf::FloatRect getHitbox() {
        return sprite.getGlobalBounds();
    }

};

struct Enemy {
    sf::Sprite sprite;
    sf::Texture* texture;

    Animation expl;

    int type = 0;
    int hp = 100;
    int level = 1;
    float xAcc = 0, yAcc = 0;
    float shotCooldownTime = 1, moveCooldownTime = 2;
    int xp = 10;
    float scaleS = 1;

    bool isMoving = false;

    sf::Vector2f destination, center;
    sf::Clock movingCooldown, shotCooldown;

    Enemy* next = nullptr;
    Enemy* prev = nullptr;

    void loadTexture(sf::Texture* enemyTexture) {  
        this->texture = enemyTexture;
        sprite.setTexture(*enemyTexture);
        sprite.setTextureRect(sf::IntRect(type % 4 * 32, type / 4 * 32, 32, 32));
        sprite.rotate(180);
        sprite.setOrigin(16, 16);
    }

    void move(sf::Vector2f vect) {
        this->sprite.move(vect);
    }

    void pos(sf::Vector2f vect) {
        this->sprite.setPosition(vect);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(this->sprite);
    }

    void scale(float x) {
        float y = x;
        sf::Vector2f vect(x, y);
        sprite.scale(vect);
    }

    void color(sf::Color color) {
        sprite.setColor(color);
    }

    void push(EntityInfo info) {
        Enemy* newEnemy = new Enemy;
       
        newEnemy->next = this->next;
        this->next = newEnemy;
        newEnemy->prev = this;
        if (newEnemy->next != nullptr) {
            newEnemy->next->prev = newEnemy;
        }

        newEnemy->type = info.type;

        newEnemy->loadTexture(texture);
        newEnemy->pos(info.pos);
        newEnemy->scale(info.scale);
        newEnemy->color(info.color);

        newEnemy->hp = info.hp;
        newEnemy->type = type;
        newEnemy->level = info.level;
        newEnemy->destination = info.destination;  
        newEnemy->center = info.center;
        newEnemy->xp = info.xp;

        newEnemy->shotCooldownTime = (float)(std::rand() % 200) / 200 + 0.5;
        newEnemy->moveCooldownTime = (float)(std::rand() % 600) / 200 + 1;
        newEnemy->scaleS = info.scale;


        ++totalEnemyEntities;
    }

    void erase(Enemy*& pointer) {
        pointer = nullptr;
        if (this->prev != nullptr) {
            pointer = this->prev;
            this->prev->next = this->next;
        }
        if (this->next != nullptr) {
            pointer = this->prev;
            this->next->prev = this->prev;
        }
        --totalEnemyEntities;
        delete this;
    }

    sf::FloatRect getHitbox() {
        return sprite.getGlobalBounds();
    }

    sf::Vector2f getPos() {
        return sprite.getPosition();
    }

};

struct Bullet {
    sf::Sprite sprite;
    sf::Texture* texture;
    Animation animation;

    int type = 0;
    int damage = 25;
    float cooldown = 0.6;

    Bullet* next = nullptr;
    Bullet* prev = nullptr;
    
    void loadTexture(sf::Texture* bulletTexture, int type) {  
        this->texture = bulletTexture;
        sprite.setTexture(*bulletTexture);
        if (type < 29) {
            sprite.setTextureRect(sf::IntRect(type * 15, 0, 15, 15));
        }
        if (type == 29) {
            sprite.setTextureRect(sf::IntRect(0, 0, 30, 11));
            sprite.setScale(2, 2);
        }
        if (type == 30) {
            sprite.setTextureRect(sf::IntRect(0, 0, 30, 13));
            sprite.setScale(2, 2);
        }
        if (type == 31) {
            sprite.setTextureRect(sf::IntRect(0, 0, 30, 15));
            sprite.setScale(2, 2);
        }
        if (type == 32) {
            sprite.setTextureRect(sf::IntRect(0, 0, 30, 16));
            sprite.setScale(3, 3);
        }
        
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    void pos(sf::Vector2f vect) {
        this->sprite.setPosition(vect);
    }

    void color(sf::Color color) {
        sprite.setColor(color);
    }

    void push(EntityInfo info) {
        Bullet* newBullet = new Bullet;

        newBullet->next = this->next;
        this->next = newBullet;
        newBullet->prev = this;
        if (newBullet->next != nullptr) {
            newBullet->next->prev = newBullet;
        }

        newBullet->type = info.type;
        newBullet->sprite.setScale(info.scale, info.scale);
        newBullet->loadTexture(texture, info.type);
        if (info.type == 29) {
            newBullet->animation.create(texture, sf::Vector2u(4, 1), 0.05);
            newBullet->sprite.setOrigin(15, 6.5);
            newBullet->sprite.rotate(270);
        }
        if (info.type == 30) {
            newBullet->animation.create(texture, sf::Vector2u(4, 1), 0.05);
            newBullet->sprite.setOrigin(15, 6.5);
            newBullet->sprite.rotate(270);
        }
        if (info.type == 31) {
            newBullet->animation.create(texture, sf::Vector2u(4, 1), 0.05);
            newBullet->sprite.setOrigin(15, 7.5);
            newBullet->sprite.rotate(270);
        }
        if (info.type == 32) {
            newBullet->animation.create(texture, sf::Vector2u(4, 1), 0.05);
            newBullet->sprite.setOrigin(15, 8);
            newBullet->sprite.rotate(270);
        }
        newBullet->pos(info.pos);
        newBullet->type = info.type;
        newBullet->damage = info.damage;
        newBullet->cooldown = info.cooldown;
        
    }

    void erase(Bullet*& pointer) {
        pointer = nullptr;
        if (this->prev != nullptr) {
            pointer = this->prev;
            this->prev->next = this->next;
        }
        if (this->next != nullptr) {
            pointer = this->prev;
            this->next->prev = this->prev;
        }
        delete this;
    }

    sf::FloatRect getHitbox() {
        return sprite.getGlobalBounds();
    }

    sf::Vector2f getPos() {
        return sprite.getPosition();
    }
};

struct Item {
    sf::Sprite sprite;
    sf::Texture* texture;

    Animation animation;

    int type = 0;

    Item* next = nullptr;
    Item* prev = nullptr;

    void loadTexture(sf::Texture* itemTexture) {  
        this->texture = itemTexture;
        sprite.setTexture(*itemTexture);
        if (this->type == 0) {
            sprite.setTextureRect(sf::IntRect(0, 0, 65, 70));
            animation.create(itemTexture, sf::Vector2u(8, 1), 0.1);
        }
        if (this->type == 1) {
            sprite.setTextureRect(sf::IntRect(0, 0, 65, 70));
            //animation.create(itemTexture, sf::Vector2u(8, 1), 0.1);
        }
        
    }

    void pos(sf::Vector2f pos) {
        sprite.setPosition(pos);
    }

    void push(sf::Texture* texture, EntityInfo info) {
        Item* newItem = new Item;
       
        newItem->next = this->next;
        this->next = newItem;
        newItem->prev = this;
        if (newItem->next != nullptr) {
            newItem->next->prev = newItem;
        }

        newItem->type = info.type;

        newItem->loadTexture(texture);
        newItem->pos(info.pos);  
        if (newItem->type == 1) {
            newItem->sprite.setColor(sf::Color(20, 0, 255));
        }
    }

    void erase(Item*& pointer) {
        pointer = nullptr;
        if (this->prev != nullptr) {
            pointer = this->prev;
            this->prev->next = this->next;
        }
        if (this->next != nullptr) {
            pointer = this->prev;
            this->next->prev = this->prev;
        }
        delete this;
    }

    sf::FloatRect getHitbox() {
        return sprite.getGlobalBounds();
    }

    sf::Vector2f getPos() {
        return sprite.getPosition();
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
};

struct Button {
    sf::Text text;
    sf::RectangleShape rectangle;
    sf::Color outColor = sf::Color(160, 158, 214), fillColor = sf::Color(111, 84, 149, 200);

    void create(sf::Vector2f pos, sf::String text) {
        rectangle.setOutlineThickness(5);
        rectangle.setSize(sf::Vector2f(400, 150));
        rectangle.setFillColor(fillColor);
        rectangle.setOutlineColor(outColor);
        rectangle.setOrigin(198.5, 75);
        rectangle.setPosition(pos);

        this->text.setFont(*font);
        this->text.setString(text);
        //this->text.Bold;
        this->text.setOrigin(this->text.getGlobalBounds().width / 2 + 5, this->text.getGlobalBounds().height / 2 + 7);
        this->text.setPosition(pos);
        this->text.setScale(1.5, 1.5);
        this->text.setFillColor(fillColor);
        this->text.setOutlineColor(outColor);
        this->text.setOutlineThickness(1);
        this->text.setLetterSpacing(1.5);
    }

    void color(bool onButton) {
        if (onButton) {
            rectangle.setOutlineColor(sf::Color::White);
            text.setOutlineColor(sf::Color::White);
        }
        else {
            rectangle.setOutlineColor(outColor);
            text.setOutlineColor(outColor);
        }

    }

    sf::FloatRect getHitbox() {
        return rectangle.getGlobalBounds();
    }

    void draw(sf::RenderWindow& window) {
        window.draw(rectangle);
        window.draw(text);
    }
};

Player player;
Enemy* enemies;
Bullet* friendlyBullets,* enemyBullets;
Item* powerups;

sf::Clock heatCooldown, damageCooldown, infiniteHeatTimer, textShowup, laserCooldown;

sf::Sprite background1, background2;
sf::Sprite ammoSquare1, ammoSquare2, ammoSquare3, ammoSquare4;
std::vector<sf::Sprite> health;

sf::RectangleShape pauseScreen;
sf::RectangleShape overHeatMain;
std::vector<sf::RectangleShape> overHeatBar;

std::vector<Button> pauseButtons, menuButtons, gameOverButtons;
sf::Text scoreText, pauseText, gameOverText, victoryText, gameOverScore, levelSelectText, levelNumbText, prevText, nextText, highscoreText, highscoreNumbText, back;
sf::Text waveText, waveNumb, curLevelText, curLevelNumbText, title;
Button levelSelectPlayButton, nextLevelButton;
sf::Sprite laser;

std::vector<std::vector<int>> level1(3, std::vector<int>(19, 0)), level2(3, std::vector<int>(19, 0)), level3(3, std::vector<int>(19, 0)), 
    level4(3, std::vector<int>(19, 0)), level5(5, std::vector<int>(19, 0)), level6(3, std::vector<int>(19, 0)), level7(3, std::vector<int>(19, 0)), level8(3, std::vector<int>(19, 0)),
    level9(3, std::vector<int>(19, 0)), level10(5, std::vector<int>(19, 0));
std::vector<EntityInfo> enemyTypesSorted, bulletTypes(4);

EntityInfo ammo, enemyAmmo, basicEnemy, bigEnemy, playerStart;

float x = 400, y = 500; 
float deltaTime = 0.0f, multiplier = 6000;
float xAcceleration = 0, yAcceleration = 0;
int xDirectionPlayer = 0, yDirectionPlayer = 0;
bool wasAccelX = false, wasAccelY = false;

int colorTheme = 0;
int difficultyLevel = 15;
int levelProgress = 1;
int score = 0;
int level = 0, wave = -1;
int threshold = 200;
bool gameOver = false, pause = false, inGame = false, inMainMenu = true, inLevelSelection = false, close = false, victory = false;
bool overheat = false, infiniteHeat = false, laserShot = false;
int heat = 0;
int ammoType = 0;
int levelsUnlocked = 1, highscore = 2000;



Animation playerAnimation, enemyAnimation, backgroundAnimation1, backgroundAnimation2;

void LoadFromFiles() {
    std::ifstream file;
    file.open("Save.txt");
    if (!file.is_open()) {
        std::cout << "FILE";
    }
    char c;
    bool switchs = false;
    highscore = 0;
    levelsUnlocked = 0;
    while ((c = file.get()) != '\n') {
        if ((c < '0' || c >'9') && c != ' ') {
            break;
        }
        if (c == ' ') {
            switchs = true;
            continue;
        }
        if (switchs) {
            levelsUnlocked = c - '0' + levelsUnlocked * 10;
            continue;
        }
        highscore = c - '0' + highscore * 10;
    }
    file.close();
    
}

void PreProcess(sf::Texture* enemyTexture, sf::Texture* bulletTexture1, sf::Texture* backgroundTexture1, sf::Texture* backgroundTexture2) {
    EntityInfo info;

    //player setup
    player.loadTexture();
    player.pos(0, 0);
    playerAnimation.create(player.texture, sf::Vector2u(5, 1), 0.1);

    //enemy setup
    if (!enemyTexture->loadFromFile("TEXTURES/enemies.png", sf::IntRect(0, 0, 128, 64))) {
        std::cout << "FAILED TO OPEN ENEMY'S TEXTURE\n";
    }
    enemies = new Enemy;
    enemies->texture = enemyTexture;

    info.type = 28;

    //bullet setup
    if (!bulletTexture1->loadFromFile("TEXTURES/Bullet_one.png", sf::IntRect(0, 0, 435, 15))) {
        std::cout << "FAILED TO OPEN BULLET'S TEXTURE\n";
    }
    if (!bulletTexture2->loadFromFile("TEXTURES/Bullet3.png", sf::IntRect(0, 0, 120, 15))) {
        std::cout << "FAILED TO OPEN BULLET'S TEXTURE 2\n";
    }
    if (!bulletTexture3->loadFromFile("TEXTURES/Bullet4.png", sf::IntRect(0, 0, 120, 15))) {
        std::cout << "FAILED TO OPEN BULLET'S TEXTURE 3\n";
    }
    if (!bulletTexture4->loadFromFile("TEXTURES/Bullet2.png", sf::IntRect(0, 0, 120, 15))) {
        std::cout << "FAILED TO OPEN BULLET'S TEXTURE 4\n";
    }
    if (!bulletTexture5->loadFromFile("TEXTURES/Bullet5.png", sf::IntRect(0, 0, 120, 15))) {
        std::cout << "FAILED TO OPEN BULLET'S TEXTURE 5\n";
    }
    if (!laserTexture->loadFromFile("TEXTURES/laser.png", sf::IntRect(0, 0, 36, 10))) {
        std::cout << "FAILED TO OPEN LASER'S TEXTURE";
    }
    friendlyBullets = new Bullet;
    friendlyBullets->loadTexture(bulletTexture2, 29);
    friendlyBullets->animation.create(bulletTexture2, sf::Vector2u(4, 1), 0.1);


    enemyBullets = new Bullet;
    enemyBullets->texture = bulletTexture1;

    laser.setTexture(*laserTexture);
    laser.setTextureRect(sf::IntRect(0, 0, 1100, 10));
    laser.setPosition(0, -10);
    laser.rotate(90);
    laser.setScale(2, 2);

    //background setup
    if (!backgroundTexture1->loadFromFile("TEXTURES/Background_.png")) {
        std::cout << "FAILED TO OPEN BACKGROUND'S TEXTURE\n";
    }
    if (!backgroundTexture2->loadFromFile("TEXTURES/Background2.png")) {
        std::cout << "FAILED TO OPEN BACKGROUND'S TEXTURE 2\n";
    }
    background1.setTexture(*backgroundTexture1);
    background1.scale(1.5, 1.35);
    background1.setColor(sf::Color(200, 200, 200, 150));
    background1.setColor(sf::Color(255, 215, 0, 150));
    backgroundAnimation1.create(backgroundTexture1, sf::Vector2u(13, 3), 0.03);

    background2.setTexture(*backgroundTexture2);
    background2.scale(1.65, 1.65);
    backgroundAnimation2.create(backgroundTexture2, sf::Vector2u(10, 9), 0.025);
    background2.setPosition(0, -100);

    //buttons
    buttonTexture = new sf::Texture;
    font = new sf::Font;
    if (!font->loadFromFile("Orbitron-VariableFont_wght.ttf")) {
        std::cout << "FAILED TO OPEN FONT\n";
    }
    if (!buttonTexture->loadFromFile("TEXTURES/button.png")) {
        std::cout << "FAILED TO OPEN BUTTON'S TEXTURE\n";
    }
    Button empt;
    pauseButtons.push_back(empt);
    pauseButtons[0].create(sf::Vector2f(400, 350), "CONTINUE");
    pauseButtons.push_back(empt);
    pauseButtons[1].create(sf::Vector2f(400, 600), "MAIN MENU");

    menuButtons.push_back(empt);
    menuButtons[0].create(sf::Vector2f(400, 300), "FREE MODE");
    menuButtons.push_back(empt);
    menuButtons[1].create(sf::Vector2f(400, 550), "STORY MODE");
    menuButtons.push_back(empt);
    menuButtons[2].create(sf::Vector2f(400, 800), "EXIT GAME");

    gameOverButtons.push_back(empt);
    gameOverButtons[0].create(sf::Vector2f(400, 350), "RETRY");
    gameOverButtons.push_back(empt);
    gameOverButtons[1].create(sf::Vector2f(400, 600), "MAIN MENU");
    nextLevelButton.create(sf::Vector2f(400, 350), "NEXT LEVEL");


    levelSelectPlayButton.create(sf::Vector2f(400, 800), "PLAY");

    //items & other ui
    sf::Sprite heart;
    if (!heartTexture->loadFromFile("TEXTURES/hearts)).png")) {
        std::cout << "FAILED TO OPEN HEART'S TEXTURE";
    }
    heart.setTexture(*heartTexture);
    heart.setTextureRect(sf::IntRect(0, 0, 65, 70));
    for (int i = 0; i < player.hp; ++i) {
        heart.setPosition(0 + 40 * i, 930);
        health.push_back(heart);
    }
    heart.setTextureRect(sf::IntRect(390, 0, 65, 70));
    heart.setPosition(0 + 40 * player.hp, 930);
    health.push_back(heart);
    //health[player.hp].setColor(sf::Color(100,200,255));

    powerups = new Item;

    pauseScreen.setFillColor(sf::Color(0, 0, 0, 150));
    pauseScreen.setSize(sf::Vector2f(800, 1000));

    scoreText.setString(std::to_string(score));
    scoreText.setFont(*font);
    scoreText.setOrigin(scoreText.getGlobalBounds().width, 0);
    scoreText.setPosition(790, 0);

    pauseText.setString("PAUSE");
    pauseText.setFont(*font);
    pauseText.setScale(1.5, 1.5);
    pauseText.setOrigin(pauseText.getGlobalBounds().width / 2, pauseText.getGlobalBounds().height / 2);
    pauseText.setPosition(440, 200);

    gameOverText.setString("GAME OVER");
    gameOverText.setFont(*font);
    gameOverText.setScale(1.5, 1.5);
    gameOverText.setOrigin(pauseText.getGlobalBounds().width / 2, pauseText.getGlobalBounds().height / 2);
    gameOverText.setPosition(380, 200);

    victoryText.setString("VICTORY");
    victoryText.setFont(*font);
    victoryText.setScale(1.5, 1.5);
    victoryText.setOrigin(pauseText.getGlobalBounds().width / 2, pauseText.getGlobalBounds().height / 2);
    victoryText.setPosition(410, 200);

    gameOverScore.setFont(*font);
    gameOverScore.setScale(1.5, 1.5);

    overHeatMain.setFillColor(sf::Color::Transparent);
    overHeatMain.setOutlineColor(sf::Color(200, 200, 200));
    overHeatMain.setOutlineThickness(4);
    overHeatMain.setSize(sf::Vector2f(200, 40));
    overHeatMain.setPosition(20, 870);

    sf::RectangleShape emptRect;
    for (int i = 0; i < 10; ++i) {
        overHeatBar.push_back(emptRect);
        overHeatBar[i].setFillColor(sf::Color(150, 150, 150, 100));
        overHeatBar[i].setPosition(20 + 20 * i, 870);
        overHeatBar[i].setSize(sf::Vector2f(20, 40));
    }

    levelSelectText.setFont(*font);
    levelSelectText.setString("LEVEL: ");
    levelSelectText.setScale(1.5, 1.5);
    levelSelectText.setOrigin(levelSelectText.getGlobalBounds().width / 2 + 20, 0);
    levelSelectText.setPosition(400, 200);
    levelSelectText.setFillColor(sf::Color::White);

    levelNumbText.setFont(*font);
    levelNumbText.setString("1");
    levelNumbText.setScale(1.5, 1.5);
    levelNumbText.setOrigin(levelSelectText.getGlobalBounds().width / 2 + 20, 0);
    levelNumbText.setPosition(750, 200);

    prevText.setFont(*font);
    prevText.setScale(1.5, 1.5);
    prevText.setString("PREV");
    prevText.setPosition(200, 650);

    nextText.setFont(*font);
    nextText.setScale(1.5, 1.5);
    nextText.setString("NEXT");
    nextText.setPosition(470, 650);

    back.setFont(*font);
    back.setScale(1.1, 1.1);
    back.setString("BACK");
    back.setPosition(0, 0);

    highscoreText.setFont(*font);
    highscoreText.setScale(1.1, 1.1);
    highscoreText.setString("HIGHSCORE: ");

    highscoreNumbText.setFont(*font);
    highscoreNumbText.setScale(1.1, 1.1);
    highscoreNumbText.setString(std::to_string(highscore));
    highscoreNumbText.setPosition(highscoreText.getGlobalBounds().width + 10, 0);

    ammoSquare1.setTexture(*bulletTexture2);
    ammoSquare1.setTextureRect(sf::IntRect(90, 0, 30, 11));
    ammoSquare1.setPosition(0, 930);
    ammoSquare1.setScale(3, 3);

    ammoSquare2.setTexture(*bulletTexture3);
    ammoSquare2.setTextureRect(sf::IntRect(90, 0, 30, 13));
    ammoSquare2.setPosition(90, 930);
    ammoSquare2.setScale(3, 3);

    ammoSquare3.setTexture(*bulletTexture4);
    ammoSquare3.setTextureRect(sf::IntRect(90, 0, 30, 15));
    ammoSquare3.setPosition(180, 930);
    ammoSquare3.setScale(3, 3);

    ammoSquare4.setTexture(*bulletTexture5);
    ammoSquare4.setTextureRect(sf::IntRect(30, 0, 30, 16));
    ammoSquare4.setPosition(270, 920);
    ammoSquare4.setScale(3, 3);

    waveText.setFont(*font);
    waveText.setString("WAVE ");
    waveText.setScale(1.5, 1.5);
    waveText.setOrigin(waveText.getGlobalBounds().width / 2, waveText.getGlobalBounds().height / 2);
    waveText.setPosition(425, 400);

    waveNumb.setFont(*font);
    waveNumb.setString(std::to_string(wave));
    waveNumb.setScale(1.5, 1.5);
    waveNumb.setOrigin(waveText.getGlobalBounds().width / 2, waveText.getGlobalBounds().height / 2);
    waveNumb.setPosition(625, 400);

    curLevelText.setFont(*font);
    curLevelText.setScale(1.1, 1.1);
    curLevelText.setString("LEVEL: ");

    curLevelNumbText.setFont(*font);
    curLevelNumbText.setScale(1.1, 1.1);
    curLevelNumbText.setString(std::to_string(level));
    curLevelNumbText.setPosition(curLevelText.getGlobalBounds().width + 10, 0);

    title.setFont(*font);
    title.setScale(3, 3);
    title.setOutlineColor(sf::Color((255, 87, 51)));
    title.setOutlineThickness(2);
    title.setString("ASTRO");
    title.setPosition(220, 70);
    title.setFillColor(sf::Color(93, 63, 211, 100));


}

void CreateLevels() {
    level1[0] = { 5 };
    level1[1] = { 3, 0, 0, 1 };
    level1[2] = { 5, 0, 0, 3, 0, 0, 2 };

    level2[0] = { 3, 0, 0,
                2, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0 };
    level2[1] = { 4, 0, 0,
                2, 0, 0,
                0, 0, 0,
                1, 0, 0,
                0, 0, 0 };
    level2[2] = { 4, 0, 0,
                0, 0, 0,
                0, 0, 0,
                2, 0, 0,
                1, 0, 0 };

    level3[0] = { 5, 0, 0,
                3, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0 };
    level3[1] = { 0, 3, 0,
                0, 1, 0,
                0, 0, 0,
                1, 0, 0,
                0, 0, 0 };
    level3[2] = { 5, 0, 0,
                2, 0, 0,
                0, 0, 0,
                0, 1, 0,
                0, 0, 0 };

    level4[0] = { 4, 0, 0,
                0, 2, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0 };
    level4[1] = { 0, 0, 0,
                0, 0, 0,
                10, 0, 0,
                1, 0, 0,
                0, 0, 0 };
    level4[2] = { 2, 0, 0,
                0, 0, 0,
                5, 0, 0,
                0, 0, 0,
                0, 1, 0 };

    level5[0] = { 5, 0, 0,
                3, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                1, 0, 0};
    level5[1] = { 5, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 2, 0,
                0, 0, 0};
    level5[2] = { 0, 0, 0,
                0, 0, 0,
                6, 0, 0,
                3, 0, 0,
                1, 0, 0,
                0, 0, 0};
    level5[3] = { 0, 0, 2,
                0, 0, 1,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0};
    level5[4] = { 0, 0, 0,
                0, 0, 2,
                0, 0, 0,
                0, 0, 3,
                0, 1, 0,
                0, 0, 0};

    level6[0] = { 5, 0, 1,
                0, 0, 1,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0 };
    level6[1] = { 0, 3, 0,
                5, 0, 0,
                0, 0, 1,
                0, 0, 0,
                0, 0, 0 };
    level6[2] = { 0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 3, 0,
                0, 2, 0 };

    level7[0] = { 0, 0, 3,
                0, 0, 0,
                10, 0, 0,
                0, 0, 0,
                1, 0, 0, 
                0, 0, 0};
    level7[1] = { 0, 0, 3,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                2, 0, 0};
    level7[2] = { 0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 4,
                0, 0, 0,
                0, 2, 0};

    level8[0] = { 10, 0, 0,
                5, 0, 0,
                0, 0, 0,
                2, 0, 0,
                1, 0, 0,
                0, 0, 0};
    level8[1] = { 0, 0, 0,
                0, 0, 0,
                20, 0, 0,
                0, 0, 0,
                0, 0, 0,
                1, 0, 0};
    level8[2] = { 0, 0, 0,
                0, 0, 0,
                0, 0, 3,
                0, 0, 0,
                0, 0, 0,
                0, 1, 0};

    level9[0] = { 0, 0, 3,
                0, 0, 2,
                0, 0, 4,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0};
    level9[1] = { 0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                5, 0, 0,
                0, 0, 0};
    level9[2] = { 10, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 1};

    level10[0] = { 0, 0, 0,
                0, 0, 0,
                0, 0, 5,
                0, 0, 0,
                0, 0, 0,
                0, 1, 0};
    level10[1] = { 0, 5, 0,
                0, 0, 1,
                0, 0, 0,
                0, 0, 0,
                0, 2, 0,
                0, 1, 0};
    level10[2] = { 0, 0, 0,
                0, 0, 0,
                5, 5, 0,
                0, 0, 3,
                0, 0, 0,
                0, 0, 0};
    level10[3] = { 0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 2, 0,
                0, 0, 1};
    level10[4] = { 0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                1};
}
    /*
    types of enemies :
1 - basic, 20 hp, score - +25, scale 1, level 1
    green, 60 hp, score +45, scale 1, level 5
    red, 70 hp, score +50, scale 1, level 10
2 - basic 100 hp, score +50, scale 1.5, level 1
    green 400 hp, score +200, scale 1.5, level 3
    red 550 hp, score +200, scale 1.5, level 5
3 - basic 10 hp, score +25, scale 0.95, level 5
    green 
    red 50 hp, score +200, scale 0.95, level 18
4 - basic 300 hp, score +500, scale 2, level 5  
    green 
    red 1000 hp, score +1000, scale 2, level 14
5 - basic 250 hp, score +175, scale 1, level 5
    green 500 hp, score +250, scale 1, level 14 
    red
6 - basic 300 hp, score +300, scale 1.2, level 10
    green 600 hp, score +1200, scale 1.2, level 15
    red 1500 hp, score +2000, scale 1.2, level 17
7 - boss 50000 hp, score +1000000, scale 5
    */

void CreateEnemyTypes() {
    EntityInfo empt;

    empt.type = 0;

    empt.hp = 20;
    empt.xp = 25;
    empt.scale = 1;
    empt.level = 1;

    enemyTypesSorted.push_back(empt);

    empt.color = (sf::Color::Green);
    empt.hp = 60;
    empt.xp = 100;
    empt.scale = 1;
    empt.level = 5;

    enemyTypesSorted.push_back(empt);

    empt.color = (sf::Color::Red);
    empt.hp = 70;
    empt.xp = 1000;
    empt.scale = 1;
    empt.level = 10;

    enemyTypesSorted.push_back(empt);

    empt.type = 1;

    empt.hp = 100;
    empt.xp = 75;
    empt.scale = 1.5;
    empt.level = 1;
    empt.color = sf::Color::White;

    enemyTypesSorted.push_back(empt);

    empt.hp = 400;
    empt.xp = 200;
    empt.scale = 1.5;
    empt.level = 3;
    empt.color = sf::Color::Green;

    enemyTypesSorted.push_back(empt);

    empt.hp = 550;
    empt.xp = 1500;
    empt.scale = 1.5;
    empt.level = 5;
    empt.color = sf::Color::Red;

    enemyTypesSorted.push_back(empt);

    empt.type = 2;

    empt.hp = 10;
    empt.xp = 45;
    empt.scale = 0.95;
    empt.level = 5;
    empt.color = sf::Color::White;

    enemyTypesSorted.push_back(empt);

    enemyTypesSorted.push_back(empt);

    empt.hp = 50;
    empt.xp = 1200;
    empt.scale = 0.95;
    empt.level = 18;
    empt.color = sf::Color::Red;

    enemyTypesSorted.push_back(empt);

    empt.type = 3;

    empt.hp = 300;
    empt.xp = 500;
    empt.scale = 2;
    empt.level = 5;
    empt.color = sf::Color::White;

    enemyTypesSorted.push_back(empt);

    enemyTypesSorted.push_back(empt);

    empt.hp = 1000;
    empt.xp = 5000;
    empt.scale = 2;
    empt.level = 14;
    empt.color = sf::Color::Red;

    enemyTypesSorted.push_back(empt);

    empt.type = 4;

    empt.hp = 250;
    empt.xp = 175;
    empt.scale = 3;
    empt.level = 5;
    empt.color = sf::Color::White;

    enemyTypesSorted.push_back(empt);

    empt.hp = 500;
    empt.xp = 250;
    empt.scale = 3;
    empt.level = 14;
    empt.color = sf::Color::Green;

    enemyTypesSorted.push_back(empt);

    enemyTypesSorted.push_back(empt);

    empt.type = 5;

    empt.hp = 300;
    empt.xp = 300;
    empt.scale = 3;
    empt.level = 10;
    empt.color = sf::Color::White;

    enemyTypesSorted.push_back(empt);

    empt.hp = 600;
    empt.xp = 1200;
    empt.scale = 3;
    empt.level = 15;
    empt.color = sf::Color::Green;

    enemyTypesSorted.push_back(empt);

    empt.hp = 1500;
    empt.xp = 7000;
    empt.scale = 3;
    empt.level = 17;
    empt.color = sf::Color::Red;

    enemyTypesSorted.push_back(empt);

    empt.type = 6;

    empt.hp = 10000;
    empt.xp = 1000000;
    empt.scale = 5;
    empt.level = 20;
    empt.color = sf::Color::White;

    enemyTypesSorted.push_back(empt);


    /*
    types of enemies :
1 - basic, 20 hp, score - +25, scale 1, level 1
    green, 60 hp, score +45, scale 1, level 5
    red, 70 hp, score +50, scale 1, level 10
2 - basic 100 hp, score +50, scale 1.5, level 1
    green 400 hp, score +200, scale 1.5, level 3
    red 550 hp, score +200, scale 1.5, level 5
3 - basic 10 hp, score +25, scale 0.95, level 5
    green 
    red 50 hp, score +200, scale 0.95, level 18
4 - basic 300 hp, score +500, scale 2, level 5  
    green 
    red 1000 hp, score +1000, scale 2, level 14
5 - basic 250 hp, score +175, scale 1, level 5
    green 500 hp, score +250, scale 1, level 14 
    red
6 - basic 300 hp, score +300, scale 1.2, level 10
    green 600 hp, score +1200, scale 1.2, level 15
    red 1500 hp, score +2000, scale 1.2, level 17
7 - boss 50000 hp, score +1000000, scale 5
    */
}

void CreateBulletTypes() {
    EntityInfo info;

    bulletTypes[0].type = 29;
    bulletTypes[0].damage = 10;
    bulletTypes[0].cooldown = 0.6;

    bulletTypes[1].type = 30;
    bulletTypes[1].damage = 50;
    bulletTypes[1].cooldown = 0.5;


    bulletTypes[2].type = 31;
    bulletTypes[2].damage = 100;
    bulletTypes[2].cooldown = 0.4;


    bulletTypes[3].type = 32;
    bulletTypes[3].damage = 400;
    bulletTypes[3].cooldown = 0.3;

}

void CreateWave() {
    laserCooldown.restart();
    std::vector<std::vector<int>> curLevel;
    if (level == 1) {
        curLevel = level1;
    }
    if (level == 2) {
        curLevel = level2;
    }
    if (level == 3) {
        curLevel = level3;
    }
    if (level == 4) {
        curLevel = level4;
    }
    if (level == 5) {
        curLevel = level5;
    }
    if (level == 6) {
        curLevel = level6;
    }
    if (level == 7) {
        curLevel = level7;
    }
    if (level == 8) {
        curLevel = level8;
    }
    if (level == 9) {
        curLevel = level9;
    }
    if (level == 10) {
        curLevel = level10;
    }
    if (wave >= curLevel.size()) {
        victory = true;
        if (level == levelsUnlocked) {
            ++levelsUnlocked;
        }
        gameOver = true;
        return;
    }
    for (int i = 0; i < curLevel[wave].size(); ++i) {
        for (int j = 0; j < curLevel[wave][i]; ++j) {
            float destX = std::rand() % 650 + 50, destY = std::rand() % 100 + 60;
            enemyTypesSorted[i].destination = sf::Vector2f(destX, destY);
            enemyTypesSorted[i].center = sf::Vector2f(destX, destY);
            enemies->push(enemyTypesSorted[i]);
        }
    }
}

void SwitchAmmoType() {
    if (ammoType == 0) {
        friendlyBullets->texture = bulletTexture2;
        ammoSquare1.setColor(sf::Color::Green);
    }
    else {
        ammoSquare1.setColor(sf::Color::White);
    }
    if (ammoType == 1) {
        ammoSquare2.setColor(sf::Color::Green);
        friendlyBullets->texture = bulletTexture3;
    }
    else {
        ammoSquare2.setColor(sf::Color::White);
    }
    if (ammoType == 2) {
        ammoSquare3.setColor(sf::Color::Green);
        friendlyBullets->texture = bulletTexture4;
    }
    else {
        ammoSquare3.setColor(sf::Color::White);
    }
    if (ammoType == 3) {
        ammoSquare4.setColor(sf::Color::Green);
        friendlyBullets->texture = bulletTexture5;
    }
    else {
        ammoSquare4.setColor(sf::Color::White);
    }
    ammo = bulletTypes[ammoType];
}

void EnemyShot(Enemy* enemy) {
    enemyAmmo.scale = enemy->scaleS * 2;
    enemyBullets->push(enemyAmmo);
    enemyBullets->next->pos(enemy->getPos());
}

void PlayerShot() {
    if (damageCooldown.getElapsedTime().asSeconds() < 1) {
        return;
    }
    if (!overheat && !infiniteHeat) {
        if (ammo.type == 29) {
            ++heat;
        }
        
        if (ammo.type == 30) {
            if (heat == 9) {
                heat = 10;
            }
            else {
                heat += 2;
            }
            
        }
        if (ammo.type == 31) {
            if (heat + 3 < 10) {
                heat += 3;
            }
            else {
                heat = 10;
            }
        }
        if (ammo.type == 32) {
            if (heat + 4 < 10) {
                heat += 4;
            }
            else {
                heat = 10;
            }
        }
    }
    if (!overheat || infiniteHeat) {

        friendlyBullets->push(ammo);
        friendlyBullets->next->pos(player.getPos());
    }
    if (heat >= 10) {
        overheat = true;
    }
    if (infiniteHeatTimer.getElapsedTime().asSeconds() > 10) {
        infiniteHeat = false;
    }
}

void PlayerHurt(int damage = 1) {
    if (damageCooldown.getElapsedTime().asSeconds() < 1) {
        return;
    }
    player.hp -= damage;
    for (int i = player.hp; i < 10; ++i) {
        health[i].setColor(sf::Color(100, 200, 255));
    }
    if (player.hp <= 0) {
        gameOver = true;
    }
    damageCooldown.restart();
    std::cout << player.hp;
}

void SpawnItem(sf::Vector2f pos) {
    int random = std::rand() % 10;
    if (random == 1 && !infiniteHeat) {
        EntityInfo info;
        info.pos = pos;
        info.type = 1;
        powerups->push(heartTexture, info);
        return;
    }
    if (player.hp < 4) {
        int random = std::rand() % 2;
        if (random == 1) {
            EntityInfo info;
            info.pos = pos;
            powerups->push(heartTexture, info);
        }
    }
    else if (player.hp < 10) {
        int random = std::rand() % 20;
        if (random == 1) {
            EntityInfo info;
            info.pos = pos;
            powerups->push(heartTexture, info);
        }
    }
    else if (player.hp == 10) {
        int random = std::rand() % 40;
        if (random == 1) {
            EntityInfo info;
            info.pos = pos;
            powerups->push(heartTexture, info);
        }
    }
}

void PickupItem(Item*& item) {
    if (item->type == 0) {
        if (player.hp < 11) {
            player.hp += 1;
            if (player.hp <= 10) {
                health[player.hp - 1].setColor(sf::Color::White);
            }
        }
        item->erase(item);
    }
    if (item->type == 1) {
        infiniteHeat = true;
        infiniteHeatTimer.restart();
        heat = 0;
        item->erase(item);
    }
}

void AiWork(sf::Clock& spawnCooldown) {
    if (level == 10 && wave == 4) {
        if (laserCooldown.getElapsedTime().asSeconds() < 0.7) {
            laser.setColor(sf::Color(255, 255, 255, 70));
        }
        else if (laserCooldown.getElapsedTime().asSeconds() < 1.1) {
            laserShot = true;
            laser.setColor(sf::Color(255, 255, 255));
        }
        else {
            laserShot = false;
            laser.setPosition(std::rand() % 950 + 20, -10);
            laserCooldown.restart();
        }
    }
    else {
        laser.setColor(sf::Color::Transparent);
        laserShot = false;
    }
    int opportunity;
    Enemy* processEnemies = enemies->next;

    while (processEnemies != nullptr) {
        opportunity = std::rand() % 21;
        if (processEnemies->movingCooldown.getElapsedTime().asSeconds() < processEnemies->moveCooldownTime) {
            processEnemies = processEnemies->next;
            continue;
        }
        float destX = std::rand() % 650 + 50, destY = std::rand() % 500 + 40;
        if (processEnemies->level >= opportunity) {
            processEnemies->xAcc = 0;
            processEnemies->yAcc = 0;
            processEnemies->isMoving = false;
            processEnemies->destination = sf::Vector2f(destX, destY);
            processEnemies->center = sf::Vector2f(destX, destY);
        }
        processEnemies->movingCooldown.restart();
        processEnemies = processEnemies->next;
    }

    opportunity = std::rand() % 21;
    if (opportunity <= difficultyLevel && spawnCooldown.getElapsedTime().asMilliseconds() > (21 - difficultyLevel) * 200) {
        float destX = std::rand() % 650 + 50, destY = std::rand() % 100 + 60;
        enemyTypesSorted[opportunity].destination = sf::Vector2f(destX, destY);
        enemyTypesSorted[opportunity].center = sf::Vector2f(destX, destY);
        
        if (totalEnemyEntities < 50) {
            enemies->push(enemyTypesSorted[opportunity]);
        }
        spawnCooldown.restart();
    }

    processEnemies = enemies->next;
    while (processEnemies != nullptr) {
        opportunity = std::rand() % 21;
        if (processEnemies->shotCooldown.getElapsedTime().asSeconds() < processEnemies->shotCooldownTime) {
            processEnemies = processEnemies->next;
            continue;
        }
        if (processEnemies->level >= opportunity) {
            EnemyShot(processEnemies);
        }
        processEnemies->shotCooldown.restart();
        processEnemies = processEnemies->next;
    }
}

void ProcessPlayerMovement(int& xDirection, int& yDirection, float& xAcceleration, float& yAcceleration, bool& wasAccelX, bool& wasAccelY) {
    xDirection = 0;
    wasAccelX = false;
    wasAccelY = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        xDirection += 1;
        if (xAcceleration < 0.15) {
            xAcceleration += 0.0001;
        } 
        wasAccelX = !wasAccelX;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        if (yAcceleration < 0.25) {
            yAcceleration += 0.0001;
        }
        wasAccelY = !wasAccelY;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        xDirection -= 1;
        if (xAcceleration > -0.15) {
            xAcceleration -= 0.0001;
        }
        wasAccelX = !wasAccelX;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        if (yAcceleration > -0.25) {
            yAcceleration -= 0.0001;
        }
        wasAccelY = !wasAccelY;
    }

    if (!wasAccelX) {
        xAcceleration = trunc(xAcceleration / 1.0005 * 100000) / 100000;
    }
    if (!wasAccelY) {
        yAcceleration = trunc(yAcceleration / 1.0005 * 100000) / 100000;
    }

    if (player.getHitbox().contains(800 - xAcceleration, y)) {
        xAcceleration = -0.07;
    }
    if (player.getHitbox().contains(0 + xAcceleration, y)) {
        xAcceleration = +0.07;
    }
    if (player.getHitbox().contains(x, 1000 - yAcceleration)) {
        PlayerHurt();
        yAcceleration = -0.07;
        
    }
    if (player.getHitbox().contains(x, 0 + yAcceleration)) {
        yAcceleration = +0.07;
    }

    x += xAcceleration * deltaTime * multiplier;
    y += (yAcceleration + 0.009) * deltaTime * multiplier;

    
    player.pos(x, y);
}

void ProcessEnemyMovement() {
    Enemy* processEnemy = enemies->next;

    while (processEnemy != nullptr) {
        if (processEnemy->destination == processEnemy->getPos()) {
            float xNew = std::rand() % 100 + processEnemy->center.x, yNew = std::rand() % 100 + processEnemy->center.y;
            processEnemy->destination = sf::Vector2f(xNew, yNew);
        }
        if (processEnemy->destination != processEnemy->getPos() && !processEnemy->isMoving) {
            processEnemy->isMoving = true;
            sf::Vector2f pos = processEnemy->getPos();
            processEnemy->xAcc = (processEnemy->destination.x - pos.x) * deltaTime * multiplier / ((21 - processEnemy->level) * 500);
            processEnemy->yAcc = (processEnemy->destination.y - pos.y) * deltaTime * multiplier / ((21 - processEnemy->level) * 500);
        }
        if (processEnemy->isMoving) {
            sf::Vector2f vect = { processEnemy->xAcc, processEnemy->yAcc };
            processEnemy->move(vect);
            if (abs(processEnemy->getPos().x - processEnemy->destination.x) < 2 && abs(processEnemy->getPos().y - processEnemy->destination.y) < 2 ) {
                processEnemy->pos(processEnemy->destination);
                processEnemy->isMoving = false;
            }
        }
        processEnemy = processEnemy->next;
    }
}

void ProcessFriendlyBulletMovement() {
    Bullet* process = friendlyBullets->next;
    while (process != nullptr && process != friendlyBullets) {
        sf::Vector2f position = process->getPos();
        position.y -= 0.2 * deltaTime * multiplier;
        process->pos(position);
        if (position.y < -100) {
            process->erase(process);
            continue;
        }
        process = process->next;
    }
}

void ProcessEnemyBulletMovement() {
    Bullet* process = enemyBullets->next;
    while (process != nullptr && process != enemyBullets) {
        sf::Vector2f position = process->getPos();
        position.y += 0.1 * deltaTime * multiplier;
        process->pos(position);
        if (position.y > 1100) {
            process->erase(process);
            continue;
        }
        process = process->next;
    }
}

void ProcessPowerupsovement() {
    Item* process = powerups->next;
    while (process != nullptr && process != powerups) {
        sf::Vector2f position = process->getPos();
        position.y += 0.01 * deltaTime * multiplier;
        process->pos(position);
        if (position.y > 1100) {
            process->erase(process);
            continue;
        }
        process = process->next;
    }
}

void ProcessBackgroundMovement() {

}

void ProcessCollision() {
    Bullet* processFriendlyBullets = friendlyBullets->next;
    Enemy* processEnemies = enemies->next;
    while (processFriendlyBullets != nullptr && processFriendlyBullets != friendlyBullets) {
        processEnemies = enemies->next;
        while (processEnemies != nullptr && processEnemies != enemies && processFriendlyBullets != nullptr && processFriendlyBullets != friendlyBullets) {
            if (processFriendlyBullets->getHitbox().intersects(processEnemies->getHitbox())) {
                processEnemies->hp -= processFriendlyBullets->damage;
                processFriendlyBullets->erase(processFriendlyBullets);
                if (processEnemies->hp <= 0) {
                    score += processEnemies->xp;
                    SpawnItem(processEnemies->getPos());
                    processEnemies->erase(processEnemies);
                }
            }
            processEnemies = processEnemies->next;
        }
        processFriendlyBullets = processFriendlyBullets->next;
    }

    Bullet* processEnemyBullets = enemyBullets->next;
    while (processEnemyBullets != nullptr && processEnemyBullets != enemyBullets) {
        if (processEnemyBullets->getHitbox().intersects(player.getHitbox())) {
            PlayerHurt();
            processEnemyBullets->erase(processEnemyBullets);
        }
        processEnemyBullets = processEnemyBullets->next;
    }

    Item* processPowerups = powerups->next;
    while (processPowerups != nullptr && processPowerups != powerups) {
        if (processPowerups->getHitbox().intersects(player.getHitbox())) {
            PickupItem(processPowerups);
        }
        processPowerups = processPowerups->next;
    }

    processEnemies = enemies->next;
    while (processEnemies != nullptr && processEnemies != enemies) {
        if (processEnemies->getHitbox().intersects(player.getHitbox())) {
            PlayerHurt();
        }
        processEnemies = processEnemies->next;
    }

    if (laserShot && player.getHitbox().intersects(laser.getGlobalBounds())) {
        PlayerHurt();
    }
}

void DrawBackground(sf::RenderWindow& window) {
    if (inGame) {
         window.draw(background1);   
    }
    if (inMainMenu) {
        window.draw(background2);
    }
}

void DrawSprites(sf::RenderWindow& window) {
    if (inGame) {
        window.draw(laser);
        Bullet* drawingFriendlyBullets = friendlyBullets->next;
        while (drawingFriendlyBullets != nullptr) {
            drawingFriendlyBullets->draw(window);
            drawingFriendlyBullets = drawingFriendlyBullets->next;
        }

        Bullet* drawingEnemyBullets = enemyBullets->next;
        while (drawingEnemyBullets != nullptr) {
            drawingEnemyBullets->draw(window);
            drawingEnemyBullets = drawingEnemyBullets->next;
        }

        player.draw(window);

        Enemy* drawingEnemies = enemies->next;
        while (drawingEnemies != nullptr) {
            drawingEnemies->draw(window);
            drawingEnemies = drawingEnemies->next;
        }

        Item* drawPowerups = powerups->next;
        while (drawPowerups != nullptr) {
            drawPowerups->draw(window);
            drawPowerups = drawPowerups->next;
        }
    }
    
}

void DrawUI(sf::RenderWindow& window) {
    if (inMainMenu) {
        if (!inLevelSelection) {
            window.draw(title);
            for (int i = 0; i < menuButtons.size(); ++i) {
                menuButtons[i].draw(window);
            }
            window.draw(highscoreText);
            window.draw(highscoreNumbText);
            window.draw(ammoSquare1);
            if (highscore > 1000) {
                window.draw(ammoSquare2);
            }
            if (highscore > 10000) {
                window.draw(ammoSquare3);
            }
            if (highscore > 1000000) {
                window.draw(ammoSquare4);
            }
        }
        else {
            levelSelectPlayButton.draw(window);
            window.draw(levelSelectText);
            window.draw(levelNumbText);
            window.draw(prevText);
            window.draw(nextText);
            window.draw(back);
        }
    }
    if (inGame) {
        if (textShowup.getElapsedTime().asSeconds() < 4) {
            window.draw(waveText);
            window.draw(waveNumb);
        }
        for (int i = 0; i < player.hp || i < 10; ++i) {
            window.draw(health[i]);
        }
        window.draw(scoreText);
        for (int i = 0; i < overHeatBar.size(); ++i) {
            window.draw(overHeatBar[i]);
        }
        window.draw(overHeatMain);
        window.draw(curLevelText);
        window.draw(curLevelNumbText);
    }
    if (gameOver) {
        window.draw(pauseScreen);
        
        if (!victory) {
            window.draw(gameOverText);
            for (int i = 0; i < gameOverButtons.size(); ++i) {
                gameOverButtons[i].draw(window);
            }
        }
        else {
            window.draw(victoryText);
            gameOverButtons[1].draw(window);
            nextLevelButton.draw(window);
        }
        window.draw(gameOverScore);
    }
    if (pause) {
        window.draw(pauseScreen);
        for (int i = 0; i < pauseButtons.size(); ++i) {
            pauseButtons[i].draw(window);
        }
        window.draw(pauseText);
    }
}

void Animate(int xDirectionPlayer = 0) {
    playerAnimation.updateMove(0, deltaTime, xDirectionPlayer);
    player.sprite.setTextureRect(playerAnimation.uvRect);
    backgroundAnimation1.updateCycle(backgroundAnimation1.currentImage.y, deltaTime);
    background1.setTextureRect(backgroundAnimation1.uvRect);
    backgroundAnimation2.updateCycle(backgroundAnimation2.currentImage.y, deltaTime);
    background2.setTextureRect(backgroundAnimation2.uvRect);

    Item* processPowerups = powerups->next;
    while (processPowerups != nullptr) {
        if (processPowerups->type == 0) {
            processPowerups->animation.updateCycle(0, deltaTime);
            processPowerups->sprite.setTextureRect(processPowerups->animation.uvRect);
        }
        /*if (processPowerups->type == 1) {
            processPowerups->animation.updateCycle(0, deltaTime);
            processPowerups->sprite.setTextureRect(processPowerups->animation.uvRect);
        }*/
        processPowerups = processPowerups->next;
    }

    Bullet* processBullets = friendlyBullets->next;
    while (processBullets != nullptr) {
        processBullets->animation.updateCycle(0, deltaTime);
        processBullets->sprite.setTextureRect(processBullets->animation.uvRect);
        processBullets = processBullets->next;
    }
}

void Pause() {
    //stop all clocks
    pause = !pause;
}

void NewGameSetup() {
    wave = -1;
    inGame = true;
    heat = 0;
    player.applyInfo(playerStart);
    xAcceleration = 0;
    yAcceleration = 0;
    wasAccelX = false;
    wasAccelY = false;
    victory = false;
    xDirectionPlayer = 0;
    yDirectionPlayer = 0;
    x = 400;
    y = 500;

    curLevelNumbText.setString(std::to_string(level));

    for (int i = 0; i < 10; ++i) {
        health[i].setColor(sf::Color::White);
    }
    
    Bullet* processFriendlyBullets = friendlyBullets->next;
    while (processFriendlyBullets != nullptr && processFriendlyBullets != friendlyBullets) {
        processFriendlyBullets->erase(processFriendlyBullets);
        processFriendlyBullets = processFriendlyBullets->next;
    }
    Enemy* processEnemies = enemies->next;
    while (processEnemies != nullptr && processEnemies != enemies) {
        processEnemies->erase(processEnemies);
        processEnemies = processEnemies->next;
    }
    Bullet* processEnemyBullets = enemyBullets->next;
    while (processEnemyBullets != nullptr && processEnemyBullets != enemyBullets) {
        processEnemyBullets->erase(processEnemyBullets);
        processEnemyBullets = processEnemyBullets->next;
    }
    Item* processPowerups = powerups->next;
    while (processPowerups != nullptr && processPowerups != powerups) {
        processPowerups->erase(processPowerups);
        processPowerups = processPowerups->next;
    }
    score = 0;
}

void SwitchSelectedLevel() {
    levelNumbText.setString(std::to_string(level));
    if (level > levelsUnlocked) {
        levelSelectText.setFillColor(sf::Color(100, 100, 100));
        levelNumbText.setFillColor(sf::Color(100, 100, 100));
    }
    else {
        levelSelectText.setFillColor(sf::Color::White);
        levelNumbText.setFillColor(sf::Color::White);
    }
}

void MouseMoved(sf::Vector2f pos) {
    if (pause) {
        for (int i = 0; i < pauseButtons.size(); ++i) {
            if (pauseButtons[i].getHitbox().contains(pos)) {
                pauseButtons[i].color(true);
            }
            else {
                pauseButtons[i].color(false);
            }
        }
    }
    if (gameOver) {
        if (!victory) {
            for (int i = 0; i < gameOverButtons.size(); ++i) {
                if (gameOverButtons[i].getHitbox().contains(pos)) {
                    gameOverButtons[i].color(true);
                }
                else {
                    gameOverButtons[i].color(false);
                }
            }
        }
        else {
            if (gameOverButtons[1].getHitbox().contains(pos)) {
                gameOverButtons[1].color(true);
            }
            else {
                gameOverButtons[1].color(false);
            }
            if (nextLevelButton.getHitbox().contains(pos)) {
                nextLevelButton.color(true);
            }
            else {
                nextLevelButton.color(false);
            }
        }
        
    }
    if (inMainMenu) {
        if (inLevelSelection) {
            if (levelSelectPlayButton.getHitbox().contains(pos)) {
                levelSelectPlayButton.color(true);
            }
            else {
                levelSelectPlayButton.color(false);
            }
            if (prevText.getGlobalBounds().contains(pos)) {
                prevText.setFillColor(sf::Color::Yellow);
            }
            else {
                prevText.setFillColor(sf::Color::White);
            }
            if (nextText.getGlobalBounds().contains(pos)) {
                nextText.setFillColor(sf::Color::Yellow);
            }
            else {
                nextText.setFillColor(sf::Color::White);
            }
            if (back.getGlobalBounds().contains(pos)) {
                back.setFillColor(sf::Color::Yellow);
            }
            else {
                back.setFillColor(sf::Color::White);
            }
        }
        else {
            for (int i = 0; i < menuButtons.size(); ++i) {
                if (menuButtons[i].getHitbox().contains(pos)) {
                    menuButtons[i].color(true);
                }
                else {
                    menuButtons[i].color(false);
                }
            }
        }
        
    }

}

void MousePressed(sf::Vector2f pos) {
    if (pause) {
        if (pauseButtons[0].getHitbox().contains(pos)) {
            pause = !pause;
            return;
        }
        if (pauseButtons[1].getHitbox().contains(pos)) {
            pause = !pause;
            inGame = false;
            inMainMenu = true;
            return;
        }
    }
    if (gameOver) {
        if (!victory) {
            if (gameOverButtons[0].getHitbox().contains(pos)) {
                gameOver = false;
                NewGameSetup();
                return;
            }
        }
        else {
            if (gameOverButtons[0].getHitbox().contains(pos)) {
                gameOver = false;
                ++level;
                NewGameSetup();
                return;
            }
        }
        if (gameOverButtons[1].getHitbox().contains(pos)) {
            gameOver = false;
            inGame = false;
            inMainMenu = true;
            return;
        }
    }
    if (inMainMenu) {
        if (!inLevelSelection) {
            if (menuButtons[0].getHitbox().contains(pos)) {
                inGame = true;
                inMainMenu = false;
                level = 0;
                difficultyLevel = 1;
                NewGameSetup();
                return;
            }
            if (menuButtons[1].getHitbox().contains(pos)) {
                inLevelSelection = true;
                level = 1;
                SwitchSelectedLevel();
                return;
            }
            if (menuButtons[2].getHitbox().contains(pos)) {
                close = true;
                return;
            }
            if (ammoSquare1.getGlobalBounds().contains(pos)) {
                ammoType = 0;
                SwitchAmmoType();
            }
            if (highscore > 1000 && ammoSquare2.getGlobalBounds().contains(pos)) {
                ammoType = 1;
                SwitchAmmoType();
            }
            if (highscore > 10000 && ammoSquare3.getGlobalBounds().contains(pos)) {
                ammoType = 2;
                SwitchAmmoType();
            }
            if (highscore > 10000 && ammoSquare4.getGlobalBounds().contains(pos)) {
                ammoType = 3;
                SwitchAmmoType();
            }
        }
        else {
            if (levelSelectPlayButton.getHitbox().contains(pos) && level <= levelsUnlocked) {
                inLevelSelection = false;
                inMainMenu = false;
                inGame = true;
                difficultyLevel = -1;
                NewGameSetup();
                return;
            }
            if (prevText.getGlobalBounds().contains(pos)) {
                if (level > 1) {
                    --level;
                }
                SwitchSelectedLevel();
            }
            if (nextText.getGlobalBounds().contains(pos)) {
                if (level < 10) {
                    ++level;
                }
                SwitchSelectedLevel();
            }
            if (back.getGlobalBounds().contains(pos)) {
                inLevelSelection = false;
            }
        }
        
    }
}

void UpdateScore() {
    if (score > highscore) {
        highscore = score;
        highscoreNumbText.setString(std::to_string(highscore));
    }
    scoreText.setString(std::to_string(score));
    scoreText.setOrigin(scoreText.getGlobalBounds().width, 0);
    scoreText.setPosition(790, 0);
    
    gameOverScore.setString(std::to_string(score));
    gameOverScore.setOrigin(scoreText.getGlobalBounds().width / 2, scoreText.getGlobalBounds().height / 2);
    gameOverScore.setPosition(400, 825);
}

void UpdateHeat() {
    if (infiniteHeat) {
        overHeatMain.setOutlineColor(sf::Color::Blue);
        for (int i = heat; i < 10; ++i) {
            overHeatBar[i].setFillColor(sf::Color(0, 150, 0, 100));
        }
    }
    if (overheat) {
        overHeatMain.setOutlineColor(sf::Color::Red);
        if (heatCooldown.getElapsedTime().asSeconds() > 1) {
            if (heat > 0) {
                --heat;
            }
            if (heat == 0) {
                overheat = false;
            }
            heatCooldown.restart();
        }
    }
    else if (heatCooldown.getElapsedTime().asSeconds() > bulletTypes[ammoType].cooldown) {
        overHeatMain.setOutlineColor(sf::Color(200, 200, 200));
        if (heat > 0) {
            --heat;
        }
        heatCooldown.restart();
    }
    for (int i = 0; i < heat; ++i) {
        if (heat < 4 && !overheat) {
            overHeatBar[i].setFillColor(sf::Color::Green);
        }
        if (heat >= 4 && heat < 7 && !overheat) {
            overHeatBar[i].setFillColor(sf::Color::Yellow);
        }
        if (heat >= 7 || overheat) {
            overHeatBar[i].setFillColor(sf::Color::Red);
        }
    }
    for (int i = heat; i < 10; ++i) {
        overHeatBar[i].setFillColor(sf::Color(150, 150, 150, 100));
    }
}

void SwitchWave() {
    if (level == 0) {
        waveText.setFillColor(sf::Color::Transparent);
        waveNumb.setFillColor(sf::Color::Transparent);
        curLevelText.setFillColor(sf::Color::Transparent);
        curLevelNumbText.setFillColor(sf::Color::Transparent);
        if (score > threshold) {
            ++difficultyLevel;
            threshold = 2 * threshold;
        }
        return;
    }
    if (enemies->next == nullptr) {
        waveText.setFillColor(sf::Color::White);
        waveNumb.setFillColor(sf::Color::White);
        curLevelText.setFillColor(sf::Color::White);
        curLevelNumbText.setFillColor(sf::Color::White);
        ++wave;
        textShowup.restart();
        waveNumb.setString(std::to_string(wave + 1));
        CreateWave();
    }
}

void LoadToFiles() {
    std::fstream file;
    file.open("Save.txt");
    if (!file.is_open()) {
        std::cout << "FILE 2";
    }
    std::string h = std::to_string(highscore);
    h.push_back(' ');
    std::string h2 = std::to_string(levelsUnlocked);
    for (int i = 0; i < h2.size(); ++i) {
        h.push_back(h2[i]);
    }
    for (int i = 0; i < h.size(); ++i) {
        file << h[i];
    }
    file.close();
}

int main() {
    std::srand(std::time(nullptr));
    sf::RenderWindow window(sf::VideoMode(800, 1000), "THE GAME");

    sf::Clock clock, shotCooldown, spawnCooldown;


    float mouseX{}, mouseY{};
    sf::Texture* enemyTexture = new sf::Texture;
    sf::Texture* bulletTexture1 = new sf::Texture;
    sf::Texture* backgroundTexture1 = new sf::Texture;
    sf::Texture* backgroundTexture2 = new sf::Texture;
    heartTexture = new sf::Texture;
    bulletTexture2 = new sf::Texture;
    bulletTexture3 = new sf::Texture;
    bulletTexture4 = new sf::Texture;
    bulletTexture5 = new sf::Texture;
    laserTexture = new sf::Texture;

    ammo.type = 29;
    ammo.damage = 400;
    enemyAmmo.type = 12;
    basicEnemy.color = sf::Color::Red;
    bigEnemy.color = sf::Color::White;
    bigEnemy.type = 5;
    bigEnemy.scale = 3;
    bigEnemy.hp = 400;
    playerStart.pos = sf::Vector2f(400, 500);
    playerStart.hp = 10;

    LoadFromFiles();
    PreProcess(enemyTexture, bulletTexture1, backgroundTexture1, backgroundTexture2);
    CreateLevels();
    CreateEnemyTypes();
    CreateBulletTypes();
    SwitchAmmoType();
    NewGameSetup();
    inGame = false;
    inMainMenu = true;

    while (window.isOpen()) {

        deltaTime = clock.restart().asSeconds();
        
            

        sf::Event event;
        while (window.pollEvent(event)) {

            if (event.type == sf::Event::Closed || close) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space && shotCooldown.getElapsedTime().asMilliseconds() > 100) {
                    PlayerShot();
                    shotCooldown.restart();
                }
                if (event.key.code == sf::Keyboard::Escape) {
                    if (inGame) {
                        Pause();
                    }
                    if (inLevelSelection) {
                        level = 0;
                        inLevelSelection = !inLevelSelection;
                    }
                }
            }

            if (event.type == sf::Event::MouseMoved) {
                MouseMoved(sf::Vector2f(event.mouseMove.x, event.mouseMove.y));
                mouseX = event.mouseMove.x;
                mouseY = event.mouseMove.y;
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                MousePressed(sf::Vector2f(mouseX, mouseY));
                //std::cout << mouseX << " " << mouseY << "\n";
            }

        }
        if (inGame && !pause && !gameOver) {
            AiWork(spawnCooldown);

            ProcessPlayerMovement(xDirectionPlayer, yDirectionPlayer, xAcceleration, yAcceleration, wasAccelX, wasAccelY);
            ProcessEnemyMovement();
            ProcessFriendlyBulletMovement();
            ProcessEnemyBulletMovement();
            ProcessPowerupsovement();
            ProcessCollision();
            UpdateScore();
            UpdateHeat();
            SwitchWave();
            
        }
        Animate(xDirectionPlayer);

        if (damageCooldown.getElapsedTime().asSeconds() < 1) {
            if (damageCooldown.getElapsedTime().asMilliseconds() % 200 < 100) {
                player.sprite.setColor(sf::Color::Transparent);
            }
            else {
                player.sprite.setColor(sf::Color::White);
            }
        }
                
        window.clear(sf::Color::Black);

        DrawBackground(window);
        DrawSprites(window);
        DrawUI(window);
        
        window.display();
    }
    LoadToFiles();
}


/*
KEY CONCEPTS:
1.Ship flying through space with custom background
2.Enemies shooting/being annoying


main classes:
enteties:
    player :
        -checking if something is touching(bool)
        -invincibility(bool)
        -movement(void)
        -shooting(void)[spawns an entity type friendly shot]
    enemy :
        

    enemy shot :


    junk :



    friedly shot :

GUI :
    bars :


    buttons :

rendering(?): - might be done outside of classes, otherwise in functions
    window:
        esc function
        levels function


    background:
        moving tiles or cotineous image

    sprites:
        player and enemies
        shots
        junk

    movement:
        movement through map

        animations



core:
    hit registration:



    enemy generation





*/