/*
 * Copyright (c) Andre 'Espyo' Silva 2025.
 *
 * === FILE DESCRIPTION ===
 * Unit tests for the spatial navigation library.
 * Please read the included readme file.
 */

#include <vector>
#include <string>

#include "../spatial_navigation.h"


constexpr const char* COLOR_BOLD = "\033[1m";
constexpr const char* COLOR_GREEN = "\033[32m";
constexpr const char* COLOR_RED = "\033[31m";
constexpr const char* COLOR_RESET = "\033[0m";
constexpr const char* COLOR_YELLOW = "\033[33m";


//Number of tests executed so far.
size_t nTestsExecuted = 0;


/**
 * @brief Represents a test interface item.
 */
struct SpatNavTestItem {
    //--- Members ---

    //Starting X coordinate.
    float startX = 0.0f;

    //Ending X coordinate.
    float endX = 0.0f;

    //Starting Y coordinate.
    float startY = 0.0f;

    //Ending Y coordinate.
    float endY = 0.0f;

};


/**
 * @brief Represents a test interface.
 */
struct SpatNavTestInterface {
    //--- Members ---

    //List of items.
    std::vector<SpatNavTestItem> items;

    //Total width.
    float width;

    //Total height.
    float height;


    //--- Function definitions ---

    /**
     * @brief Constructs a new test interface object.
     */
    SpatNavTestInterface() { }

    /**
     * @brief Constructs a new test interface object, and reads its data
     * from a string with ASCII art representing it.
     * 
     * @param s The string.
     */
    SpatNavTestInterface(const std::string& s) {
        fromString(s);
    }

    /**
     * @brief Reads data from a string with ASCII art representing an interface.
     * 
     * @param s The string.
     */
    void fromString(const std::string& s) {
        bool inItem = false;
        size_t itemStartX = 0;
        size_t caretX = 0;
        size_t caretY = 0;

        const auto finishItem =
        [this, &inItem, &itemStartX, &caretX, &caretY] () {
            if(!inItem) return;
            //Finish a new item.
            bool updated = false;
            for(size_t i = 0; i < items.size(); i++) {
                if(
                    items[i].startX == itemStartX &&
                    items[i].endX == caretX &&
                    items[i].endY == caretY
                ) {
                    //Update the one from the previous line(s).
                    items[i].endY = caretY + 1.0f;
                    updated = true;
                    break;
                }
            }
            if(!updated) {
                //Save a new one.
                items.push_back(
                    SpatNavTestItem {
                        .startX = (float) itemStartX,
                        .endX = (float) caretX,
                        .startY = (float) caretY,
                        .endY = (float) caretY + 1.0f,
                    }
                );
            }
            inItem = false;
        };

        const auto finishLine =
        [this, &caretX, &caretY] () {
            width = std::max(width, (float) caretX);
            caretX = 0;
            caretY++;
        };

        for(size_t c = 0; c < s.size(); c++) {
            bool isItem = s[c] == '#';
            bool isLineBreak = s[c] == '\n';

            if(isItem) {
                if(!inItem) {
                    //Start a new item.
                    itemStartX = caretX;
                    inItem = true;
                }
            } else {
                if(inItem) {
                    finishItem();
                }
            }

            if(isLineBreak) {
                finishLine();
            } else {
                caretX++;
            }
        }

        finishItem();
        finishLine();
        height = caretY;
    }

};


/**
 * @brief Performs a generic test. Aborts if it fails.
 * 
 * @param testDescription Description of the test is doing.
 * @param intendedNr Intended number to compare.
 * @param actualNr Actual number.
 * @param extraInfo Extra information to display if the test fails.
 */
void test(
    const std::string& testDescription,
    size_t intendedNr, size_t actualNr,
    const std::string& extraInfo = ""
) {
    if(actualNr != intendedNr) {
        printf(
            "%sTest #%u failed!%s\n",
            COLOR_RED, (unsigned int) nTestsExecuted + 1, COLOR_RESET
        );
        printf(
            "  \"%s\"\n",
            testDescription.c_str()
        );
        printf(
            "  Expected %s%u%s, got %s%u%s.\n",
            COLOR_BOLD, (unsigned int) intendedNr, COLOR_RESET,
            COLOR_BOLD, (unsigned int) actualNr, COLOR_RESET
        );
        if(!extraInfo.empty()) {
            printf("  %s\n", extraInfo.c_str());
        }
        abort();
    }

    nTestsExecuted++;
}


