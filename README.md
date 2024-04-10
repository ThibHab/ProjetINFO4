Conception d'un support physique, pour controller le logiciel Cardinal-VCVRack. Réalisé dans le cadre d'un projet pour POLYTECH Grenoble.

------
Journal de Bord
------

Journal de Bord - Projet Cardinal

Semaine 1

    Exploration des fonctionnalités de Cardinal et des modules disponibles.
    Décision de limiter le projet à une configuration prédéfinie.
    Apprentissage des fonctionnalités d'un synthétiseur.

Objectifs Fixés:

    Conception d'un support physique pour manipuler une configuration préétablie de Cardinal.

Semaine 2

    Définition du support physique : une carte Arduino permettant la connexion et la déconnexion des fils sur Cardinal.
    Modification des valeurs des potentiomètres sur le VCO à l'aide de potentiomètres physiques.
    Recherche sur l'interprétation des signaux MIDI par Cardinal et la manipulation à distance de celui-ci.
    Découverte de JACK et du logiciel qjackctl pour tester la connectique entre les applications.

Objectifs :

    Décider de l'utilité de JACK dans le projet, voir s'il est possible de faire quelque chose partant de JACK

Semaine 3

Répartition des Tâches

    Thibaut : Modification du code de Cardinal, faire le lien entre Cardinal et l'interface avec JACK.
    Yann : Découverte de PipeWire et qpwgraph, alternative plus récente à JACK et qjackctl pour modifier les E/S audio.
    Gwénolé : Mapping et programmation Arduino.

Changement d'Objectifs

    Réorientation des efforts sur la modification direct du code de 
    Cardinal. Le Chemin étant plus prometteur

Semaine 4

    Thibaut : Progrès dans la modification du code, mais compilation toujours infructueuse.
    Gwénolé : Réalisation du mapping et de la carte physique.
    Yann : Abandon de l'idée d'utiliser JACK ou PipeWire, aucune utilité réelle pour le projet.

Semaine 5

    Yann : Réalisation de l'interface Arduino-Python.
    Thibaut : Progrès dans la modification du programme, réussite dans la compilation du pogramme. 
    Gwénolé : Modification de la map arduino pour mieux répondre aux contraintes

Semaine 6

    Thibaut : Etablissement d'une connexion entre Python et Cardinal via socket traitement de données transmises
    Gwénolé : Formattage des outputs arduino, Ajout de la détection d'erreurs
    Yann : Travail sur la normalisation des inputs/outputs


Semaine 7

    Thibaut : Finition du traitement de données envoyées par la socket
    Gwénolé + Yann : Mise en accord sur le format des échanges entre cardinal et l'arduino
    Yann : Gestion des crash dans l'interface 


Semaine 8 : Partiels

    Tous : Finitions et documentations