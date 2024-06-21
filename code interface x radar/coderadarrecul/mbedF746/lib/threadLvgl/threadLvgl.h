#ifndef __THREADLVGL_H
#define __THREADLVGL_H

#include "mbed.h"
#include "lvgl.h"

// Fonctionnalités expérimentales
//#define THREAD_LVGL_DOUBLE_BUFFERING
//#define THREAD_LVGL_FULL_REFRESH

class ThreadLvgl
{
protected:
    Mutex mutex;
#ifdef THREAD_LVGL_DOUBLE_BUFFERING
    static EventFlags m_flags;
    enum {
        FLAG_RELOAD = 1
    };
#endif
    void runLvgl();
    static void run(ThreadLvgl *p)
    {
        p->runLvgl();
    }
    Thread m_mainThread;
    void displayInit();
    static void refreshDisplay(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
    void touchpadInit();
    static void touchpadRead(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
    lv_indev_t *indevTouchpad;    
    static bool refreshEnabled;
    Ticker lvTicker;
    static int refreshTime;
    static void lvTimeCounter();
    void setRefreshEnable(bool enabled = true) { refreshEnabled = enabled; }
    static void wait(lv_disp_drv_t *disp_drv);
#ifdef THREAD_LVGL_DOUBLE_BUFFERING
    static void reloadEventCallback(LTDC_HandleTypeDef *hltdc);
#endif
public:
    ThreadLvgl(int refreshTimeInit = 30);
    ~ThreadLvgl();
    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }
};

#endif
