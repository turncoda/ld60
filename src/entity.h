#pragma once

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>

#include <data.h>

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
  X(ENT_MOVE)            \
  X(ENT_STALL)

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
  SDL_Rect collider;
  SDL_Point origin;
  SDL_Point loc;
  SDL_Point tgt;
  SDL_Texture* tex;
  int meter;
  Uint64 t;
  Uint8 frame_i;
  Uint16 frame_r;
  const Frame* frame_data;
  int num_frames;
  OnInteract on_interact;
  OnUpdate on_update;
} Entity;

const SDL_Rect* Entity_GetTexRect(Entity* ent);
void Entity_SyncOrigin(Entity* ent);
void Cyclone_OnUpdate(Entity* ent, Game* g, Uint64 dt);
bool Village_OnInteract(Entity* ent, Game* g);
const char* EntityType_ToString(EntityType type);
