=====================================================================
 TOP-DOWN STRIKE  -  OOP Lab Project  (C++ / Raylib)
=====================================================================
A top-down tactical shooter. You are a Counter-Terrorist defending bomb
sites A and B against waves of AI Terrorists. Survive 5 rounds.

---------------------------------------------------------------------
 1. PROJECT STRUCTURE
---------------------------------------------------------------------
top-down-strike/
  main.cpp          creates a Game and calls run()
  Game.h / .cpp     main loop, menus, rounds, HUD (owns everything)
  Character.h/.cpp  ABSTRACT base class: position, health, movement
  Player.h / .cpp   Player : Character   (keyboard + mouse, weapons)
  Enemy.h / .cpp    ABSTRACT Enemy : Character
                    NormalTerrorist / AggressiveTerrorist / SniperTerrorist
  Weapon.h / .cpp   ABSTRACT Weapon
                    Pistol / AssaultRifle / Shotgun / SniperRifle
  Bullet.h / .cpp   Bullet (projectile) and Effect (hit sparks)
  Bomb.h / .cpp     C4: planting, countdown, defusing, explosion
  Map.h / .cpp      walls, crates, bomb sites, spawns, line of sight
  Utils.h           small inline math helpers (the only extra file)
  README.txt, VIVA_QUESTIONS.txt

---------------------------------------------------------------------
 2. INSTALLING RAYLIB AND COMPILING ON WINDOWS
---------------------------------------------------------------------
You need raylib 4.0 or newer (the code uses MOUSE_BUTTON_LEFT) and a
C++11 compiler (MinGW-w64 / TDM-GCC). The compiler and the raylib build
must match (both 64-bit or both 32-bit).

OPTION A - Official raylib installer (easiest, recommended)
  1. Download "raylib installer" from https://www.raylib.com
     (it installs raylib + the w64devkit compiler + Notepad++).
  2. Copy the whole top-down-strike folder anywhere, open a terminal
     in it (w64devkit shell or the raylib "Command prompt") and run:

     g++ main.cpp Game.cpp Character.cpp Player.cpp Enemy.cpp Weapon.cpp Bullet.cpp Bomb.cpp Map.cpp -o top-down-strike.exe -std=c++11 -O2 -lraylib -lopengl32 -lgdi32 -lwinmm

  3. Run top-down-strike.exe (or just open main.cpp in VS Code and press F5)

OPTION B - Dev-C++ (Embarcadero / Orwell, TDM-GCC)
  1. Download a raylib release built for MinGW-w64 from
     https://github.com/raysan5/raylib/releases (raylib-x.y_win64_mingw-w64.zip).
     Copy its include\raylib.h to  <Dev-Cpp>\MinGW64\include  and
     lib\libraylib.a to  <Dev-Cpp>\MinGW64\lib   (use the matching
     32-bit/64-bit folder of your Dev-C++ compiler).
  2. File > New > Project > Console Application > C++ Project.
     Add all .cpp files (Project > Add to Project). Keep the .h files
     in the same folder.
  3. Project > Project Options > Parameters:
        C++ compiler :  -std=c++11
        Linker       :  -lraylib -lopengl32 -lgdi32 -lwinmm
     (If Dev-C++ uses its own compiler settings instead: Tools >
      Compiler Options > Settings > Code Generation > Language standard
      = ISO C++11, and add the linker line above to the Linker box.)
  4. Execute > Rebuild All, then Execute > Run.

Troubleshooting
  * "cannot find -lraylib"  -> libraylib.a is not in the compiler's lib folder.
  * "raylib.h: No such file" -> raylib.h is not in the compiler's include folder.
  * 32/64-bit mismatch errors -> download the raylib build that matches your compiler.
  * "MOUSE_BUTTON_LEFT not declared" -> your raylib is older than 4.0; update it.

---------------------------------------------------------------------
 3. CONTROLS AND GAMEPLAY
