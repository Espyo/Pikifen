import os

from .common import *

CRAMPED_THING_INCLUDE = 0
CRAMPED_THING_USING = 1
CRAMPED_THING_CLOSE_BRACE = 2

def check_cramped_things_in_file(file_path):
    file = open(file_path)
    line_nr = 0
    problems = []
    found_thing = False
    thing = CRAMPED_THING_INCLUDE
    empty_lines = 0

    for l in file:
        line_nr = line_nr + 1
        
        if not found_thing:
            if l.startswith('}'):
                found_thing = True
                empty_lines = 0
                thing = CRAMPED_THING_CLOSE_BRACE
            elif l.startswith('#include'):
                found_thing = True
                empty_lines = 0
                thing = CRAMPED_THING_INCLUDE
            elif l.startswith('using'):
                found_thing = True
                empty_lines = 0
                thing = CRAMPED_THING_USING
        
        else:
            if l == '\n':
                empty_lines = empty_lines + 1
            else:
                found_thing = False
                ignore = False

                if thing == CRAMPED_THING_INCLUDE and l.startswith('#include'):
                    ignore = True
                if thing == CRAMPED_THING_USING and l.startswith('using'):
                    ignore = True

                if empty_lines < 2 and not ignore:
                    thing_name = ''
                    if thing == CRAMPED_THING_INCLUDE:
                        thing_name = 'include'
                    elif thing == CRAMPED_THING_USING:
                        thing_name = 'using'
                    elif thing == CRAMPED_THING_CLOSE_BRACE:
                        thing_name = 'close brace'
                    problems.append((file_path, 'Two newlines missing after ' + thing_name, pad(line_nr - empty_lines - 1, 4)))

    return problems


def find_cramped_things():
    problems = []
    for dirpath, dirnames, files in os.walk('Source/source'):
        for f in files:
            if f.endswith('cpp') or f.endswith('h'):
                file_problems = check_cramped_things_in_file(os.path.join(dirpath, f))
                if file_problems is not None:
                    problems = problems + file_problems
    return problems
