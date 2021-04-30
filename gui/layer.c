#include <gui/layer.h>
#include <gui/vga.h>
#include <libc/string.h>
static BOOTINFO* bootInfo = (BOOTINFO*)BOOTINFO_ADDR; 

LayerManager* init_layman(MemMan* memman) {
    int i;
    LayerManager* layman = (LayerManager*)mem_alloc_4k(memman, sizeof(LayerManager));
    layman->numOfLayer = 0;
    layman->vram = (uint8_t*)bootInfo->vram;
    mem_alloc_4k(memman, 0x100000); // vmware's bug
    layman->map = (uint8_t*)mem_alloc_4k(memman, bootInfo->screen_h * bootInfo->screen_w);
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
        emptyLayer->layman = layman;
    }

    add_layer(layman, emptyLayer);
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

void del_layer(Layer* layer) {
    int i, j;
    LayerManager* layman = layer->layman;
    for (i = 0; i < layman->numOfLayer; i++) {
        if (layman->layerIndexList[i] == layer) break;
    }

    for (j = i; j < layman->numOfLayer - 1; j++) {
        layman->layerIndexList[j] = layman->layerIndexList[j + 1];
    }
    layman->numOfLayer--;
    repaint_partial_layers(layman, layer->x, layer->y, layer->x+layer->weight, layer->y+layer->height, 0);
}

void set_layer_level(Layer* layer, int z) {
    int i;
    int begin, end;
    int old_z = layer->z;
    LayerManager* layman = layer->layman;
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
void move_layer(Layer* layer, int x, int y) {
    LayerManager* layman = layer->layman;

    int old_x = layer->x;
    int old_y = layer->y;
    layer->x = x;
    layer->y = y;
    
    /* for the old position, repaint from the bottom */
    repaint_partial_layers(layman, old_x, old_y, old_x + layer->weight, old_y + layer->height, 0);
    
    /* for the current position, repaint from the current layer */
    repaint_partial_layers(layman, x, y, x + layer->weight, y + layer->height, layer->z);
}


/**
 * repaint all layers with entire area
 */
void repaint_layers(LayerManager* layman) {
    int i, x, y, lid, index;
    uint8_t c;
    Layer* layer;
    for (i = 0; i < layman->numOfLayer; i++) {
        layer = layman->layerIndexList[i];
        lid = i; // layer id
        for (y = 0; y < layer->height; y++) {
            for (x = 0; x < layer->weight; x++) {
                c = layer->buf[y*layer->weight+x];
                index = (y+layer->y)*bootInfo->screen_w + (x + layer->x);
                if (c != TRANSPARENT) {
                    layman->vram[index] = c;
                    layman->map[index] = lid;
                }
            }
        }
    }
    
}

void repaint_single_layer(Layer* l, int x0, int y0, int w, int h) {
    repaint_partial_layers(l->layman, l->x + x0, l->y + y0, 
                    l->x + x0 + w, l->y + y0 + h, l->z);
}

/**
 * repaint partial layer
 * (x0, y0) is upper-left coordinate, (x1, y1) is the lower-right coordinate
 */
void repaint_partial_layers(LayerManager* layman, int x0, int y0, int x1, int y1, int begin_z) {
    refresh_partial_map(layman, x0, y0, x1, y1);

    /* x, y is offset in buf, vx, vy if offset in video ram */
    int i, x, y, vx, vy, lid, index;
    int x_begin, x_end, y_begin, y_end;
    uint8_t c;
    if (x1 > bootInfo->screen_w) x1 = bootInfo->screen_w;
    if (y1 > bootInfo->screen_h) y1 = bootInfo->screen_h; //part of the area is not visible

    for (i = 0; i < layman->numOfLayer; i++) {
        Layer* layer = layman->layerIndexList[i];
        lid = i;
        if (layer->z < begin_z) continue;       

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
                index = vy * bootInfo->screen_w + vx;
 
 
                /* only repaint top level layer */
                if ((c != TRANSPARENT) && (layman->map[index] == lid)) {
                    layman->vram[index] = c;
                }
            }
        }
    }
}

void refresh_partial_map(LayerManager* layman, int x0, int y0, int x1, int y1) {
    /* x, y is offset in buf, vx, vy if offset in map */
    int i, x, y, vx, vy, lid, index;
    int x_begin, x_end, y_begin, y_end;
    uint8_t c;
    if (x1 > bootInfo->screen_w) x1 = bootInfo->screen_w;
    if (y1 > bootInfo->screen_h) y1 = bootInfo->screen_h; //part of the area is not visible

    for (i = 0; i < layman->numOfLayer; i++) {
        Layer* layer = layman->layerIndexList[i];
        lid = i;
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
                index = vy * bootInfo->screen_w + vx;
                /* only repaint top level layer */
                if (c != TRANSPARENT) {
                    layman->map[index] = lid;
                }
            }
        }
    }
}

void put_str_refresh(Layer* layer, int x0, int y0, char* str, int color) {
    put_string(layer->buf, layer->weight, x0, y0, str, color);
    repaint_single_layer(layer, x0, y0, strlen(str)*CHAR_W, CHAR_H);
}

void put_char_refresh(Layer* layer, int x0, int y0, char c, int color) {
    put_char(layer->buf, layer->weight, x0, y0, c, color);
    repaint_single_layer(layer, x0, y0, CHAR_W, CHAR_H);
}

void put_rect_refresh(Layer* layer, int x0, int y0, int weight, int height, int color) {
    fill_rect(layer->buf, layer->weight, x0, y0, weight, height, color);
    repaint_single_layer(layer, x0, y0, weight, height);
}