'''
Quick little script to allow me to transfer over a GUI data file's
item coordinates onto the C++ code file that initializes the default coordinates.
'''

import re
import sys

print('===INPUT===')
input_lines = []
while True:
    line = sys.stdin.readline()
    if line == '\n':
        break
    input_lines.append(line)

largest_name_len = 0
largest_x_len = 0
largest_y_len = 0
largest_w_len = 0
largest_h_len = 0
items = []
latest_item = {}
for line in input_lines:
    line = line.strip()
    
    match = re.match(r'\s*(.+)\s*\{', line)
    if match:
        if len(latest_item) > 0:
            items.append(latest_item)
            latest_item = {}
        latest_item['name'] = match.group(1)
    
    match = re.match(r'\s*center\s*=(.+) (.+)', line)
    if match:
        latest_item['x'] = str(float(match.group(1))).replace('.0', '')
        latest_item['y'] = str(float(match.group(2))).replace('.0', '')
    
    match = re.match(r'\s*size\s*=(.+) (.+)', line)
    if match:
        latest_item['w'] = str(float(match.group(1))).replace('.0', '')
        latest_item['h'] = str(float(match.group(2))).replace('.0', '')

if len(latest_item) > 0:
    items.append(latest_item)

for i in items:
    largest_name_len = max(largest_name_len, len(i['name']))
    largest_x_len = max(largest_x_len, len(i['x']))
    largest_y_len = max(largest_y_len, len(i['y']))
    largest_w_len = max(largest_w_len, len(i['w']))
    largest_h_len = max(largest_h_len, len(i['h']))


print('===OUTPUT===')
for i in items:
    line = '    gui.registerCoords('
    line += f'"{i["name"]}",'.ljust(largest_name_len + 3)
    line += f'{i["x"]},'.rjust(largest_x_len + 2)
    line += f'{i["y"]},'.rjust(largest_y_len + 2)
    line += f'{i["w"]},'.rjust(largest_w_len + 2)
    line += f'{i["h"]}'.rjust(largest_h_len + 1)
    line += ');'
    print(line)

print('')
