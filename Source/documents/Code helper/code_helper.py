import re

from find_problems.common import *
from find_problems.find_alphabetical_problems import *
from find_problems.find_bad_documentation import *
from find_problems.find_long_lines import *
from find_problems.find_misc import *

def change_version_numbers():
    new = input("What is the new version? ")
    new_parts = new.split(".")
    system_call("sed -i 's/VERSION_MAJOR = .*;/VERSION_MAJOR = " + new_parts[0] + ";/g' Source/source/const.h")
    system_call("sed -i 's/VERSION_MINOR = .*;/VERSION_MINOR = " + new_parts[1] + ";/g' Source/source/const.h")
    system_call("sed -i 's/VERSION_REV   = .*;/VERSION_REV   = " + new_parts[2] + ";/g' Source/source/const.h")
    
    rc_files = ["Source/source/Pikifen.rc", "Source/visual_studio_2019/resource.rc"]
    for fn in rc_files:
        rc_contents = []
        with open(fn, "rt", encoding="utf-16") as i:
            for line in i:
                r = re.search("(.*FILEVERSION ).*,.*,.*(,.*)", line)
                if r is not None:
                    line = r.group(1) + new_parts[0] + "," + new_parts[1] + "," + new_parts[2] + r.group(2) + "\n"
                r = re.search("(.*PRODUCTVERSION ).*,.*,.*(,.*)", line)
                if r is not None:
                    line = r.group(1) + new_parts[0] + "," + new_parts[1] + "," + new_parts[2] + r.group(2) + "\n"
                r = re.search("(.*FileVersion\", \").*\..*\..*(\..*)", line)
                if r is not None:
                    line = r.group(1) + new_parts[0] + "." + new_parts[1] + "." + new_parts[2] + r.group(2) + "\n"
                r = re.search("(.*ProductVersion\", \").*\..*\..*(\..*)", line)
                if r is not None:
                    line = r.group(1) + new_parts[0] + "." + new_parts[1] + "." + new_parts[2] + r.group(2) + "\n"
                
                rc_contents.append(line)
        with open(fn, "wt", encoding="utf-16", newline="\r\n") as o:
            for line in rc_contents:
                o.write(line)

    input("Done. Don't forget the area versions. Press Enter...")


def get_new_source_files():
    aux = system_call("git diff --name-status $(git describe --abbrev=0 --tags) | grep ^\[AD\].Source/source")

    if len(aux) == 0:
        print("No new/deleted source files.")
    else:
        print("New/deleted source files:")
        for line in aux.splitlines():
            print("  " + line)
    
    input("Done. Press Enter...")


def get_cout():
    aux = system_call("grep -rnE -B2 -A2 '(cout|iostream)' --include='*.cpp' Source")
    if len(aux) == 0:
        print("No instances of \"cout\"/\"iostream\"")
    else:
        print("Instances of \"cout\"/\"iostream\":")
        for line in aux.splitlines():
            print("  " + line)
    
    input("Done. Press Enter...")


def get_new_textures():
    aux = system_call("git diff --name-status $(git describe --abbrev=0 --tags) | grep -E '^[AM].*Textures.*(png|jpg)'")
    if len(aux) == 0:
        print("No new textures.")
    else:
        print("New textures:")
        for line in aux.splitlines():
            print("  " + line)
    
    input("Done. Press Enter...")


def get_new_png():
    #TODO crush them in this script
    aux = system_call("git diff --name-status $(git describe --abbrev=0 --tags) | grep -E '^[AM].*(png)'")
    if len(aux) == 0:
        print("No new PNGs.")
    else:
        print("New PNGs:")
        for line in aux.splitlines():
            print("  " + line)
    
    input("Done. Press Enter...")


def get_new_svg():
    aux = system_call("git diff --name-status $(git describe --abbrev=0 --tags) | grep -E '^[AM].*svg'")
    if len(aux) == 0:
        print("No new SVGs.")
    else:
        print("New SVGs:")
        for line in aux.splitlines():
            print("  " + line)
    
    input("Done. Press Enter...")


def find_problems():
    problems = []
    problems = problems + find_long_lines()
    problems = problems + find_alphabetical_problems()
    problems = problems + find_cramped_things()
    problems = problems + find_bad_documentation()
    
    problems_by_file = {}
    
    for file, problem, info in problems:
        if not file in problems_by_file:
            problems_by_file[file] = []
        problems_by_file[file].append((problem, info))
    
    for file, dummy in sorted(problems_by_file.items()):
        if file.find('imgui') != -1:
            continue
        print(file)
        for problem, info in problems_by_file[file]:
            print('  ' + problem + ': ' + info)
    
    input("Done. Press Enter...")
    
if __name__=="__main__":
    try:
        ls = system_call("ls Game_data")
        if len(ls) == 0:
            print("The current directory is not Pikifen's directory!")
            exit(1)
    except:
        print("The current directory is not Pikifen's directory!")
        exit(1)
    
    running = True
    while running:
        latest = system_call("git describe --abbrev=0 --tags")
    
        aux = system_call("grep -oP 'VERSION_MAJOR = .*;' Source/source/const.h")
        cur_major = aux[16:-1]
        aux = system_call("grep -oP 'VERSION_MINOR = .*;' Source/source/const.h")
        cur_minor = aux[16:-1]
        aux = system_call("grep -oP 'VERSION_REV   = .*;' Source/source/const.h")
        cur_rev = aux[16:-1]
        current = cur_major + "." + cur_minor + "." + cur_rev
        
        print("Pikifen code helper   (Pikifen code: " + current + ", last tagged: " + latest + ")")
        print("1. Change engine version numbers in const.h/resource files")
        print("2. Get new/deleted source files (for Visual Studio)")
        print("3. Get new textures")
        print("4. Get new PNG files")
        print("5. Get new SVG files")
        print("6. Find problems in code")
        print("Q. Quit")
        option = input()
        
        if option == "q" or option == "Q":
            running = False
            break
        elif option == "1":
            change_version_numbers()
        elif option == "2":
            get_new_source_files()
        elif option == "3":
            get_new_textures()
        elif option == "4":
            get_new_png()
        elif option == "5":
            get_new_svg()
        elif option == "6":
            find_problems()
        
        print("")