---------------------------------------------------------------------
  W A S D            move
  Mouse              aim
  Left click         shoot (rifle is automatic: hold the button)
  R                  reload (an empty magazine reloads by itself)
  1 2 3 4 / wheel    Pistol / Assault Rifle / Shotgun / Sniper Rifle
  E (hold)           defuse the bomb (stand within the red circle, you cannot
                     move or shoot while defusing; letting go resets progress)
  P or ESC           pause menu
  Menus              UP/DOWN + ENTER, or the mouse

How a round works
  * At the start the HUD tells you which site the terrorists are heading for.
  * One terrorist (marked "C4") walks to that site and plants the bomb
    (3 second plant timer). If he dies, the nearest terrorist takes over.
  * Once planted, the bomb explodes in 40 seconds. Hold E next to it for
    5 seconds to defuse. The other terrorists guard the bomb.
  * You WIN the round by killing everyone before the bomb is planted, or by
    defusing it. You LOSE if you die or the bomb explodes.
  * Health and ammo reset every round. Each round adds one more terrorist
    and makes them tougher, faster and more accurate. Survive round 5 = VICTORY.
  * Terrorists behind walls are hidden (line of sight), like in the real game.

Enemy types
  Terrorist (red)  : medium HP, walks towards you then strafes, steady fire
  Rusher (orange)  : fast, rushes you and fires very often, low damage
  Sniper (purple)  : fragile, keeps its distance, slow powerful bullet and a
                     red laser warning line just before it shoots

Weapons
  Pistol         22 dmg   0.30s  12 rounds  1.2s reload   accurate, semi-auto
  Assault Rifle  14 dmg   0.10s  30 rounds  2.0s reload   automatic, recoil makes
                                                          sustained fire less accurate
  Shotgun        13 x 8   0.85s   6 rounds  2.2s reload   8 pellets, short range
  Sniper Rifle  100 dmg   1.40s   5 rounds  2.8s reload   very fast bullet, 1 hit kills most

---------------------------------------------------------------------
 4. CLASS DIAGRAM  (inheritance = arrow with triangle,  composition = <>--)
---------------------------------------------------------------------

                          +------------------+
                          |   Character      |  <<abstract>>
                          |------------------|
                          | # position,radius|
                          | # speed          |
                          | # health         |
                          |------------------|
                          | +update() = 0    |
                          | +draw()   = 0    |
                          | +takeDamage()    |
                          | +moveBy()        |
                          +---------^--------+
                  +-----------------+------------------+
                  |                                    |
        +---------+--------+                +----------+---------+
        |      Player      |                |  Enemy <<abstract>>|
        |------------------|                |--------------------|
        | -weapons         |                | #damage, #fireInt. |
        | -currentWeapon   |                | #detectRange ...   |
        |------------------|                |--------------------|
        | +update()        |                | +update() (AI)     |
        | +draw()          |                | +draw()            |
        | +takeDamage()    |                | #behave()   = 0    |
        +--------<>--------+                | #attack()   = 0    |
                 |  owns 4                  | +getTypeName() = 0 |
                 v                          +----------^---------+
        +------------------+                  +--------+---------+----------+
        | Weapon <<abstract>>|     +-----------+--+ +---+---------------+ +-+--------------+
        |------------------|      |NormalTerrorist| |AggressiveTerrorist| | SniperTerrorist|
        | -stats, -ammo    |      +---------------+ +-------------------+ +----------------+
        | +update() virtual|
        | +shoot()   = 0   |
        | +tryFire()       |
        +---------^--------+
      +-----------+-----------+-------------+
      |           |           |             |
   Pistol   AssaultRifle   Shotgun     SniperRifle


   COMPOSITION (Game HAS-A ...)
   +--------------------------------------------------------------+
   |                           Game                               |
   | <>-- Map           (walls, crates, sites, spawns)            |
   | <>-- Player        (unique_ptr)                              |
   | <>-- Enemy 0..*    (vector<unique_ptr<Enemy>>)  polymorphic  |
   | <>-- Bullet 0..*   (vector<Bullet>)                          |
   | <>-- Effect 0..*   (vector<Effect>)                          |
   | <>-- Bomb                                                    |
   +--------------------------------------------------------------+
   Player <>-- Weapon 4   (vector<unique_ptr<Weapon>>)

