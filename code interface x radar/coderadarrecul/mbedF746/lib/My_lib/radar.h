#ifndef RADAR_H
#define RADAR_H

#include <mbed.h>
#include <threadLvgl.h>

// Configuration I2C pour le capteur SRF08
extern I2C my_i2c;
extern const int SRF08_ADDRESS;

// Configuration du buzzer
extern PwmOut buzzer;

// Variables de contrôle
extern bool radar_active;

// Initialisation du thread LVGL
extern ThreadLvgl threadLvgl;


// Déclaration des fonctions
void create_ui();
void start_button_event_handler(lv_event_t * e);
void stop_button_event_handler(lv_event_t * e);
void update_progress_bar(int dist);
void sound_buzzer(int dist);
void update_distance_label(int dist); 
void buzzer_switch_event_handler(lv_event_t *e);
void slider_event_handler(lv_event_t *e);
#endif // RADAR_H
