from twisted.internet import reactor
from twisted.internet.protocol import Factory

from hockey2.controller_polarity import ControllerPolarity
from network2.online_gateway_polarity import OnlineGatewayPolarity
from network2.communication import CommunicationP2


class ChatFactory(Factory):
    def __init__(self):
        self.users = {}
        self.online_gateway = OnlineGatewayPolarity(lambda: ControllerPolarity(15, 15), timeout=2, debug=True)

    def buildProtocol(self, addr):
        return CommunicationP2(self.users, self.online_gateway)


cf = ChatFactory()
reactor.listenTCP(8023, cf)
reactor.run()
