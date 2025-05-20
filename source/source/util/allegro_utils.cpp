/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Allegro-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#include <algorithm>
#include <clocale>
#include <vector>

#include <allegro5/allegro_native_dialog.h>

#include "allegro_utils.h"

#include "../core/misc_functions.h"
#include "general_utils.h"
#include "math_utils.h"
#include "string_utils.h"


using std::vector;


/**
 * @brief Checks if two colors are the same.
 *
 * @param c1 First color.
 * @param c2 Second color.
 * @return Whether they are the same.
 */
bool operator==(const ALLEGRO_COLOR& c1, const ALLEGRO_COLOR& c2) {
    if(c1.r != c2.r) return false;
    if(c1.g != c2.g) return false;
    if(c1.b != c2.b) return false;
    return c1.a == c2.a;
}


/**
 * @brief Checks if two colors are different.
 *
 * @param c1 First color.
 * @param c2 Second color.
 * @return Whether they are the same.
 */
bool operator!=(const ALLEGRO_COLOR& c1, const ALLEGRO_COLOR& c2) {
    return !operator==(c1, c2);
}


/**
 * @brief Calls al_fwrite, but with an std::string instead of a c-string.
 *
 * @param f Allegro file.
 * @param s String to write.
 */
void al_fwrite(ALLEGRO_FILE* f, const string& s) {
    al_fwrite(f, s.c_str(), s.size());
}


/**
 * @brief Converts a color to its string representation.
 *
 * @param c Color to convert.
 * @return The string.
 */
string c2s(const ALLEGRO_COLOR& c) {
    return i2s(c.r * 255) + " " + i2s(c.g * 255) + " " + i2s(c.b * 255) +
           (c.a == 1 ? "" : " " + i2s(c.a * 255));
}


/**
 * @brief Returns the color that was provided, but with the alpha changed.
 *
 * @param c The color to change the alpha on.
 * @param a The new alpha, [0-255].
 * @return The new color.
 */
ALLEGRO_COLOR changeAlpha(const ALLEGRO_COLOR& c, unsigned char a) {
    ALLEGRO_COLOR c2;
    c2.r = c.r; c2.g = c.g; c2.b = c.b;
    c2.a = a / 255.0;
    return c2;
}


/**
 * @brief Returns the color provided, but darker or lighter by l amount.
 *
 * @param c The color to change the lighting on.
 * @param l Lighting amount, positive or negative, from 0 to 1.
 * @return The new color.
 */
ALLEGRO_COLOR changeColorLighting(const ALLEGRO_COLOR& c, float l) {
    ALLEGRO_COLOR c2;
    c2.r = std::clamp(c.r + l, 0.0f, 1.0f);
    c2.g = std::clamp(c.g + l, 0.0f, 1.0f);
    c2.b = std::clamp(c.b + l, 0.0f, 1.0f);
    c2.a = c.a;
    return c2;
}


/**
 * @brief Deletes a file on the disk.
 *
 * @param filePath Path to the file.
 * @return A status code.
 */
FS_DELETE_RESULT deleteFile(const string& filePath) {
    //Panic check to make sure nothing went wrong and it's an important file.
    //"", "C:", "C:/, "/", etc. are all 3 characters or fewer, so this works.
    engineAssert(
        filePath.size() >= 4,
        "Tried to delete the file \"" + filePath + "\"!"
    );
    
    ALLEGRO_FS_ENTRY* file = al_create_fs_entry(filePath.c_str());
    if(!file) {
        return FS_DELETE_RESULT_NOT_FOUND;
    }
    
    if(hasFlag(al_get_fs_entry_mode(file), ALLEGRO_FILEMODE_ISDIR)) {
        al_destroy_fs_entry(file);
        return FS_DELETE_RESULT_NOT_FOUND;
    }
    
    if(!al_remove_fs_entry(file)) {
        al_destroy_fs_entry(file);
        return FS_DELETE_RESULT_DELETE_ERROR;
    }
    
    al_destroy_fs_entry(file);
    
    return FS_DELETE_RESULT_OK;
}


/**
 * @brief Returns whether a given file exists.
 *
 * @param path Path to the file.
 * @return Whether it exists.
 */
bool fileExists(const string& path) {
    return al_filename_exists(path.c_str());
}


