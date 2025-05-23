import re

from find_problems.common import *
from find_problems.find_alphabetical_problems import *
from find_problems.find_bad_documentation import *
from find_problems.find_long_lines import *
from find_problems.find_misc import *


## Asks the user for the new engine version, and
#  changes numbers in the source code to match.
def change_version_numbers():
    new = input('What is the new version (X.Y.Z)? ')
    new_parts = new.split('.')
    source_dir_to_use = get_source_dir_to_use()
    system_call('sed -i "s/VERSION_MAJOR = .*;/VERSION_MAJOR = ' + new_parts[0] + ';/g" ' + source_dir_to_use + '/core/const.h')
    system_call('sed -i "s/VERSION_MINOR = .*;/VERSION_MINOR = ' + new_parts[1] + ';/g" ' + source_dir_to_use + '/core/const.h')
    system_call('sed -i "s/VERSION_REV   = .*;/VERSION_REV   = ' + new_parts[2] + ';/g" ' + source_dir_to_use + '/core/const.h')
    
    rc_files = [source_dir_to_use + '/pikifen.rc', source_dir_to_use + '/../visual_studio_2019/resource.rc']
    for fn in rc_files:
        rc_contents = []
        with open(fn, 'rt', encoding='utf-16') as i:
            for line in i:
                r = re.search('(.*FILEVERSION ).*,.*,.*(,.*)', line)
                if r is not None:
                    line = r.group(1) + new_parts[0] + ',' + new_parts[1] + ',' + new_parts[2] + r.group(2) + '\n'
                r = re.search('(.*PRODUCTVERSION ).*,.*,.*(,.*)', line)
                if r is not None:
                    line = r.group(1) + new_parts[0] + ',' + new_parts[1] + ',' + new_parts[2] + r.group(2) + '\n'
                r = re.search('(.*FileVersion\', \').*\..*\..*(\..*)', line)
                if r is not None:
                    line = r.group(1) + new_parts[0] + '.' + new_parts[1] + '.' + new_parts[2] + r.group(2) + '\n'
                r = re.search('(.*ProductVersion\', \').*\..*\..*(\..*)', line)
                if r is not None:
                    line = r.group(1) + new_parts[0] + '.' + new_parts[1] + '.' + new_parts[2] + r.group(2) + '\n'
                
                rc_contents.append(line)
        with open(fn, 'wt', encoding='utf-16', newline='\r\n') as o:
            for line in rc_contents:
                o.write(line)

    input('Done. Press Enter...')


## Writes any code style problems found in source files.
def write_code_problems():
    problems = []
    problems = problems + get_all_long_line_problems()
    problems = problems + get_all_unordered_problems()
    problems = problems + get_all_cramped_problems()
    problems = problems + get_all_documentation_problems()
    has_problems = False
    problem_file = open('problems.txt', 'w')
    
    problems_by_file = {}
    
    for file, problem, info in problems:
        if not file in problems_by_file:
            problems_by_file[file] = []
        problems_by_file[file].append((problem, info))
    
    for file, dummy in sorted(problems_by_file.items()):
        if file.find('imgui/') != -1: continue
        if file.find('shaders_source.cpp') != -1: continue
        if file.find('code_debug.cpp') != -1: continue
            
        has_problems = True
        print(file)
        problem_file.write(file + '\n')
        for problem, info in problems_by_file[file]:
            problem_str = '  ' + problem + ': ' + info
            print(problem_str)
            problem_file.write(problem_str + '\n')
    
    if not has_problems:
        print('No problems found.')
    
    problem_file.close()
    input('Done. Problems also written to "problems.txt". Press Enter...')


## Writes instances of 'cout' or 'iostream' in source files.
def write_cout_uses():
    source_dir_to_use = get_source_dir_to_use()
    aux = system_call('grep -rnE -B2 -A2 "(cout|iostream)" --include="*.cpp" ' + source_dir_to_use)
    if len(aux) == 0:
        print('No instances of "cout"/"iostream".')
    else:
        print('Instances of "cout"/"iostream":')
        for line in aux.splitlines():
            print('  ' + line)
    
    input('Done. Press Enter...')


