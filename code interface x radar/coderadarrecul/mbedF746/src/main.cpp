#include <mbed.h>
#include <threadLvgl.h> // Inclusion de la bibliothèque pour le thread LVGL
#include <cstdio>       // Inclusion pour les entrées/sorties formatées (stdio)
#include <radar.h>      // Inclusion de la bibliothèque pour le radar (non spécifiée dans le code)
#include <iostream>
#define UART7_RX PE_7
#define UART7_TX PE_8
#define MAXIMUM_BUFFER_SIZE                                                  32

// Configuration I2C pour le capteur SRF08
I2C my_i2c(D14, D15);           // Déclaration et initialisation d'un objet I2C avec les broches D14 et D15 pour SDA et SCL respectivement
const int SRF08_ADDRESS = 0xE0; // Adresse I2C du capteur SRF08

// config UART
BufferedSerial serial7(PF_7, PF_6); // RX, TX
// Configuration du buzzer
PwmOut buzzer(D10); // Déclaration et initialisation d'un objet PwmOut pour le buzzer avec la broche D10

// Variables de contrôle
bool radar_active = false; // Variable booléenne pour indiquer si le radar est actif ou non

// Initialisation du thread LVGL
ThreadLvgl threadLvgl(30); // Déclaration et initialisation d'un objet ThreadLvgl avec une priorité de 30

lv_obj_t *ecran1;
lv_obj_t *ecran2;
lv_obj_t *switch_btn2;
lv_obj_t *switch_btn1;
lv_obj_t *switch_page2;
lv_obj_t *switch_page1;

lv_obj_t *btnOn;
lv_obj_t *btnOff;
lv_obj_t *titre;
lv_obj_t *txtLumiere;
lv_obj_t *txtVitesse;
lv_obj_t *needle_line;
lv_obj_t *arc;
lv_style_t style;

int v = 50;
int vitesse_screen = 1;
int l = 0;
 char buf[128]; // Tampon pour stocker les données lues
int received_int=0;

void lumiere(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (btn == btnOn)
        {
            l = 1;
            lv_label_set_text(txtLumiere, "Lumiere ON");
        }
        else if (btn == btnOff)
        {
            l = 1;
            lv_label_set_text(txtLumiere, "Lumiere OFF");
        }
    }
}

void vitesse(void)
{
}

void VitesseLumiere()
{
    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_color_make(0, 0, 0));
    lv_style_set_bg_opa(&style, LV_OPA_COVER);

    titre = lv_label_create(ecran1);
    lv_label_set_text(titre, "Vitesse & Lumiere");
    lv_obj_align(titre, LV_ALIGN_TOP_MID, 0, 60); // Alignement au centre en haut

    txtLumiere = lv_label_create(ecran1);
    lv_label_set_text(txtLumiere, "Lumiere OFF");
    lv_obj_align(txtLumiere, LV_ALIGN_BOTTOM_RIGHT, -60, -160);

    txtVitesse = lv_label_create(ecran1);
    lv_label_set_text(txtVitesse, "0 km/h");
    lv_obj_align(txtVitesse, LV_ALIGN_CENTER, -100, 50);

    btnOn = lv_btn_create(ecran1);
    lv_obj_set_size(btnOn, 150, 50);
    lv_obj_align(btnOn, LV_ALIGN_BOTTOM_RIGHT, -20, -90);
    lv_obj_add_event_cb(btnOn, lumiere, LV_EVENT_ALL, NULL);
    lv_obj_t *label = lv_label_create(btnOn);
    lv_label_set_text(label, "Lumiere On");
    lv_obj_center(label);

    btnOff = lv_btn_create(ecran1);
    lv_obj_set_size(btnOff, 150, 50);
    lv_obj_align(btnOff, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_add_event_cb(btnOff, lumiere, LV_EVENT_ALL, NULL);
    lv_obj_t *label5 = lv_label_create(btnOff);
    lv_label_set_text(label5, "Lumiere Off");
    lv_obj_center(label5);

    arc = lv_arc_create(ecran1);
    lv_arc_set_start_angle(arc, 135);
    lv_arc_set_end_angle(arc, 45);
    lv_obj_set_size(arc, 200, 180);
    lv_obj_align(arc, LV_ALIGN_CENTER, -90, 40);
}

void update_vitesse(int vit)
{
    char txt[32];
    lv_arc_set_value(arc, vit);
    sprintf(txt, "%d km/h", vit);
    lv_label_set_text(txtVitesse, txt);
}

