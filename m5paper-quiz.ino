#include <M5Unified.h>
#include <M5GFX.h>

// ─── Data ────────────────────────────────────────────────────────────────────

struct QuizItem {
    const char* phrase;
    const char* hint;
    const char* answer;
};

static const QuizItem MBA_ITEMS[] = {
    { "The process of identifying, assessing, and controlling threats to an organization's capital and earnings.",
      "Think about uncertainty and potential loss.",
      "Risk Management" },
    { "A framework that maps out the activities required to create and deliver a product or service.",
      "Porter introduced this concept.",
      "Value Chain Analysis" },
    { "A measure of how efficiently a company uses its assets to generate profit.",
      "Net income divided by total assets.",
      "Return on Assets (ROA)" },
    { "The minimum viable set of features needed to satisfy early customers and gather feedback.",
      "Acronym: MVP.",
      "Minimum Viable Product" },
    { "A strategic planning technique that evaluates Strengths, Weaknesses, Opportunities, and Threats.",
      "Four-quadrant internal/external analysis.",
      "SWOT Analysis" },
};

static const QuizItem MANAGEMENT_ITEMS[] = {
    { "A leadership style where the manager makes all decisions without input from the team.",
      "Opposite of democratic leadership.",
      "Autocratic Leadership" },
    { "The tendency for people to work less hard when they are part of a group.",
      "Ringelmann effect is related to this.",
      "Social Loafing" },
    { "A goal-setting framework where objectives are Specific, Measurable, Achievable, Relevant, and Time-bound.",
      "Acronym used in performance reviews.",
      "SMART Goals" },
    { "The management theory that views organizations as systems with interrelated parts.",
      "Borrowed from biology and engineering.",
      "Systems Theory" },
    { "The number of subordinates a manager directly supervises.",
      "Wide vs. narrow versions affect org hierarchy.",
      "Span of Control" },
};

static const QuizItem ADVENTURE_ITEMS[] = {
    { "The technique of using a map and compass to navigate without electronic aids.",
      "Triangulating landmarks is key.",
      "Land Navigation / Orienteering" },
    { "A knot used to create a fixed loop at the end of a rope that does not slip under load.",
      "Named after a type of post.",
      "Bowline Knot" },
    { "The rule of thumb for treating hypothermia in the field: remove wet clothing, insulate, and supply warm fluids if conscious.",
      "Heat loss prevention, not heat addition.",
      "Passive Rewarming" },
    { "A rappelling technique where you descend facing downhill with the rope running across your body.",
      "Named after an Austrian mountaineer.",
      "Dulfersitz (Body Rappel)" },
    { "The Leave No Trace principle that covers disposing of waste properly, minimizing campfire impact, and respecting wildlife.",
      "Seven principles total; this covers your footprint.",
      "Leave No Trace (LNT) Ethics" },
};

static const QuizItem GENERAL_ITEMS[] = {
    { "The phenomenon where a body submerged in a fluid experiences an upward force equal to the weight of the fluid it displaces.",
      "Greek mathematician in a bathtub.",
      "Archimedes' Principle" },
    { "The logical fallacy of assuming that because something has happened in the past, it will continue to happen.",
      "Often used to critique technical analysis in finance.",
      "Gambler's Fallacy / Inductive Fallacy" },
    { "The SI unit of electric resistance, defined as the resistance between two points when a constant potential difference of one volt produces a current of one ampere.",
      "Symbol: Ω",
      "Ohm" },
    { "The cognitive bias where people place more value on things they own than on identical things they do not own.",
      "Kahneman and Thaler studied this.",
      "Endowment Effect" },
    { "The treaty signed in 1648 that ended the Thirty Years' War and established the modern concept of state sovereignty.",
      "Named after a region in Germany.",
      "Peace of Westphalia" },
};

// ─── Constants ───────────────────────────────────────────────────────────────

#define NUM_CATEGORIES 4
static const char* CATEGORY_NAMES[NUM_CATEGORIES] = { "MBA", "Management", "Adventure", "General" };
static const QuizItem* CATEGORY_DATA[NUM_CATEGORIES] = {
    MBA_ITEMS, MANAGEMENT_ITEMS, ADVENTURE_ITEMS, GENERAL_ITEMS
};
static const int CATEGORY_SIZES[NUM_CATEGORIES] = { 5, 5, 5, 5 };

// Layout constants (portrait 540×960)
#define SCREEN_W     540
#define SCREEN_H     960
#define MARGIN       36
#define BTN_H        80
#define BTN_W        220
#define BTN_Y        (SCREEN_H - MARGIN - BTN_H)
#define BTN_LEFT_X   MARGIN
#define BTN_RIGHT_X  (SCREEN_W - MARGIN - BTN_W)

// ─── State ───────────────────────────────────────────────────────────────────

enum Screen { SCREEN_SELECT, SCREEN_QUIZ };
enum QuizState { STATE_QUESTION, STATE_HINT, STATE_REVEALED };

