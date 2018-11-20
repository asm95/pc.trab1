/**
 * Trabalho 1 de Programação Concorrente
 * 
 * Cristiano Cardoso 15/0058349
 * 
 * Repositório do github: https://github.com/asm95/pc.trab1 
 *
*/
#include <SFML/Graphics.hpp>

#include <iostream>
#include <string>
#include <list>
#include <cstdlib>

#include <pthread.h>

#include "util.h"

#define APP_NAME "PC Toolbox"

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

const std::string g_sprite_folder = "sprites";
const std::string g_font_folder = "fonts";
void load_sprite_tex(sf::Texture &the_texture, std::string file_name){
    std::string full_sprite_path = g_sprite_folder+ "/" +file_name;
    if (!the_texture.loadFromFile(full_sprite_path)){
        std::cout << "(E) Error on loading image from " << file_name << std::endl;
        exit(1);
    }
}
void load_font(sf::Font &the_font, std::string file_name){
    std::string full_sprite_path = g_font_folder+ "/" +file_name;
    if (!the_font.loadFromFile(full_sprite_path)){
        std::cout << "(E) Error on loading font from " << file_name << std::endl;
        exit(1);
    }
}

int main()
{
    const float w_width = 640.f, w_height = 480.f;
    sf::RenderWindow window(sf::VideoMode(w_width, w_height), "SFML works!");
    window.setFramerateLimit(30u);

    sf::Clock frameClock;
    char title_buf[128];

    // create object texture data
    sf::Texture crystal_tex, heart_tex, door_tex, background_tex;
    load_sprite_tex(crystal_tex, "crystal.png");
    load_sprite_tex(heart_tex, "heart.png");
    load_sprite_tex(door_tex, "door.png");
    load_sprite_tex(background_tex, "background.png");

    monkey_texture_set[0] = &crystal_tex;
    monkey_texture_set[1] = &heart_tex;

    // create debug text
    std::list<sf::Text> debugTextList;
    sf::Text text_monkey_cl, text_monkey_cr;
    sf::Font DebugTextFont;
    load_font(DebugTextFont, "PCBius.ttf");
    text_monkey_cl.setPosition(20, 230);
    text_monkey_cl.setFont(DebugTextFont);
    text_monkey_cr.setFont(DebugTextFont);
    text_monkey_cr.setPosition(20, 265);
    debugTextList.push_back(text_monkey_cl);
    debugTextList.push_back(text_monkey_cr);

    // spawn background
    sf::Sprite background_spr;
    background_spr.setTexture(background_tex);
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

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        // draw background
        window.draw(background_spr);
        // draw doors
        for (int i=0; i<DOOR_COUNT; i++){
            window.draw(doors_spr[i]);
        }
        for (int i=0; i<MAX_MONKEY_COUNT; i++){
            if (monkey_set[i]->visible){
                window.draw(monkey_set[i]->sprite);
            }
        }

        // update text
        text_monkey_cl.setString("Macacos a Esq.: " + std::to_string(monkey_count[0]) );
        text_monkey_cr.setString("Macacos a Dir.: " + std::to_string(monkey_count[1]) );
        window.draw(text_monkey_cl);
        window.draw(text_monkey_cr);

        window.display();

        const float time = 1.f / frameClock.getElapsedTime().asSeconds();

        sprintf(title_buf, "%s | FPS: %.2f", APP_NAME, time);
        window.setTitle(title_buf);
    }

    return 0;
}
