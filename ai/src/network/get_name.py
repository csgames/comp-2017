from network.online_gateway import MESSAGES


class GetName(object):
    def __init__(self, communication):
        self.communication = communication

    def connectionMade(self):
        self.communication.sendLine(MESSAGES['who'])

    def lineReceived(self, name):
        if name in self.communication.users.values():
            self.communication.sendLine(MESSAGES['name_taken'])
            return
        self.communication.sendLine(MESSAGES['welcome'].format(name, len(self.communication.users)))
        self.communication._game_on()
        self.communication._register(name)
