#include "raylib.h"
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>

using namespace std;

const int WORLD_WIDTH = 6400;
const int WORLD_HEIGHT = 720;
const int SECRET_ROOM_WIDTH = 1280;
const float SECRET_X_OFFSET = -(float)SECRET_ROOM_WIDTH;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const float PLAYER_SPEED = 5.0f;

// Sprite / animation
const float PLAYER_WIDTH = 226.0f;
const float PLAYER_HEIGHT = 182.0f;

const int FRAME_COUNT = 16;
const int FRAME_WIDTH = 539;
const int FRAME_HEIGHT = 439;
const int FRAME_SPEED = 10;

const int RUN_FRAME_COUNT = 8;
const int RUN_FRAME_SPEED = 12;

const int JUMP_FRAME_COUNT = 11;
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

const int CATSPINNING_FRAME_COUNT = 24;
const int CATSPINNING_FRAME_WIDTH = 200;
const int CATSPINNING_FRAME_HEIGHT = 134;
const int CATSPINNING_FRAME_SPEED = 24;

struct NPC
{
    Rectangle bounds;
    Rectangle interactionArea;
    vector<string> lines;
    int spriteId; // 0=npc,1=cat-pop,2=cat-crunch,3=cat-cry
    Sound* speech; // pointer to loaded Sound or nullptr
    bool hasSpeech;
};

struct NPCDefinition
{
    float x;
    vector<string> lines;
    int spriteId = 0;
    Sound* speech = nullptr;
};

const float INTERACTION_RADIUS = 200.0f;
const int TEXT_SPEED = 30;
const int TEXTBOX_HEIGHT = 200;
const int TEXT_FONT_SIZE = 36;
const int TEXT_PADDING = 20;

