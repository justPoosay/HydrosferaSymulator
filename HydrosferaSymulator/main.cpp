#include "raylib.h"
#include <iostream>
#include <string>
#include <vector>
#include <cmath> 
#include <algorithm> 
#include <sstream>

using namespace std;

// Define the dimensions of your world/level
const int WORLD_WIDTH = 4000;
const int WORLD_HEIGHT = 720;

// Define the screen dimensions
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// Player characteristics
const float PLAYER_SPEED = 5.0f;

// --- SPRITE AND ANIMATION CONSTANTS ---
// MODIFIED: Player sprite is now 2x larger (70 * 2 = 140)
const float PLAYER_WIDTH = 60.0f;
const float PLAYER_HEIGHT = 90.0f;

// UPDATED: Adjusting frame count to 6 based on the stickman.png image uploaded.
const int FRAME_COUNT = 2;
const int FRAME_WIDTH = 20;
const int FRAME_HEIGHT = 30;
const int FRAME_SPEED = 4;

// --- NPC DEFINITIONS ---
struct NPC
{
    Rectangle bounds;
    Rectangle interactionArea;
    vector<string> dialogueLines;
    bool dialogueFinished; // Flag to track if the current conversation is complete
};

// --- DIALOGUE SYSTEM CONSTANTS ---
const float INTERACTION_RADIUS = 150.0f;
const int TEXT_SPEED = 15; // Characters per second
const int TEXTBOX_HEIGHT = 150;
const int TEXT_FONT_SIZE = 30; // Font size for dialogue text
const int TEXT_PADDING = 20; // Padding inside the dialogue box

// --- TEXT WRAPPING HELPER FUNCTION ---
// Takes raw text and inserts newline characters (\n) so it fits within maxWidth.
string WordWrapText(const string& text, int maxWidth, int fontSize)
{
    if (text.empty()) return "";

    string wrappedText;
    string currentLine;

    // Use an istringstream to split the text into words
    stringstream ss(text);
    string word;

    while (ss >> word)
    {
        // Temporarily form the next potential line (current line + space + next word)
        string testLine = currentLine.empty() ? word : currentLine + " " + word;

        // Measure the width of the test line
        int testWidth = MeasureText(testLine.c_str(), fontSize);

        if (testWidth > maxWidth)
        {
            if (!currentLine.empty())
            {
                // Current line is full, wrap it
                wrappedText += currentLine + "\n";
                currentLine = word; // Start the new line with the current word
            }
            else
            {
                // Single word is too long (rare for dialogue, but handle it)
                wrappedText += word + "\n";
                currentLine = "";
            }
        }
        else
        {
            // Line still fits, update the current line
            currentLine = testLine;
        }
    }

    // Add the remaining line
    if (!currentLine.empty())
    {
        wrappedText += currentLine;
    }

    return wrappedText;
}