Screen currentScreen = SCREEN_SELECT;
int selectedCategory = 0;
int currentItem = 0;
QuizState quizState = STATE_QUESTION;
bool needsFullRefresh = false;

// ─── Drawing helpers ─────────────────────────────────────────────────────────

void drawButton(int x, int y, int w, int h, const char* label, bool inverted = false) {
    if (inverted) {
        M5.Display.fillRoundRect(x, y, w, h, 10, TFT_BLACK);
        M5.Display.setTextColor(TFT_WHITE);
    } else {
        M5.Display.fillRoundRect(x, y, w, h, 10, TFT_WHITE);
        M5.Display.drawRoundRect(x, y, w, h, 10, TFT_BLACK);
        M5.Display.setTextColor(TFT_BLACK);
    }
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString(label, x + w / 2, y + h / 2);
}

// Wrap text and return height used
int drawWrappedText(const char* text, int x, int y, int maxW, int textSize) {
    M5.Display.setTextSize(textSize);
    M5.Display.setTextDatum(TL_DATUM);
    int lineH = M5.Display.fontHeight() + 4;
    int spaceW = M5.Display.textWidth(" ");

    String word = "";
    String line = "";
    int curY = y;

    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        if (c == ' ' || c == '\n' || text[i + 1] == '\0') {
            if (text[i + 1] == '\0' && c != ' ' && c != '\n') word += c;
            if (line.length() == 0) {
                line = word;
            } else if ((int)M5.Display.textWidth((line + " " + word).c_str()) <= maxW) {
                line += " ";
                line += word;
            } else {
                M5.Display.drawString(line.c_str(), x, curY);
                curY += lineH;
                line = word;
            }
            word = "";
            if (c == '\n') {
                M5.Display.drawString(line.c_str(), x, curY);
                curY += lineH;
                line = "";
            }
        } else {
            word += c;
        }
    }
    if (line.length() > 0) {
        M5.Display.drawString(line.c_str(), x, curY);
        curY += lineH;
    }
    return curY - y;
}

// ─── Selection Screen ─────────────────────────────────────────────────────────

void drawSelectionScreen() {
    M5.Display.startWrite();
    M5.Display.fillScreen(TFT_WHITE);

    // Title
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Memory Quizlet", SCREEN_W / 2, MARGIN);

    // Category buttons stacked vertically, centered
    int btnH = 90;
    int btnW = SCREEN_W - 2 * MARGIN;
    int startY = 160;
    int gap = 24;
    int btnX = MARGIN;

    for (int i = 0; i < NUM_CATEGORIES; i++) {
        int by = startY + i * (btnH + gap);
        M5.Display.fillRoundRect(btnX, by, btnW, btnH, 10, TFT_WHITE);
        M5.Display.drawRoundRect(btnX, by, btnW, btnH, 10, TFT_BLACK);
        M5.Display.setTextColor(TFT_BLACK);
        M5.Display.setTextSize(3);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString(CATEGORY_NAMES[i], btnX + btnW / 2, by + btnH / 2);
    }

    // Exit button
    int exitY = SCREEN_H - MARGIN - BTN_H;
    drawButton(BTN_LEFT_X, exitY, BTN_W, BTN_H, "Exit");

    M5.Display.endWrite();
}

bool handleSelectionTouch(int tx, int ty) {
    int btnH = 90;
    int btnW = SCREEN_W - 2 * MARGIN;
    int startY = 160;
    int gap = 24;
    int btnX = MARGIN;

    for (int i = 0; i < NUM_CATEGORIES; i++) {
        int by = startY + i * (btnH + gap);
        if (tx >= btnX && tx <= btnX + btnW && ty >= by && ty <= by + btnH) {
            selectedCategory = i;
            currentItem = 0;
            quizState = STATE_QUESTION;
            currentScreen = SCREEN_QUIZ;
            return true;
        }
    }

    // Exit button
    int exitY = SCREEN_H - MARGIN - BTN_H;
    if (tx >= BTN_LEFT_X && tx <= BTN_LEFT_X + BTN_W && ty >= exitY && ty <= exitY + BTN_H) {
        M5.Display.setEpdMode(epd_quality);
        M5.Display.startWrite();
        M5.Display.fillScreen(TFT_WHITE);
        for (int i = 0; i < 12; i++) {
            int cx = random(MARGIN, SCREEN_W - MARGIN);
            int cy = random(MARGIN, SCREEN_H - MARGIN);
            int r  = random(20, 120);
            M5.Display.drawCircle(cx, cy, r, TFT_BLACK);
        }
        M5.Display.endWrite();
        M5.Power.powerOff();
    }

    return false;
}

// ─── Quiz Screen ──────────────────────────────────────────────────────────────