// Ground settings
const int GROUND_HEIGHT = 64;
const int GRASS_TILE_SIZE = 64;

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
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wpływ człowieka na hydrosferę");
    InitAudioDevice();

    // Assets
    Texture2D catWalkTexture = LoadTexture("assets/player/walk.png");
    if (catWalkTexture.id == 0) cerr << "ERROR: Could not load texture 'walk.png'." << endl;

    Texture2D catRunTexture = LoadTexture("assets/player/run.png");
    if (catRunTexture.id == 0) cerr << "ERROR: Could not load texture 'run.png'." << endl;

    Texture2D catJumpTexture = LoadTexture("assets/player/jump.png");
    if (catJumpTexture.id == 0) cerr << "ERROR: Could not load texture 'jump.png'." << endl;
    
    Texture2D happyTexture = LoadTexture("assets/player/happy.png");
    if (happyTexture.id == 0) cerr << "WARNING: Could not load texture 'happy.png'." << endl;

    Texture2D npcTexture = LoadTexture("assets/npc/gatito.png");
    if (npcTexture.id == 0) cerr << "ERROR: Could not load texture 'gatito.png'." << endl;

    Texture2D catPopTexture = LoadTexture("assets/npc/catPop.png");
    if (catPopTexture.id == 0) cerr << "ERROR: Could not load texture 'catPop.png'." << endl;

    Texture2D catCrunchTexture = LoadTexture("assets/npc/catCrunch.png");
    if (catCrunchTexture.id == 0) cerr << "ERROR: Could not load texture 'catCrunch.png'." << endl;

    Texture2D catCryTexture = LoadTexture("assets/npc/catCry.png");
    if (catCryTexture.id == 0) cerr << "ERROR: Could not load texture 'catCry.png'." << endl;

    Texture2D catSpinningTexture = LoadTexture("assets/npc/catSpinning.png");
    if (catSpinningTexture.id == 0) cerr << "ERROR: Could not load texture 'catSpinning.png'." << endl;

    Texture2D grassTexture = LoadTexture("assets/level/grass.png");
    if (grassTexture.id == 0) cerr << "WARNING: Could not load texture 'grass.png'." << endl;

    Texture2D finishTexture = LoadTexture("assets/level/finish.png");
    if (finishTexture.id == 0) cerr << "WARNING: Could not load texture 'finish.png'." << endl;

    Texture2D congratsTexture = LoadTexture("assets/level/congratulation.png");
    if (congratsTexture.id == 0) cerr << "WARNING: Could not load texture 'congratulation.png'." << endl;

    // Biome background textures
    const int SEG_W = 1280;
    const int SEG_COUNT = 5;
    Texture2D biomeTextures[SEG_COUNT];
    for (int i = 0; i < SEG_COUNT; ++i)
    {
        string filename = "assets/level/biome" + to_string(i + 1) + ".png";
        biomeTextures[i] = LoadTexture(filename.c_str());
        if (biomeTextures[i].id == 0)
        {
            cerr << "WARNING: Could not load texture biome'" << to_string(i + 1) << ".png'." << endl;
        }
    }

    // Background crossfade state
    int displayedBiome = -1;
    int fadingFrom = -1;
    int fadingTo = -1;
    float fadeTimer = 0.0f;
    const float FADE_DURATION = 0.6f;

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
    if (FileExists("assets/sound/meow1.wav"))
    {
        meow1Sound = LoadSound("assets/sound/meow1.wav");
    }
    else
    {
        cerr << "WARNING: 'meow1.wav' not found." << endl;
    }

    Sound meow2Sound = { 0 };
    if (FileExists("assets/sound/meow2.wav"))
    {
        meow2Sound = LoadSound("assets/sound/meow2.wav");
    }
    else
    {
        cerr << "WARNING: 'meow2.wav' not found." << endl;
    }

    Sound popSound = { 0 };
    if (FileExists("assets/sound/pop.wav"))
    {
        popSound = LoadSound("assets/sound/pop.wav");
    }
    else
    {
        cerr << "WARNING: 'pop.wav' not found." << endl;
    }

    Sound vanishSound = { 0 };
    if (FileExists("assets/sound/vanish.wav"))
    {
        vanishSound = LoadSound("assets/sound/vanish.wav");
    }
    else
    {
        cerr << "WARNING: 'vanish.wav' not found." << endl;
    }

    Sound crunchSound = { 0 };
    if (FileExists("assets/sound/crunch.wav"))
    {
        crunchSound = LoadSound("assets/sound/crunch.wav");
    }
    else
    {
        cerr << "WARNING: 'crunch.wav' not found." << endl;
    }

    Sound jumpSound = { 0 };
    if (FileExists("assets/sound/jump.wav"))
    {
        jumpSound = LoadSound("assets/sound/jump.wav");
    }
    else
    {
        cerr << "WARNING: 'jump.wav' not found." << endl;
    }

    Sound sprintSound = { 0 };
    if (FileExists("assets/sound/sprint.wav"))
    {
        sprintSound = LoadSound("assets/sound/sprint.wav");
    }
    else
    {
        cerr << "WARNING: 'sprint.wav' not found." << endl;
    }

    Sound cheerSound = { 0 };
    if (FileExists("assets/sound/cheer.wav"))
    {
        cheerSound = LoadSound("assets/sound/cheer.wav");
    }
    else
    {
        cerr << "WARNING: 'cheer.wav' not found." << endl;
    }

    // Background music settings
    vector<string> playlistFiles = { "assets/sound/Investigations.wav", "assets/sound/Fluffing_a_Duck.wav", "assets/sound/Sneaky_Adventure.wav" };
    vector<Music> musicPlaylist;
    int currentTrackIndex = -1;

    for (const string& fileName : playlistFiles) {
        if (FileExists(fileName.c_str())) {
            Music m = LoadMusicStream(fileName.c_str());
            m.looping = false;
            musicPlaylist.push_back(m);
        }
        else {
            cerr << "WARNING: '" << fileName << "' not found." << endl;
        }
    }

    auto PlayRandomTrack = [&]() {
        if (musicPlaylist.empty()) return;

        int nextTrack = 0;

        if (musicPlaylist.size() > 1) {
            nextTrack = GetRandomValue(0, (int)musicPlaylist.size() - 1);

            if (nextTrack == currentTrackIndex)
            {
                nextTrack = (nextTrack + 1) % (int)musicPlaylist.size();
            }
        }

        currentTrackIndex = nextTrack;
        PlayMusicStream(musicPlaylist[currentTrackIndex]);
        SetMusicVolume(musicPlaylist[currentTrackIndex], 0.5f);
    };

    // Initial start
    if (!musicPlaylist.empty()) PlayRandomTrack();

    // initial player rect
    Rectangle player = { 0.0f, 0.0f, PLAYER_WIDTH, PLAYER_HEIGHT };

    // preserve 10px overlap into ground
    const float bottomOffset = (float)GROUND_HEIGHT - 5.0f;
    player.y = (float)SCREEN_HEIGHT - PLAYER_HEIGHT - bottomOffset;

    float PLAYER_GROUND_Y = player.y;

    Camera2D camera = { 0 };
    camera.target = { player.x + player.width / 2.0f, player.y + player.height / 2.0f };
    camera.offset = { (float)SCREEN_WIDTH / 2.0f, (float)SCREEN_HEIGHT / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    vector<NPC> npcs;
    npcs.reserve(5);

    // Helper to create NPCs
    auto makeNpc = [&](float x, const vector<string>& lines, int spriteId = 0, Sound* speech = nullptr) -> NPC
        {
            float w = 64.0f;
            float h = 120.0f;
            float y = (float)SCREEN_HEIGHT - h - (float)GROUND_HEIGHT;
            Rectangle bounds = { x, y, w, h };
            Rectangle interaction = { x + w / 2.0f - INTERACTION_RADIUS / 2.0f, y, INTERACTION_RADIUS, PLAYER_HEIGHT - GROUND_HEIGHT };

            NPC npc;
            npc.bounds = bounds;
            npc.interactionArea = interaction;
            npc.lines = lines;
            npc.spriteId = spriteId;
            npc.speech = (speech != nullptr && speech->frameCount != 0) ? speech : nullptr;
            npc.hasSpeech = (npc.speech != nullptr);
            return npc;
        };

    vector<NPCDefinition> npcDefinitions = {
        { 640.0f, {
            "Zróżnicowanie zasobów wody na świecie: jedne regiony mają dużo wody słodkiej, inne bardzo mało.",
            "Dostępność wody słodkiej zależy od klimatu, geologii i infrastruktury.",
            "Zrozumienie tego zróżnicowania jest kluczowe dla planowania i sprawiedliwego dostępu."
            }, 0, (meow1Sound.frameCount != 0 ? &meow1Sound : nullptr) },

        { 1920.0f, {
            "Niedobory wody dotykają miliardy ludzi. Przyczyny to wzrost populacji, zanieczyszczenia i zmiany klimatu.",
            "Susze i nadmierne pobory zasilają kryzysy wodne, szczególnie w krajach rozwijających się.",
            "Inwestycje w infrastrukturę, zarządzanie zasobami i edukacja są niezbędne, by łagodzić skutki."
            }, 1, nullptr },

        { 3200.0f, {
            "Człowiek zagraża hydrosferze poprzez zanieczyszczenia, nadmierne pobory i degradację siedlisk.",
            "Plastiki, chemikalia i ścieki przemysłowe zmniejszają jakość wody i szkodzą organizmom.",
            "Ograniczanie emisji, regulacje i ochrona stref brzegowych to kluczowe działania."
            }, 3, (meow2Sound.frameCount != 0 ? &meow2Sound : nullptr) },

        { 4480.0f, {
            "Jezioro Aralskie to przykład katastrofy ekologicznej: odpływ rzek do nawadniania zmniejszył jego powierzchnię.",
            "Wysoka Tama na Nilu miała korzyści w hydroenergetyce, ale zmieniła sedymentację i lokalne ekosystemy.",
            "Studium tych przykładów uczy nas o konsekwencjach dużych projektów wodnych i konieczności zrównoważenia."
            }, 2, nullptr },

        { 5760.0f, {
            "Jak chronić hydrosferę? Oszczędzanie wody, oczyszczanie ścieków i redukcja zanieczyszczeń są podstawowe.",
            "Inwestycje w odnawialne źródła, zrównoważone rolnictwo i ochrona terenów przybrzeżnych są kluczowe.",
            "Edukacja i współpraca międzynarodowa umożliwiają długotrwałe rozwiązania dla całej hydrosfery."
            }, 0, (meow1Sound.frameCount != 0 ? &meow1Sound : nullptr) }
    };

    for (const auto& def : npcDefinitions)
        npcs.push_back(makeNpc(def.x, def.lines, def.spriteId, def.speech));

    // Animation / state
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

    int catSpinningFrame = 0;
    float catSpinningTimer = 0.0f;

    int activeNPC = -1;
    int currentDialogueLine = 0;
    string rawDialogueText;
    string wrappedDialogueText;
    int textDisplayLength = 0;
    int prevTextDisplayLength = 0;

    bool spinningCatVanished = false;
    bool spinningCatVanishing = false;
    float spinningCatVanishTimer = 0.0f;
    const float SPINNING_CAT_VANISH_DURATION = 1.0f;
    float spinningCatScale = 1.5f;

    float spinningCatDestX = SECRET_X_OFFSET + (SECRET_ROOM_WIDTH / 2.0f) - ((CATSPINNING_FRAME_WIDTH * 1.5f) / 2.0f);
    float spinningCatDestY = SCREEN_HEIGHT - GROUND_HEIGHT - (CATSPINNING_FRAME_HEIGHT * 1.5f);
    Rectangle spinningCatInteractionArea = {
        spinningCatDestX,
        spinningCatDestY,
        CATSPINNING_FRAME_WIDTH * 1.5f,
        CATSPINNING_FRAME_HEIGHT * 1.5f
    };

    // Per-character reveal timers
    float charTimer = 0.0f;
    const float charInterval = (TEXT_SPEED > 0) ? (1.0f / (float)TEXT_SPEED) : 1e6f;
    float punctuationPauseRemaining = 0.0f;
    const float PUNCTUATION_PAUSE = 0.35f;

    bool mouthOpen = false;
    float mouthTimer = 0.0f;
    const float mouthToggleInterval = (TEXT_SPEED > 0) ? (2.0f / (float)TEXT_SPEED) : 1e6f;

    const int MAX_TEXT_WIDTH = (int)(SCREEN_WIDTH * 0.8f) - (2 * TEXT_PADDING);

    const string PUNCTUATION_CHARS = ".,;:!?";

    auto PlayDialogueCharSound = [&](int npcIndex, char ch)
        {
            if (npcIndex < 0 || (size_t)npcIndex >= npcs.size()) return;
            if (!npcs[npcIndex].hasSpeech) return;
            if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') return;
            Sound* s = npcs[npcIndex].speech;
            if (s != nullptr && s->frameCount != 0) PlaySound(*s);
        };

    // Finish/kitty-happy animation settings
    bool finishTriggered = false;
    float happyTimer = 0.0f;
    const int HAPPY_FRAME_COUNT = 64;
    const int HAPPY_FRAME_W = 128;
    const int HAPPY_FRAME_H = 128;
    const float HAPPY_FPS = 24.0f;
    const float HAPPY_FRAME_TIME = 1.0f / HAPPY_FPS;
    const float HAPPY_RENDER_SIZE = 339.0f;
    const float HAPPY_DRAW_OFFSET_Y = 144.0f;
    const int HAPPY_LOOPS = 2;

    // congratulation spritesheet settings
    const int CONGRATS_FRAMES = 4;
    const int CONGRATS_W = 600;
    const int CONGRATS_H = 193;
    const float CONGRATS_FPS = 8.0f;
    const float CONGRATS_FRAME_TIME = 1.0f / CONGRATS_FPS;
    const float CONGRATS_RENDER_W = (float)CONGRATS_W * 1.5f;
    const float CONGRATS_RENDER_H = (float)CONGRATS_H * 1.5f;
    float congratsTimer = 0.0f;
    const float CONGRATS_MARGIN_TOP = 20.0f;

    // Finish flag collision bounds
    const float FINISH_FLAG_W = 184.0f;
    const float FINISH_FLAG_H = 92.0f;
    Rectangle finishFlagBounds = { (float)(WORLD_WIDTH - 200), (float)(SCREEN_HEIGHT - GROUND_HEIGHT - FINISH_FLAG_H), FINISH_FLAG_W, FINISH_FLAG_H };

    SetTargetFPS(60);
    
    bool isDebugMode = false;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        
        // Debug mode toggle
        if (IsKeyPressed(KEY_F3))
        {
            isDebugMode = !isDebugMode;
        }

        // Hot Reload Assets
        if (IsKeyPressed(KEY_F5))
        {
            // Unload existing then reload textures
            if (catWalkTexture.id != 0) UnloadTexture(catWalkTexture);
            catWalkTexture = LoadTexture("walk.png");

            if (catRunTexture.id != 0) UnloadTexture(catRunTexture);
            catRunTexture = LoadTexture("run.png");

            if (catJumpTexture.id != 0) UnloadTexture(catJumpTexture);
            catJumpTexture = LoadTexture("jump.png");

            if (npcTexture.id != 0) UnloadTexture(npcTexture);
            npcTexture = LoadTexture("npc.png");

            if (catPopTexture.id != 0) UnloadTexture(catPopTexture);
            catPopTexture = LoadTexture("cat-pop.png");

            if (catCrunchTexture.id != 0) UnloadTexture(catCrunchTexture);
            catCrunchTexture = LoadTexture("cat-crunch.png");

            if (catCryTexture.id != 0) UnloadTexture(catCryTexture);
            catCryTexture = LoadTexture("cat-cry.png");

            if (catSpinningTexture.id != 0) UnloadTexture(catSpinningTexture);
            catSpinningTexture = LoadTexture("cat-spinning.png");

            if (grassTexture.id != 0) UnloadTexture(grassTexture);
            grassTexture = LoadTexture("grass.png");

            if (finishTexture.id != 0) UnloadTexture(finishTexture);
            finishTexture = LoadTexture("finish.png");

            if (happyTexture.id != 0) UnloadTexture(happyTexture);
            happyTexture = LoadTexture("kitty-happy.png");

            if (congratsTexture.id != 0) UnloadTexture(congratsTexture);
            congratsTexture = LoadTexture("congratulation.png");

            for (int i = 0; i < SEG_COUNT; ++i)
            {
                if (biomeTextures[i].id != 0) UnloadTexture(biomeTextures[i]);
                string filename = "biome" + to_string(i + 1) + ".png";
                biomeTextures[i] = LoadTexture(filename.c_str());
            }

            // Reload Sounds
            if (meow1Sound.frameCount != 0) UnloadSound(meow1Sound);
            if (FileExists("meow1.wav")) meow1Sound = LoadSound("meow1.wav");

            if (meow2Sound.frameCount != 0) UnloadSound(meow2Sound);
            if (FileExists("meow2.wav")) meow2Sound = LoadSound("meow2.wav");

            if (popSound.frameCount != 0) UnloadSound(popSound);
            if (FileExists("pop.wav")) popSound = LoadSound("pop.wav");

            if (vanishSound.frameCount != 0) UnloadSound(vanishSound);
            if (FileExists("vanish.wav")) vanishSound = LoadSound("vanish.wav");

            if (crunchSound.frameCount != 0) UnloadSound(crunchSound);
            if (FileExists("crunch.wav")) crunchSound = LoadSound("crunch.wav");

            if (jumpSound.frameCount != 0) UnloadSound(jumpSound);
            if (FileExists("jump.wav")) jumpSound = LoadSound("jump.wav");

            if (sprintSound.frameCount != 0) UnloadSound(sprintSound);
            if (FileExists("sprint.wav")) sprintSound = LoadSound("sprint.wav");

            if (cheerSound.frameCount != 0) UnloadSound(cheerSound);
            if (FileExists("cheer.wav")) cheerSound = LoadSound("cheer.wav");
            
            // Reassign Speech Pointers for NPCs
            if (npcs.size() >= 5)
            {
                npcs[0].speech = (meow1Sound.frameCount != 0 ? &meow1Sound : nullptr);
                npcs[0].hasSpeech = (npcs[0].speech != nullptr);
                
                npcs[2].speech = (meow2Sound.frameCount != 0 ? &meow2Sound : nullptr);
                npcs[2].hasSpeech = (npcs[2].speech != nullptr);
                
                npcs[4].speech = (meow1Sound.frameCount != 0 ? &meow1Sound : nullptr);
                npcs[4].hasSpeech = (npcs[4].speech != nullptr);
            }
        }

        isMoving = false;

        if (!finishTriggered && !musicPlaylist.empty() && currentTrackIndex != -1)
        {
            UpdateMusicStream(musicPlaylist[currentTrackIndex]);

            // Mute background music if the player is in the secret room
            int playerCenterXForMusic = player.x + player.width / 2;
            if (playerCenterXForMusic < 0)
            {
                SetMusicVolume(musicPlaylist[currentTrackIndex], 0.0f);
            }
            else
            {
                SetMusicVolume(musicPlaylist[currentTrackIndex], 0.5f);
            }

            float played = GetMusicTimePlayed(musicPlaylist[currentTrackIndex]);
            float length = GetMusicTimeLength(musicPlaylist[currentTrackIndex]);

            if (played >= length - 1.0f)
            {
                StopMusicStream(musicPlaylist[currentTrackIndex]);
                PlayRandomTrack();
            }
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

        // cat-spinning animation
        catSpinningTimer += dt;
        const float catSpinningFrameTime = 1.0f / (float)CATSPINNING_FRAME_SPEED;
        if (catSpinningTimer >= catSpinningFrameTime)
        {
            catSpinningTimer -= catSpinningFrameTime;
            catSpinningFrame = (catSpinningFrame + 1) % CATSPINNING_FRAME_COUNT;
        }

        if (spinningCatVanishing)
        {
            spinningCatVanishTimer += dt;
            if (spinningCatVanishTimer >= SPINNING_CAT_VANISH_DURATION)
            {
                spinningCatVanished = true;
                spinningCatVanishing = false;
            }
            else
            {
                // rapidly increase scale during vanishing
                spinningCatScale = 1.5f + (spinningCatVanishTimer / SPINNING_CAT_VANISH_DURATION) * 5.0f;
            }
        }

        // Jump update
        if (isJumping)
        {
            jumpTimer += dt;
            int frameIndex = (int)floorf((jumpTimer / JUMP_DURATION) * (float)JUMP_FRAME_COUNT);
            frameIndex = max(0, min(frameIndex, JUMP_FRAME_COUNT - 1));
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

        float speedMultiplier = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) ? (isDebugMode ? 6.0f : 3.0f) : 1.0f;

        // Movement (disabled during dialogue or when finishTriggered)
        if (!finishTriggered && activeNPC == -1)
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
                if (jumpSound.frameCount != 0) PlaySound(jumpSound);
            }
        }

        player.x = max(SECRET_X_OFFSET, min(player.x, (float)(WORLD_WIDTH - (int)player.width)));

        // Check collision with finish flag
        if (!finishTriggered && CheckCollisionRecs(player, finishFlagBounds))
        {
            finishTriggered = true;
            activeNPC = -1;
            textDisplayLength = 0;
            prevTextDisplayLength = 0;
            charTimer = 0.0f;
            punctuationPauseRemaining = 0.0f;
            mouthOpen = false;
            mouthTimer = 0.0f;
            happyTimer = 0.0f;
            congratsTimer = 0.0f;
            if (!musicPlaylist.empty() && currentTrackIndex != -1) StopMusicStream(musicPlaylist[currentTrackIndex]);
            
            if (cheerSound.frameCount != 0) PlaySound(cheerSound);
        }

        // Determine current segment and manage fade
        int playerCenterX = player.x + player.width / 2;
        int segIndex = (playerCenterX < 0) ? 0 : (playerCenterX / SEG_W);
        segIndex = max(0, min(segIndex, SEG_COUNT - 1));

        if (displayedBiome == -1)
        {
            displayedBiome = segIndex;
            fadingFrom = -1;
            fadingTo = -1;
            fadeTimer = 0.0f;
        }
        else if (segIndex != displayedBiome && segIndex != fadingTo)
        {
            fadingFrom = displayedBiome;
            fadingTo = segIndex;
            fadeTimer = 0.0f;
        }

        // Walking / Running animation
        if (!isJumping && !finishTriggered)
        {
            if (isMoving)
            {
                if (speedMultiplier > 1.0f) // Running
                {
                    runFrameCounter++;
                    int runFrameDelay = max(1, (int)round(60.0f / (float)RUN_FRAME_SPEED));
                    if (runFrameCounter >= runFrameDelay)
                    {
                        runFrameCounter = 0;
                        currentRunFrame = (currentRunFrame + 1) % RUN_FRAME_COUNT;
                    }
                }
                else // Walking
                {
                    frameCounter++;
                    int frameDelay = max(1, (int)round(60.0f / (float)FRAME_SPEED));
                    if (frameCounter >= frameDelay)
                    {
                        frameCounter = 0;
                        currentFrame = (currentFrame + 1) % FRAME_COUNT;
                    }
                }
            }
        }

        // Check nearby NPC
        int foundNear = -1;
        for (size_t i = 0; i < npcs.size(); ++i)
        {
            if (CheckCollisionRecs(player, npcs[i].interactionArea))
            {
                foundNear = (int)i;
                break;
            }
        }

        bool nearSpinningCat = false;
        if (!spinningCatVanished && !spinningCatVanishing && CheckCollisionRecs(player, spinningCatInteractionArea))
        {
            nearSpinningCat = true;
        }

        bool enterConsumedForStart = false;

        if (nearSpinningCat && IsKeyPressed(KEY_ENTER))
        {
            spinningCatVanishing = true;
            enterConsumedForStart = true;
            if (vanishSound.frameCount != 0) PlaySound(vanishSound);
        }

        // Start dialogue
        if (!finishTriggered && foundNear != -1 && activeNPC == -1 && IsKeyPressed(KEY_ENTER) && !enterConsumedForStart)
        {
            activeNPC = foundNear;
            currentDialogueLine = 0;
            rawDialogueText = npcs[activeNPC].lines[currentDialogueLine];
            wrappedDialogueText = WordWrapText(rawDialogueText, MAX_TEXT_WIDTH, uiFont, TEXT_FONT_SIZE, 4.0f);
            textDisplayLength = 0;
            prevTextDisplayLength = 0;
            charTimer = 0.0f;
            punctuationPauseRemaining = 0.0f;
            mouthOpen = false;
            mouthTimer = 0.0f;
            enterConsumedForStart = true;

            int sid = npcs[activeNPC].spriteId;
            if (sid == 1 && popSound.frameCount != 0) PlaySound(popSound);
            if (sid == 2 && crunchSound.frameCount != 0) PlaySound(crunchSound);
        }

        // Leave dialogue if player exits area
        if (!finishTriggered && foundNear == -1 && activeNPC != -1)
        {
            int sid = npcs[activeNPC].spriteId;
            if (sid == 1 && popSound.frameCount != 0) StopSound(popSound);
            if (sid == 2 && crunchSound.frameCount != 0) StopSound(crunchSound);

            activeNPC = -1;
            rawDialogueText.clear();
            wrappedDialogueText.clear();
            textDisplayLength = 0;
            prevTextDisplayLength = 0;
            charTimer = 0.0f;
            punctuationPauseRemaining = 0.0f;
            mouthOpen = false;
            mouthTimer = 0.0f;
        }

        // Advance/skip dialogue
        if (!finishTriggered && activeNPC != -1 && IsKeyPressed(KEY_ENTER) && !enterConsumedForStart)
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

                punctuationPauseRemaining = 0.0f;
                charTimer = 0.0f;

                if (sid == 1 && popSound.frameCount != 0) StopSound(popSound);
            }
            else
            {
                currentDialogueLine++;
                if (currentDialogueLine < (int)npcs[activeNPC].lines.size())
                {
                    rawDialogueText = npcs[activeNPC].lines[currentDialogueLine];
                    wrappedDialogueText = WordWrapText(rawDialogueText, MAX_TEXT_WIDTH, uiFont, TEXT_FONT_SIZE, 4.0f);
                    textDisplayLength = 0;
                    prevTextDisplayLength = 0;
                    charTimer = 0.0f;
                    punctuationPauseRemaining = 0.0f;
                    mouthOpen = false;
                    mouthTimer = 0.0f;

                    if (sid == 1 && !(popSound.frameCount != 0 && IsSoundPlaying(popSound)) && popSound.frameCount != 0) PlaySound(popSound);
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
                    charTimer = 0.0f;
                    punctuationPauseRemaining = 0.0f;
                    mouthOpen = false;
                    mouthTimer = 0.0f;
                }
            }
        }

        // Reveal text with punctuation pause
        if (!finishTriggered && activeNPC != -1 && textDisplayLength < (int)wrappedDialogueText.length())
        {
            if (punctuationPauseRemaining > 0.0f)
            {
                punctuationPauseRemaining -= dt;
                if (punctuationPauseRemaining <= 0.0f)
                {
                    punctuationPauseRemaining = 0.0f;
                    charTimer = 0.0f;
                }
            }
            else
            {
                charTimer += dt;
                while (charTimer >= charInterval && textDisplayLength < (int)wrappedDialogueText.length())
                {
                    charTimer -= charInterval;
                    int revealIndex = textDisplayLength;
                    char ch = wrappedDialogueText[revealIndex];
                    textDisplayLength++;
                    PlayDialogueCharSound(activeNPC, ch);

                    if (PUNCTUATION_CHARS.find(ch) != string::npos)
                    {
                        punctuationPauseRemaining = PUNCTUATION_PAUSE;
                        break;
                    }
                }

                if (textDisplayLength >= (int)wrappedDialogueText.length())
                {
                    int sid = npcs[activeNPC].spriteId;
                    if (sid == 1 && popSound.frameCount != 0) StopSound(popSound);
                }
            }
        }

        // Ensure crunch loops during conversation
        if (!finishTriggered && activeNPC != -1 && npcs[activeNPC].spriteId == 2)
        {
            if (!(crunchSound.frameCount != 0 && IsSoundPlaying(crunchSound)) && crunchSound.frameCount != 0) PlaySound(crunchSound);
        }

        // Mouth animation while text reveals
        if (!finishTriggered && activeNPC != -1 && textDisplayLength < (int)wrappedDialogueText.length())
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

        // Advance fade timer if crossfading
        if (fadingTo != -1)
        {
            fadeTimer += dt;
            if (fadeTimer >= FADE_DURATION)
            {
                displayedBiome = fadingTo;
                fadingFrom = -1;
                fadingTo = -1;
                fadeTimer = 0.0f;
            }
        }

        // If finish triggered update happy animation timer and determine if finished
        if (finishTriggered)
        {
            happyTimer += dt;
            congratsTimer += dt;
            int totalFramesPlayed = (int)floorf(happyTimer / HAPPY_FRAME_TIME);
            if (totalFramesPlayed >= HAPPY_FRAME_COUNT * HAPPY_LOOPS)
            {
                // finished all loops -> exit main loop to allow cleanup and close
                break;
            }
        }

        // Camera follow
        camera.target = { player.x + player.width / 2.0f, player.y + player.height / 2.0f };
        if (playerCenterX < 0.0f)
        {
            camera.target.x = SECRET_X_OFFSET / 2.0f;
        }
        else
        {
            camera.target.x = playerCenterX;

            float minCamX = (float)SCREEN_WIDTH / 2.0f;
            float maxCamX = (float)WORLD_WIDTH - (SCREEN_WIDTH / 2.0f);

            if (camera.target.x < minCamX) camera.target.x = minCamX;
            if (camera.target.x > maxCamX) camera.target.x = maxCamX;
        }
        camera.target.y = (float)SCREEN_HEIGHT / 2.0f;

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw static screen-space biome backgrounds with fade
        auto drawBiomeTex = [&](int idx, float alphaFactor) {
            if (idx < 0 || idx >= SEG_COUNT) return false;
            if (biomeTextures[idx].id != 0)
            {
                Rectangle src = { 0.0f, 0.0f, (float)biomeTextures[idx].width, (float)biomeTextures[idx].height };
                Rectangle dst = { 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
                Color c = WHITE;
                c.a = (unsigned char)(255 * alphaFactor);
                DrawTexturePro(biomeTextures[idx], src, dst, { 0, 0 }, 0.0f, c);
                return true;
            }
            return false;
            };

        if (displayedBiome >= 0)
        {
            if (camera.target.x > 0)
            {
                if (fadingTo == -1) {
                    drawBiomeTex(displayedBiome, 1.0f);
                }
                else {
                    float t = fmin(1.0f, fadeTimer / FADE_DURATION);
                    drawBiomeTex(fadingFrom, 1.0f - t);
                    drawBiomeTex(fadingTo, t);
                }
            }
        }
        else
        {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SKYBLUE);
        }

        BeginMode2D(camera);

        // Draw secret room
        DrawRectangle(SECRET_X_OFFSET, 0, SECRET_ROOM_WIDTH, SCREEN_HEIGHT, CLITERAL(Color){ 10, 10, 30, 255 });

        if (catSpinningTexture.id != 0 && !spinningCatVanished)
        {
            float destW = CATSPINNING_FRAME_WIDTH * spinningCatScale;
            float destH = CATSPINNING_FRAME_HEIGHT * spinningCatScale;
            float destX = SECRET_X_OFFSET + (SECRET_ROOM_WIDTH / 2.0f) - (destW / 2.0f);
            float destY = SCREEN_HEIGHT - GROUND_HEIGHT - destH;
            
            Color c = WHITE;
            if (spinningCatVanishing)
            {
                float alpha = 1.0f - (spinningCatVanishTimer / SPINNING_CAT_VANISH_DURATION);
                c.a = (unsigned char)(255 * fmax(0.0f, alpha));
            }
            
            Rectangle srcRec = { (float)(catSpinningFrame * CATSPINNING_FRAME_WIDTH), 0.0f, (float)CATSPINNING_FRAME_WIDTH, (float)CATSPINNING_FRAME_HEIGHT };
            Rectangle destRec = { destX, destY, destW, destH };
            DrawTexturePro(catSpinningTexture, srcRec, destRec, { 0, 0 }, 0.0f, c);
            
            // Interaction border for spinning cat
            if (!spinningCatVanishing && isDebugMode)
            {
                Color zoneColor = nearSpinningCat ? RED : YELLOW;
                DrawRectangleLinesEx(spinningCatInteractionArea, 2, zoneColor);
            }
        }

        // Draw tiled grass
        int tileW = (grassTexture.id != 0) ? grassTexture.width : GRASS_TILE_SIZE;
        int groundY = SCREEN_HEIGHT - GROUND_HEIGHT;
        for (int gx = 0; gx < WORLD_WIDTH; gx += tileW)
        {
            if (grassTexture.id != 0)
                DrawTexture(grassTexture, gx, groundY, WHITE);
            else
                DrawRectangle(gx, groundY, tileW, GROUND_HEIGHT, DARKGREEN);
        }

        // Draw finish flag
        if (finishTexture.id != 0)
        {
            Rectangle srcF = { 0.0f, 0.0f, (float)finishTexture.width, (float)finishTexture.height };
            Rectangle dstF = { finishFlagBounds.x, finishFlagBounds.y, finishFlagBounds.width, finishFlagBounds.height };
            DrawTexturePro(finishTexture, srcF, dstF, { 0, 0 }, 0.0f, WHITE);
        }
        else
        {
            DrawRectangle(finishFlagBounds.x, finishFlagBounds.y, finishFlagBounds.width, finishFlagBounds.height, RED);
        }

        // Draw finish flag border
        if (isDebugMode)
        {
            bool playerNearFlag = CheckCollisionRecs(player, finishFlagBounds);
            Color zoneColor = playerNearFlag ? RED : YELLOW;
            DrawRectangleLinesEx(finishFlagBounds, 2, zoneColor);
        }

        // Draw NPCs
        for (size_t i = 0; i < npcs.size(); ++i)
        {
            NPC& npc = npcs[i];
            bool isCurrentlyNear = ((int)i == foundNear);

            if (isDebugMode)
            {
                Color zoneColor = isCurrentlyNear ? (YELLOW) : YELLOW;
                if (isCurrentlyNear) zoneColor = (/*dialogueFinished?*/ false ? DARKGRAY : RED);
                DrawRectangleLinesEx(npc.interactionArea, 2, zoneColor);
            }

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
            }
            else if (npc.spriteId == 3 && catCryTexture.id != 0)
            {
                texPtr = &catCryTexture;
                frameWidth = CATCRY_FRAME_WIDTH;
                frameHeight = CATCRY_FRAME_HEIGHT;
                frameCount = CATCRY_FRAME_COUNT;
                frameIndex = catCryFrame;
            }
            else if (npc.spriteId == 1 && catPopTexture.id != 0)
            {
                texPtr = &catPopTexture;
                frameWidth = CATPOP_FRAME_WIDTH;
                frameHeight = CATPOP_FRAME_HEIGHT;
                frameCount = CATPOP_FRAME_COUNT;
                frameIndex = ((int)i == activeNPC && mouthOpen) ? 1 : 0;
            }
            else if (npcTexture.id != 0)
            {
                texPtr = &npcTexture;
                frameWidth = NPC_FRAME_WIDTH;
                frameHeight = NPC_FRAME_HEIGHT;
                frameCount = NPC_FRAME_COUNT;
                frameIndex = ((int)i == activeNPC && mouthOpen) ? 1 : 0;
            }

            if (frameCount > 0 && frameIndex >= frameCount) frameIndex = frameCount - 1;

            if (texPtr != nullptr && texPtr->id != 0)
            {
                float targetHeight = npc.bounds.height * 2.2f;
                float scale = targetHeight / (float)frameHeight;
                float renderW = frameWidth * scale;
                float renderH = frameHeight * scale;

                float destX = npc.bounds.x + npc.bounds.width / 2.0f - renderW / 2.0f;
                float destY = npc.bounds.y + npc.bounds.height - renderH;

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
        if (finishTriggered && happyTexture.id != 0)
        {
            int totalFramesPlayed = (int)floorf(happyTimer / HAPPY_FRAME_TIME);
            int happyFrameIndex = totalFramesPlayed % HAPPY_FRAME_COUNT;
            int cols = (happyTexture.width > 0) ? (happyTexture.width / HAPPY_FRAME_W) : 1;
            if (cols <= 0) cols = 1;
            int row = happyFrameIndex / cols;
            int col = happyFrameIndex % cols;
            float srcX = (float)(col * HAPPY_FRAME_W);
            float srcY = (float)(row * HAPPY_FRAME_H);

            if (srcX + HAPPY_FRAME_W > happyTexture.width) srcX = (float)max(0, happyTexture.width - HAPPY_FRAME_W);
            if (srcY + HAPPY_FRAME_H > happyTexture.height) srcY = (float)max(0, happyTexture.height - HAPPY_FRAME_H);

            Rectangle srcRec = { srcX, srcY, (float)HAPPY_FRAME_W, (float)HAPPY_FRAME_H };
            Rectangle destRec = { player.x, player.y - HAPPY_DRAW_OFFSET_Y, HAPPY_RENDER_SIZE, HAPPY_RENDER_SIZE };
            DrawTexturePro(happyTexture, srcRec, destRec, { 0, 0 }, 0.0f, WHITE);
        }
        else
        {
            // normal player rendering (jump/run/walk)
            if (isJumping && catJumpTexture.id != 0)
            {
                Rectangle sourceRec = {
                    (float)jumpFrame * (float)FRAME_WIDTH,
                    0.0f,
                    (float)FRAME_WIDTH * frameDirection,
                    (float)FRAME_HEIGHT
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
                Rectangle sourceRec = {
                    (float)currentRunFrame * (float)FRAME_WIDTH,
                    0.0f,
                    (float)FRAME_WIDTH * frameDirection,
                    (float)FRAME_HEIGHT
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
        }

        if (isDebugMode)
        {
            DrawRectangleLinesEx(player, 2, GREEN);
        }

        EndMode2D();

        // Draw congratulation animation
        if (finishTriggered && congratsTexture.id != 0)
        {
            int frameIndex = (int)floorf(congratsTimer / CONGRATS_FRAME_TIME) % CONGRATS_FRAMES;
            int cols = (congratsTexture.width > 0) ? (congratsTexture.width / CONGRATS_W) : 1;
            if (cols <= 0) cols = 1;
            int row = frameIndex / cols;
            int col = frameIndex % cols;
            float srcX = (float)(col * CONGRATS_W);
            float srcY = (float)(row * CONGRATS_H);

            if (srcX + CONGRATS_W > congratsTexture.width) srcX = (float)max(0, congratsTexture.width - CONGRATS_W);
            if (srcY + CONGRATS_H > congratsTexture.height) srcY = (float)max(0, congratsTexture.height - CONGRATS_H);

            Rectangle src = { srcX, srcY, (float)CONGRATS_W, (float)CONGRATS_H };

            float cx = (float)SCREEN_WIDTH * 0.5f - CONGRATS_RENDER_W * 0.5f;
            Rectangle dst = { cx, CONGRATS_MARGIN_TOP, CONGRATS_RENDER_W, CONGRATS_RENDER_H };
            DrawTexturePro(congratsTexture, src, dst, { 0, 0 }, 0.0f, WHITE);
        }

        // Dialogue box
        if (!finishTriggered && activeNPC != -1)
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

        if (isDebugMode)
        {
            DrawTextEx(uiFont, TextFormat("Player X: %.2f", player.x), { 10.0f, 10.0f }, 20.0f, 1.0f, DARKGRAY);
            DrawTextEx(uiFont, TextFormat("Active NPC: %s", activeNPC == -1 ? "NONE" : "YES"), { 10.0f, 40.0f }, 20.0f, 1.0f, DARKGRAY);
        }

        EndDrawing();
    }

    // Cleanup textures
    UnloadTexture(catWalkTexture);
    UnloadTexture(catRunTexture);
    UnloadTexture(npcTexture);
    UnloadTexture(catPopTexture);
    UnloadTexture(catCrunchTexture);
    UnloadTexture(catCryTexture);
    UnloadTexture(catJumpTexture);
    UnloadTexture(catSpinningTexture);

    if (grassTexture.id != 0) UnloadTexture(grassTexture);
    for (int i = 0; i < SEG_COUNT; ++i)
        if (biomeTextures[i].id != 0) UnloadTexture(biomeTextures[i]);

    // Unload finish/happy assets
    if (finishTexture.id != 0) UnloadTexture(finishTexture);
    if (happyTexture.id != 0) UnloadTexture(happyTexture);
    if (congratsTexture.id != 0) UnloadTexture(congratsTexture);

    UnloadFont(uiFont);

    if (meow1Sound.frameCount != 0) UnloadSound(meow1Sound);
    if (meow2Sound.frameCount != 0) UnloadSound(meow2Sound);
    if (popSound.frameCount != 0) UnloadSound(popSound);
    if (crunchSound.frameCount != 0) UnloadSound(crunchSound);
    if (jumpSound.frameCount != 0) UnloadSound(jumpSound);
    if (sprintSound.frameCount != 0) UnloadSound(sprintSound);
    if (cheerSound.frameCount != 0) UnloadSound(cheerSound);
    if (vanishSound.frameCount != 0) UnloadSound(vanishSound);

    // Stop and unload background music
    for (auto& m : musicPlaylist) {
        StopMusicStream(m);
        UnloadMusicStream(m);
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}