//Read data_file.h for more info.

#undef _CMATH_
#include <fstream>

#include <allegro5/allegro.h>

#include "data_file.h"


using std::size_t;
using std::string;
using std::vector;


namespace DATA_FILE {

//When encrypting, this is the lowest ASCII value that can be affected.
const unsigned char ENCRYPTION_MIN_VALUE = 32; //Space character.

//When encrypting, rotate the character values forward by this amount.
const unsigned char ENCRYPTION_ROT_AMOUNT = 111;

//If a file starts with these bytes, then it's UTF-8.
const string UTF8_MAGIC_NUMBER = "\xEF\xBB\xBF";

}


/**
 * @brief Constructs a new empty data node object.
 */
data_node::data_node() {

}


/**
 * @brief Constructs a new data node object, using the data and creating a copy
 * of the children from another node.
 *
 * @param dn2 The node to copy data from.
 */
data_node::data_node(const data_node &dn2) :
    name(dn2.name),
    value(dn2.value),
    file_was_opened(dn2.file_was_opened),
    file_path(dn2.file_path),
    line_nr(dn2.line_nr) {
    
    for(size_t c = 0; c < dn2.children.size(); ++c) {
        children.push_back(new data_node(*(dn2.children[c])));
    }
    for(size_t dc = 0; dc < dn2.dummy_children.size(); ++dc) {
        dummy_children.push_back(new data_node(*(dn2.dummy_children[dc])));
    }
}


/**
 * @brief Constructs a new data node object from a file, given the file name.
 *
 * @param file_path Name of the file to load.
 */
data_node::data_node(const string &file_path) :
    file_path(file_path) {
    
    load_file(file_path);
}


/**
 * @brief Constructs a new data node object by filling its name and value.
 *
 * @param name The node's name.
 * @param value Its value.
 */
data_node::data_node(const string &name, const string &value) :
    name(name),
    value(value) {
    
}


/**
 * @brief Destroys the data node object and all the children within.
 *
 */
data_node::~data_node() {
    for(size_t c = 0; c < children.size(); ++c) {
        delete children[c];
    }
    
    for(size_t dc = 0; dc < dummy_children.size(); ++dc) {
        delete dummy_children[dc];
    }
}


/**
 * @brief Adds a new child to the list.
 *
 * @param new_node The node to add.
 * @return The new child's index.
 */
size_t data_node::add(data_node* new_node) {
    children.push_back(new_node);
    return children.size() - 1;
}


/**
 * @brief Creates a dummy node. If the programmer requests an invalid node,
 * a dummy is returned.
 *
 * @return The dummy node.
 */
data_node* data_node::create_dummy() {
    data_node* new_dummy_child = new data_node();
    new_dummy_child->line_nr = line_nr;
    new_dummy_child->file_path = file_path;
    new_dummy_child->file_was_opened = file_was_opened;
    dummy_children.push_back(new_dummy_child);
    return new_dummy_child;
}


/**
 * @brief "Decrypts" a character for loading an encrypted data file.
 *
 * See encrypt_char for more info.
 *
 * @param c Character to decrypt.
 * @return The decrypted character.
 */
unsigned char data_node::decrypt_char(unsigned char c) {
    if(c < DATA_FILE::ENCRYPTION_MIN_VALUE) {
        return c;
    }
    const unsigned char range = 255 - DATA_FILE::ENCRYPTION_MIN_VALUE;
    int c2 = c;
    c2 -= DATA_FILE::ENCRYPTION_MIN_VALUE;
    c2 += range; //Negative modulo isn't such a good idea.
    c2 -= DATA_FILE::ENCRYPTION_ROT_AMOUNT;
    c2 %= range;
    c2 += DATA_FILE::ENCRYPTION_MIN_VALUE;
    return c2;
}


/**
 * @brief "Encrypts" a character for saving in an encrypted data file.
 *
 * It does this by rotating each character's ASCII value backwards by 111,
 * but only if it's a printable character, as other characters that tend
 * to be reserved for important things, like \0 or EOF.
 *
 * @param c Character to encrypt.
 * @return The encrypted character.
 */
unsigned char data_node::encrypt_char(unsigned char c) {
    if(c < DATA_FILE::ENCRYPTION_MIN_VALUE) {
        return c;
    }
    const unsigned char range = 255 - DATA_FILE::ENCRYPTION_MIN_VALUE;
    int c2 = c;
    c2 -= DATA_FILE::ENCRYPTION_MIN_VALUE;
    c2 += DATA_FILE::ENCRYPTION_ROT_AMOUNT;
    c2 %= range;
    c2 += DATA_FILE::ENCRYPTION_MIN_VALUE;
    return c2;
}


/**
 * @brief "Encrypts" an entire string for saving in an encrypted data file.
 *
 * See encrypt_char for more info.
 *
 * @param s String to encrypt.
 */
