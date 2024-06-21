#include "threadLvgl.h"
#include "Drivers/BSP/STM32746G-Discovery/stm32746g_discovery_lcd.h"
#include "Drivers/BSP/STM32746G-Discovery/stm32746g_discovery_ts.h"
#include "misc/lv_color.h"
#include "draw/stm32_dma2d/lv_gpu_stm32_dma2d.h"

#ifdef THREAD_LVGL_DOUBLE_BUFFERING
EventFlags ThreadLvgl::m_flags;
#endif

bool ThreadLvgl::refreshEnabled = true;
int ThreadLvgl::refreshTime = 50;

ThreadLvgl::ThreadLvgl(int refreshTimeInit)
{
    lv_init();
    displayInit();
    touchpadInit();
    refreshTime = refreshTimeInit;
    lvTicker.attach(lvTimeCounter, chrono::milliseconds(refreshTime));
    m_mainThread.start(callback(ThreadLvgl::run, this));
}

ThreadLvgl::~ThreadLvgl()
{
    m_mainThread.join();
}

void ThreadLvgl::lvTimeCounter()
{
    lv_tick_inc(refreshTime);
}

void ThreadLvgl::runLvgl()
{
    while (1) {
        //lv_tick_inc(5); 
        //Call lv_tick_inc(x) every x milliseconds in a Timer or Task (x should be between 1 and 10). 
        //It is required for the internal timing of LittlevGL.
        mutex.lock();
        lv_task_handler();
        mutex.unlock(); 
        //Call lv_task_handler() periodically every few milliseconds. 
        //It will redraw the screen if required, handle input devices etc.
        ThisThread::sleep_for(chrono::milliseconds(refreshTime));
    }
}

void ThreadLvgl::displayInit(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    /*You code here*/
    // Init the touch screen display via the BSP driver. Based on ST's example.
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
    BSP_LCD_DisplayOn();
    BSP_LCD_SelectLayer(0);
    
#ifdef THREAD_LVGL_DOUBLE_BUFFERING
    BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS+0x00200000);
    HAL_LTDC_RegisterCallback(BSP_LCD_GetHandler(), HAL_LTDC_RELOAD_EVENT_CB_ID, reloadEventCallback);
#endif
    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     */

#ifndef THREAD_LVGL_DOUBLE_BUFFERING      
     /* 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     */
    lv_color_t *buf0 = (lv_color_t *)(LCD_FB_START_ADDRESS+0x00200000); /*A screen sized buffer*/
    lv_color_t *buf1 = (lv_color_t *)(LCD_FB_START_ADDRESS+0x00400000); /*Another screen sized buffer*/
#else
     /* 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */
    lv_color_t *buf0 = (lv_color_t *)(LCD_FB_START_ADDRESS); /*A screen sized buffer*/
    lv_color_t *buf1 = (lv_color_t *)(LCD_FB_START_ADDRESS+0x00200000); /*Another screen sized buffer*/
#endif
    static lv_disp_draw_buf_t draw_buf_dsc;
    lv_disp_draw_buf_init(&draw_buf_dsc, buf0, buf1,
                          BSP_LCD_GetXSize() * BSP_LCD_GetYSize()); /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);   /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = BSP_LCD_GetXSize();
    disp_drv.ver_res = BSP_LCD_GetYSize();

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = refreshDisplay;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc;

#ifdef THREAD_LVGL_DOUBLE_BUFFERING
#ifdef THREAD_LVGL_FULL_REFRESH
    /*Waiting for DMA2D transfer to finish*/
    disp_drv.wait_cb = ThreadLvgl::wait;
    disp_drv.full_refresh = 1;
#endif
    disp_drv.direct_mode = 1;
#endif
    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
void ThreadLvgl::refreshDisplay(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    if (refreshEnabled)
    {
#ifndef THREAD_LVGL_DOUBLE_BUFFERING
#if LV_USE_GPU_STM32_DMA2D
        lv_color_t *fbp32 = (lv_color_t *)LCD_FB_START_ADDRESS;
        lv_draw_stm32_dma2d_buffer_copy(NULL, fbp32 + area->y1 * LV_HOR_RES + area->x1,
        	(lv_coord_t) LV_HOR_RES, area, color_p, area->x2 - area->x1 + 1, area);
        lv_gpu_stm32_dma2d_wait_cb(disp_drv->draw_ctx);
#else
        lv_color_t *p = color_p;
        for (lv_coord_t y = area->y1; y <= area->y2; y++) {
            for (lv_coord_t x = area->x1; x <= area->x2; x++) {
                BSP_LCD_DrawPixel(x, y, p->full);
                p++;
            }
        }
#endif
#else
        uint32_t src = LCD_FB_START_ADDRESS;
        uint32_t dst = LCD_FB_START_ADDRESS;
        if (color_p == (lv_color_t *)src) {
            BSP_LCD_SetLayerVisible_NoReload(0, ENABLE);
            BSP_LCD_SetLayerVisible_NoReload(1, DISABLE);
            BSP_LCD_SelectLayer(1);
            src += 0x00200000;
        } else {
            BSP_LCD_SetLayerVisible_NoReload(1, ENABLE);
            BSP_LCD_SetLayerVisible_NoReload(0, DISABLE);
            BSP_LCD_SelectLayer(0);
            dst += 0x00200000;
        }
        BSP_LCD_Reload(LCD_RELOAD_VERTICAL_BLANKING);
        m_flags.wait_all(FLAG_RELOAD);
#ifndef THREAD_LVGL_FULL_REFRESH        
        lv_draw_stm32_dma2d_buffer_copy(disp_drv->draw_ctx, (lv_color_t *)src,
        	(lv_coord_t)BSP_LCD_GetXSize(), area, (lv_color_t *)dst, (lv_coord_t)BSP_LCD_GetXSize(), area);
        lv_gpu_stm32_dma2d_wait_cb(disp_drv->draw_ctx);
#endif        
#endif
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

/*------------------
 * Touchpad
 * -----------------*/

/*Will be called by the library to read the touchpad*/
void ThreadLvgl::touchpadRead(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    /*Save the pressed coordinates and the state*/
    TS_StateTypeDef TS_State; 
    
    BSP_TS_GetState(&TS_State);
    if(TS_State.touchDetected) {
        last_x = TS_State.touchX[0];
        last_y = TS_State.touchY[0];
        data->state = LV_INDEV_STATE_PR;
    }
    else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Set the last pressed coordinates*/
    data->point.x = last_x;
    data->point.y = last_y;
}

void ThreadLvgl::touchpadInit(void){
    static lv_indev_drv_t indev_drv;

    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/
    BSP_TS_Init(480, 272);

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpadRead;
    indevTouchpad = lv_indev_drv_register(&indev_drv);
}

/*Wait completion DMA2D operation
*/
void ThreadLvgl::wait(lv_disp_drv_t *disp_drv)
{
    ThisThread::sleep_for(1ms);
}

#ifdef THREAD_LVGL_DOUBLE_BUFFERING
/**
  * @brief  Reload Event callback.
  * @param  hltdc  pointer to a LTDC_HandleTypeDef structure that contains
  *                the configuration information for the LTDC.
  * @retval None
  */
void ThreadLvgl::reloadEventCallback(LTDC_HandleTypeDef *hltdc)
{
  /* Prevent unused argument(s) compilation warning */
    UNUSED(hltdc);
    m_flags.set(FLAG_RELOAD);
}
#endif
