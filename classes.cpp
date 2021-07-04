#include "classes.h"

const short int STOP   = -1;
const short int BASIC  =  0;
const short int SAGA   =  1;
const short int TEST   =  2;
const short int CLEAR  =  3;
const short int NEW    =  4;
const short int ATTACK =  5;
const short int DEFEND =  6;

rune_stone::rune_stone(){
    srand(rand()+time(NULL));
    szin = rand()%6;
    combo = 0;
    falling = false;
}

rune_stone::~rune_stone(){
}

palya::palya(){
    my_HP = 0;
    my_MAX_HP = 0;
    folyamat = TEST;
    combo_num = 0;
    combo_tmp = 0;
    for(int i=0; i<5; i++)
    {
        AttackAll[i] = false;
    }
    my_heal=0;
    target = -1;
}

palya::~palya(){
}

void palya::SetElemek_helye(int windowH, int z,int FallingCycle){
    for(int i=0; i<5 ;i++)
        for(int j=0; j<6 ;j++)
            if(elemek[i][j].falling)
                elemek_helye[i][j] = {j*3*z, windowH-15*z+i*3*z-FallingCycle, 3*z, 3*z};
            else
                elemek_helye[i][j] = {j*3*z, windowH-15*z+i*3*z, 3*z, 3*z};
}

void palya::SetElemek_helye(int windowH, int z){
    SetElemek_helye(windowH, z, 0);
}

bool palya::csere(const int a, const int b){
    ///minta elemek[starta][startb].szin=elemek[starta][startb].szin  -/+  elemek[starta][startb].szin;
    if(a<starta)
    {
        ///csere felfele
        elemek[starta][startb].szin  =elemek[starta][startb].szin+elemek[starta-1][startb].szin;
        elemek[starta-1][startb].szin=elemek[starta][startb].szin-elemek[starta-1][startb].szin;
        elemek[starta][startb].szin  =elemek[starta][startb].szin-elemek[starta-1][startb].szin;
        starta--;
        return true;
    }
    if(b<startb)
    {
        ///csere balra
        elemek[starta][startb].szin  =elemek[starta][startb].szin+elemek[starta][startb-1].szin;
        elemek[starta][startb-1].szin=elemek[starta][startb].szin-elemek[starta][startb-1].szin;
        elemek[starta][startb].szin  =elemek[starta][startb].szin-elemek[starta][startb-1].szin;
        startb--;
        return true;
    }
    if(a>starta)
    {
        ///csere lefele
        elemek[starta][startb].szin  =elemek[starta][startb].szin+elemek[starta+1][startb].szin;
        elemek[starta+1][startb].szin=elemek[starta][startb].szin-elemek[starta+1][startb].szin;
        elemek[starta][startb].szin  =elemek[starta][startb].szin-elemek[starta+1][startb].szin;
        starta++;
        return true;
    }
    if(b>startb)
    {
        ///csere jobbra
        elemek[starta][startb].szin  =elemek[starta][startb].szin+elemek[starta][startb+1].szin;
        elemek[starta][startb+1].szin=elemek[starta][startb].szin-elemek[starta][startb+1].szin;
        elemek[starta][startb].szin  =elemek[starta][startb].szin-elemek[starta][startb+1].szin;
        startb++;
        return true;
    }
    return false;
}

void palya::FillForCombo(const int i,const int j){
    elemek[i][j].combo=combo_tmp;

    ///     ^
    ///     |   fel
    ///     |
    if(i>0 and elemek[i-1][j].combo==0 and istorlendo_fuggoleges(i-1,j) and elemek[i-1][j].szin==elemek[i][j].szin)
        this->FillForCombo(i-1,j);

    ///
    ///     <-- balra
    ///
    if(j>0 and elemek[i][j-1].combo==0 and istorlendo_vizszintes(i,j-1) and elemek[i][j-1].szin==elemek[i][j].szin)
        this->FillForCombo(i,j-1);

    ///     |
    ///     |   le
    ///     !
    if(i<4 and elemek[i+1][j].combo==0 and istorlendo_fuggoleges(i+1,j) and elemek[i+1][j].szin==elemek[i][j].szin)
        this->FillForCombo(i+1,j);

    ///
    ///     --> jobbra
    ///
    if(j<5 and elemek[i][j+1].combo==0 and istorlendo_vizszintes(i,j+1) and elemek[i][j+1].szin==elemek[i][j].szin)
        this->FillForCombo(i,j+1);
}

