/*
 * Copyright (c) Andre 'Espyo' Silva 2025.
 *
 * === FILE DESCRIPTION ===
 * Unit tests for the Easy Analog Cleaner library.
 * Please read the included readme file.
 */


#ifdef EASY_ANALOG_CLEANER_UNIT_TESTS


#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <vector>

#include "../easy_analog_cleaner.h"


#pragma region Test scaffolding


constexpr const char* COLOR_BOLD = "\033[1m";
constexpr const char* COLOR_GREEN = "\033[32m";
constexpr const char* COLOR_RED = "\033[31m";
constexpr const char* COLOR_RESET = "\033[0m";
constexpr const char* COLOR_YELLOW = "\033[33m";


//Number of tests executed so far.
size_t nTestsExecuted = 0;


/**
 * @brief Performs a generic test. Aborts if it fails.
 *
 * @param testDescription Description of the test is doing.
 * @param expected Expected number(s) to compare.
 * @param actual Actual number(s).
 * @param input Raw number(s) used for the algorithm input.
 * @param nAxes Number of axes.
 * @param extraInfo Extra information to display if the test fails.
 */
void test(
    const std::string& testDescription,
    float expected[], float actual[], float input[], unsigned int nAxes,
    const std::string& extraInfo = ""
) {
    bool passed = true;
    
    unsigned int axis = 0;
    for(; axis < nAxes; axis++) {
        if(fabs(actual[axis] - expected[axis]) > 0.00001f) {
            passed = false;
            break;
        }
    }
    
    if(!passed) {
        printf(
            "%sTest #%u failed!%s\n",
            COLOR_RED, (unsigned int) nTestsExecuted + 1, COLOR_RESET
        );
        printf(
            "  \"%s\"\n",
            testDescription.c_str()
        );
        printf(
            "  Expected coordinate %u to be %s%f%s, got %s%f%s.\n",
            axis + 1, COLOR_BOLD, expected[axis], COLOR_RESET,
            COLOR_BOLD, actual[axis], COLOR_RESET
        );
        if(!extraInfo.empty()) {
            printf("  %s\n", extraInfo.c_str());
        }
        
        if(nAxes == 2) {
            const int DRAWING_W = 41;
            const int DRAWING_H = 21;
            
            const auto terminalToScene =
            [] (int x, int y, float * outX, float * outY) {
                *outX = x / (float) DRAWING_W;
                *outY = y / (float) DRAWING_H;
                *outX = *outX * 2.0f - 1.0f;
                *outY = *outY * 2.0f - 1.0f;
            };
            const auto sceneToTerminal =
            [] (float x, float y, int* outX, int* outY) {
                x = (x + 1.0f) / 2.0f;
                y = (y + 1.0f) / 2.0f;
                *outX = x * DRAWING_W;
                *outY = y * DRAWING_H;
            };
            
            for(int row = 0; row < DRAWING_H + 1; row ++) {
                for(int col = 0; col < DRAWING_W + 1; col ++) {
                    float sceneX, sceneY;
                    terminalToScene(col, row, &sceneX, &sceneY);
                    int originCol, originRow;
                    sceneToTerminal(0, 0, &originCol, &originRow);
                    int actCol, actRow;
                    sceneToTerminal(actual[0], actual[1], &actCol, &actRow);
                    int expCol, expRow;
                    sceneToTerminal(expected[0], expected[1], &expCol, &expRow);
                    int inpCol, inpRow;
                    sceneToTerminal(input[0], input[1], &inpCol, &inpRow);
                    float radius =
                        (float) sqrt(sceneX * sceneX + sceneY * sceneY);
                    unsigned char ch = ' ';
                    
                    if(originCol == col && originRow == row) {
                        ch = '+';
                    } else if(radius > 1.0f) {
                        ch = '#';
                    }
                    if(actCol == col && actRow == row) {
                        ch = 'A';
                    } else if(expCol == col && expRow == row) {
                        ch = 'E';
                    } else if(inpCol == col && inpRow == row) {
                        ch = 'I';
                    }
                    printf("%c", ch);
                }
                printf("\n");
            }
        }
        
        abort();
    }
    
    nTestsExecuted++;
}