/**
 * @brief Performs a test, navigating from some item to some item.
 * 
 * @param testDescription Description of the test is doing.
 * @param interface Interface to run the test on.
 * @param direction Direction of navigation.
 * @param focusedItemNr Number of the currently focused item (starts at 1).
 * @param intendedItemNr Index of the item it should land on (starts at 1).
 * @param heuristics Heuristics to use, if different from the default.
 * @param settings Settings to use, if different from the default.
 * If the limits aren't properly set (i.e. the defaults), they will be
 * correctly set to the limits of the test interface.
 */
void testNav(
    const std::string& testDescription,
    SpatNavTestInterface& interface, SpatNav::DIRECTION direction,
    size_t focusedItemNr, size_t intendedItemNr,
    const SpatNav::Interface::Heuristics& heuristics = {},
    const SpatNav::Interface::Settings& settings = {}
) {
    SpatNav::Interface spatNavManager;
    spatNavManager.heuristics = heuristics;
    spatNavManager.settings = settings;

    if(settings.limitX2 == 0.0f) {
        spatNavManager.settings.limitX1 = 0.0f;
        spatNavManager.settings.limitX2 = interface.width;
        spatNavManager.settings.limitY1 = 0.0f;
        spatNavManager.settings.limitY2 = interface.height;
    }

    for(size_t i = 0; i < interface.items.size(); i++) {
        spatNavManager.addItem(
            (void*) (i + 1),
            (interface.items[i].startX + interface.items[i].endX) / 2.0f,
            (interface.items[i].startY + interface.items[i].endY) / 2.0f,
            interface.items[i].endX - interface.items[i].startX,
            interface.items[i].endY - interface.items[i].startY
        );
    }

    size_t targetItemNr =
        (size_t) spatNavManager.navigate(direction, (void*) focusedItemNr);
    
    test(testDescription, intendedItemNr, targetItemNr);
}


using SpatNav::DIRECTION_RIGHT;
using SpatNav::DIRECTION_DOWN;
using SpatNav::DIRECTION_LEFT;
using SpatNav::DIRECTION_UP;


/**
 * @brief Main testing program.
 * 
 * @param argc Unused.
 * @param argv Unused.
 * @return 0.
 */
int main(int argc, char** argv) {
    //Startup.
    printf(
        "%s====== SPATIAL NAVIGATION UNIT TESTS ======%s\n",
        COLOR_BOLD, COLOR_RESET
    );
    printf("Testing...\n");

    //Setup some scenarios.
    SpatNavTestInterface
        ifBasic2By2(
            "# #\n"
            "   \n"
            "# #"
        );
    SpatNavTestInterface
        ifBasic3By3(
            "# # #\n"
            "     \n"
            "# # #\n"
            "     \n"
            "# # #"
        );
    SpatNavTestInterface
        ifListVertical5(
            "     \n"
            "  #  \n"
            "     \n"
            "  #  \n"
            "     \n"
            "  #  \n"
            "     \n"
            "  #  \n"
            "     \n"
            "  #  \n"
            "     "
        );
    
    //Do the tests.

    //Basic navigation.
    testNav(
        "Test that basic navigation to the right works.",
        ifBasic2By2, DIRECTION_RIGHT, 1, 2
    );
    testNav(
        "Test that basic navigation to the left works.",
        ifBasic2By2, DIRECTION_LEFT, 2, 1
    );
    testNav(
        "Test that basic navigation down works.",
        ifBasic2By2, DIRECTION_DOWN, 1, 3
    );
    testNav(
        "Test that basic navigation up works.",
        ifBasic2By2, DIRECTION_UP, 3, 1
    );
    testNav(
        "Test that navigation to the right works with another item beyond.",
        ifBasic3By3, DIRECTION_RIGHT, 1, 2
    );
    testNav(
        "Test that navigation to the left works with another item beyond.",
        ifBasic3By3, DIRECTION_LEFT, 3, 2
    );
    testNav(
        "Test that navigation down works with another item beyond.",
        ifBasic3By3, DIRECTION_DOWN, 1, 4
    );
    testNav(
        "Test that navigation up works with another item beyond.",
        ifBasic3By3, DIRECTION_UP, 7, 4
    );

    //TODO distance calculation methods
    //TODO looping enabled/disabled
    //TODO all heuristics
    //TODO parent items

    //Misc.
    testNav(
        "Test that navigating horizontally on a vertical list, "
        "with looping, doesn't select an item above or below.",
        ifListVertical5, DIRECTION_RIGHT, 3, 0
    );

    //Finish.
    printf(
        "%sAll %u tests succeeded!%s\n",
        COLOR_GREEN, (unsigned int) nTestsExecuted, COLOR_RESET
    );
}
