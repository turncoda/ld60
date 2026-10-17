#pragma once

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>

constexpr int kMaxNumGamepads = 8;
constexpr int kSubpixelPrecision = 16;

constexpr int kPlayerSpeed = 2000;

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
  SDL_Texture* tex_overworld;
} Assets;

typedef struct {
  // Units: subpixels
  SDL_Point origin;
  // Units: pixels
  // (Relative to origin)
  SDL_Rect collider;

  // Units: subpixels per millisecond
  SDL_Point vel;
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
