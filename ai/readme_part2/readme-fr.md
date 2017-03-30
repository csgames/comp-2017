# Paper Hockey - Parti 2

---

## Power-up

La position du power-up est envoyé au joueurs au début de la partie:
```
power up is at (x, y)
```
Ou `(0,0)` est le coin nord-ouest.

Le premier joueur qui atteind cette destination bénificie du power up.
le power up permet de faire un rebond même si la node de destination n'a pas été visité.

---

Le joueur peu utiliser le power upen utilisant le préfix `power` à ça commande:
```
power south
```
ce qui permet au joueur un lancé au sud et de rejouer.

---

## Polarité

Le message suivant peux être envoyé au joueur au courant de la partie:
```
polarity of the goal has been inverted
```

Ceci signifie que les but ont été inverser. Donc si tu votre objectif était de faire un but au nord, maintenant il faut vous rendre au sud. 

---

<b>Ceci peux arrivé plusieurs fois dans une même partie</b>

From: src/hockey2/controller_polarity.py#L35
```
def move(self, action):
    [...]
    result = super(ControllerPolarity, self).move(action)
    if result.valid:
        polarityInverted = random.randint(0, 10) == 9
    [...]
```

---

## Nouvelle taille de patinoire

La patinoire est maintenant de dimension `15x15` au lieu de `11x11`.

---

## Évaluation

- Chaque AI va jouer jusqu'à 50000 parties (ou 10 minutes) contre chaque adversaire.
- Vous fournissez votre code source ainsi qu'un binaire exécutable. Assurez-vous d'avoir déclaré vos dépendancse de manière explicite.
- Votre AI n'aura pas accès à internet - désolé DeepMind.
- Assurez vous que votre code fonctionne sous ubuntu 16.04 avec les packages officiels
- 10% du pointage est donnée si vous avez un script bash "run.sh" qui démarre votre AI.
- 90% du pointage est donnée en fonction de vos points dans le round robin.
 
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
