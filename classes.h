#ifndef classes
#define classes

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
//#include <graphics.h>
#include <time.h>
#include <ctime>
#include <vector>

#undef main


class rune_stone
{
public:
    short int szin;
    short int combo;
    bool falling;
    rune_stone();
    ~rune_stone();
};

class palya
{
public:
    rune_stone elemek[5][6];
    SDL_FRect elemek_helye[5][6];
    short int starta, startb;
    short int folyamat;
    short int combo_num;
    short int combo_tmp;
    short int my_heal;
    short int my_HP;
    short int my_MAX_HP;
    bool AttackAll[5];
    short int target;
    void SetElemek_helye(int windowH, int z, int FallinCycle);
    void SetElemek_helye(int windowH, int z);
    bool csere(const int a, const int b);
    void FillForCombo(const int i,const int j);
    bool istorlendo(const int,const int);
    bool istorlendo_vizszintes(const int,const int);
    bool istorlendo_fuggoleges(const int,const int);
    palya();
    ~palya();
};

class monster
{
public:
    short int color;///a szine
    short int HP;///elet
    short int MAX_HP;///az eletcsikok szamitasahoz
    short int basic_dmg;///alap tamado ertek
    short int precalculated_dmg;///eloreszamitott dmg (erre meg rajon a elemental es combo bonusz)
    short int recovery;///visszatoltesi ertek
    ///short int M_type; ///coming soon...
    void SetMonster_FromColor();///bealit egy tipusu szornyet a szine szerint;
    monster();
    ~monster();
};

#endif