void data_node::encrypt_string(string &s) {
    for(size_t c = 0; c < s.size(); ++c) {
        s[c] = encrypt_char(s[c]);
    }
}


/**
 * @brief Like an std::getline(), but for ALLEGRO_FILE*.
 *
 * @param file Allegro file handle.
 * @param line String to save the line into.
 * @param encrypted If true, the document is encrypted and needs decrypting.
 */
void data_node::getline(
    ALLEGRO_FILE* file, string &line, const bool encrypted
) {
    line.clear();
    if(!file) {
        return;
    }
    
    size_t bytes_read;
    char* c_ptr = new char;
    
    bytes_read = al_fread(file, c_ptr, 1);
    while(bytes_read > 0) {
        unsigned char c = *((unsigned char*) c_ptr);
        
        if(encrypted) {
            c = decrypt_char(c);
        }
        
        if(c == '\r') {
            //Let's check if the next character is a \n. If so, they should
            //both be consumed by al_fread().
            bytes_read = al_fread(file, c_ptr, 1);
            unsigned char peek_c = *((unsigned char*) c_ptr);
            if(encrypted) {
                peek_c = decrypt_char(peek_c);
            }
            if(bytes_read > 0) {
                if(peek_c == '\n') {
                    //Yep. Done.
                    break;
                } else {
                    //Oops, we're reading an entirely new line. Let's go back.
                    al_fseek(file, -1, ALLEGRO_SEEK_CUR);
                    break;
                }
            }
            
        } else if(c == '\n') {
            //Standard line break.
            break;
            
        } else {
            //Line content.
            line.push_back(c);
            
        }
        
        bytes_read = al_fread(file, c_ptr, 1);
    }
    
    delete c_ptr;
}


/**
 * @brief Returns a child node given its number on the list
 * (direct children only).
 *
 * @param number The index number of the child.
 * @return The node.
 */
data_node* data_node::get_child(const size_t number) {
    if(number >= children.size()) return create_dummy();
    return children[number];
}


/**
 * @brief Returns the nth child with this name on the list
 * (direct children only).
 *
 * @param name The name the child must have.
 * @param occurrence_number This function will return the nth child with
 * the specified name.
 * @return The node.
 */
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


/**
 * @brief Returns the number of children nodes (direct children only).
 *
 * @return The number.
 */
size_t data_node::get_nr_of_children() const {
    return children.size();
}


/**
 * @brief Returns the number of occurences of a child name
 * (direct children only).
 *
 * @param name Name the children must have.
 * @return The number.
 */
size_t data_node::get_nr_of_children_by_name(const string &name) const {
    size_t number = 0;
    
    for(size_t c = 0; c < children.size(); ++c) {
        if(name == children[c]->name) ++number;
    }
    
    return number;
}


/**
 * @brief Returns the value of a node, or def if it has no value.
 *
 * @param def Default value.
 * @return The value.
 */
string data_node::get_value_or_default(const string &def) const {
    return (value.empty() ? def : value);
}


/**
 * @brief Loads data from a file.
 *
 * @param file_path Path to the file to load.
 * @param trim_values If true, spaces before and after the value will
 * be trimmed off.
 * @param names_only_after_root If true, any nodes that are not in the
 * root node (i.e. they are children of some node inside the file)
 * will only have a name and no value; the entire contents of their
 * line will be their name.
 * @param encrypted If true, the file is encrypted, and needs decrypting.
 */
