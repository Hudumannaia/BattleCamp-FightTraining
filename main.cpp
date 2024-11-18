#include "classes.h"

using namespace std;

const int FPS = 60;  /// megkozelitoleg a jatek fps-se

const short int STOP = -1;
const short int BASIC = 0;
const short int SAGA = 1;
const short int TEST = 2;
const short int CLEAR = 3;
const short int NEW = 4;
const short int ATTACK = 5;
const short int DEFEND = 6;

/// 0-tuz; 1-level; 2-viz; 3-ko; 4-szel; 5-regen
const float elemental_bonus[5][5] = {{1.0, 0.75, 1.33, 1.0, 1.0},
									 {1.33, 1.0, 0.75, 1.0, 1.0},
									 {0.75, 1.33, 1.0, 1.0, 1.0},
									 {1.0, 1.0, 1.0, 1.0, 1.33},
									 {1.0, 1.0, 1.0, 1.33, 1.0}};
/// egy aranylag gyors hullas (kisebb szam gyorsabb eses - egy negyzet (3*z) eses ideje ms-ba kifejezve)
const short int FallingTime = 200;
/// A gemek gyujtogetesere allo ido ms-ban kifejezve
const short int SagaTime = 5000;  /// 5000 az eredeti (5s)

bool rect_inside(SDL_FRect, int, int);

