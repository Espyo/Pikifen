import os, sys

input_file = open("../../Game_data/Misc/Tools.txt", "rb")
lines = []
mode = ""
option = ""

if(len(sys.argv) >= 3) :
    mode = sys.argv[1]
    option = sys.argv[2]

for line in input_file :
    if(line.find("auto_start_mode") != -1) :
        lines.append("auto_start_mode = " + mode + "\n")
    elif(line.find("auto_start_option") != -1) :
        lines.append("auto_start_option = " + option + "\n")
    else :
        lines.append(line)

input_file.close()

output_file = open("../../Game_data/Misc/Tools.txt", "wb")

for line in lines :
    output_file.write(line)
