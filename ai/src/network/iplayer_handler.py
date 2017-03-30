import abc

ABC = abc.ABCMeta('ABC', (object,), {})


class IPlayerHandler(ABC):
    @abc.abstractmethod
    def send_message(self, message):
        pass

    @abc.abstractmethod
    def end_game(self):
        pass
