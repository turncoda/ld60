#include <game.h>

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>

// generated
#include <meta_overworld.h>
#include <meta_smallship_bob.h>
#include <spr_overworld.h>
#include <spr_smallship_bob.h>
#include <tilemap_overworld.h>

Game* Game_Create(void) {
  Game* g = SDL_calloc(1, sizeof(Game));
  Game_Init(g);
  return g;
}

void Game_LoadAssets(Game* g) {
  {
    SDL_Surface* surface = SDL_LoadPNG_IO(
        SDL_IOFromConstMem(spr_overworld, spr_overworld_len), true);
    g->assets.tex_overworld =
        SDL_CreateTextureFromSurface(g->renderer, surface);
    SDL_SetTextureScaleMode(g->assets.tex_overworld, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(surface);
  }
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

  g->player.state = IDLE;
  g->player.loc = (SDL_Point){
      .x = 2,
      .y = 4,
  };

  g->player.origin.x = (g->player.loc.x * kTileWidth) << kSubpixelPrecision;
  g->player.origin.y = (g->player.loc.y * kTileHeight) << kSubpixelPrecision;
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

void Game_RenderChunk(Game* g, int cx, int cy, int px, int py) {
  SDL_Texture* tex = g->assets.tex_overworld;
  int ci = cy * kWorldWidth + cx;
  const Tile* base_ptr = kOverworld[ci];
  for (int y = 0; y < kChunkHeight; y++) {
    for (int x = 0; x < kChunkWidth; x++) {
      int i = y * kChunkWidth + x;
      const Tile* t = base_ptr + i;
      const SDL_Rect* src_rect = &kFrames_overworld[*t].rect;
      Game_RenderTexture(g, tex, src_rect, px + x * kTileWidth,
                         py + y * kTileHeight, false);
    }
  }
}

void Game_RenderOverworld(Game* g) {
  Game_RenderChunk(g, 0, 0, 0, 0);
  Game_RenderChunk(g, 1, 0, kChunkWidth * kTileWidth, 0);
  Game_RenderChunk(g, 0, 1, 0, kChunkHeight * kTileHeight);
  Game_RenderChunk(g, 1, 1, kChunkWidth * kTileWidth,
                   kChunkHeight * kTileHeight);
}

void Game_RenderPlayer(Game* g) {
  SDL_Texture* tex = NULL;
  const SDL_Rect* src_rect = NULL;
  tex = g->assets.tex_smallship_bob;
  src_rect = &kFrames_smallship_bob[0].rect;
  // Units: pixels
  int x = g->player.origin.x >> kSubpixelPrecision;
  int y = g->player.origin.y >> kSubpixelPrecision;
  Game_RenderTexture(g, tex, src_rect, x, y, false);
}

const char* Player_DebugState(PlayerState state) {
  switch (state) {
#define X(NAME) \
  case NAME:    \
    return #NAME;
    PLAYER_STATES(X)
#undef X
  }
}

void Game_RenderDebug(Game* g) {
  SDL_SetRenderScale(g->renderer, 1, 1);
  SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
  SDL_RenderDebugTextFormat(g->renderer, 0, 0, "origin=(%d, %d)",
                            g->player.origin.x >> kSubpixelPrecision,
                            g->player.origin.y >> kSubpixelPrecision);
  SDL_RenderDebugTextFormat(g->renderer, 0, 8, "loc=(%d, %d)", g->player.loc.x,
                            g->player.loc.y);
  SDL_RenderDebugTextFormat(g->renderer, 0, 16, "tgt=(%d, %d)",
                            g->player.target_loc.x, g->player.target_loc.y);
  SDL_RenderDebugTextFormat(g->renderer, 0, 24, "state=%s",
                            Player_DebugState(g->player.state));
  if (g->player.state == MOVE) {
    SDL_FRect r = {.x = g->player.target_loc.x * kTileWidth * g->render_scale,
                   .y = g->player.target_loc.y * kTileHeight * g->render_scale,
                   .w = kTileWidth * g->render_scale,
                   .h = kTileHeight * g->render_scale};
    SDL_SetRenderDrawColor(g->renderer, 255, 0, 0, 255);
    SDL_RenderRect(g->renderer, &r);
  }
}

void Game_Render(Game* g) {
  SDL_SetRenderScale(g->renderer, g->render_scale, g->render_scale);
  SDL_SetRenderDrawColor(g->renderer, 99, 155, 255, 255);
  SDL_RenderClear(g->renderer);
  Game_RenderOverworld(g);
  Game_RenderPlayer(g);
  Game_RenderDebug(g);
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
  if (g->player.state == IDLE) {
    if (g->input.up) {
      g->player.target_loc = g->player.loc;
      g->player.target_loc.y--;
      g->player.state = MOVE;
    }
    if (g->input.down) {
      g->player.target_loc = g->player.loc;
      g->player.target_loc.y++;
      g->player.state = MOVE;
    }
    if (g->input.left) {
      g->player.target_loc = g->player.loc;
      g->player.target_loc.x--;
      g->player.state = MOVE;
    }
    if (g->input.right) {
      g->player.target_loc = g->player.loc;
      g->player.target_loc.x++;
      g->player.state = MOVE;
    }
  }
  if (g->player.state == MOVE) {
    int target_subpixel_x =
        ((g->player.target_loc.x * kTileWidth) << kSubpixelPrecision);
    int target_subpixel_y =
        ((g->player.target_loc.y * kTileHeight) << kSubpixelPrecision);
    int dx = target_subpixel_x - g->player.origin.x;
    int dy = target_subpixel_y - g->player.origin.y;
    if (dx != 0) {
      int sign = dx < 0 ? -1 : 1;
      int d = dt * kPlayerSpeed;
      int max_d = dx * sign;
      int clamped_d = SDL_min(max_d, d);
      int final_d = clamped_d * sign;
      g->player.origin.x += final_d;
    }
    if (dy != 0) {
      int sign = dy < 0 ? -1 : 1;
      int d = dt * kPlayerSpeed;
      int max_d = dy * sign;
      int clamped_d = SDL_min(max_d, d);
      int final_d = clamped_d * sign;
      g->player.origin.y += final_d;
    }
    if (target_subpixel_x == g->player.origin.x &&
        target_subpixel_y == g->player.origin.y) {
      g->player.loc = g->player.target_loc;
      g->player.state = IDLE;
    }
  }
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
