# 🏠 PostClient - Client Domotique M5Stack CoreS3

## 📋 Description

Client de contrôle et de monitoring pour **M5Stack CoreS3** permettant d'interagir avec un système domotique centralisé. Ce projet agit comme une télécommande intelligente capable d'envoyer des données environnementales et de piloter des équipements distants.

Ce projet implémente un **client ESP32** qui communique avec un serveur pour :

- **S'authentifier** de manière sécurisée avec le serveur central
- **Envoyer** les données de température/humidité (DHT22)
- **Contrôler** des équipements distants (Lumière, Porte, Chauffage) via l'écran tactile
- **Diffuser** l'écran en streaming via HTTP

## ✨ Fonctionnalités

### 🖥️ Interface Graphique (Menus)

- **Navigation par pages** (3 menus distincts)
- **Design moderne** avec cartes et feedback visuel
- **Menus disponibles** :
  1. **SYSTEM & AUTH** : Gestion de la connexion et du statut
  2. **CLIMATE & LIGHTS** : Gestion de l'envoi météo et des lumières
  3. **SECURITY & HEAT** : Contrôle de la porte et du chauffage

### 🔐 Authentification

- Appairage avec le serveur via `DeviceID` unique (`307D68F23A08`)
- Récupération et stockage d'un **Token** de session
- Sécurisation des requêtes via header `Authorization: DeviceID:Token`

### 📡 Envoi Météo Automatique

- **Mode Auto** : Envoi des données du capteur DHT22 toutes les 5 secondes
- Activation/désactivation facile via le Menu 2
- Indicateur visuel de l'état (AUTO: ON/OFF)

### 📺 Streaming Écran

- **Serveur Web intégré** sur port 80
- Capture d'écran en temps réel (format BMP)
- Page HTML intégrée avec auto-refresh
- Accessible via navigateur : `http://[IP_CLIENT]/`

## 🔧 Configuration Matérielle

### Matériel Requis

- **M5Stack CoreS3** (ESP32-S3)
- **Capteur DHT22** (Température & Humidité)
- **Réseau WiFi** (SSID: `MyHouseOS`)

### Connexions

```
GPIO 17 → Pin DATA du DHT22
3V3     → VCC DHT22
GND     → GND DHT22
```

## ⚙️ Configuration Logicielle

### Bibliothèques Nécessaires

```cpp
#include "M5CoreS3.h"      // M5Stack CoreS3
#include <WiFi.h>          // WiFi
#include <ArduinoJson.h>   // JSON
#include <HTTPClient.h>    // Client HTTP
#include <DHT.h>           // Capteur DHT
#include <WebServer.h>     // Serveur Web
```

### Paramètres Réseau

À modifier dans `PostClient.ino` selon votre infrastructure :

```cpp
// WiFi
const char* ssid = "MyHouseOS";
const char* password = "12345678";

// URLs API
const char* authUrl  = "http://192.168.4.1/link";
const char* tempUrl  = "http://192.168.4.2:3000/temp";
const char* ledUrl   = "http://192.168.4.2:3000/toggle/light";
const char* doorUrl  = "http://192.168.4.2:3000/toggle/door";
const char* heatUrl  = "http://192.168.4.2:3000/toggle/heat";
```

## 🚀 Installation

### 1. Prérequis

- [Arduino IDE](https://www.arduino.cc/en/software)
- Gestionnaire de cartes ESP32 installé

### 2. Installation des Bibliothèques

Via le gestionnaire de bibliothèques Arduino :
- `M5CoreS3`
- `ArduinoJson`
- `DHT sensor library`
- `Adafruit Unified Sensor`

### 3. Upload

1. Sélectionner la carte "M5Stack CoreS3"
2. Connecter l'appareil via USB
3. Téléverser le sketch `PostClient.ino`

## 📱 Utilisation

### Navigation

Le bouton **B (Milieu)** permet de changer de menu (1 → 2 → 3 → 1).

### Menu 1 : SYSTEM & AUTH

| Bouton | Action |
| :--- | :--- |
| **A** (Gauche) | **AUTHENTICATE** : S'identifier auprès du serveur |
| **B** (Milieu) | Menu Suivant |
| **C** (Droite) | **STATUS CHECK** : Affiche l'IP locale |

### Menu 2 : CLIMATE & LIGHTS

| Bouton | Action |
| :--- | :--- |
| **A** (Gauche) | **AUTO ON/OFF** : Activer l'envoi auto de la température |
| **B** (Milieu) | Menu Suivant |
| **C** (Droite) | **TOGGLE LED** : Allumer/Éteindre la lumière distante |

### Menu 3 : SECURITY & HEAT

| Bouton | Action |
| :--- | :--- |
| **A** (Gauche) | **DOOR LOCK** : Verrouiller/Déverrouiller la porte |
| **B** (Milieu) | Retour Menu 1 |
| **C** (Droite) | **HEAT CONTROL** : Allumer/Éteindre le chauffage |

## 🌐 API et Communication

### Authentification

**POST** `/link`
```json
Request: { "id": "307D68F23A08" }
Response: { "token": "votre_token_session" }
```

### Envoi de Données (Météo)

**POST** `/temp`
```json
Header: Authorization: 307D68F23A08:TOKEN
Body: { "temp": "24.5" }
```

### Contrôle (Generic Post)

Utilisé pour Lumière, Porte, Chauffage.

**POST** `/toggle/xxx`
```json
Header: Authorization: 307D68F23A08:TOKEN
Body: { "id": "307D68F23A08", "token": "TOKEN" }
```

## 🎨 Interface Utilisateur

### Palette de Couleurs

```cpp
Fond          : #0f172a (Bleu nuit)
Cartes        : #1e293b (Gris foncé)
Accent        : #6366f1 (Indigo)
Texte         : #f8fafc (Blanc)
Succès        : #22c55e (Vert)
Warning       : #fb923c (Orange)
```

## 🐛 Dépannage

### "AUTH REQUIRED"
- Vous essayez d'effectuer une action sans être authentifié.
- Allez dans le **Menu 1** et appuyez sur **BTN A** pour vous connecter.

### "SENSOR ERROR"
- Le capteur DHT22 ne répond pas.
- Vérifiez le câblage sur le GPIO 17.

### "HTTP ERR"
- Le serveur est injoignable ou renvoie une erreur.
- Vérifiez que le serveur (192.168.4.2) est allumé et accessible sur le même réseau WiFi.

---

**Version :** 1.0
**Date :** Décembre 2025
**Plateforme :** M5Stack CoreS3
