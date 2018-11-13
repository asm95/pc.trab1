#include <SFML/Graphics.hpp>

#define APP_NAME "PC Toolbox"

#define ERR_SPR_LOAD "(E) Error on loading sprites from folder. Exiting..."

class Monkey {
    public:
        Monkey();
        ~Monkey();
    private:
        int side;
};



int main()
{
    const float w_width = 640.f, w_height = 480.f;
    sf::RenderWindow window(sf::VideoMode(w_width, w_height), "SFML works!");
    window.setFramerateLimit(30u);

    sf::Clock frameClock;
    char title_buf[128];

    sf::Texture crystal_tex, heart_tex;

    const std::string sprites_folder = "sprites/";

    if (!crystal_tex.loadFromFile(sprites_folder+"crystal.png")){
        printf(ERR_SPR_LOAD);
        exit(1);
    }
    if (!heart_tex.loadFromFile(sprites_folder+"heart.png")){
        printf(ERR_SPR_LOAD);
        exit(1);
    }

    sf::Sprite crystal_spr, heart_spr;
    crystal_spr.setTexture(crystal_tex);
    crystal_spr.setPosition(sf::Vector2f(10.f, 10.f));
    crystal_spr.setScale(sf::Vector2f(0.2f, 0.2f));

    heart_spr.setTexture(heart_tex);
    heart_spr.setPosition(sf::Vector2f(20.f, 10.f));
    heart_spr.setScale(sf::Vector2f(0.2f, 0.2f));


    const float limit_x = w_width - ( crystal_spr.getGlobalBounds().width + 20 );
    float espeed_x = 20.f;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(crystal_spr);
        window.draw(heart_spr);
        window.display();

        if (crystal_spr.getPosition().x >= limit_x){
            espeed_x = 0;
        }
        crystal_spr.move(espeed_x, 0);

        const float time = 1.f / frameClock.getElapsedTime().asSeconds();
        sprintf(title_buf, "%s | FPS: %.2f", APP_NAME, time);
        
        window.setTitle(title_buf);
    }

    return 0;
}