void event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {

        if ((btn == switch_btn1) || (btn == switch_page1))
        {
            vitesse_screen = 1;

            lv_obj_clean(ecran1);
            lv_obj_clean(lv_scr_act());

            // Bouton pour aller à l'écran 1
            switch_page1 = lv_btn_create(ecran1);
            lv_obj_align(switch_page1, LV_ALIGN_TOP_MID, -100, 10);
            lv_obj_add_event_cb(switch_page1, event_handler, LV_EVENT_ALL, NULL);
            lv_obj_t *label3 = lv_label_create(switch_page1);
            lv_label_set_text(label3, "Vitesse & Lumiere");

            // Bouton pour aller à l'écran 2
            switch_page2 = lv_btn_create(ecran1);
            lv_obj_align(switch_page2, LV_ALIGN_TOP_MID, 100, 10);
            lv_obj_add_event_cb(switch_page2, event_handler, LV_EVENT_ALL, NULL);
            lv_obj_t *label4 = lv_label_create(switch_page2);
            lv_label_set_text(label4, "Radar de recule");

            lv_scr_load(ecran1);
            VitesseLumiere();
        }
        else if ((btn == switch_btn2) || (btn == switch_page2))
        {
            vitesse_screen = 0;
            lv_obj_clean(ecran1);
            lv_obj_clean(lv_scr_act());

            // lv_obj_clean(ecran2);
            switch_btn1 = lv_btn_create(lv_scr_act());
            lv_obj_align(switch_btn1, LV_ALIGN_TOP_MID, -100, 10);
            lv_obj_add_event_cb(switch_btn1, event_handler, LV_EVENT_ALL, NULL);
            lv_obj_t *label1 = lv_label_create(switch_btn1);
            lv_label_set_text(label1, "Vitesse & Lumiere");

            // Bouton pour aller à l'écran 2
            switch_btn2 = lv_btn_create(lv_scr_act());
            lv_obj_align(switch_btn2, LV_ALIGN_TOP_MID, 100, 10);
            lv_obj_add_event_cb(switch_btn2, event_handler, LV_EVENT_ALL, NULL);
            lv_obj_t *label2 = lv_label_create(switch_btn2);
            lv_label_set_text(label2, "Radar de recule");

            lv_scr_load(lv_scr_act());
            create_ui();
        }
    }
}

int main()
{

    serial7.set_baud(115200);
    // Initialisation LVGL et création de l'interface utilisateur
    serial7.set_format(8, BufferedSerial::None, 1);
    threadLvgl.lock(); // Verrouillage du thread LVGL

    ecran1 = lv_obj_create(NULL);
    ecran2 = lv_obj_create(NULL);

    lv_scr_load(ecran1);
    VitesseLumiere();

    // Bouton pour aller à l'écran 1
    switch_btn1 = lv_btn_create(lv_scr_act());
    lv_obj_align(switch_btn1, LV_ALIGN_TOP_MID, -100, 10);
    lv_obj_add_event_cb(switch_btn1, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_t *label1 = lv_label_create(switch_btn1);
    lv_label_set_text(label1, "Vitesse & Lumiere");

    // Bouton pour aller à l'écran 2
    switch_btn2 = lv_btn_create(lv_scr_act());
    lv_obj_align(switch_btn2, LV_ALIGN_TOP_MID, 100, 10);
    lv_obj_add_event_cb(switch_btn2, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_t *label2 = lv_label_create(switch_btn2);
    lv_label_set_text(label2, "Radar de recule");

    // Bouton pour aller à l'écran 1
    switch_page1 = lv_btn_create(ecran1);
    lv_obj_align(switch_page1, LV_ALIGN_TOP_MID, -100, 10);
    lv_obj_add_event_cb(switch_page1, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_t *label3 = lv_label_create(switch_page1);
    lv_label_set_text(label3, "Vitesse & Lumiere");

    // Bouton pour aller à l'écran 2
    switch_page2 = lv_btn_create(ecran1);
    lv_obj_align(switch_page2, LV_ALIGN_TOP_MID, 100, 10);
    lv_obj_add_event_cb(switch_page2, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_t *label4 = lv_label_create(switch_page2);
    lv_label_set_text(label4, "Radar de recule");

    // create_ui();         // Appel d'une fonction pour créer l'interface utilisateur
    threadLvgl.unlock(); // Déverrouillage du thread LVGL

    // Configuration I2C
    char init_tab[2] = {0x00, 0x51}; // Tableau d'initialisation pour le capteur SRF08
    char data[2];                    // Tableau pour stocker les données lues depuis le capteur
    char reg_data_dist[1] = {0x02};  // Registre de données de distance pour le capteur SRF08
    int dist;                        // Variable pour stocker la distance mesurée
    my_i2c.frequency(100000);        // Définition de la fréquence de communication I2C à 100 kHz

    while (true) // Boucle principale du programme
    {
        uint num_bytes = serial7.read(buf, sizeof(buf)); // Lecture des données depuis Serial7
        if (num_bytes > 0)
        {
            received_int=(int)buf;
            printf("Failed to convert to integer from: %d\n", buf);

           
        
        
        if (radar_active && !vitesse_screen) // Vérification si le radar est actif
        {
            my_i2c.write(SRF08_ADDRESS, init_tab, 2);      // Envoi des données d'initialisation au capteur SRF08
            ThisThread::sleep_for(100ms);                  // Attente de 100 millisecondes
            my_i2c.write(SRF08_ADDRESS, reg_data_dist, 1); // Envoi de la demande de lecture de la distance au capteur
            my_i2c.read(SRF08_ADDRESS, data, 2);           // Lecture des données de distance depuis le capteur
            dist = (data[0] << 8) + data[1];               // Calcul de la distance à partir des données lues
            printf("Distance: %d cm\n", dist);             // Affichage de la distance mesurée sur la console
            sound_buzzer(dist);                            // Appel d'une fonction pour activer le buzzer en fonction de la distance
            threadLvgl.lock();                             // Verrouillage du thread LVGL
            update_progress_bar(dist);                     // Appel d'une fonction pour mettre à jour la barre de progression (non définie dans le code)
            update_distance_label(dist);                   // Appel d'une fonction pour mettre à jour l'étiquette de distance (non définie dans le code)
            threadLvgl.unlock();                           // Déverrouillage du thread LVGL
        }
        else if (vitesse_screen)
        {
            lv_label_set_text(txtVitesse, "ok");
            threadLvgl.lock();
            update_vitesse(received_int);
            threadLvgl.unlock();
        }

        ThisThread::sleep_for(200ms); // Attente de 200 millisecondes avant de continuer la boucle
    }
}
}
