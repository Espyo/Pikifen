//Read data_file.h for more info.
#include <fstream>

#include <allegro5/allegro.h>

#include "data_file.h"

using namespace std;

//Creates a dummy node. If the programmer
//requests an invalid node, a dummy is returned.
data_node* data_node::create_dummy() {
    data_node* new_dummy_child = new data_node();
    new_dummy_child->line_nr = line_nr;
    new_dummy_child->file_name = file_name;
    new_dummy_child->file_was_opened = file_was_opened;
    dummy_children.push_back(new_dummy_child);
    return new_dummy_child;
}


//Returns the number of children nodes (direct children only).
size_t data_node::get_nr_of_children() {
    return children.size();
}


//Returns a child node given its number on the list (direct children only).
data_node* data_node::get_child(const size_t number) {
    if(number >= children.size()) return create_dummy();
    return children[number];
}


//Returns the number of occurences of a child name (direct children only).
size_t data_node::get_nr_of_children_by_name(const string &name) {
    size_t number = 0;
    
    for(size_t c = 0; c < children.size(); ++c) {
        if(name == children[c]->name) ++number;
    }
    
    return number;
}


//Returns the nth child with this name on the list (direct children only).
data_node* data_node::get_child_by_name(
    const string &name, const size_t occurrence_number
) {
    size_t cur_occurrence_number = 0;
    
    for(size_t c = 0; c < children.size(); ++c) {
        if(name == children[c]->name) {
            if(cur_occurrence_number == occurrence_number) {
                //We found it.
                return children[c];
            } else {
                ++cur_occurrence_number;
            }
        }
    }
    
    return create_dummy();
}


//Returns the value of a node, or def if it has no value.
string data_node::get_value_or_default(const string &def) {
    return (value.empty() ? def : value);
}


//Adds a new child to the list.
size_t data_node::add(data_node* new_node) {
    children.push_back(new_node);
    return children.size() - 1;
}


//Removes and destroys a child from the list.
bool data_node::remove(data_node* node_to_remove) {
    for(size_t c = 0; c < children.size(); ++c) {
        if(children[c] == node_to_remove) {
            delete node_to_remove;
            children.erase(children.begin() + c);
            return true;
        }
    }
    return false;
}


//Loads data from a file.
void data_node::load_file(
    const string &file_name, const bool trim_values,
    const bool names_only_after_root
) {
    vector<string> lines;
    
    file_was_opened = false;
    this->file_name = file_name;
    
    ALLEGRO_FILE* file = al_fopen(file_name.c_str(), "r");
    if(file) {
        bool is_first_line = true;
        file_was_opened = true;
        string line;
        while(!al_feof(file)) {
            getline(file, line);
            
            if(is_first_line) {
                //Let's just check if it starts with the UTF-8 Magic Number.
                if(
                    line.size() >= 3 &&
                    line.substr(0, 3) == UTF8_MAGIC_NUMBER
                ) {
                    line = line.erase(0, 3);
                }
            }
            lines.push_back(line);
            is_first_line = false;
        }
        al_fclose(file);
    }
    
    load_node(lines, trim_values, 0, 0, names_only_after_root);
}


//Loads data from a list of text lines.
//Returns the number of the line this node ended on, judging by start_line.
//This is used for the recursion.
size_t data_node::load_node(
    const vector<string> &lines, const bool trim_values,
    const size_t start_line, const size_t depth,
    const bool names_only_after_root
) {
    children.clear();
    
    if(start_line > lines.size()) return start_line;
    
    bool returning_from_sub_node = false;
    
    for(size_t l = start_line; l < lines.size(); ++l) {
        string line = lines[l];
        
        line = trim_spaces(line, true); //Removes the leftmost spaces.
        
        if(line.empty()) continue;
        
        if(line.size() >= 2) {
            if(line[0] == '/' && line[1] == '/') {
                //A comment; ignore this line.
                continue;
            }
        }
        
        //Sub-node end.
        size_t pos = line.find('}');
        if(pos != string::npos) {
            if(returning_from_sub_node) {
                //The sub-node just ended.
                //Let's leave what's after the bracket, and let the rest
                //of the code make use of it.
                returning_from_sub_node = false;
                line = line.substr(pos + 1, line.size() - (pos + 1));
                line = trim_spaces(line, true);
                if(line.empty()) continue;
            } else {
                return l;
            }
        }
        
        //Sub-node start.
        pos = line.find('{');
        if(pos != string::npos) {
        
            data_node* new_child = new data_node();
            new_child->name = trim_spaces(line.substr(0, pos));
            new_child->value.clear();
            new_child->file_was_opened = file_was_opened;
            new_child->file_name = file_name;
            new_child->line_nr = l + 1;
            l =
                new_child->load_node(
                    lines, trim_values, l + 1, depth + 1, names_only_after_root
                );
            l--; //So the block-ending line gets re-examined.
            children.push_back(new_child);
            
            returning_from_sub_node = true;
            continue;
        }
        
        //Option=value.
        pos = line.find('=');
        string n, v;
        if(
            (!names_only_after_root || depth == 0) &&
            pos != string::npos && pos > 0 && line.size() > 2
        ) {
            n = line.substr(0, pos);
            v = line.substr(pos + 1, line.size() - (pos + 1));
        } else {
            n = line;
        }
        if(trim_values) v = trim_spaces(v);
        
        data_node* new_child = new data_node();
        new_child->name = trim_spaces(n);
        new_child->value = v;
        new_child->file_was_opened = file_was_opened;
        new_child->file_name = file_name;
        new_child->line_nr = l + 1;
        children.push_back(new_child);
        
    }
    
    return lines.size() - 1;
}