void drawQuizScreen() {
    const QuizItem& item = CATEGORY_DATA[selectedCategory][currentItem];

    M5.Display.startWrite();
    M5.Display.fillScreen(TFT_WHITE);

    // Header: category and item number
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TL_DATUM);
    char header[64];
    snprintf(header, sizeof(header), "%s  —  %d / %d",
             CATEGORY_NAMES[selectedCategory],
             currentItem + 1,
             CATEGORY_SIZES[selectedCategory]);
    M5.Display.drawString(header, MARGIN, 14);

    // Divider
    int divY = 14 + M5.Display.fontHeight() + 8;
    M5.Display.drawLine(MARGIN, divY, SCREEN_W - MARGIN, divY, TFT_BLACK);

    // Phrase
    int textAreaW = SCREEN_W - 2 * MARGIN;
    int curY = divY + 14;

    M5.Display.setTextColor(TFT_BLACK);
    int phraseH = drawWrappedText(item.phrase, MARGIN, curY, textAreaW, 3);
    curY += phraseH + 16;

    // Hint
    if (quizState == STATE_HINT || quizState == STATE_REVEALED) {
        M5.Display.drawLine(MARGIN, curY, SCREEN_W - MARGIN, curY, TFT_DARKGREY);
        curY += 10;
        M5.Display.setTextColor(TFT_DARKGREY);
        M5.Display.setTextSize(2);
        M5.Display.setTextDatum(TL_DATUM);
        M5.Display.drawString("Hint:", MARGIN, curY);
        curY += M5.Display.fontHeight() + 4;
        M5.Display.setTextColor(TFT_BLACK);
        int hintH = drawWrappedText(item.hint, MARGIN, curY, textAreaW, 3);
        curY += hintH + 16;
    }

    // Answer
    if (quizState == STATE_REVEALED) {
        M5.Display.drawLine(MARGIN, curY, SCREEN_W - MARGIN, curY, TFT_DARKGREY);
        curY += 10;
        M5.Display.setTextColor(TFT_DARKGREY);
        M5.Display.setTextSize(2);
        M5.Display.setTextDatum(TL_DATUM);
        M5.Display.drawString("Answer:", MARGIN, curY);
        curY += M5.Display.fontHeight() + 4;
        M5.Display.setTextColor(TFT_BLACK);
        drawWrappedText(item.answer, MARGIN, curY, textAreaW, 4);
    }

    // Buttons (side by side)
    if (quizState == STATE_REVEALED) {
        drawButton(BTN_LEFT_X,  BTN_Y, BTN_W, BTN_H, "Quit");
        drawButton(BTN_RIGHT_X, BTN_Y, BTN_W, BTN_H, "Next", true);
    } else {
        bool hintActive = (quizState == STATE_HINT);
        drawButton(BTN_LEFT_X,  BTN_Y, BTN_W, BTN_H, "Hint", hintActive);
        drawButton(BTN_RIGHT_X, BTN_Y, BTN_W, BTN_H, "Reveal");
    }

    M5.Display.endWrite();
}

bool handleQuizTouch(int tx, int ty) {
    bool leftHit  = tx >= BTN_LEFT_X  && tx <= BTN_LEFT_X  + BTN_W && ty >= BTN_Y && ty <= BTN_Y + BTN_H;
    bool rightHit = tx >= BTN_RIGHT_X && tx <= BTN_RIGHT_X + BTN_W && ty >= BTN_Y && ty <= BTN_Y + BTN_H;

    needsFullRefresh = false;
    if (quizState == STATE_REVEALED) {
        if (leftHit) {
            // Quit → selection screen
            currentScreen = SCREEN_SELECT;
            needsFullRefresh = true;
            return true;
        }
        if (rightHit) {
            // Next question
            int size = CATEGORY_SIZES[selectedCategory];
            currentItem = (currentItem + 1) % size;
            quizState = STATE_QUESTION;
            needsFullRefresh = true;
            return true;
        }
    } else {
        if (leftHit) {
            quizState = (quizState == STATE_HINT) ? STATE_QUESTION : STATE_HINT;
            return true;
        }
        if (rightHit) {
            quizState = STATE_REVEALED;
            return true;
        }
    }
    return false;
}

// ─── Setup / Loop ────────────────────────────────────────────────────────────

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);

    M5.Display.setRotation(0);
    M5.Display.setEpdMode(epd_fast);
    M5.Display.clear();

    drawSelectionScreen();
}

void loop() {
    M5.update();

    auto touch = M5.Touch.getDetail();
    if (touch.wasPressed()) {
        int tx = touch.x;
        int ty = touch.y;

        bool changed = false;
        if (currentScreen == SCREEN_SELECT) {
            changed = handleSelectionTouch(tx, ty);
        } else {
            changed = handleQuizTouch(tx, ty);
        }

        if (changed) {
            if (currentScreen == SCREEN_SELECT) {
                // Full refresh when returning to selection (Quit) or entering quiz
                M5.Display.setEpdMode(epd_quality);
                M5.Display.clear();
                M5.Display.setEpdMode(epd_fast);
                drawSelectionScreen();
            } else {
                // Full refresh on Next; fast refresh for Hint/Reveal toggles
                if (needsFullRefresh) {
                    M5.Display.setEpdMode(epd_quality);
                    M5.Display.clear();
                    M5.Display.setEpdMode(epd_fast);
                }
                drawQuizScreen();
            }
        }
    }
}
