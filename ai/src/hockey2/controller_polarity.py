import random

from hockey.board_builder import BoardBuilder
from hockey.controller import ControllerGentle, NoRuleEnforcerFound, ApplyLegalMoveGently, GameTerminated


class ApplyPowerLegalMoveGently(ApplyLegalMoveGently):
    def __init__(self, controller, next_rule):
        super(ApplyPowerLegalMoveGently, self).__init__(controller, next_rule)

    def apply_rule(self, action):
        initial_active = self.controller.active_player
        result = super(ApplyPowerLegalMoveGently, self).apply_rule(action)
        if self.controller.ball == self.controller.power_up_position:
            self.controller.power_up = initial_active
        return result


class ControllerPolarity(ControllerGentle):
    power_up = None

    def __init__(self, size_x=11, size_y=11, builder=BoardBuilder):
        super(ControllerPolarity, self).__init__(size_x, size_y, builder)
        self.power_up_position = self.random_position()

    def move(self, action):
        power_up = True if "power" in action and self.power_up == self.active_player else False
        if "power" in action:
            action = action.split(' ')[1]
        initial_active = self.active_player
        result = super(ControllerPolarity, self).move(action)
        polarityInverted = False
        if result.valid:
            polarityInverted = random.randint(0, 10) == 9
            if polarityInverted:
                self.inverse_polarity()
        if power_up:
            self.power_up = None
            self.active_player = initial_active

        return result, polarityInverted

    def inverse_polarity(self):
        self.goal_by_player = self.goal_by_player[::-1]

    def rule(self):
        next_rule = NoRuleEnforcerFound(self, None)
        next_rule = ApplyPowerLegalMoveGently(self, next_rule)
        next_rule = GameTerminated(self, next_rule)
        return next_rule

    def random_position(self):
        position = self.ball
        while position in [self.ball, (self.ball[0] + 1, self.ball[1]), (self.ball[0] - 1, self.ball[1]),
                           (self.ball[0], self.ball[1] - 1), (self.ball[0] - 1, self.ball[1] + 1)]:
            position = random.randint(1, self.size_x - 1), random.randint(1, self.size_y - 1)
        return position
