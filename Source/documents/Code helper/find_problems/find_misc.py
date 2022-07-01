import os

from .common import *

# Cramped thing type -- an include statement.
CRAMPED_THING_INCLUDE = 0
# Cramped thing type -- a using statement.
CRAMPED_THING_USING = 1
# Cramped thing type -- a closing brace.
CRAMPED_THING_CLOSE_BRACE = 2


## Returns any cramped things in all source files.
#  @return Cramped things as problems.
def get_all_cramped_problems():
    problems = []
    for dirpath, dirnames, files in os.walk(source_dir_to_use):
        for f in files:
            if f.endswith('cpp') or f.endswith('h'):
                file_problems = get_cramped_problems_in_file(os.path.join(dirpath, f))
                if file_problems is not None:
                    problems = problems + file_problems
    return problems


## Returns any cramped things in a source file.
#  @param file_path Path to the file.
#  @return Cramped things as problems.
def get_cramped_problems_in_file(file_path):
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