/**
 * @brief Performs a test, cleaning some analog stick coordinates.
 *
 * The test passes if the actual values land within a tiny margin of the
 * expected values. This is so that tiny floating point calculation differences
 * between different hardware don't cause a test to fail when everything
 * is in reality working properly.
 *
 * @param testDescription Description of what the test is doing.
 * @param x X coordinate to clean [-1 - 1].
 * @param y Y coordinate to clean [-1 - 1].
 * @param expectedX What the cleaned X coordinate should be [-1 - 1].
 * @param expectedY What the cleaned Y coordinate should be [-1 - 1].
 * @param settings Settings to use, if different from the default.
 * @param prevX X of the previous reading, for use in low-pass filtering.
 * @param prevY Y of the previous reading, for use in low-pass filtering.
 */
void testClean(
    const std::string& testDescription,
    float x, float y, float expectedX, float expectedY,
    const EasyAnalogCleaner::Settings settings = {},
    float prevX = 0.0f, float prevY = 0.0f
) {
    float input[2] = { x, y };
    float coords[2] = { x, y };
    float expectedArr[2] = { expectedX, expectedY };
    float prevCoords[2] = { prevX, prevY };
    
    EasyAnalogCleaner::clean(coords, settings, prevCoords);
    
    test(testDescription, expectedArr, coords, input, 2);
}


/**
 * @brief Performs a test, cleaning an analog button pressure value.
 *
 * The test passes if the actual value lands within a tiny margin of the
 * expected value. This is so that tiny floating point calculation differences
 * between different hardware don't cause a test to fail when everything
 * is in reality working properly.
 *
 * @param testDescription Description of what the test is doing.
 * @param pressure Pressure value to clean [-1 - 1].
 * @param expectedPressure What the cleaned pressure value should be [-1 - 1].
 * @param settings Settings to use, if different from the default.
 * @param prevPressure Pressure of the previous reading,
 * for use in low-pass filtering.
 */
void testCleanButton(
    const std::string& testDescription,
    float pressure, float expectedPressure,
    const EasyAnalogCleaner::Settings settings = {},
    float prevPressure = 0.0f
) {
    float inputArr[1] = { pressure };
    float expectedArr[1] = { expectedPressure };
    
    EasyAnalogCleaner::cleanButton(&pressure, settings, prevPressure);
    
    float actualArr[1] = { pressure };
    test(testDescription, expectedArr, actualArr, inputArr, 1);
}


