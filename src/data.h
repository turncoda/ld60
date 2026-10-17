#pragma once

// Units: tiles
constexpr int kChunkWidth = 16;

// Units: tiles
constexpr int kChunkHeight = 16;

// Units: pixels
constexpr int kTileWidth = 8;

// Units: pixels
constexpr int kTileHeight = 8;

typedef int Tile;
typedef Tile Chunk[kChunkWidth*kChunkHeight];
