#include "raylib.h"
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>

using namespace std;

const int WORLD_WIDTH = 4000;
const int WORLD_HEIGHT = 720;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const float PLAYER_SPEED = 5.0f;

// Sprite / animation
const float PLAYER_WIDTH = 226.0f;
const float PLAYER_HEIGHT = 182.0f;

const int FRAME_COUNT = 5;
const int FRAME_WIDTH = 339;
const int FRAME_HEIGHT = 273;
const int FRAME_SPEED = 6;

const int RUN_FRAME_COUNT = 8;
const int RUN_FRAME_WIDTH = FRAME_WIDTH;
const int RUN_FRAME_HEIGHT = FRAME_HEIGHT;
const int RUN_FRAME_SPEED = 12;

const int JUMP_FRAME_COUNT = 11;
const int JUMP_FRAME_WIDTH = FRAME_WIDTH;
const int JUMP_FRAME_HEIGHT = FRAME_HEIGHT;
const float JUMP_DURATION = 1.0f;
const float JUMP_HEIGHT = (float)WORLD_HEIGHT / 2.0f;

// NPC sprites
const int NPC_FRAME_COUNT = 2;
const int NPC_FRAME_WIDTH = 316;
const int NPC_FRAME_HEIGHT = 362;

const int CATPOP_FRAME_COUNT = 2;
const int CATPOP_FRAME_WIDTH = 81;
const int CATPOP_FRAME_HEIGHT = 84;

const int CATCRUNCH_FRAME_COUNT = 24;
const int CATCRUNCH_FRAME_WIDTH = 113;
const int CATCRUNCH_FRAME_HEIGHT = 200;
const int CATCRUNCH_FRAME_SPEED = 12;

const int CATCRY_FRAME_COUNT = 60;
const int CATCRY_FRAME_WIDTH = 100;
const int CATCRY_FRAME_HEIGHT = 125;
const int CATCRY_FRAME_SPEED = 12;

struct Dialogue
{
    vector<string> lines;
};

struct NPC
{
    Rectangle bounds;
    Rectangle interactionArea;
    Dialogue dialogue;
    bool dialogueFinished;
    int spriteId; // 0=npc,1=cat-pop,2=cat-crunch,3=cat-cry

    Sound speechSound;
    bool hasSpeechSound;
};

const float INTERACTION_RADIUS = 150.0f;
const int TEXT_SPEED = 30;
const int TEXTBOX_HEIGHT = 200;
const int TEXT_FONT_SIZE = 36;
const int TEXT_PADDING = 20;

// Wrap text to fit maxWidth using provided font
string WordWrapText(const string& text, int maxWidth, const Font& font, int fontSize, float charSpacing)
{
    if (text.empty()) return "";

    string wrappedText;
    string currentLine;

    stringstream ss(text);
    string word;

    while (ss >> word)
    {
        string testLine = currentLine.empty() ? word : currentLine + " " + word;

        Vector2 size = MeasureTextEx(font, testLine.c_str(), (float)fontSize, charSpacing);
        if ((int)size.x > maxWidth)
        {
            if (!currentLine.empty())
            {
                wrappedText += currentLine + "\n";
                currentLine = word;
            }
            else
            {
                // single too-long word
                wrappedText += word + "\n";
                currentLine.clear();
            }
        }
        else
        {
            currentLine = testLine;
        }
    }

    if (!currentLine.empty())
    {
        wrappedText += currentLine;
    }

    return wrappedText;
}

