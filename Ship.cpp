#define _CRT_SECURE_NO_WARNINGS 

#define for_x(n) for(int x = 0; x < n ; x++) 
#define for_y(n) for(int y = 0; y < n ; y++) 

#include <iostream>
#include <Windows.h>
#include <malloc.h>
#include <time.h>
#include <conio.h>
using namespace std; 
static bool RequestUpdate = true;
static int FrameTime = 500;

enum grid_data {
	data_empty = 0,
	data_occupied = 1,
};

struct grid {
    int Width; 
    int Height; 
    int* Data; 
};
static grid CreateGrid(int Width,int Height) {
    grid Result = {}; 
    Result.Width = Width; 
    Result.Height = Height; 
    size_t Amount = sizeof(int) * Width * Height;
    Result.Data = (int*)malloc(Amount); 
    if (!Result.Data) {
        std::cout << "Error allocating memory\n";
        exit(-1);
    };

    memset(Result.Data, 0, Amount); 

    return Result; 
}; 

enum game_mode {
	state_request_input = 1,
    state_main_menu, 
    state_place_ships,
	state_play,
};



static void WaitForFrames(int Frames) {
    while (Frames > 0) {
        Sleep(FrameTime);
        Frames--;
    };
};


enum direction {
	horizontal = 0,
	vertical,
};
enum ship_type {
	small_ship,
	medium_ship,
	large_ship,
	num_ship_types
};

struct cursor{
	bool Enabled; 

	int OldX;
	int OldY; 	
	int OldDirection;
	int OldExtend;

	int _X; 
	int _Y; 	
	int _Direction; 
	int _Extend; 
	
};
struct ship_placement_data {
	int CurrentType;
	int CurrentDirection;
	int Data[num_ship_types];
};



static int GetAmountOfShipsToPlace(ship_placement_data Data) {
	int Result = 0;
	for_x(num_ship_types) {
		Result += Data.Data[x];
	};
	return Result;
};
static ship_placement_data CreateShipPlacementData() {
	ship_placement_data Result = {};
	Result.Data[small_ship] = 3;
	Result.Data[medium_ship] = 2;
	Result.Data[large_ship] = 1;
	return Result;
};

