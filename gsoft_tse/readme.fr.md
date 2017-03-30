# GSOFT - Team Software Engineering

Chez GSOFT, on s'est développé un amour pour le café. Dernièrement, on s'est dit qu'on pourrait donner un coup de pouce à Sébastien (Notre Barista) avec les commandes de café.
Nous avons donc équipé les machines à espresso avec un Arduino et on a hacké un Roomba afin d'automatiser l'approvisionnement de café au bureau.

Votre objectif est de créer un programme qui prend en entrée une carte plus une liste de commandes et qui va écrire en sortie une série de commandes qui permettra au Roomba et aux machines à espresso de préparer des cafés pour les apporter aux bonnes personnes.

Le programme doit répondre aux spécifications d'entrées suivantes:

    <votre_programme> <map> <missions>

Vous pouvez utiliser le langage de programmation de votre choix pour accomplir cette tâche. Vous devrez fournir toute la documentation nécessaire pour compiler et exécuter votre programme.

## Input
### Carte

La carte fournie est dans un format ASCII:

    ##################      Légende
    #1          #    #      A: Alexandre
    #   #########    #      B: Bastien
    #   ###B         #      C: Carl
    # @ ###          #      ...
    #                #      Z: Zoey
    ############2    #      
    #A               #      1...9: Machines à espressos
    ##################      @: Roomba
                            #: Mur

- Les lettres de A à Z correspondent aux personnes dans la carte, les chiffres correspondent à chaque machine à espresso disponible dans la carte.
- La position de départ du Roomba est représentée par le @.
- Il possible de se déplacer uniquement dans les cases vides.

### Missions
Quelqu'un peut commander du café de 2 façons, soit en passant une description détaillée de la commande en JSON ou sinon avec le nom d'un café qui est disponible dans le menu du café GSOFT.

Une mission est dans le format suivant:

    <person> <coffee>

L'attribut `coffee` est une string UTF8 encodée en Base64 qui contient soit un objet JSON pour un café personnalisé, soit le nom du café à commander à partir du menu GSOFT.

Une mission personnalisée contient la structure JSON suivante:

    {
        "size": <"solo", "doppio", "triplo">,
        "length": <"ristretto", "normale", "lungo">,
        "inverted": <true, false>,
        "ingredients":
            "milk": <0-500>,
            "foam": <0-500>,
            "hot_water": <0-500>
        }
    }

- L'attribut `size` correspond à si il s'agit d'un espresso simple, double ou triple.
- L'attribut `length` correspond à si il s'agit d'un espresso court, normal ou allongé.
- L'attribut `inverted` est à `true` lorsque les ingredients doivent être mis dans la tasse avant le café.
- Les ingredients possibles sont `milk`, `foam` et `hot_water`.
- Les ingredients sont en millilitres (ml).
- Les ingredients sont des multiples de 25.
- Le menu du café GSOFT est disponible dans le fichier `menu.json`.

## Roomba

Le Roomba comprend les commandes suivantes:

Commande | Signification | Description
--- | --- | ---
`F` | "Forward" | Avance le Roomba d'une case selon l'orientation du Roomba
`R` | "Right" | Pivote le Roomba de 90' dans le sens horaire
`L` | "Left" | Pivote le Roomba de 90' dans le sens anti-horaire
`T` | "Take the cup" | Récupère la tasse de la Machine à espresso adjacent
`G` | "Give" | Donne le bon café à la personne adjacent
` ` | "Wait" | Aucune commande

### F.A.Q. sur le Roomba
- Le Roomba peut transporter 2 cafés à la fois.
- Le Roomba commence toujours la mission orienté vers le nord.
- Le Roomba est considéré comme adjacent à la machine à espresso ou à une personne lorsqu'il est soit au Nord, Sud, Est ou Ouest.
- Le Roomba peut interagir avec la machine à espresso ou les personnes peu importe son orientation
- Un transfert de café de la machine au Roomba doit être synchronisé. C'est à dire que le Roomba doit envoyer la commande "T" en même temps que la machine à espresso envoie la commande "G"

## Machine à espresso

Chaque Machine à Espresso comprends les commandes suivantes:

Commande | Signification | Description
--- | --- | ---
`S` | "Insert a Small Cup" | Place une tasse vide de 50ml dans la Machine à espresso
`M` | "Insert a Medium Cup" | Place une tasse vide de 250ml dans la Machine à espresso
`L` | "Insert a Large Cup" | Place une tasse vide de 750ml dans la Machine à espresso
`B` | "Grind 1 portion of coffee beans" | Mouler une portion de grains de café dans le portefiltre
`I` | "Insert Portafilter" | Insère le portefiltre dans la machine à espresso
`E` | "Extract" | Extrait le café moulu dans le portefiltre
`K` | "Milk" | Verse 25ml de lait dans la tasse
`F` | "Foam" | Verse 25ml de mousse de lait dans la tasse
`W` | "Water" | Verse 25ml d'eau chaude dans la tasse
`G` | "Give" | Donne la tasse au Roomba adjacent
`R` | "Remove Portafilter" | Retire le portefiltre de la machine à espresso
`C` | "Clean Portafilter" | Nettoie le portefiltre
` ` | "Wait" | Aucune commande

