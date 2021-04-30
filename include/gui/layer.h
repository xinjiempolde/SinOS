#ifndef LAYER_H
#define LAYER_H

#include <mm/memory.h>
#include <cpu/type.h>

#define MAX_LAYER_NUM 256
#define LAYER_NO_USE  0
#define LAYER_IN_USE  1

typedef struct Layer Layer;
typedef struct LayerManager LayerManager;
struct Layer{
    uint8_t* buf;
    int x, y;            // x, y are upper-left coordinates in the plane
    int z;               // z represents which layer
    int weight, height;  // size
    bool flag;           // is layer in use or not; 
    LayerManager* layman; // for convenience, there is no need to pass 
                        // layermanager into function every time
};

struct LayerManager{
    int numOfLayer;
    uint8_t* vram;
    uint8_t* map;  // record the pixel belongs to which layer
    Layer* layerIndexList[MAX_LAYER_NUM]; // record which layers exists asc by layer.z
    Layer layerList[MAX_LAYER_NUM];
};

LayerManager* init_layman(MemMan* memman);
Layer* alloc_layer(LayerManager* layman, uint8_t* buf, int h, int w, int z);
void add_layer(LayerManager* layman, Layer* layer);
void set_layer_level(Layer* layer, int z);
void del_layer(Layer* layer);
void move_layer(Layer* layer, int x, int y);
void repaint_layers(LayerManager* layman);
void repaint_partial_layers(LayerManager* layman, int x0, int y0, int x1, int y1, int begin_z);
void repaint_single_layer(Layer* layer, int x0, int y0, int w, int h);
void refresh_partial_map(LayerManager* layman, int x0, int y0, int x1, int y1);

void put_str_refresh(Layer* layer, int x0, int y0, char* str, int color);
void put_char_refresh(Layer* layer, int x0, int y0, char c, int color);
void put_rect_refresh(Layer* layer, int x0, int y0, int weight, int height, int color);
#endif