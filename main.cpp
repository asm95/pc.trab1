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
#include <cstdio>
#include <cstdlib>

#include <pthread.h>

#include "util.h"

#include "vendor/inih/cpp/INIReader.h"

#define APP_NAME "PC Toolbox"

sf::Texture *monkey_texture_set[2];

class Monkey {
    public:
        Monkey(int side, int id){
            this->side = side;
            this->id = id;
            this->visible = false;
            this->pos_x = door_stop[side];
            this->speed = base_speed;

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
        static double door_stop[2];
        static double base_speed;
        static int sleep_intl;

        void walk(){
            printf("(I) Monkey %d,%d walking...\n", side, id);
            visible = true;
            if (side){
                while(sprite.getPosition().x >= door_stop[0]){
                    sprite.move(-speed, 0); os_sleep(sleep_intl);
                }
            } else {
                while(sprite.getPosition().x <= door_stop[1]){
                    sprite.move(speed, 0); os_sleep(sleep_intl);
                }
            }
            visible = false;
            printf("(I) Monkey %d,%d stopped\n", side, id);
        }
    private:
};

double Monkey::door_stop[] = {0};
double Monkey::base_speed = 0;
int Monkey::sleep_intl = 100;

#define DOOR_COUNT 2
#define BRIDGE_COUNT 2
#define MAX_MONKEY_COUNT 10

#define mlock pthread_mutex_lock
#define munlock pthread_mutex_unlock
#define csignal pthread_cond_signal
#define cwait pthread_cond_wait

pthread_mutex_t g_bridge_lock[3];   // locks the bridge access for a side
pthread_mutex_t g_va_state_lock, cond_lock, render_lock;           // locks a C global variable write access
pthread_cond_t g_cond_okdest = PTHREAD_COND_INITIALIZER; //
uint monkey_count[2] = {0};             // how many monkeys are waiting to switch side (if zero, the bridge is released)
uint monkey_side_count[2] = {0};    // how many monkeys actually crossed the bridge until no one left
const char *open_text = "(I) Portal from %s to %s opened.\n";
const char *transfer_text = "(I) Transfering %d monkeys from %s.\n";
const char *state_text[2] = {AS_COLOR(COLOR_GREEN,"left"),AS_COLOR(COLOR_BLUE,"right")};

void *monkey_loop(void *args){
    Monkey *m = (Monkey*)args;

    mlock(&g_va_state_lock);
    monkey_side_count[m->side] += 1;
    munlock(&g_va_state_lock);

    mlock(&g_bridge_lock[m->side]);
    monkey_count[m->side] += 1;
    
    if (monkey_count[m->side] == 1){
        printf("Monkey %d,%d requesting for access...\n", m->side, m->id);
        mlock(&g_bridge_lock[2]);
        printf(open_text, state_text[m->side], state_text[1-(m->side)]);
    }
    munlock(&g_bridge_lock[m->side]);
    
    printf("(I) Monkey from %d,%d will cross...\n", m->side, m->id);
    mlock(&g_bridge_lock[m->side]);
    printf("(I) Monkey from %d,%d is crossing...\n", m->side, m->id);
    monkey_count[m->side] += -1;
    m->walk();
    if (monkey_count[m->side] == 0){
        printf("(I) Monkey %d,%d checking all out...\n", m->side, m->id);
        munlock(&g_bridge_lock[2]);
    }
    munlock(&g_bridge_lock[m->side]);
    printf("(I) Monkey %d,%d exiting...\n", m->side, m->id);

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

class ConfManager{
    private:
        INIReader *confHandler;
        std::string file_path;
    public:
    ConfManager(std::string confPath){
        file_path = confPath;
        confHandler = new INIReader(file_path);
        if (confHandler->ParseError() < 0) {
            throw std::runtime_error("Could not read configuration file. It may be corrupted or does not exist");
        }
    }
    ~ConfManager(){
    }
    void assert_param_exists(std::string &val, const char *sec, const char *var){
        if (val.length() == 0){
            throw std::runtime_error("Could not load parameter (" 
                + std::string(sec) + "," + std::string(var) + 
                ") from " + file_path + "."
            );
        }
    }
    sf::Vector2f get_pos(const char *sec, const char *var){
        std::string val = confHandler->Get(sec, var, "");
        assert_param_exists(val, sec, var);

        double x,y;
        sscanf(val.c_str(), "(%lf,%lf)", &x, &y);
        return sf::Vector2f(x,y);
    }
    double get_double(const char *sec, const char *var){
        std::string val = confHandler->Get(sec, var, "");
        assert_param_exists(val, sec, var);
        double a; int r;
        r = sscanf(val.c_str(), "%lf", &a);
        if (!r){
            throw std::runtime_error("Invalid real parameter ("
                + std::string(sec) + "," + std::string(var) +
                ") from " + file_path + "."
            );
        }
        return a;
    }
    int get_int(const char *sec, const char *var){
        std::string val = confHandler->Get(sec, var, "");
        int a; int r;
        r = sscanf(val.c_str(), "%d", &a);
        if (!r){
            throw std::runtime_error("Invalid integer parameter ("
                + std::string(sec) + "," + std::string(var) +
                ") from " + file_path + "."
            );
        }
        return a;
    }
};

int main()
{
    const float w_width = 640.f, w_height = 480.f;
    sf::RenderWindow window(sf::VideoMode(w_width, w_height), APP_NAME " | Loading...");
    window.setFramerateLimit(30u);

    sf::Clock frameClock;
    char title_buf[128];

    // load our configuration file
    ConfManager cm("conf.ini");

    // create object texture data
    sf::Texture crystal_tex, heart_tex, door_tex, background_tex, plate_tex;
    load_sprite_tex(crystal_tex, "crystal.png");
    load_sprite_tex(heart_tex, "heart.png");
    load_sprite_tex(door_tex, "door.png");
    load_sprite_tex(background_tex, "background.png");
    load_sprite_tex(plate_tex, "plate1.png");

    monkey_texture_set[0] = &crystal_tex;
    monkey_texture_set[1] = &heart_tex;

    // create debug text
    std::list<sf::Text> debugTextList;
    sf::Text text_monkey_cl, text_monkey_cr;
    sf::Font DebugTextFont;
    load_font(DebugTextFont, "PCBius.ttf");
    text_monkey_cl.setPosition(cm.get_pos("debug_text","monkey_cl_pos"));
    text_monkey_cl.setFont(DebugTextFont);
    text_monkey_cr.setFont(DebugTextFont);
    text_monkey_cr.setPosition(cm.get_pos("debug_text","monkey_cr_pos"));
    debugTextList.push_back(text_monkey_cl);
    debugTextList.push_back(text_monkey_cr);

    // spawn background
    sf::Sprite background_spr;
    background_spr.setTexture(background_tex);
    // spawn doors
    const float margin_x = 20.f, door_dist = 500.f;
    const float floor_aligment = w_height - cm.get_double("obj_door","floor_align");
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
    // spawn bridge
    sf::Sprite bridge_spr[BRIDGE_COUNT];
    char sec_text[32];
    for (int i=0; i<BRIDGE_COUNT; i++){
        bridge_spr[i].setTexture(plate_tex);
        sprintf(sec_text, "el%d_pos", i);
        bridge_spr[i].setPosition(cm.get_pos("obj_bridge", sec_text));
        bridge_spr[i].setScale(cm.get_pos("obj_bridge", "scale"));
    }

    // set monkey properties
    Monkey::door_stop[0] = cm.get_double("monkey","doorL");
    Monkey::door_stop[1] = cm.get_double("monkey","doorR");
    Monkey::base_speed = cm.get_double("monkey","speed");
    Monkey::sleep_intl = cm.get_int("monkey","sleep_time");

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
        for (int i=0; i<BRIDGE_COUNT; i++){
            window.draw(bridge_spr[i]);
        }
        for (int i=0; i<MAX_MONKEY_COUNT; i++){
            if (monkey_set[i]->visible){
                window.draw(monkey_set[i]->sprite);
            }
        }

        // update text
        pthread_mutex_lock(&g_va_state_lock);
        text_monkey_cl.setString("Macacos a Esq.: " + std::to_string(monkey_side_count[0]) );
        text_monkey_cr.setString("Macacos a Dir..: " + std::to_string(monkey_side_count[1]) );
        pthread_mutex_unlock(&g_va_state_lock);
        window.draw(text_monkey_cl);
        window.draw(text_monkey_cr);

        window.display();

        const float time = 1.f / frameClock.getElapsedTime().asSeconds();

        sprintf(title_buf, "%s | FPS: %.2f", APP_NAME, time);
        window.setTitle(title_buf);
    }

    return 0;
}