/**
 * @brief Returns whether a given folder exists.
 *
 * @param path Path to the folder.
 * @return Whether it exists.
 */
bool folderExists(const string& path) {
    bool result = true;
    ALLEGRO_FS_ENTRY* fsEntry = al_create_fs_entry(path.c_str());
    if(!fsEntry || !al_open_directory(fsEntry)) {
        result = false;
    }
    al_close_directory(fsEntry);
    al_destroy_fs_entry(fsEntry);
    return result;
}


/**
 * @brief Stores the names of all files in a folder into a vector.
 *
 * @param folderPath Path to the folder.
 * @param folders If true, only read folders. If false, only read files.
 * @param outFolderFound If not nullptr, whether the folder was
 * found or not is returned here.
 * @return The vector.
 */
vector<string> folderToVector(
    string folderPath, bool folders, bool* outFolderFound
) {
    vector<string> v;
    
    if(folderPath.empty()) {
        if(outFolderFound) *outFolderFound = false;
        return v;
    }
    
    //Normalize the folder's path.
    folderPath = standardizePath(folderPath);
    
    ALLEGRO_FS_ENTRY* folder =
        al_create_fs_entry(folderPath.c_str());
    if(!folder || !al_open_directory(folder)) {
        if(outFolderFound) *outFolderFound = false;
        return v;
    }
    
    
    ALLEGRO_FS_ENTRY* entry = nullptr;
    while((entry = al_read_directory(folder)) != nullptr) {
        if(
            folders ==
            (hasFlag(al_get_fs_entry_mode(entry), ALLEGRO_FILEMODE_ISDIR))
        ) {
        
            string entryName =
                standardizePath(al_get_fs_entry_name(entry));
                
            //Only save what's after the final slash.
            size_t pos = entryName.find_last_of("/");
            if(pos != string::npos) {
                entryName =
                    entryName.substr(pos + 1, entryName.size() - pos - 1);
            }
            
            v.push_back(entryName);
        }
        al_destroy_fs_entry(entry);
    }
    al_close_directory(folder);
    al_destroy_fs_entry(folder);
    
    
    sort(v.begin(), v.end(), [] (const string& s1, const string& s2) -> bool {
        return strToLower(s1) < strToLower(s2);
    });
    
    if(outFolderFound) *outFolderFound = true;
    return v;
}


/**
 * @brief Stores the names of all files in a folder into a vector, but also
 * recursively enters subfolders.
 *
 * @param folderPath Path to the folder.
 * @param folders If true, only read folders. If false, only read files.
 * @param outFolderFound If not nullptr, whether the folder was
 * found or not is returned here.
 * @return The vector.
 */
vector<string> folderToVectorRecursively(
    string folderPath, bool folders, bool* outFolderFound
) {
    //Figure out what subfolders exist, both to add to the list if needed, as
    //well as to navigate recursively.
    vector<string> v;
    bool found;
    vector<string> subfolders = folderToVector(folderPath, true, &found);
    
    if(!found) {
        if(outFolderFound) *outFolderFound = false;
        return v;
    }
    
    //Add the current folder's things.
    if(!folders) {
        vector<string> files = folderToVector(folderPath, false);
        v.insert(v.end(), files.begin(), files.end());
    } else {
        v.insert(v.end(), subfolders.begin(), subfolders.end());
    }
    
    //Go recursively.
    for(size_t s = 0; s < subfolders.size(); s++) {
        vector<string> recursiveResult =
            folderToVectorRecursively(
                folderPath + "/" + subfolders[s], folders
            );
        for(size_t r = 0; r < recursiveResult.size(); r++) {
            v.push_back(subfolders[s] + "/" + recursiveResult[r]);
        }
    }
    
    //Finish.
    if(outFolderFound) *outFolderFound = true;
    return v;
}


/**
 * @brief Returns the width and height of an Allegro bitmap in a
 * point structure.
 *
 * @param bmp Bitmap to check.
 * @return The dimensions.
 */
Point getBitmapDimensions(ALLEGRO_BITMAP* bmp) {
    return
        Point(
            al_get_bitmap_width(bmp),
            al_get_bitmap_height(bmp)
        );
}


