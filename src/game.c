#include <game.h>

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>

// generated
#include <meta_smallship_bob.h>
#include <spr_smallship_bob.h>

Game* Game_Create(void) {
  Game* g = SDL_calloc(1, sizeof(Game));
  Game_Init(g);
  return g;
}

void Game_LoadAssets(Game* g) {
  {
    SDL_Surface* surface = SDL_LoadPNG_IO(
        SDL_IOFromConstMem(spr_smallship_bob, spr_smallship_bob_len), true);
    g->assets.tex_smallship_bob =
        SDL_CreateTextureFromSurface(g->renderer, surface);
    SDL_SetTextureScaleMode(g->assets.tex_smallship_bob, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(surface);
  }
}

void Game_Init(Game* g) {
  constexpr int kViewportWidth = 160;
  constexpr int kViewportHeight = 144;
  constexpr Uint8 kScaleFactor = 4;

  g->viewport.w = kViewportWidth;
  g->viewport.h = kViewportHeight;
  g->render_scale = kScaleFactor;

  g->gamepad_input_map = (GamepadInputMap){
      .a = SDL_GAMEPAD_BUTTON_EAST,
      .b = SDL_GAMEPAD_BUTTON_SOUTH,
      .start = SDL_GAMEPAD_BUTTON_START,
      .select = SDL_GAMEPAD_BUTTON_BACK,
      .up = SDL_GAMEPAD_BUTTON_DPAD_UP,
      .down = SDL_GAMEPAD_BUTTON_DPAD_DOWN,
      .left = SDL_GAMEPAD_BUTTON_DPAD_LEFT,
      .right = SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
  };

  g->keyboard_input_map = (KeyboardInputMap){
      .a = SDL_SCANCODE_Z,
      .b = SDL_SCANCODE_X,
      .start = SDL_SCANCODE_RETURN,
      .select = SDL_SCANCODE_TAB,
      .up = SDL_SCANCODE_UP,
      .down = SDL_SCANCODE_DOWN,
      .left = SDL_SCANCODE_LEFT,
      .right = SDL_SCANCODE_RIGHT,
  };

  g->player.origin.x = g->viewport.w >> 1 << kSubpixelPrecision;
  g->player.origin.y = g->viewport.h >> 1 << kSubpixelPrecision;
}

SDL_Rect Game_GetWindowSize(Game* g) {
  return (SDL_Rect){
      .w = g->viewport.w * g->render_scale,
      .h = g->viewport.h * g->render_scale,
  };
}

void Game_RenderTexture(Game* g,
                        SDL_Texture* tex,
                        const SDL_Rect* src_rect,
                        int x,
                        int y,
                        bool mirror_x) {
  SDL_assert(src_rect);
  SDL_FRect s, d;
  SDL_RectToFRect(src_rect, &s);
  SDL_RectToFRect(src_rect, &d);
  d.x = x;
  d.y = y;
  SDL_RenderTextureRotated(g->renderer, tex, &s, &d, 0, NULL,
                           mirror_x ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void Game_RenderPlayer(Game* g) {
  SDL_Texture* tex = NULL;
  const SDL_Rect* src_rect = NULL;
  const SDL_Point* spr_offset = NULL;
  tex = g->assets.tex_smallship_bob;
  src_rect = &kFrames_smallship_bob[0].rect;
  spr_offset = &kPivot_smallship_bob;
  // Units: pixels
  int x = g->player.origin.x >> kSubpixelPrecision;
  int y = g->player.origin.y >> kSubpixelPrecision;
  y -= spr_offset->y;
  Game_RenderTexture(g, tex, src_rect, x, y, false);
}

void Game_Render(Game* g) {
  SDL_SetRenderScale(g->renderer, g->render_scale, g->render_scale);
  SDL_SetRenderDrawColor(g->renderer, 99, 155, 255, 255);
  SDL_RenderClear(g->renderer);
  Game_RenderPlayer(g);
  SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
  SDL_SetRenderScale(g->renderer, 2, 2);
  SDL_RenderDebugTextFormat(g->renderer, 0, 0, "vx=%d", g->player.vel.x);
  SDL_RenderDebugTextFormat(g->renderer, 0, 8, "vy=%d", g->player.vel.y);
  SDL_RenderPresent(g->renderer);
}

void Game_PollInput(Game* g) {
  SDL_PumpEvents();
  g->input = (Input){0};
  for (int i = 0; i < kMaxNumGamepads; i++) {
    SDL_Gamepad* gpad = g->gamepads[i];
    if (gpad == NULL) {
      continue;
    }
    GamepadInputMap* im = &g->gamepad_input_map;
    g->input.a = g->input.a || SDL_GetGamepadButton(gpad, im->a);
    g->input.b = g->input.b || SDL_GetGamepadButton(gpad, im->b);
    g->input.start = g->input.start || SDL_GetGamepadButton(gpad, im->start);
    g->input.select = g->input.select || SDL_GetGamepadButton(gpad, im->select);
    g->input.up = g->input.up || SDL_GetGamepadButton(gpad, im->up);
    g->input.down = g->input.down || SDL_GetGamepadButton(gpad, im->down);
    g->input.left = g->input.left || SDL_GetGamepadButton(gpad, im->left);
    g->input.right = g->input.right || SDL_GetGamepadButton(gpad, im->right);
  }
  {
    const bool* keyboard_state = SDL_GetKeyboardState(NULL);
    KeyboardInputMap* im = &g->keyboard_input_map;
    g->input.a = g->input.a || keyboard_state[im->a];
    g->input.b = g->input.b || keyboard_state[im->b];
    g->input.start = g->input.start || keyboard_state[im->start];
    g->input.select = g->input.select || keyboard_state[im->select];
    g->input.up = g->input.up || keyboard_state[im->up];
    g->input.down = g->input.down || keyboard_state[im->down];
    g->input.left = g->input.left || keyboard_state[im->left];
    g->input.right = g->input.right || keyboard_state[im->right];
  }
}

void Game_Update(Game* g, Uint64 dt) {
  (void)g;
  (void)dt;
  g->player.vel.x = 0;
  g->player.vel.y = 0;
  if (g->input.up) {
    g->player.vel.y -= kPlayerSpeed;
  }
  if (g->input.down) {
    g->player.vel.y += kPlayerSpeed;
  }
  if (g->input.left) {
    g->player.vel.x -= kPlayerSpeed;
  }
  if (g->input.right) {
    g->player.vel.x += kPlayerSpeed;
  }
  g->player.origin.x += dt * g->player.vel.x;
  g->player.origin.y += dt * g->player.vel.y;
  SDL_memcpy(&g->prev_input, &g->input, sizeof(g->input));
}

void Game_AddGamepad(Game* g, SDL_Gamepad* gamepad) {
  for (int i = 0; i < kMaxNumGamepads; i++) {
    if (g->gamepads[i] == NULL) {
      g->gamepads[i] = gamepad;
      break;
    }
  }
}

void Game_RemoveGamepad(Game* g, SDL_Gamepad* gamepad) {
  for (int i = 0; i < kMaxNumGamepads; i++) {
    if (g->gamepads[i] == gamepad) {
      g->gamepads[i] = NULL;
      break;
    }
  }
}
