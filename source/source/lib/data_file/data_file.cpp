/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 *
 * === FILE DESCRIPTION ===
 * Data file class and related functions.
 *
 * Please read the header file for more information.
 */

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
DataNode::DataNode() {

}


/**
 * @brief Constructs a new data node object, using the data and creating a copy
 * of the children from another node.
 *
 * @param dn2 The node to copy data from.
 */
DataNode::DataNode(const DataNode& dn2) :
    name(dn2.name),
    value(dn2.value),
    fileWasOpened(dn2.fileWasOpened),
    filePath(dn2.filePath),
    lineNr(dn2.lineNr) {
    
    for(size_t c = 0; c < dn2.children.size(); c++) {
        children.push_back(new DataNode(*(dn2.children[c])));
    }
    for(size_t dc = 0; dc < dn2.dummyChildren.size(); dc++) {
        dummyChildren.push_back(new DataNode(*(dn2.dummyChildren[dc])));
    }
}


/**
 * @brief Constructs a new data node object from a file, given the file name.
 *
 * @param filePath Name of the file to load.
 */
DataNode::DataNode(const string& filePath) :
    filePath(filePath) {
    
    loadFile(filePath);
}


/**
 * @brief Constructs a new data node object by filling its name and value.
 *
 * @param name The node's name.
 * @param value Its value.
 */
DataNode::DataNode(const string& name, const string& value) :
    name(name),
    value(value) {
    
}


/**
 * @brief Destroys the data node object and all the children within.
 *
 */
DataNode::~DataNode() {
    clear();
}


/**
 * @brief Adds a new child to the list.
 *
 * @param newNode The node to add.
 * @return The new child's index.
 */
size_t DataNode::add(DataNode* newNode) {
    children.push_back(newNode);
    return children.size() - 1;
}


/**
 * @brief Creates a new node and adds it as a child.
 *
 * @param name Name of the new node.
 * @param value Value of the new node.
 * @return The new node.
 */
DataNode* DataNode::addNew(const string& name, const string& value) {
    DataNode* newNode = new DataNode(name, value);
    add(newNode);
    return newNode;
}


/**
 * @brief Clears the data inside a node.
 */
void DataNode::clear() {
    name.clear();
    value.clear();
    fileWasOpened = false;
    filePath.clear();
    lineNr = 0;
    clearChildren();
}


/**
 * @brief Clears the children data inside a node.
 */
void DataNode::clearChildren() {
    for(size_t c = 0; c < children.size(); c++) {
        delete children[c];
    }
    children.clear();
    
    for(size_t dc = 0; dc < dummyChildren.size(); dc++) {
        delete dummyChildren[dc];
    }
    dummyChildren.clear();
}


/**
 * @brief Creates a dummy node. If the programmer requests an invalid node,
 * a dummy is returned.
 *
 * @return The dummy node.
 */
DataNode* DataNode::createDummy() {
    DataNode* newDummyChild = new DataNode();
    newDummyChild->lineNr = lineNr;
    newDummyChild->filePath = filePath;
    newDummyChild->fileWasOpened = fileWasOpened;
    dummyChildren.push_back(newDummyChild);
    return newDummyChild;
}


/**
 * @brief "Decrypts" a character for loading an encrypted data file.
 *
 * See encryptChar() for more info.
 *
 * @param c Character to decrypt.
 * @return The decrypted character.
 */
