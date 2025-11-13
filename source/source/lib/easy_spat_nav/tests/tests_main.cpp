/*
 * Copyright (c) Andre 'Espyo' Silva 2025.
 *
 * === FILE DESCRIPTION ===
 * Unit tests for the Easy Spatial Navigation library.
 * Please read the included readme file.
 */

#include <vector>
#include <string>

#include "../easy_spat_nav.h"


constexpr const char* COLOR_BOLD = "\033[1m";
constexpr const char* COLOR_GREEN = "\033[32m";
constexpr const char* COLOR_RED = "\033[31m";
constexpr const char* COLOR_RESET = "\033[0m";
constexpr const char* COLOR_YELLOW = "\033[33m";


//Number of tests executed so far.
size_t nTestsExecuted = 0;

//Global manager.
EasySpatNav::Interface spatNavManager;


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
    float width = 0.0f;

    //Total height.
    float height = 0.0f;

    //List of existing parents and what their relative item numbers are.
    std::vector<size_t> parentNrs;

    //Children items definitions, for each parent in order.
    std::vector<SpatNavTestInterface*> children;


    //--- Function declarations ---

    SpatNavTestInterface();
    SpatNavTestInterface(
        const std::string& s,
        std::vector<SpatNavTestInterface*> children = {}
    );
    void fromString(const std::string& s);

};


/**
 * @brief Constructs a new test interface object.
 */
SpatNavTestInterface::SpatNavTestInterface() { }


/**
 * @brief Constructs a new test interface object, and reads its data
 * from a string with ASCII art representing it.
 * 
 * @param s The string.
 * @param children If the interface contains parent items, specify their
 * children here. Each entry in the vector corresponds to one parent.
 */
SpatNavTestInterface::SpatNavTestInterface(
    const std::string& s,
    std::vector<SpatNavTestInterface*> children
) {
    fromString(s);
    this->children = children;
}


/**
 * @brief Reads data from a string with ASCII art representing an interface.
 * 
 * @param s The string.
 */