int main() {

    bool res = SDL_Init (SDL_INIT_VIDEO);
    if (!res) { return 666; }

	palya mezo;  /// 1 kell belole valoszinuleg
	/// mindenfele valtozo letrehozasa
	int z;                                        /// a hires egyseg :D
	int windowW, windowH;                         /// az ablak meretei
	int fps_counter = clock();                    /// counter az FPS-hez
	int x = 0, y = 0, a = 0, b = 0;               /// az eger helyenek a meghatarozasat szolgaljak
	int saga_timer = 0, falling_timer = clock();  /// timerekhez kell
	short int FallingCycle = 0;                   /// az eses szimulalasahoz kell
	bool running = true;                          /// a program vegeig true
	monster Allys[5];
	for (int i = 0; i < 5; i++) {  /// a szornyek bealitasa es egyuttal az "en" MAX_HP-at (mezo.my_MAX_HP)
		Allys[i].color = i;
		Allys[i].SetMonster_FromColor();
		mezo.my_MAX_HP += Allys[i].MAX_HP;
	}
	monster Enemys[5];
	mezo.my_HP = mezo.my_MAX_HP;  /// a pillanatnyi HP a maxon kell legyen
	{

		const SDL_DisplayMode* displayMode = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
		if (displayMode == nullptr) {
			int tmp = SDL_GetPrimaryDisplay();
			SDL_Quit();
			return tmp+1;
			z = 1032/32-2;
		} else {
		    z = displayMode->h/32-2;
		}

		// z = getmaxheight()/32-2; ///kiirtando headerbol
	}
	windowH = 32 * z;
	windowW = 18 * z;
	SDL_Surface* Loader_Surface;       /// 1 kell belole
	SDL_Event event;                   /// 1 kell belole
	SDL_Texture* palya_hatter;         /// a palya hatterenek kepe
	SDL_Texture* full_hatter;          /// a szornyek mogotti ter kepe
	SDL_Texture* runes_textures[6];    /// az elemek kepei
	SDL_Texture* Allies_textures[5];   /// a lenyek texturai
	SDL_Texture* Enemy_textures[5];    /// a lenyek texturai
	SDL_Texture* full_healtbar;        /// a healbar kepei
	SDL_Texture* full_timer;           /// a timer kepei
	SDL_Texture* Monster_healtbar[5];  /// a szornyek healtbar kepei
	/// A rect felepitese: {x, y, w, h}
	SDL_FRect palya_hatter_helye = {0, windowH - 15 * z, 18 * z, 15 * z};                               /// hatter elrendezes
	SDL_FRect healtbar_start = {0, 0, 10, 25};                                                          /// a healtbar eleje
	SDL_FRect healtbar_mid = {9, 0, 2, 25};                                                             /// a healtbar maga (az adat)
	SDL_FRect healtbar_end = {10, 0, 10, 25};                                                           /// a healtbar vege
	SDL_FRect timer_start = {0, 0, 10, 25};                                                             /// a timer eleje
	SDL_FRect timer_mid = {9, 0, 2, 25};                                                                /// a timer maga (az ido...)
	SDL_FRect timer_end = {10, 0, 10, 25};                                                              /// a timer vege
	SDL_FRect healtbar_start_helye = {0, 16 * z, z, z};                                                 /// a healtbar elejenek a helye
	SDL_FRect healtbar_mid_helye = {z, 16 * z, 16 * z * mezo.my_HP / mezo.my_MAX_HP, z};                /// a healtbar kozepenek a helye
	SDL_FRect healtbar_end_helye = {17 * z * mezo.my_HP / mezo.my_MAX_HP, 16 * z, z, z};                /// a healtbar vegenek a helye
	SDL_FRect timer_start_helye = {0, 16 * z, z, z};                                                    /// a timer elejenek a helye
	SDL_FRect timer_mid_helye = {z, 16 * z, 16 * z * (SagaTime - clock() + saga_timer) / SagaTime, z};  /// a timer kozepenek a helye
	SDL_FRect timer_end_helye = {17 * z * (SagaTime - clock() + saga_timer) / SagaTime, 16 * z, z, z};  /// a timer vegenek a helye
	SDL_FRect Ally_monster_helye[5];
	SDL_FRect Enemy_monster_helye[5];
	SDL_FRect Enemy_monster_healtbar_start[5];
	SDL_FRect Enemy_monster_healtbar_mid[5];
	SDL_FRect Enemy_monster_healtbar_end[5];

	/// szornyek es az enemy healtbarjanak a helyenek a bealitasa :D
	for (int i = 0; i < 5; i++) {
		Ally_monster_helye[i] = {i * 18 * z / 5, 12 * z, 18 * z / 5, 4 * z};
		Enemy_monster_helye[i] = {i * 18 * z / 5, 6 * z + (i % 2) * z, 18 * z / 5, 5 * z};
		Enemy_monster_healtbar_start[i] = {i * 7 * windowW / 35, 10 * z + (i % 2) * z, windowW / 35, z};
		Enemy_monster_healtbar_mid[i] = {i * 7 * windowW / 35 + windowW / 35, 10 * z + (i % 2) * z, 5 * windowW * Enemys[i].HP / Enemys[i].MAX_HP / 35, z};
		Enemy_monster_healtbar_end[i] = {i * 7 * windowW / 35 + 6 * windowW * Enemys[i].HP / Enemys[i].MAX_HP / 35, 10 * z + (i % 2) * z, windowW / 35, z};
	}

	mezo.SetElemek_helye(windowH, z);  /// elemek elrendezes

	/// Az ablak es a renderer letrehozasa
	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Battle Camp - fight training");
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, windowW);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, windowH);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);
	SDL_Window* window = SDL_CreateWindowWithProperties(props);
	SDL_ClearError();  /// a SDL_CreateWindow-val van baja...
	SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr); //sdl2bol ez lehet kene SDL_RENDERER_ACCELERATED
	SDL_SetWindowMinimumSize(window, 180, 320);  /// minimum ablakmeret bealitasa
	SDL_ShowWindow (window);

	/// a kepek a betoltese a texturakba es hibakezeles
	vector<char*> hibak;
	while (true) {
		Loader_Surface = IMG_Load("healtbar color 0.png");
		if (Loader_Surface)
			Monster_healtbar[0] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'healtbar color 0.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("healtbar color 1.png");
		if (Loader_Surface)
			Monster_healtbar[1] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'healtbar color 1.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("healtbar color 2.png");
		if (Loader_Surface)
			Monster_healtbar[2] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'healtbar color 2.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("healtbar color 3.png");
		if (Loader_Surface)
			Monster_healtbar[3] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'healtbar color 3.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("healtbar color 4.png");
		if (Loader_Surface)
			Monster_healtbar[4] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'healtbar color 4.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("Lavajaw(enemy0).png");
		if (Loader_Surface)
			Enemy_textures[0] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'Lavajaw(enemy0).png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("Underbrush(enemy1).png");
		if (Loader_Surface)
			Enemy_textures[1] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'Underbrush(enemy1).png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("Tankfish(enemy2).png");
		if (Loader_Surface)
			Enemy_textures[2] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'Tankfish(enemy2).png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("Craggy(enemy3).png");
		if (Loader_Surface)
			Enemy_textures[3] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'healtbar color 0.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("Puffbelly(enemy4).png");
		if (Loader_Surface)
			Enemy_textures[4] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'Puffbelly(enemy4).png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("Lavajaw(allay0).png");
		if (Loader_Surface)
			Allies_textures[0] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'Lavajaw(allay0).png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("Underbrush(allay1).png");
		if (Loader_Surface)
			Allies_textures[1] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'Underbrush(allay1).png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("Tankfish(allay2).png");
		if (Loader_Surface)
			Allies_textures[2] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'Tankfish(allay2).png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("Craggy(allay3).png");
		if (Loader_Surface)
			Allies_textures[3] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'Craggy(allay3).png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("Puffbelly(allay4).png");
		if (Loader_Surface)
			Allies_textures[4] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'Puffbelly(m4).png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("fullhatter.png");
		if (Loader_Surface)
			full_hatter = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'fullhatter.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("healtbar.png");
		if (Loader_Surface)
			full_healtbar = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'healtbar.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("timer.png");
		if (Loader_Surface)
			full_timer = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'timer.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("hatter.png");
		if (Loader_Surface)
			palya_hatter = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'hatter.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("element_0.png");
		if (Loader_Surface)
			runes_textures[0] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'element_0.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("element_1.png");
		if (Loader_Surface)
			runes_textures[1] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'element_1.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("element_2.png");
		if (Loader_Surface)
			runes_textures[2] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'element_2.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("element_3.png");
		if (Loader_Surface)
			runes_textures[3] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'element_3' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("element_4.png");
		if (Loader_Surface)
			runes_textures[4] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'element_4.png' is missing.");
			hibak.push_back(hiba);
		}

		Loader_Surface = IMG_Load("element_5.png");
		if (Loader_Surface)
			runes_textures[5] = SDL_CreateTextureFromSurface(renderer, Loader_Surface);
		else {
			char* hiba = new char[40];
			strcpy(hiba, "'element_5.png' is missing.");
			hibak.push_back(hiba);
		}
		if (hibak.size()) {
			SDL_Delay (500);
			/*initwindow(500, 100 + 16 * hibak.size(), "Hiba!");
			settextjustify(1, 1);
			settextstyle(0, 0, 2);
			setfillstyle(1, WHITE);
			int i = 0, x, y, baleger = false, trying, click = false;
			*/
			while (hibak.size()) {
				//outtextxy(getmaxx() / 2, 40 + 16 * i++, hibak.back());
				delete[] hibak.back();
				hibak.pop_back();
			}
			/*bar(getmaxx() / 2 - 3, getmaxy() - 15, getmaxx() / 2 + 3, getmaxy());  /// fuggoleges vonal
			bar(0, getmaxy() - 17, getmaxx(), getmaxy() - 15);                     /// vizszintes vonal
			outtextxy(getmaxx() / 2, getmaxy() - 25, "Do you want to try again loading?");
			outtextxy(getmaxx() / 4, getmaxy() - 3, "YES");
			outtextxy(getmaxx() * 3 / 4, getmaxy() - 3, "NO");
			while (!click) {
				if (ismouseclick(WM_MOUSEMOVE)) { getmouseclick(WM_MOUSEMOVE, x, y); }
				if (baleger == false and ismouseclick(WM_LBUTTONDOWN)) {
					baleger = true;
					clearmouseclick(WM_LBUTTONDOWN);
				}
				if (baleger == true and ismouseclick(WM_LBUTTONUP)) {
					baleger = false;
					clearmouseclick(WM_LBUTTONUP);
				}
				if (baleger and x < getmaxx() / 2 - 3 and y > getmaxy() - 10) {
					trying = true;
					click = true;
				}
				if (baleger and x > getmaxx() / 2 + 3 and y > getmaxy() - 10) {
					trying = false;
					click = true;
				}
			}
			closegraph();
			if (!trying) {
				break;  /// kilep a programbol gondolom XD
			}
			*/
		} else  /// nincs hiba
		{
			break;
		}
	}
	/// jatszhato mezo generalasa :D
	while (mezo.folyamat != BASIC) {
		switch (mezo.folyamat) {
			case TEST:
				for (int i = 0; i < 5; i++)
					for (int j = 0; j < 6; j++)
						if (mezo.istorlendo(i, j)) {
							mezo.folyamat = CLEAR;  /// talalt torlendot, jon a CLEAR folyamat
							mezo.elemek[i][j].combo = 1;
						}
				if (mezo.folyamat == TEST)  /// a kereses sikertelen
					mezo.folyamat = BASIC;  /// a palya startra kesz
				break;
			case CLEAR:
				for (int i = 0; i < 5; i++)
					for (int j = 0; j < 6; j++)
						if (mezo.elemek[i][j].combo > 0) {
							mezo.elemek[i][j].szin = -1;
							mezo.elemek[i][j].combo = 0;
						}
				mezo.folyamat = NEW;
				break;
			case NEW:
				mezo.folyamat = TEST;
				for (int i = 4; i >= 0; i--)
					for (int j = 0; j < 6; j++)
						if (mezo.elemek[i][j].szin == -1) {
							mezo.folyamat = NEW;  /// folyamatosan esik...
							if (i == 0)
								mezo.elemek[i][j].szin = rand() % 6;
							else {
								mezo.elemek[i][j].szin = mezo.elemek[i - 1][j].szin;
								mezo.elemek[i - 1][j].szin = -1;
							}
						}
				break;
		}
	}

	while (running)  /// a program magja
	{
		while (SDL_PollEvent(&event))  /// event kezeles
			switch (event.type) {
				case SDL_EVENT_QUIT: {  /// kilepes
					running = false;
					break;
				}
				case SDL_EVENT_WINDOW_RESIZED: {  /// ablak atmeretezes

						SDL_GetWindowSize(window, &windowW, &windowH);
						z = windowH / 32;
						windowH = 32 * z;
						windowW = 18 * z;
						palya_hatter_helye = {0, windowH - 15 * z, 18 * z, 15 * z};
						mezo.SetElemek_helye(windowH, z);
						SDL_SetWindowSize(window, windowW, windowH);
						healtbar_start_helye = {0, 16 * z, z, z};                                   /// a healtbar elejenek a helyere rakaja
						healtbar_mid_helye = {z, 16 * z, 16 * z * mezo.my_HP / mezo.my_MAX_HP, z};  /// a healtbar kozepenek a helyere
																									/// rakaja
						healtbar_end_helye = {17 * z * mezo.my_HP / mezo.my_MAX_HP, 16 * z, z, z};  /// a healtbar vegenek a helyere rakaja
						timer_start_helye = {0, 16 * z, z, z};  /// a timer elejenek a helye  NINCS NAGYON SZUKSEG RA
						timer_mid_helye = {z, 16 * z, 16 * z * (SagaTime - clock() + saga_timer) / SagaTime, z};  /// a timer kozepenek a
																												  /// helye NINCS NAGYON
																												  /// SZUKSEG RA
						timer_end_helye = {17 * z * (SagaTime - clock() + saga_timer) / SagaTime, 16 * z, z, z};  /// a timer vegenek a
																												  /// helye   NINCS NAGYON
																												  /// SZUKSEG RA
						for (int i = 0; i < 5; i++) {
							Ally_monster_helye[i] = {i * 18 * z / 5, 12 * z, 18 * z / 5, 4 * z};  /// baratsagos szornyek helyenek a
																								  /// bealitasa
							Enemy_monster_helye[i] = {i * 18 * z / 5, 6 * z + (i % 2) * z, 18 * z / 5, 5 * z};  /// ellenseges szornyek
																												/// helyenek a bealitasa
							Enemy_monster_healtbar_start[i] = {i * 7 * windowW / 35, 10 * z + (i % 2) * z, windowW / 35, z};
							Enemy_monster_healtbar_mid[i] = {i * 7 * windowW / 35 + windowW / 35,
															 10 * z + (i % 2) * z,
															 5 * windowW * Enemys[i].HP / Enemys[i].MAX_HP / 35,
															 z};
							Enemy_monster_healtbar_end[i] = {i * 7 * windowW / 35 + 6 * windowW * Enemys[i].HP / Enemys[i].MAX_HP / 35,
															 10 * z + (i % 2) * z,
															 windowW / 35,
															 z};
						}

					break;
				}
				case SDL_EVENT_MOUSE_MOTION: {  /// egermozgas
					if (mezo.folyamat == BASIC or mezo.folyamat == SAGA) {
						float tmpx, tmpy;
						SDL_GetMouseState(&tmpx, &tmpy);
						x = (int)SDL_roundf(tmpx);
						y = (int)SDL_roundf(tmpy);
						a = (y - 17 * z) / (3 * z);
						if (a < 0) a = 0;
						if (a > 4) a = 4;
						b = x / (3 * z);
						if (event.button.button == SDL_BUTTON_LEFT and mezo.folyamat == SAGA) {
							if (mezo.csere(a, b) and saga_timer == 0)  /// elemek csereje es egyeb :D
								saga_timer = clock();
							mezo.SetElemek_helye(windowH, z);  /// kell, hogy a cserelt elemeket a helyukre rakja
							/// mozgatott elem pontos helye
							if (x > z + z / 2 and x < 16 * z + z / 2) mezo.elemek_helye[a][b].x = x - z - z / 2;
							if (y > 18 * z + z / 2 and y < 30 * z + z / 2) mezo.elemek_helye[a][b].y = y - z - z / 2;
						}
					}
					break;
				}
				case SDL_EVENT_MOUSE_BUTTON_DOWN: {  /// katintas
					if (mezo.folyamat == BASIC) {
						bool target_change = false;
						for (int i = 0; i < 5; i++) {
							/// Enemy_monster_helye[i] = { i*18*z/5, 6*z+(i%2)*z, 18*z/5, 5*z };
							if (rect_inside(Enemy_monster_helye[i], x, y) and Enemys[i].HP)  /// ha ez egyik szornyre (es el...) kattint
																							 /// akkor az targeteli :)
							{
								mezo.target = i;
								target_change = true;
								break;  /// nem muszaly
							}
						}
						if (!target_change)
							if (event.button.button == SDL_BUTTON_LEFT) {
								/// SAGA kezdese
								mezo.starta = a;
								mezo.startb = b;
								mezo.folyamat = SAGA;
								saga_timer = 0;  /// nem biztos hogy kell...??? o.O
							}
					}
					break;
				}
				case SDL_EVENT_MOUSE_BUTTON_UP: {  ///
					if (event.button.button == SDL_BUTTON_LEFT and mezo.folyamat == SAGA) {
						if (saga_timer == 0) {
							mezo.folyamat = BASIC;
							mezo.SetElemek_helye(windowH, z);
						} else {
							mezo.folyamat = TEST;
							saga_timer = 0;
						}
					} else {
					}
					break;
				}
			}
		switch (mezo.folyamat) {  /// A jatek folyamatait szabalyozo resz
			default: {
				//(szinte lehetetlen)
				break;
			}
			case BASIC: {
				break;
			}
			case SAGA: {
				if (clock() - saga_timer > SagaTime and saga_timer != 0) {
					mezo.folyamat = TEST;
					saga_timer = 0;
				}
				break;
			}
			case TEST: {
				mezo.combo_tmp = 0;
				mezo.SetElemek_helye(windowH, z);  /// mozgatott elem helyrerakasa
				for (int i = 0; i < 5; i++)
					for (int j = 0; j < 6; j++)
						if (mezo.istorlendo(i, j) and mezo.elemek[i][j].combo == 0) {
							mezo.folyamat = CLEAR;  /// talalt torlendot, jon a CLEAR folyamat
							mezo.combo_tmp++;
							mezo.FillForCombo(i, j);
						}
				mezo.combo_num += mezo.combo_tmp;
				for (int c = 1; c <= mezo.combo_tmp; c++) {
					int counter = 2;
					int szin;
					for (int i = 0; i < 5; i++)
						for (int j = 0; j < 6; j++)
							if (mezo.elemek[i][j].combo == c) {
								counter++;                      /// egy csoport elemeinek szamanak szamolasa
								szin = mezo.elemek[i][j].szin;  /// szin megjegyzese
							}
					for (int i = 0; i < 5; i++)      /// minden host vegigveszunk
						if (Allys[i].color == szin)  /// es az adott elementhez tartozo monster damaget
							Allys[i].precalculated_dmg += Allys[i].basic_dmg * 0.2 * counter;           /// megvaltoztatjuk
					if (szin == 5)                                                                      /// szivecske
						for (int i = 0; i < 5; i++) mezo.my_heal += Allys[i].recovery * 0.2 * counter;  /// a healeles kiszamolasa
					else if (counter >= 7)            /// ha a csoport tobbmint 5 gembol all
						mezo.AttackAll[szin] = true;  /// mindenkit tamad az a szin
				}
				if (mezo.folyamat == TEST)   /// a kereses sikertelen
					mezo.folyamat = ATTACK;  /// igy atlep a tamadasra
				break;
			}
			case CLEAR: {
				/// cout << "Folyamat: CLEAR";
				for (int i = 0; i < 5; i++)
					for (int j = 0; j < 6; j++)
						if (mezo.elemek[i][j].combo > 0) {
							mezo.elemek[i][j].szin = -1;
							mezo.elemek[i][j].combo = 0;
							mezo.elemek[i][j].falling = true;
							/// mezo.frissites=true;
							/// mezo.falling=true;
						}
				mezo.folyamat = NEW;
				break;
			}
			case NEW: {
				/// cout << "Folyamat: NEW";
				if (FallingCycle == 0) {
					mezo.folyamat = TEST;
					for (int i = 4; i >= 0; i--)
						for (int j = 0; j < 6; j++) {
							if (i < 4 and mezo.elemek[i + 1][j].falling == true)
								mezo.elemek[i][j].falling = true;  /// az elemnek esnie kell
							else
								mezo.elemek[i][j].falling = false;
							if (mezo.elemek[i][j].szin == -1)  /// ures mezo
							{
								mezo.folyamat = NEW;      /// folyamatosan esik...
								falling_timer = clock();  /// esesi timer ujrainditasa
								FallingCycle = 3 * z;     /// esesi ciklus ujrainditasa
								if (i == 0)
									mezo.elemek[i][j].szin = rand() % 6;

								else {
									mezo.elemek[i][j].szin = mezo.elemek[i - 1][j].szin;
									mezo.elemek[i - 1][j].szin = -1;
								}
								mezo.elemek[i][j].falling = true;
							}
						}
				} else if (clock() - falling_timer > FallingTime / 3 / z) {
					FallingCycle--;
					falling_timer = clock();
				}
				mezo.SetElemek_helye(windowH, z, FallingCycle);
				break;
			}
			case ATTACK: {
				float ComboBonus = 0.8;
				ComboBonus += mezo.combo_num * 0.2;  /// hozzaadom a combozasbol szarmazo bonuszt
				for (int i = 0; i < 5; i++)          /// minden szornyunket sorra vesszuk
				{
					bool random_target = false;
					if (mezo.target < 0)  /// nincs kivalaszott celpont
					{
						short int tmp = rand() % 5;  /// randomizalunk
						while (Enemys[tmp].HP == 0)  /// egy celpontot
						{
							tmp = rand() % 5;
						}
						mezo.target = tmp;
						random_target = true;
					}
					if (Allys[i].HP > 0 and Allys[i].precalculated_dmg > 0)  /// tamad amelyik kell
					{
						if (mezo.AttackAll[i])  /// mindenkit kell tamadjon
						{
							for (int j = 0; j < 5; j++) {
								if (Enemys[j].HP > 0) {
									Enemys[j].HP -=
										(int)(Allys[i].precalculated_dmg * ComboBonus * elemental_bonus[Enemys[j].color][Allys[i].color]);
									if (Enemys[j].HP < 0) Enemys[j].HP = 0;
								}
							}
						} else {
							/// az kivalasztott ellenfel eletebol levonunk a:  bonus[][] * ComboBonusz * gyujtott elem a szinben * szorny
							/// alap DMG / 3
							Enemys[mezo.target].HP -=
								(int)(Allys[i].precalculated_dmg * ComboBonus * elemental_bonus[Enemys[mezo.target].color][Allys[i].color]);
							if (Enemys[mezo.target].HP < 0) Enemys[mezo.target].HP = 0;
						}
						if (Enemys[mezo.target].HP == 0)  /// halott-e a tamadott ellenfel?
							mezo.target = -1;
					}
					if (random_target) mezo.target = -1;  /// ha igen uj ellenfelet tamadunk

					for (int j = 0; j < 5; j++) {
						if (Enemys[j].HP > 0)  /// van meg elo...
							break;
						if (j == 4) running = false;  /// nincs elo, kilep
					}
					if (!running)  /// ha nincs ez a sor akkor kifagy!!! -.-"
						break;
				}
				/// healelem magam
				if (mezo.my_heal) {
					mezo.my_HP += mezo.my_heal * ComboBonus;
					if (mezo.my_HP > mezo.my_MAX_HP) mezo.my_HP = mezo.my_MAX_HP;
					healtbar_start_helye = {0, 16 * z, z, z};                                   /// a healtbar elejenek a helyere rakaja
					healtbar_mid_helye = {z, 16 * z, 16 * z * mezo.my_HP / mezo.my_MAX_HP, z};  /// a healtbar kozepenek a helyere rakaja
					healtbar_end_helye = {17 * z * mezo.my_HP / mezo.my_MAX_HP, 16 * z, z, z};  /// a healtbar vegenek a helyere rakaja
					mezo.my_heal = 0;
				}
				mezo.combo_num = 0;
				mezo.target = -1;
				for (int i = 0; i < 5; i++) {
					/// Saga tulajdonsagainak torlese...
					mezo.AttackAll[i] = false;
					Allys[i].precalculated_dmg = 0;
					/// a szornyek haeltbarjanak
					Enemy_monster_healtbar_start[i] = {i * 7 * windowW / 35, 10 * z + (i % 2) * z, windowW / 35, z};
					Enemy_monster_healtbar_mid[i] = {i * 7 * windowW / 35 + windowW / 35,
													 10 * z + (i % 2) * z,
													 5 * windowW * Enemys[i].HP / Enemys[i].MAX_HP / 35,
													 z};
					Enemy_monster_healtbar_end[i] = {i * 7 * windowW / 35 + 6 * windowW * Enemys[i].HP / Enemys[i].MAX_HP / 35,
													 10 * z + (i % 2) * z,
													 windowW / 35,
													 z};
				}
				if (running)
					mezo.folyamat = DEFEND;
				else { mezo.folyamat = STOP; }
				break;
			}
			case DEFEND: {
				int tamadasok_szama = 0;
				for (int i = 0; i < 5; i++)
					if (Enemys[i].HP > 0) tamadasok_szama++;
				/// if(rand()%2)///marad 5 tamadas vagy valtozhat /// NEM SZERETNEM BELE
				{  /// tamadaszam randomizalas
					int ellenfelek_szama = tamadasok_szama;
					int random;
					while (true) {
						srand(rand() + clock());
						random = rand() % 3 - 1;  /// random szam ( -1 0 1 ) kozul
						tamadasok_szama += random;
						if (random == 0 or tamadasok_szama == 0 or tamadasok_szama == 2 * ellenfelek_szama) break;
					}
					if (tamadasok_szama > ellenfelek_szama + ellenfelek_szama)  /// ne tamadjanak tul sokat
						tamadasok_szama = 2 * ellenfelek_szama;
				}
				/// jon a tamadas
				for (int i = 1; i <= tamadasok_szama; i++) {
					int random = rand() % 5;
					while (Enemys[random].HP == 0) random = rand() % 5;
					mezo.my_HP -= Enemys[random].basic_dmg;
				}
				if (mezo.my_HP < 0) {
					mezo.my_HP = 0;
					mezo.folyamat = STOP;
					running = false;
				}
				healtbar_start_helye = {0, 16 * z, z, z};                                   /// a healtbar elejenek a helyere rakaja
				healtbar_mid_helye = {z, 16 * z, 16 * z * mezo.my_HP / mezo.my_MAX_HP, z};  /// a healtbar kozepenek a helyere rakaja
				healtbar_end_helye = {17 * z * mezo.my_HP / mezo.my_MAX_HP, 16 * z, z, z};  /// a healtbar vegenek a helyere rakaja
				mezo.folyamat = BASIC;
				break;
			}
		}
		if (clock() - fps_counter > 1000 / FPS) {  /// ez a "frissites"
			fps_counter = clock();
			SDL_RenderClear(renderer);  /// renderer torles...

			SDL_RenderTexture(renderer, palya_hatter, NULL, &palya_hatter_helye);  /// jatek hatter bevagas
			for (int i = 0; i < 5; i++)
				for (int j = 0; j < 6; j++)
					/// szines kovek bevagasa
					if (mezo.elemek[i][j].szin > -1 and mezo.elemek[i][j].szin < 6)
						SDL_RenderTexture(renderer, runes_textures[mezo.elemek[i][j].szin], NULL, &mezo.elemek_helye[i][j]);

			SDL_RenderTexture(renderer, full_hatter, NULL, NULL);  /// a keret hatter
			for (int i = 0; i < 5; i++) {
				SDL_RenderTexture(renderer, Allies_textures[Allys[i].color], NULL, &Ally_monster_helye[i]);  /// a "te" szornyeid berakasa
				if (Enemys[i].HP and running) {
					/// az ellenseg berakasa az eletevel egyutt :DD de kiraly, ez nehez volt!!
					SDL_RenderTexture(renderer, Enemy_textures[Enemys[i].color], NULL, &Enemy_monster_helye[i]);
					SDL_RenderTexture(renderer, Monster_healtbar[Enemys[i].color], &healtbar_start, &Enemy_monster_healtbar_start[i]);
					SDL_RenderTexture(renderer, Monster_healtbar[Enemys[i].color], &healtbar_mid, &Enemy_monster_healtbar_mid[i]);
					SDL_RenderTexture(renderer, Monster_healtbar[Enemys[i].color], &healtbar_end, &Enemy_monster_healtbar_end[i]);
				}
			}
			if (saga_timer) {
				timer_start_helye = {0, 16 * z, z, z};                                                    /// a timer elejenek a helye
				timer_mid_helye = {z, 16 * z, 16 * z * (SagaTime - clock() + saga_timer) / SagaTime, z};  /// a timer kozepenek a helye
				timer_end_helye = {17 * z * (SagaTime - clock() + saga_timer) / SagaTime, 16 * z, z, z};  /// a timer vegenek a helye
				SDL_RenderTexture(renderer, full_timer, &timer_start, &timer_start_helye);                   /// a timer elejenek beszurasa
				SDL_RenderTexture(renderer, full_timer, &timer_mid, &timer_mid_helye);                       /// a timer kozepenek beszurasa
				SDL_RenderTexture(renderer, full_timer, &timer_end, &timer_end_helye);                       /// a timer vegenek beszurasa
			} else {
				SDL_RenderTexture(renderer, full_healtbar, &healtbar_start, &healtbar_start_helye);  /// a healtbar elejenek beszurasa
				SDL_RenderTexture(renderer, full_healtbar, &healtbar_mid, &healtbar_mid_helye);      /// a healtbar kozepenek beszurasa
				SDL_RenderTexture(renderer, full_healtbar, &healtbar_end, &healtbar_end_helye);      /// a healtbar vegenek beszurasa
			}

			/// A mozgatott kovet megegyszer megjelenitjuk a tetejen :D
			if (event.button.button == SDL_BUTTON_LEFT and mezo.folyamat == SAGA and mezo.elemek[a][b].szin > -1 and mezo.elemek[a][b].szin < 6)
				SDL_RenderTexture(renderer, runes_textures[mezo.elemek[a][b].szin], NULL, &mezo.elemek_helye[a][b]);
			SDL_RenderPresent(renderer);  /// ujrakirajzolas
		}
	}

	{                               /// meg egyszer megjelenitem, hogy ne lepjen ki olyan hirtelen
		SDL_Delay(500);                 /// small w8
		SDL_RenderClear(renderer);  /// renderer torles...

		SDL_RenderTexture(renderer, palya_hatter, NULL, &palya_hatter_helye);  /// jatek hatter bevagas
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 6; j++)
				/// szines kovek bevagasa
				if (mezo.elemek[i][j].szin > -1 and mezo.elemek[i][j].szin < 6)
					SDL_RenderTexture(renderer, runes_textures[mezo.elemek[i][j].szin], NULL, &mezo.elemek_helye[i][j]);

		SDL_RenderTexture(renderer, full_hatter, NULL, NULL);  /// a keret hatter
		for (int i = 0; i < 5; i++) {
			SDL_RenderTexture(renderer, Allies_textures[Allys[i].color], NULL, &Ally_monster_helye[i]);  /// a "te" szornyeid berakasa
			if (Enemys[i].HP and running) {
				/// az ellenseg berakasa az eletevel egyutt :DD de kiraly, ez nehez volt!!
				SDL_RenderTexture(renderer, Enemy_textures[Enemys[i].color], NULL, &Enemy_monster_helye[i]);
				SDL_RenderTexture(renderer, Monster_healtbar[Enemys[i].color], &healtbar_start, &Enemy_monster_healtbar_start[i]);
				SDL_RenderTexture(renderer, Monster_healtbar[Enemys[i].color], &healtbar_mid, &Enemy_monster_healtbar_mid[i]);
				SDL_RenderTexture(renderer, Monster_healtbar[Enemys[i].color], &healtbar_end, &Enemy_monster_healtbar_end[i]);
			}
		}
		SDL_RenderTexture(renderer, full_healtbar, &healtbar_start, &healtbar_start_helye);  /// a healtbar elejenek beszurasa
		SDL_RenderTexture(renderer, full_healtbar, &healtbar_mid, &healtbar_mid_helye);      /// a healtbar kozepenek beszurasa
		SDL_RenderTexture(renderer, full_healtbar, &healtbar_end, &healtbar_end_helye);      /// a healtbar vegenek beszurasa

		/// A mozgatott kovet megegyszer megjelenitjuk a tetejen :D
		if (event.button.button == SDL_BUTTON_LEFT and mezo.folyamat == SAGA and mezo.elemek[a][b].szin > -1 and mezo.elemek[a][b].szin < 6)
			SDL_RenderTexture(renderer, runes_textures[mezo.elemek[a][b].szin], NULL, &mezo.elemek_helye[a][b]);
		SDL_RenderPresent(renderer);  /// ujrakirajzolas
	}
	SDL_Delay(3000);  /// end w8
}

bool rect_inside(SDL_FRect hatarok, int x, int y) {
	if (hatarok.x < x and hatarok.y < y and hatarok.x + hatarok.w > x and hatarok.y + hatarok.h > y)
		return true;
	else
		return false;
};