struct game_state{
	bool RequestUpdate; 
	struct {
		int Width;
		int Height; 
		int* Data; 
		//Last row that was written.
		int LastRowIndex; 
	}TextBox;
	cursor Cursor; 	
	int* CursorData;
	grid Grid; 
}; 
static void SetCursorDirection(game_state* State,int Dir) {
	State->Cursor._Direction = Dir; 
};
static void SetCursorExtend(game_state*State,int Extend){
	State->Cursor._Extend = Extend; 
};
static void SetCursorPosition(game_state* State, int X, int Y) {
	if (X >= 0 && Y >= 0 && X <= State->Grid.Width - 1 && Y <= State->Grid.Height - 1) {
		State->Cursor._X = X; 
		State->Cursor._Y = Y;
	};
};
static void MoveCursor(game_state* State, int DirX, int DirY) {

	if (DirX != 0) {
		int NewX = State->Cursor._X + DirX;
		if (NewX >= 0 && NewX <= State->Grid.Width - 1) {
			State->Cursor._X = NewX;
			State->RequestUpdate = 1;
		}
	};

	if (DirY != 0) {
		int NewY = State->Cursor._Y + DirY;
		if (NewY >= 0 && NewY <= State->Grid.Height - 1) {
			State->Cursor._Y = NewY;
			State->RequestUpdate = 1;
		}
	};

}; 
static bool PlacePlayerShip(game_state* State,ship_placement_data Data) {
	int Type = Data.CurrentType; 
	int MaxCoords = 10;
	int* Coords = (int*)malloc(sizeof(int)*MaxCoords);
	memset(Coords, 0, sizeof(int) * MaxCoords); 
	int CoordCount = 0;
	int Dir = Data.CurrentDirection;
	int X = State->Cursor._X;
	int Y = State->Cursor._Y;

	switch (Type) {
		case small_ship: {
			CoordCount = 1;
			Coords[0] = X; 
			Coords[1] = Y;
		}break;
		case medium_ship: {
			Coords[0] = X;
			Coords[1] = Y;
			if (Dir == horizontal) {
				Coords[2] = X + 1; 
				Coords[3] = Y;
				Coords[4] = X + 2;
				Coords[5] = Y;
			}
			else {
				Coords[2] = X;
				Coords[3] = Y + 1;
				Coords[4] = X;
				Coords[5] = Y + 2;
			}; 
			CoordCount = 3;
		}; break;
		case large_ship: {
			CoordCount = 5; 

			Coords[0] = X;
			Coords[1] = Y;
			if (Dir == horizontal) {
				Coords[2] = X + 1;
				Coords[3] = Y;
				Coords[4] = X + 2;
				Coords[5] = Y;
				Coords[6] = X + 3;
				Coords[7] = Y;
				Coords[8] = X + 4;
				Coords[9] = Y;
			}
			else {
				Coords[2] = X;
				Coords[3] = Y + 1;
				Coords[4] = X;
				Coords[5] = Y + 2;
				Coords[6] = X;
				Coords[7] = Y + 3;
				Coords[8] = X;
				Coords[9] = Y + 4;
			};

		}; break;
	};

	bool PlaceShip = true;
	for_x(CoordCount) {
		int X = Coords[x * 2]; 
		int Y = Coords[(x * 2) + 1];
		int Index = X + (Y * State->Grid.Width);
		if (State->Grid.Data[Index] != 0) {
			PlaceShip = false;
			break;
		}
	}; 

	if (PlaceShip) {
		for_x(CoordCount) {
			int X = Coords[x * 2];
			int Y = Coords[(x * 2) + 1];
			int Index = X + (Y * State->Grid.Width);
			State->Grid.Data[Index] = data_occupied;
		}
	}
	State->RequestUpdate  = 1;
	free(Coords); 
	return PlaceShip;
}; 
static void UpdateCursorData(game_state* State) {
	if(State){
		if(State->Grid.Data){
			if(!State->CursorData){
				size_t Size = sizeof(int)*State->Grid.Width*State->Grid.Height;
				State->CursorData = (int*)malloc(Size);
				memset(State->CursorData,0,Size);
			}
			if (State->CursorData) {
				cursor* Cursor = &State->Cursor;
				if(Cursor->OldX != Cursor->_X || Cursor->OldY != Cursor->_Y || Cursor->OldDirection != Cursor->_Direction || Cursor->OldExtend != Cursor->_Extend){
					State->RequestUpdate = 1;
					for_x(State->Grid.Width*State->Grid.Height){
						State->CursorData[x] = 0; 
					};
					Cursor->OldX = Cursor->_X; 
					Cursor->OldY = Cursor->_Y; 
					Cursor->OldDirection = Cursor->_Direction;				
					Cursor->OldExtend = Cursor->_Extend;
					if(Cursor->_Extend > 1){
						if(Cursor->_Direction == horizontal){
							if(Cursor->_X + Cursor->_Extend >= State->Grid.Width){
								int Diff = (Cursor->_X + Cursor->_Extend) - (State->Grid.Width); 
								Cursor->_X -= Diff; 

							}; 
							State->CursorData[State->Cursor._X + (State->Cursor._Y * State->Grid.Width)] = 1;
							for_x(Cursor->_Extend-1) {
								State->CursorData[(State->Cursor._X + 1 + x) + (State->Cursor._Y * State->Grid.Width)] = 1;
							};

						}else if(Cursor->_Direction == vertical){
							if (Cursor->_Y + Cursor->_Extend >= State->Grid.Height) {
								int Diff = (Cursor->_Y + Cursor->_Extend) - (State->Grid.Height);
								Cursor->_Y -= Diff;

							};
							State->CursorData[State->Cursor._X + (State->Cursor._Y * State->Grid.Width)] = 1;
							for_x(Cursor->_Extend - 1) {
								State->CursorData[State->Cursor._X + ((State->Cursor._Y  +  1 + x)* State->Grid.Width)] = 1;
							};

						}; 
					}else{
						State->CursorData[State->Cursor._X + (State->Cursor._Y * State->Grid.Width)] = 1;											
					}; 
				};
			}
		}
	}
};
static int InitializeState(game_state* State){
	int Result = 0; 
	if(State){
		State->TextBox.Width = 120; 
		State->TextBox.Height = 28; 
		State->RequestUpdate = 0;
		size_t Size = State->TextBox.Width*State->TextBox.Height*sizeof(int); 
		State->TextBox.Data = (int*)malloc(Size); 
		if(State->TextBox.Data){
			memset(State->TextBox.Data,'\0',Size); 
			Result = 1; 
		}; 		
	}; 
	return Result; 
}; 