## Writes any PNG file that has been added or removed.
#  since the last version.
def write_new_png_files():
    #TODO crush them in this script
    aux = system_call('git diff --name-status $(git describe --abbrev=0 --tags) | grep -E "^[AM].*(png)"')
    if len(aux) == 0:
        print('No altered PNGs.')
    else:
        print('Altered PNGs:')
        for line in aux.splitlines():
            print('  ' + line)
    
    input('Done. Press Enter...')


## Writes any source files that have been added or removed.
#  since the last version.
def write_new_source_files():
    source_dir_to_use = get_source_dir_to_use()
    aux = system_call('git diff --name-status $(git describe --abbrev=0 --tags) | grep ^\[AD\].' + source_dir_to_use)

    if len(aux) == 0:
        print('No new/deleted source files.')
    else:
        print('New/deleted source files:')
        for line in aux.splitlines():
            print('  ' + line)
    
    input('Done. Press Enter...')


## Writes any SVG file that has been added or removed.
#  since the last version.
def write_new_svg_files(prev_version = False):
    if prev_version:
        aux = system_call('git diff --name-status $(git describe --abbrev=0 --tags $(git describe --abbrev=0 --tags)^) | grep -E "^[AM].*svg"')
    else:
        aux = system_call('git diff --name-status $(git describe --abbrev=0 --tags) | grep -E "^[AM].*svg"')
    if len(aux) == 0:
        if not prev_version:
            print('No altered SVGs. Trying previous version...')
            write_new_svg_files(True)
            return
        else:
            print('No altered SVGs.')
    else:
        print('Altered SVGs:')
        for line in aux.splitlines():
            print('  ' + line)
    
    input('Done. Press Enter...')


## Writes any texture files that have been added or removed.
#  since the last version.
def write_new_textures():
    aux = system_call('git diff --name-status $(git describe --abbrev=0 --tags) | grep -E "^[AM].*Textures.*(png|jpg)"')
    if len(aux) == 0:
        print('No new textures.')
    else:
        print('New textures:')
        for line in aux.splitlines():
            print('  ' + line)
    
    input('Done. Press Enter...')


## Main function.
if __name__ == '__main__':
    debug_mode = get_argument_idx('debug') != -1
    source_dir_to_use = get_source_dir_to_use()
    
    if not debug_mode:
        try:
            ls = system_call('ls game_data')
            if len(ls) == 0:
                print('The current directory is not Pikifen\'s directory!')
                exit(1)
        except Exception:
            print('The current directory is not Pikifen\'s directory!')
            exit(1)
    
    try:
        ls = system_call('ls ' + source_dir_to_use)
        if len(ls) == 0:
            print('The source folder ("' + source_dir_to_use + '") does not exist!')
            exit(1)
    except Exception:
        print('The source folder ("' + source_dir_to_use + '") does not exist!')
        exit(1)
    
    running = True
    while running:

        if not debug_mode:
            latest = system_call('git describe --abbrev=0 --tags')
        
            aux = system_call('grep -oP "VERSION_MAJOR = .*;" source/source/core/const.h')
            cur_major = aux[16:-1]
            aux = system_call('grep -oP "VERSION_MINOR = .*;" source/source/core/const.h')
            cur_minor = aux[16:-1]
            aux = system_call('grep -oP "VERSION_REV   = .*;" source/source/core/const.h')
            cur_rev = aux[16:-1]
            current = cur_major + '.' + cur_minor + '.' + cur_rev
            
            print('Pikifen code helper   (Pikifen code: ' + current + ', last tagged: ' + latest + ')')
        else:
            print('Pikifen code helper   (DEBUG MODE)')
        
        print('1. Change engine version numbers in const.h/resource files')
        print('2. Get new/deleted source files (for Visual Studio)')
        print('3. Get new textures')
        print('4. Get altered PNG files')
        print('5. Get altered SVG files')
        print('6. Find problems in code')
        print('Q. Quit')
        option = input()
        
        if option == 'q' or option == 'Q':
            running = False
            break
        elif option == '1':
            change_version_numbers()
        elif option == '2':
            write_new_source_files()
        elif option == '3':
            write_new_textures()
        elif option == '4':
            write_new_png_files()
        elif option == '5':
            write_new_svg_files()
        elif option == '6':
            write_code_problems()
        
        print('')

