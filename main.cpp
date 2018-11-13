/**
 * Trabalho 1 de Programação Concorrente
 * 
 * Cristiano Cardoso 15/0058349
 * 
 * Repositório do github: https://github.com/asm95/pc.trab1 
 *
*/
#include <SFML/Graphics.hpp>

#include <cstdlib>

#include <pthread.h>

#include "util.h"

#define APP_NAME "PC Toolbox"

#define ERR_SPR_LOAD "(E) Error on loading sprites from folder. Exiting..."

sf::Texture *monkey_texture_set[2];
float init_pos[2] = {30.f, 540.f};

class Monkey {
    public:
        Monkey(int side, int id){
            this->side = side;
            this->id = id;
            this->visible = false;
            this->pos_x = init_pos[side];
            this->speed = 90.f;

            sprite.setTexture(*monkey_texture_set[side]);
            sprite.setPosition(sf::Vector2f(this->pos_x, 160.f));
            sprite.setScale(sf::Vector2f(0.2f, 0.2f));
        };
        ~Monkey();
        int side;
        int id;
        float pos_x, pos_y;
        float speed;
        bool visible;
        sf::Sprite sprite;
    private:
};

#define DOOR_COUNT 2
#define MAX_MONKEY_COUNT 10

pthread_mutex_t global_bridge_access_lock[2];   // locks the bridge access for a side
pthread_mutex_t global_va_state_lock, cond_lock, render_lock;           // locks a C global variable write access
pthread_cond_t global_cond_transfered = PTHREAD_COND_INITIALIZER; //
uint monkey_count[2] = {0};             // how many monkeys are waiting to switch side (if zero, the bridge is released)
uint monkey_transfer_count[2] = {0};    // how many monkeys actually crossed the bridge until no one left
const char *open_text = "(I) Portal from %s to %s opened.\n";
const char *transfer_text = "(I) Transfering %d monkeys from %s.\n";
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
    
    bool is_last_monkey = false;
    printf("(I) Monkey from %d,%d will cross...\n", m->side, m->id);
    pthread_mutex_lock(&global_bridge_access_lock[m->side]);
    printf("(I) Monkey from %d,%d is crossing...\n", m->side, m->id);
    monkey_count[m->side] += -1;
    monkey_transfer_count[m->side] += 1;
    //m->side = 1-(m->side);                                                      // monkey knows now it switched side
    if (monkey_count[m->side] == 0){
        is_last_monkey = true;
        printf(transfer_text, monkey_transfer_count[m->side], state_text[m->side]);
        pthread_mutex_lock(&cond_lock);
        while( monkey_transfer_count[m->side] != 1 ){
            pthread_cond_wait(&global_cond_transfered, &cond_lock);
        }
        pthread_mutex_unlock(&cond_lock);
    }
    pthread_mutex_unlock(&global_bridge_access_lock[m->side]);

    printf("(I) Monkey %d,%d walking...\n", m->side, m->id);
    m->visible = true;
    if (m->side){
        while(m->sprite.getPosition().x >= init_pos[0]){
            m->sprite.move(-m->speed, 0); os_sleep(500);
        }
    } else {
        while(m->sprite.getPosition().x <= init_pos[1]){
            m->sprite.move(m->speed, 0); os_sleep(500);
        }
    }
    m->visible = false;
    printf("(I) Monkey %d,%d stopped\n", m->side, m->id);

    if (!is_last_monkey){
        printf("(I) Checking out...\n");
        pthread_mutex_lock(&cond_lock);
        monkey_transfer_count[m->side] += -1;
        printf("(I) Now value is %d\n", monkey_transfer_count[m->side]);
        pthread_cond_signal(&global_cond_transfered);
        pthread_mutex_unlock(&cond_lock);
    } else {
        printf("(I) Checking all out...\n");
        monkey_transfer_count[m->side] = 0;
        pthread_mutex_unlock(&global_bridge_access_lock[1-(m->side)]);          // until all monkeys have made their route safely
        pthread_mutex_unlock(&global_bridge_access_lock[(m->side)]);          // until all monkeys have made their route safely
    }


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

    monkey_texture_set[0] = &crystal_tex;
    monkey_texture_set[1] = &heart_tex;

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
        for (int i=0; i<MAX_MONKEY_COUNT; i++){
            if (monkey_set[i]->visible){
                window.draw(monkey_set[i]->sprite);
            }
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