/**
 * @brief Returns a name for the specified Allegro keyboard keycode.
 *
 * This basically makes use of al_keycode_to_name, but with some special cases
 * and with some nice capitalization.
 *
 * @param keycode Keycode to check.
 * @param condensed If true, only the key name is returned. If false,
 * some extra disambiguation is returned too, e.g. whether this is
 * the left or right Ctrl.
 * @return The name, or an empty string on error.
 */
string getKeyName(int keycode, bool condensed) {
    switch(keycode) {
    case ALLEGRO_KEY_ESCAPE: {
        return "Esc";
    }
    case ALLEGRO_KEY_INSERT: {
        return "Ins";
    }
    case ALLEGRO_KEY_DELETE: {
        return "Del";
    }
    case ALLEGRO_KEY_PGUP: {
        return "PgUp";
    }
    case ALLEGRO_KEY_PGDN: {
        return "PgDn";
    }
    case ALLEGRO_KEY_PAD_0: {
        return "0 KP";
    }
    case ALLEGRO_KEY_PAD_1: {
        return "1 KP";
    }
    case ALLEGRO_KEY_PAD_2: {
        return "2 KP";
    }
    case ALLEGRO_KEY_PAD_3: {
        return "3 KP";
    }
    case ALLEGRO_KEY_PAD_4: {
        return "4 KP";
    }
    case ALLEGRO_KEY_PAD_5: {
        return "5 KP";
    }
    case ALLEGRO_KEY_PAD_6: {
        return "6 KP";
    }
    case ALLEGRO_KEY_PAD_7: {
        return "7 KP";
    }
    case ALLEGRO_KEY_PAD_8: {
        return "8 KP";
    }
    case ALLEGRO_KEY_PAD_9: {
        return "9 KP";
    }
    case ALLEGRO_KEY_PAD_ASTERISK: {
        return "* KP";
    }
    case ALLEGRO_KEY_PAD_DELETE: {
        return "Del KP";
    }
    case ALLEGRO_KEY_PAD_ENTER: {
        return "Enter KP";
    }
    case ALLEGRO_KEY_PAD_EQUALS: {
        return "= KP";
    }
    case ALLEGRO_KEY_PAD_MINUS: {
        return "- KP";
    }
    case ALLEGRO_KEY_PAD_PLUS: {
        return "+ KP";
    }
    case ALLEGRO_KEY_PAD_SLASH: {
        return "/ KP";
    }
    case ALLEGRO_KEY_LSHIFT: {
        if(!condensed) {
            return "Shift (left)";
        } else {
            return "Shift";
        }
    }
    case ALLEGRO_KEY_RSHIFT: {
        if(!condensed) {
            return "Shift (right)";
        } else {
            return "Shift";
        }
    }
    case ALLEGRO_KEY_ALT: {
        return "Alt";
    }
    case ALLEGRO_KEY_ALTGR: {
        return "AltGr";
    }
    case ALLEGRO_KEY_LCTRL: {
        if(!condensed) {
            return "Ctrl (left)";
        } else {
            return "Ctrl";
        }
    }
    case ALLEGRO_KEY_RCTRL: {
        if(!condensed) {
            return "Ctrl (right)";
        } else {
            return "Ctrl";
        }
    }
    case ALLEGRO_KEY_BACKSLASH:
    case ALLEGRO_KEY_BACKSLASH2: {
        return "\\";
    }
    case ALLEGRO_KEY_BACKSPACE: {
        if(!condensed) {
            return "Backspace";
        } else {
            return "BkSpc";
        }
    }
    case ALLEGRO_KEY_ENTER: {
        return "Enter";
    }
    }
    string name = strToTitle(al_keycode_to_name(keycode));
    for(size_t c = 0; c < name.size(); c++) {
        if(name[c] == '_') name[c] = ' ';
    }
    return name;
}


/**
 * @brief Returns whether any Shift key, any Ctrl key, and any Alt key
 * is pressed.
 *
 * @param outShiftState If not nullptr,
 * whether Shift is pressed is returned here.
 * @param outCtrlState If not nullptr,
 * whether Ctrl is pressed is returned here.
 * @param outAltState If not nullptr,
 * whether Alt is pressed is returned here.
 */
