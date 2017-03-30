import copy
import math

from hockey.action import Action
from hockey.action_results import ActionResults
from hockey.board_builder import BoardBuilder
from hockey.board_printer import BoardPrinterCurrent


class RuleEnforcer(object):
    controller = None
    next_rule = None

    def __init__(self, controller, next_rule):
        self.controller = controller
        self.next_rule = next_rule

    def process(self, action):
        action_result = self.apply_rule(action)
        if action_result is None:
            return self.next_rule.process(action)
        return action_result

    def apply_rule(self, action):
        pass

    def _get_ball_destination(self, action):
        ball_x, ball_y = self.controller.ball
        x_to, y_to = Action.move[action]
        x = x_to + ball_x
        y = y_to + ball_y
        return x, y

    def _out_of_bound_move(self, action):
        x, y = self._get_ball_destination(action)
        return x not in self.controller.dots or y not in self.controller.dots[x]

    def _illegal_move(self, action):
        ball_x, ball_y = self.controller.ball
        return self.controller.dots[ball_x][ball_y]['actions'][action]


class NoRuleEnforcerFound(RuleEnforcer):
    def apply_rule(self, action):
        raise Exception('No Rule matching move!')


class GameTerminated(RuleEnforcer):
    def apply_rule(self, action):
        if self.controller.terminated:
            return ActionResults(self.controller.active_player_name(), terminated=True)


class OutOfBound(RuleEnforcer):
    def apply_rule(self, action):
        if self._out_of_bound_move(action):
            return ActionResults(self.controller.active_player_name(), terminated=True)


class IllegalMove(RuleEnforcer):
    def apply_rule(self, action):
        if self._illegal_move(action):
            return ActionResults(self.controller.active_player_name(), terminated=True)


class ApplyLegalMove(RuleEnforcer):
    def apply_rule(self, action):
        self.controller.actions.append((self.controller.ball, self.controller.active_player, action))
        ball_x, ball_y = self.controller.ball
        x, y = self._get_ball_destination(action)
        if not self.controller.dots[x][y]['bounce']:
            self.controller._switch_player()

        self.controller.ball = (x, y)

        self.controller.dots[ball_x][ball_y]['actions'][action] = True
        self.controller.dots[x][y]['actions'][self._opposite_action(action)] = True

        self.controller.dots[x][y]['bounce'] = True
        if self.controller.dots[x][y]['is_goal']:
            result = ActionResults(self.controller.active_player_name(), terminated=True, reason="a goal was made")
        else:
            result = ActionResults(self.controller.active_player_name(), terminated=False)
            if len(self.controller.get_possible_actions(x, y)) == 0:
                self.controller._switch_player()
                result = ActionResults(self.controller.active_player_name(), terminated=True,
                                       reason="checkmate was achieved")
        return result

    def _opposite_action(self, action):
        return Action.from_number((Action.to_number(action) + 4) % 8)


class ApplyLegalMoveGently(ApplyLegalMove):
    def apply_rule(self, action):
        if self._illegal_move(action) or self._out_of_bound_move(action):
            return ActionResults(self.controller.active_player_name(), valid=False)
        return super(ApplyLegalMoveGently, self).apply_rule(action)


class Controller(object):
    def __init__(self, size_x=11, size_y=11, builder=BoardBuilder, printer=BoardPrinterCurrent):
        self.actions = []
        self.ball = int(round(math.ceil(size_x / 2.0) - 1, 0)), int(round(math.ceil(size_y / 2.0) - 1, 0))
        self.size_x = size_x
        self.size_y = size_y
        self.goal_by_player = (-1, self.size_y)
        self.dots = builder.init(self.size_x, self.size_y)
        self.initial_dots = copy.deepcopy(self.dots)
        self.dots[self.ball[0]][self.ball[1]]['bounce'] = True
        self.players = []
        self.active_player = 0
        self.terminated = False
        self.printer = printer()

        next_rule = self.rule()

        self.rule_chain = next_rule

    def active_player_name(self):
        return self.players[self.active_player]

    def in_active_player_name(self):
        return self.players[(self.active_player + 1) % 2]

    def rule(self):
        next_rule = NoRuleEnforcerFound(self, None)
        next_rule = ApplyLegalMove(self, next_rule)
        next_rule = IllegalMove(self, next_rule)
        next_rule = OutOfBound(self, next_rule)
        next_rule = GameTerminated(self, next_rule)
        return next_rule

    def move(self, action):
        id = (self.active_player + 1) % 2
        action_result = self.rule_chain.process(action)
        if action_result.terminated:
            self.printer.print_gif(self)
            if self.ball[1] == self.goal_by_player[0]:
                action_result.winner = self.players[0]
            elif self.ball[1] == self.goal_by_player[1]:
                action_result.winner = self.players[1]
            elif len(self.get_possible_actions(self.ball[0], self.ball[1])) == 0:
                action_result.winner = self.players[id]
        return action_result

    def _switch_player(self):
        self.active_player += 1
        self.active_player %= 2

    def register(self, player_name):
        self.players.append(player_name)

    def get_possible_actions(self, x, y):
        place = self.dots[x][y]['actions']

        return [action for action in place if not place[action]]


class ControllerGentle(Controller):
    def rule(self):
        next_rule = NoRuleEnforcerFound(self, None)
        next_rule = ApplyLegalMoveGently(self, next_rule)
        next_rule = GameTerminated(self, next_rule)
        return next_rule


if __name__ == '__main__':
    c = Controller()
