import random
from unittest import TestCase

from src.hockey.action import Action
from src.hockey.action_results import ActionResults
from src.hockey.controller import ControllerGentle

from test.controller_test import BOB
from test.controller_test import MALORY


class BoardPrinterTest(TestCase):
    def testPrint(self):
        self.controller = ControllerGentle(11, 11)
        self.controller.register(BOB)
        self.controller.register(MALORY)

        still_on = ActionResults(BOB, False), ActionResults(MALORY, False)
        result = ActionResults(BOB, False)
        while result in still_on:
            result = self.controller.move(Action.from_number(random.randint(0, 7)))
