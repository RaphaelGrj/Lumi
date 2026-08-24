# PROGRESS.md

Cahier d'avancement du projet Lumi. À lire en premier en début de session, à mettre à jour en fin de session (ou dès qu'un problème est résolu).

Format par entrée : date, ce qui a été fait, problèmes rencontrés et comment ils ont été résolus (ou pas), et la suite. Entrées classées de la plus récente à la plus ancienne.

---

## 2026-08-24 (session 2) — Passage à une structure modulaire

**Fait :**
- Réécriture complète des deux firmwares en modules, selon le plan validé avec l'utilisateur (portée choisie : enum d'état + réception UART caméra côté brain ; **pas** de découpage FreeRTOS multi-tâches complet cette fois, pour limiter le risque de bugs de concurrence difficiles à déboguer sur hardware réel).
- Côté brain (`src/brain/`) : `pins.h`, `state.h` (enum `EmotionState` remplaçant les magic numbers de `etatEmotion`), `behavior.cpp/h` (seul module qui écrit l'état), `display.cpp/h` (tâche FreeRTOS d'affichage, inchangée niveau core/priorité/stack), `sensors.cpp/h`, `audio.cpp/h` (mic, Wit.ai STT, TTS, détection de déclenchement, comparaison au wake word), `gemini.cpp/h`, `wifi.cpp/h` (portail + Preferences), `camera_link.cpp/h` (**nouveau** — n'existait pas avant), `main.cpp` réduit à l'orchestration `setup()`/`loop()`.
- Côté vision (`src/vision/`) : `pins.h`, `motion.cpp/h` (init caméra + détection de mouvement par diff de pixels), `protocol.cpp/h` (émission du protocole `T,x,y` / `P,1`/`P,0`), `main.cpp` réduit à l'orchestration.
- Les deux environnements compilent : `pio run -e lumi_brain_s3` et `pio run -e lumi_vision_cam` → `SUCCESS` (RAM 18%/8%, Flash 40%/18%).

**Problème rencontré et résolu — liaison UART caméra↔brain :**
En concevant `camera_link`/`protocol`, j'avais d'abord prévu un `Serial1` dédié des deux côtés (S3 GPIO43/44 ↔ CAM GPIO1/3). Erreur : l'ESP32-CAM AI-Thinker n'a **qu'un seul UART matériel exposé** (GPIO1/3, UART0), déjà utilisé par `Serial` pour le flash/les logs — il n'y a pas de second UART libre à câbler côté caméra (contrairement au S3, qui a un vrai UART0 libre sur GPIO43/44 grâce à l'USB natif). Corrigé avant toute génération de code définitif : côté vision, `protocol.cpp` envoie les lignes `T,`/`P,` sur le même `Serial` que les logs de boot — exactement comme le faisait déjà le code d'origine avec son `Serial.printf("FACE:...")`. Conséquence assumée : le lien S3↔CAM et l'adaptateur FTDI de flash ne peuvent pas être branchés en même temps sur la CAM.

**Broches UART proposées (non câblées physiquement, à valider avant soudure) :**
- S3 (brain) : `Serial1` sur GPIO43 (TX) / GPIO44 (RX) — libres car `ARDUINO_USB_CDC_ON_BOOT=1` fait passer `Serial` par l'USB natif.
- CAM (vision) : `Serial` partagé, GPIO1 (TX) / GPIO3 (RX) — mêmes broches que le connecteur de programmation FTDI.
- GND commun entre les deux cartes (rappel doc §16).

**Suite possible (non commencé) :**
- Souder/tester physiquement le lien UART une fois les broches ci-dessus validées par l'utilisateur, puis vérifier sur hardware réel que `camera_link.cpp` parse correctement les trames envoyées par `protocol.cpp`.
- Brancher `camera_gazeX()/Y()`/`camera_hasPresence()` (déjà exposés côté brain) sur le mouvement des yeux dans `display.cpp` — actuellement les yeux bougent toujours de façon aléatoire, la caméra n'influence rien visuellement pour l'instant.
- `E,id` (identité) n'est pas émis — pas de reconnaissance faciale/animale existante ; à ajouter le jour où cette capacité existe côté vision.
- Le découpage FreeRTOS multi-tâches complet (§9) et le vrai Mood Engine (§11-12, variables continues) restent à faire, volontairement hors de cette passe.
- Warnings de compilation pré-existants (non introduits ici) : `pin_sscb_sda`/`pin_sscb_scl` dépréciés dans `esp_camera.h` côté vision — cosmétique, à corriger un jour en renommant vers `pin_sccb_*`.

**Commit & push :** `e92ae18` sur `platformio-project`, poussé sur GitHub (`RaphaelGrj/Lumi-Project`). `gh` CLI a été installé en local (`~/.local/bin/gh`, sans sudo) et authentifié via device code (`gh auth login`) pour ce push — réutilisable directement dans une prochaine session (`gh auth status` pour vérifier que le token est toujours valide).

---

## 2026-08-24 (session 1) — Initialisation du projet (CLAUDE.md + PROGRESS.md)

**Fait :**
- Lecture complète de `LUMI — Architecture & Roadmap.md`, `README.md`, `platformio.ini`, `src/brain/main.cpp`, `src/vision/main.cpp`.
- Création de `CLAUDE.md` (consignes pour Claude Code) : commandes PlatformIO, architecture réelle vs architecture cible, pinout figé, règles de code du doc d'architecture.
- Création de ce fichier `PROGRESS.md`.

**Constat sur l'état réel du code** (par rapport au doc d'architecture qui décrit la cible) :
- Les deux firmwares sont des fichiers `main.cpp` monolithiques, pas encore découpés en modules (`display.cpp/h`, `behavior.cpp/h`, `sensors.cpp/h`, etc. — voir doc §20).
- Pas de vraie State Machine / Mood Engine : `etatEmotion` est un simple `volatile int` avec des valeurs numériques magiques (0 = idle, 4 = glitch démarrage, 5 = parle, 7 = portail wifi ouvert, 10 = écoute active).
- Côté brain : une seule tâche FreeRTOS (affichage, core 0) existe. Pas de TaskSensors / TaskCamera / TaskAudio / TaskBehavior séparées comme prévu au §9.
- Côté vision : le protocole UART envoyé est `FACE:<x>,<y>\n`, alors que le doc d'architecture (§7/§16) prévoit `T,x,y` / `P,1` / `E,id`. **Pas encore réconcilié — le brain actuel n'a d'ailleurs aucune lecture UART de la caméra implémentée.**
- Le mot de réveil est vérifié en dur (`texteEntendu == "lumi"`) alors qu'un `build_flag` `WAKE_WORD="Lumi"` existe déjà dans `platformio.ini` sans être utilisé dans le code.
- `client.setInsecure()` est utilisé pour les deux appels HTTPS (Wit.ai et Gemini) — un certificat root CA est présent en commentaire dans le code mais non branché (TODO explicite dans le code lui-même).
- `upload_port` de l'environnement `lumi_vision_cam` est commenté dans `platformio.ini` (à décommenter/adapter avant flash).

**Problèmes rencontrés :** aucun, session de découverte/initialisation uniquement, rien codé sur le firmware.

**Suite possible (non commencé) :**
- Décider si on garde le code monolithique actuel comme base ou si on repart sur la structure modulaire cible (l'utilisateur a donné carte blanche pour repartir de zéro si besoin).
- Implémenter la réception UART côté brain (actuellement absente) et aligner le protocole vision → brain sur un format unique.
- Sortir `etatEmotion` des valeurs magiques vers des constantes nommées / enum, prérequis avant toute vraie state machine.

---

## Historique antérieur (importé de `MAJ /MAJ 20260813`, non structuré au même format — conservé pour référence)

État au 13/08/2026 :
- Cerveau (S3) & animations : ~90 % — architecture dual-core fonctionnelle (les yeux ne freezent pas), portail Wi-Fi open-source intégré, BME280 (température) OK, MPU6050 (secousses) en cours.
- Suivi du regard (caméra) : ~90 % — flux vidéo Wi-Fi abandonné (conflits matériels), tracking par différence de pixels écrit, en attente de validation.
- Audio (I2S) : à faire — intégration haut-parleur + ampli MAX98357A, TTS des réponses Gemini.
- Âme / personnalité : à faire ensuite — "header contextuel" dynamique et gestion des humeurs.
- Home Assistant : pas commencé.

*(Note : au vu du code actuel dans `src/`, une partie de ce qui précède — I2S, TTS Gemini, portail Wi-Fi, BME280 — semble avoir avancé depuis le 13/08 puisque le code présent l'implémente déjà. À confirmer avec l'utilisateur si besoin de savoir précisément ce qui a été testé sur le hardware réel vs seulement écrit.)*
