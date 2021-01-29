#ifndef LAYER_H
#define LAYER_H

#include <mm/memory.h>
#include <cpu/type.h>

#define MAX_LAYER_NUM 256
#define LAYER_NO_USE  0
#define LAYER_IN_USE  1
typedef struct {
    uint8_t* buf;
    int x, y;            // x, y are upper-left coordinates in the plane
    int z;               // z represents which layer
    int weight, height;  // size
    bool flag;           // is layer in use or not; 
}Layer;

typedef struct {
    int numOfLayer;
    uint8_t* vram;
    Layer* layerIndexList[MAX_LAYER_NUM]; // record which layers exists asc by layer.z
    Layer layerList[MAX_LAYER_NUM];
}LayerManager;

LayerManager* init_layman(MemMan* memman);
Layer* alloc_layer(LayerManager* layman, uint8_t* buf, int h, int w, int z);
void add_layer(LayerManager* layman, Layer* layer);
void set_layer_level(LayerManager* layman, Layer* layer, int z);
void move_layer(LayerManager* layman, Layer* layer, int x, int y);
void repaint_layers(LayerManager* layman);
void repaint_partial_layers(LayerManager* layman, int x0, int y0, int x1, int y1);
#endif