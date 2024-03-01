import os, re

from .common import *

## Returns any documentation-related problems found in all source files.
#  @return Documentation-related problems.
def get_all_documentation_problems():
    problems = []
    source_dir_to_use = get_source_dir_to_use()
    for dirpath, dirnames, files in os.walk(source_dir_to_use):
        for f in files:
            if f.endswith('cpp'):
                file_problems = get_documentation_problems_in_file(os.path.join(dirpath, f))
                if file_problems is not None:
                    problems = problems + file_problems
    return problems


## Returns any documentation-related problems found in a source file.
#  @param file_path Path to the file.
#  @return Documentation-related problems.
def get_documentation_problems_in_file(file_path):
    file = open(file_path)
    lines = file.readlines()
    functions = get_functions_in_file(file_path)
    problems = []

    for f in functions:

        finding_comment = True
        cur_line = f.line
        comment_start = 0

        while finding_comment:
            if cur_line == 1:
                # Reached the top of the file.
                finding_comment = False
                break
            else:
                cur_line = cur_line - 1
                if lines[cur_line - 1].startswith('/**'):
                    # Found the start of the comment!
                    comment_start = cur_line
                    finding_comment = False
                elif lines[cur_line] == '\n':
                    # Something new is starting. Couldn't find comment.
                    finding_comment = False

        if comment_start == 0:
            problems.append((file_path, 'Function without documentation', pad(f.line, 4)))
            continue

        comment_lines = lines[comment_start:f.line - 1]

        params_documented = []
        for l in comment_lines:
            match = re.search(r'\* @param ([^ \.]+)', l)
            if match is not None:
                params_documented.append(match.group(1))
        
        for p in f.params:
            if p not in params_documented:
                problems.append((file_path, 'Undocumented parameter', pad(f.line, 4) + ' ' + pad(p, 40)))
        
        for p in params_documented:
            if p not in f.params:
                problems.append((file_path, 'Unknown documented parameter', pad(comment_start, 4) + ' ' + pad(p, 20)))
    
    return problems
