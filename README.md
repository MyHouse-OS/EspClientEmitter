# PostClient - M5Stack CoreS3 Home Automation Client

Ce projet est un client domotique pour le **M5Stack CoreS3**. Il permet d'interagir avec un serveur central pour contrôler des appareils (lumières, chauffage, porte), surveiller la température via un capteur DHT22, et offre une interface web pour la capture d'écran en direct.

## 🛠 Matériel Requis

- **M5Stack CoreS3**
- **Capteur DHT22** (Température & Humidité)
  - Connecté sur la broche **GPIO 17**

## 📚 Bibliothèques Nécessaires

Assurez-vous d'avoir installé les bibliothèques suivantes dans votre IDE Arduino :

- `M5CoreS3`
- `ArduinoJson`
- `DHT sensor library`
- `WiFi` (Standard ESP32)
- `HTTPClient` (Standard ESP32)
- `WebServer` (Standard ESP32)

## ⚙️ Configuration

Avant de téléverser le code, modifiez les variables de configuration réseau au début du fichier `PostClient.ino` :

```cpp
const char* ssid = "MyHouseOS";        // Votre SSID WiFi
const char* password = "12345678";     // Votre mot de passe WiFi

// URLs du serveur API
const char* authUrl  = "http://192.168.4.1/link";
const char* tempUrl  = "http://192.168.4.2:3000/temp";
const char* ledUrl   = "http://192.168.4.2:3000/toggle/light";
const char* doorUrl  = "http://192.168.4.2:3000/toggle/door";
const char* heatUrl  = "http://192.168.4.2:3000/toggle/heat";
```

## 🚀 Fonctionnalités

### Interface Utilisateur (Menus)
L'application dispose de 3 menus navigables via le **Bouton B**.

#### 1. SYSTEM & AUTH
- **Bouton A** : Authentification auprès du serveur (Récupération du Token).
- **Bouton B** : Menu suivant.
- **Bouton C** : Vérification du statut (Affiche l'IP locale).

#### 2. CLIMATE & LIGHTS
- **Bouton A** : Activer/Désactiver l'envoi automatique de la météo (toutes les 5s).
- **Bouton B** : Menu suivant.
- **Bouton C** : Allumer/Éteindre la lumière (LED).

#### 3. SECURITY & HEAT
- **Bouton A** : Verrouiller/Déverrouiller la porte.
- **Bouton B** : Retour au menu 1.
- **Bouton C** : Contrôle du chauffage.

### Interface Web (Screen Mirroring)
Le M5Stack héberge un serveur web local.
- Accédez à `http://<IP_DU_M5STACK>/` pour voir une capture d'écran en direct de l'appareil.
- L'URL est affichée dans le moniteur série au démarrage.

### Indicateurs Visuels
- **Barre d'état** : Indique si l'appareil est lié au serveur ("LINKED OK" ou "NOT LINKED").
- **Feedback** : Des popups de couleur apparaissent pour confirmer les actions (Succès en vert, Erreur en orange/rouge).

## 📝 Notes Techniques
- L'authentification utilise un `DeviceID` codé en dur (`307D68F23A08`).
- Les requêtes HTTP incluent un header `Authorization` avec le format `DeviceID:Token`.
- Le capteur DHT22 est lu pour envoyer la température au endpoint `/temp`.
