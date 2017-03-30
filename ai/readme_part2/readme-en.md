# Paper Hockey - Part 2

---

## Power-ups

Power-up information is sent to players when a game starts:
```
power up is at (x, y)
```
where `(0,0)` is the North West corner.

The first player moving at this location can then use the power up once
as part of a valid move to bounce, even if no line goes through a visited node/wall.

---

Players can use a power-up by issuing a `power` command:
```
power south
```
which allows a player to move south and play again.

---

## Polarity

The following message can be sent to players at any point during the game:
```
polarity of the goal has been inverted
```

Which means the goals have been inverted. i.e. If you were supposed to score north, your target is now south.

---

<b>This can happen at any time and multiple times each game.</b>

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

## New rink size

The hockey rink is now `15x15` instead of `11x11`.

---

## Evaluation

 - Each AI will play up to 50000 games (or 10 minutes) against every team.
 - You provide source code and an executable package. Make sure you explicitly declare all your dependencies.
 - Your AI won't have access to the internet - sorry DeepMind.
 - Please make sure your code work on ubuntu 16.04 with official packages
 - 10% of the score is given if you provide a bash script "run.sh" that start your AI.
 - 90% of the score is given by your ranking of the round robin.
 
---

### Scoring detailed

score = (team_count-position)/(team_count-1)

Example if there are 30 teams

- position == score 
- 1 == 90% maximum score
- 2 == 87%
- 3 == 84%
- 4 == 81%
- 5 == 78%
- 6 == 74%
- 7 == 71%