Mermaid version (paste into https://mermaid.live to get a picture):

classDiagram
  Character <|-- Player
  Character <|-- Enemy
  Enemy <|-- NormalTerrorist
  Enemy <|-- AggressiveTerrorist
  Enemy <|-- SniperTerrorist
  Weapon <|-- Pistol
  Weapon <|-- AssaultRifle
  Weapon <|-- Shotgun
  Weapon <|-- SniperRifle
  Game *-- Map
  Game *-- Player
  Game *-- Enemy
  Game *-- Bullet
  Game *-- Effect
  Game *-- Bomb
  Player *-- Weapon
  class Character{
    <<abstract>>
    +update()*
    +draw()*
    +takeDamage()
  }
  class Enemy{
    <<abstract>>
    #behave()*
    #attack()*
    +getTypeName()*
  }
  class Weapon{
    <<abstract>>
    +shoot()*
    +update()
    +tryFire()
  }

---------------------------------------------------------------------
 5. OOP CONCEPTS USED - WHERE AND WHY
---------------------------------------------------------------------
INHERITANCE
  Player and Enemy inherit from Character (position, health, radius,
  moveBy with wall collision, takeDamage). The three terrorists inherit
  from Enemy (shared AI: detect, patrol, plant). The four guns inherit
  from Weapon (ammo, reload, fire-rate logic). Shared code is written
  once in the base class.

RUNTIME POLYMORPHISM (virtual functions + base-class pointers)
  * Game keeps vector<unique_ptr<Enemy>>. The loop calls
      enemies[i]->update(...)  and  enemies[i]->draw()
    and C++ chooses NormalTerrorist / AggressiveTerrorist / SniperTerrorist
    code at runtime.
  * Inside Enemy::update the virtual calls behave() and attack() make each
    terrorist move and shoot differently (Template Method pattern).
  * Player keeps vector<unique_ptr<Weapon>>. Weapon::tryFire() calls the
    virtual shoot(): Pistol fires 1 bullet, Shotgun fires 8 pellets, the
    AssaultRifle adds recoil, the Sniper fires one fast bullet.
  * Overrides: update(), draw(), takeDamage() (Player), shoot(), behave(),
    attack(), getTypeName(). AssaultRifle::update() overrides and ALSO calls
    Weapon::update().
  * Every base class has a virtual destructor so deleting through a base
    pointer is safe.

ENCAPSULATION
  Data members are private or protected; outside code uses public
  getters/setters/behaviour functions, e.g. takeDamage() instead of
  changing health directly, Weapon::tryFire() instead of editing ammo,
  Bomb::plant()/defuse() instead of changing its state. Bomb and Weapon
  keep their state private and enforce their own rules.

ABSTRACTION
  Character, Enemy and Weapon are abstract classes (pure virtual
  functions). You cannot create "a Weapon" or "an Enemy", only a concrete
  one. The abstract class defines WHAT every weapon/enemy must do; the
  derived class defines HOW. Game only talks to the abstract interface.

COMPOSITION
  Game owns Map, Player, Enemies, Bullets, Effects and Bomb. Player owns
  its Weapons. Objects live and die with their owner. unique_ptr deletes
  them automatically (no manual delete, no memory leaks).

OTHER NOTES
  * No global variables. Constants are class enums or file-local consts.
  * Bullets use a swept hit test so fast bullets never skip over enemies.
  * Line of sight uses segment-vs-rectangle tests against walls/crates.
  * Written in C++11: unique_ptr, override, enum class, lambdas.
