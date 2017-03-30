Bienvenue à Silicon Valley.

Ici, le jeu d'échec est obselète. Ainsi, les gens ont commencé à jouer à un nouveau jeu à un contre un...

Ça s'appelle paper hockey.

---

# Paper Hockey

Paper Hockey est un jeu en tour par tour qui se déroule sur une grille de format `11x11`.

Le jeu commence alors que la rondelle est au centre de la grille. L'objectif du jeu est d'amener la rondelle dans le filet de votre adversaire.

Les buts sont aux limites nord et sud de la grille(voir l'image ci dessous)

<img src="images/empty_board.png" width="300">

---

## Règles:
 - À chaque tour, un joueur lance la rondelle dans une des 8 directions ((N, S, O, E et leurs diagonales))
 - La rondelle ne peut emprunter deux fois la même ligne - une fois que la glace est brisée, il n'y a pas de retour en arrière.
 - Lorsque la rondelle frappe une node déjà visitée ou un mur, elle rebondit, ce qui permet au joueur actif de faire un autre lancé.
 - Vous avez 2 secondes pour faire votre lancé.  Si vous échouez à faire votre lancé durant ce délai, vous perdez la partie.
 - La partie est gagnée en faisant un but ou en forçant le joueur adverse à l'état d'échec et mat (voir la section `Échec et mat`)
 
---

## Exemple de but

1. Joueur 1 (vert) lance au nord (vers le but dans lequel il doit marquer):

<img src="images/one.png" width="300" alt="vert nord"/>

---

2. Joueur 2 (rouge) lance au nord-est (un mouvement stupide):

<img src="images/two.png" width="300" alt="vert nord, rouge nord est"/>

---

3. Joueur 1 (vert) lance au nord.
4. Joueur 2 (red) lance au nord-ouest.
5. Joueur 1 (vert) lance au nord-ouest, atteint le mur, ce qui résulte en un **rebond**.
6. Joueur 1 (vert) lance au nord-est, score un **but** et gagne la partie.

<img src="images/won.png" width="300" alt="vert nord, rouge nord est, vert nord, rouge nord ouest, vert nord ouest, vert nord est">

Voici un récapitulatif de la partie:

 * P1 - nord
 * P2 - nord est
 * P1 - nord
 * P2 - nord ouest
 * P1 - nord ouest [bounce]
 * P1 - nord est

---

## Exemple d'un rebondissement épique

Imaginons le scénario où les joueurs dessineraient un sapin en s'échangeant la rondelle.:

<img src="images/pine.gif" width="300"/>

---

Le joueur 2 a maintenant l'opportunité de terminer la partie en un seul tour en utilisant une série de rebonds sur des nodes déjà visitée et sur le mur:

<img src="images/pine_wild.gif" width="300"/>

Certains peuvent seulement espérer que le canadien de Montréal possède ce genre d'habiletés d'offensive.

---

## Exemple Échec et mat

Dans ce jeu, le joueur 1 vient de lancer la rondelle vers le mur et doit rejouer (rebond).  Puisque tous les segments qui sortent de cette node sont brisés, Joueur 1 ne peut faire de lancé valide et il perd par échec et mat.  

<img src="images/check.gif" width="300" alt="vert nord ouest, rouge nord ouest, vert nord ouest, rouge nord ouest, vert nord ouest"/>

---

Ceci est un autre exemple d'échec et mat où le joueur 1 perd.

<img src="images/checkmate.gif" width="300"/>

---

## Setup

<i>Requis: Python & tox</i>

### Installation
```bash
tox -e py3
source .tox/py3/bin/activate
```
ou
```bash
pip install -r requirements.txt
```

---

### Démarrer le serveur
```bash
python src/server.py
```

### Démarrer un AI aléatoire
```bash
source .tox/py3/bin/activate
python src/client.py
```

---

## Par où commencer?
Pour jouer, il faut vous connecter sur localhost:8023.
Le protocole est text-based - vous pouvez même jouer à partir de telnet.

Une fois connecté, la première chose que vous receverez est:
```
What's your name?
```

Vous devez répondre par votre nom d'équipe et recevoir cette confirmation:

```
Welcome, Bob you're Joueur 0!
```

---

Une fois que les deux joueurs se sont connectés à la partie, vous allez obtenir l'information suivante:
* position de la balle
* où se situe votre but
* {nom_du_joueur} is active Joueur [seulement si c'est votre tour]
```
Game is on - 1
ball is at (5, 5) - 3
your goal is north - 5
Bob is active player - 7
```

---

### Lancés

Lorsque vous êtes le joueur actif, vous pouvez envoyer une des commandes suivantes:

```
north
north east
east
south east
south
south west
west
north west
```

---
                                                     
Une fois qu'un lancé valide a été donné, le joueur 1 (Bob dans cette exemple) va recevoir:
```
south
Bob did go south - 9
```
et le joueur 2 (Malory dans cet exemple) va recevoir:
```
Bob did go south - 8
Malory is active player - 10
```

---

### Rebond
Le joueur qui rebondit sera informé par le message "{nom_du_joueur} is active Joueur":

```
Bob did go north west - 15
Malory is active player - 17
east
Malory did go east - 18
Malory is active player - 20
east
Malory did go east - 22
```

---

### Mouvement invalide

Si Malory essaie d'aller au Nord maintenant, elle recevra le message "invalid move".

Bob ne sera pas informé de ce mouvement invalide.

```
Bob did go south - 8
Malory is active player - 10
invalid move - 11
```

---

Si vous envoyez une action pendant que ce n'est pas votre tour vous receverez:

```
ignoring action south - 20
```

---

### Gagner la partie
Voici le résultat lorsqu'un joueur gagne la partie:

```
Malory won a goal was made - 34
```

### Replay
Vous pouvez consultez le replay de la partie dans le répertoire <i>test/output/</i>.

---

## Évaluation

- Chaque AI va jouer jusqu'à 50000 parties (ou 10 minutes) contre chaque adversaire.
- Vous fournissez votre code source ainsi qu'un binaire exécutable. Assurez-vous d'avoir déclaré vos dépendancse de manière explicite.
- Votre AI n'aura pas accès à internet - désolé DeepMind.
- 10% du pointage est donnée si vous avez un script bash "run.sh" qui démarre votre AI.
- 90% du pointage est donnée en fonction de vos points dans le round robin.
- Cette partie de la compétition 4% du pointage global 
---

### Pointage détaillé

score = (team_count-position)/(team_count-1) 

Exemple s'il y a 30 équipes

- position == score 
- 1 == 90% maximum score
- 2 == 87%
- 3 == 84%
- 4 == 81%
- 5 == 78%
- 6 == 74%
- 7 == 71%
