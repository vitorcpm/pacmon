#include "../include/resources.h"
#include <iostream>

ALLEGRO_DISPLAY *display = nullptr;
ALLEGRO_EVENT_QUEUE *event_queue = nullptr;
ALLEGRO_TIMER *timer = nullptr;
ALLEGRO_TIMER *logic_timer = nullptr;
ALLEGRO_BITMAP *map_bitmap = nullptr;
ALLEGRO_BITMAP *pacmon_bitmap[4] = {nullptr};
ALLEGRO_BITMAP *hunter_red = nullptr;
ALLEGRO_BITMAP *hunter_blue = nullptr;
ALLEGRO_BITMAP *hunter_pink = nullptr;
ALLEGRO_BITMAP *hunter_orange = nullptr;
ALLEGRO_BITMAP *banana_bitmap = nullptr;
ALLEGRO_BITMAP *victory_bitmap = nullptr;
ALLEGRO_BITMAP *defeat_bitmap = nullptr;
ALLEGRO_FONT *font = nullptr;
ALLEGRO_SAMPLE *banana_sound = nullptr;
ALLEGRO_SAMPLE *victory_sound = nullptr;
ALLEGRO_SAMPLE *defeat_sound = nullptr;
ALLEGRO_AUDIO_STREAM *music = nullptr;

bool initializeResources() {
    if (!al_init()) { std::cout << "Failed to initialize Allegro!" << std::endl; return false; }
    if (!al_install_keyboard()) { std::cout << "Failed to initialize keyboard!" << std::endl; return false; }
    if (!al_init_image_addon()) { std::cout << "Failed to initialize image addon!" << std::endl; return false; }
    if (!al_init_font_addon()) { std::cout << "Failed to initialize font addon!" << std::endl; return false; }
    if (!al_init_ttf_addon()) { std::cout << "Failed to initialize TTF addon!" << std::endl; return false; }
    if (!al_install_audio()) { std::cout << "Failed to install audio!" << std::endl; return false; }
    if (!al_init_acodec_addon()) { std::cout << "Failed to initialize audio codecs!" << std::endl; return false; }
    if (!al_reserve_samples(2)) { std::cout << "Failed to reserve audio samples!" << std::endl; return false; }

    timer = al_create_timer(1.0 / FPS);
    if (!timer) { std::cout << "Failed to create timer!" << std::endl; return false; }

    logic_timer = al_create_timer(1.0 / LOGIC_FPS);
    if (!logic_timer) { std::cout << "Failed to create logic timer!" << std::endl; return false; }

    display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display) { std::cout << "Failed to create display!" << std::endl; return false; }

    map_bitmap = al_load_bitmap("assets/map/map.bmp");
    if (!map_bitmap) { std::cout << "Failed to load map!" << std::endl; return false; }

    banana_bitmap = al_load_bitmap("assets/map/banana.png");
    if (!banana_bitmap) { std::cout << "Failed to load bananas!" << std::endl; return false; }

    victory_bitmap = al_load_bitmap("assets/map/victorymap.bmp");
    if (!victory_bitmap) { std::cout << "Failed to load victory screen!" << std::endl; return false; }

    defeat_bitmap = al_load_bitmap("assets/map/defeatmap.bmp");
    if (!defeat_bitmap) { std::cout << "Failed to load defeat screen!" << std::endl; return false; }

    pacmon_bitmap[DIR_RIGHT] = al_load_bitmap("assets/characters/pacright.png");
    pacmon_bitmap[DIR_DOWN] = al_load_bitmap("assets/characters/pacdown.png");
    pacmon_bitmap[DIR_LEFT] = al_load_bitmap("assets/characters/pacleft.png");
    pacmon_bitmap[DIR_UP] = al_load_bitmap("assets/characters/pacup.png");
    if (!pacmon_bitmap[DIR_RIGHT]) { std::cout << "Failed to load pacmon right!" << std::endl; return false; }

    hunter_red = al_load_bitmap("assets/characters/hunterr.png");
    if (!hunter_red) { std::cout << "Failed to load red hunter!" << std::endl; return false; }

    hunter_blue = al_load_bitmap("assets/characters/hunterb.png");
    if (!hunter_blue) { std::cout << "Failed to load blue hunter!" << std::endl; return false; }

    hunter_pink = al_load_bitmap("assets/characters/hunterp.png");
    if (!hunter_pink) { std::cout << "Failed to load pink hunter!" << std::endl; return false; }

    hunter_orange = al_load_bitmap("assets/characters/huntero.png");
    if (!hunter_orange) { std::cout << "Failed to load orange hunter!" << std::endl; return false; }

    font = al_load_font("assets/fonts/high_pixel-7.ttf", 25, 0);
    if (!font) { std::cout << "Failed to load font!" << std::endl; return false; }

    banana_sound = al_load_sample("assets/sounds/bananacoincut.wav");
    if (!banana_sound) { std::cout << "Failed to load banana sound!" << std::endl; return false; }

    victory_sound = al_load_sample("assets/sounds/victory.wav");
    if (!victory_sound) { std::cout << "Failed to load victory sound!" << std::endl; return false; }

    defeat_sound = al_load_sample("assets/sounds/youfailed.wav");
    if (!defeat_sound) { std::cout << "Failed to load defeat sound!" << std::endl; return false; }

    music = al_load_audio_stream("assets/sounds/junglemungle.wav", 4, 1024);
    if (!music) { std::cout << "Failed to load music!" << std::endl; return false; }

    event_queue = al_create_event_queue();
    if (!event_queue) { std::cout << "Failed to create event queue!" << std::endl; return false; }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_timer_event_source(logic_timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    al_clear_to_color(al_map_rgb(91, 141, 91));
    al_flip_display();
    al_start_timer(timer);
    al_start_timer(logic_timer);

    return true;
}

void cleanupResources() {
    if (banana_sound) al_destroy_sample(banana_sound);
    if (victory_sound) al_destroy_sample(victory_sound);
    if (defeat_sound) al_destroy_sample(defeat_sound);
    if (music) al_destroy_audio_stream(music);
    for (int i = 0; i < 4; i++) if (pacmon_bitmap[i]) al_destroy_bitmap(pacmon_bitmap[i]);
    if (map_bitmap) al_destroy_bitmap(map_bitmap);
    if (hunter_red) al_destroy_bitmap(hunter_red);
    if (hunter_blue) al_destroy_bitmap(hunter_blue);
    if (hunter_pink) al_destroy_bitmap(hunter_pink);
    if (hunter_orange) al_destroy_bitmap(hunter_orange);
    if (banana_bitmap) al_destroy_bitmap(banana_bitmap);
    if (victory_bitmap) al_destroy_bitmap(victory_bitmap);
    if (defeat_bitmap) al_destroy_bitmap(defeat_bitmap);
    if (font) al_destroy_font(font);
    if (timer) al_destroy_timer(timer);
    if (logic_timer) al_destroy_timer(logic_timer);
    if (display) al_destroy_display(display);
    if (event_queue) al_destroy_event_queue(event_queue);
}