void getShiftCtrlAltState(
    bool* outShiftState, bool* outCtrlState, bool* outAltState
) {
    ALLEGRO_KEYBOARD_STATE keyboardState;
    al_get_keyboard_state(&keyboardState);
    if(outShiftState) {
        *outShiftState =
            al_key_down(&keyboardState, ALLEGRO_KEY_LSHIFT) ||
            al_key_down(&keyboardState, ALLEGRO_KEY_RSHIFT);
    }
    if(outCtrlState) {
        *outCtrlState =
            al_key_down(&keyboardState, ALLEGRO_KEY_LCTRL) ||
            al_key_down(&keyboardState, ALLEGRO_KEY_RCTRL) ||
            al_key_down(&keyboardState, ALLEGRO_KEY_COMMAND);
    }
    if(outAltState) {
        *outAltState =
            al_key_down(&keyboardState, ALLEGRO_KEY_ALT) ||
            al_key_down(&keyboardState, ALLEGRO_KEY_ALTGR);
    }
}


/**
 * @brief Like an std::getline(), but for ALLEGRO_FILE*.
 *
 * @param file Allegro file handle.
 * @param line String to save the line into.
 */
void getline(ALLEGRO_FILE* file, string& line) {
    line.clear();
    if(!file) {
        return;
    }
    
    size_t bytesRead;
    char* cPtr = new char;
    
    bytesRead = al_fread(file, cPtr, 1);
    while(bytesRead > 0) {
        unsigned char c = *((unsigned char*) cPtr);
        
        if(c == '\r') {
            //Let's check if the next character is a \n. If so, they should
            //both be consumed by al_fread().
            bytesRead = al_fread(file, cPtr, 1);
            unsigned char peekC = *((unsigned char*) cPtr);
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
 * @brief Returns the interpolation between two colors, given a number in
 * an interval.
 *
 * @param input The input number.
 * @param inputStart Start of the interval the input number falls on,
 * inclusive. The closer to inputStart, the closer the output is
 * to outputStart.
 * @param inputEnd End of the interval the number falls on, inclusive.
 * @param outputStart Color on the starting tip of the interpolation.
 * @param outputEnd Color on the ending tip of the interpolation.
 * @return The interpolated color.
 */
ALLEGRO_COLOR interpolateColor(
    float input, float inputStart, float inputEnd,
    const ALLEGRO_COLOR& outputStart, const ALLEGRO_COLOR& outputEnd
) {
    float progress =
        (float) (input - inputStart) / (float) (inputEnd - inputStart);
    return
        al_map_rgba_f(
            outputStart.r + progress * (outputEnd.r - outputStart.r),
            outputStart.g + progress * (outputEnd.g - outputStart.g),
            outputStart.b + progress * (outputEnd.b - outputStart.b),
            outputStart.a + progress * (outputEnd.a - outputStart.a)
        );
}


/**
 * @brief Creates and opens an Allegro native file dialog.
 *
 * @param initialPath Initial path for the dialog.
 * @param title Title of the dialog.
 * @param patterns File name patterns to match, separated by semicolon.
 * @param mode al_create_native_file_dialog mode flags.
 * @param display Allegro display the box belongs to.
 * @return The user's choice(s).
 */
vector<string> promptFileDialog(
    const string& initialPath, const string& title,
    const string& patterns, int mode, ALLEGRO_DISPLAY* display
) {
    ALLEGRO_FILECHOOSER* dialog =
        al_create_native_file_dialog(
            initialPath.c_str(), title.c_str(),
            patterns.c_str(), mode
        );
    al_show_native_file_dialog(display, dialog);
    
    //Reset the locale, which gets set by Allegro's native dialogs...
    //and breaks s2f().
    setlocale(LC_ALL, "C");
    
    vector<string> result;
    size_t nChoices = al_get_native_file_dialog_count(dialog);
    for(size_t c = 0; c < nChoices; c++) {
        result.push_back(
            standardizePath(
                al_get_native_file_dialog_path(dialog, c)
            )
        );
    }
    
    al_destroy_native_file_dialog(dialog);
    return result;
}


/**
 * @brief Creates and opens an Allegro native file dialog, but confines
 * the results to a specific folder.
 *
 * The result pointer returns FILE_DIALOG_RESULT_SUCCESS on success,
 * FILE_DIALOG_RESULT_WRONG_FOLDER if the one or more choices do not belong to
 * the specified folder, and FILE_DIALOG_RESULT_CANCELED if the user canceled.
 * The list of choices that are returned only have the file name, not the
 * rest of the path. Choices can also be contained inside subfolders of the
 * specified folder.
 *
 * @param folderPath The folder to lock to, without the ending slash.
 * @param title Title of the dialog.
 * @param patterns File name patterns to match, separated by semicolon.
 * @param mode al_create_native_file_dialog mode flags.
 * @param result The result of the dialog is returned here.
 * @param display Allegro display the box belongs to.
 * @return The user's choice(s).
 */
vector<string> promptFileDialogLockedToFolder(
    const string& folderPath, const string& title,
    const string& patterns, int mode, FILE_DIALOG_RESULT* result,
    ALLEGRO_DISPLAY* display
) {
    vector<string> f =
        promptFileDialog(folderPath + "/", title, patterns, mode, display);
        
    if(f.empty() || f[0].empty()) {
        *result = FILE_DIALOG_RESULT_CANCELED;
        return vector<string>();
    }
    
    for(size_t fi = 0; fi < f.size(); fi++) {
        size_t folderPos = f[0].find(folderPath);
        if(folderPos == string::npos) {
            //This isn't in the specified folder!
            *result = FILE_DIALOG_RESULT_WRONG_FOLDER;
            return vector<string>();
        } else {
            f[fi] =
                f[fi].substr(folderPos + folderPath.size() + 1, string::npos);
        }
    }
    
    *result = FILE_DIALOG_RESULT_SUCCESS;
    return f;
}


/**
 * @brief Basically, it destroys and recreates a bitmap.
 * The main purpose of this is to update its mipmap.
 *
 * @param b The bitmap.
 * @return The recreated bitmap.
 */
ALLEGRO_BITMAP* recreateBitmap(ALLEGRO_BITMAP* b) {
    ALLEGRO_BITMAP* fixedMipmap = al_clone_bitmap(b);
    al_destroy_bitmap(b);
    return fixedMipmap;
}


/**
 * @brief Converts a string to an Allegro color.
 * Components are separated by spaces, and the final one (alpha) is optional.
 *
 * @param s String to convert.
 * @return The color.
 */
ALLEGRO_COLOR s2c(const string& s) {
    string s2 = s;
    s2 = trimSpaces(s2);
    
    unsigned char alpha = 255;
    vector<string> components = split(s2);
    if(components.size() >= 2) alpha = s2i(components[1]);
    
    if(s2 == "nothing") return al_map_rgba(0,   0,   0,   0);
    if(s2 == "none")    return al_map_rgba(0,   0,   0,   0);
    if(s2 == "black")   return al_map_rgba(0,   0,   0,   alpha);
    if(s2 == "gray")    return al_map_rgba(128, 128, 128, alpha);
    if(s2 == "grey")    return al_map_rgba(128, 128, 128, alpha);
    if(s2 == "white")   return mapAlpha(alpha);
    if(s2 == "yellow")  return al_map_rgba(255, 255, 0,   alpha);
    if(s2 == "orange")  return al_map_rgba(255, 128, 0,   alpha);
    if(s2 == "brown")   return al_map_rgba(128, 64,  0,   alpha);
    if(s2 == "red")     return al_map_rgba(255, 0,   0,   alpha);
    if(s2 == "violet")  return al_map_rgba(255, 0,   255, alpha);
    if(s2 == "purple")  return al_map_rgba(128, 0,   255, alpha);
    if(s2 == "blue")    return al_map_rgba(0,   0,   255, alpha);
    if(s2 == "cyan")    return al_map_rgba(0,   255, 255, alpha);
    if(s2 == "green")   return al_map_rgba(0,   255, 0,   alpha);
    
    ALLEGRO_COLOR c =
        al_map_rgba(
            ((components.size() > 0) ? s2i(components[0]) : 0),
            ((components.size() > 1) ? s2i(components[1]) : 0),
            ((components.size() > 2) ? s2i(components[2]) : 0),
            ((components.size() > 3) ? s2i(components[3]) : 255)
        );
    return c;
}


/**
 * @brief Calls al_set_clipping_rectangle, but makes sure that the new clipping
 * rectangle is inside of an older one, as to not suddenly start drawing
 * in places that the older rectangle said not to.
 * The order doesn't really matter.
 *
 * @param x1 First rectangle's top-left X coordinate.
 * @param y1 First rectangle's top-left Y coordinate.
 * @param w1 First rectangle's width.
 * @param h1 First rectangle's width.
 * @param x2 Second rectangle's top-left X coordinate.
 * @param y2 Second rectangle's top-left Y coordinate.
 * @param w2 Second rectangle's width.
 * @param h2 Second rectangle's width.
 */
void setCombinedClippingRectangles(
    float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2
) {
    float bestLeft = std::max(x1, x2);
    float bestTop = std::max(y1, y2);
    float bestRight = std::min(x1 + w1, x2 + w2);
    float bestBottom = std::min(y1 + h1, y2 + h2);
    al_set_clipping_rectangle(
        bestLeft, bestTop,
        bestRight - bestLeft, bestBottom - bestTop
    );
}


/**
 * @brief Shows a native message box. It is better to call this rather than
 * al_show_native_message_box() directly because it does not reset the locale
 * after it is done.
 *
 * @param display Display responsible for this dialog.
 * @param title Title to display on the dialog.
 * @param heading Heading text to display on the dialog.
 * @param text Main text to display on the dialog.
 * @param buttons Buttons the user can press.
 * @param flags al_show_native_message_box flags.
 * @return 0 if the window was closed without activating a button.
 * 1 if the OK or Yes button was pressed.
 * 2 if the Cancel or No button was pressed.
 */
int showSystemMessageBox(
    ALLEGRO_DISPLAY* display, char const* title, char const* heading,
    char const* text, char const* buttons, int flags
) {
    int ret =
        al_show_native_message_box(
            display, title, heading, text, buttons, flags
        );
    //Reset the locale, which gets set by Allegro's native dialogs...
    //and breaks s2f().
    setlocale(LC_ALL, "C");
    
    return ret;
}


/**
 * @brief Deletes all "non-important" files inside of a folder.
 * Then, if the folder ends up empty, also deletes the folder.
 *
 * @param folderPath Path to the folder to wipe.
 * @param nonImportantFiles List of files that can be deleted.
 * @return An error code.
 */
FS_DELETE_RESULT wipeFolder(
    const string& folderPath, const vector<string>& nonImportantFiles
) {
    //Panic check to make sure nothing went wrong and it's an important folder.
    //"", "C:", "C:/, "/", etc. are all 3 characters or fewer, so this works.
    engineAssert(
        folderPath.size() >= 4,
        "Tried to wipe the folder \"" + folderPath + "\"!"
    );
    
    ALLEGRO_FS_ENTRY* folder =
        al_create_fs_entry(folderPath.c_str());
    if(!folder || !al_open_directory(folder)) {
        return FS_DELETE_RESULT_NOT_FOUND;
    }
    
    bool hasImportantFiles = false;
    bool hasFolders = false;
    bool nonImportantFileDeleteError = false;
    bool folderDeleteError = false;
    
    ALLEGRO_FS_ENTRY* entry = al_read_directory(folder);
    while(entry) {
        if(hasFlag(al_get_fs_entry_mode(entry), ALLEGRO_FILEMODE_ISDIR)) {
            hasFolders = true;
            
        } else {
            string entryName =
                standardizePath(al_get_fs_entry_name(entry));
                
            //Only save what's after the final slash.
            size_t pos = entryName.find_last_of("/");
            if(pos != string::npos) {
                entryName =
                    entryName.substr(pos + 1, entryName.size() - pos - 1);
            }
            
            if(!isInContainer(nonImportantFiles, entryName)) {
                //Name not found in the non-important file list.
                hasImportantFiles = true;
            } else {
                if(!al_remove_fs_entry(entry)) {
                    nonImportantFileDeleteError = true;
                }
            }
            
        }
        
        al_destroy_fs_entry(entry);
        entry = al_read_directory(folder);
    }
    
    al_close_directory(folder);
    
    if(
        !hasImportantFiles &&
        !hasFolders &&
        !nonImportantFileDeleteError
    ) {
        if(!al_remove_fs_entry(folder)) {
            folderDeleteError = true;
        }
    }
    
    al_destroy_fs_entry(folder);
    
    if(nonImportantFileDeleteError || folderDeleteError) {
        return FS_DELETE_RESULT_DELETE_ERROR;
    }
    if(hasImportantFiles || hasFolders) {
        return FS_DELETE_RESULT_HAS_IMPORTANT;
    }
    return FS_DELETE_RESULT_OK;
}