static void ClearTextBox(game_state* State) {
	for_x(State->TextBox.Width * State->TextBox.Height) {
		State->TextBox.Data[x] = '\0';
	}; 
}; 

static void Render(game_state* State){
	if(State->RequestUpdate){
		system("CLS"); 
		for_y(State->TextBox.LastRowIndex){
			for_x(State->TextBox.Width){
				char C = State->TextBox.Data[x + (y*State->TextBox.Width)]; 
				if(C != '\0'){	
					printf("%c",C); 
				}
			}; 
			printf("\n"); 
		}; 
		printf("\n");

		grid* Grid = &State->Grid; 
	

		if (Grid->Data) {
			for_y(Grid->Height) {
				for_x(Grid->Width) {
					int Data = Grid->Data[x + (y * Grid->Width)];
					int CursorData = State->CursorData[x + (y * Grid->Width)];
					if (CursorData) {
						printf("X ");
					}else {				
						switch (Data) {
							case data_empty: {
								printf(". "); 
							}break;
							case data_occupied: {
								printf("O ");
							}; break;
						};
					}
				};
				printf("\n"); 
			};
		}; 
		State->RequestUpdate = 0; 
	}; 
}; 

static int StrLen(const char* Text){
	int Result = 0; 
	while(Text[0] != '\0'){
		Text++; 
		Result++; 
	}; 
	return Result; 
}; 

static void AppendLine(game_state* State,const char* Text){
	if(State){
		if(Text){
			int Width = State->TextBox.Width;
			int Height = State->TextBox.Height; 
			int Row = State->TextBox.LastRowIndex;
			if(Row >= 0 && Row <= Height - 1){
				
				int L = StrLen(Text); 
				if(L > Width){
					L = Width;
				};
				for_x(L){
					State->TextBox.Data[x + (Row*Width)] = Text[x];
				}; 				
				State->TextBox.LastRowIndex++;
			}; 			
		}; 
	}; 
}; 

static void BeginFrame(game_state* State) {
		State->TextBox.LastRowIndex = 0; 
		AppendLine(State, "Sink the ship! V.0.01a");
};

struct input {
	char Key; 
};
static bool IsAlphabet(char C) {
	return ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z'));
}; 
static char ToUpper(char C) {
	if (C >= 'a' && C <= 'z') {
		return C - 32; 
	}
	return C; 
};
static bool IsNumber(char C) {
	return (C >= '0' && C <= '9'); 
}


