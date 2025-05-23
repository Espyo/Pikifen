import os, re

from .common import *


## Returns whether two constants are in the right order.
#  @param c1 The first constant.
#  @param c2 The second constant.
#  @return Whether two constants are in the right order.
def are_constants_ordered(c1, c2):
    if len(c1) == 0:
        return len(c2) == 0
    if len(c2) == 0:
        return True
    return c1.lower() < c2.lower()


## Returns whether two functions are in the right order.
#  @param f1 The first function.
#  @param f2 The second function.
#  @return Whether two functions are in the right order.
def are_functions_ordered(f1, f2):
    if f2.is_const_or_dest():
        return f1.is_const_or_dest()
    if f1.is_const_or_dest():
        return True
    return f1.name.lower() < f2.name.lower()


## Returns whether two includes are in the right order.
#  @param i1 The first include.
#  @param i2 The second include.
#  @return Whether two includes are in the right order.
def are_includes_ordered(i1, i2):
    if i1.type > i2.type:
        return False
    if i1.type < i2.type:
        return True
    if abs(i1.line - i2.line) > 1:
        # Different blocks. No comparison.
        return True
    return i1.name.lower() < i2.name.lower()


## Returns whether two namespaces are in the right order.
#  @param n1 The first namespace.
#  @param n2 The second namespace.
#  @return Whether two namespaces are in the right order.
def are_namespaces_ordered(n1, n2):
    if len(n1) == 0:
        return len(n2) == 0
    if len(n2) == 0:
        return True
    return n1.lower() < n2.lower()


## Returns all unordered things in all source files as problems.
#  @return All unordered thing problems found.
def get_all_unordered_problems():
    problems = []
    source_dir_to_use = get_source_dir_to_use()
    for dirpath, dirnames, files in os.walk(get_source_dir_to_use()):
        for f in files:
            if f.endswith('cpp'):
                file_problems = get_unordered_problems_in_file(os.path.join(dirpath, f))
                if file_problems is not None:
                    problems = problems + file_problems
    return problems


## Returns all unordered constants, organizing them in pairs.
#  @param constants List of constants to check.
#  @return All unordered constants.
def get_unordered_constants(constants):
    unordered_constants = []
    
    for c in range(0, len(constants) - 1):
        if constants[c].namespace != constants[c + 1].namespace:
            continue
        
        if not are_constants_ordered(constants[c].name, constants[c + 1].name):
            unordered_constants.append((constants[c], constants[c + 1]))
    
    return unordered_constants


## Returns all unordered functions, organizing them in pairs.
#  @param functions List of functions to check.
#  @return All unordered functions.
def get_unordered_functions(functions):
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


## Returns all unordered namespaces, organizing them in pairs.
#  @param namespaces List of namespaces to check.
#  @return All unordered namespaces.
def get_unordered_namespaces(functions):
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


## Returns all unordered includes, organizing them in pairs.
#  @param includes List of includes to check.
#  @return All unordered includes.
def get_unordered_includes(includes):
    unordered_includes = []
    
    for i in range(0, len(includes) - 1):
        if not are_includes_ordered(includes[i], includes[i + 1]):
            unordered_includes.append((includes[i], includes[i + 1]))
    
    return unordered_includes


## Returns all unordered things in a source file as problems.
#  @param file_path Path to the file to check.
#  @return All unordered thing problems found.
def get_unordered_problems_in_file(file_path):
    namespace_constants = get_namespace_constants_in_file(file_path)
    functions = get_functions_in_file(file_path)
    includes = get_includes_in_file(file_path)
    
    unordered_functions = get_unordered_functions(functions)
    unordered_namespaces = get_unordered_namespaces(functions)
    unordered_includes = get_unordered_includes(includes)
    unordered_namespace_constants = get_unordered_constants(namespace_constants)
    
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