int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Interactive Presentation - Word Wrap Dialogue Demo");

    // --- TEXTURE LOADING (Player) ---
    Texture2D stickmanTexture = LoadTexture("frisk.png");

    if (stickmanTexture.id == 0)
    {
        cerr << "ERROR: Could not load texture 'stickman.png'." << endl;
    }

    // Player properties
    Rectangle player = {
        // Player position adjusted for new size to keep feet at the same ground level
        (float)SCREEN_WIDTH / 2.0f - PLAYER_WIDTH / 2.0f,
        (float)SCREEN_HEIGHT - PLAYER_HEIGHT - 10.0f,
        PLAYER_WIDTH,
        PLAYER_HEIGHT
    };

    // Camera structure for scrolling
    Camera2D camera = { 0 };
    camera.target = { player.x + player.width / 2.0f, player.y + player.height / 2.0f };
    camera.offset = { (float)SCREEN_WIDTH / 2.0f, (float)SCREEN_HEIGHT / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // --- NPC SETUP ---
    NPC friendNPC = {
        // Bounds (NPC is 40x80, placed at x=1000)
        { 1000.0f, (float)SCREEN_HEIGHT - 80.0f - 20.0f, 40.0f, 80.0f },
        // Interaction Area 
        { 1000.0f + 40.0f / 2.0f - INTERACTION_RADIUS / 2.0f, (float)SCREEN_HEIGHT - 80.0f - 20.0f - 10.0f, INTERACTION_RADIUS, PLAYER_HEIGHT + 20.0f },
        // Dialogue (Multiple lines in a vector, using a long line to test wrapping)
        {
            "Hello world! This first line is short.",
            "The text wrapping feature is now active! This is a much longer sentence that should definitely be broken down into two or maybe even three lines depending on the screen size and the font settings you chose.",
            "This final short line proves the system still works for normal sentences.",
        },
        false // dialogueFinished starts as false
    };

    // --- ANIMATION STATE VARIABLES ---
    int frameCounter = 0;
    int currentFrame = 0;
    float frameDirection = 1.0f;
    bool isMoving = false;

    // --- DIALOGUE STATE VARIABLES ---
    bool isNearNPC = false;
    int currentDialogueLine = 0;
    string rawDialogueText = "";           // The original text of the current line
    string wrappedDialogueText = "";       // The wrapped version with '\n'
    int textDisplayLength = 0;
    float textTimer = 0.0f;

    // Calculate maximum width for the text within the dialogue box
    const int MAX_TEXT_WIDTH = (int)(SCREEN_WIDTH * 0.8f) - (2 * TEXT_PADDING);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        float dt = GetFrameTime();

        isMoving = false;

        // 1. Player Movement Input
        if (!isNearNPC)
        {
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
            {
                player.x += PLAYER_SPEED;
                frameDirection = 1.0f;
                isMoving = true;
            }
            else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
            {
                player.x -= PLAYER_SPEED;
                frameDirection = -1.0f;
                isMoving = true;
            }
        }

        // 2. Player World Boundary Check
        if (player.x < 0) player.x = 0;
        if (player.x + player.width > WORLD_WIDTH) player.x = WORLD_WIDTH - player.width;


        // 3. Animation Logic Update 
        if (isMoving)
        {
            frameCounter++;
            // Note: FRAME_COUNT is 6 now
            if (frameCounter >= (60 / FRAME_SPEED))
            {
                frameCounter = 0;
                currentFrame++;
                if (currentFrame >= FRAME_COUNT)
                {
                    currentFrame = 0;
                }
            }
        }
        else
        {
            currentFrame = 0;
            frameCounter = 0;
        }


        // 4. Interaction and Dialogue Logic

        bool currentlyNear = CheckCollisionRecs(player, friendNPC.interactionArea);

        if (currentlyNear)
        {
            if (!isNearNPC && !friendNPC.dialogueFinished)
            {
                // START conversation
                isNearNPC = true;
                currentDialogueLine = 0;
                rawDialogueText = friendNPC.dialogueLines[currentDialogueLine];

                // NEW: Apply word wrapping when starting a new line
                wrappedDialogueText = WordWrapText(rawDialogueText, MAX_TEXT_WIDTH, TEXT_FONT_SIZE);

                textDisplayLength = 0;
                textTimer = 0.0f;
            }
        }
        else
        {
            if (friendNPC.dialogueFinished)
            {
                // Player left after finishing: Reset flag
                friendNPC.dialogueFinished = false;
            }
            if (isNearNPC)
            {
                // Player left while dialogue was active: Dismiss
                isNearNPC = false;
                rawDialogueText = "";
                wrappedDialogueText = "";
            }
        }

        // --- Input Handling (KEY_ENTER) ---
        if (isNearNPC && IsKeyPressed(KEY_ENTER))
        {
            // We compare against the length of the WRAPPED text now!
            if (textDisplayLength < wrappedDialogueText.length())
            {
                // Text is animating: instantly show the full text
                textDisplayLength = wrappedDialogueText.length();
            }
            else
            {
                // Text is fully displayed: advance the line
                currentDialogueLine++;

                if (currentDialogueLine < friendNPC.dialogueLines.size())
                {
                    // More lines exist: load next line, wrap, and restart animation
                    rawDialogueText = friendNPC.dialogueLines[currentDialogueLine];
                    wrappedDialogueText = WordWrapText(rawDialogueText, MAX_TEXT_WIDTH, TEXT_FONT_SIZE);
                    textDisplayLength = 0;
                    textTimer = 0.0f;
                }
                else
                {
                    // End of conversation: Set flag and dismiss dialogue
                    friendNPC.dialogueFinished = true;
                    isNearNPC = false;
                    rawDialogueText = "";
                    wrappedDialogueText = "";
                    currentDialogueLine = 0;
                }
            }
        }


        // Animated Text Update
        // Compare against the length of the WRAPPED text
        if (isNearNPC && textDisplayLength < wrappedDialogueText.length())
        {
            textTimer += dt;
            textDisplayLength = min((int)(textTimer * TEXT_SPEED), (int)wrappedDialogueText.length());
        }


        // 5. Camera (Scrolling) Update (same as before)
        camera.target = { player.x + player.width / 2.0f, player.y + player.height / 2.0f };
        camera.target.x = fmax(SCREEN_WIDTH / 2.0f, camera.target.x);
        camera.target.x = fmin(WORLD_WIDTH - SCREEN_WIDTH / 2.0f, camera.target.x);
        camera.target.y = (float)SCREEN_HEIGHT / 2.0f;

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode2D(camera);

        // --- Temporary Scrolling World Background --- 
        DrawLine(0, SCREEN_HEIGHT - 20, WORLD_WIDTH, SCREEN_HEIGHT - 20, BROWN);
        DrawRectangle(0, 0, WORLD_WIDTH, SCREEN_HEIGHT - 20, SKYBLUE);
        DrawRectangle(0, SCREEN_HEIGHT - 20, WORLD_WIDTH, 20, DARKGREEN);

        // DEBUG: Draw the NPC Interaction Zone
        Color zoneColor = currentlyNear ? (friendNPC.dialogueFinished ? DARKGRAY : RED) : YELLOW;
        DrawRectangleLinesEx(friendNPC.interactionArea, 2, zoneColor);

        // --- NPC DRAWING ---
        DrawRectangleRec(friendNPC.bounds, BLUE);
        DrawText("NPC", (int)friendNPC.bounds.x + 5, (int)friendNPC.bounds.y - 20, 20, BLUE);

        // --- Animated Player Sprite ---
        // Source rectangle remains the size of one frame (70x70)
        Rectangle sourceRec = {
            (float)currentFrame * FRAME_WIDTH,
            0.0f,
            FRAME_WIDTH * frameDirection,
            FRAME_HEIGHT
        };

        // Destination rectangle is the player's bounding box (now 140x140)
        Rectangle destRec = {
            player.x,
            player.y,
            player.width,
            player.height
        };

        // Use DrawTexturePro to draw the source frame into the larger destination rectangle
        DrawTexturePro(stickmanTexture, sourceRec, destRec, { 0, 0 }, 0.0f, WHITE);


        EndMode2D();

        // --- DIALOGUE BOX (Drawn outside camera view, fixed to screen) ---
        if (isNearNPC)
        {
            // Box background
            Rectangle dialogueBoxRec = {
                (float)SCREEN_WIDTH * 0.1f,
                (float)SCREEN_HEIGHT - TEXTBOX_HEIGHT - 10.0f,
                (float)SCREEN_WIDTH * 0.8f,
                (float)TEXTBOX_HEIGHT
            };
            DrawRectangleRec(dialogueBoxRec, CLITERAL(Color){ 20, 20, 20, 220 }); // Dark gray, slight transparency
            DrawRectangleLinesEx(dialogueBoxRec, 5, WHITE);

            // Animated Text
            // Use the wrapped text for display
            string visibleText = wrappedDialogueText.substr(0, textDisplayLength);
            DrawText(visibleText.c_str(), (int)dialogueBoxRec.x + TEXT_PADDING, (int)dialogueBoxRec.y + TEXT_PADDING, TEXT_FONT_SIZE, WHITE);

            // Prompt when text is complete (I've commented out the original prompt logic, but keeping the comment block for completeness)
            //if (textDisplayLength == wrappedDialogueText.length())
            //{
            //    const char* prompt = (currentDialogueLine < friendNPC.dialogueLines.size() - 1)
            //        ? "Press [Enter] for next line..."
            //        : "Press [Enter] to close.";

            //    DrawText(prompt, (int)dialogueBoxRec.x + TEXT_PADDING, (int)dialogueBoxRec.y + TEXTBOX_HEIGHT - 40, 20, GRAY);
            //}
            //else
            //{
            //    DrawText("Press [Enter] to skip...", (int)dialogueBoxRec.x + TEXT_PADDING, (int)dialogueBoxRec.y + TEXTBOX_HEIGHT - 40, 20, GRAY);
            //}
        }

        // --- UI Elements ---
        DrawText(TextFormat("Player X: %.2f", player.x), 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("Near NPC: %s | Finished: %s", currentlyNear ? "YES" : "NO", friendNPC.dialogueFinished ? "YES" : "NO"), 10, 40, 20, currentlyNear ? RED : DARKGRAY);
        DrawText("Use ARROWS or A/D to move", 10, 70, 20, DARKGRAY);


        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(stickmanTexture);
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}