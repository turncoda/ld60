#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>

#include <game.h>

static SDL_Window* window = NULL;
static Uint32 last_ticks = 0;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  Game* g = Game_Create();
  *appstate = g;

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_Rect window_size = Game_GetWindowSize(g);
  if (!SDL_CreateWindowAndRenderer("mygame", window_size.w, window_size.h,
                                   SDL_WINDOW_RESIZABLE, &window,
                                   &g->renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_SetRenderVSync(g->renderer, 1);
  SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);

  Game_InitBuffers(g);
  Game_LoadAssets(g);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  Game* g = (Game*)appstate;
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  } else if (event->type == SDL_EVENT_GAMEPAD_ADDED) {
    // this event is sent for each hotplugged stick, but also each
    // already-connected gamepad during SDL_Init().
    const SDL_JoystickID which = event->gdevice.which;
    SDL_Gamepad* gamepad = SDL_OpenGamepad(which);
    Game_AddGamepad(g, gamepad);
    if (!gamepad) {
      SDL_Log("Gamepad #%u add, but not opened: %s", (unsigned int)which,
              SDL_GetError());
    } else {
      char* mapping = SDL_GetGamepadMapping(gamepad);
      SDL_Log("Gamepad #%u ('%s') added", (unsigned int)which,
              SDL_GetGamepadName(gamepad));
      if (mapping) {
        SDL_Log("Gamepad #%u mapping: %s", (unsigned int)which, mapping);
        SDL_free(mapping);
      }
    }
  } else if (event->type == SDL_EVENT_GAMEPAD_REMOVED) {
    const SDL_JoystickID which = event->gdevice.which;
    SDL_Gamepad* gamepad = SDL_GetGamepadFromID(which);
    Game_RemoveGamepad(g, gamepad);
    if (gamepad) {
      SDL_CloseGamepad(gamepad); /* the gamepad was unplugged. */
    }
    SDL_Log("Gamepad #%u removed", (unsigned int)which);
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  Game* g = (Game*)appstate;

  Game_PollInput(g);

  const Uint64 now = SDL_GetTicks();

  Uint32 dt = (Uint32)(now - last_ticks);
  last_ticks = now;

  // Clamp dt in case of massive stutter (e.g. on startup).
  // By clamping to 33, we ensure that, below 30Hz, the game steps don't get any
  // bigger.
  dt = SDL_min(33, dt);

  Game_Update(g, dt);
  Game_Render(g);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  (void)appstate;
  (void)result;
  SDL_Quit();
}