unsigned char DataNode::decryptChar(unsigned char c) {
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
unsigned char DataNode::encryptChar(unsigned char c) {
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
 * See encryptChar() for more info.
 *
 * @param s String to encrypt.
 */
void DataNode::encryptString(string& s) {
    for(size_t c = 0; c < s.size(); c++) {
        s[c] = encryptChar(s[c]);
    }
}


/**
 * @brief Returns a child node given its number on the list
 * (direct children only).
 *
 * @param number The index number of the child.
 * @return The node.
 */
DataNode* DataNode::getChild(size_t number) {
    if(number >= children.size()) return createDummy();
    return children[number];
}


/**
 * @brief Returns the nth child with this name on the list
 * (direct children only).
 *
 * @param name The name the child must have.
 * @param occurrenceNr This function will return the nth child with
 * the specified name.
 * @return The node.
 */
DataNode* DataNode::getChildByName(
    const string& name, size_t occurrenceNr
) {
    size_t curOccurrenceNr = 0;
    
    for(size_t c = 0; c < children.size(); c++) {
        if(name == children[c]->name) {
            if(curOccurrenceNr == occurrenceNr) {
                //We found it.
                return children[c];
            } else {
                curOccurrenceNr++;
            }
        }
    }
    
    return createDummy();
}


/**
 * @brief Returns the first child with this name on the list
 * (direct children only). If it doesn't exist, creates it, adds it to the list,
 * and then returns it.
 *
 * @param name The name the child must have.
 * @return The node.
 */
DataNode* DataNode::getChildOrAddNew(const string& name) {
    for(size_t c = 0; c < children.size(); c++) {
        if(name == children[c]->name) {
            return children[c];
        }
    }
    return addNew(name);
}


/**
 * @brief Like an std::getline(), but for ALLEGRO_FILE*.
 *
 * @param file Allegro file handle.
 * @param line String to save the line into.
 * @param encrypted If true, the document is encrypted and needs decrypting.
 */
void DataNode::getline(
    ALLEGRO_FILE* file, string& line, bool encrypted
) {
    line.clear();
    if(!file) {
        return;
    }
    
    size_t bytesRead;
    char* cPtr = new char;
    
    bytesRead = al_fread(file, cPtr, 1);
    while(bytesRead > 0) {
        unsigned char c = *((unsigned char*) cPtr);
        
        if(encrypted) {
            c = decryptChar(c);
        }
        
        if(c == '\r') {
            //Let's check if the next character is a \n. If so, they should
            //both be consumed by al_fread().
            bytesRead = al_fread(file, cPtr, 1);
            unsigned char peekC = *((unsigned char*) cPtr);
            if(encrypted) {
                peekC = decryptChar(peekC);
            }
            if(bytesRead > 0) {
                if(peekC == '\n') {
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
        
        bytesRead = al_fread(file, cPtr, 1);
    }
    
    delete cPtr;
}


/**
 * @brief Returns the number of children nodes (direct children only).
 *
 * @return The number.
 */
size_t DataNode::getNrOfChildren() const {
    return children.size();
}


/**
 * @brief Returns the number of occurrences of a child name
 * (direct children only).
 *
 * @param name Name the children must have.
 * @return The number.
 */
size_t DataNode::getNrOfChildrenByName(const string& name) const {
    size_t number = 0;
    
    for(size_t c = 0; c < children.size(); c++) {
        if(name == children[c]->name) number++;
    }
    
    return number;
}


/**
 * @brief Returns the value of a node, or def if it has no value.
 *
 * @param def Default value.
 * @return The value.
 */
string DataNode::getValueOrDefault(const string& def) const {
    return (value.empty() ? def : value);
}


/**
 * @brief Loads data from a file.
 *
 * @param filePath Path to the file to load.
 * @param trimValues If true, spaces before and after the value will
 * be trimmed off.
 * @param namesOnlyAfterRoot If true, any nodes that are not in the
 * root node (i.e. they are children of some node inside the file)
 * will only have a name and no value; the entire contents of their
 * line will be their name.
 * @param encrypted If true, the file is encrypted, and needs decrypting.
 */
void DataNode::loadFile(
    const string& filePath, bool trimValues,
    bool namesOnlyAfterRoot, bool encrypted
) {
    vector<string> lines;
    
    fileWasOpened = false;
    this->filePath = filePath;
    
    ALLEGRO_FILE* file = al_fopen(filePath.c_str(), "r");
    if(file) {
        bool isFirstLine = true;
        fileWasOpened = true;
        string line;
        while(!al_feof(file)) {
            getline(file, line, encrypted);
            
            if(isFirstLine && !encrypted) {
                //Let's just check if it starts with the UTF-8 Magic Number.
                if(
                    line.size() >= 3 &&
                    line.substr(0, 3) == DATA_FILE::UTF8_MAGIC_NUMBER
                ) {
                    line = line.erase(0, 3);
                }
            }
            lines.push_back(line);
            isFirstLine = false;
        }
        al_fclose(file);
    }
    
    loadNode(lines, trimValues, 0, 0, namesOnlyAfterRoot);
}


/**
 * @brief Loads data from a list of text lines.
 *
 * @param lines Text lines that make up the node.
 * @param trimValues If true, spaces before and after the value will
 * be trimmed off.
 * @param startLine This node starts at this line of the document.
 * @param depth Depth of this node. 0 means root.
 * @param namesOnlyAfterRoot If true, any nodes that are not in the
 * root node (i.e. they are children of some node inside the file)
 * will only have a name and no value; the entire contents of their
 * line will be their name.
 * @return Returns the number of the line this node ended on,
 * judging by startLine. This is used for the recursion.
 */
size_t DataNode::loadNode(
    const vector<string>& lines, bool trimValues,
    size_t startLine, size_t depth,
    bool namesOnlyAfterRoot
) {
    children.clear();
    
    if(startLine >= lines.size()) return startLine;
    
    bool returningFromSubNode = false;
    
    for(size_t l = startLine; l < lines.size(); l++) {
        string line = lines[l];
        
        line = trimSpaces(line, true); //Removes the leftmost spaces.
        
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
            if(returningFromSubNode) {
                //The sub-node just ended.
                //Let's leave what's after the bracket, and let the rest
                //of the code make use of it.
                returningFromSubNode = false;
                line = line.substr(pos + 1, line.size() - (pos + 1));
                line = trimSpaces(line, true);
                if(line.empty()) continue;
            } else {
                return l;
            }
        }
        
        //Sub-node start.
        pos = line.find('{');
        if(pos != string::npos) {
        
            DataNode* newChild = new DataNode();
            newChild->name = trimSpaces(line.substr(0, pos));
            newChild->value.clear();
            newChild->fileWasOpened = fileWasOpened;
            newChild->filePath = filePath;
            newChild->lineNr = l + 1;
            l =
                newChild->loadNode(
                    lines, trimValues, l + 1, depth + 1, namesOnlyAfterRoot
                );
            l--; //So the block-ending line gets re-examined.
            children.push_back(newChild);
            
            returningFromSubNode = true;
            continue;
        }
        
        //Option=value.
        pos = line.find('=');
        string n, v;
        if(
            (!namesOnlyAfterRoot || depth == 0) &&
            pos != string::npos && pos > 0 && line.size() > 2
        ) {
            n = line.substr(0, pos);
            v = line.substr(pos + 1, line.size() - (pos + 1));
        } else {
            n = line;
        }
        if(trimValues) v = trimSpaces(v);
        
        DataNode* newChild = new DataNode();
        newChild->name = trimSpaces(n);
        newChild->value = v;
        newChild->fileWasOpened = fileWasOpened;
        newChild->filePath = filePath;
        newChild->lineNr = l + 1;
        children.push_back(newChild);
        
    }
    
    return lines.size() - 1;
}


/**
 * @brief Copies data from another data node.
 *
 * @param dn2 Node to copy from.
 * @return The current node.
 */
DataNode& DataNode::operator=(const DataNode& dn2) {
    if(this != &dn2) {
        clear();
        
        name = dn2.name;
        value = dn2.value;
        fileWasOpened = dn2.fileWasOpened;
        filePath = dn2.filePath;
        lineNr = dn2.lineNr;
        
        for(size_t c = 0; c < dn2.children.size(); c++) {
            children.push_back(new DataNode(*(dn2.children[c])));
        }
        for(size_t dc = 0; dc < dn2.dummyChildren.size(); dc++) {
            dummyChildren.push_back(new DataNode(*(dn2.dummyChildren[dc])));
        }
    }
    
    return *this;
}


/**
 * @brief Removes and destroys a child from the list.
 *
 * @param nodeToRemove The node to be removed.
 * @return Whether the node existed.
 */
bool DataNode::remove(DataNode* nodeToRemove) {
    for(size_t c = 0; c < children.size(); c++) {
        if(children[c] == nodeToRemove) {
            delete nodeToRemove;
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
 * @param destinationFilePath Path to the file to save to.
 * @param childrenOnly If true, only save the nodes inside this node.
 * @param includeEmptyValues If true, even nodes with an empty value
 * will be saved.
 * @param encrypted If true, the file must be encrypted.
 * @return Whether it succeeded.
 */
bool DataNode::saveFile(
    string destinationFilePath, bool childrenOnly,
    bool includeEmptyValues, bool encrypted
) const {

    if(destinationFilePath == "") destinationFilePath = this->filePath;
    
    //Create any missing folders.
    size_t nextSlashPos = destinationFilePath.find('/', 0);
    while(nextSlashPos != string::npos) {
        string pathSoFar = destinationFilePath.substr(0, nextSlashPos);
        if(!al_make_directory(pathSoFar.c_str())) {
            return false;
        }
        nextSlashPos = destinationFilePath.find('/', nextSlashPos + 1);
    }
    
    //Save the file.
    ALLEGRO_FILE* file = al_fopen(destinationFilePath.c_str(), "w");
    if(file) {
        if(childrenOnly) {
            for(size_t c = 0; c < children.size(); c++) {
                children[c]->saveNode(
                    file, 0, includeEmptyValues, encrypted
                );
            }
        } else {
            saveNode(file, 0, includeEmptyValues, encrypted);
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
 * @param includeEmptyValues If true, even nodes with an empty value
 * will be saved.
 * @param encrypted If true, the file must be encrypted.
 */
void DataNode::saveNode(
    ALLEGRO_FILE* file, size_t level,
    bool includeEmptyValues, bool encrypted
) const {

    string tabsStr(level, '\t');
    if(encrypted) tabsStr.clear();
    string nameStr = name;
    if(encrypted) DataNode::encryptString(nameStr);
    string blockStartStr = "{\n";
    if(encrypted) DataNode::encryptString(blockStartStr);
    string blockEndStr = "}";
    if(encrypted) DataNode::encryptString(blockEndStr);
    string valueStr = "=" + value;
    if(encrypted) DataNode::encryptString(valueStr);
    string newlineStr = "\n";
    if(encrypted) DataNode::encryptString(newlineStr);
    
    al_fwrite(file, tabsStr.c_str(), tabsStr.size());
    al_fwrite(file, nameStr.c_str(), nameStr.size());
    
    if(!children.empty()) {
        al_fwrite(file, blockStartStr.c_str(), blockStartStr.size());
        for(size_t c = 0; c < children.size(); c++) {
            children[c]->saveNode(
                file, level + 1, includeEmptyValues, encrypted
            );
        }
        al_fwrite(file, tabsStr.c_str(), tabsStr.size());
        al_fwrite(file, blockEndStr.c_str(), blockEndStr.size());
        
    } else if(!value.empty() || includeEmptyValues) {
        al_fwrite(file, valueStr.c_str(), valueStr.size());
    }
    al_fwrite(file, newlineStr.c_str(), newlineStr.size());
    
}


/**
 * @brief Removes all trailing and preceding spaces.
 * This means space and tab characters before and after the 'middle' characters.
 *
 * @param s The original string.
 * @param leftOnly If true, only trim the spaces at the left.
 * @return The trimmed string.
 */
string DataNode::trimSpaces(const string& s, bool leftOnly) {
    string orig = s;
    //Spaces before.
    if(orig.size()) {
        while(orig[0] == ' ' || orig[0] == '\t') {
            orig.erase(0, 1);
            if(orig.empty()) break;
        }
    }
    
    if(!leftOnly) {
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
