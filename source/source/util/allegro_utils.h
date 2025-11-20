/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for Allegro-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#pragma once

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "geometry_utils.h"

using std::string;
using std::vector;


//Possible results for the player interacting with a file dialog.
enum FILE_DIALOG_RESULT {

    //Successful operation.
    FILE_DIALOG_RESULT_SUCCESS,
    
    //The option picked is not in the expected folder.
    FILE_DIALOG_RESULT_WRONG_FOLDER,
    
    //The player cancelled the dialog.
    FILE_DIALOG_RESULT_CANCELED,
    
};


//Possible results for a filesystem deletion operation.
enum FS_DELETE_RESULT {

    //Successful.
    FS_DELETE_RESULT_OK,
    
    //File or folder not found.
    FS_DELETE_RESULT_NOT_FOUND,
    
    //Folder has important files inside, or has folders inside.
    FS_DELETE_RESULT_HAS_IMPORTANT,
    
    //An error occurred somewhere when deleting a file or folder.
    FS_DELETE_RESULT_DELETE_ERROR,
    
};


//Returns a white color with the specified alpha [0 - 255].
#define mapAlpha(a) al_map_rgba(255, 255, 255, (a))

//Returns a gray with all indexes [0 - 255] the same as specified value;
//it's fully opaque.
#define mapGray(g) al_map_rgb((g), (g), (g))


bool operator==(const ALLEGRO_COLOR& c1, const ALLEGRO_COLOR& c2);
bool operator!=(const ALLEGRO_COLOR& c1, const ALLEGRO_COLOR& c2);
void al_fwrite(ALLEGRO_FILE* f, const string& s);
string c2s(const ALLEGRO_COLOR& c);
ALLEGRO_COLOR changeAlpha(const ALLEGRO_COLOR& c, unsigned char a);
ALLEGRO_COLOR changeColorLighting(const ALLEGRO_COLOR& c, float l);
FS_DELETE_RESULT deleteFile(const string& filePath);
Point getBitmapDimensions(ALLEGRO_BITMAP* bmp);
string getButtonName(int buttonNr, const string& controllerName);
string getKeyName(int keycode, bool condensed);
string getStickName(int stickNr, const string& controllerName);
void getShiftCtrlAltState(
    bool* outShiftState, bool* outCtrlState, bool* outAltState
);
void getline(ALLEGRO_FILE* file, string& line);
bool fileExists(const string& path);
bool folderExists(const string& path);
vector<string> folderToVector(
    string folderName, bool folders, bool* outFolderFound = nullptr
);
vector<string> folderToVectorRecursively(
    string folderName, bool folders, bool* outFolderFound = nullptr
);
ALLEGRO_COLOR interpolateColor(
    float input, float inputStart, float inputEnd,
    const ALLEGRO_COLOR& outputStart, const ALLEGRO_COLOR& outputEnd
);
ALLEGRO_COLOR multAlpha(const ALLEGRO_COLOR& c, float mult);
vector<string> promptFileDialog(
    const string& initialPath, const string& title,
    const string& patterns, int mode, ALLEGRO_DISPLAY* display
);
vector<string> promptFileDialogLockedToFolder(
    const string& folderPath, const string& title,
    const string& patterns, int mode, FILE_DIALOG_RESULT* result,
    ALLEGRO_DISPLAY* display
);
ALLEGRO_BITMAP* recreateBitmap(ALLEGRO_BITMAP* b);
ALLEGRO_COLOR s2c(const string& s);
void setCombinedClippingRectangles(
    float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2
);
int showSystemMessageBox(
    ALLEGRO_DISPLAY* display, char const* title, char const* heading,
    char const* text, char const* buttons, int flags
);
ALLEGRO_COLOR tintColor(const ALLEGRO_COLOR& c, const ALLEGRO_COLOR& t);
FS_DELETE_RESULT wipeFolder(
    const string& folderPath, const vector<string>& nonImportantFiles
);
