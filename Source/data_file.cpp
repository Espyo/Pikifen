/* © André Luís Gomes da Silva ([SRB2]Espyo)
 * This file belongs to the source code of an open-source rhythm game, PK Rhythm.
 * Please read the help files that come with the game, or go to http://pkrhythm.sf.net for more info.
 ***
 * See data_file.h.
 */

#include <allegro5/allegro.h>

#include <fstream>
#include <string>

#include "data_file.h"
#include "functions.h"

using namespace std;

string data_node_list::get_value(string def_value) {
    if(list.size() == 0) return def_value;
    return list[0].value;
}

data_node &data_node_list::operator[](size_t nr) {
    if(nr > list.size() - 1 || list.size() == 0) {
        dummy_nodes.push_back(data_node());
        return dummy_nodes[dummy_nodes.size() - 1];
    }
    return list[nr];
};

data_node_list &data_node_list::operator[](string name) {
    return operator[](0)[name];
}

size_t data_node_list::size() {
    return list.size();
}

void data_node_list::add() {
    list.push_back(data_node());
}

data_node &data_node_list::last() {
    return operator[](list.size() - 1);
}

data_node_list::data_node_list() {}
data_node_list::data_node_list(const data_node_list &dnl) {
    list = dnl.list;
}
data_node_list::~data_node_list() {}



data_node_list &data_node::operator[](string name) {
    if(nodes.find(name) == nodes.end()) {
        dummy_lists.push_back(data_node_list());
        return dummy_lists[dummy_lists.size() - 1];
    }
    return nodes[name];
}

data_node_list &data_node::get_node_list_by_nr(size_t nr, string* name) {
    if(nr < nodes.size()) {
        for(map<string, data_node_list>::iterator n = nodes.begin(); n != nodes.end(); n++) {
            if(nr == 0) {
                if(name) *name = n->first;
                return n->second;
            }
            nr--;
        }
    }
    
    dummy_lists.push_back(data_node_list());
    return dummy_lists[dummy_lists.size() - 1];
}

void data_node::load_file(string filename) {
    vector<string> lines;
    ALLEGRO_FILE* file = al_fopen(filename.c_str(), "r");
    bool is_first_line = true;
    file_was_opened = false;
    if(file) {
        file_was_opened = true;
        while(!al_feof(file)) {
            string line;
            getline(file, line);
            
            if(is_first_line) {
                //Let's just check if it starts with the UTF-8 Magic Number.
                if(line.size() >= 3) {
                    if(line.substr(0, 3) == UTF8_MAGIC_NUMBER) line = line.erase(0, 3);
                }
            }
            lines.push_back(line);
            is_first_line = false;
        }
        al_fclose(file);
    }
    
    load_node(lines);
}

/*
 * Loads a node from a bit of text.
 * lines:       A vector with the lines of text.
 * start_line:  Start on this line. Used for sub-nodes.
 * Returns the line number it ended on, so the parent node can continue from there.
 */
size_t data_node::load_node(vector<string> lines, size_t start_line) {
    nodes.clear();
    value = "";
    size_t n_lines = lines.size();
    if(start_line > n_lines) return start_line;
    
    for(size_t l = start_line; l < n_lines; l++) {
        string line = lines[l];
        
        line = trim_spaces(line, true);     //Removes the leftmost spaces.
        
        if(line.size() >= 2)
            if(line[0] == '/' && line[1] == '/') continue;  //A comment.
            
        //Option=value
        size_t pos = line.find('=');
        if(pos != string::npos && pos > 0 && line.size() >= 2) {
            string option = trim_spaces(line.substr(0, pos));
            string value = line.substr(pos + 1, line.size() - (pos + 1));
            nodes[option].add();
            nodes[option].last().value = value;
            continue;
        }
        
        //Sub-node start
        pos = line.find('{');
        if(pos != string::npos && pos > 0 && line.size() >= 2) {
            string section_name = trim_spaces(line.substr(0, pos));
            nodes[section_name].add();
            l = nodes[section_name].last().load_node(lines, l + 1);
            continue;
        }
        
        //Sub-node end
        pos = line.find('}');
        if(pos != string::npos) {
            return l;
        }
    }
    
    return n_lines - 1;
}

size_t data_node::size() {
    return nodes.size();
}

data_node::data_node() {
    file_was_opened = false;
}
data_node::data_node(const data_node &dn) {
    file_was_opened = dn.file_was_opened;
    nodes = dn.nodes;
    value = dn.value;
}
data_node::data_node(string filename) {
    file_was_opened = false;
    load_file(filename);
}
data_node::~data_node() {}

/* ----------------------------------------------------------------------------
 * Like an std::getline(), but for ALLEGRO_FILE*.
 */
void getline(ALLEGRO_FILE* file, string &line) {
    line = "";
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
string trim_spaces(string s, bool left_only) {
    //Spaces before.
    if(s.size()) {
        while(s[0] == ' ' || s[0] == '\t') {
            s.erase(0, 1);
            if(s.size() == 0) break;
        }
    }
    
    if(!left_only) {
        //Spaces after.
        if(s.size()) {
            while(s[s.size() - 1] == ' ' || s[s.size() - 1] == '\t') {
                s.erase(s.size() - 1, 1);
                if(s.size() == 0) break;
            }
        }
    }
    
    return s;
}