bool palya::istorlendo(const int i,const int j){
    if
    (
        (elemek[i][j].szin==elemek[i][j+1].szin and elemek[i][j].szin==elemek[i][j+2].szin and
         0<=i and i<5 and 0<=j and j<4 and elemek[i][j].szin!=-1) or
        (elemek[i][j].szin==elemek[i][j-1].szin and elemek[i][j].szin==elemek[i][j+1].szin and
         0<=i and i<5 and 0<j  and j<5 and elemek[i][j].szin!=-1) or
        (elemek[i][j].szin==elemek[i][j-1].szin and elemek[i][j].szin==elemek[i][j-2].szin and
         0<=i and i<5 and 1<j  and j<6 and elemek[i][j].szin!=-1) or
        (elemek[i][j].szin==elemek[i+1][j].szin and elemek[i][j].szin==elemek[i+2][j].szin and
         0<=i and i<3 and 0<=j and j<6 and elemek[i][j].szin!=-1) or
        (elemek[i][j].szin==elemek[i-1][j].szin and elemek[i][j].szin==elemek[i+1][j].szin and
         0<i  and i<4 and 0<=j and j<6 and elemek[i][j].szin!=-1) or
        (elemek[i][j].szin==elemek[i-1][j].szin and elemek[i][j].szin==elemek[i-2][j].szin and
         1<i  and i<5 and 0<=j and j<6 and elemek[i][j].szin!=-1)
    )
        return true;
    else
        return false;
}

bool palya::istorlendo_fuggoleges(const int i,const int j){
    if(
    (elemek[i][j].szin==elemek[i+1][j].szin and elemek[i][j].szin==elemek[i+2][j].szin and
    0<=i and i<3 and 0<=j and j<6 and elemek[i][j].szin!=-1) or
    (elemek[i][j].szin==elemek[i-1][j].szin and elemek[i][j].szin==elemek[i+1][j].szin and
    0<i  and i<4 and 0<=j and j<6 and elemek[i][j].szin!=-1) or
    (elemek[i][j].szin==elemek[i-1][j].szin and elemek[i][j].szin==elemek[i-2][j].szin and
    1<i  and i<5 and 0<=j and j<6 and elemek[i][j].szin!=-1))
        return true;
    else
        return false;
}

bool palya::istorlendo_vizszintes(const int i,const int j){
    if(
    (elemek[i][j].szin==elemek[i][j+1].szin and elemek[i][j].szin==elemek[i][j+2].szin and
    0<=i and i<5 and 0<=j and j<4 and elemek[i][j].szin!=-1) or
    (elemek[i][j].szin==elemek[i][j-1].szin and elemek[i][j].szin==elemek[i][j+1].szin and
    0<=i and i<5 and 0<j  and j<5 and elemek[i][j].szin!=-1) or
    (elemek[i][j].szin==elemek[i][j-1].szin and elemek[i][j].szin==elemek[i][j-2].szin and
    0<=i and i<5 and 1<j  and j<6 and elemek[i][j].szin!=-1))
        return true;
    else
        return false;
}

monster::monster(){
    srand(rand()+time(NULL));
    color = rand()%5;
    ///color = -1; majd :P
    this->SetMonster_FromColor();
}

monster::~monster(){
}

void monster::SetMonster_FromColor(){
    switch(color)
    {
    case 0:///tuz
        MAX_HP = 1263;
        basic_dmg = 90;
        recovery = 104;
        break;
    case 1:///level
        MAX_HP = 1263;
        basic_dmg = 88;
        recovery = 106;
        break;
    case 2:///viz
        MAX_HP = 1263;
        basic_dmg = 87;
        recovery = 107;
        break;
    case 3:///ko
        MAX_HP = 1263;
        basic_dmg = 92;
        recovery = 102;
        break;
    case 4:///szel

        MAX_HP = 1263;
        basic_dmg = 85;
        recovery = 109;
        break;
    }
    HP = 1263;
    precalculated_dmg = 0;
}
