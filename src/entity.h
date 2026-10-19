#pragma once

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>

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

#define ENTITY_STATES(X) \
  X(ENT_IDLE)            \
  X(ENT_MOVE)

typedef enum {
#define X(NAME) NAME,
  ENTITY_STATES(X)
#undef X
} EntityState;

typedef struct Game Game;
typedef struct Entity Entity;
typedef bool (*OnInteract)(Entity*, Game*);
typedef void (*OnUpdate)(Entity*, Game*, Uint64);

typedef struct Entity {
  EntityType type;
  EntityState state;
  SDL_Point origin;
  SDL_Point loc;
  SDL_Point tgt;
  SDL_Texture* tex;
  SDL_Rect tex_rect;
  int meter;
  Uint64 t;
  OnInteract on_interact;
  OnUpdate on_update;
} Entity;

void Cyclone_OnUpdate(Entity* ent, Game* g, Uint64 dt);
bool Village_OnInteract(Entity* ent, Game* g);
const char* EntityType_ToString(EntityType type);
