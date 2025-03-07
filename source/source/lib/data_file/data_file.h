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
 *   for(size_t l = 0; l < file.get_nr_of_children_by_name("level"); l++) {
 *       DataNode* level_objects =
 *           file.get_child_by_name("level", l)->get_child_by_name("objects");
 *       for(size_t o = 0; o < level_objects->get_nr_of_children(); o++){
 *           string type = level_objects->get_child(o)->name;
 *           string size =
 *              level_objects->get_child(o)->get_child_by_name("size")->value;
 *       }
 *   }
 */

#pragma once

#include <allegro5/allegro.h>

#include <string>
#include <vector>


using std::string;
using std::vector;


namespace DATA_FILE {
extern const unsigned char ENCRYPTION_MIN_VALUE;
extern const unsigned char ENCRYPTION_ROT_AMOUNT;
extern const string UTF8_MAGIC_NUMBER;
}


/**
 * @brief A node of data. Nodes may contain a value,
 * and/or a list of children nodes.
 */
class DataNode {

public:

    //--- Members ---
    
    //The node's name.
    string name;
    
    //The node's value.
    string value;
    
    //True if the node or parent(s) was created from a file
    //that was opened successfully.
    bool file_was_opened = false;
    
    //File path of the file used to open this node or its parent(s), if any.
    string file_path;
    
    //Line on the text file this node's in.
    size_t line_nr = 0;
    
    
    //--- Function declarations ---
    
    DataNode();
    explicit DataNode(const string &file_path);
    DataNode(const string &name, const string &value);
    DataNode(const DataNode &dn2);
    DataNode &operator=(const DataNode &dn2);
    ~DataNode();
    void clear();
    string get_value_or_default(const string &def) const;
    size_t get_nr_of_children() const;
    DataNode* get_child(size_t number);
    size_t get_nr_of_children_by_name(const string &name) const;
    DataNode* get_child_by_name(
        const string &name, size_t occurrence_number = 0
    );
    size_t add(DataNode* new_node);
    bool remove(DataNode* node_to_remove);
    void load_file(
        const string &file_path,
        bool trim_values = true,
        bool names_only_after_root = false,
        bool encrypted = false
    );
    size_t load_node(
        const vector<string> &lines, bool trim_values,
        size_t start_line = 0, size_t depth = 0,
        bool names_only_after_root = false
    );
    bool save_file(
        string destination_file_path = "", bool children_only = true,
        bool include_empty_values = false,
        bool encrypted = false
    ) const;
    void save_node(
        ALLEGRO_FILE* file, size_t level = 0,
        bool include_empty_values = false,
        bool encrypted = false
    ) const;
    
private:

    //--- Members ---
    
    //List of children nodes.
    vector<DataNode*> children;
    
    //Dummy children, returned upon error.
    vector<DataNode*> dummy_children;
    
    
    //--- Function declarations ---
    
    DataNode* create_dummy();
    static unsigned char decrypt_char(unsigned char c);
    static unsigned char encrypt_char(unsigned char c);
    static void encrypt_string(string &s);
    static void getline(
        ALLEGRO_FILE* file, string &line, bool encrypted = false
    );
    static string trim_spaces(const string &s, bool left_only = false);
    
};
