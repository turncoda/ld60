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

#include <entity.h>

#include <data.h>

// generated
#include <spr_overworld2.h>
#include <spr_sky.h>

// generated
// NOTE update when new asset is introduced
#include <meta_cyclone.h>
#include <spr_cyclone.h>

#include <meta_smallship_bob.h>
#include <spr_smallship_bob.h>

#include <meta_tiles.h>
#include <spr_tiles.h>

#include <meta_village.h>
#include <spr_village.h>

Uint8 u8_hash(int n) {
  n *= 198347193467913;
  n = n >> 7;
  n *= 347918734983418;
  n = n << 5;
  n *= 987972459874791;
  return n % 255;
}

bool PointsEqual(SDL_Point a, SDL_Point b) {
  return a.x == b.x && a.y == b.y;
}

// Returns a moved towards b by amt.
SDL_Point MoveTowards(SDL_Point a, SDL_Point b, int amt) {
  SDL_Point result = a;
  int dx = b.x - a.x;
  int dy = b.y - a.y;
  if (dx != 0) {
    int sign = dx < 0 ? -1 : 1;
    int d = amt;
    int max_d = dx * sign;
    int clamped_d = SDL_min(max_d, d);
    int final_d = clamped_d * sign;
    result.x += final_d;
  }
  if (dy != 0) {
    int sign = dy < 0 ? -1 : 1;
    int d = amt;
    int max_d = dy * sign;
    int clamped_d = SDL_min(max_d, d);
    int final_d = clamped_d * sign;
    result.y += final_d;
  }
  return result;
}

void Game_ShowDialog(Game* g, const char* msg) {
  g->player.state = DIALOG;
  g->shop_menu = (ShopMenu){0};
  SDL_Log("%s", msg);
}

Entity Game_NewEntity(Game* g, int x, int y, EntityType type) {
  Entity ent = {0};
  SDL_Log("Spawning %s at %d,%d", EntityType_ToString(type), x, y);
  // NOTE update when new entity type introduced
  ent.type = type;
  ent.loc.x = x;
  ent.loc.y = y;
  Entity_SyncOrigin(&ent);
  ent.collider.w = 8;
  ent.collider.h = 8;
  switch (type) {
    case VILLAGE:
      ent.tex = g->assets.tex_village;
      ent.frame_data = kFrames_village;
      ent.num_frames = kNumFrames_village;
      ent.on_interact = Village_OnInteract;
      break;
    case CYCLONE:
      ent.tex = g->assets.tex_cyclone;
      ent.frame_data = kFrames_cyclone;
      ent.num_frames = kNumFrames_cyclone;
      ent.on_update = Cyclone_OnUpdate;
      break;
    default:
      break;
  }
  return ent;
}

void Game_SyncPlayerOrigin(Game* g) {
  g->player.origin.x = (g->player.loc.x * kTileWidth) << kSubpixelPrecision;
  g->player.origin.y = (g->player.loc.y * kTileHeight) << kSubpixelPrecision;
}

void Game_ResetEntities(Game* g) {
  g->num_ent = 0;
  int ci = g->cc.y * g->world_width + g->cc.x;
  EntityType* bp = &g->entity_spawns[ci * kChunkHeight * kChunkWidth];
  for (int y = 0; y < kChunkHeight; y++) {
    for (int x = 0; x < kChunkWidth; x++) {
      int i = y * kChunkWidth + x;
      EntityType type = *(bp + i);
      if (type == NOTYPE) {
        continue;
      }
      Entity ent = Game_NewEntity(g, x, y, type);
      g->entities[g->num_ent++] = ent;
    }
  }
}

Game* Game_Create(void) {
  Game* g = SDL_calloc(1, sizeof(Game));
  Game_OneTimeSetup(g);
  return g;
}

void Game_InitBuffers(Game* g) {
  g->entities = SDL_calloc(kMaxNumEntities, sizeof(Entity));
}