// Draw text that may contain newlines
void DrawWrappedText(const Font& font, const string& text, float x, float y, int fontSize, float spacing, Color color)
{
    string line;
    istringstream stream(text);
    float cursorY = y;
    while (std::getline(stream, line))
    {
        DrawTextEx(font, line.c_str(), { x, cursorY }, (float)fontSize, spacing, color);
        cursorY += fontSize + spacing;
    }
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wpływ człowieka na hydrosferę");
    InitAudioDevice();

    // Assets
    Texture2D catWalkTexture = LoadTexture("catwalk.png");
    if (catWalkTexture.id == 0) cerr << "ERROR: Could not load texture 'catwalk.png'." << endl;

    Texture2D catRunTexture = LoadTexture("catrun.png");
    if (catRunTexture.id == 0) cerr << "ERROR: Could not load texture 'catrun.png'." << endl;

    Texture2D catJumpTexture = LoadTexture("catjump.png");
    if (catJumpTexture.id == 0) cerr << "ERROR: Could not load texture 'catjump.png'." << endl;

    Texture2D npcTexture = LoadTexture("npc.png");
    if (npcTexture.id == 0) cerr << "ERROR: Could not load texture 'npc.png'." << endl;

    Texture2D catPopTexture = LoadTexture("cat-pop.png");
    if (catPopTexture.id == 0) cerr << "ERROR: Could not load texture 'cat-pop.png'." << endl;

    Texture2D catCrunchTexture = LoadTexture("cat-crunch.png");
    if (catCrunchTexture.id == 0) cerr << "ERROR: Could not load texture 'cat-crunch.png'." << endl;

    Texture2D catCryTexture = LoadTexture("cat-cry.png");
    if (catCryTexture.id == 0) cerr << "ERROR: Could not load texture 'cat-cry.png'." << endl;

    // Load font
    int codepointsCount = 0;
    int* codepoints = LoadCodepoints(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ąćęłńóśźżĄĆĘŁŃÓŚŹŻ", &codepointsCount);
    Font uiFont = LoadFontEx("C:/Windows/Fonts/consola.ttf", TEXT_FONT_SIZE, codepoints, codepointsCount);
    UnloadCodepoints(codepoints);
    if (uiFont.texture.id == 0)
    {
        uiFont = GetFontDefault();
        cerr << "WARNING: Could not load system font. Falling back to default." << endl;
    }

    // Sounds
    Sound meow1Sound = { 0 };
    if (FileExists("meow1.wav")) meow1Sound = LoadSound("meow1.wav");
    else cerr << "WARNING: 'meow1.wav' not found." << endl;

    Sound meow2Sound = { 0 };
    if (FileExists("meow2.wav")) meow2Sound = LoadSound("meow2.wav");
    else cerr << "WARNING: 'meow2.wav' not found." << endl;

    Sound popSound = { 0 };
    if (FileExists("pop.wav")) popSound = LoadSound("pop.wav");
    else cerr << "WARNING: 'pop.wav' not found." << endl;

    Sound crunchSound = { 0 };
    if (FileExists("crunch.wav")) crunchSound = LoadSound("crunch.wav");
    else cerr << "WARNING: 'crunch.wav' not found." << endl;

    Sound jumpSound = { 0 };
    if (FileExists("jump.wav")) jumpSound = LoadSound("jump.wav");
    else cerr << "WARNING: 'jump.wav' not found." << endl;

    Sound sprintSound = { 0 };
    if (FileExists("sprint.wav")) sprintSound = LoadSound("sprint.wav");
    else cerr << "WARNING: 'sprint.wav' not found." << endl;

    // Background music (looping)
    Music bgMusic = { 0 };
    bool bgMusicLoaded = false;
    if (FileExists("BlindSpots.wav"))
    {
        bgMusic = LoadMusicStream("BlindSpots.wav");
        bgMusicLoaded = (bgMusic.ctxType != 0 || bgMusic.frameCount != 0); // best-effort check
        if (bgMusicLoaded)
        {
            PlayMusicStream(bgMusic);
            SetMusicVolume(bgMusic, 0.5f); // adjust background volume as desired
        }
        else
        {
            cerr << "WARNING: Failed to load 'BlindSpots.wav'." << endl;
        }
    }
    else
    {
        cerr << "WARNING: 'BlindSpots.wav' not found." << endl;
    }

    Rectangle player = {
        0.0f,
        (float)SCREEN_HEIGHT - PLAYER_HEIGHT - 10.0f,
        PLAYER_WIDTH,
        PLAYER_HEIGHT
    };

    const float PLAYER_GROUND_Y = player.y;

    Camera2D camera = { 0 };
    camera.target = { player.x + player.width / 2.0f, player.y + player.height / 2.0f };
    camera.offset = { (float)SCREEN_WIDTH / 2.0f, (float)SCREEN_HEIGHT / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    vector<NPC> npcs;
    Sound emptySound = { 0 };

    // Helper to create NPCs
    auto makeNpc = [&](float x, const vector<string>& lines, int spriteId = 0, Sound* speech = nullptr) -> NPC
        {
            float w = 64.0f;
            float h = 120.0f;
            float y = (float)SCREEN_HEIGHT - h - 20.0f;
            Rectangle bounds = { x, y, w, h };
            Rectangle interaction = { x + w / 2.0f - INTERACTION_RADIUS / 2.0f, y - 10.0f, INTERACTION_RADIUS, PLAYER_HEIGHT + 60.0f };
            Dialogue d; d.lines = lines;

            NPC npc;
            npc.bounds = bounds;
            npc.interactionArea = interaction;
            npc.dialogue = d;
            npc.dialogueFinished = false;
            npc.spriteId = spriteId;

            if (speech != nullptr)
            {
                npc.speechSound = *speech;
                npc.hasSpeechSound = true;
            }
            else
            {
                npc.speechSound = emptySound;
                npc.hasSpeechSound = false;
            }

            return npc;
        };

    // Create NPCs (Polish text)
    npcs.push_back(makeNpc(700.0f, {
        "Zróżnicowanie zasobów wody na świecie: jedne regiony mają dużo wody słodkiej, inne bardzo mało.",
        "Dostępność wody słodkiej zależy od klimatu, geologii i infrastruktury.",
        "Zrozumienie tego zróżnicowania jest kluczowe dla planowania i sprawiedliwego dostępu."
        }, 0, (meow1Sound.frameCount != 0 ? &meow1Sound : nullptr)));

    npcs.push_back(makeNpc(1400.0f, {
        "Niedobory wody dotykają miliardy ludzi. Przyczyny to wzrost populacji, zanieczyszczenia i zmiany klimatu.",
        "Susze i nadmierne pobory zasilają kryzysy wodne, szczególnie w krajach rozwijających się.",
        "Inwestycje w infrastrukturę, zarządzanie zasobami i edukacja są niezbędne, by łagodzić skutki."
        }, 1));

    npcs.push_back(makeNpc(2100.0f, {
        "Człowiek zagraża hydrosferze poprzez zanieczyszczenia, nadmierne pobory i degradację siedlisk.",
        "Plastiki, chemikalia i ścieki przemysłowe zmniejszają jakość wody i szkodzą organizmom.",
        "Ograniczanie emisji, regulacje i ochrona stref brzegowych to kluczowe działania."
        }, 3, (meow2Sound.frameCount != 0 ? &meow2Sound : nullptr)));

    npcs.push_back(makeNpc(2800.0f, {
        "Jezioro Aralskie to przykład katastrofy ekologicznej: odpływ rzek do nawadniania zmniejszył jego powierzchnię.",
        "Wysoka Tama na Nilu miała korzyści w hydroenergetyce, ale zmieniła sedymentację i lokalne ekosystemy.",
        "Studium tych przykładów uczy nas o konsekwencjach dużych projektów wodnych i konieczności zrównoważenia."
        }, 2));

    npcs.push_back(makeNpc(3400.0f, {
        "Jak chronić hydrosferę? Oszczędzanie wody, oczyszczanie ścieków i redukcja zanieczyszczeń są podstawowe.",
        "Inwestycje w odnawialne źródła, zrównoważone rolnictwo i ochrona terenów przybrzeżnych są kluczowe.",
        "Edukacja i współpraca międzynarodowa umożliwiają długotrwałe rozwiązania dla całej hydrosfery."
        }, 0, (meow1Sound.frameCount != 0 ? &meow1Sound : nullptr)));

    int frameCounter = 0;
    int currentFrame = 0;
    int runFrameCounter = 0;
    int currentRunFrame = 0;
    float frameDirection = 1.0f;
    bool isMoving = false;

    bool isJumping = false;
    float jumpTimer = 0.0f;
    int jumpFrame = 0;

    int catCrunchFrame = 0;
    float catCrunchTimer = 0.0f;

    int catCryFrame = 0;
    float catCryTimer = 0.0f;

    int activeNPC = -1;
    int currentDialogueLine = 0;
    string rawDialogueText = "";
    string wrappedDialogueText = "";
    int textDisplayLength = 0;
    int prevTextDisplayLength = 0;
    float textTimer = 0.0f;

    bool mouthOpen = false;
    float mouthTimer = 0.0f;
    float mouthToggleInterval = (TEXT_SPEED > 0) ? (2.0f / (float)TEXT_SPEED) : 1000000.0f;

    const int MAX_TEXT_WIDTH = (int)(SCREEN_WIDTH * 0.8f) - (2 * TEXT_PADDING);

    // Play per-character sound (skip whitespace)
    auto PlayDialogueCharSound = [&](int npcIndex, char ch)
        {
            if (npcIndex < 0 || npcIndex >= (int)npcs.size()) return;
            if (!npcs[npcIndex].hasSpeechSound) return;
            if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') return;

            PlaySound(npcs[npcIndex].speechSound);
        };

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        isMoving = false;

        // Update background music stream each frame (keeps it playing)
        if (bgMusicLoaded)
        {
            UpdateMusicStream(bgMusic);
            // Ensure it keeps playing (if it stopped for some reason)
            if (!IsMusicStreamPlaying(bgMusic)) PlayMusicStream(bgMusic);
        }

        // cat-crunch animation
        catCrunchTimer += dt;
        const float catCrunchFrameTime = 1.0f / (float)CATCRUNCH_FRAME_SPEED;
        if (catCrunchTimer >= catCrunchFrameTime)
        {
            catCrunchTimer -= catCrunchFrameTime;
            catCrunchFrame = (catCrunchFrame + 1) % CATCRUNCH_FRAME_COUNT;
        }

        // cat-cry animation
        catCryTimer += dt;
        const float catCryFrameTime = 1.0f / (float)CATCRY_FRAME_SPEED;
        if (catCryTimer >= catCryFrameTime)
        {
            catCryTimer -= catCryFrameTime;
            catCryFrame = (catCryFrame + 1) % CATCRY_FRAME_COUNT;
        }

        // Jump update
        if (isJumping)
        {
            jumpTimer += dt;
            int frameIndex = (int)floorf((jumpTimer / JUMP_DURATION) * (float)JUMP_FRAME_COUNT);
            if (frameIndex < 0) frameIndex = 0;
            if (frameIndex >= JUMP_FRAME_COUNT) frameIndex = JUMP_FRAME_COUNT - 1;
            jumpFrame = frameIndex;

            if (jumpTimer >= JUMP_DURATION)
            {
                isJumping = false;
                jumpTimer = 0.0f;
                jumpFrame = 0;
                player.y = PLAYER_GROUND_Y;
            }
            else
            {
                float t = jumpTimer;
                float T = JUMP_DURATION;
                float s = 4.0f * JUMP_HEIGHT * (t / T) * (1.0f - (t / T));
                player.y = PLAYER_GROUND_Y - s;
                if (player.y < 0.0f) player.y = 0.0f;
            }
        }

        float speedMultiplier = 1.0f;
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
            speedMultiplier = 3.0f;
        }

        // Movement (disabled during dialogue)
        if (activeNPC == -1)
        {
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
            {
                player.x += PLAYER_SPEED * speedMultiplier;
                frameDirection = 1.0f;
                isMoving = true;
            }
            else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
            {
                player.x -= PLAYER_SPEED * speedMultiplier;
                frameDirection = -1.0f;
                isMoving = true;
            }

            if ((IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) && sprintSound.frameCount != 0) 
            {
                PlaySound(sprintSound);
            }
            
            if (!isJumping && (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)))
            {
                isJumping = true;
                jumpTimer = 0.0f;
                jumpFrame = 0;

                // Play jump sound once at jump start
                if (jumpSound.frameCount != 0) PlaySound(jumpSound);
            }
        }

        if (player.x < 0) player.x = 0;
        if (player.x + player.width > WORLD_WIDTH) player.x = WORLD_WIDTH - player.width;

        // Walking / Running animation
        if (!isJumping)
        {
            if (isMoving)
            {
                if (speedMultiplier > 1.0f)
                {
                    // Running animation
                    runFrameCounter++;
                    float effectiveRunSpeed = RUN_FRAME_SPEED;
                    int runFrameDelay = (int)round(60.0f / effectiveRunSpeed);
                    if (runFrameDelay < 1) runFrameDelay = 1;
                    if (runFrameCounter >= runFrameDelay)
                    {
                        runFrameCounter = 0;
                        currentRunFrame++;
                        if (currentRunFrame >= RUN_FRAME_COUNT) currentRunFrame = 0;
                    }

                    // reset walk counters to keep state consistent
                    currentFrame = 0;
                    frameCounter = 0;
                }
                else
                {
                    // Walking animation
                    frameCounter++;
                    float effectiveFrameSpeed = FRAME_SPEED;
                    int frameDelay = (int)round(60.0f / effectiveFrameSpeed);
                    if (frameDelay < 1) frameDelay = 1;
                    if (frameCounter >= frameDelay)
                    {
                        frameCounter = 0;
                        currentFrame++;
                        if (currentFrame >= FRAME_COUNT) currentFrame = 0;
                    }

                    // reset run counters to keep state consistent
                    currentRunFrame = 0;
                    runFrameCounter = 0;
                }
            }
            else
            {
                currentFrame = 0;
                frameCounter = 0;
                currentRunFrame = 0;
                runFrameCounter = 0;
            }
        }

        // Check nearby NPC
        int foundNear = -1;
        for (int i = 0; i < (int)npcs.size(); ++i)
        {
            if (CheckCollisionRecs(player, npcs[i].interactionArea))
            {
                foundNear = i;
                break;
            }
        }

        bool enterConsumedForStart = false;

        // Start dialogue
        if (foundNear != -1 && activeNPC == -1 && IsKeyPressed(KEY_ENTER))
        {
            activeNPC = foundNear;
            currentDialogueLine = 0;
            rawDialogueText = npcs[activeNPC].dialogue.lines[currentDialogueLine];
            wrappedDialogueText = WordWrapText(rawDialogueText, MAX_TEXT_WIDTH, uiFont, TEXT_FONT_SIZE, 4.0f);
            textDisplayLength = 0;
            prevTextDisplayLength = 0;
            textTimer = 0.0f;
            mouthOpen = false;
            mouthTimer = 0.0f;
            enterConsumedForStart = true;

            int sid = npcs[activeNPC].spriteId;
            if (sid == 1 && popSound.frameCount != 0) PlaySound(popSound);
            if (sid == 2 && crunchSound.frameCount != 0) PlaySound(crunchSound);
        }

        // Leave dialogue if player exits area
        if (foundNear == -1 && activeNPC != -1)
        {
            int sid = npcs[activeNPC].spriteId;
            if (sid == 1 && popSound.frameCount != 0) StopSound(popSound);
            if (sid == 2 && crunchSound.frameCount != 0) StopSound(crunchSound);

            activeNPC = -1;
            rawDialogueText.clear();
            wrappedDialogueText.clear();
            textDisplayLength = 0;
            prevTextDisplayLength = 0;
            textTimer = 0.0f;
            mouthOpen = false;
            mouthTimer = 0.0f;
        }

        // Advance/skip dialogue
        if (activeNPC != -1 && IsKeyPressed(KEY_ENTER) && !enterConsumedForStart)
        {
            int sid = npcs[activeNPC].spriteId;

            if (textDisplayLength < (int)wrappedDialogueText.length())
            {
                prevTextDisplayLength = textDisplayLength;
                textDisplayLength = (int)wrappedDialogueText.length();

                for (int k = prevTextDisplayLength; k < textDisplayLength; ++k)
                {
                    PlayDialogueCharSound(activeNPC, wrappedDialogueText[k]);
                }

                if (sid == 1 && popSound.frameCount != 0) StopSound(popSound);
            }
            else
            {
                currentDialogueLine++;
                if (currentDialogueLine < (int)npcs[activeNPC].dialogue.lines.size())
                {
                    rawDialogueText = npcs[activeNPC].dialogue.lines[currentDialogueLine];
                    wrappedDialogueText = WordWrapText(rawDialogueText, MAX_TEXT_WIDTH, uiFont, TEXT_FONT_SIZE, 4.0f);
                    textDisplayLength = 0;
                    prevTextDisplayLength = 0;
                    textTimer = 0.0f;
                    mouthOpen = false;
                    mouthTimer = 0.0f;

                    if (sid == 1 && popSound.frameCount != 0 && !IsSoundPlaying(popSound)) PlaySound(popSound);
                }
                else
                {
                    if (sid == 1 && popSound.frameCount != 0) StopSound(popSound);
                    if (sid == 2 && crunchSound.frameCount != 0) StopSound(crunchSound);

                    activeNPC = -1;
                    rawDialogueText.clear();
                    wrappedDialogueText.clear();
                    textDisplayLength = 0;
                    prevTextDisplayLength = 0;
                    textTimer = 0.0f;
                    mouthOpen = false;
                    mouthTimer = 0.0f;
                }
            }
        }

        // Reveal text over time
        if (activeNPC != -1 && textDisplayLength < (int)wrappedDialogueText.length())
        {
            textTimer += dt;
            int newLength = min((int)(textTimer * TEXT_SPEED), (int)wrappedDialogueText.length());

            if (newLength > textDisplayLength)
            {
                for (int k = textDisplayLength; k < newLength; ++k)
                {
                    PlayDialogueCharSound(activeNPC, wrappedDialogueText[k]);
                }
                prevTextDisplayLength = textDisplayLength;
                textDisplayLength = newLength;

                int sid = npcs[activeNPC].spriteId;
                if (sid == 1 && popSound.frameCount != 0)
                {
                    if (!IsSoundPlaying(popSound)) PlaySound(popSound);
                    if (textDisplayLength >= (int)wrappedDialogueText.length()) StopSound(popSound);
                }
            }
        }

        // Ensure crunch loops during conversation
        if (activeNPC != -1 && npcs[activeNPC].spriteId == 2 && crunchSound.frameCount != 0)
        {
            if (!IsSoundPlaying(crunchSound)) PlaySound(crunchSound);
        }

        // Mouth animation while text reveals
        if (activeNPC != -1 && textDisplayLength < (int)wrappedDialogueText.length())
        {
            mouthTimer += dt;
            if (mouthTimer >= mouthToggleInterval)
            {
                mouthOpen = !mouthOpen;
                mouthTimer = 0.0f;
            }
        }
        else
        {
            mouthOpen = false;
            mouthTimer = 0.0f;
        }

        // Camera follow
        camera.target = { player.x + player.width / 2.0f, player.y + player.height / 2.0f };
        camera.target.x = fmax(SCREEN_WIDTH / 2.0f, camera.target.x);
        camera.target.x = fmin(WORLD_WIDTH - SCREEN_WIDTH / 2.0f, camera.target.x);
        camera.target.y = (float)SCREEN_HEIGHT / 2.0f;

        // DRAW
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode2D(camera);

        DrawRectangle(0, 0, WORLD_WIDTH, SCREEN_HEIGHT - 20, SKYBLUE);
        DrawRectangle(0, SCREEN_HEIGHT - 20, WORLD_WIDTH, 20, DARKGREEN);
        DrawLine(0, SCREEN_HEIGHT - 20, WORLD_WIDTH, SCREEN_HEIGHT - 20, BROWN);

        // Draw NPCs
        for (int i = 0; i < (int)npcs.size(); ++i)
        {
            NPC& npc = npcs[i];
            bool isCurrentlyNear = (i == foundNear);

            Color zoneColor = isCurrentlyNear ? (npc.dialogueFinished ? DARKGRAY : RED) : YELLOW;
            DrawRectangleLinesEx(npc.interactionArea, 2, zoneColor);

            Texture2D const* texPtr = nullptr;
            int frameWidth = 0;
            int frameHeight = 0;
            int frameCount = 0;
            int frameIndex = 0;

            if (npc.spriteId == 2 && catCrunchTexture.id != 0)
            {
                texPtr = &catCrunchTexture;
                frameWidth = CATCRUNCH_FRAME_WIDTH;
                frameHeight = CATCRUNCH_FRAME_HEIGHT;
                frameCount = CATCRUNCH_FRAME_COUNT;
                frameIndex = catCrunchFrame;
                if (frameIndex >= frameCount) frameIndex = frameCount - 1;
            }
            else if (npc.spriteId == 3 && catCryTexture.id != 0)
            {
                texPtr = &catCryTexture;
                frameWidth = CATCRY_FRAME_WIDTH;
                frameHeight = CATCRY_FRAME_HEIGHT;
                frameCount = CATCRY_FRAME_COUNT;
                frameIndex = catCryFrame;
                if (frameIndex >= frameCount) frameIndex = frameCount - 1;
            }
            else if (npc.spriteId == 1 && catPopTexture.id != 0)
            {
                texPtr = &catPopTexture;
                frameWidth = CATPOP_FRAME_WIDTH;
                frameHeight = CATPOP_FRAME_HEIGHT;
                frameCount = CATPOP_FRAME_COUNT;
                frameIndex = (i == activeNPC && mouthOpen) ? 1 : 0;
                if (frameIndex >= frameCount) frameIndex = frameCount - 1;
            }
            else if (npcTexture.id != 0)
            {
                texPtr = &npcTexture;
                frameWidth = NPC_FRAME_WIDTH;
                frameHeight = NPC_FRAME_HEIGHT;
                frameCount = NPC_FRAME_COUNT;
                frameIndex = (i == activeNPC && mouthOpen) ? 1 : 0;
                if (frameIndex >= frameCount) frameIndex = frameCount - 1;
            }

            if (texPtr != nullptr && texPtr->id != 0)
            {
                float targetHeight = npc.bounds.height * 2.2f;
                float scale = targetHeight / (float)frameHeight;
                float renderW = frameWidth * scale;
                float renderH = frameHeight * scale;

                float destX = npc.bounds.x + npc.bounds.width / 2.0f - renderW / 2.0f;
                float destY = npc.bounds.y + npc.bounds.height - renderH + 18.0f;

                Rectangle srcRec = { (float)(frameIndex * frameWidth), 0.0f, (float)frameWidth, (float)frameHeight };
                Rectangle destRec = { destX, destY, renderW, renderH };

                DrawTexturePro(*texPtr, srcRec, destRec, { 0, 0 }, 0.0f, WHITE);
            }
            else
            {
                DrawRectangleRec(npc.bounds, BLUE);
                DrawTextEx(uiFont, "NPC", { npc.bounds.x + 5, npc.bounds.y - 20 }, 20.0f, 1.0f, BLUE);
            }
        }

        // Player
        if (isJumping && catJumpTexture.id != 0)
        {
            Rectangle sourceRec = {
                (float)jumpFrame * (float)JUMP_FRAME_WIDTH,
                0.0f,
                (float)JUMP_FRAME_WIDTH * frameDirection,
                (float)JUMP_FRAME_HEIGHT
            };
            Rectangle destRec = {
                player.x,
                player.y,
                player.width,
                player.height
            };
            DrawTexturePro(catJumpTexture, sourceRec, destRec, { 0, 0 }, 0.0f, WHITE);
        }
        else if (speedMultiplier > 1.0f && isMoving && catRunTexture.id != 0)
        {
            // Running animation draw
            Rectangle sourceRec = {
                (float)currentRunFrame * (float)RUN_FRAME_WIDTH,
                0.0f,
                (float)RUN_FRAME_WIDTH * frameDirection,
                (float)RUN_FRAME_HEIGHT
            };
            Rectangle destRec = {
                player.x,
                player.y,
                player.width,
                player.height
            };
            DrawTexturePro(catRunTexture, sourceRec, destRec, { 0, 0 }, 0.0f, WHITE);
        }
        else
        {
            Rectangle sourceRec = {
                (float)currentFrame * FRAME_WIDTH,
                0.0f,
                FRAME_WIDTH * frameDirection,
                FRAME_HEIGHT
            };
            Rectangle destRec = {
                player.x,
                player.y,
                player.width,
                player.height
            };
            DrawTexturePro(catWalkTexture, sourceRec, destRec, { 0, 0 }, 0.0f, WHITE);
        }

        EndMode2D();

        // Dialogue box
        if (activeNPC != -1)
        {
            Rectangle dialogueBoxRec = {
                (float)SCREEN_WIDTH * 0.1f,
                10.0f,
                (float)SCREEN_WIDTH * 0.8f,
                (float)TEXTBOX_HEIGHT
            };
            DrawRectangleRec(dialogueBoxRec, CLITERAL(Color){ 20, 20, 20, 220 });
            DrawRectangleLinesEx(dialogueBoxRec, 5, WHITE);

            string visibleText = wrappedDialogueText.substr(0, textDisplayLength);
            DrawWrappedText(uiFont, visibleText, dialogueBoxRec.x + TEXT_PADDING, dialogueBoxRec.y + TEXT_PADDING, TEXT_FONT_SIZE, 4.0f, WHITE);
        }

        DrawTextEx(uiFont, TextFormat("Player X: %.2f", player.x), { 10.0f, 10.0f }, 20.0f, 1.0f, DARKGRAY);
        DrawTextEx(uiFont, TextFormat("Active NPC: %s", activeNPC == -1 ? "NONE" : "YES"), { 10.0f, 40.0f }, 20.0f, 1.0f, DARKGRAY);
        DrawTextEx(uiFont, "Use ARROWS or A/D to move. SPACE/W/Up to jump. Press [Enter] to interact.", { 10.0f, 70.0f }, 18.0f, 1.0f, DARKGRAY);

        EndDrawing();
    }

    // Cleanup
    UnloadTexture(catWalkTexture);
    UnloadTexture(catRunTexture);
    UnloadTexture(npcTexture);
    UnloadTexture(catPopTexture);
    UnloadTexture(catCrunchTexture);
    UnloadTexture(catCryTexture);
    UnloadTexture(catJumpTexture);
    UnloadFont(uiFont);

    if (meow1Sound.frameCount != 0) UnloadSound(meow1Sound);
    if (meow2Sound.frameCount != 0) UnloadSound(meow2Sound);
    if (popSound.frameCount != 0) UnloadSound(popSound);
    if (crunchSound.frameCount != 0) UnloadSound(crunchSound);
    if (jumpSound.frameCount != 0) UnloadSound(jumpSound);
    if (sprintSound.frameCount != 0) UnloadSound(sprintSound);

    // Stop and unload background music
    if (bgMusicLoaded)
    {
        StopMusicStream(bgMusic);
        UnloadMusicStream(bgMusic);
    }

    CloseAudioDevice();

    CloseWindow();
    return 0;
}