//Saves a node into a new text file. Line numbers are ignored.
//If you don't provide a file name, it'll use the node's file name.
//Returns true on success.
bool data_node::save_file(
    string file_name, const bool children_only,
    const bool include_empty_values
) {
    if(file_name == "") file_name = this->file_name;
    
    //Create any missing folders.
    size_t next_slash_pos = file_name.find('/', 0);
    while(next_slash_pos != string::npos) {
        string path_so_far = file_name.substr(0, next_slash_pos);
        if(!al_make_directory(path_so_far.c_str())) {
            return false;
        }
        next_slash_pos = file_name.find('/', next_slash_pos + 1);
    }
    
    //Save the file.
    ALLEGRO_FILE* file = al_fopen(file_name.c_str(), "w");
    if(file) {
        if(children_only) {
            for(size_t c = 0; c < children.size(); ++c) {
                children[c]->save_node(file, 0, include_empty_values);
            }
        } else {
            save_node(file, 0, include_empty_values);
        }
        al_fclose(file);
        return true;
    } else {
        return false;
    }
}


//Saved a node into a text file.
void data_node::save_node(
    ALLEGRO_FILE* file, const size_t level,
    const bool include_empty_values
) {
    string tabs = string(level, '\t');
    
    al_fwrite(file, tabs.c_str(), tabs.size());
    al_fwrite(file, name.c_str(), name.size());
    
    if(!children.empty()) {
        al_fwrite(file, "{\n", 2);
        for(size_t c = 0; c < children.size(); ++c) {
            children[c]->save_node(file, level + 1, include_empty_values);
        }
        al_fwrite(file, tabs.c_str(), tabs.size());
        al_fwrite(file, "}", 1);
        
    } else if(!value.empty() || include_empty_values) {
        al_fwrite(file, "=", 1);
        al_fwrite(file, value.c_str(), value.size());
    }
    al_fwrite(file, "\n", 1);
    
}


//Creates an empty data node.
data_node::data_node() :
    file_was_opened(false),
    line_nr(0) {
    
}


//Creates a data node, using the data and creating a copy
//of the children from another node.
data_node::data_node(const data_node &dn2) :
    name(dn2.name),
    value(dn2.value),
    file_was_opened(dn2.file_was_opened),
    file_name(dn2.file_name),
    line_nr(dn2.line_nr) {
    
    for(size_t c = 0; c < dn2.children.size(); ++c) {
        children.push_back(new data_node(*(dn2.children[c])));
    }
    for(size_t dc = 0; dc < dn2.dummy_children.size(); ++dc) {
        dummy_children.push_back(new data_node(*(dn2.dummy_children[dc])));
    }
}


//Creates a data node from a file, given the file name.
data_node::data_node(const string &file_name) :
    file_was_opened(false),
    file_name(file_name),
    line_nr(0) {
    
    load_file(file_name);
}


//Creates a data node by filling its name and value.
data_node::data_node(const string &name, const string &value) :
    name(name),
    value(value),
    file_was_opened(false),
    line_nr(0) {
    
}


//Destroys a data node and all the children within.
data_node::~data_node() {
    for(size_t c = 0; c < children.size(); ++c) {
        delete children[c];
    }
    
    for(size_t dc = 0; dc < dummy_children.size(); ++dc) {
        delete dummy_children[dc];
    }
}


/* ----------------------------------------------------------------------------
 * Like an std::getline(), but for ALLEGRO_FILE*.
 */
void getline(ALLEGRO_FILE* file, string &line) {
    line.clear();
    if(!file) {
        return;
    }
    
    size_t bytes_read;
    char* c_ptr = new char;
    char c;
    
    bytes_read = al_fread(file, c_ptr, 1);
    while(bytes_read > 0) {
        c = *((char*) c_ptr);
        
        if(c == '\r' || c == '\n') {
            break;
        } else {
            line.push_back(c);
        }
        
        bytes_read = al_fread(file, c_ptr, 1);
    }
    
    delete c_ptr;
}


/* ----------------------------------------------------------------------------
 * Removes all trailing and preceding spaces.
 * This means space and tab characters before and after the 'middle' characters.
 * s:         The original string.
 * left_only: If true, only trim the spaces at the left.
 */
string data_node::trim_spaces(const string &s, const bool left_only) {
    string orig = s;
    //Spaces before.
    if(orig.size()) {
        while(orig[0] == ' ' || orig[0] == '\t') {
            orig.erase(0, 1);
            if(orig.empty()) break;
        }
    }
    
    if(!left_only) {
        //Spaces after.
        if(orig.size()) {
            while(
                orig[orig.size() - 1] == ' ' ||
                orig[orig.size() - 1] == '\t'
            ) {
                orig.erase(orig.size() - 1, 1);
                if(orig.empty()) break;
            }
        }
    }
    
    return orig;
}
