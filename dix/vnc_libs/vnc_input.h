#include <stdlib.h>
#include "input.h"
#include "mi_priv.h"
#include "resource.h"
#include "xkbsrv.h"






typedef struct {
    const char *name;
    int keycode;
} KeyMapEntry;

/* Default Xvfb/evdev keycodes for common keys */
static KeyMapEntry keymap[] = {
    /* letters */
    {"a", 38}, {"b", 56}, {"c", 54}, {"d", 40}, {"e", 26},
    {"f", 41}, {"g", 42}, {"h", 43}, {"i", 31}, {"j", 44},
    {"k", 45}, {"l", 46}, {"m", 58}, {"n", 57}, {"o", 32},
    {"p", 33}, {"q", 24}, {"r", 27}, {"s", 39}, {"t", 28},
    {"u", 30}, {"v", 55}, {"w", 25}, {"x", 53}, {"y", 29}, {"z", 52},

    /* numbers */
    {"1", 10}, {"2", 11}, {"3", 12}, {"4", 13}, {"5", 14},
    {"6", 15}, {"7", 16}, {"8", 17}, {"9", 18}, {"0", 19},

    /* common symbols */
    {"space", 65}, {"Enter", 36}, {"Tab", 23}, {"BackSpace", 22},
    {"Escape", 9}, {"Caps_Lock", 66},
    {"Shift_L", 50}, {"Shift_R", 62}, {"Control_L", 37}, {"Control_R", 105},
    {"Alt_L", 64}, {"Alt_R", 108},
    {"Left", 113}, {"Right", 114}, {"Up", 111}, {"Down", 116},

    /* function keys */
    {"F1", 67}, {"F2", 68}, {"F3", 69}, {"F4", 70}, {"F5", 71},
    {"F6", 72}, {"F7", 73}, {"F8", 74}, {"F9", 75}, {"F10", 76},
    {"F11", 95}, {"F12", 96},
};

/* lookup function */
int get_keycode(const char *name)
{
    if (!name) return 0;

    /* if single lowercase letter, normalize */
    if (strlen(name) == 1 && isalpha(name[0])) {
        char c = tolower(name[0]);
        for (int i = 0; i < sizeof(keymap)/sizeof(KeyMapEntry); i++) {
            if (keymap[i].name[0] == c) return keymap[i].keycode;
        }
        return 0;
    }

    /* lookup special keys */
    for (int i = 0; i < sizeof(keymap)/sizeof(KeyMapEntry); i++) {
        if (strcmp(name, keymap[i].name) == 0) return keymap[i].keycode;
    }

    return 0;
}


/* Defined in mainvnc.h */
extern int appRunning;
int input_active = 0;
DeviceIntPtr kbd; 

void input_init(void);
int lookup_keycode(const char *name);


void input_init(void) {
    kbd = inputInfo.keyboard;
    input_active = 1;
}

void process_key_press(int keycode, int is_pressed);
void process_key_press(int keycode, int is_pressed) {
    if(!appRunning && !input_active) return;
    printf("got it\n");
    QueueKeyboardEvents(kbd, is_pressed ? KeyPress : KeyRelease, keycode);
    ProcessInputEvents();
}


void process_client_Input(char *data, int clientSD);
void process_client_Input(char *data, int clientSD) {
    printf("Got data from %d : %s\n", clientSD, data);
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
            int keycode = get_keycode(data + 2);
            process_key_press(keycode, 1);
            process_key_press(keycode, 0);
            printf("data if cond %c %d which %d\n", data[1], data[1], keycode);
        }
        else if (data[1] == 50)
        {
            char buffer[15] = {0};
            int _i = 2;
            while (data[_i] != ' ')
            {
                buffer[_i - 2] = data[i];
                _i++;
            }
            _i++;
            int keycode1 = lookup_keycode(buffer);
            int keycode2 = lookup_keycode(data + _i);
            process_key_press(keycode1, 1);
            process_key_press(keycode2, 1);
            process_key_press(keycode2, 0);
            process_key_press(keycode1, 0);
        }
    }
}


#include <string.h>

int lookup_keycode(const char *name) {
    if (!name) return 0;

    /* Single characters (letters, digits, symbols) */
    if (strlen(name) == 1) {
        char c = name[0];
        if (c >= 'a' && c <= 'z')
            return 38 + (c - 'a');   /* XK_a → keycode 38 */
        if (c >= 'A' && c <= 'Z')
            return 38 + (c - 'A');   /* uppercase same as lowercase */
        if (c >= '0' && c <= '9') {
            static const int numcodes[10] = {19,10,11,12,13,14,15,16,17,18};
            return numcodes[c - '0'];
        }
        /* common symbols */
        switch (c) {
            case ' ': return 65;  /* space */
            case '\n': return 36; /* Enter */
            case '\t': return 23; /* Tab */
            case '-': return 20;
            case '=': return 21;
            case '[': return 34;
            case ']': return 35;
            case ';': return 47;
            case '\'': return 48;
            case ',': return 59;
            case '.': return 60;
            case '/': return 61;
            case '\\': return 51;
            default: return 0;
        }
    }

    /* Named keys */
    if (strcmp(name, "Return") == 0 || strcmp(name, "Enter") == 0) return 36;
    if (strcmp(name, "Escape") == 0) return 9;
    if (strcmp(name, "BackSpace") == 0) return 22;
    if (strcmp(name, "Tab") == 0) return 23;
    if (strcmp(name, "Caps_Lock") == 0) return 66;

    if (strcmp(name, "Shift_L") == 0) return 50;
    if (strcmp(name, "Shift_R") == 0) return 62;
    if (strcmp(name, "Control_L") == 0) return 37;
    if (strcmp(name, "Control_R") == 0) return 105;
    if (strcmp(name, "Alt_L") == 0) return 64;
    if (strcmp(name, "Alt_R") == 0) return 108;
    if (strcmp(name, "Meta_L") == 0 || strcmp(name, "Super_L") == 0) return 133;
    if (strcmp(name, "Meta_R") == 0 || strcmp(name, "Super_R") == 0) return 134;

    if (strcmp(name, "Left") == 0) return 113;
    if (strcmp(name, "Right") == 0) return 114;
    if (strcmp(name, "Up") == 0) return 111;
    if (strcmp(name, "Down") == 0) return 116;

    if (strcmp(name, "Home") == 0) return 110;
    if (strcmp(name, "End") == 0) return 115;
    if (strcmp(name, "Page_Up") == 0 || strcmp(name, "Prior") == 0) return 112;
    if (strcmp(name, "Page_Down") == 0 || strcmp(name, "Next") == 0) return 117;
    if (strcmp(name, "Insert") == 0) return 118;
    if (strcmp(name, "Delete") == 0) return 119;

    /* Function keys */
    if (strcmp(name, "F1") == 0) return 67;
    if (strcmp(name, "F2") == 0) return 68;
    if (strcmp(name, "F3") == 0) return 69;
    if (strcmp(name, "F4") == 0) return 70;
    if (strcmp(name, "F5") == 0) return 71;
    if (strcmp(name, "F6") == 0) return 72;
    if (strcmp(name, "F7") == 0) return 73;
    if (strcmp(name, "F8") == 0) return 74;
    if (strcmp(name, "F9") == 0) return 75;
    if (strcmp(name, "F10") == 0) return 76;
    if (strcmp(name, "F11") == 0) return 95;
    if (strcmp(name, "F12") == 0) return 96;

    return 0; /* not found */
}
