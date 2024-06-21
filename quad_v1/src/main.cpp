#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_VCNL4040.h>

// Définir les broches
const int brocheCapteur = 27; // Broche où est connecté le CNY70
const float diametreRoue = 0.2; // Diamètre de la roue en mètres
volatile unsigned int compteurTours = 0; // Compteur de tours de roue

// Variables pour calculer la vitesse
unsigned long tempsPrecedent = 0;
const unsigned long intervalle = 1000; // Intervalle pour le calcul de la vitesse (en ms)
float vitesse = 0.0; // Vitesse en m/s
float vitesseKmh = 0.0; // Vitesse en km/h

// Définir le pin du capteur de courant
#define SENSOR_PIN 25

// Définir les broches pour I2C
#define SDA_PIN 21
#define SCL_PIN 22

// Définir les broches pour les LED
#define LED1_PIN 19
#define LED2_PIN 18

// Initialiser le capteur
Adafruit_VCNL4040 vcnl4040 = Adafruit_VCNL4040();

// Seuil de luminosité pour allumer les LED
#define LUMINOSITY_THRESHOLD 100

// Fonction d'interruption pour compter les tours
void IRAM_ATTR compterTours() {
compteurTours++;
}

// Tâche pour calculer la vitesse
void calculerVitesse(void *parameter) {
while (1) {
unsigned long tempsActuel = millis();

    if (tempsActuel - tempsPrecedent >= intervalle) {
        tempsPrecedent = tempsActuel;

        // Calcul de la vitesse en m/s
        float distanceParTour = PI * diametreRoue; // Distance parcourue par tour (circonférence de la roue)
        vitesse = (compteurTours * distanceParTour) / (intervalle / 1000.0); // Vitesse en m/s
        compteurTours = 0; // Réinitialiser le compteur

        // Conversion de la vitesse en km/h
        vitesseKmh = vitesse * 3.6;

        // Afficher la vitesse
        Serial.print("Vitesse: ");
        Serial.print(vitesseKmh);
        Serial.println(" km/h");
    }

    // Délai pour permettre à d'autres tâches de s'exécuter
    vTaskDelay(100 / portTICK_PERIOD_MS);
}
}

// Tâche pour lire le capteur de courant
void lireCapteurCourant(void *parameter) {
while (1) {
// Lire la valeur analogique du capteur de courant
int analogValue = analogRead(SENSOR_PIN);

    // Convertir la valeur analogique en courant (A)
    float voltage = (analogValue / 4095.0) * 3.3;  // ADC 12 bits, tension de référence 3.3V
    float current = (voltage - 2.5) / 0.04;

    // Afficher le courant
    Serial.print("Courant: ");
    Serial.print(current);
    Serial.println(" A");

    // Délai pour permettre à d'autres tâches de s'exécuter
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
}

// Tâche pour gérer la luminosité et les LED
void gererLuminositeEtLED(void *parameter) {
// Initialiser les broches des LED
pinMode(LED1_PIN, OUTPUT);
pinMode(LED2_PIN, OUTPUT);

// Initialiser la communication I2C
Wire.begin(SDA_PIN, SCL_PIN);

// Vérifier si le capteur est correctement initialisé
if (!vcnl4040.begin(VCNL4040_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("Erreur de communication avec le capteur VCNL4040 !");
    while (1); // Boucle infinie si le capteur ne se connecte pas
}

Serial.println("Capteur VCNL4040 initialisé avec succès !");

while (1) {
    // Lire la valeur de luminosité
    uint16_t proximity = vcnl4040.getProximity();

    // Afficher la valeur de luminosité sur la console série
    Serial.print("Proximité: ");
    Serial.println(proximity);

    // Allumer ou éteindre les LED en fonction de la luminosité
    if (proximity < LUMINOSITY_THRESHOLD) {
        digitalWrite(LED1_PIN, HIGH); // Allumer LED1
        digitalWrite(LED2_PIN, HIGH); // Allumer LED2
    } else {
        digitalWrite(LED1_PIN, LOW); // Éteindre LED1
        digitalWrite(LED2_PIN, LOW); // Éteindre LED2
    }

    // Délai pour permettre à d'autres tâches de s'exécuter
    vTaskDelay(500 / portTICK_PERIOD_MS);
}
}

void setup() {
Serial.begin(115200); // Initialisation de la communication série
pinMode(brocheCapteur, INPUT); // Définir la broche du capteur comme entrée
attachInterrupt(digitalPinToInterrupt(brocheCapteur), compterTours, RISING); // Attacher l'interruption à la broche du capteur

pinMode(SENSOR_PIN, INPUT); // Définir la broche du capteur de courant comme entrée

// Créer la tâche pour calculer la vitesse
xTaskCreate(calculerVitesse, "Calculer Vitesse",10000,NULL, 1,NULL);

// Créer la tâche pour lire le capteur de courant
xTaskCreate(lireCapteurCourant,"Lire Capteur Courant", 10000,NULL,1,NULL);

// Créer la tâche pour gérer la luminosité et les LED
xTaskCreate(gererLuminositeEtLED, "Gerer Luminosite LED",  10000, NULL,1,NULL );
}

void loop() {
// La boucle principale est vide car tout est géré par les tâches
}