import os, re

from .common import *

def are_functions_ordered(f1, f2):
    if f2.is_const_or_dest():
        return f1.is_const_or_dest()
    if f1.is_const_or_dest():
        return True
    return f1.name < f2.name


def check_function_order(functions):
    unordered_functions = []
    
    for f in functions:
        if re.match('operator.+', f.name):
            f.name = 'operator'
    
    for f in range(0, len(functions) - 1):
        if functions[f].namespace != functions[f + 1].namespace:
            continue
        if functions[f].name == functions[f + 1].name:
            continue
        if functions[f].is_exception() or functions[f + 1].is_exception():
            continue
        
        if not are_functions_ordered(functions[f], functions[f + 1]):
            unordered_functions.append((functions[f], functions[f + 1]))
    
    return unordered_functions


def check_constant_order(constants):
    unordered_constants = []
    
    for c in range(0, len(constants) - 1):
        if constants[c].namespace != constants[c + 1].namespace:
            continue
        
        if not are_constants_ordered(constants[c].name, constants[c + 1].name):
            unordered_constants.append((constants[c], constants[c + 1]))
    
    return unordered_constants


def are_namespaces_ordered(n1, n2):
    if len(n1) == 0:
        return len(n2) == 0
    if len(n2) == 0:
        return True
    return n1 < n2


def are_constants_ordered(c1, c2):
    if len(c1) == 0:
        return len(c2) == 0
    if len(c2) == 0:
        return True
    return c1 < c2


def check_namespace_order(functions):
    unordered_namespaces = []
    namespace_starters = []
    last_namespace = None
    
    for f in functions:
        if f.namespace != last_namespace:
            namespace_starters.append(f)
            last_namespace = f.namespace
    
    for n in range(0, len(namespace_starters) - 1):
        if not are_namespaces_ordered(namespace_starters[n].namespace, namespace_starters[n + 1].namespace):
            unordered_namespaces.append((namespace_starters[n], namespace_starters[n + 1]))
    
    return unordered_namespaces


def are_includes_ordered(i1, i2):
    if i1.type > i2.type:
        return False
    if i1.type < i2.type:
        return True
    if abs(i1.line - i2.line) > 1:
        # Different blocks. No comparison.
        return True
    return i1.name < i2.name


def check_include_order(includes):
    unordered_includes = []
    
    for i in range(0, len(includes) - 1):
        if not are_includes_ordered(includes[i], includes[i + 1]):
            unordered_includes.append((includes[i], includes[i + 1]))
    
    return unordered_includes


def check_unordered_things_in_file(file_path):
    namespace_constants = get_namespace_constants(file_path)
    functions = get_functions(file_path)
    includes = get_includes(file_path)
    
    unordered_functions = check_function_order(functions)
    unordered_namespaces = check_namespace_order(functions)
    unordered_includes = check_include_order(includes)
    unordered_namespace_constants = check_constant_order(namespace_constants)
    
    problems = []
    
    for f1, f2 in unordered_functions:
        problems.append((file_path, 'Unordered functions', pad(f1.line, 4) + ' ' + pad(f1.get_full(), 40) + ' - ' + pad(f2.line, 4) + ' ' + pad(f2.name, 20)))
    
    for f1, f2 in unordered_namespaces:
        problems.append((file_path, 'Unordered namespaces', pad(f1.line, 4) + ' ' + pad(f1.namespace, 20) + ' - ' + pad(f2.line, 4) + ' ' + pad(f2.namespace, 20)))
    
    for i1, i2 in unordered_includes:
        problems.append((file_path, 'Unordered includes', pad(i1.line, 4) + ' ' + pad(i1.name, 40) + ' - ' + pad(i2.line, 4) + ' ' + pad(i2.name, 40)))
    
    for c1, c2 in unordered_namespace_constants:
        problems.append((file_path, 'Unordered namespace constants', pad(c1.line, 4) + ' ' + pad(c1.name, 20) + ' - ' + pad(c2.line, 4) + ' ' + pad(c2.name, 20)))
    
    return problems


def find_alphabetical_problems(debug_mode):
    problems = []
    path = 'Source/source' if not debug_mode else '.'
    for dirpath, dirnames, files in os.walk(path):
        for f in files:
            if f.endswith('cpp'):
                file_problems = check_unordered_things_in_file(os.path.join(dirpath, f))
                if file_problems is not None:
                    problems = problems + file_problems
    return problems
