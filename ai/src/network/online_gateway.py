import time

from twisted.internet import task
import json

MESSAGES = {
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
    'action': "{} did go {}",
}


class OnlineGateway(object):
    looping_call = None
    msgid = 0

    def __init__(self, controller_factory, timeout, debug):
        self.timeout = timeout
        self.controller_factory = controller_factory
        self.debug = debug
        self._initialize_controller()

    def _initialize_controller(self):
        self.handlers = []
        self.last_time_played = {}
        self.state = "on"
        self.controller = self.controller_factory()
        self.looping_call = task.LoopingCall(self.is_active_player_timeout)
        self.looping_call.start(1.0)

    def register_online(self, player_name, handler):
        self.controller.register(player_name)
        self.handlers.append(handler)
        if len(self.handlers) == 2:
            self._starting_game()
            self.last_time_played[self.controller.active_player] = time.time()

    def move_player(self, name, action):
        if self.controller.active_player_name() is name:
            action_result = self.controller.move(action)
            if action_result.valid:
                if action_result.terminated:
                    reason = MESSAGES['won'].format(action_result.winner, action_result.reason, json.dumps(self.controller.players))
                    self._game_id_ended(reason)
                else:
                    self.last_time_played[self.controller.active_player] = time.time()
                    self._inform_players(MESSAGES['action'].format(name, action))
                    self._inform_active_player_turn()
            else:
                self._inform_active_players(MESSAGES['invalid'])
        else:
            self._inform_inactive_players(MESSAGES['ignoring_inactive'].format(action))

    def _inform_players(self, message):
        self._inform_active_players(message)
        self._inform_inactive_players(message)

    def _inform_active_players(self, message):
        self._ship_it(self.controller.active_player, message)

    def _inform_inactive_players(self, message):
        id = (self.controller.active_player + 1) % 2
        self._ship_it(id, message)

    def _ship_it(self, player_id, message):
        self.msgid += 1
        if 'invalid' not in message and player_id!=0:
            if self.debug:
                print(self.msgid)
        self.handlers[player_id].send_message('{} - {}'.format(message, self.msgid))

    def is_active_player_timeout(self):
        if self.state is "ended":
            self.looping_call.stop()
            self._initialize_controller()
        if self.last_time_played:
            start_time = self.last_time_played[self.controller.active_player]
            now = time.time()
            delta = now - start_time
            if delta > self.timeout:
                reason = MESSAGES['timeout'].format(self.controller.in_active_player_name(), json.dumps(self.controller.players))
                self._game_id_ended(reason)
                self.looping_call.stop()
                self._initialize_controller()

    def _game_id_ended(self, m):
        self._inform_active_players(m)
        self._inform_inactive_players(m)
        print(m)
        self.state = "ended"
        self.handlers[0].end_game()
        self.handlers[1].end_game()

    def _inform_active_player_turn(self):
        self._inform_active_players(MESSAGES['active_player'].format(self.controller.active_player_name()))

    def _starting_game(self):
        self._inform_players(MESSAGES['game_on'])
        self._inform_players(MESSAGES['ball_at'].format(self.controller.ball))
        self._inform_active_players(MESSAGES['goal_north'])
        self._inform_inactive_players(MESSAGES['goal_south'])
        self._inform_active_player_turn()
