#include "dix.h"
#include "damage.h"
#include <pthread.h>
#include <stdio.h>
#include "damagestr.h"
#include "scrnintstr.h"
#include "vnc_libs/vncqueue.h"
#include "vnc_libs/websocket.h"

int httpPort = 0;
DamageQueue * gdq;
int appRunning = 0;
Websocket *g_ws = NULL;
int isServerRunning = 0;
static DamagePtr myDamage = NULL;

void VNC_loop(void);
void VNC_close(void);
void VNC_log(const char *msg);
void* ws_thread_func(void* arg);
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
    DamageQueue dq;
    gdq = &dq;
    dq_init(gdq, screen->width, screen->height, 100);
    appRunning = 1;
    isServerRunning = 1;
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
    isServerRunning = 0;
    appRunning = 0;
    free(g_ws);
    g_ws = NULL;
}

void* ws_thread_func(void* arg) {
    Websocket* ws = (Websocket*)arg;
    printf(" few details %d %d %d\n", ws->clients, ws->server_fd, ws->socketPort);
    ws_connections(ws); // Run your poll/select loop
    return NULL;
}


void VNC_loop(void) {
    g_ws = malloc(sizeof(Websocket));
    if (!g_ws) { perror("malloc"); return; }

    ws_init(g_ws);
    ws_begin(g_ws, 8000);

    pthread_t ws_thread;
    if (pthread_create(&ws_thread, NULL, ws_thread_func, g_ws) != 0) {
        perror("Failed to create websocket thread");
        free(g_ws);
        g_ws = NULL;
        return;
    }
    pthread_detach(ws_thread);
}
