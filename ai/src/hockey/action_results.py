class ActionResults(object):
    def __init__(self, active_player, valid=True, terminated=False, winner=None, reason=None):
        self.active_player = active_player
        self.valid = valid
        self.terminated = terminated
        self.winner = winner
        self.reason = reason

    def __eq__(self, other):
        return self.terminated == other.terminated and self.active_player == other.active_player and self.winner == other.winner

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        return '{} played, game state is {}'.format(self.active_player, 'terminated' if self.terminated else 'on')
