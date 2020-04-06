/*
 * Copyright (c) Andre 'Espyo' Silva; no license, feel free to use!
 *
 * Data file.
 * A data file is composed of nodes, that can either have
 * a value, or children nodes.
 * In the text file, each line represents something.
 *   A line starting with "//" is a comment, and is ignored.
 *   A line like "option = value" is a node with a value.
 *   A line like "option {" is a node with children nodes.
 *     The children of this node are declared in the following lines,
 *     until a matching "}" is found.
 *
 * Example of a data text file, imagine a file that houses
 * the data for all levels:
 *   level {
 *       gems_needed = 10
 *       objects {
 *           blue_monster {
 *               coords = 20 10
 *               size = 20
 *           }
 *       }
 *   }
 *
 * To get the type and size of the third object of every level,
 * you would do something like:
 *   data_node file("levels.txt");
 *   for(size_t l = 0; l < file.get_nr_of_children_by_name("level"); ++l) {
 *       data_node* level_objects =
 *           file.get_child_by_name("level", l)->get_child_by_name("objects");
 *       for(size_t o = 0; o < level_objects->get_nr_of_children(); ++o){
 *           string type = level_objects->get_child(o)->name;
 *           string size =
 *              level_objects->get_child(o)->get_child_by_name("size")->value;
 *       }
 *   }
 */

#ifndef DATA_FILE_INCLUDED
#define DATA_FILE_INCLUDED

#include <allegro5/allegro.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

const string UTF8_MAGIC_NUMBER = "\xEF\xBB\xBF";



/* ----------------------------------------------------------------------------
 * A node of data. Nodes may contain a value, and/or a list of children nodes.
 */
class data_node {
private:
    vector<data_node*> children;
    vector<data_node*> dummy_children;
    data_node* create_dummy();
    
    string trim_spaces(const string &s, const bool left_only = false);
    
public:
    string name;    //The node's name.
    string value;   //And its value.
    
    //True if the node or parent(s) was created from a file
    //that was opened successfuly.
    bool file_was_opened;
    //Full file name of the file used to open this node or its parent(s).
    string file_name;
    //Line on the text file this node's in.
    size_t line_nr;
    
    string get_value_or_default(const string &def);
    
    size_t get_nr_of_children();
    data_node* get_child(const size_t number);
    size_t get_nr_of_children_by_name(const string &name);
    data_node* get_child_by_name(
        const string &name, const size_t occurrence_number = 0
    );
    
    size_t add(data_node* new_node);
    bool remove(data_node* node_to_remove);
    
    void load_file(
        const string &file_name,
        const bool trim_values = true,
        const bool names_only_after_root = false
    );
    size_t load_node(
        const vector<string> &lines, const bool trim_values,
        const size_t start_line = 0, const size_t depth = 0,
        const bool names_only_after_root = false
    );
    bool save_file(
        string file_name = "", const bool children_only = true,
        const bool include_empty_values = false
    );
    void save_node(
        ALLEGRO_FILE* file, const size_t level = 0,
        const bool include_empty_values = false
    );
    
    data_node();
    explicit data_node(const string &file_name);
    data_node(const string &name, const string &value);
    data_node(const data_node &dn2);
    const data_node &operator=(const data_node &dn2);
    ~data_node();
};



void getline(ALLEGRO_FILE* file, string &line);

#endif //ifndef DATA_FILE_INCLUDED
