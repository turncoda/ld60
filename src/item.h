#pragma once

#define ITEM_TYPES(X) \
  X(salmon)           \
  X(cod)              \
  X(tuna)             \
  X(snapper)          \
  X(sturgeon)         \
  X(artifact)

typedef enum {
#define X(NAME) ItemType_##NAME,
  ITEM_TYPES(X)
#undef X
      ItemType_Count,
} ItemType;

const char* ItemType_ToString(ItemType type);
