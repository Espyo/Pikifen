/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Maker tool structures and functions.
 */

#include "console.hpp"

#include "../game.hpp"


#pragma region Maker console


namespace CONSOLE {

//Blink interval for the text input caret.
const float CARET_BLINK_INTERVAL = 0.8f;

//Background color of the console, alpha included.
const ALLEGRO_COLOR COLOR_BG = al_map_rgba(0, 0, 0, 160);

//Highlight color of the console decor details, alpha included.
const ALLEGRO_COLOR COLOR_DECOR_DARK = al_map_rgba(64, 64, 64, 192);

//Shade color of the console decor details, alpha included.
const ALLEGRO_COLOR COLOR_DECOR_LIGHT = al_map_rgba(128, 128, 128, 192);

//Error text color, alpha included.
const ALLEGRO_COLOR COLOR_ERROR = al_map_rgba(255, 128, 128, 192);

//Main text color, alpha included.
const ALLEGRO_COLOR COLOR_MAIN = al_map_rgba(255, 255, 255, 192);

//Muted text color, alpha included.
const ALLEGRO_COLOR COLOR_MUTED = al_map_rgba(255, 255, 255, 128);

//Suggestion text color, alpha included.
const ALLEGRO_COLOR COLOR_SUGGESTION = al_map_rgba(208, 208, 224, 192);

//Warning text color, alpha included.
const ALLEGRO_COLOR COLOR_WARNING = al_map_rgba(255, 255, 128, 192);

//Maximum number of entries in the command history,
//currently-written text included.
const size_t INPUT_HISTORY_MAX_SIZE = 30;

//Size of the control bind icon.
const float INPUT_ICON_SIZE = 24.0f;

//Maximum size of the text input.
const size_t INPUT_TEXT_MAX_SIZE = 80;

//Padding between each line.
const int LINE_PADDING = 2;

//Number of characters to indent the next line of the same
//word-wrapped entry by.
const size_t OUTPUT_INDENT = 2;

//Maximum amount of characters per line of text.
const size_t OUTPUT_MAX_CHARS_PER_LINE = 80;

//Maximum amount of output lines that can be in the console.
const size_t OUTPUT_MAX_SIZE = 100;

//Maximum amount of output lines that can be visible.
const size_t OUTPUT_MAX_VISIBLE_LINES = 10;

//Padding on all sides.
const int PADDING = 8;

//How much the visibility value [0 - 1] changes per second, when opening
//or closing something from the console.
const float VISIBILITY_CHANGE_SPEED = 3.5f;

}


/**
 * @brief Clears everything.
 */
void MakerConsole::clear() {
    terminal.clear();
    notifier.clear();
}


/**
 * @brief Closes the console terminal,
 * gradually hiding it unless specified otherwise.
 *
 * @param instant Whether to close it instantaneously.
 */
void MakerConsole::closeTerminal(bool instant) {
    terminal.close(instant);
}


/**
 * @brief Draws the console terminal and notifier on-screen.
 *
 * @param y Y coordinate to draw at.
 * @return Its height.
 */
