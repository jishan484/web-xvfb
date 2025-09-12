#ifndef VNC_MAIN_H
#define VNC_MAIN_H


#include "vnc_libs/img.h"
#include "pixmap.h"   
#include "damage.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "damagestr.h"
#include "screenint.h"
#include "scrnintstr.h"
#include "vnc_libs/vncqueue.h"
#include "vnc_libs/websocket.h"

#define getDelay(fps) (1000000 / (fps))

int httpPort = 0;
char config[128]; // make sure this buffer is large enough]
DamageQueue * gdq;
int appRunning = 0;
ScreenPtr g_screen;
int currentScreen = 0;
Websocket * g_ws = NULL;
int isServerRunning = 0;
static DamagePtr myDamage = NULL;
unsigned char * buffer;

void VNC_loop(void);
void VNC_close(void);
void ws_onconnect(int sid);
void VNC_log(const char *msg);
void* ws_thread_func(void* arg);
void* vnc_thread_func(void* arg);
void sendFrame(ScreenPtr screen);
void initMyDamage(ScreenPtr pScreen);
void VNC_service_init(ScreenPtr screen);
void VNC_log_appended_int(const char * msg,int val);
static void DamageExtDestroy(DamagePtr pDamage, void *closure){}
void myDamageReport(DamagePtr pDamage, RegionPtr pRegion, void *closure);


void VNC_service_init(ScreenPtr screen) {
    VNC_log("XwebVNC service started");
    VNC_log("XwebVNC server started: success stage 1");    
    VNC_log_appended_int("[XwebVNC] > INF: XwebVNC server listening on port %d\n",httpPort);
    VNC_log("> (help: use -web or -http to change, e.g. -web 8000 or -http 8000)"); 
    initMyDamage(screen);
    gdq = malloc(sizeof(DamageQueue));
    dq_init(gdq, screen->width, screen->height, 100);
    appRunning = 1;
    isServerRunning = 1;
    g_screen = screen;
    snprintf(config, sizeof(config), "{'screen':%d,'width':%d,'height':%d}", screen->myNum, screen->width, screen->height);
    VNC_log(config);
    buffer = (unsigned char *)malloc(screen->width * screen->height * 30 * sizeof(unsigned char));
    VNC_loop();
    VNC_log("XwebVNC server started: success stage 2");
}


void myDamageReport(DamagePtr pDamage, RegionPtr pRegion, void *closure) {
    int nboxes;
    pixman_box16_t *boxes = pixman_region_rectangles(pRegion, &nboxes);
    for (int i = 0; i < nboxes; i++) {
        Rect r;
        r.x1 = boxes[i].x1;
        r.y1 = boxes[i].y1;
        r.x2 = boxes[i].x2;
        r.y2 = boxes[i].y2;
        dq_push(gdq, r);
    }
}

void initMyDamage(ScreenPtr pScreen) {
    myDamage = DamageCreate(myDamageReport,
                            DamageExtDestroy,
                            DamageReportRawRegion,
                            TRUE,
                            pScreen,
                            NULL);
    if (!myDamage) {
        printf("Failed to create Damage object\n");
        return;
    }
    // Attach to root window
    DamageRegister((DrawablePtr)pScreen->root, myDamage);
    VNC_log_appended_int("[XwebVNC] > LOG: Damage listener attached to screen %d root window\n", pScreen->myNum);
}


void VNC_log(const char *msg) {
    printf("[XwebVNC] > LOG: %s\n", msg);
}
 
void VNC_log_appended_int(const char * msg,int val) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-nonliteral"
    printf(msg, val);
    #pragma GCC diagnostic pop
}


void VNC_close(void) {
    VNC_log("Session on-close activity: starting");
    dq_destroy(gdq);
    DamageDestroy(myDamage);
    free(gdq);
    isServerRunning = 0;
    appRunning = 0;
    g_ws->stop = 1;
    VNC_log("Session closed: success");
    sleep(2);
    if(g_ws != NULL) {
        free(g_ws);
        g_ws = NULL;
    }
}

void* ws_thread_func(void* arg) {
    Websocket* ws = (Websocket*)arg;
    ws_connections(ws); // Run your poll/select loop
    printf("done\n");
    return NULL;
}

void* vnc_thread_func(void* arg) {
    int delay = getDelay(15);
    while(appRunning) {
        sendFrame(g_screen);
        usleep(delay);
    }
    return NULL;
}

void VNC_loop(void) {
    g_ws = malloc(sizeof(Websocket));
    if (!g_ws) { perror("malloc"); return; }

    ws_init(g_ws);
    ws_begin(g_ws, httpPort);
    g_ws->callBack = ws_onconnect;
    printf("ws setup done\n");

    pthread_t ws_thread;
    if (pthread_create(&ws_thread, NULL, ws_thread_func, g_ws) != 0) {
        perror("Failed to create websocket thread");
        return;
    }

    pthread_t vnc_thread;
    if (pthread_create(&vnc_thread, NULL, vnc_thread_func, 0) != 0) {
        perror("Failed to create VMC thread");
        return;
    }
}

void sendFrame(ScreenPtr screen) {
   dq_merge(gdq);
   int i = 10;
   while(dq_hasNext(gdq) && i--){
        Rect r;
        if(!dq_get(gdq, &r)) return;
        if(g_ws->clients < 1) continue;
        extractRectRGB(screen, r.x1, r.y1, r.x2 - r.x1, r.y2 - r.y1, buffer);
        int img_size = 0;
        char *jpeg_data = (char* )compress_image_to_jpeg(buffer, r.x2 - r.x1, r.y2 - r.y1, &img_size, 20);
        char data[256];  // adjust size if needed
        snprintf(
            data, sizeof(data),
            "VPD%d %d %d %d %d %d \n", r.x1, r.y1, r.x2 - r.x1, r.y2 - r.y1, 24, img_size
        );
        ws_p_sendRaw(g_ws, 130, data, jpeg_data, strlen(data), img_size, -1);
        free(jpeg_data);
   }
}

void ws_onconnect(int sid) {
    VNC_log_appended_int("[XwebVNC] > INF: New Websocket client connection established! sid: %d\n", sid);
    ws_sendText(g_ws, config, sid);
}

#endif