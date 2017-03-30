from hockey.action import Action
from network.iplayer_handler import IPlayerHandler

POSSIBILITIES = list(Action.move.keys())


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
