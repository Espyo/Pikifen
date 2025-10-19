/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Header for the data file class and related functions.
 *
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
 *   dataNode file("levels.txt");
 *   for(size_t l = 0; l < file.getNrOfChildrenByName("level"); l++) {
 *       DataNode* levelObjects =
 *           file.getChildByName("level", l)->getChildByName("objects");
 *       for(size_t o = 0; o < levelObjects->getNrOfChildren(); o++){
 *           string type = levelObjects->getChild(o)->name;
 *           string size =
 *              levelObjects->getChild(o)->getChildByName("size")->value;
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
    bool fileWasOpened = false;
    
    //File path of the file used to open this node or its parent(s), if any.
    string filePath;
    
    //Line on the text file this node's in.
    size_t lineNr = 0;
    
    
    //--- Function declarations ---
    
    DataNode();
    explicit DataNode(const string& filePath);
    DataNode(const string& name, const string& value);
    DataNode(const DataNode& dn2);
    DataNode& operator=(const DataNode& dn2);
    ~DataNode();
    void clear();
    void clearChildren();
    string getValueOrDefault(const string& def) const;
    size_t getNrOfChildren() const;
    DataNode* getChild(size_t number);
    size_t getNrOfChildrenByName(const string& name) const;
    DataNode* getChildByName(
        const string& name, size_t occurrenceNumber = 0
    );
    size_t add(DataNode* newNode);
    DataNode* addNew(const string& name, const string& value = "");
    bool remove(DataNode* nodeToRemove);
    void loadFile(
        const string& filePath,
        bool trimValues = true,
        bool namesOnlyAfterRoot = false,
        bool encrypted = false
    );
    size_t loadNode(
        const vector<string>& lines, bool trimValues,
        size_t startLine = 0, size_t depth = 0,
        bool namesOnlyAfterRoot = false
    );
    bool saveFile(
        string destinationFilePath = "", bool childrenOnly = true,
        bool includeEmptyValues = false,
        bool encrypted = false
    ) const;
    void saveNode(
        ALLEGRO_FILE* file, size_t level = 0,
        bool includeEmptyValues = false,
        bool encrypted = false
    ) const;
    
protected:

    //--- Members ---
    
    //List of children nodes.
    vector<DataNode*> children;
    
    //Dummy children, returned upon error.
    vector<DataNode*> dummyChildren;
    
    
    //--- Function declarations ---
    
    DataNode* createDummy();
    static unsigned char decryptChar(unsigned char c);
    static unsigned char encryptChar(unsigned char c);
    static void encryptString(string& s);
    static void getline(
        ALLEGRO_FILE* file, string& line, bool encrypted = false
    );
    static string trimSpaces(const string& s, bool leftOnly = false);
    
};
