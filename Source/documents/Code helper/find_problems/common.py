import re
import subprocess
import sys


# An include statement that includes a system header file.
INCLUDE_TYPE_SYSTEM = 0
# An include statement that includes an Allegro header file.
INCLUDE_TYPE_ALLEGRO = 1
# An include statement that includes a local header file.
INCLUDE_TYPE_LOCAL = 2


## Represents a function in a source file.
class Function:
    ## Constructor.
    #  @param self Self.
    def __init__(self):
        # Namespace (or class or whatever) the function is in.
        self.namespace = ''
        # Name of the function.
        self.name = ''
        # Line in the source file.
        self.line = 0
        # Parameters it takes.
        self.params = []
    
    ## Returns a string representing the full name + namespace.
    #  @param self Self.
    #  @return A string representing the full name + namespace.
    def get_full(self):
        if len(self.namespace) == 0:
            return self.name
        else:
            return self.namespace + '::' + self.name
    
    ## Returns whether it is a constructor or destructor.
    #  @param self Self.
    #  @return Whether it is a constructor or destructor.
    def is_const_or_dest(self):
        if self.namespace.split('::')[-1] == self.name:
            return True
        if self.name.find('~') != -1:
            return True
        if re.match(r'.+ : .+', self.name) is not None:
            return True
        return False
    
    ## Returns whether this is an exception case for the engine's style.
    #  @param self Self.
    #  @param return Whether this is an exception case for the engine's style.
    def is_exception(self):
        if self.name == 'create_fsm':
            return True
        return False


## Represents an include statement in a source file.
class Include:
    ## Constructor.
    #  @param self Self.
    def __init__(self):
        # Include file name.
        self.name = ''
        # Line in the source file.
        self.line = 0


## Represents a constant declaration inside a namespace, in a source file.
class NamespaceConstant:
    ## Constructor.
    #  @param self Self.
    def __init__(self):
        # Name of the constant.
        self.name = ''
        # Namespace it is in.
        self.namespace = ''
        # Line in the source file.
        self.line = 0


## Returns the index of a given argument in the Python script's call.
## Returns -1 if it doesn't exist.
#  @param name Name of the argument to search for.
#  @return The argument's index.
def get_argument_idx(name):
    for a in range(len(sys.argv)):
        if sys.argv[a] == name:
            return a
    return -1


## Returns a list of functions in a source file.
#  @param file_path Path to the source file.
#  @return A list of functions in a source file.
def get_functions_in_file(file_path):
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

            f.params = get_params_in_file_lines(lines, line_nr)
            
            functions.append(f)
            continue
        
        # Style #2: simple function.
        r = re.match(r'[^\(]+ ([^\(]+)\(', l)
        
        if r is not None:
            f = Function()
            f.line = line_nr
            
            f.name = r.group(1).strip()

            f.params = get_params_in_file_lines(lines, line_nr)
            
            functions.append(f)
            continue
    
    file.close()
    
    return functions


## Returns a list of include statements in a source file.
#  @param file_path Path to the source file.
#  @return A list of include statements in a source file.
def get_includes_in_file(file_path):
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
    
    file.close()
    
    return includes


## Returns a list of constants inside namespaces, in a source file.
#  @param file_path Path to the source file.
#  @return A list of constants inside namespaces, in a source file.
def get_namespace_constants_in_file(file_path):
    file = open(file_path, 'r')
    lines = file.readlines()
    
    constants = []
    line_nr = 0
    in_namespace = None
    
    for l in lines:
        line_nr += 1

        r = re.match(r'namespace ([A-Z_]+) {', l)
        if r is not None:
            in_namespace = r.group(1).strip()
            continue
        
        r = re.match(r'\}', l)
        if r is not None:
            in_namespace = None
            continue
        
        if in_namespace is not None:
            r = re.match(r'.*const .+ ([A-Z_]+)(;|[^\)]+;)', l)

            if r is not None:
                c = NamespaceConstant()
                c.name = r.group(1).strip()
                c.namespace = in_namespace
                c.line = line_nr
                constants.append(c)
                continue
    
    file.close()
    
    return constants


## Returns a list of function parameters in a source file.
#  @param lines List of lines composing the source file.
#  @param line_nr Number of the line to start scanning at.
#  @return A list of function parameters in a source file.
def get_params_in_file_lines(lines, line_nr):
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

def get_source_dir_to_use():
    debug_arg_idx = get_argument_idx('debug') != -1
    script_debug_mode = (debug_arg_idx != -1)
    if script_debug_mode:
        return sys.argv[debug_arg_idx + 1]
    else:
        return 'Source/source'


## Pads a string such that it fits in a certain space.
#  This will trim the string to fit, or add spaces to the right, if necessary.
#  @param s The string to pad.
#  @param nr Number of characters it should take up.
#  @return The padded string.
def pad(s, nr):
    return str(s)[:nr].ljust(nr)


## Removes anything inside two tokens in the given string, as well as the tokens themselves.
#  @param s String to process.
#  @param l Token to the left of the text to remove.
#  @param r Token to the right of the text to remove.
#  @return The string without the text inside or the tokens.
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


## Performs a system call.
#  @param command Command to run.
#  @return A string with the command output. Returns an empty string if no output.
def system_call(command):
    try:
        o = subprocess.check_output(command, shell=True, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        if len(e.output) == 0:
            return ''
        else:
            raise e
    return o.decode('utf-8')[:-1]
