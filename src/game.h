#pragma once

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>

constexpr int kViewportWidth = 160;
constexpr int kViewportHeight = 144;
constexpr Uint8 kScaleFactor = 4;

constexpr int kMaxNumGamepads = 8;
constexpr int kSubpixelPrecision = 16;

constexpr int kPlayerSpeed = 3200;

constexpr int kDayTickPrecision = 10;

constexpr float kPi = 3.141592f;

typedef struct {
  SDL_GamepadButton a;
  SDL_GamepadButton b;
  SDL_GamepadButton start;
  SDL_GamepadButton select;
  SDL_GamepadButton up;
  SDL_GamepadButton down;
  SDL_GamepadButton left;
  SDL_GamepadButton right;
} GamepadInputMap;

typedef struct {
  SDL_Scancode a;
  SDL_Scancode b;
  SDL_Scancode start;
  SDL_Scancode select;
  SDL_Scancode up;
  SDL_Scancode down;
  SDL_Scancode left;
  SDL_Scancode right;
} KeyboardInputMap;

typedef struct {
  SDL_Texture* tex_smallship_bob;
  SDL_Texture* tex_tiles;
} Assets;

#define ENTITY_TYPES(X) \
  X(BLANK)              \
  X(WATER)              \
  X(GRASS)              \
  X(SAND)

typedef enum {
#define X(NAME) NAME,
  ENTITY_TYPES(X)
#undef X
} Tile2;

#define PLAYER_STATES(X) \
  X(IDLE)                \
  X(MOVE)

typedef enum {
#define X(NAME) NAME,
  PLAYER_STATES(X)
#undef X
} PlayerState;

typedef struct {
  // Units: subpixels
  SDL_Point origin;
  // Units: pixels
  // (Relative to origin)
  SDL_Rect collider;

  // Units: subpixels per millisecond
  SDL_Point vel;

  // Units: tile coordinates
  SDL_Point loc;

  // Units: tile coordinates
  SDL_Point tgt;

  PlayerState state;

} Player;

typedef struct {
  bool a;
  bool b;
  bool start;
  bool select;
  bool up;
  bool down;
  bool left;
  bool right;
} Input;

typedef struct {
  SDL_Point loc;
  SDL_FPoint xfm;
  Uint8 brightness;
} Star;

typedef struct {
  SDL_Renderer* renderer;
  SDL_Rect viewport;
  Uint8 render_scale;
  Assets assets;
  SDL_Gamepad* gamepads[kMaxNumGamepads];
  GamepadInputMap gamepad_input_map;
  KeyboardInputMap keyboard_input_map;
  Input prev_input;
  Input input;
  Player player;
  // Units: chunk coordinates
  SDL_Point cc;

  Star* stars;
  SDL_Point* o_stars;
  SDL_FPoint* t_stars;
  int num_stars;

  Uint64 day_ticks;
  Uint8 time_mult;
  bool is_map_visible;

  Tile2* overworld;
  // Units: chunks
  int world_width;
  // Units: chunks
  int world_height;
} Game;

Game* Game_Create(void);
void Game_LoadAssets(Game* g);
void Game_Init(Game* g);
SDL_Rect Game_GetWindowSize(Game* g);
void Game_PollInput(Game* g);
void Game_Render(Game* g);
void Game_Update(Game* g, Uint64 dt);
void Game_AddGamepad(Game* g, SDL_Gamepad* gamepad);
void Game_RemoveGamepad(Game* g, SDL_Gamepad* gamepad);