int main()
{
   
    game_mode CurrentState = state_main_menu;
	game_mode PreviousState;
	input Input = {};
	game_state* State = (game_state*)malloc(sizeof(game_state)); 
	memset(State, 0, sizeof(game_state));
	RequestUpdate = 1; 
	
	ship_placement_data PlayerShipData = CreateShipPlacementData();
	if(InitializeState(State)){
		bool Running = 1; 
		while(Running){
			switch(CurrentState){
				case state_request_input: {
					Input.Key = _getch(); 
					CurrentState = PreviousState; 
					if (IsAlphabet(Input.Key)) {
						Input.Key = ToUpper(Input.Key);
					};
				}break; 
				case state_main_menu : {					
					static bool InvalidSelection = false; 
					
					if(RequestUpdate){		
						BeginFrame(State); 
						if (InvalidSelection) {
							AppendLine(State, "Invalid selection, please select again.");
						}; 

						AppendLine(State,"Select map size"); 

						AppendLine(State, "1 = 8x8");
						AppendLine(State, "2 = 10x10");
						AppendLine(State, "3 = 12x12");

						CurrentState = state_request_input; 
						PreviousState = state_main_menu; 
						RequestUpdate = 0; 
						State->RequestUpdate = 1;

					}else {
						int Selection= Input.Key;
						int SelectedMapSize = 0; 
						switch (Selection) {
							case '1': {SelectedMapSize = 8;}break;
							case '2': {SelectedMapSize = 10;}break;
							case '3': {SelectedMapSize = 12;}break;
							default: {
								InvalidSelection = true;
								RequestUpdate = true;
							};
						}; 
						if (SelectedMapSize != 0) {
							ClearTextBox(State);
							CurrentState = state_place_ships;
							RequestUpdate = 1; 
							State->Grid = CreateGrid(SelectedMapSize, SelectedMapSize); 
							State->Cursor.Enabled = 1;
							SetCursorPosition(State, State->Grid.Width >> 1, State->Grid.Height >> 1);
							State->RequestUpdate = 1;
						}; 
					};
				}break; 
				case state_place_ships: {
					if (RequestUpdate) {
						BeginFrame(State); 
						char Text[128];
						grid* Grid = &State->Grid; 
						sprintf(Text,"Map size : %ix%i\n",Grid->Width,Grid->Height); 
						AppendLine(State, Text); 
						AppendLine(State,"Choose ship placement"); 
						const char* ShipName = "";
						switch(PlayerShipData.CurrentType){
							case small_ship  : {ShipName  = "Small(1x1)";};break;
							case medium_ship: {ShipName = "Medium(1x3)"; }; break;
							case large_ship: {ShipName = "Large(1x5)"; }; break;
						}; 
						int ShipCount = PlayerShipData.Data[PlayerShipData.CurrentType]; 
						sprintf(Text, "1-3 = Ship type(Current Type : %s %i left)",ShipName,ShipCount);						
						AppendLine(State,Text);
						AppendLine(State, "W/A/S/D = Position");
						AppendLine(State, "R = Vertical/Horizontal");
						AppendLine(State, "ENTER = Place Ship");

						CurrentState = state_request_input;
						PreviousState = state_place_ships;
						RequestUpdate = 0;
						State->RequestUpdate = 1;

					}else {
						int ShipsToPlace = GetAmountOfShipsToPlace(PlayerShipData);
						if (ShipsToPlace > 0) {
							char DirX = 0;
							char DirY = 0;
							char Key = Input.Key;
							if (IsAlphabet(Key)) {
								if (Key == 'W') {
									DirY = -1; 
								}
								if (Key == 'S') {
									DirY = 1;
								}
								if (Key == 'A') {
									DirX = -1;
								}
								if (Key == 'D') {
									DirX = 1;
								}
								MoveCursor(State,DirX,DirY); 
								if (Key == 'R') {
									if (PlayerShipData.CurrentDirection == vertical) {
										PlayerShipData.CurrentDirection = horizontal;
									}else {
										PlayerShipData.CurrentDirection = vertical;
									}
									SetCursorDirection(State,PlayerShipData.CurrentDirection); 
								}
							}else if(IsNumber(Key)){
								int PrevType = PlayerShipData.CurrentType; 
								int Type = PrevType;
								switch(Key){
									case '1' : {Type = small_ship;}break; 
									case '2' : {Type = medium_ship;}break; 
									case '3' : {Type = large_ship;}break; 
								};

								if(PrevType != Type){
									PlayerShipData.CurrentType = Type; 
									switch(Type){
										case small_ship  : {SetCursorExtend(State,1);}break; 
										case medium_ship : {SetCursorExtend(State, 3);}break;
										case large_ship  : {SetCursorExtend(State, 5); }break;
									}; 
									RequestUpdate = 1; 
								}; 
								
							}else if(Key == 13){
								PlacePlayerShip(State,PlayerShipData);
							};
							
							if(!RequestUpdate){	
								CurrentState = state_request_input;
								PreviousState = state_place_ships;
							}
						}else {
							CurrentState = state_play; 
						}
					}
				}break; 
			};						

			UpdateCursorData(State); 
			Render(State); 		
		}; 				
	}else{
		cout<<"Error initializing State\n"; 
		exit(-1); 
	}; 
}


