# Jeu 2048 en C

Ce jeu propose de jouer sur un terminal au 2048 avec les mêmes règles que les jeux en lignes

## Table des matières
- [Fonctionnement](#fonctionnement)
- [Technologies utilisées](#technologies-utilisées)
- [Exécution](#exécution)
- [Collaboration](#collarboration)
- [Structure du projet](#structure-du-projet)
- [Images du jeu](#exemple-imagé-du-jeu)

## Fonctionnement

Le fichier *input_manager.c* gère les entrées utilisateurs, c'est à dire les flèches  (↑, ↓, ←, →) pour jouer ou encore le ctrl + C ou q pour quitter le programme. Il transmet ensuite cette commande avec un signal au thread
processus 2048 qui gère le fait de bouger la grille, la vérification de la victoire ou défaite et le jeu en lui même. Le thread move and score permet de bouger les éléments de la grille de jeu et de gérer la logique du jeu en fonction
de l'entrée utilisateur fourni. Le thread goal permet de vérifier si il y a une case 2048 ce qui engendre la victoire du joueur, ou si il n'est plus possible de jouer et donc cela amène à la fin de la partie et du programme.
Toutes ces infos sont transmises à display qui affiche la grille de jeu et l'état de la partie (victoire, défaite, en cours).

## Technologies utilisées
- **Langage** : C
- **Émulateur** : Visual Studio Code
- **Compilation** : Makefile
- **Structure** : Thread / Signaux / Pipe (nommé et anonyme)
- **Interface Graphique** : Terminal Linux

## Exécution

1. **Cloner le projet** 
   ```bash
   git clone (http / ssh du projet)
   ```
2. **Compiler le projet**
    ```bash
    make clean
    ```
    ```bash
    make
    ```
3. **Executer le projet**
    ```bash
    ./bin/
    ```

## Collaboration

Projet réalisé en groupe de 3 étudiants en 2ème année de BUT Informatique. 

([Lisnarde](https://github.com/Lisnarde) , [Spoltrim](https://github.com/noahdumangin) 

## Structure du projet
Structure demandé par les professeurs pour le projet : 


![1](res/Shema_prof.png)


Schéma de la structure que nous avons réalisé à l'issue du projet : 


![1](res/Schema_structure.png)


## Exemple imagés du jeu

Image en jeu : 



![1](res/InGame.png)


Image de fin en cas de défaite : 



![1](res/Defeat.png)