void data_node::load_file(
    const string &file_path, const bool trim_values,
    const bool names_only_after_root, const bool encrypted
) {
    vector<string> lines;
    
    file_was_opened = false;
    this->file_path = file_path;
    
    ALLEGRO_FILE* file = al_fopen(file_path.c_str(), "r");
    if(file) {
        bool is_first_line = true;
        file_was_opened = true;
        string line;
        while(!al_feof(file)) {
            getline(file, line, encrypted);
            
            if(is_first_line && !encrypted) {
                //Let's just check if it starts with the UTF-8 Magic Number.
                if(
                    line.size() >= 3 &&
                    line.substr(0, 3) == DATA_FILE::UTF8_MAGIC_NUMBER
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


/**
 * @brief Loads data from a list of text lines.
 *
 * @param lines Text lines that make up the node.
 * @param trim_values If true, spaces before and after the value will
 * be trimmed off.
 * @param start_line This node starts at this line of the document.
 * @param depth Depth of this node. 0 means root.
 * @param names_only_after_root If true, any nodes that are not in the
 * root node (i.e. they are children of some node inside the file)
 * will only have a name and no value; the entire contents of their
 * line will be their name.
 * @return Returns the number of the line this node ended on,
 * judging by start_line. This is used for the recursion.
 */
size_t data_node::load_node(
    const vector<string> &lines, const bool trim_values,
    const size_t start_line, const size_t depth,
    const bool names_only_after_root
) {
    children.clear();
    
    if(start_line >= lines.size()) return start_line;
    
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
            new_child->file_path = file_path;
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
        new_child->file_path = file_path;
        new_child->line_nr = l + 1;
        children.push_back(new_child);
        
    }
    
    return lines.size() - 1;
}


/**
 * @brief Copies data from another data node.
 *
 * @param dn2 Node to copy from.
 * @return The current node.
 */
data_node &data_node::operator=(const data_node &dn2) {
    if(this != &dn2) {
        name = dn2.name;
        value = dn2.value;
        file_was_opened = dn2.file_was_opened;
        file_path = dn2.file_path;
        line_nr = dn2.line_nr;
        
        children.clear();
        for(size_t c = 0; c < dn2.children.size(); ++c) {
            children.push_back(new data_node(*(dn2.children[c])));
        }
        dummy_children.clear();
        for(size_t dc = 0; dc < dn2.dummy_children.size(); ++dc) {
            dummy_children.push_back(new data_node(*(dn2.dummy_children[dc])));
        }
    }
    
    return *this;
}


/**
 * @brief Removes and destroys a child from the list.
 *
 * @param node_to_remove The node to be removed.
 * @return Whether the node existed.
 */
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


/**
 * @brief Saves a node into a new text file. Line numbers are ignored.
 * If you don't provide a file name, it'll use the node's file name.
 *
 * @param file_path Path to the file to save to.
 * @param children_only If true, only save the nodes inside this node.
 * @param include_empty_values If true, even nodes with an empty value
 * will be saved.
 * @param encrypted If true, the file must be encrypted.
 * @return Whether it succeded.
 */
bool data_node::save_file(
    string file_path, const bool children_only,
    const bool include_empty_values, const bool encrypted
) const {

    if(file_path == "") file_path = this->file_path;
    
    //Create any missing folders.
    size_t next_slash_pos = file_path.find('/', 0);
    while(next_slash_pos != string::npos) {
        string path_so_far = file_path.substr(0, next_slash_pos);
        if(!al_make_directory(path_so_far.c_str())) {
            return false;
        }
        next_slash_pos = file_path.find('/', next_slash_pos + 1);
    }
    
    //Save the file.
    ALLEGRO_FILE* file = al_fopen(file_path.c_str(), "w");
    if(file) {
        if(children_only) {
            for(size_t c = 0; c < children.size(); ++c) {
                children[c]->save_node(
                    file, 0, include_empty_values, encrypted
                );
            }
        } else {
            save_node(file, 0, include_empty_values, encrypted);
        }
        al_fclose(file);
        return true;
    } else {
        return false;
    }
}


/**
 * @brief Save a node into a text file.
 *
 * @param file Allegro file handle.
 * @param level Current level of depth.
 * @param include_empty_values If true, even nodes with an empty value
 * will be saved.
 * @param encrypted If true, the file must be encrypted.
 */
void data_node::save_node(
    ALLEGRO_FILE* file, const size_t level,
    const bool include_empty_values, const bool encrypted
) const {

    string tabs_str(level, '\t');
    if(encrypted) tabs_str.clear();
    string name_str = name;
    if(encrypted) data_node::encrypt_string(name_str);
    string block_start_str = "{\n";
    if(encrypted) data_node::encrypt_string(block_start_str);
    string block_end_str = "}";
    if(encrypted) data_node::encrypt_string(block_end_str);
    string value_str = "=" + value;
    if(encrypted) data_node::encrypt_string(value_str);
    string newline_str = "\n";
    if(encrypted) data_node::encrypt_string(newline_str);
    
    al_fwrite(file, tabs_str.c_str(), tabs_str.size());
    al_fwrite(file, name_str.c_str(), name_str.size());
    
    if(!children.empty()) {
        al_fwrite(file, block_start_str.c_str(), block_start_str.size());
        for(size_t c = 0; c < children.size(); ++c) {
            children[c]->save_node(
                file, level + 1, include_empty_values, encrypted
            );
        }
        al_fwrite(file, tabs_str.c_str(), tabs_str.size());
        al_fwrite(file, block_end_str.c_str(), block_end_str.size());
        
    } else if(!value.empty() || include_empty_values) {
        al_fwrite(file, value_str.c_str(), value_str.size());
    }
    al_fwrite(file, newline_str.c_str(), newline_str.size());
    
}


/**
 * @brief Removes all trailing and preceding spaces.
 * This means space and tab characters before and after the 'middle' characters.
 *
 * @param s The original string.
 * @param left_only If true, only trim the spaces at the left.
 * @return The trimmed string.
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
