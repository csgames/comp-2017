class Action(object):
    @staticmethod
    def from_number(number):
        return Action.Name[number]

    @staticmethod
    def to_number(action):
        return list(Action.Name.values()).index(action)

    NORTH = 'north'
    NORTH_EAST = 'north east'
    EAST = 'east'
    SOUTH_EAST = 'south east'
    SOUTH = 'south'
    SOUTH_WEST = 'south west'
    WEST = 'west'
    NORTH_WEST = 'north west'
    move = {
        NORTH: (0, -1),
        NORTH_EAST: (1, -1),
        EAST: (1, 0),
        SOUTH_EAST: (1, 1),
        SOUTH: (0, 1),
        SOUTH_WEST: (-1, 1),
        WEST: (-1, 0),
        NORTH_WEST: (-1, -1),
    }
    Name = {
        0: NORTH,
        1: NORTH_EAST,
        2: EAST,
        3: SOUTH_EAST,
        4: SOUTH,
        5: SOUTH_WEST,
        6: WEST,
        7: NORTH_WEST,
    }
