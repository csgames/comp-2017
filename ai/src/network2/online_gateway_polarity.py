import time

from network.online_gateway import OnlineGateway
import json

MESSAGES = {
    'power_up': 'power up is at {}',
    'who': "What's your name?",
    'name_taken': "Name taken, please choose another.",
    'welcome': 'Welcome, {} you\'re player {}!',
    'game_on': 'Game is on',
    'ball_at': "ball is at {}",
    'goal_north': "your goal is north",
    'goal_south': "your goal is south",
    'active_player': '{} is active player',
    'timeout': '{} won : timeout |||{}',
    'won': '{} won {} |||{}',
    'invalid': 'invalid move',
    'ignoring_inactive': 'ignoring action {}',
    'action': '{} did go {}',
    'inverted': 'polarity of the goal has been inverted',
}


class OnlineGatewayPolarity(OnlineGateway):
    looping_call = None
    msgid = 0

    def move_player(self, name, action):
        if self.controller.active_player_name() is name:
            action_result, inverted = self.controller.move(action)
            if action_result.valid:
                if action_result.terminated:
                    reason = MESSAGES['won'].format(action_result.winner, action_result.reason, json.dumps(self.controller.players))
                    self._game_id_ended(reason)
                else:
                    self.last_time_played[self.controller.active_player] = time.time()
                    if inverted:
                        self._inform_players(MESSAGES['inverted'])
                    self._inform_players(MESSAGES['action'].format(name, action))
                    self._inform_active_player_turn()
            else:
                self._inform_active_players(MESSAGES['invalid'])
        else:
            self._inform_inactive_players(MESSAGES['ignoring_inactive'].format(action))

    def _starting_game(self):
        self._inform_players(MESSAGES['game_on'])
        self._inform_players(MESSAGES['ball_at'].format(self.controller.ball))
        self._inform_active_players(MESSAGES['goal_north'])
        self._inform_inactive_players(MESSAGES['goal_south'])
        self._inform_active_player_turn()
        self._inform_players(MESSAGES['power_up'].format(self.controller.power_up_position))
