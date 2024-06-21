// Inclusion des bibliothèques nécessaires
#include "radar.h" // Bibliothèque spécifique au radar
#include <mbed.h>  // Bibliothèque pour le microcontrôleur mbed
#include <cstdio>  // Bibliothèque standard pour les entrées/sorties

// Déclaration des objets de l'interface graphique
lv_obj_t *title;          // Objet pour le titre
lv_obj_t *progress_bar;   // Objet pour la barre de progression
lv_obj_t *dist_label;     // Objet pour afficher la distance
lv_obj_t *btn_cont;       // Conteneur pour les boutons
lv_obj_t *start_btn;      // Bouton Start
lv_obj_t *start_label;    // Label pour le bouton Start
lv_obj_t *stop_btn;       // Bouton Stop
lv_obj_t *stop_label;     // Label pour le bouton Stop
lv_obj_t *buzzer_switch;  // Interrupteur pour le buzzer
lv_obj_t *buzzer_label;   // Label pour l'interrupteur du buzzer
lv_obj_t *slider;         // Slider pour ajuster les distances d'activation
lv_obj_t *adjust_label;   // Nouveau label pour afficher l'ajustement

// Déclaration des styles pour la barre de progression
lv_style_t style_red;          // Style pour la couleur rouge
lv_style_t style_orange;       // Style pour la couleur orange
lv_style_t style_yellow;       // Style pour la couleur jaune
lv_style_t style_green;        // Style pour la couleur verte
lv_style_t style_transparent;  // Style pour la transparence
lv_style_t style_switch_on;    // Style pour l'interrupteur activé
lv_style_t style_switch_off;   // Style pour l'interrupteur désactivé

// Variables pour contrôler l'état du buzzer et l'ajustement de la distance
bool buzzer_enabled = true;  // Variable pour savoir si le buzzer est activé
int adjustment = 0;          // Variable pour stocker l'ajustement de la distance



