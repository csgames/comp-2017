import os
import uuid
from time import gmtime
from time import strftime

from PIL import Image
from PIL import ImageDraw

from hockey.action import Action

GREEN = (0, 255, 0)
RED = (255, 0, 0)
BLACK = (0, 0, 0)


class BoardPrinter(object):
    offset = 20

    def print_game(self, controller, size=(800, 600)):
        im = self._draw_initial_board(controller)
        draw = ImageDraw.Draw(im)

        actions = controller.actions
        draw.text((15, 0), controller.players[0], GREEN)
        draw.text((15, im.height-10), controller.players[1], RED)
        image_list = [im]

        for source, player, action in actions:
            source = tuple([i + self.offset for i in self._in_game_position_to_img(source)])
            destination = self._get_destination_from_action(action, source)
            color = self._color(player)
            draw.line((source, destination), color)
            image_list.append(im.resize(size).copy())

        image_list[-1].resize(size).save(self._get_filename_png(controller.players))

    def print_gif(self, controller, size=(800, 600)):
        im = self._draw_initial_board(controller)
        draw = ImageDraw.Draw(im)
        actions = controller.actions
        image_list = []
        draw.text((15, 0), controller.players[0], GREEN)
        draw.text((15, im.height-10), controller.players[1], RED)

        for source, player, action in actions:
            source = tuple([i + self.offset for i in self._in_game_position_to_img(source)])
            destination = self._get_destination_from_action(action, source)
            color = self._color(player)
            draw.line((source, destination), color)
            image_list.append(im.resize(size).copy())

        im.resize(size).save(self._get_filename_gif(controller.players), save_all=True,
                             append_images=image_list,
                             optimize=False,
                             loop=0)

    def _get_filename_png(self, players):
        dir_path = os.path.dirname(os.path.realpath(__file__))
        filename = "{}/../../test/output/test-{}-{}VS{}-{}.png".format(dir_path,
                                                                       strftime("%Y-%m-%d_%Hh%Mm%S", gmtime()),
                                                                       players[0], players[1], uuid.uuid4())
        return filename

    def _get_filename_gif(self, players):
        dir_path = os.path.dirname(os.path.realpath(__file__))
        filename = "{}/../../test/output/test-{}-{}VS{}-{}.gif".format(dir_path,
                                                                       strftime("%Y-%m-%d_%Hh%Mm%S", gmtime()),
                                                                       players[0], players[1], uuid.uuid4())
        return filename

    def _draw_initial_board(self, controller):
        dots = controller.initial_dots
        width = (len(dots) + 3) * 10
        height = (max([len(dots[x]) for x in range(len(dots))]) + 1) * 10
        im = Image.new("RGB", (width, height), (255, 255, 255))
        draw = ImageDraw.Draw(im)
        self._draw_outline(dots, draw)
        self._draw_cardinal(draw, height, width)
        return im

    def _draw_cardinal(self, draw, height, width):
        draw.text((0, 0), 'NW', BLACK)
        draw.text((0, height - 10), 'SW', BLACK)
        draw.text((width - 12, 0), 'NE', BLACK)
        draw.text((width - 12, height - 10), 'SE', BLACK)

    def _in_game_position_to_img(self, position):
        return position[0] * 10, position[1] * 10

    def _draw_outline(self, dots, draw):
        for x in sorted(dots.keys()):
            for y in sorted(dots[x].keys()):
                actions = dots[x][y]['actions']
                keys = sorted(actions.keys())
                for key in keys:
                    action = key
                    value = actions[key]
                    if value is True:
                        source = tuple([i + self.offset for i in self._in_game_position_to_img((x, y))])
                        destination = self._get_destination_from_action(action, source)
                        draw.line((source, destination), BLACK)

    def _get_destination_from_action(self, action, source):
        move = list(map(lambda x: x * 10, Action.move[action]))
        destination = source[0] + move[0], source[1] + move[1]
        return destination

    def _color(self, player):
        return RED if player == 1 else GREEN


class BoardPrinterCurrent(BoardPrinter):
    def _get_filename_png(self, players):
        dir_path = os.path.dirname(os.path.realpath(__file__))
        filename = "{}/../../test/output/test-{}-{}VS{}.png".format(dir_path, 'current', players[0], players[1])
        return filename

    def _get_filename_gif(self, players):
        dir_path = os.path.dirname(os.path.realpath(__file__))
        filename = "{}/../../test/output/test-{}-{}VS{}-{}.gif".format(dir_path, 'current', players[0], players[1], uuid.uuid4())
        return filename