void SpatNavTestInterface::fromString(const std::string& s) {
    bool inItem = false;
    bool itemIsParent = false;
    size_t itemStartX = 0;
    size_t caretX = 0;
    size_t caretY = 0;

    const auto finishItem =
    [this, &inItem, &itemIsParent, &itemStartX, &caretX, &caretY] () {
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
        if(itemIsParent) {
            parentNrs.push_back(items.size());
        }
        inItem = false;
        itemIsParent = false;
    };

    const auto finishLine =
    [this, &caretX, &caretY] () {
        width = std::max(width, (float) caretX);
        caretX = 0;
        caretY++;
    };

    for(size_t c = 0; c < s.size(); c++) {
        bool isItem = s[c] == '#' || s[c] == 'P';
        bool isLineBreak = s[c] == '\n';

        if(isItem) {
            if(!inItem) {
                //Start a new item.
                itemStartX = caretX;
                inItem = true;
                itemIsParent = (s[c] == 'P');
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


/**
 * @brief Recursively adds children interfaces to the EasySpatNav manager.
 * 
 * @param manager The manager.
 * @param interface Interface whose children to add.
 * @param itemNr Current item number.
 * @param interfaceFirstItemNr Item number of the first item in the interface.
 */
void addChildren(
    EasySpatNav::Interface& manager, SpatNavTestInterface* interface,
    size_t& itemNr, size_t interfaceFirstItemNr
) {
    for(size_t ci = 0; ci < interface->children.size(); ci++) {
        SpatNavTestInterface* childIf = interface->children[ci];
        size_t childInterfaceFirstItemNr = itemNr;
        for(size_t i = 0; i < childIf->items.size(); i++) {
            manager.addItem(
                (void*) (itemNr),
                (childIf->items[i].startX + childIf->items[i].endX) / 2.0f,
                (childIf->items[i].startY + childIf->items[i].endY) / 2.0f,
                childIf->items[i].endX - childIf->items[i].startX,
                childIf->items[i].endY - childIf->items[i].startY
            );
            manager.setParentItem(
                (void*) itemNr,
                (void*) (interfaceFirstItemNr + interface->parentNrs[ci] - 1)
            );
            itemNr++;
        }
        addChildren(
            manager, interface->children[ci], itemNr, childInterfaceFirstItemNr
        );
    }
}


/**
 * @brief Performs a generic test. Aborts if it fails.
 * 
 * @param testDescription Description of the test is doing.
 * @param expectedNr Expected number to compare.
 * @param actualNr Actual number.
 * @param extraInfo Extra information to display if the test fails.
 */
void test(
    const std::string& testDescription,
    size_t expectedNr, size_t actualNr,
    const std::string& extraInfo = ""
) {
    if(actualNr != expectedNr) {
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
            COLOR_BOLD, (unsigned int) expectedNr, COLOR_RESET,
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
 * @param expectedItemNr Index of the item it should land on (starts at 1).
 * @param heuristics Heuristics to use, if different from the default.
 * @param settings Settings to use, if different from the default.
 * If the limits aren't properly set (i.e. the defaults), they will be
 * correctly set to the limits of the test interface.
 * @param resetHistory Whether the navigation history should be reset.
 */
void testNav(
    const std::string& testDescription,
    SpatNavTestInterface& interface, EasySpatNav::DIRECTION direction,
    size_t focusedItemNr, size_t expectedItemNr,
    const EasySpatNav::Interface::Heuristics& heuristics = {},
    const EasySpatNav::Interface::Settings& settings = {},
    bool resetHistory = true
) {
    spatNavManager.reset(resetHistory);
    spatNavManager.heuristics = heuristics;
    spatNavManager.settings = settings;

    if(settings.limitX2 == 0.0f) {
        spatNavManager.settings.limitX1 = -0.001f;
        spatNavManager.settings.limitX2 = interface.width + 0.001f;
        spatNavManager.settings.limitY1 = -0.001f;
        spatNavManager.settings.limitY2 = interface.height + 0.001f;
    }

    size_t itemNr = 1;
    for(size_t i = 0; i < interface.items.size(); i++) {
        spatNavManager.addItem(
            (void*) (itemNr),
            (interface.items[i].startX + interface.items[i].endX) / 2.0f,
            (interface.items[i].startY + interface.items[i].endY) / 2.0f,
            interface.items[i].endX - interface.items[i].startX,
            interface.items[i].endY - interface.items[i].startY
        );
        itemNr++;
    }
    addChildren(spatNavManager, &interface, itemNr, 1);

    size_t targetItemNr =
        (size_t) spatNavManager.navigate(direction, (void*) focusedItemNr);
    
    test(testDescription, expectedItemNr, targetItemNr);
}


using EasySpatNav::DIRECTION_RIGHT;
using EasySpatNav::DIRECTION_DOWN;
using EasySpatNav::DIRECTION_LEFT;
using EasySpatNav::DIRECTION_UP;


/**
 * @brief Main testing program.
 * 
 * @param argc Unused.
 * @param argv Unused.
 * @return 0.
 */
int main(int argc, char** argv) {
    //--- Startup ---
    printf(
        "%s====== SPATIAL NAVIGATION UNIT TESTS ======%s\n",
        COLOR_BOLD, COLOR_RESET
    );
    printf("Testing...\n");

    //--- Setup some scenarios ---
    SpatNavTestInterface
        ifBasic2By2(
            "# #\n"
            "   \n"
            "# #"
        );
    SpatNavTestInterface
        ifBasic2By2WithSpace(
            "     \n"
            " # # \n"
            "     \n"
            " # # \n"
            "     "
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
        ifList(
            "# # # # #"
        );
    SpatNavTestInterface
        ifDistances(
            "  #\n"
            "#  \n"
            "   \n"
            "  #"
        );
    SpatNavTestInterface
        ifDistances2(
            "  #\n"
            "   \n"
            "#  \n"
            "   \n"
            "  #"
        );
    SpatNavTestInterface
        ifLoopPass(
            "# #  \n"
            "     \n"
            "     \n"
            "     \n"
            "    #"
        );
    SpatNavTestInterface
        ifBasicParentChild1(
            "  # #"
        );
    SpatNavTestInterface
        ifBasicParentTop(
            "# PPP",
            { &ifBasicParentChild1 }
        );
    SpatNavTestInterface
        ifListParentChild1(
            "# # #"
        );
    SpatNavTestInterface
        ifListParentTop(
            "# PPP",
            { &ifListParentChild1 }
        );
    SpatNavTestInterface
        ifDoubleParentChild1Child1(
            "     #   "
        );
    SpatNavTestInterface
        ifDoubleParentChild1(
            "    PPP  ",
            { &ifDoubleParentChild1Child1 }
        );
    SpatNavTestInterface
        ifDoubleParentTop(
            "# PPPPPPP",
            { &ifDoubleParentChild1 }
        );
    SpatNavTestInterface
        ifLargeOverflowChild1(
            "        #"
        );
    SpatNavTestInterface
        ifLargeOverflowTop(
            "# PPP #  ",
            { &ifLargeOverflowChild1 }
        );
    SpatNavTestInterface
        ifTie1(
            "#   #\n"
            "     \n"
            "  #  "
        );
    SpatNavTestInterface
        ifTie2(
            "  #  \n"
            "     \n"
            "#   #"
        );
    SpatNavTestInterface
        ifEmpty;
    SpatNavTestInterface
        ifJust1(
            "#"
        );
    
    //--- Do the tests ---

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

    //Looping tests.
    testNav(
        "Test that basic looping to the right works.",
        ifBasic3By3, DIRECTION_RIGHT, 3, 1
    );
    testNav(
        "Test that basic looping to the left works.",
        ifBasic3By3, DIRECTION_LEFT, 1, 3
    );
    testNav(
        "Test that basic looping down works.",
        ifBasic3By3, DIRECTION_DOWN, 7, 1
    );
    testNav(
        "Test that basic looping up works.",
        ifBasic3By3, DIRECTION_UP, 1, 7
    );
    testNav(
        "Test that looping to the right won't be done if disabled.",
        ifBasic3By3, DIRECTION_RIGHT, 3, 0, {},
        { .loopX = false, .loopY = false }
    );
    testNav(
        "Test that looping to the left won't be done if disabled.",
        ifBasic3By3, DIRECTION_LEFT, 1, 0, {},
        { .loopX = false, .loopY = false }
    );
    testNav(
        "Test that looping down won't be done if disabled.",
        ifBasic3By3, DIRECTION_DOWN, 7, 0, {},
        { .loopX = false, .loopY = false }
    );
    testNav(
        "Test that looping up won't be done if disabled.",
        ifBasic3By3, DIRECTION_UP, 1, 0, {},
        { .loopX = false, .loopY = false }
    );

    //Distance calculation methods.
    testNav(
        "Test that Euclidean distance checks pick the best option.",
        ifDistances, DIRECTION_UP, 3, 2,
        { .distCalcMethod = EasySpatNav::DIST_CALC_METHOD_EUCLIDEAN }
    );
    testNav(
        "Test that taxicab distance checks pick the best option.",
        ifDistances, DIRECTION_UP, 3, 1,
        { .distCalcMethod = EasySpatNav::DIST_CALC_METHOD_TAXICAB }
    );
    testNav(
        "Test that taxicab 2 distance checks pick the best option.",
        ifDistances2, DIRECTION_UP, 3, 1,
        { .distCalcMethod = EasySpatNav::DIST_CALC_METHOD_TAXICAB_2 }
    );

    //Single loop pass.
    testNav(
        "Test that the correct item is picked with single-loop pass on.",
        ifLoopPass, DIRECTION_RIGHT, 2, 1,
        { .singleLoopPass = true }
    );
    testNav(
        "Test that the correct item is picked with single-loop pass off.",
        ifLoopPass, DIRECTION_RIGHT, 2, 3,
        { .singleLoopPass = false }
    );

    //Parents.
    testNav(
        "Test that simple navigation with children works, 1.",
        ifBasicParentTop, DIRECTION_RIGHT, 1, 3
    );
    testNav(
        "Test that simple navigation with children works, 2.",
        ifBasicParentTop, DIRECTION_RIGHT, 3, 4
    );
    testNav(
        "Test that simple navigation with children works, 3.",
        ifBasicParentTop, DIRECTION_RIGHT, 4, 1
    );
    testNav(
        "Test that simple navigation with overflowing children works, 1.",
        ifListParentTop, DIRECTION_RIGHT, 1, 3
    );
    testNav(
        "Test that simple navigation with overflowing children works, 2.",
        ifListParentTop, DIRECTION_RIGHT, 4, 5
    );
    testNav(
        "Test that simple navigation with overflowing children works, 3.",
        ifListParentTop, DIRECTION_RIGHT, 5, 1
    );
    testNav(
        "Test that navigation with largely overflowing children works, 1.",
        ifLargeOverflowTop, DIRECTION_RIGHT, 1, 4
    );
    testNav(
        "Test that navigation with largely overflowing children works, 2.",
        ifLargeOverflowTop, DIRECTION_RIGHT, 4, 1
    ); //Focus being overflowed is not supported.
    testNav(
        "Test that navigation to a child inside two parents works.",
        ifDoubleParentTop, DIRECTION_RIGHT, 1, 4
    );

    //Tie-breakers.
    testNav(
        "Test that the history is followed in a tie-breaker scenario, "
        "setup 1.",
        ifTie1, DIRECTION_DOWN, 1, 3
    );
    testNav(
        "Test that the history is followed in a tie-breaker scenario, "
        "navigation 1.",
        ifTie1, DIRECTION_UP, 3, 1, {}, {}, false
    );
    testNav(
        "Test that the history is followed in a tie-breaker scenario, "
        "setup 2.",
        ifTie1, DIRECTION_DOWN, 2, 3
    );
    testNav(
        "Test that the history is followed in a tie-breaker scenario, "
        "navigation 2.",
        ifTie1, DIRECTION_UP, 3, 2, {}, {}, false
    );
    testNav(
        "Test that in a tie-breaker scenario with history disabled, "
        "the first added item wins, 1.",
        ifTie1, DIRECTION_UP, 3, 1, { .historyScoreThreshold = -1.0f }, {}
    );
    testNav(
        "Test that in a tie-breaker scenario with history disabled, "
        "the first added item wins, 2.",
        ifTie2, DIRECTION_DOWN, 1, 2, { .historyScoreThreshold = -1.0f }, {}
    );

    //Misc.
    testNav(
        "Test that navigating vertically on a horizontal list, "
        "with looping, doesn't select an item to the left or right.",
        ifList, DIRECTION_DOWN, 3, 0
    );
    testNav(
        "Test that an empty interface returns nullptr.",
        ifEmpty, DIRECTION_RIGHT, 1, 0
    );
    testNav(
        "Test that an interface with one item returns nullptr.",
        ifJust1, DIRECTION_RIGHT, 1, 0
    );
    testNav(
        "Test that an interface with no valid starting item returns nullptr.",
        ifBasic2By2WithSpace, DIRECTION_RIGHT, 0, 1
    );

    //--- Finish ---
    printf(
        "%sAll %u tests succeeded!%s\n",
        COLOR_GREEN, (unsigned int) nTestsExecuted, COLOR_RESET
    );

    return 0;
}
