#include <SFML/Graphics.hpp>

#include <cstdlib>
#include <ctime>

#include <pthread.h>

#include "util.h"

#define APP_NAME "PC Toolbox"

#define ERR_SPR_LOAD "(E) Error on loading sprites from folder. Exiting..."

class Monkey {
    public:
        Monkey(int side, int id){
            this->side = side;
            this->id = id;
        };
        ~Monkey();
        int side;
        int id;
    private:
};

#define DOOR_COUNT 2
#define MAX_MONKEY_COUNT 10

pthread_mutex_t global_bridge_access_lock[2];   // locks the bridge access for a side
pthread_mutex_t global_va_state_lock;           // locks a C global variable write access
uint monkey_count[2] = {0};             // how many monkeys are waiting to switch side (if zero, the bridge is released)
uint monkey_transfer_count[2] = {0};    // how many monkeys actually crossed the bridge until no one left
const char *open_text = "(I) Portal from %s to %s opened.\n";
const char *transfer_text = "(I) Transfered %d monkeys from %s.\n";
const char *state_text[2] = {AS_COLOR(COLOR_GREEN,"left"),AS_COLOR(COLOR_BLUE,"right")};

void *monkey_loop(void *args){
    Monkey *m = (Monkey*)args;
    pthread_mutex_lock(&global_va_state_lock);
    monkey_count[m->side] += 1;
    pthread_mutex_unlock(&global_va_state_lock);
    if (monkey_count[m->side] == 1){
        pthread_mutex_lock(&global_bridge_access_lock[m->side]);                // if i can lock the brigde
        pthread_mutex_lock(&global_bridge_access_lock[1-(m->side)]);            // than i lock the other side of the bridge
        printf(open_text, state_text[m->side], state_text[1-(m->side)]);
        pthread_mutex_unlock(&global_bridge_access_lock[m->side]);              // so only my side shall pass
    }
    
    pthread_mutex_lock(&global_bridge_access_lock[m->side]);
    monkey_count[m->side] += -1;
    monkey_transfer_count[m->side] += 1;   
    //m->side = 1-(m->side);                                                      // monkey knows now it switched side
    if (monkey_count[m->side] == 0){
        printf(transfer_text, monkey_transfer_count[m->side], state_text[m->side]);
        monkey_transfer_count[m->side] = 0;
        pthread_mutex_unlock(&global_bridge_access_lock[1-(m->side)]);          // until all monkeys have made their route safely
    }
    pthread_mutex_unlock(&global_bridge_access_lock[m->side]);

    pthread_exit(0);
}

int main()
{
    const float w_width = 640.f, w_height = 480.f;
    sf::RenderWindow window(sf::VideoMode(w_width, w_height), "SFML works!");
    window.setFramerateLimit(30u);

    sf::Clock frameClock;
    char title_buf[128];

    sf::Texture crystal_tex, heart_tex, door_tex;

    const std::string sprites_folder = "sprites/";

    if (!crystal_tex.loadFromFile(sprites_folder+"crystal.png")){
        printf(ERR_SPR_LOAD);
        exit(1);
    }
    if (!heart_tex.loadFromFile(sprites_folder+"heart.png")){
        printf(ERR_SPR_LOAD);
        exit(1);
    }
    if (!door_tex.loadFromFile(sprites_folder+"door.png")){
        printf(ERR_SPR_LOAD);
        exit(1);
    }

    // spawn doors
    const float margin_x = 20.f, door_dist = 500.f;
    float floor_aligment = w_height - 400.f;
    sf::Sprite doors_spr[DOOR_COUNT];
    float cur_x, cur_y;
    cur_x = margin_x;
    cur_y = floor_aligment;
    for (int i=0; i<DOOR_COUNT; i++){
        doors_spr[i].setTexture(door_tex);
        doors_spr[i].setPosition(sf::Vector2f(cur_x, cur_y));
        doors_spr[i].setScale(sf::Vector2f(0.2f, 0.2f));
        cur_x += door_dist;
    }

    sf::Sprite crystal_spr, heart_spr;
    crystal_spr.setTexture(crystal_tex);
    crystal_spr.setPosition(sf::Vector2f(10.f, 10.f));
    crystal_spr.setScale(sf::Vector2f(0.2f, 0.2f));

    heart_spr.setTexture(heart_tex);
    heart_spr.setPosition(sf::Vector2f(20.f, 10.f));
    heart_spr.setScale(sf::Vector2f(0.2f, 0.2f));


    std::srand(std::time(nullptr));

    // pthread creation thing
    pthread_t thread_pool[MAX_MONKEY_COUNT];
    Monkey *monkey_set[MAX_MONKEY_COUNT];
    int i, choise;
    for (i = 0; i < MAX_MONKEY_COUNT; i++) {
        choise = (std::rand())%2;
        monkey_set[i] = new Monkey(choise, i);
        pthread_create(&thread_pool[i], NULL, monkey_loop, monkey_set[i]);
    }

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

        // draw doors
        for (int i=0; i<DOOR_COUNT; i++){
            window.draw(doors_spr[i]);
        }

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
