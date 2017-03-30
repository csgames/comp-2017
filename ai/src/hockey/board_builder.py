from copy import copy

from hockey.action import Action


class BoardBuilder(object):
    @staticmethod
    def init(size_x, size_y):
        dots = {}
        for x in range(0, size_x):
            if x not in dots:
                dots[x] = {}
            for y in range(0, size_y):
                bounce = False
                if x == 0 or x == size_x - 1 or y == 0 or y == size_y - 1:
                    bounce = True

                actions = BoardBuilder.initial_actions()

                if y == 0:
                    actions[Action.NORTH] = True
                    actions[Action.NORTH_WEST] = True
                    actions[Action.NORTH_EAST] = True
                    actions[Action.EAST] = True
                    actions[Action.WEST] = True
                elif y == size_y - 1:
                    actions[Action.SOUTH] = True
                    actions[Action.SOUTH_WEST] = True
                    actions[Action.SOUTH_EAST] = True
                    actions[Action.EAST] = True
                    actions[Action.WEST] = True

                if x == 0:
                    actions[Action.WEST] = True
                    actions[Action.SOUTH_WEST] = True
                    actions[Action.NORTH_WEST] = True
                    actions[Action.NORTH] = True
                    actions[Action.SOUTH] = True
                elif x == size_x - 1:
                    actions[Action.SOUTH] = True
                    actions[Action.NORTH] = True
                    actions[Action.SOUTH_EAST] = True
                    actions[Action.NORTH_EAST] = True
                    actions[Action.EAST] = True

                dots[x][y] = {
                    'actions': actions,
                    'bounce': bounce,
                    'is_goal': False
                }

        BoardBuilder._handle_basic_goal(dots, size_x, size_y)

        return dots

    @staticmethod
    def initial_actions():
        return {
            Action.NORTH: False,
            Action.NORTH_EAST: False,
            Action.EAST: False,
            Action.SOUTH_EAST: False,
            Action.SOUTH: False,
            Action.SOUTH_WEST: False,
            Action.WEST: False,
            Action.NORTH_WEST: False,
        }

    @staticmethod
    def _handle_basic_goal(dots, size_x, size_y):
        goal_x = int(round(size_x / 2.0) - 1)

        dots[goal_x - 1][0]['actions'][Action.NORTH_EAST] = False
        dots[goal_x - 1][0]['actions'][Action.EAST] = False
        dots[goal_x + 1][0]['actions'][Action.NORTH_WEST] = False
        dots[goal_x + 1][0]['actions'][Action.WEST] = False
        dots[goal_x - 1][size_y - 1]['actions'][Action.SOUTH_EAST] = False
        dots[goal_x - 1][size_y - 1]['actions'][Action.EAST] = False
        dots[goal_x + 1][size_y - 1]['actions'][Action.SOUTH_WEST] = False
        dots[goal_x + 1][size_y - 1]['actions'][Action.WEST] = False

        dots[goal_x][0]['actions'][Action.NORTH_EAST] = False
        dots[goal_x][0]['actions'][Action.NORTH] = False
        dots[goal_x][0]['actions'][Action.NORTH_WEST] = False
        dots[goal_x][0]['actions'][Action.EAST] = False
        dots[goal_x][0]['actions'][Action.WEST] = False
        dots[goal_x][size_y - 1]['actions'][Action.SOUTH_WEST] = False
        dots[goal_x][size_y - 1]['actions'][Action.SOUTH] = False
        dots[goal_x][size_y - 1]['actions'][Action.SOUTH_EAST] = False
        dots[goal_x][size_y - 1]['actions'][Action.WEST] = False
        dots[goal_x][size_y - 1]['actions'][Action.EAST] = False

        dots[goal_x][-1] = {'actions': BoardBuilder.initial_actions(), 'is_goal': True, 'bounce': False}
        dots[goal_x][-1]['actions'][Action.EAST] = True
        dots[goal_x][-1]['actions'][Action.WEST] = True
        dots[goal_x][-1]['actions'][Action.NORTH] = True
        dots[goal_x][-1]['actions'][Action.NORTH_EAST] = True
        dots[goal_x][-1]['actions'][Action.NORTH_WEST] = True
        dots[goal_x - 1][-1] = {'actions': copy(dots[0][0]['actions']), 'is_goal': True, 'bounce': False}
        dots[goal_x + 1][-1] = {'actions': copy(dots[size_x - 1][0]['actions']), 'is_goal': True, 'bounce': False}

        dots[goal_x][size_y] = {'actions': BoardBuilder.initial_actions(), 'is_goal': True, 'bounce': False}
        dots[goal_x][size_y]['actions'][Action.SOUTH_WEST] = True
        dots[goal_x][size_y]['actions'][Action.SOUTH] = True
        dots[goal_x][size_y]['actions'][Action.SOUTH_EAST] = True
        dots[goal_x][size_y]['actions'][Action.WEST] = True
        dots[goal_x][size_y]['actions'][Action.EAST] = True
        dots[goal_x - 1][size_y] = {'actions': copy(dots[0][size_y - 1]['actions']), 'is_goal': True, 'bounce': False}
        dots[goal_x + 1][size_y] = {'actions': copy(dots[size_x - 1][size_y - 1]['actions']), 'is_goal': True,
                                    'bounce': False}

        dots[goal_x][0]['bounce'] = False
        dots[goal_x][size_y - 1]['bounce'] = False