// Fonction pour créer l'interface utilisateur
void create_ui()
{
    // Création de l'écran
    lv_obj_t *scr = lv_scr_act();
    // Ajout du titre
    title = lv_label_create(scr);
    lv_label_set_text(title, "Radar de Recul");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 60); // Alignement au centre en haut

    // Création de la barre de progression
    progress_bar = lv_bar_create(scr);
    lv_bar_set_range(progress_bar, 0, 60);
    lv_obj_set_size(progress_bar, 200, 20); // Dimensions de la barre de progression
    lv_obj_align(progress_bar, LV_ALIGN_CENTER, 0, -20); // Positionner au centre avec un décalage vers le haut

    // Création du texte pour afficher la distance
    dist_label = lv_label_create(scr);
    lv_label_set_text(dist_label, "L'obstacle se trouve a ... cm");
    lv_obj_align(dist_label, LV_ALIGN_CENTER, 0, 20); // Positionner sous la barre de progression

    // Création du conteneur pour les boutons
    btn_cont = lv_obj_create(scr);
    lv_obj_set_size(btn_cont, LV_PCT(100), 90); // Largeur 100% de l'écran, hauteur 100 pixels
    lv_obj_align(btn_cont, LV_ALIGN_BOTTOM_MID, 0, 0); // Aligner en bas au centre avec un décalage vers le haut

    // Ajuster la disposition des boutons
    lv_obj_set_layout(btn_cont, LV_LAYOUT_FLEX);
    lv_obj_set_style_pad_column(btn_cont, 20, 0); // Espacement entre les boutons
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW); // Disposition en ligne
    lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); // Centrer les boutons horizontalement et verticalement

    // Création du bouton Start
    start_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(start_btn, 100, 50);
    start_label = lv_label_create(start_btn);
    lv_label_set_text(start_label, "Start");
    lv_obj_center(start_label);
    lv_obj_add_event_cb(start_btn, start_button_event_handler, LV_EVENT_CLICKED, NULL);

    // Création du bouton Stop
    stop_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(stop_btn, 100, 50);
    stop_label = lv_label_create(stop_btn);
    lv_label_set_text(stop_label, "Stop");
    lv_obj_center(stop_label);
    lv_obj_add_event_cb(stop_btn, stop_button_event_handler, LV_EVENT_CLICKED, NULL);

    // Création du switch Buzzer On/Off
    buzzer_switch = lv_switch_create(btn_cont);
    lv_obj_set_size(buzzer_switch, 100, 50);
    lv_obj_add_event_cb(buzzer_switch, buzzer_switch_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // Création du label pour le switch
    buzzer_label = lv_label_create(btn_cont);
    lv_label_set_text(buzzer_label, "Buzzer On");
    lv_obj_align_to(buzzer_label, buzzer_switch, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // Création du label pour afficher l'ajustement
    adjust_label = lv_label_create(scr);
    lv_label_set_text(adjust_label, "0 cm");
    lv_obj_align(adjust_label, LV_ALIGN_LEFT_MID, 50, 10); // Positionner à gauche au milieu avec un décalage vers la gauche

    // Initialisation des styles
    lv_style_init(&style_red);
    lv_style_set_bg_color(&style_red, lv_color_make(255, 0, 0));
    lv_style_set_bg_opa(&style_red, LV_OPA_COVER);

    lv_style_init(&style_orange);
    lv_style_set_bg_color(&style_orange, lv_color_make(255, 165, 0));
    lv_style_set_bg_opa(&style_orange, LV_OPA_COVER);

    lv_style_init(&style_yellow);
    lv_style_set_bg_color(&style_yellow, lv_color_make(255, 255, 0));
    lv_style_set_bg_opa(&style_yellow, LV_OPA_COVER);

    lv_style_init(&style_green);
    lv_style_set_bg_color(&style_green, lv_color_make(0, 255, 0));
    lv_style_set_bg_opa(&style_green, LV_OPA_COVER);

    lv_style_init(&style_transparent);
    lv_style_set_bg_opa(&style_transparent, LV_OPA_TRANSP);

    lv_style_init(&style_switch_off);
    lv_style_set_bg_color(&style_switch_off, lv_color_make(255, 0, 0));
    lv_style_set_bg_opa(&style_switch_off, LV_OPA_COVER);

    lv_style_init(&style_switch_on);
    lv_style_set_bg_color(&style_switch_on, lv_color_make(0, 0, 255));
    lv_style_set_bg_opa(&style_switch_on, LV_OPA_COVER);

    // Application du style initial
    lv_obj_add_style(progress_bar, &style_green, LV_PART_INDICATOR);

    // Création du slider pour ajuster les distances d'activation
    slider = lv_slider_create(scr);
    lv_obj_set_size(slider, 20, 100); // Dimensions du slider
    lv_obj_align(slider, LV_ALIGN_LEFT_MID, 20, -20); // Positionner à gauche au milieu avec un décalage vers la droite
    lv_slider_set_range(slider, 0, 100); // Définir la plage de valeurs du slider
    lv_obj_add_event_cb(slider, slider_event_handler, LV_EVENT_VALUE_CHANGED, NULL); // Ajouter un gestionnaire d'événements
}

// Fonction de gestion de l'événement du bouton Start
void start_button_event_handler(lv_event_t *e) 
{
    radar_active = true; // Activer le radar
    printf("Radar started\n"); // Imprimer un message indiquant que le radar a démarré
}

// Fonction de gestion de l'événement du bouton Stop
void stop_button_event_handler(lv_event_t *e) 
{
    radar_active = false; // Désactiver le radar
    buzzer = 0; // Assurer que le buzzer est éteint quand le radar est arrêté
    printf("Radar stopped\n"); // Imprimer un message indiquant que le radar a été arrêté
}

// Fonction de gestion de l'événement du switch Buzzer On/Off
void buzzer_switch_event_handler(lv_event_t *e)
{
    lv_obj_t *sw = lv_event_get_target(e); // Obtenir l'objet cible de l'événement
    buzzer_enabled = lv_obj_has_state(sw, LV_STATE_CHECKED); // Vérifier si le switch est activé

    if (buzzer_enabled)
    {
        lv_label_set_text(buzzer_label, "Buzzer On"); // Mettre à jour le label
        lv_obj_add_style(sw, &style_switch_on, 0); // Appliquer le style pour le switch activé
        lv_obj_remove_style(sw, &style_switch_off, 0); // Retirer le style pour le switch désactivé
    }
    else
    {
        lv_label_set_text(buzzer_label, "Buzzer Off"); // Mettre à jour le label
        lv_obj_add_style(sw, &style_switch_off, 0); // Appliquer le style pour le switch désactivé
        lv_obj_remove_style(sw, &style_switch_on, 0); // Retirer le style pour le switch activé
        buzzer = 0; // Éteindre le buzzer immédiatement
    }
    printf("Buzzer %s\n", buzzer_enabled ? "enabled" : "disabled"); // Imprimer l'état du buzzer
}

// Fonction de gestion de l'événement du slider
void slider_event_handler(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e); // Obtenir l'objet cible de l'événement
    int slider_value = lv_slider_get_value(slider); // Obtenir la valeur du slider

    // Calculer l'ajustement en fonction de la valeur du slider
    adjustment = slider_value / 20; // Par exemple, si slider_value est de 100, adjustment sera de 5

    // Mettre à jour le label d'ajustement
    char buf[32];
    sprintf(buf, "+%d cm", adjustment); // Formatage de la chaîne avec l'ajustement
    lv_label_set_text(adjust_label, buf); // Mettre à jour le texte du label

    printf("Slider value: %d, Adjustment: +%d cm\n", slider_value, adjustment); // Imprimer la valeur du slider et l'ajustement
}

