#include <entity.h>

#include <SDL3/SDL_log.h>

#include <game.h>

bool Entity_OnInteractImpassable(Entity* ent, Game* g) {
  (void)g;
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
