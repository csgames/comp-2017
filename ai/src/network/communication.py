from twisted.protocols.basic import LineReceiver

from network.game_on import GameOn
from network.get_name import GetName


class Communication(LineReceiver, object):
    communication_handler = []
    users = {}
    state = 'get_name'

    def __init__(self, users, online_gateway):
        self.users = users
        self.online_gateway = online_gateway
        self.communication_handler = self.get_communication_handler(online_gateway)

    def get_communication_handler(self, online_gateway):
        return {
            'get_name': GetName(self),
            'game_on': GameOn(self, online_gateway)
        }

    def sendLine(self, line):
        super(Communication, self).sendLine(line.encode('UTF-8'))

    def lineReceived(self, line):
        line = line.decode('UTF-8')
        self.communication_handler[self.state].lineReceived(line)

    def connectionMade(self):
        self.communication_handler[self.state].connectionMade()

    def connectionLost(self, reason):
        index = [i for i in self.users if self.users[i] is self.name][0]
        del self.users[index]

    def _game_on(self):
        self.state = 'game_on'

    def _register(self, name):
        self.name = name
        self.users[len(self.users)] = name
        self.online_gateway.register_online(name, self.communication_handler[self.state])
        self.communication_handler[self.state].name = name
