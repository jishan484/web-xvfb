#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include "input.h"
#include "inputstr.h"
#include "mi_priv.h"
#include "xkbsrv.h"
#include <X11/keysym.h>
#include <X11/keysymdef.h>





/* Defined in mainvnc.h */
extern ScreenPtr g_screen;
extern int appRunning;
DeviceIntPtr kbd; 

void input_init(void);
void input_init(void) {
    kbd = inputInfo.keyboard;
    
}

void process_key_press(int keycode, int is_pressed);
void process_key_press(int keycode, int is_pressed) {
    if(!appRunning) return;
    KeySym sym = XStringToKeysym("a");
    
    QueueKeyboardEvents(kbd, is_pressed ? KeyPress : KeyRelease, keycode);
}


void process_client_Input(char *data, int clientSD);
void process_client_Input(char *data, int clientSD) {
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
            // int keycode = XKeysymToKeycode(this->display, XStringToKeysym(data + 2));
            // XTestFakeKeyEvent(this->display, keycode, True, CurrentTime);
            // XTestFakeKeyEvent(this->display, keycode, False, CurrentTime);
            // XFlush(this->display);
        }
        else if (data[1] == 50)
        {
            char buffer[15] = {0};
            int i = 2;
            while (data[i] != ' ')
            {
                buffer[i - 2] = data[i];
                i++;
            }
            i++;
            // int keycode1 = XKeysymToKeycode(this->display, XStringToKeysym(buffer));
            // int keycode2 = XKeysymToKeycode(this->display, XStringToKeysym(data + i));
            // XTestFakeKeyEvent(this->display, keycode1, True, 0);
            // XTestFakeKeyEvent(this->display, keycode2, True, 0);
            // XTestFakeKeyEvent(this->display, keycode2, False, 0);
            // XTestFakeKeyEvent(this->display, keycode1, False, 0);
            // XFlush(this->display);
        }
    }
}