void Game_LoadAssets(Game* g) {
  {
    SDL_Surface* surface = SDL_LoadPNG_IO(
        SDL_IOFromConstMem(spr_overworld2, spr_overworld2_len), true);
    // assumes pixel format is RGBA8
    Uint32* pixels = surface->pixels;
    g->overworld = SDL_calloc(surface->w * surface->h, sizeof(Tile2));
    g->entity_spawns = SDL_calloc(surface->w * surface->h, sizeof(EntityType));
    g->world_width = surface->w / kChunkWidth;
    g->world_height = surface->h / kChunkHeight;
    int num_unknown = 0;
    for (int y = 0; y < surface->h; y++) {
      for (int x = 0; x < surface->w; x++) {
        int i = y * surface->w + x;
        int cx = x / kChunkWidth;
        int cy = y / kChunkHeight;
        int ci = cy * g->world_width + cx;
        int rx = x % kChunkWidth;
        int ry = y % kChunkHeight;
        int j = ci * kChunkWidth * kChunkHeight + ry * kChunkWidth + rx;
        Uint32 d = pixels[i];

        // NOTE update when new color is introduced
        // color format: 0xAABBGGRR
        switch (d) {
          case 0xff0000ff: /* player */
            g->overworld[j] = WATER;
            g->cc.x = cx;
            g->cc.y = cy;
            g->player.loc.x = rx;
            g->player.loc.y = ry;
            break;
          case 0xff244b52: /* mountain */
            g->overworld[j] = MOUNTAIN;
            break;
          case 0xff9ac3ee: /* sand */
            g->overworld[j] = SAND;
            break;
          case 0xff30be6a: /* grass */
            g->overworld[j] = GRASS;
            break;
          case 0xffff9b63: /* water */
            g->overworld[j] = WATER;
            break;
          case 0xff8a4276: /* village */
            g->overworld[j] = GRASS;
            g->entity_spawns[j] = VILLAGE;
            break;
          case 0xffffffff: /* cyclone */
            g->overworld[j] = WATER;
            g->entity_spawns[j] = CYCLONE;
            break;
          default:
            num_unknown++;
            SDL_Log("warning: unknown color 0x%x", d);
            break;
        }
      }
    }
    if (num_unknown > 0) {
      SDL_Log(
          "warning: encountered %d unknown tiles while parsing overworld map",
          num_unknown);
    }
    SDL_DestroySurface(surface);
  }
  {
    SDL_Surface* surface =
        SDL_LoadPNG_IO(SDL_IOFromConstMem(spr_sky, spr_sky_len), true);
    // assumes pixel format is RGBA8
    const int half_w = surface->w / 2;
    const int half_h = surface->h / 2;
    Uint32* pixels = surface->pixels;
    int num_points = 0;
    for (int y = 0; y < surface->h; y++) {
      for (int x = 0; x < surface->w; x++) {
        int i = y * surface->w + x;
        Uint32 d = pixels[i];
        constexpr Uint32 kWhite = ~0u;
        if (d == kWhite) {
          num_points++;
        }
      }
    }
    g->num_stars = num_points;
    g->stars = SDL_calloc(num_points, sizeof(Star));
    int j = 0;
    for (int y = 0; y < surface->h; y++) {
      for (int x = 0; x < surface->w; x++) {
        int i = y * surface->w + x;
        Uint32 d = pixels[i];
        constexpr Uint32 kWhite = ~0u;
        if (d == kWhite) {
          Star* star = g->stars + j;
          star->loc.x = x - half_w;
          star->loc.y = y - half_h;
          star->brightness = u8_hash(i);
          j++;
        }
      }
    }
    SDL_Log("loaded %d stars", num_points);
    SDL_DestroySurface(surface);
  }

#define X(NAME)                                                                \
  {                                                                            \
    SDL_Surface* surface = SDL_LoadPNG_IO(                                     \
        SDL_IOFromConstMem(spr_##NAME, spr_##NAME##_len), true);               \
    g->assets.tex_##NAME = SDL_CreateTextureFromSurface(g->renderer, surface); \
    SDL_SetTextureScaleMode(g->assets.tex_##NAME, SDL_SCALEMODE_NEAREST);      \
    SDL_DestroySurface(surface);                                               \
  }
  SPRITES(X)
#undef X
}

void Game_OneTimeSetup(Game* g) {
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
}

void Game_Init(Game* g) {
  g->time_mult = 1;
  g->player.state = IDLE;
  g->player.tgt = g->player.loc;
  g->player.collider.w = 8;
  g->player.collider.h = 8;
  g->player.warp_meter = 0;
  Game_SyncPlayerOrigin(g);
  Game_ResetEntities(g);
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
                        double angle,
                        bool mirror_x) {
  SDL_assert(src_rect);
  SDL_FRect s, d;
  SDL_RectToFRect(src_rect, &s);
  SDL_RectToFRect(src_rect, &d);
  d.x = x;
  d.y = y;
  SDL_RenderTextureRotated(g->renderer, tex, &s, &d, angle, NULL,
                           mirror_x ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void Game_RenderChunk(Game* g, int cx, int cy, int px, int py) {
  if (cx < 0 || cy < 0 || cx >= g->world_width || cy >= g->world_height) {
    return;
  }
  SDL_Texture* tex = g->assets.tex_tiles;
  int ci = cy * g->world_width + cx;
  Tile2* base_ptr = &g->overworld[ci * kChunkWidth * kChunkHeight];
  for (int y = 0; y < kChunkHeight; y++) {
    for (int x = 0; x < kChunkWidth; x++) {
      int i = y * kChunkWidth + x;
      Tile2* t = base_ptr + i;
      const SDL_Rect* src_rect = &kFrames_tiles[*t].rect;
      Game_RenderTexture(g, tex, src_rect, px + x * kTileWidth,
                         py + y * kTileHeight, 0, false);
    }
  }
}

void Game_RenderOverworld(Game* g) {
  Game_RenderChunk(g, g->cc.x, g->cc.y, 0, 0);
  Game_RenderChunk(g, g->cc.x + 1, g->cc.y, kChunkWidth * kTileWidth, 0);
  Game_RenderChunk(g, g->cc.x, g->cc.y + 1, 0, kChunkHeight * kTileHeight);
  Game_RenderChunk(g, g->cc.x + 1, g->cc.y + 1, kChunkWidth * kTileWidth,
                   kChunkHeight * kTileHeight);
}

void Game_RenderEntities(Game* g) {
  for (int i = 0; i < g->num_ent; i++) {
    Entity* ent = &g->entities[i];
    int x = ent->origin.x >> kSubpixelPrecision;
    int y = ent->origin.y >> kSubpixelPrecision;
    Game_RenderTexture(g, ent->tex, Entity_GetTexRect(ent), x, y, 0, false);
  }
}

void Game_RenderPlayer(Game* g) {
  SDL_Texture* tex = NULL;
  const SDL_Rect* src_rect = NULL;
  tex = g->assets.tex_smallship_bob;
  src_rect = &kFrames_smallship_bob[0].rect;
  // Units: pixels
  int x = g->player.origin.x >> kSubpixelPrecision;
  int y = g->player.origin.y >> kSubpixelPrecision;
  double angle = g->player.rot_counter2 * 90;
  Game_RenderTexture(g, tex, src_rect, x, y, angle, false);
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

void Game_RenderDialog(Game* g) {
  constexpr int kPad = 12;
  constexpr int kMargin = 12;
  SDL_FRect r = {
      .x = kPad,
      .y = kPad,
      .w = kViewportWidth - 2 * kPad,
      .h = kViewportHeight - 2 * kPad,
  };
  SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 255);
  SDL_RenderFillRect(g->renderer, &r);
  if (g->shop_menu.mode == ShopMode_Buy) {
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
  } else {
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 128);
  }
  SDL_RenderDebugTextFormat(g->renderer, kPad + kMargin, kPad + kMargin, "buy");
  if (g->shop_menu.mode == ShopMode_Sell) {
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
  } else {
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 128);
  }
  SDL_RenderDebugTextFormat(g->renderer, kPad + kMargin + 5 * kTileWidth,
                            kPad + kMargin, "sell");
}

void Game_RenderMap(Game* g) {
  SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, 128);
  {
    SDL_FRect r = {
        .x = 0,
        .y = 0,
        .w = kViewportWidth,
        .h = kViewportHeight,
    };
    SDL_RenderFillRect(g->renderer, &r);
  }
  for (int i = 0; i < g->num_stars; i++) {
    Star* star = &g->stars[i];
    Uint8 v = star->brightness;
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, v);
    SDL_RenderPoint(g->renderer, star->xfm.x, star->xfm.y);
  }

  SDL_SetRenderDrawColor(g->renderer, 255, 0, 0, 255);
  SDL_RenderPoint(g->renderer, g->cc.x, g->cc.y);
}

void Game_RenderDebug(Game* g) {
  SDL_SetRenderScale(g->renderer, 1, 1);
  SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
  SDL_RenderDebugTextFormat(g->renderer, 0, 0, "origin=(%d, %d)",
                            g->player.origin.x >> kSubpixelPrecision,
                            g->player.origin.y >> kSubpixelPrecision);
  SDL_RenderDebugTextFormat(g->renderer, 0, 8, "loc=(%d, %d)", g->player.loc.x,
                            g->player.loc.y);
  SDL_RenderDebugTextFormat(g->renderer, 0, 16, "tgt=(%d, %d)", g->player.tgt.x,
                            g->player.tgt.y);
  SDL_RenderDebugTextFormat(g->renderer, 0, 24, "c=(%d, %d)", g->cc.x, g->cc.y);
  SDL_RenderDebugTextFormat(g->renderer, 0, 32, "state=%s",
                            Player_DebugState(g->player.state));
  SDL_RenderDebugTextFormat(g->renderer, 0, 40, "num_ent=%d", g->num_ent);
  SDL_RenderDebugTextFormat(g->renderer, 0, 48, "warp=%d",
                            g->player.warp_meter);
  SDL_RenderDebugTextFormat(g->renderer, 0, 56, "rot2=%d",
                            g->player.rot_counter2);
  if (g->player.state == MOVE) {
    SDL_FRect r = {.x = g->player.tgt.x * kTileWidth * g->render_scale,
                   .y = g->player.tgt.y * kTileHeight * g->render_scale,
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
  Game_RenderEntities(g);
  Game_RenderPlayer(g);
  if (g->is_map_visible) {
    Game_RenderMap(g);
  }
  if (g->player.state == DIALOG) {
    Game_RenderDialog(g);
  }
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
  if (g->player.state == DIALOG) {
    if (g->input.b && !g->prev_input.b) {
      g->player.state = IDLE;
    }
    if (g->input.select && !g->prev_input.select) {
      g->shop_menu.mode++;
      g->shop_menu.mode %= ShopMode_Count;
    }
    SDL_memcpy(&g->prev_input, &g->input, sizeof(g->input));
    return;
  }

  if (g->player.warp_meter == 0) {
    g->player.rot_counter = 0;
    g->player.rot_counter2 = 0;
  } else {
    g->player.rot_counter += dt * g->player.warp_meter;
    constexpr int kRotCounterPrecision = 19;
    g->player.rot_counter2 =
        (g->player.rot_counter >> kRotCounterPrecision) % 4;
  }

  bool is_player_touching_cyclone = false;
  for (int i = 0; i < g->num_ent; i++) {
    Entity* ent = &g->entities[i];
    if (ent->on_update) {
      ent->on_update(ent, g, dt);
    }
    if (ent->type == CYCLONE) {
      SDL_Rect ent_rect = {
          .x = ent->origin.x >> kSubpixelPrecision,
          .y = ent->origin.y >> kSubpixelPrecision,
          .w = ent->collider.w,
          .h = ent->collider.h,
      };
      SDL_Rect player_rect = {
          .x = g->player.origin.x >> kSubpixelPrecision,
          .y = g->player.origin.y >> kSubpixelPrecision,
          .w = g->player.collider.w,
          .h = g->player.collider.h,
      };
      if (SDL_HasRectIntersection(&ent_rect, &player_rect)) {
        is_player_touching_cyclone = true;
      }
    }
  }

  constexpr int kWarpMeterAttackSpeed = 2;
  constexpr int kWarpMeterDecaySpeed = 8;
  if (is_player_touching_cyclone) {
    g->player.warp_meter += dt * kWarpMeterAttackSpeed;
    constexpr int kWarpMeterMax = 5000;
    if (g->player.warp_meter > kWarpMeterMax) {
      g->player.warp_meter = kWarpMeterMax;
    }
  } else {
    Uint64 decay_amt = dt * kWarpMeterDecaySpeed;
    if (g->player.warp_meter < decay_amt) {
      g->player.warp_meter = 0;
    } else {
      g->player.warp_meter -= decay_amt;
    }
  }

  if (!g->prev_input.select && g->input.select) {
    g->is_map_visible = !g->is_map_visible;
  }

  g->day_ticks += g->time_mult * dt;

  for (int i = 0; i < g->num_stars; i++) {
    float x = g->stars[i].loc.x;
    float y = g->stars[i].loc.y;
    float theta = (g->day_ticks >> kDayTickPrecision) * kPi / 180;
    float s = SDL_sinf(theta);
    float c = SDL_cosf(theta);
    float rx = c * x + -s * y;
    float ry = s * x + c * y;
    g->stars[i].xfm.x = rx + (float)kViewportWidth / 2;
    g->stars[i].xfm.y = ry + (float)kViewportHeight / 2;
  }

  if (g->player.state == IDLE) {
    if (g->input.up) {
      g->player.tgt = g->player.loc;
      g->player.tgt.y--;
    }
    if (g->input.down) {
      g->player.tgt = g->player.loc;
      g->player.tgt.y++;
    }
    if (g->input.left) {
      g->player.tgt = g->player.loc;
      g->player.tgt.x--;
    }
    if (g->input.right) {
      g->player.tgt = g->player.loc;
      g->player.tgt.x++;
    }

    if (!PointsEqual(g->player.tgt, g->player.loc)) {
      g->player.state = MOVE;
      for (int i = 0; i < g->num_ent; i++) {
        Entity* ent = &g->entities[i];
        if (PointsEqual(ent->loc, g->player.tgt)) {
          if (ent->on_interact) {
            if (!ent->on_interact(ent, g)) {
              g->player.tgt = g->player.loc;
            }
          }
        }
      }
    }
  }

  if (g->player.state == MOVE) {
    SDL_Point prev_cc = g->cc;
    if (g->player.tgt.x > kChunkWidth) {
      g->cc.x++;
      g->player.tgt.x -= kChunkWidth;
      g->player.loc.x -= kChunkWidth;
    } else if (g->player.tgt.x < 1) {
      g->cc.x--;
      g->player.tgt.x += kChunkWidth;
      g->player.loc.x += kChunkWidth;
    }
    if (g->player.tgt.y > kChunkHeight) {
      g->cc.y++;
      g->player.tgt.y -= kChunkHeight;
      g->player.loc.y -= kChunkHeight;
    } else if (g->player.tgt.y < 1) {
      g->cc.y--;
      g->player.tgt.y += kChunkHeight;
      g->player.loc.y += kChunkHeight;
    }

    if (!(g->cc.x == prev_cc.x && g->cc.y == prev_cc.y)) {
      Game_SyncPlayerOrigin(g);
      Game_ResetEntities(g);
    }
  }

  if (g->player.state == MOVE) {
    int target_subpixel_x =
        ((g->player.tgt.x * kTileWidth) << kSubpixelPrecision);
    int target_subpixel_y =
        ((g->player.tgt.y * kTileHeight) << kSubpixelPrecision);
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
      g->player.loc = g->player.tgt;
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
