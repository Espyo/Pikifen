'''
Quick little script to allow me to transfer over a GUI data file's
item coordinates onto the C++ code file that initializes the default coordinates.
'''

import re
import sys

print('===INPUT===')
lines = []
while True:
    line = sys.stdin.readline()
    if line == '\n':
        break
    lines.append(line)

largest_name_len = 0
largest_x_len = 0
largest_y_len = 0
largest_w_len = 0
largest_h_len = 0
items = []
for line in lines:
    line = line.strip()
    match = re.match(r'(.+)=(.+) (.+) (.+) (.+)', line)
    item = {
        'name': match.group(1).strip(),
        'x': str(float(match.group(2))).replace('.0', ''),
        'y': str(float(match.group(3))).replace('.0', ''),
        'w': str(float(match.group(4))).replace('.0', ''),
        'h': str(float(match.group(5))).replace('.0', '')
    }
    largest_name_len = max(largest_name_len, len(item['name']))
    largest_x_len = max(largest_x_len, len(item['x']))
    largest_y_len = max(largest_y_len, len(item['y']))
    largest_w_len = max(largest_w_len, len(item['w']))
    largest_h_len = max(largest_h_len, len(item['h']))
    items.append(item)

print('===OUTPUT===')
for i in items:
    line = '    gui.register_coords('
    line += f'"{i["name"]}",'.ljust(largest_name_len + 3)
    line += f'{i["x"]},'.rjust(largest_x_len + 2)
    line += f'{i["y"]},'.rjust(largest_y_len + 2)
    line += f'{i["w"]},'.rjust(largest_w_len + 2)
    line += f'{i["h"]}'.rjust(largest_h_len + 1)
    line += ');'
    print(line)

print('')
