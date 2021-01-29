#include <gui/layer.h>
#include <gui/vga.h>
LayerManager* init_layman(MemMan* memman) {
    int i;
    LayerManager* layman = (LayerManager*)mem_alloc_4k(memman, sizeof(LayerManager));
    layman->numOfLayer = 0;
    layman->vram = (uint8_t*)VIDEO_13H_ADDRESS;
    for (i = 0; i < MAX_LAYER_NUM; i++) {
        layman->layerList[i].flag = LAYER_NO_USE;
    }
    return layman;
}

Layer* alloc_layer(LayerManager* layman, uint8_t* buf, int h, int w, int z) {
    int i;
    Layer* emptyLayer = NULL;
    /* to find a empty(no use) layer */
    for (i = 0; i < MAX_LAYER_NUM; i++) {
        if (layman->layerList[i].flag == LAYER_NO_USE) break;
    }
    /* if found */
    if (i < MAX_LAYER_NUM) {
        emptyLayer = &(layman->layerList[i]);
        emptyLayer->buf = buf;
        emptyLayer->height = h;
        emptyLayer->weight = w;
        emptyLayer->x = 0;
        emptyLayer->y = 0;
        emptyLayer->z = z>MAX_LAYER_NUM?MAX_LAYER_NUM:z;
        emptyLayer->flag = LAYER_IN_USE;
    }
    return emptyLayer;
    
}

void add_layer(LayerManager* layman, Layer* layer) {
    int i, j;
    int z = layer->z;
    /* there is no space to add layer */
    if (layman->numOfLayer >= MAX_LAYER_NUM) {
        return;
    }

    /* find the insert point */
    for (i = 0; i < layman->numOfLayer; i++) {
        if (z < layman->layerIndexList[i]->z) break;
    }
    
    /* move forward one by one */
    for (j = layman->numOfLayer; j > i; j--) {
        layman->layerIndexList[j] = layman->layerIndexList[j - 1];
    }
    layman->layerIndexList[i] = layer; // add layer
    layman->numOfLayer++;
    repaint_layers(layman);
}

void set_layer_level(LayerManager* layman, Layer* layer, int z) {
    int i;
    int begin, end;
    int old_z = layer->z;
    z = z>MAX_LAYER_NUM?MAX_LAYER_NUM:z;
    /* increase the level of layer */
    if (z > old_z) {
        for (i = 0; i < layman->numOfLayer; i++) {
            if (layman->layerIndexList[i]->z == old_z) begin = i;
            if (layman->layerIndexList[i]->z >= z) {
                end = i;
                break;
            } 
        }
        for (i = begin; i < end; i++) {
            layman->layerIndexList[i] = layman->layerIndexList[i+1];
        }
        layman->layerIndexList[i] = layer;
    }

    /* decrease the level of layer */
    if (z < old_z) {
        for (i = 0; i < layman->numOfLayer; i++) {
            if (layman->layerIndexList[i]->z == old_z) {
                end = i;
                break;
            }
            if (layman->layerIndexList[i]->z <= z) begin = i;
        }
        for (i = end; i > begin; i--) {
            layman->layerIndexList[i] = layman->layerIndexList[i-1];
        }
        layman->layerIndexList[i] = layer;
    }
}

/**
 * move layer to (x, y), (x, y) is upper-left coordinate
 */
void move_layer(LayerManager* layman, Layer* layer, int x, int y) {
    int old_x = layer->x;
    int old_y = layer->y;
    layer->x = x;
    layer->y = y;
    //repaint_layers(layman);
    repaint_partial_layers(layman, old_x, old_y, old_x + layer->weight, old_y + layer->height);
    repaint_partial_layers(layman, x, y, x + layer->weight, y + layer->height);
}

/**
 * repaint all layers with entire area
 */
void repaint_layers(LayerManager* layman) {
    int i, x, y;
    uint8_t c;
    for (i = 0; i < layman->numOfLayer; i++) {
        Layer* layer = layman->layerIndexList[i];
        for (y = 0; y < layer->height; y++) {
            for (x = 0; x < layer->weight; x++) {
                c = layer->buf[y*layer->weight+x];
                if (c != TRANSPARENT) {
                    layman->vram[(y+layer->y)*SCREEN_W + (x + layer->x)] = c;
                }
            }
        }
    }
}


/**
 * repaint partial layer
 * (x0, y0) is upper-left coordinate, (x1, y1) is the lower-right coordinate
 */
void repaint_partial_layers(LayerManager* layman, int x0, int y0, int x1, int y1) {
    /* x, y is offset in buf, vx, vy if offset in video ram */
    int i, x, y, vx, vy;
    int x_begin, x_end, y_begin, y_end;
    uint8_t c;
    if (x1 > SCREEN_W) x1 = SCREEN_W;
    if (y1 > SCREEN_H) y1 = SCREEN_H; //part of the area is not visible

    for (i = 0; i < layman->numOfLayer; i++) {
        Layer* layer = layman->layerIndexList[i];
        
        /* partial of buf */
        y_begin = y0 - layer->y;
        y_end = y1 - layer->y;
        x_begin = x0 - layer->x;
        x_end = x1 - layer->x;

        if (y_begin < 0) y_begin = 0; // move down
        if (x_begin < 0) x_begin = 0; // move right
        if (y_end > layer->height) y_end = layer->height; // move up
        if (x_end > layer->weight) x_end = layer->weight; // move left

        for (y = y_begin; y < y_end; y++) {
            vy = y + layer->y;
            for (x = x_begin; x < x_end; x++) {
                vx = x + layer->x;
                c = layer->buf[y*layer->weight+x];
                if (c != TRANSPARENT) {
                    layman->vram[vy*SCREEN_W + vx] = c;
                }
            }
        }
    }
}