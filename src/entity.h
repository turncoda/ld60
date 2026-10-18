#pragma once

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

// NOTE update when new entity type introduced
#define ENTITY_TYPES(X) \
  X(NOTYPE)             \
  X(VILLAGE)            \
  X(CYCLONE)

typedef enum {
#define X(NAME) NAME,
  ENTITY_TYPES(X)
#undef X
} EntityType;

typedef struct Game Game;
typedef struct Entity Entity;
typedef bool (*OnInteract)(Entity*, Game*);

typedef struct Entity {
  EntityType type;
  SDL_Point loc;
  SDL_Texture* tex;
  SDL_Rect src_rect;
  OnInteract on_interact;
} Entity;

bool Entity_OnInteractImpassable(Entity* ent, Game* g);
const char* EntityType_ToString(EntityType type);
