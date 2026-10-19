#include <entity.h>

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

#include <game.h>

void Cyclone_OnUpdate(Entity* ent, Game* g, Uint64 dt) {
  (void)g;
  ent->t += dt;
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
