#include <entity.h>

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

void Cyclone_OnUpdate(Entity* ent, Game* g, Uint64 dt) {
  (void)g;
  ent->t += dt;
  ent->frame_r += dt;
  int duration = ent->frame_data[ent->frame_i].duration;
  if (ent->frame_r > duration) {
    ent->frame_r -= duration;
    ent->frame_i++;
    ent->frame_i %= ent->num_frames;
  }
}

bool Village_OnInteract(Entity* ent, Game* g) {
  (void)ent;
  Game_ShowDialog(g, "you shall not pass");
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
