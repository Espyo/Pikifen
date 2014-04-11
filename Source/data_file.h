/**
 * Data file.
 * A data file is composed of nodes, that can either have a value, or children nodes.
 * In the text file, each line represents something.
 ** A line starting with "//" is a comment, and is ignored.
 ** A line like "option = value" is a node with a value.
 ** A line like "option {" is a node with children nodes.
 *** The children of this node are declared in the following lines, until a matching "}" is found.
 *
 * Example of a data text file, imagine a file that houses the data for all levels:
 ** level {
 **     gems_needed = 10
 **     objects {
 **         blue_monster {
 **             coords = 20 10
 **             size = 20
 **         }
 **     }
 ** }
 *
 * To get the type and size of the third object of every level, you would do something like:
 ** data_node file("levels.txt");
 ** for(size_t l = 0; l < file.get_nr_of_children_by_name("level"); l++){
 **     data_node* level_objects = file.get_child_by_name("level", l)->get_child_by_name("objects");
 **     for(size_t o = 0; o < level_objects->get_nr_of_children(); o++){
 **         cout << "Type: " << level_objects->get_child(o)->name << "\n";
 **         cout << "Size: " << level_objects->get_child(o)->get_child_by_name("size")->value << "\n";
 **     }
 ** }
 */

#ifndef DATA_FILE_INCLUDED
#define DATA_FILE_INCLUDED

#include <string>
#include <vector>

using namespace std;

#define UTF8_MAGIC_NUMBER "\xEF\xBB\xBF"

class data_node {
private:
    vector<data_node*> children;
    vector<data_node*> dummy_children;
    data_node* create_dummy();
    
public:
    string name;    //The node's name.
    string value;   //And its value.
    
    bool file_was_opened; //True if the node or parent(s) was created from a file that was opened successfuly.
    string filename;      //Full file name of the file used to open this node or its parent(s).
    size_t line_nr;       //Line on the text file this node's in.
    
    string get_value_or_default(string def);
    
    size_t get_nr_of_children();
    data_node* get_child(size_t number);
    size_t get_nr_of_children_by_name(string name);
    data_node* get_child_by_name(string name, size_t occurrence_number = 0);
    
    size_t add(data_node* new_node);
    bool remove(data_node* node_to_remove);
    
    void load_file(string filename, bool trim_values = true);
    size_t load_node(vector<string> &lines, bool trim_values, size_t start_line = 0);
    bool save_file(string filename = "", bool children_only = true);
    void save_node(ALLEGRO_FILE* file, size_t level = 0);
    
    data_node();
    data_node(string filename);
    data_node(string name, string value);
    data_node(const data_node &dn2);
    ~data_node();
};

void getline(ALLEGRO_FILE* file, string &line);
string trim_spaces(string s, bool left_only = false);

#endif //ifndef DATA_FILE_INCLUDED