Préparer un café comprend les étapes suivantes:

- Insérer une tasse de la bonne taille (Voir formule pour déterminer la taille d'un café)
- Mouler la bonne quantité de grains de café 
    - Solo = 1 portion
    - Doppio = 2 portion
    - Triplo = 3 portion
- Insérer le porte filtre
- Si c'est un café de style `inverted`, il faut mettre l'eau, le lait et la mousse de lait avant l'extraction des grains de café moulu
- Extraire la bonne quantité de café 
    - Ristretto = 1 extraction
    - Normale = 2 extraction
    - Lungo = 3 extraction
- Si ce n'est pas un café de style `inverted`, il faut mettre l'eau, le lait et la mousse de lait avant l'extraction des grains de café moulu
- Retirer le porte-filtre
- Nettoyer le porte-filtre
- Donner le café au Roomba

La taille d'un café est déterminée à l'aide de la formule suivante:

(`size` * 25ml) * (`length` * 0.5) + SUM(`ingredients`)

### F.A.Q. sur les Machine à Espressos
- L'ordre des ingrédients est important, il faut toujours y aller dans cet ordre: `hot_water`, `milk` et `foam`.
- Il faut obligatoirement nettoyer la machine avant de préparer un autre café.
- Une fois le café terminé, on peut immédiatement le donner au Roomba et nettoyer la machine par la suite.
- La machine à expresso est limité à préparer 1 café à la fois. Il faut donner le café au Roomba afin de pouvoir préparer un autre café.
- Un transfert de café de la machine au Roomba doit être synchronisé. C'est à dire que le Roomba doit envoyer la commande "T" en même temps que la machine à espresso envoie la commande "G".

## Sortie

La sortie du programme devra être les séries de commandes à executer par le Roomba et les machines à expressos pour la carte et les missions fournit en entrée.

Chaque ligne devra être identifié en premier par l'acteur des commandes (Roomba ou le numéro de la machine à espresso) suivit des commandes séparées par des espaces.

*Exemple*

    1     S B I E K G R C
    @ R F F F L F F T R F F F G

## Exemple avec Solution

*Carte*

    ######
    #1 @  #
     ###   #
    #A    # 
    ######

*Mission*

    Alexandre eyJzaXplIjoiZG9wcGlvIiwibGVuZ3RoIjoibm9ybWFsZSIsImludmVydGVkIjpmYWxzZSwiaW5ncmVkaWVudHMiOnsibWlsayI6NTAsImZvYW0iOjUwfX0=

*Output*

    1 M B B I E E K K F F G R C
    @ L F R R             T F F R F F R F F G

*Explication*

La Mission corresponds au payload suivant:

    {
        "size": "doppio",
        "length": "normale",
        "inverted": false,
        "ingredients": {
            "milk": 50,
        "foam": 50
        }
    }

La taille du café est de 150ml donc il faut une tasse médium. 

    (2 * 25ml) * (2 * 0.5) + (50 + 50) = 150ml

Il ne s'agit pas d'un café inversé, donc il faut extraire le café et ensuite ajouter les ingrédients.

    1 M B B I E E K K F F G R C

De son côté le Roomba doit se rendre à la machine à espresso #1 et attendre après son café. Puisqu'il commence face au nord, il doit se tourner vers la gauche et avancer d'une case pour être adjacent à la machine à espresso.
Il peut profiter de l'attente pour s'orienter dans la bonne direction pour son départ.

    @ L F R R             T F F R F F R F F G

La machine à espresso peut retirer le porte filtre et se nettoyer tandis que le Roomba se dirige vers Alexandre pour lui donner son café une fois adjacent.

## Grille d'évaluation

### General (10%)

2.5% - La remise contient un fichier "run.sh" qui prend les paramètres d'entrée, qui compile et exécute votre solution 

2.5% - Le code fourni compile et s'exécute sans erreurs

2.5% - Votre programme a un menu d'aide (`--help`)

2.5% - Votre programme gère bien les inputs manquants ou invalides

### Machine à Espresso (25%)

2.5% - Les cafés sont préparées avec la bonne taille de tasse

2.5% - Les cafés peuvent être préparées dans le style "inverted"

5.0% - Les cafés du Menu GSOFT sont préparées correctement

7.5% - Les cafés personnalisées sont préparées correctement

7.5% - Les cafés préparés n'attendent pas à la machine à espresso (afin de rester chaud le plus longtemps possible!)

### Roomba (20%)

 2.5% - Le Roomba récupère de façon synchronisée le café des machines à espresso
 
10.0% - Le Roomba visite toutes les personnes qui ont commandé
 
 7.5% - Le Roomba ne percute pas d'obstacles

### Performance (45%)

 5.0% - Carte Easy
 
10.0% - Carte Medium
 
15.0% - Carte Hard

15.0% - Carte Correction

L'équipe qui réussit une carte avec le moins de commandes aura 100% au Bonus Performance. Les autres équipes seront évaluées au prorata de la meilleure équipe.
