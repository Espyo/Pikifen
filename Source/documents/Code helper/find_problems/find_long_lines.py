from .common import *

def find_long_lines():
    aux = system_call("grep -Enr --include=\"*.cpp\" ^.\{81,\}$")
    aux = aux + system_call("grep -Enr --include=\"*.h\" ^.\{81,\}$")
    problems = []
    
    for output_line in aux.splitlines():
        path = output_line[:output_line.find(':')]
        remainder = output_line[output_line.find(':') + 1:]
        line_nr = remainder[:remainder.find(':')]
        problems.append((path, 'Line longer than 80 char', line_nr))
    
    return problems
