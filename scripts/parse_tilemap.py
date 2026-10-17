import json
import sys

# Units: tiles
CHUNK_WIDTH = 16
CHUNK_HEIGHT = 16

json_file = sys.argv[1]
json_data = None
with open(json_file) as f:
    json_data = json.load(f)

tilemap_layer = None
for layer in json_data['layers']:
    if 'Tilemap' in layer['name']:
        tilemap_layer = layer

tilemap_data = tilemap_layer['cels'][0]['tilemap']
w = tilemap_data['width']
h = tilemap_data['height']
d = tilemap_data['tiles']
print('width is {} tiles'.format(w))
print('height is {} tiles'.format(h))

def get_chunk(cx, cy):
    chunks = []
    for y in range(cy, cy + CHUNK_HEIGHT):
        line = []
        for x in range(cx, cx + CHUNK_WIDTH):
            i = y * w + x
            line.append(d[i])
        chunks.append(line)
    return chunks

chunks = []
world_width = w // CHUNK_WIDTH
world_height = h // CHUNK_HEIGHT
for cy in range(0, h, CHUNK_HEIGHT):
    for cx in range(0, w, CHUNK_WIDTH):
        chunks.append(get_chunk(cx, cy))

print('made {} chunks'.format(len(chunks)))

with open(sys.argv[2], 'w') as f:
    f.write('#pragma once\n')
    f.write('/* GENERATED */\n')
    f.write('#include <data.h>\n')
    f.write('constexpr int kWorldWidth = {};\n'.format(world_width))
    f.write('constexpr int kWorldHeight = {};\n'.format(world_height))
    f.write('static const Chunk kOverworld[] = {\n')
    for chunk in chunks:
        f.write('  {\n')
        for line in chunk:
            f.write('    {}\n'.format(', '.join(map(str, line)) + ','))
        f.write('  },\n')
    f.write('};\n')