#pragma endregion
#pragma region Test program


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
        "%s====== EASY ANALOG CLEANER UNIT TESTS ======%s\n",
        COLOR_BOLD, COLOR_RESET
    );
    printf("Testing...\n");
    
    //--- Do the tests ---
    
    //Unbothered hardware.
    testClean(
        "Test that a centered stick is left alone by default.",
        0.0f, 0.0f, 0.0f, 0.0f
    );
    testCleanButton(
        "Test that an unpressed button is left alone by default.",
        0.0f, 0.0f
    );
    
    //Radial deadzones.
    testClean(
        "Test that radial deadzones, with interpolation and default "
        "thresholds, give proper values inside the inner deadzone.",
        0.023f, -0.056f, 0.0f, -0.0f
    );
    testClean(
        "Test that radial deadzones, with interpolation and default "
        "thresholds, give proper values outside either deadzone.",
        0.123f, -0.456f, 0.101305924f, -0.375573426f
    );
    testClean(
        "Test that radial deadzones, with interpolation and default "
        "thresholds, give proper values inside the outer deadzone.",
        -0.68f, 0.70f, -0.696785629f, 0.717279434f
    );
    testClean(
        "Test that radial deadzones, with interpolation and some other "
        "thresholds, give proper values outside either deadzone.",
        0.123f, -0.456f, 0.179898947f, -0.666942835f,
    { .deadzones { .radial { .inner = 0.12f, .outer = 0.63f } } }
    );
    testClean(
        "Test that radial deadzones, without interpolation, give proper "
        "values inside the inner deadzone.",
        0.05f, -0.01f, 0.0f, 0.0f,
    { .deadzones { .radial { .interpolate = false } } }
    );
    testClean(
        "Test that radial deadzones, without interpolation, give proper "
        "values outside either deadzone.",
        -0.35f, 0.71f, -0.35f, 0.71f,
    { .deadzones { .radial { .interpolate = false } } }
    );
    testClean(
        "Test that radial deadzones, without interpolation, give proper "
        "values inside the outer deadzone.",
        -0.97f, -0.66f, -0.826768041f, -0.562543035,
    { .deadzones { .radial { .interpolate = false } } }
    );
    
    //Horizontal angular deadzone.
    testClean(
        "Test that the horizontal angular deadzone, with interpolation and "
        "default thresholds, and no radial deadzones, gives proper values "
        "inside the deadzone.",
    -0.654f, -0.04f, -0.655222058f, 0.0f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular { .horizontal = M_PI * 0.10f }
        }
    }
    );
    testClean(
        "Test that the horizontal angular deadzone, with interpolation and "
        "default thresholds, and no radial deadzones, gives proper values "
        "outside the deadzone.",
    -0.39f, -0.12f, -0.401685148f, -0.0717565417f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular { .horizontal = M_PI * 0.10f }
        }
    }
    );
    testClean(
        "Test that the horizontal angular deadzone, without interpolation and "
        "default thresholds, and no radial deadzones, gives proper values "
        "inside the deadzone.",
    0.61f, 0.033f, 0.610891998f, 0.0f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular {
                .horizontal = M_PI * 0.10f,
                .interpolate = false
            }
        }
    }
    );
    testClean(
        "Test that the horizontal angular deadzone, without interpolation and "
        "default thresholds, and no radial deadzones, gives proper values "
        "outside the deadzone.",
    -0.37f, -0.39f, -0.37f, -0.39f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular {
                .horizontal = M_PI * 0.10f,
                .interpolate = false
            }
        }
    }
    );
    
    //Vertical angular deadzone.
    testClean(
        "Test that the vertical angular deadzone, with interpolation and "
        "default thresholds, and no radial deadzones, gives proper values "
        "inside the deadzone.",
    -0.037f, -0.62f, 0.0f, -0.621103048f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular { .vertical = M_PI * 0.10f }
        }
    }
    );
    testClean(
        "Test that the vertical angular deadzone, with interpolation and "
        "default thresholds, and no radial deadzones, gives proper values "
        "outside the deadzone.",
    -0.18f, -0.57f, -0.110540397f, -0.587435782f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular { .vertical = M_PI * 0.10f }
        }
    }
    );
    testClean(
        "Test that the vertical angular deadzone, without interpolation and "
        "default thresholds, and no radial deadzones, gives proper values "
        "inside the deadzone.",
    0.01f, -0.60f, 0.0f, -0.600083351f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular {
                .vertical = M_PI * 0.10f,
                .interpolate = false
            }
        }
    }
    );
    testClean(
        "Test that the vertical angular deadzone, without interpolation and "
        "default thresholds, and no radial deadzones, gives proper values "
        "outside the deadzone.",
    -0.41f, -0.32f, -0.41f, -0.32f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular {
                .vertical = M_PI * 0.10f,
                .interpolate = false
            }
        }
    }
    );
    
    //Diagonal angular deadzone.
    testClean(
        "Test that the diagonal angular deadzone, with interpolation and "
        "default thresholds, and no radial deadzones, gives proper values "
        "inside the deadzone.",
    -0.33f, -0.33f, -0.330000043f, -0.329999983, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular { .diagonal = M_PI * 0.10f }
        }
    }
    );
    testClean(
        "Test that the diagonal angular deadzone, with interpolation and "
        "default thresholds, and no radial deadzones, gives proper values "
        "outside the deadzone.",
    -0.22f, -0.70f, -0.27260074f, -0.681240618f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular { .diagonal = M_PI * 0.10f }
        }
    }
    );
    testClean(
        "Test that the diagonal angular deadzone, without interpolation and "
        "default thresholds, and no radial deadzones, gives proper values "
        "inside the deadzone.",
    0.322f, -0.333f, 0.32754612f, -0.327546209f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular {
                .diagonal = M_PI * 0.10f,
                .interpolate = false
            }
        }
    }
    );
    testClean(
        "Test that the diagonal angular deadzone, without interpolation and "
        "default thresholds, and no radial deadzones, gives proper values "
        "outside the deadzone.",
    -0.41f, -0.12f, -0.41f, -0.119999938f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular {
                .diagonal = M_PI * 0.10f,
                .interpolate = false
            }
        }
    }
    );
    
    //Multiple deadzones tests.
    testClean(
        "Test that horizontal and vertical angular deadzones, without "
        "interpolation and default thresholds, and no radial deadzones, "
        "gives proper values outside the deadzones.",
    -0.14f, -0.48f, -0.14f, -0.48f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular {
                .horizontal = M_PI * 0.10f,
                .vertical = M_PI * 0.10f,
                .interpolate = false
            }
        }
    }
    );
    testClean(
        "Test that all angular deadzones, without "
        "interpolation and default thresholds, and no radial deadzones, "
        "gives proper values outside the deadzones.",
    0.29f, -0.12f, 0.29f, -0.12f, {
        .deadzones {
            .radial { .inner = 0.0f, .outer = 1.0f, .interpolate = false },
            .angular {
                .horizontal = M_PI * 0.10f,
                .vertical = M_PI * 0.10f,
                .diagonal = M_PI * 0.10f,
                .interpolate = false
            }
        }
    }
    );
    testClean(
        "Test that all deadzones enabled, with interpolation, gives "
        "proper values outside the deadzones.",
    -0.61f, 0.15f, -0.60568434f, 0.0853933692f, {
        .deadzones {
            .angular {
                .horizontal = M_PI * 0.10f,
                .vertical = M_PI * 0.10f,
                .diagonal = M_PI * 0.10f,
            }
        }
    }
    );
    testClean(
        "Test that all deadzones enabled, without interpolation, gives "
        "proper values outside the deadzones.",
    0.69f, 0.14f, 0.69f, 0.14f, {
        .deadzones {
            .radial {
                .interpolate = false
            },
            .angular {
                .horizontal = M_PI * 0.10f,
                .vertical = M_PI * 0.10f,
                .diagonal = M_PI * 0.10f,
                .interpolate = false
            }
        }
    }
    );
    
    //Button deadzones.
    testCleanButton(
        "Test that button deadzones, with interpolation and default "
        "thresholds, give proper values inside the unpressed deadzone.",
        0.05f, 0.0f
    );
    testCleanButton(
        "Test that button deadzones, with interpolation and default "
        "thresholds, give proper values outside either deadzone.",
        0.38f, 0.350000024f
    );
    testCleanButton(
        "Test that button deadzones, with interpolation and default "
        "thresholds, give proper values inside the pressed deadzone.",
        0.96f, 1.0f
    );
    testCleanButton(
        "Test that button deadzones, without interpolation, give proper "
        "values outside either deadzone.",
        0.77f, 0.77f,
    { .deadzones { .button{ .interpolate = false } } }
    );
    
    //Low-pass filter.
    testClean(
        "Tests that the stick low-pass filter, with all the other settings "
        "set to defaults, gives proper values.",
        0.14f, 0.82f, 0.169723749f, 0.889810681f,
    { .lowPassFilter { .factor = 0.9f } },
    0.33f, 0.89f
    );
    testClean(
        "Tests that the stick low-pass filter, with all the other features "
        "disabled, gives proper values.",
    0.14f, 0.82f, 0.166723758f, 0.871810675f, {
        .deadzones {
            .angular {
                .interpolate = false
            }
        },
        .lowPassFilter {
            .factor = 0.9f
        }
    },
    0.30f, 0.71f
    );
    testCleanButton(
        "Tests that the button low-pass filter, with all the other settings "
        "set to defaults, gives proper values.",
        0.41f, 0.378749996f,
    { .lowPassFilter { .factorButton = 0.9f } },
    0.30f
    );
    testCleanButton(
        "Tests that the button low-pass filter, with all the other features "
        "disabled, gives proper values.",
    0.69f, 0.694000006f, {
        .deadzones {
            .button {
                .interpolate = false
            }
        },
        .lowPassFilter {
            .factorButton = 0.9f
        }
    },
    0.73f
    );
    
    //Misc.
    testClean(
        "Test that the analog stick values are left alone with the "
        "\"no changes\" config,"
        "analog stick.",
        0.123f, 0.456f, 0.123f, 0.456f,
        EasyAnalogCleaner::SETTINGS_NO_CHANGES
    );
    testCleanButton(
        "Test that the analog button values are left alone with the "
        "\"no changes\" config,"
        "analog button.",
        0.123f, 0.123f,
        EasyAnalogCleaner::SETTINGS_NO_CHANGES
    );
    testClean(
        "Test that unit circle clamping alone works.",
    0.98f, 0.56f, 0.868243158f, 0.496138901f, {
        .deadzones {
            .angular {
                .interpolate = false
            }
        }
    }
    );
    
    //--- Finish ---
    printf(
        "%sAll %u tests succeeded!%s\n",
        COLOR_GREEN, (unsigned int) nTestsExecuted, COLOR_RESET
    );
    
    return 0;
}


#pragma endregion


#endif //ifdef EASY_ANALOG_CLEANER_UNIT_TESTS
