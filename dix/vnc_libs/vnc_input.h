
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "input.h"
#include "inputstr.h"
#include "xkbstr.h"
#include "xkbsrv.h"



extern int appRunning; // Defined in mainvnc.h //
int input_active = 0;
DeviceIntPtr kbd;

static int *keysym_table[256] = {0};

void add_mapping(long sym, int keycode);
void add_mapping(long sym, int keycode) {
    int hi = (sym >> 8) & 0xFF;
    int lo = sym & 0xFF;
    if (!keysym_table[hi]) {
        keysym_table[hi] = calloc(256, sizeof(int));
    }
    keysym_table[hi][lo] = keycode;
}

int lookup_keycode(KeySym sym);
int lookup_keycode(KeySym sym) {
    int hi = (sym >> 8) & 0xFF;
    int lo = sym & 0xFF;
    return keysym_table[hi] ? keysym_table[hi][lo] : 0;
}



void input_init(InputInfo * inputInfoPtr);
void input_init(InputInfo * inputInfoPtr) {
    kbd = inputInfoPtr->keyboard;
    input_active = 1;
    // setup keycode mapping for current master keyboard
    DeviceIntPtr dev;
    for (dev = inputInfo.devices; dev; dev = dev->next) {
        if (dev->type == MASTER_KEYBOARD) {
            XkbDescPtr xkb = inputInfo.keyboard->key->xkbInfo->desc;
            if (!xkb)
                return;
            int min = xkb->min_key_code;
            int max = xkb->max_key_code;

            for (int kc = min; kc <= max; ++kc) {
                XkbSymMapPtr map = &xkb->map->key_sym_map[kc];

                for (int level = 0; level < map->width; ++level) {
                    KeySym ks = xkb->map->syms[map->offset + level];
                    if (ks != NoSymbol) {
                        add_mapping(ks, kc);
                    }
                }
            }
        }
    }
}

void process_key_press(int keycode, int is_pressed);
void process_key_press(int keycode, int is_pressed) {
    if(!appRunning && !input_active) return;
    usleep(20000);
    QueueKeyboardEvents(kbd, is_pressed ? KeyPress : KeyRelease, keycode);
    ProcessInputEvents();
}


void process_client_Input(char *data, int clientSD);
void process_client_Input(char *data, int clientSD) {
    // printf("Got data from %d : %s\n", clientSD, data);
    if(!appRunning) return;
    int len = strlen(data);
    int x = 0, y = 0, i = 1, x2 = 0, y2 = 0;

    if (data[0] == 'C')
    {
        while (data[i] != 32 && i < len)
            x = x * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y = y * 10 + data[i++] - 48;
        //mouse click event at x,y pos
    }
    else if (data[0] == 'M')
    {
        while (data[i] != 32 && i < len)
            x = x * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y = y * 10 + data[i++] - 48;
        // mouse move to x,y pos
    }
    else if (data[0] == 'R')
    {
        while (data[i] != 32 && i < len)
            x = x * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y = y * 10 + data[i++] - 48;
        // mouse right click at pos x,y pos
    }
    else if (data[0] == 'D')
    {
        while (data[i] != 32 && i < len)
            x = x * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y = y * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            x2 = x2 * 10 + data[i++] - 48;
        i++;
        while (data[i] != 32 && i < len)
            y2 = y2 * 10 + data[i++] - 48;
        
        // mouse select/drag event from x1,y1 to x2,y2 pos
    }
    else if (data[0] == 'S')
    {
        if (data[1] == 'U')
        {
            // XTestFakeButtonEvent(this->display, 4, 1, 0);
            // XTestFakeButtonEvent(this->display, 4, 0, 70);
            // scroll up
        }
        else
        {
            // XTestFakeButtonEvent(this->display, 5, 1, 0);
            // XTestFakeButtonEvent(this->display, 5, 0, 70);
            // scroll down
        }
    }
    else if (data[0] == 'K')
    {
        if (data[1] == 49)
        {
            int sym = atol(data+2);
            int keycode = lookup_keycode(sym);
            process_key_press(keycode, 1);
            process_key_press(keycode, 0);
            // printf("data if cond %c %d which %d\n", data[1], data[1], keycode);
        }
        else if (data[1] == 50)
        {
            char buffer[15] = {0};
            int _i = 2;
            while (data[_i] != ' ')
            {
                buffer[_i - 2] = data[_i];
                _i++;
            }
            _i++;
            long sym1 = atol(buffer);
            long sym2 = atol(data+_i);
            int keycode1 = lookup_keycode(sym1);
            int keycode2 = lookup_keycode(sym2);
            process_key_press(keycode1, 1);
            process_key_press(keycode2, 1);
            process_key_press(keycode2, 0);
            process_key_press(keycode1, 0);
        }
    }
}