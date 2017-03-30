from network.communication import Communication
from network.game_on import GameOn
from network.get_name import GetName
from hockey.action import Action
from network.iplayer_handler import IPlayerHandler

POSSIBILITIES = list(Action.move.keys()) + ['power {}'.format(i) for i in Action.move.keys()]

class PowerGameOn(GameOn):
    name = None

    def lineReceived(self, line):
        line = line.lower()
        if line in POSSIBILITIES:
            self.online_gateway.move_player(self.name, line)
        else:
            self.handler.sendLine('Invalid action')


class GameOn(IPlayerHandler):
    name = None

    def __init__(self, handler, online_gateway):
        self.handler = handler
        self.online_gateway = online_gateway

    def lineReceived(self, line):
        line = line.lower()
        if line in POSSIBILITIES:
            self.online_gateway.move_player(self.name, line)
        else:
            self.handler.sendLine('Invalid action')

    def send_message(self, message):
        self.handler.sendLine(message)

    def end_game(self):
        self.handler.transport.loseConnection()


class CommunicationP2(Communication):
    def get_communication_handler(self, online_gateway):
        return {
            'get_name': GetName(self),
            'game_on': PowerGameOn(self, online_gateway)
        }
