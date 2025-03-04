#include "include/game.h"
#include "include/entities.h"
#include "include/resources.h"
#include <iostream>

int main(int argc, char **argv) {
    if (!initializeResources()) {
        std::cout << "Initialization failed!" << std::endl;
        return -1;
    }

    al_attach_audio_stream_to_mixer(music, al_get_default_mixer());
    al_set_audio_stream_playing(music, true);

    runGameLoop();

    cleanupResources();
    return 0;
}