#include <entity.h>

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

#include <game.h>

const SDL_Rect* Entity_GetTexRect(Entity* ent) {
  return &ent->frame_data[ent->frame_i].rect;
}

void Entity_SyncOrigin(Entity* ent) {
  ent->tgt = ent->loc;
  ent->origin.x = (ent->loc.x * kTileWidth) << kSubpixelPrecision;
  ent->origin.y = (ent->loc.y * kTileHeight) << kSubpixelPrecision;
}

void Fish_OnUpdate(Entity* ent, Game* g, Uint64 dt) {
  constexpr int kFishUpMs = 3000;
  if (ent->fish_meter > kFishUpMs) {
    Game_ShowMsg(g, "fish get");
    ent->should_delete = true;
    return;
  }
  if (ent->state == ENT_IDLE) {
    int dx = 0;
    int dy = 0;
    switch (SDL_rand(8)) {
      case 0:
        dx = 1;
        break;
      case 1:
        dx = -1;
        break;
      case 2:
        dy = 1;
        break;
      case 3:
        dy = -1;
        break;
      default:
        break;
    }

    {
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
        dx = 0;
        dy = 0;
        ent->fish_meter += dt;
      }
    }

    ent->tgt.x = ent->loc.x + dx;
    ent->tgt.y = ent->loc.y + dy;

    if (ent->tgt.x < 1 || ent->tgt.x >= kChunkWidth || ent->tgt.y < 1 ||
        ent->tgt.y >= kChunkHeight) {
      ent->tgt = ent->loc;
    }

    if (!PointsEqual(ent->tgt, ent->loc)) {
      ent->state = ENT_MOVE;
    }
  }
  if (ent->state == ENT_MOVE) {
    SDL_Point tgt_subp = {
        .x = (ent->tgt.x * kTileWidth) << kSubpixelPrecision,
        .y = (ent->tgt.y * kTileHeight) << kSubpixelPrecision,
    };
    constexpr int kFishSpeed = 500;
    ent->origin = MoveTowards(ent->origin, tgt_subp, dt * kFishSpeed);
    if (PointsEqual(ent->origin, tgt_subp)) {
      ent->loc = ent->tgt;
      ent->state = ENT_IDLE;
    }
  }
  ent->frame_r += dt;
  int duration = ent->frame_data[ent->frame_i].duration;
  if (ent->frame_r > duration) {
    ent->frame_r -= duration;
    ent->frame_i++;
    ent->frame_i %= ent->num_frames;
  }
}


void Cyclone_OnUpdate(Entity* ent, Game* g, Uint64 dt) {
  (void)g;
  ent->t += dt;
  if (ent->state == ENT_IDLE) {
    int dx = 0;
    int dy = 0;
    switch (SDL_rand(8)) {
      case 0:
        dx = 1;
        break;
      case 1:
        dx = -1;
        break;
      case 2:
        dy = 1;
        break;
      case 3:
        dy = -1;
        break;
      default:
        break;
    }

    {
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
        dx = 0;
        dy = 0;
      }
    }

    ent->tgt.x = ent->loc.x + dx;
    ent->tgt.y = ent->loc.y + dy;

    if (ent->tgt.x < 1 || ent->tgt.x >= kChunkWidth || ent->tgt.y < 1 ||
        ent->tgt.y >= kChunkHeight) {
      ent->tgt = ent->loc;
    }

    if (!PointsEqual(ent->tgt, ent->loc)) {
      ent->state = ENT_MOVE;
    }
  }
  if (ent->state == ENT_MOVE) {
    SDL_Point tgt_subp = {
        .x = (ent->tgt.x * kTileWidth) << kSubpixelPrecision,
        .y = (ent->tgt.y * kTileHeight) << kSubpixelPrecision,
    };
    constexpr int kCycloneSpeed = 500;
    ent->origin = MoveTowards(ent->origin, tgt_subp, dt * kCycloneSpeed);
    if (PointsEqual(ent->origin, tgt_subp)) {
      ent->loc = ent->tgt;
      ent->state = ENT_IDLE;
    }
  }
  ent->frame_r += dt;
  int duration = ent->frame_data[ent->frame_i].duration;
  if (ent->frame_r > duration) {
    ent->frame_r -= duration;
    ent->frame_i++;
    ent->frame_i %= ent->num_frames;
  }
}

bool Village_OnInteract(Entity* ent, Game* g) {
  Game_InitDialog(g, ent);
  return false;
}

const char* EntityType_ToString(EntityType type) {
  switch (type) {
#define X(NAME) \
  case NAME:    \
    return #NAME;
    ENTITY_TYPES(X)
#undef X
  }
}

bool Entity_ApplyShopData(Entity* ent, int cx, int cy, int x, int y) {
  SDL_assert(ent->type == VILLAGE);
  // load: village @ chunk=(77,67), loc=(11,6)
  // load: village @ chunk=(77,69), loc=(5,5)
  if (cx == 77 && cy == 67 && x == 11 && y == 6) {
    ent->shop_data[ItemType_sturgeon] = 999;
    return true;
  }
  if (cx == 77 && cy == 69 && x == 5 && y == 5) {
    ent->shop_data[ItemType_cod] = 20;
    return true;
  }
  SDL_Log("village spawned with no shop data: chunk=(%d,%d), loc=(%d,%d)", cx,
          cy, x, y);
  return false;
}
