# RACER - Jeu de Course sur Écran OLED

Ce projet est une implémentation d'un jeu de course automobile ("RACER") pour système embarqué Linux (type DE1-SoC). Le jeu s'interface avec un écran OLED 4D Systems via une liaison série et utilise des boutons connectés aux GPIO pour le contrôle.

## 📋 Description

Le joueur pilote une voiture sur une route à trois voies et doit éviter des obstacles. Le but est de survivre le plus longtemps possible pour augmenter son score.
- **Score** : Augmente avec la distance et les obstacles évités.
- **High Score** : Le meilleur score est conservé tant que le programme est actif.

## ⚙️ Configuration Matérielle

Le logiciel est conçu pour fonctionner avec la configuration suivante :

### Affichage (OLED)
* **Écran** : Module OLED Goldelox (4D Systems).
* **Connexion** : Port série `/dev/ttyAL0`.
* **Vitesse** : 9600 bauds.

### Contrôles (GPIO)
* **Bouton Gauche** : `gpiochip2`, ligne 1 (Falling Edge).
* **Bouton Droit** : `gpiochip2`, ligne 0 (Falling Edge).
* **Bouton Select** : `gpiochip3`, ligne 0 (Both Edges).

## 🚀 Compilation et Installation

Ce projet utilise un `Makefile` pour la compilation croisée (Cross-compilation pour ARM).

### Prérequis
* Chaîne de compilation ARM (ex: `arm-none-linux-gnueabihf-gcc`).
* Outils standards (Make).

### Commandes de base

1.  **Compiler le projet** :
    ```bash
    make
    ```
    Cela génère l'exécutable binaire `game`.

2.  **Nettoyer les fichiers de compilation** :
    ```bash
    make clean
    ```

3.  **Installer sur la cible** :
    ```bash
    make install
    ```
    Installe le programme dans `/home/root/` (par défaut défini dans le Makefile).

## 🎮 Comment Jouer

Lancez le jeu depuis le terminal de la cible :
```bash
./game