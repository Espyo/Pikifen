/* © André Luís Gomes da Silva ([SRB2]Espyo)
 * This file belongs to the source code of an open-source rhythm game, PK Rhythm.
 * Please read the help files that come with the game, or go to http://pkrhythm.sf.net for more info.
 ***
 * Data file loading and saving.
 * It's easy to use. Create a data_node variable, use .load_file() to load a file,
 * and to access nodes, just go like [node][sub-node][sub-node]...
 * 
 * First off, what is a node? A node is either something that contains a value or a section with sub-nodes.
 * For 'game_fps=30', the node is 'game_fps', its value is '30'.
 * For 'enemy{', the node is 'enemy', and it contains the nodes that can be found in the next lines,
 * until it's closed with a matching '}'.
 * 
 * That said, you can find the same node name appearing several times - most commonly section nodes.
 * e.g. 'tempo{...} tempo{...} tempo{...}'. Each of these is called an instance.
 * To access an instance, when you specify the name of the node, you should also specify the instance number.
 * So for instance, '["tempo"][1]' to access the 2nd instance of all 'tempo' nodes.
 *
 * If you want to get the value of a node, when you specify it, just type .value.
 * There's a special thing you can do, though. If it's a node that you only expect to exist once (1 instance),
 * you won't need to specify the instance number (0). You can just specify the node and type .get_value().
 * 
 * An example:
 * file=Song.ogg
 * enemy{
 *    boss=1
 *    bg1{
 *       distortion=3
 *    }
 * }
 * enemy{
 *    boss=1
 *    bg1{
 *       distortion=3
 *    }
 * }
 * 
 * To access the first enemy's 'is boss?' boolean, it's just ["enemy"]["boss"].get_value().
 * To access the filename ('file' node's value), it's just ["file"].get_value().
 * To access the second enemy's top background's distortion, it's just ["enemy"][1]["bg1"]["distortion"].get_value().
 * 
 * Some final things about data files: trailing spaces and tabs to the left and right of a node name are ignored,
 * so use those to format your files in a pretty way (to align all strings, to indent sub-nodes).
 * Any line starting with two forward slashes ('//'), even after trailing spaces and tabs, are ignored.
 * A section must start on its own line, have its contents one per line on the following lines,
 * and the matching, closing bracket should be on a line of its own.
 */

#ifndef DATA_FILE_H
#define DATA_FILE_H

#include <map>
#include <string>
#include <vector>

using namespace std;

#define UTF8_MAGIC_NUMBER "\xEF\xBB\xBF"

//ToDo error management.

class data_node;

class data_node_list{
private:
	vector<data_node> list;						//List of nodes (instances).
	vector<data_node> dummy_nodes;				//Create empty nodes here, and return them in case of error (this way, values can return empty strings).
public:
	string get_value(string def_value="");		//The same as getting the value of the first node in the list; a shortcut that's useful if there's only meant to be one instance to begin with.
	data_node& operator[](size_t nr);			//Accesses an element in the list of instances.
	data_node_list& operator[](string name);	//Accesses the first instance's node instances.
	size_t size();								//Number of instances.

	void add();									//Adds an instance.
	data_node& last();							//Returns the last instance added; useful for when you've created a node, and want to actually give it stuff.

	data_node_list();
	data_node_list(const data_node_list &dnl);
	~data_node_list();
};

class data_node{
private:
	map<string, data_node_list> nodes;			//Instances of each of its nodes.
	vector<data_node_list> dummy_lists;			//Create empty node lists here, and return them in case of error (this way, values can return empty strings).
public:
	bool file_was_opened;						//If we opened this with a filename, this'll tell us if it was successful.

	string value;								//Value of this node, if it has it (node=value).

	data_node_list& operator[](string name);		//Accesses a sub-node's list of instances.
	size_t size();									//Number of nodes it contains.
	data_node_list& get_node_list_by_nr(size_t nr, string* name=NULL);	//Accesses a list of instances, by its number.

	void load_file(string filename);			//Loads a file by filename, recursively loading every node.
	size_t load_node(vector<string> lines, size_t start_line=0);	//Loads an individual node.
	
	data_node();
	data_node(string filename);					//Create a node, and load it with a file right away.
	data_node(const data_node &dn);
	~data_node();
};

void getline(ALLEGRO_FILE* file, string &line);
string trim_trailings(string s, bool left_only=false);

#endif //ifndef DATA_FILE_H