// Fonction de mise à jour de la barre de progression en fonction de la distance
void update_progress_bar(int dist)
{
    if (dist < 5 + adjustment) 
    {
        lv_bar_set_value(progress_bar, 60, LV_ANIM_OFF); // Remplir complètement la barre de progression
        lv_obj_remove_style(progress_bar, NULL, LV_PART_INDICATOR); // Retirer les styles précédents
        lv_obj_add_style(progress_bar, &style_red, LV_PART_INDICATOR); // Appliquer le style rouge
    } 
    else 
    {
        // Mise à jour de la valeur de la barre de progression
        lv_bar_set_value(progress_bar, dist, LV_ANIM_OFF);

        // Supprimer les styles précédents
        lv_obj_remove_style(progress_bar, NULL, LV_PART_INDICATOR);

        // Changer la couleur de la barre de progression en fonction de la distance
        if (dist <= 10 + adjustment) 
        {
            lv_obj_add_style(progress_bar, &style_red, LV_PART_INDICATOR);
        } 
        else if (dist <= 20 + adjustment) 
        {
            lv_obj_add_style(progress_bar, &style_orange, LV_PART_INDICATOR);
        } 
        else if (dist <= 30 + adjustment) 
        {
            lv_obj_add_style(progress_bar, &style_yellow, LV_PART_INDICATOR);
        } 
        else if (dist <= 40 + adjustment) 
        {
            lv_obj_add_style(progress_bar, &style_green, LV_PART_INDICATOR);
        } 
        else 
        {
            lv_obj_add_style(progress_bar, &style_transparent, LV_PART_INDICATOR);
        }
    }
}

// Fonction de mise à jour du texte avec la distance
void update_distance_label(int dist) 
{
    char buf[32]; // Un tampon pour stocker la chaîne formatée
    sprintf(buf, "L'obstacle se trouve a %d cm", dist); // Formatage de la chaîne avec la distance
    lv_label_set_text(dist_label, buf); // Mettre à jour le texte du label avec la nouvelle chaîne
}

// Fonction de gestion du buzzer en fonction de la distance
void sound_buzzer(int dist)
{
    if (!buzzer_enabled) 
    {
        buzzer = 0; // Éteindre le buzzer si désactivé
        return;
    }

    if (dist < 5 + adjustment) 
    {
        buzzer = 1; // Activer le buzzer en continu
    } 
    else if (dist <= 10 + adjustment) 
    {
        buzzer.period(1.0 / 1000.0); // Fréquence de 1 kHz
        buzzer = 0.5; // 50% duty cycle
        ThisThread::sleep_for(100ms);
        buzzer = 0;
    } 
    else if (dist <= 20 + adjustment) 
    {
        buzzer.period(1.0 / 1000.0); // Fréquence de 1 kHz
        buzzer = 0.5; // 50% duty cycle
        ThisThread::sleep_for(200ms);
        buzzer = 0;
    } 
    else if (dist <= 30 + adjustment) 
    {
        buzzer.period(1.0 / 1000.0); // Fréquence de 1 kHz
        buzzer = 0.5; // 50% duty cycle
        ThisThread::sleep_for(400ms);
        buzzer = 0;
    }  
    else if (dist <= 40 + adjustment) 
    {
        buzzer.period(1.0 / 1000.0); // Fréquence de 1 kHz
        buzzer = 0.5; // 50% duty cycle
        ThisThread::sleep_for(700ms);
        buzzer = 0;
    } 
    else 
    {
        buzzer = 0; // Éteindre le buzzer si la distance est supérieure à 40 cm
    }
}