int MakerConsole::draw(int y) const {
    int terminalHeight = terminal.draw(y);
    int notifierHeight = notifier.draw(y + terminalHeight + CONSOLE::PADDING);
    return
        terminalHeight +
        (notifierHeight > 0 ? CONSOLE::PADDING + notifierHeight : 0);
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev The event.
 * @return Whether it was handled.
 */
bool MakerConsole::handleAllegroEvent(const ALLEGRO_EVENT& ev) {
    return terminal.handleAllegroEvent(ev);
}


/**
 * @brief Opens the console terminal,
 * gradually showing it unless specified otherwise.
 *
 * @param instant Whether to open it instantaneously.
 */
void MakerConsole::openTerminal(bool instant) {
    terminal.open(instant);
    notifier.close(instant);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void MakerConsole::tick(float deltaT) {
    terminal.tick(deltaT);
    notifier.tick(deltaT);
}


/**
 * @brief Toggles between opening and closing the console terminal,
 * gradually showing or hiding it unless specified otherwise.
 *
 * @param instant Whether to open or close it instantaneously.
 */
void MakerConsole::toggleTerminal(bool instant) {
    switch(terminal.getVisibilityState()) {
    case OPEN_CLOSE_STATE_CLOSED:
    case OPEN_CLOSE_STATE_CLOSING: {
        openTerminal();
        break;
    } case OPEN_CLOSE_STATE_OPEN:
    case OPEN_CLOSE_STATE_OPENING: {
        closeTerminal();
        break;
    }
    }
}


/**
 * @brief Writes a line of output.
 *
 * @param level Log level.
 * @param text The line of text.
 * @param notificationDuration If a notification is meant to appear
 * in the notifier, specify the duration here. Otherwise, this should be 0.
 */
void MakerConsole::write(
    LOG_LEVEL level, const string& text, float notificationDuration
) {
    terminal.write(level, text);
    
    OPEN_CLOSE_STATE terminalVisibilityState = terminal.getVisibilityState();
    bool isTerminalVisible =
        terminalVisibilityState == OPEN_CLOSE_STATE_OPEN ||
        terminalVisibilityState == OPEN_CLOSE_STATE_OPENING;
        
    if(notificationDuration > 0.0f && !isTerminalVisible) {
        notifier.write(level, text, notificationDuration);
    }
}


#pragma endregion
#pragma region Maker console notifier


/**
 * @brief Clears the data of the notifier.
 */
void MakerConsoleNotifier::clear() {
    text.clear();
    visibilityState = OPEN_CLOSE_STATE_CLOSED;
    visibility = 0.0f;
    timeLeft = 0.0f;
}


/**
 * @brief Closes the console notifier,
 * gradually hiding it unless specified otherwise.
 *
 * @param instant Whether to close it instantaneously.
 */
void MakerConsoleNotifier::close(bool instant) {
    switch(visibilityState) {
    case OPEN_CLOSE_STATE_CLOSED:
    case OPEN_CLOSE_STATE_CLOSING: {
        break;
    } case OPEN_CLOSE_STATE_OPEN:
    case OPEN_CLOSE_STATE_OPENING: {
        if(instant) {
            visibilityState = OPEN_CLOSE_STATE_CLOSED;
            visibility = 0.0f;
        } else {
            visibilityState = OPEN_CLOSE_STATE_CLOSING;
        }
        break;
    }
    }
}


/**
 * @brief Draws the console notifier on-screen.
 *
 * @param y Y coordinate to draw at.
 * @return Its height.
 */
int MakerConsoleNotifier::draw(int y) const {
    if(visibilityState == OPEN_CLOSE_STATE_CLOSED) return 0;
    
    //Setup.
    const ALLEGRO_FONT* FONT = game.sysContent.fntBuiltin;
    const string wrappedText =
        wordWrap(
            text, CONSOLE::OUTPUT_MAX_CHARS_PER_LINE, CONSOLE::OUTPUT_INDENT
        );
    const vector<string> lines = split(wrappedText, "\n", true);
    const int fontHeight = al_get_font_line_height(FONT);
    
    int longestLineWidth = 0;
    forIdx(l, lines) {
        longestLineWidth =
            std::max(
                longestLineWidth,
                al_get_text_width(FONT, lines[l].c_str())
            );
    }
    
    int textW = longestLineWidth;
    int textH =
        lines.size() * fontHeight +
        (lines.size() - 1) * CONSOLE::LINE_PADDING;
    int boxW =
        CONSOLE::PADDING + CONSOLE::INPUT_ICON_SIZE + CONSOLE::PADDING +
        textW + CONSOLE::PADDING;
    boxW = std::max(boxW, (int) (game.winW / 2.0f) - CONSOLE::PADDING * 2);
    int boxH =
        std::max(
            CONSOLE::PADDING + (int) CONSOLE::INPUT_ICON_SIZE + CONSOLE::PADDING,
            CONSOLE::PADDING + textH + CONSOLE::PADDING
        );
    int boxX = CONSOLE::PADDING;
    int boxY = y;
    int textX =
        boxX + CONSOLE::PADDING + CONSOLE::INPUT_ICON_SIZE + CONSOLE::PADDING;
    int textY = (boxY + boxH / 2.0f) - textH / 2.0f;
    
    float alpha = visibility;
    
    const auto getLineY =
    [textY, fontHeight] (size_t idx) {
        return textY + (idx * (fontHeight + CONSOLE::LINE_PADDING));
    };
    
    //Background.
    al_draw_filled_rectangle(
        boxX, boxY, boxX + boxW, boxY + boxH,
        multAlpha(CONSOLE::COLOR_BG, alpha)
    );
    
    //Text lines.
    forIdx(l, lines) {
        int lineY = getLineY(l);
        ALLEGRO_COLOR color = COLOR_WHITE;
        
        switch(level) {
        case LOG_LEVEL_INFO: {
            color = CONSOLE::COLOR_MAIN;
            break;
        } case LOG_LEVEL_WARNING: {
            color = CONSOLE::COLOR_WARNING;
            break;
        } case LOG_LEVEL_ERROR: {
            color = CONSOLE::COLOR_ERROR;
            break;
        }
        }
        
        drawText(
            lines[l], FONT,
            Point(textX, lineY),
            Point(textW, fontHeight),
            multAlpha(color, alpha),
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP,
            TEXT_SETTING_FLAG_CANT_GROW | TEXT_SETTING_FLAG_CANT_SHRINK
        );
    }
    
    //Terminal toggle input.
    drawPlayerActionInputSourceIcon(
        PLAYER_ACTION_TYPE_MT_CONSOLE,
        Point(
            boxX + CONSOLE::PADDING + (CONSOLE::INPUT_ICON_SIZE / 2.0f),
            boxY + (boxH / 2.0f)
        ),
        Point(CONSOLE::INPUT_ICON_SIZE),
        true, game.sysContent.fntSlim, mapAlpha(alpha * 255)
    );
    
    //Decor.
    al_draw_line(
        boxX + boxW * 0.00f, boxY - 1.0f,
        boxX + boxW * 1.00f, boxY - 1.0f,
        multAlpha(CONSOLE::COLOR_DECOR_LIGHT, alpha), 2.0f
    );
    al_draw_line(
        boxX + boxW * 0.00f, boxY + 1.0f,
        boxX + boxW * 1.00f, boxY + 1.0f,
        multAlpha(CONSOLE::COLOR_DECOR_DARK, alpha), 2.0f
    );
    
    return boxH;
}


/**
 * @brief Opens the console notifier,
 * gradually showing it unless specified otherwise.
 *
 * @param instant Whether to open it instantaneously.
 */
void MakerConsoleNotifier::open(bool instant) {
    switch(visibilityState) {
    case OPEN_CLOSE_STATE_OPEN:
    case OPEN_CLOSE_STATE_OPENING: {
        break;
    } case OPEN_CLOSE_STATE_CLOSED:
    case OPEN_CLOSE_STATE_CLOSING: {
        if(instant) {
            visibilityState = OPEN_CLOSE_STATE_OPEN;
            visibility = 1.0f;
        } else {
            visibilityState = OPEN_CLOSE_STATE_OPENING;
        }
        break;
    }
    }
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void MakerConsoleNotifier::tick(float deltaT) {
    if(timeLeft > 0.0f) {
        timeLeft -= deltaT;
        if(timeLeft <= 0.0f) {
            timeLeft = 0.0f;
            close();
        }
    }
    
    switch(visibilityState) {
    case OPEN_CLOSE_STATE_CLOSED:
    case OPEN_CLOSE_STATE_OPEN: {
        break;
    } case OPEN_CLOSE_STATE_CLOSING:
    case OPEN_CLOSE_STATE_OPENING: {
        visibility =
            inchTowards(
                visibility,
                visibilityState == OPEN_CLOSE_STATE_CLOSING ? 0.0f : 1.0f,
                CONSOLE::VISIBILITY_CHANGE_SPEED * deltaT
            );
        if(visibility >= 1.0f) {
            visibility = 1.0f;
            visibilityState = OPEN_CLOSE_STATE_OPEN;
        } else if(visibility <= 0.0f) {
            visibility = 0.0f;
            visibilityState = OPEN_CLOSE_STATE_CLOSED;
        }
        break;
    }
    }
}


/**
 * @brief Sets the text to show on the notifier.
 *
 * @param level Log level.
 * @param text Text to show.
 * @param duration How long to show the notification for.
 */
void MakerConsoleNotifier::write(
    LOG_LEVEL level, const string& text, float duration
) {
    this->text = text;
    this->level = level;
    timeLeft = duration;
    open();
}


#pragma endregion
#pragma region Maker console terminal


MakerConsoleTerminal::MakerConsoleTerminal() {
    input.maxSize = CONSOLE::INPUT_TEXT_MAX_SIZE;
    commandHistory.assign(1, ""); //The currently-written text.
}


/**
 * @brief Accepts the current suggestion.
 */
void MakerConsoleTerminal::acceptSuggestion() {
    string suggestion = getSuggestion();
    if(suggestion.empty()) return;
    
    string text = input.getString();
    size_t wordStart, wordEnd;
    getWordStartAndEnd(text, input.getCaretPos(), &wordStart, &wordEnd);
    
    text.replace(wordStart, wordEnd - wordStart, suggestion);
    if(text.back() != ' ') text.push_back(' ');
    
    input.setString(text);
    input.setCaretPos(wordStart + suggestion.size() + 1);
}


/**
 * @brief Browses to a point of the command history, if possible.
 *
 * @param offset Offset compared to the current point of the command history.
 */
void MakerConsoleTerminal::browseCommandHistory(int offset) {
    int oldIdxInt = commandHistoryIndex;
    int newIdxInt = commandHistoryIndex + offset;
    newIdxInt = std::clamp(newIdxInt, 0, (int) commandHistory.size() - 1);
    
    if(newIdxInt == oldIdxInt) return;
    
    if(oldIdxInt == 0 && newIdxInt != 0) {
        commandHistory[0] = input.getString();
    }
    
    commandHistoryIndex = newIdxInt;
    
    input.setString(commandHistory[newIdxInt], true);
}


/**
 * @brief Clears the list of output lines.
 */
void MakerConsoleTerminal::clear() {
    output.clear();
    outputTimestamps.clear();
    outputLevels.clear();
    scroll = 0;
    input.reset();
    visibilityState = OPEN_CLOSE_STATE_CLOSED;
    visibility = 0.0f;
}


/**
 * @brief Closes the console terminal,
 * gradually hiding it unless specified otherwise.
 *
 * @param instant Whether to close it instantaneously.
 */
void MakerConsoleTerminal::close(bool instant) {
    game.controls.releaseAll();
    switch(visibilityState) {
    case OPEN_CLOSE_STATE_CLOSED:
    case OPEN_CLOSE_STATE_CLOSING: {
        break;
    } case OPEN_CLOSE_STATE_OPEN:
    case OPEN_CLOSE_STATE_OPENING: {
        if(instant) {
            visibilityState = OPEN_CLOSE_STATE_CLOSED;
            visibility = 0.0f;
        } else {
            visibilityState = OPEN_CLOSE_STATE_CLOSING;
        }
        break;
    }
    }
}


/**
 * @brief Draws the console terminal on-screen.
 *
 * @param y Y coordinate to draw at.
 * @return Its height.
 */
int MakerConsoleTerminal::draw(int y) const {
    if(visibilityState == OPEN_CLOSE_STATE_CLOSED) return 0;
    
    //Setup.
    const ALLEGRO_FONT* FONT = game.sysContent.fntBuiltin;
    const string INPUT_LINE_PREFIX = "> ";
    const size_t nLines = CONSOLE::OUTPUT_MAX_VISIBLE_LINES + 2;
    const int fontHeight = al_get_font_line_height(FONT);
    
    int textW = game.winW - CONSOLE::PADDING * 2;
    int textH =
        nLines * fontHeight +
        (nLines - 1) * CONSOLE::LINE_PADDING;
    int boxW = game.winW;
    int boxH = CONSOLE::PADDING + textH + CONSOLE::PADDING;
    int boxX = 0;
    int boxY = y;
    int textX = boxX + CONSOLE::PADDING;
    int textY = boxY + CONSOLE::PADDING;
    
    int yOffset = -(boxH * (1.0f - ease(visibility, EASE_METHOD_IN_OUT)));
    boxY += yOffset;
    textY += yOffset;
    
    const auto getLineY =
    [textY, fontHeight] (size_t idx) {
        return textY + (idx * (fontHeight + CONSOLE::LINE_PADDING));
    };
    
    //Background.
    al_draw_filled_rectangle(
        boxX, boxY, boxX + boxW, boxY + boxH,
        CONSOLE::COLOR_BG
    );
    
    //Scrollbar.
    if(output.size() > 1) {
        bool hasMoreAbove = scroll < output.size() - 1;
        drawFilledEquilateralTriangle(
            Point(boxX + boxW - 8.0f, boxY + 8.0f),
            6.0f, -(TAU / 4.0f),
            multAlpha(CONSOLE::COLOR_MUTED, hasMoreAbove ? 1.0f : 0.3f)
        );
        bool hasMoreBelow = scroll > 0;
        drawFilledEquilateralTriangle(
            Point(
                boxX + boxW - 8.0f,
                getLineY(CONSOLE::OUTPUT_MAX_VISIBLE_LINES) - 8.0f
            ), 6.0f, TAU / 4.0f,
            multAlpha(CONSOLE::COLOR_MUTED, hasMoreBelow ? 1.0f : 0.3f)
        );
        int scrollZoneY1 = boxY + 18.0f;
        int scrollZoneY2 =
            getLineY(CONSOLE::OUTPUT_MAX_VISIBLE_LINES) - 18.0f;
        float scrollRatio = scroll / (float) (output.size() - 1);
        int scrollY =
            interpolateNumber(
                scrollRatio, 0.0f, 1.0f, scrollZoneY2, scrollZoneY1
            );
        al_draw_line(
            boxX + boxW - 14.0f, scrollY,
            boxX + boxW - 2.0f, scrollY,
            CONSOLE::COLOR_MUTED, 6.0f
        );
    }
    
    //Calculate the lines to show.
    vector<string> lines;
    vector<float> lineTimestamps;
    vector<LOG_LEVEL> lineLevels;
    size_t entryIt = scroll;
    size_t entriesDrawn = 0;
    
    while(true) {
        bool done = false;
        int entryIdx = output.size() - entryIt - 1;
        if(entryIdx < 0) break;
        if(entryIdx >= (int) output.size()) break;
        if(entriesDrawn >= CONSOLE::OUTPUT_MAX_VISIBLE_LINES) break;
        
        //Split the entry into lines, since it may be too long to fit
        //in the game window.
        const string wrappedEntry =
            wordWrap(
                output[entryIdx],
                CONSOLE::OUTPUT_MAX_CHARS_PER_LINE, CONSOLE::OUTPUT_INDENT
            );
        const vector<string> entryLines = split(wrappedEntry, "\n");
        
        //Add each line in reverse order.
        for(size_t l = 0; l < entryLines.size(); l++) {
            int entryLineIdx = entryLines.size() - l - 1;
            
            lines.push_back(entryLines[entryLineIdx]);
            if(entryLineIdx == 0) {
                lineTimestamps.push_back(outputTimestamps[entryIdx]);
            } else {
                lineTimestamps.push_back(-1.0f);
            }
            lineLevels.push_back(outputLevels[entryIdx]);
            
            if(lines.size() >= CONSOLE::OUTPUT_MAX_VISIBLE_LINES) {
                done = true;
                break;
            }
        }
        
        if(done) break;
        entryIt++;
        entriesDrawn++;
    }
    
    //Text lines.
    for(size_t l = 0; l < lines.size(); l++) {
        int entryYIdx = CONSOLE::OUTPUT_MAX_VISIBLE_LINES - l;
        int entryY = getLineY(entryYIdx - 1);
        
        //Timestamp.
        string timestampStr;
        if(lineTimestamps[l] >= 0.0f) {
            timestampStr =
                resizeString(f2s(lineTimestamps[l]), 8, true, true, true);
            timestampStr = "[" + timestampStr + "] ";
        } else {
            timestampStr = string(11, ' ');
        }
        int timestampWidth = al_get_text_width(FONT, timestampStr.c_str());
        drawText(
            timestampStr, FONT,
            Point(textX, entryY),
            Point(timestampWidth, fontHeight),
            CONSOLE::COLOR_MUTED, ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP,
            TEXT_SETTING_FLAG_CANT_GROW | TEXT_SETTING_FLAG_CANT_SHRINK
        );
        
        //Text.
        ALLEGRO_COLOR color = COLOR_WHITE;
        switch(lineLevels[l]) {
        case LOG_LEVEL_INFO: {
            color = CONSOLE::COLOR_MAIN;
            break;
        } case LOG_LEVEL_WARNING: {
            color = CONSOLE::COLOR_WARNING;
            break;
        } case LOG_LEVEL_ERROR: {
            color = CONSOLE::COLOR_ERROR;
            break;
        }
        }
        
        drawText(
            lines[l], FONT,
            Point(textX + timestampWidth, entryY),
            Point(textW - timestampWidth, fontHeight), color,
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP,
            TEXT_SETTING_FLAG_CANT_GROW | TEXT_SETTING_FLAG_CANT_SHRINK
        );
    }
    
    //Divisor line between output and input.
    int lineY = getLineY(CONSOLE::OUTPUT_MAX_VISIBLE_LINES) + fontHeight / 2.0f;
    al_draw_line(
        textX, lineY, textX + textW, lineY,
        CONSOLE::COLOR_MUTED, 2.0f
    );
    
    //Input text.
    string inputLineFullText = INPUT_LINE_PREFIX + input.getString();
    int inputLineTextY = getLineY(CONSOLE::OUTPUT_MAX_VISIBLE_LINES + 1);
    drawText(
        inputLineFullText, FONT,
        Point(textX, inputLineTextY),
        Point(textW, fontHeight),
        CONSOLE::COLOR_MAIN, ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP,
        TEXT_SETTING_FLAG_CANT_GROW | TEXT_SETTING_FLAG_CANT_SHRINK
    );
    
    //Input text caret.
    float caretT = fmod(game.timePassed, CONSOLE::CARET_BLINK_INTERVAL);
    int caretX = 0;
    int prefixWidth = al_get_text_width(FONT, INPUT_LINE_PREFIX.c_str());
    string inputPreCaretText =
        input.getString().substr(0, input.getCaretPos());
    int inputPreCaretTextWidth =
        al_get_text_width(FONT, inputPreCaretText.c_str());
    caretX = textX + prefixWidth + inputPreCaretTextWidth;
    
    if(caretT < CONSOLE::CARET_BLINK_INTERVAL / 2.0f) {
        al_draw_line(
            caretX, inputLineTextY,
            caretX, inputLineTextY + fontHeight,
            CONSOLE::COLOR_MAIN, 2.0f
        );
    }
    
    //Suggestion.
    string suggestion = getSuggestion();
    if(!suggestion.empty()) {
        int suggestionW = al_get_text_width(FONT, suggestion.c_str());
        Rect suggestionRect(
            Point(caretX, getLineY(CONSOLE::OUTPUT_MAX_VISIBLE_LINES) - 8.0f),
            Point(suggestionW + 6.0f, fontHeight + 6.0f)
        );
        suggestionRect =
            clampRectPosToRect(
                suggestionRect,
                rectCornersToRect(
                    RectCorners(Point(0.0f), Point(game.winW, game.winH))
                )
            );
        al_draw_filled_rectangle(
            suggestionRect.center.x - suggestionRect.size.x / 2.0f,
            suggestionRect.center.y - suggestionRect.size.y / 2.0f,
            suggestionRect.center.x + suggestionRect.size.x / 2.0f,
            suggestionRect.center.y + suggestionRect.size.y / 2.0f,
            CONSOLE::COLOR_BG
        );
        al_draw_rectangle(
            suggestionRect.center.x - suggestionRect.size.x / 2.0f,
            suggestionRect.center.y - suggestionRect.size.y / 2.0f,
            suggestionRect.center.x + suggestionRect.size.x / 2.0f,
            suggestionRect.center.y + suggestionRect.size.y / 2.0f,
            CONSOLE::COLOR_SUGGESTION, 1.0f
        );
        drawText(
            suggestion, FONT, suggestionRect.center, suggestionRect.size,
            CONSOLE::COLOR_SUGGESTION,
            ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER,
            TEXT_SETTING_FLAG_FIXED_SIZE
        );
    }
    
    //Decor.
    al_draw_line(
        0.0f, yOffset + boxH - 1.0f,
        game.winW, yOffset + boxH - 1.0f,
        CONSOLE::COLOR_DECOR_LIGHT, 2.0f
    );
    al_draw_line(
        0.0f, yOffset + boxH + 1.0f,
        game.winW, yOffset + boxH + 1.0f,
        CONSOLE::COLOR_DECOR_DARK, 2.0f
    );
    
    return boxH + yOffset;
}


/**
 * @brief Returns a suggestion for the word the user is currently typing.
 *
 * @return The suggestion, or an empty string if there is none.
 */
string MakerConsoleTerminal::getSuggestion() const {
    string text = input.getString();
    if(text.empty()) return "";
    string result;
    
    //Figure out the word.
    size_t wordStart, wordEnd;
    getWordStartAndEnd(text, input.getCaretPos(), &wordStart, &wordEnd);
    string word = text.substr(wordStart, wordEnd - wordStart);
    
    size_t wordIdx = 0;
    size_t prevSpaceIdx = string::npos;
    while(true) {
        prevSpaceIdx =
            text.find(' ', prevSpaceIdx == string::npos ? 0 : prevSpaceIdx + 1);
        if(prevSpaceIdx >= wordEnd) {
            break;
        }
        wordIdx++;
    }
    
    //Figure out what to suggest with.
    if(wordIdx == 0) {
        vector<string> possibleItems;
        forIdx(t, game.makerTools.types) {
            MakerToolType* tPtr = &game.makerTools.types[t];
            if(tPtr->type == MAKER_TOOL_TYPE_NONE) continue;
            possibleItems.push_back(tPtr->name);
        }
        
        possibleItems = getStringSubsequenceMatches(word, possibleItems);
        
        if(!possibleItems.empty()) result = possibleItems[0];
    }
    
    if(word == result) {
        //Already the result.
        return "";
    }
    
    return result;
}


/**
 * @brief Returns the current visibility state.
 *
 * @return The state.
 */
OPEN_CLOSE_STATE MakerConsoleTerminal::getVisibilityState() const {
    return visibilityState;
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev The event.
 * @return Whether it was handled.
 */
bool MakerConsoleTerminal::handleAllegroEvent(const ALLEGRO_EVENT& ev) {
    if(visibilityState == OPEN_CLOSE_STATE_CLOSED) return false;
    
    if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        if(
            game.controls.getInputSourceIsBoundToActionType(
                game.controls.allegroCharEventToInput(ev).source,
                PLAYER_ACTION_TYPE_MT_CONSOLE
            )
        ) {
            //The user probably wants to toggle the console, not type
            //whatever character it matches.
            close();
            return true;
        }
        
        switch(ev.keyboard.keycode) {
        case ALLEGRO_KEY_UP: {
            browseCommandHistory(+1);
            break;
        } case ALLEGRO_KEY_DOWN: {
            browseCommandHistory(-1);
            break;
        } case ALLEGRO_KEY_PGUP: {
            if(output.size() > 1 && scroll < output.size() - 1) scroll++;
            break;
        } case ALLEGRO_KEY_PGDN: {
            if(output.size() > 1 && scroll > 0) scroll--;
            break;
        } case ALLEGRO_KEY_ENTER: {
            if(input.getString().size() > 0) submitCommand();
            break;
        } case ALLEGRO_KEY_TAB: {
            if(input.getString().size() > 0) acceptSuggestion();
            break;
        } case ALLEGRO_KEY_ESCAPE: {
            close();
            break;
        }
        }
    }
    
    if(input.handleAllegroEvent(ev)) {
        if(commandHistoryIndex != 0) {
            //We're showing a history item, but the user edited it, so
            //reset and make it seem like this is what the user's writing.
            commandHistoryIndex = 0;
            commandHistory[0] = input.getString();
        }
    }
    
    return true;
}


/**
 * @brief Opens the console terminal,
 * gradually showing it unless specified otherwise.
 *
 * @param instant Whether to open it instantaneously.
 */
void MakerConsoleTerminal::open(bool instant) {
    game.controls.releaseAll();
    switch(visibilityState) {
    case OPEN_CLOSE_STATE_OPEN:
    case OPEN_CLOSE_STATE_OPENING: {
        break;
    } case OPEN_CLOSE_STATE_CLOSED:
    case OPEN_CLOSE_STATE_CLOSING: {
        if(instant) {
            visibilityState = OPEN_CLOSE_STATE_OPEN;
            visibility = 1.0f;
        } else {
            visibilityState = OPEN_CLOSE_STATE_OPENING;
        }
        break;
    }
    }
}


/**
 * @brief Submits the currently written command in the input.
 */
void MakerConsoleTerminal::submitCommand() {
    //Initial parsing and checks.
    string inputString = trimSpaces(input.getString());
    
    if(inputString.empty()) return;
    
    //Acknowledge the submission.
    write(LOG_LEVEL_INFO, "> " + inputString);
    commandHistory[0] = input.getString();
    commandHistory.insert(commandHistory.begin(), "");
    if(commandHistory.size() > CONSOLE::INPUT_HISTORY_MAX_SIZE) {
        commandHistory.pop_back();
    }
    input.reset();
    
    //Check if maker tools are not allowed right now.
    if(!game.makerTools.checkMakerToolsAllowed(1.0f)) return;
    
    //Further cleanup.
    vector<string> inputWords = split(inputString);
    forIdx(w, inputWords) {
        inputWords[w] = trimSpaces(inputWords[w]);
    }
    
    //Run!
    string toolName = inputWords[0];
    eraseInVector(inputWords, 0);
    game.makerTools.runTool(toolName, inputWords);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void MakerConsoleTerminal::tick(float deltaT) {
    switch(visibilityState) {
    case OPEN_CLOSE_STATE_CLOSED:
    case OPEN_CLOSE_STATE_OPEN: {
        break;
    } case OPEN_CLOSE_STATE_CLOSING:
    case OPEN_CLOSE_STATE_OPENING: {
        visibility =
            inchTowards(
                visibility,
                visibilityState == OPEN_CLOSE_STATE_CLOSING ? 0.0f : 1.0f,
                CONSOLE::VISIBILITY_CHANGE_SPEED * deltaT
            );
        if(visibility >= 1.0f) {
            visibility = 1.0f;
            visibilityState = OPEN_CLOSE_STATE_OPEN;
        } else if(visibility <= 0.0f) {
            visibility = 0.0f;
            visibilityState = OPEN_CLOSE_STATE_CLOSED;
        }
        break;
    }
    }
}


/**
 * @brief Toggles between opening and closing the console terminal,
 * gradually showing or hiding it unless specified otherwise.
 *
 * @param instant Whether to open or close it instantaneously.
 */
void MakerConsoleTerminal::toggle(bool instant) {
    switch(visibilityState) {
    case OPEN_CLOSE_STATE_CLOSED:
    case OPEN_CLOSE_STATE_CLOSING: {
        open(instant);
        break;
    } case OPEN_CLOSE_STATE_OPEN:
    case OPEN_CLOSE_STATE_OPENING: {
        close(instant);
        break;
    }
    }
}


/**
 * @brief Adds an output line.
 *
 * @param level Log level.
 * @param text Text to add. It can have line breaks, in which case it's split
 * into different output lines.
 */
void MakerConsoleTerminal::write(LOG_LEVEL level, const string& text) {
    vector<string> lines = split(text, "\n", true);
    forIdx(l, lines) {
        output.push_back(lines[l]);
        outputTimestamps.push_back(game.timePassed);
        outputLevels.push_back(level);
    }
    
    if(scroll > 0) scroll += lines.size();
    
    while(output.size() > CONSOLE::OUTPUT_MAX_SIZE) {
        eraseInVector(output, 0);
        eraseInVector(outputTimestamps, 0);
        eraseInVector(outputLevels, 0);
    }
    
    scroll = std::min(scroll, output.size() - 1);
}


#pragma endregion
