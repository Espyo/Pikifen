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


//Returns a white color with the specified alpha.
#define map_alpha(a) al_map_rgba(255, 255, 255, (a))

//Returns a gray with all indexes the same as specified value;
//it's fully opaque.
#define map_gray(g) al_map_rgb((g), (g), (g))


bool operator==(const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2);
bool operator!=(const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2);
void al_fwrite(ALLEGRO_FILE* f, const string &s);
string c2s(const ALLEGRO_COLOR &c);
ALLEGRO_COLOR change_alpha(const ALLEGRO_COLOR &c, unsigned char a);
ALLEGRO_COLOR change_color_lighting(const ALLEGRO_COLOR &c, float l);
FS_DELETE_RESULT delete_file(const string& file_path);
string get_key_name(int keycode, bool condensed);
void getline(ALLEGRO_FILE* file, string &line);
bool file_exists(const string &path);
bool folder_exists(const string &path);
vector<string> folder_to_vector(
    string folder_name, bool folders, bool* out_folder_found = nullptr
);
vector<string> folder_to_vector_recursively(
    string folder_name, bool folders, bool* out_folder_found = nullptr
);
ALLEGRO_COLOR interpolate_color(
    float input, float input_start, float input_end,
    const ALLEGRO_COLOR &output_start, const ALLEGRO_COLOR &output_end
);
vector<string> prompt_file_dialog(
    const string &initial_path, const string &title,
    const string &patterns, int mode, ALLEGRO_DISPLAY* display
);
vector<string> prompt_file_dialog_locked_to_folder(
    const string &folder_path, const string &title,
    const string &patterns, int mode, FILE_DIALOG_RESULT* result,
    ALLEGRO_DISPLAY* display
);
ALLEGRO_BITMAP* recreate_bitmap(ALLEGRO_BITMAP* b);
ALLEGRO_COLOR s2c(const string &s);
void set_combined_clipping_rectangles(
    float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2
);
int show_message_box(
    ALLEGRO_DISPLAY* display, char const* title, char const* heading,
    char const* text, char const* buttons, int flags
);
FS_DELETE_RESULT wipe_folder(
    const string &folder_path, const vector<string> &non_important_files
);
