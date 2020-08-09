import subprocess, re

class Function:
    def __init__(self):
        self.namespace = ''
        self.name = ''
        self.line = 0
        self.params = []
    def get_full(self):
        if len(self.namespace) == 0:
            return self.name
        else:
            return self.namespace + '::' + self.name
    def is_const_or_dest(self):
        if self.namespace.split('::')[-1] == self.name:
            return True
        if '~' + self.namespace == self.name:
            return True
        if re.match(r'.+ : .+', self.name) is not None:
            return True
        return False
    def is_exception(self):
        if self.name == 'create_fsm':
            return True
        return False


class Include:
    def __init__(self):
        self.name = ''
        self.line = 0


def remove_inside(s, l, r):
    c = 0
    bracket_start = 0
    level = 0
    while c < len(s):
        if s[c] == l:
            if level == 0:
                level = 1
                bracket_start = c
            else:
                level = level + 1
        elif s[c] == r:
            level = level - 1
            if level <= 0:
                pre = s[:bracket_start]
                pos = s[c + 1:]
                s = pre + pos
                c = bracket_start
        c = c + 1
    return s


def get_params(lines, line_nr):
    params = []
    sig = ''
    finding_sig = True

    # Scan the file to find the function's entire signature.
    while finding_sig:
        if line_nr > len(lines):
            # Reached the end of file.
            finding_sig = False
            break
        
        line = lines[line_nr - 1]
        
        bracket_pos = line.find('{')
        if bracket_pos != -1:
            finding_sig = False
            line = line[:bracket_pos + 1]
        colon_pos = line.find(':\n')
        if colon_pos != -1:
            finding_sig = False
            line = line[:colon_pos + 1]
        
        sig = sig + ' ' + line[:-1]
        line_nr = line_nr + 1
    
    sig = sig.strip()
    
    if len(sig) == 0:
        return params
    
    if sig.find('(') == -1 or sig.find(')') == -1:
        # Something went wrong...
        return params
    
    # Keep just what is between parentheses.
    sig = sig[sig.find('(') + 1:]
    sig = sig[:sig.rfind(')')]
    sig = sig.strip()

    # Keep it simple - remove angle brackets and parentheses, and everything in between.
    sig = remove_inside(sig, '<', '>')
    sig = remove_inside(sig, '[', ']')
    sig = remove_inside(sig, '(', ')')

    # Remove default values from parameters.
    sig = re.sub(r' = [^\,]+\,', ',', sig)
    sig = re.sub(r' = .+$', '', sig)

    # To make it easier, replace the end with a comma.
    sig = sig + ','

    # Get parameters.
    params = re.findall(r' ([^ ]+)\,', sig)

    for p in range(0, len(params)):
        if params[p][0] == '&' or params[p][0] == '*':
            params[p] = params[p][1:]

    return params


def get_functions(file_path):
    file = open(file_path, 'r')
    lines = file.readlines()
    
    functions = []
    line_nr = 0
    
    for l in lines:
        line_nr += 1
        
        if l[0] == ' ':
            # Anything indented is not a function definition.
            continue
        if l[0] == '#':
            # Preprocessor.
            continue
        if l[0] == '/':
            # Comment.
            continue
        if l.find('=') != -1:
            # Variable declaration.
            continue
        # Simplify calls to std::.
        l = l.replace('std::', '')
        
        # Style #1: function belonging to a namespace.
        r = re.match(r'(.+)::([^\(]+)\(', l)
        
        if r is not None:
            f = Function()
            f.line = line_nr
            
            f.namespace = r.group(1).strip()
            last_space_pos = f.namespace.rfind(' ')
            if last_space_pos != -1:
                f.namespace = f.namespace[last_space_pos + 1:]
            
            f.name = r.group(2).strip()

            f.params = get_params(lines, line_nr)
            
            functions.append(f)
            continue
        
        # Style #2: simple function.
        r = re.match(r'[^\(]+ ([^\(]+)\(', l)
        
        if r is not None:
            f = Function()
            f.line = line_nr
            
            f.name = r.group(1).strip()

            f.params = get_params(lines, line_nr)
            
            functions.append(f)
            continue
    
    return functions


def get_includes(file_path):
    file = open(file_path, 'r')
    
    includes = []
    line_nr = 0
    
    for l in file:
        line_nr += 1
        
        if l[0] == '/':
            # Comment.
            continue
        
        r = re.match(r'#include (.+)', l)
        
        if r is not None:
            i = Include()
            i.line = line_nr
            i.name = r.group(1).strip()
            
            if i.name.find('<allegro') != -1:
                i.type = INCLUDE_TYPE_ALLEGRO
            elif i.name.startswith('<'):
                i.type = INCLUDE_TYPE_SYSTEM
            else:
                i.type = INCLUDE_TYPE_LOCAL
            
            includes.append(i)
    
    return includes


def pad(s, nr):
    return str(s)[:nr].ljust(nr)


def system_call(command):
    try:
        o = subprocess.check_output(command, shell=True, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        if len(e.output) == 0:
            return ""
        else:
            raise e
    return o.decode("utf-8")[:-1]


INCLUDE_TYPE_SYSTEM = 0
INCLUDE_TYPE_ALLEGRO = 1
INCLUDE_TYPE_LOCAL = 2
