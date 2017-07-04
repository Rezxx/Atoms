#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "atoms.h"

//for the extraction of colours 
char colOne[] = {'R', 'G', 'P', 'B', 'Y', 'W'};
char *colTwo[] = {"Red", "Green", "Purple", "Blue", "Yellow", "White"};

//global variables serve for later 
grid_t* ptr_grid = NULL;
player_t* ptr_players = NULL;
move_t* ptr_head = NULL;
move_t* last_move = NULL;
int *data;
int width;
int height;
int players_num;
int current;// current payer represent as int 
int moves;// total number of moves 
int lost_players_num;


/*------------------------------------function for initialize global variables --------------------------------------*/

void start(int num, int w, int h){
	width = w;
	height = h;
	players_num = num;
	moves = 0;
	ptr_grid = (grid_t*)malloc(width * height * sizeof(grid_t));//memory space for all of grids
	for(int i = 0; i < height; i++){
		for(int o = 0; o < width; o ++){
			grid_t* the_grid = ptr_grid + i + o * height;
			the_grid -> owner = NULL;
			the_grid -> atom_count = 0;
			if((i + o) == 0 || (i == 0 && o == width - 1)
		   	|| (i == height - 1 && o == 0) 
		   	|| ( i == height - 1 && o == width - 1)){
				the_grid -> limit = 2;//the grids in the conor
			}else if(i == 0 || i == height - 1 || o == 0 || o == width - 1){
				the_grid -> limit = 3;//the grids oneach side but not the conor 
			}else{
				the_grid -> limit = 4;//the other grids in the middle
			}
		}
	}
	ptr_players = (player_t*)malloc(players_num * sizeof(player_t));
	for(int i = 0; i < players_num; i++){
		(ptr_players + i) -> colour = i;
		(ptr_players + i) -> grids_owned = 0;
		(ptr_players + i) -> lost = false;//determine if the player has lost yet 
	}
	printf("Game Ready\nRed's Turn\n\n");
	current = 0;
	return;
}

/*----------------------------------------function for free heap memory-----------------------------------------------*/

void free_memory(){
	free(ptr_grid);
	free(ptr_players);
	move_t* free_time;
	if(last_move != NULL){
	//when there exists at leat one move
	    while(last_move -> parent != NULL){
		   if(last_move -> extra != NULL){
	  	      move_t* extra_moves = last_move-> extra;
	  		  while(extra_moves ->extra != NULL){
	  		    extra_moves = extra_moves -> extra;//get the last extra move in each manual move 
	  		  }
			  //start from the last auto move towards the manual moves created those moves
	  		  move_t* tmp;
	  		  while(extra_moves -> parent != last_move){
	  			tmp = extra_moves -> parent;
	  			free(extra_moves);//free the auto move
	  			extra_moves = tmp;
	  		  }
	  		  free(extra_moves);
	  	   }
			//Updating the manual move until the first move
	       free_time = last_move -> parent;
	       free(last_move);
	   	   last_move = free_time;
	    }
	    free(last_move);	
    }
}

/*-----------------------------------------function for place the atom-----------------------------------------------*/

/*automa means if this move is made by other move*/
void place(int x, int y, bool automa){
	grid_t* the_grid = ptr_grid + (y * height + x);
	//when manually put an atom on other players' grid
	if(the_grid -> owner != NULL){
		 if((the_grid-> owner)->colour != current && automa == false){
			printf("Cannot Place Atom Here\n\n");
			return;
		 }
	}
	
	move_t* ptr_move = (move_t*)calloc(1, sizeof(move_t));
	ptr_move -> x = x;
	ptr_move -> y = y;
	if(automa == false){
	//Updating current move(last_move) and create linked list with last move.
		if(last_move == NULL){
			ptr_head = ptr_move;
			last_move = ptr_move;
		}else{
			last_move -> next = ptr_move;
			ptr_move -> parent = last_move;
			last_move = ptr_move;
		}
	}else{
		if(last_move -> extra == NULL){
		//when explosion happens, create (move_t*)extra of last manual move
			last_move -> extra = ptr_move;
			ptr_move -> parent = last_move;
		}else{
		//when more than one extra moves, connect them with extra and parent in move structure
			move_t* last_extra = last_move -> extra;
			while(last_extra -> extra != NULL){
				last_extra = last_extra -> extra;
			}
			last_extra -> extra = ptr_move;
			ptr_move -> parent = last_extra;
		}
	}
	
	//Updating all the player and grid's information
	if(the_grid -> owner == NULL){
	//when the move is on a empty grid
		ptr_move -> old_owner = NULL;
		the_grid -> owner = ptr_players + current;
		the_grid -> atom_count = 1;
		(ptr_players + current)-> grids_owned ++;
	}else if((the_grid-> owner)->colour == current || automa == true){
	//when the move is auto and on current player's grid
		if((the_grid-> owner)->colour != current){
			// auto move 
			ptr_move -> old_owner = the_grid-> owner;
			if((the_grid-> owner) -> grids_owned == 1){
			//the time when someone has lost the game
				(the_grid-> owner) -> lost = true;
				lost_players_num++;
			}
			(ptr_players + current)-> grids_owned++;
			(the_grid-> owner) -> grids_owned--;
			the_grid-> owner = ptr_players + current;
		}
		if((the_grid-> owner)->colour == current){
		//updating old owner into current player
			ptr_move -> old_owner = ptr_players + current;
		}
		if(the_grid -> atom_count + 1 < the_grid -> limit){
		//when the grid's atom_count hasn't reached the limit yet
			the_grid -> atom_count ++;
		}else{
		//when the time for explosion
			(the_grid-> owner) -> grids_owned--;
			the_grid-> owner = NULL;
			the_grid -> atom_count = 0;
			if(x == 0){
				if(y == 0){
				//left top conor
					place(1, 0, true);
					place(0, 1, true);
				}else if(y == height - 1){
				//left botton conor
					place(0, y - 1, true);
					place(1, y, true);
				}else{
				//left edge
					place(0, y - 1, true);
					place(1, y, true);
					place(0, y + 1, true);
				}
			}else if(y == 0){
				if(x == width - 1){
				//right top conor
					place(x, 1, true);
					place(x - 1, 0, true);
				}else{
				//top edge
					place(x + 1, 0, true);
					place(x, 1, true);
					place(x - 1, 0, true);
				}
			}else if(x == width - 1){
				if(y == height - 1){
				//right bottom conor
					place(x - 1, y, true);
					place(x, y - 1, true);
				}else{
				//right edge
					place(x, y + 1, true);
					place(x - 1, y, true);
					place(x, y - 1, true);
				}
			}else if(y == height - 1){
			//bottom edge
				place(x - 1, y, true);
				place(x, y - 1, true);
				place(x + 1, y, true);
			}else{
			//all of middle geinds
				place(x, y - 1, true);
				place(x + 1, y, true);
				place(x, y + 1, true);
				place(x - 1, y, true);
			}
		}
	}
	if(lost_players_num == players_num - 1){
	//checking if the game is over 
		printf("%s Wins!\n",  colTwo[current]);
		free_memory();
		exit(0);
	}
	
	//Updating current player
	if(automa == false){
		while(true){
			if(current < players_num - 1){
				current++;
			}else{
				current = 0;
			}
			if((ptr_players + current)-> lost == false){
				break;
			}
		}
		moves++;// increment moves
	}
}

/*--------------------------------------------function for saving data-----------------------------------------------*/

void save(char* name){
	//Testing if the file exists
	FILE *test = fopen(name, "r");
	if(test != NULL){
		printf("File Already Exists\n\n");
		fclose(test);
		return;
	}else{
		FILE *test = fopen(name, "wb");
		//Writing the first three bytes
		fwrite(&width, 1, 1, test);
		fwrite(&height, 1, 1, test);
		fwrite(&players_num, 1, 1, test);
		//Writing the moves data if there exists one 
		if(ptr_head != NULL){
			move_t* move = ptr_head;
			while(move!= NULL){
				int zero = 0;
				fwrite(&(move -> x), 1, 1, test);
				fwrite(&(move -> y), 1, 1, test);
				fwrite(&zero, 1, 2, test);//for the padding
				move = move -> next;
			}
		}
		printf("Game Saved\n\n");
		fclose(test);
	}
}

/*----------------------------------------function for displaying grids---------------------------------------------*/
void display(){
	//first row
	printf("\n+");
	for(int i = 0; i < 3 * width - 1; i++){
		printf("-");
	}
	printf("+\n");
	
	//the middle 
	for(int i = 0; i < height; i++){
		printf("|");
		for(int o = 0; o < width; o++){
			if((ptr_grid + (i*height) + o)-> owner == NULL){
				printf("  |");
			}else{
				char owner = colOne[((ptr_grid + (i*height) + o)-> owner) -> colour];
				int number = (ptr_grid + (i*height) + o) -> atom_count;
				printf("%c%d|", owner, number);
			}
		}
		printf("\n");
	}
	
	//the last row
	printf("+");
	for(int i = 0; i < 3 * width - 1; i++){
		printf("-");
	}
	printf("+\n\n");
}

/*----------------------------------function for display the stat of players----------------------------------------*/

void display_stat(){
	
	for(int i = 0; i < players_num; i++){
		printf("Player %s:\n", colTwo[i]);
		if((ptr_players + i) -> lost == false){
			printf("Grid Count: %d\n\n", (ptr_players + i) -> grids_owned);
		}else{
			printf("Lost\n\n");// when the player has lost the game
		}
	}
}



/*----------------------------------------function for loading the game-----------------------------------------------*/

void load(char* name){
	FILE *file = fopen(name, "rb");
	//read the first three bytes
	fread(&width, 1,1, file);
	fread(&height, 1,1, file);
	fread(&players_num, 1,1, file);
	moves = 0;
	data = (int*)calloc(100,sizeof(int));// temporarialy storing coordinates
	//reading the rest of moves into data
	while(!feof(file)){
		int garbage;
		fread(data + 2* moves, 1,1, file);
		fread(data + 2* moves + 1, 1,1, file);
		fread(&garbage, 1, 2, file);
		moves++;
	}
	fclose(file);
	printf("Game Loaded\n\n");
}

/*--------------------------------------function for playing after n moves-----------------------------------------*/

void playfrom(int turn){
	//initialize essential data as in start 
	moves = 0;
	current = 0;
	ptr_grid = (grid_t*)malloc(width * height * sizeof(grid_t));
	for(int i = 0; i < height; i++){
		for(int o = 0; o < width; o ++){
			grid_t* the_grid = ptr_grid + i + o * height;
			the_grid -> owner = NULL;
			the_grid -> atom_count = 0;
			if((i + o) == 0 || (i == 0 && o == width - 1)
		   	|| (i == height - 1 && o == 0) 
		   	|| ( i == height - 1 && o == width - 1)){
				the_grid -> limit = 2;
			}else if(i == 0 || i == height - 1 || o == 0 || o == width - 1){
				the_grid -> limit = 3;
			}else{
				the_grid -> limit = 4;	
			}
		}
	}
	ptr_players = (player_t*)malloc(players_num * sizeof(player_t));
	for(int i = 0; i < players_num; i++){
		(ptr_players + i) -> colour = i;
		(ptr_players + i) -> grids_owned = 0;
		(ptr_players + i) -> lost = false;
	}
	// depends on the moves using place to create same information
	for(int i = 0; i < turn - 1; i++){
		int x = *(data + 2*i);
		int y = *(data + 2*i + 1);
		place(x, y, false);
	}
	free(data);
	printf("Game Ready\n%s's Turn\n\n", colTwo[current]);
}

/*--------------------------------------function for undoing the extra moves------------------------------------------*/

void undo_extra(move_t* this){
	player_t *last_player = ptr_players + current - 1;
	grid_t *this_grid = ptr_grid + (this -> y * height + this -> x);
	if(this == last_move){
	//when the time recusion gets to the manual move
		return;
	}else{
		if(this -> old_owner == NULL){
		//when there's no atom before
			this_grid -> owner = NULL;
			this_grid -> atom_count--;
			last_player -> grids_owned--;
		}else if(this -> old_owner == last_player){
		//when the old owner is the same player(include the situation reached limit) 
			if(this_grid-> atom_count == 0){
				this_grid -> atom_count = this_grid -> limit - 1;
				this_grid -> owner = last_player;
				last_player -> grids_owned++;
			}else{
				this_grid -> atom_count--;
			}
		}else if(this -> old_owner != last_player) {
		//when the move's old owner is other player 
			if(this -> old_owner->lost == true){
			//when this extra move made someone lost
				this -> old_owner-> lost = false;
			}
			this_grid -> owner = this -> old_owner;
			this_grid -> atom_count--;
			this -> old_owner-> grids_owned++;
			last_player -> grids_owned--;
		}
	}
	undo_extra(this->parent);// keep recursion
	free(this);
}

/*----------------------------------------the main function for input stuff-------------------------------------------*/

int main(int argc, char** argv) {
	bool started = false;//if the game is started or begin after playfrom
	bool loaded = false;//true after load function false after playerfrom
	while(1){
		char string[50];//take the whole stdin
		char *command[5];//design to store the command after split by " "
		int s = scanf("%[^\n]%*c", string);
		if(s == 0){
		//when reading is unsuccessful
			printf("Invalid Command\n\n");
			continue;
		}
		int count = 0;//1(command) + #arguments 
		//spliting
		char*p = strtok(string, " ");
		while(p!= NULL){
			command[count++] = p;
			p = strtok(NULL, " ");
		}
		
		//making the command case insensitive
		char *first = command[0];
		for(int i = 0; *(first + i) != 0; i++){
			*(first + i) = toupper(*(first + i));
		}
		//choosing from different command
		if(strcmp(first, "HELP") == 0){
			//the status after load
			if(loaded == true){
				printf("Invalid Command\n\n");
				continue;
			}
			//no argument
			if(count == 1){
				printf("\nHELP displays this help message\n"
		 		  "QUIT quits the current game\n\n"
		 		  "DISPLAY draws the game board in terminal\n"
				  "START <number of players> <width> <height> starts the game\n"
				  "PLACE <x> <y> places an atom in a grid space\n"
				  "UNDO undoes the last move made\n"
				  "STAT displays game statistics\n\n"
				  "SAVE <filename> saves the state of the game\n"
				  "LOAD <filename> loads a save file\n"
				  "PLAYFROM <turn> plays from n steps into the game\n\n");
			}else{
				printf("Invalid Command\n\n");
				continue;
			}
		}else if(strcmp(first, "QUIT") == 0){
			if(count == 1){
			//no argument 
				printf("Bye!\n");
				free_memory();
				exit(0);
			}else{
				printf("Invalid Command\n\n");
				continue;
			}

		}else if(strcmp(first, "START") == 0){
			if(count > 4){
				printf("Too Many Arguments\n\n");
				continue;
			}else if(count < 4){
				printf("Missing Argument\n\n");
				continue;
			}else{
				int argument[3];
				char* end;
				bool fail = false; 
				//convert the arguments from string to int
				for(int i = 0; i < 3; i++){
					argument[i] = (int)strtol(command[1+i], &end, 10);
					if(*end != 0 ){
						fail = true;
						printf("%c.", *end);
						break;
					}
				}
				//no arguments can be negtive
				if(fail == false){
					for(int i = 0; i < 3; i++){
						if(argument[i] < 0){
							fail = true;
						}
					}
				}
				//other constraints
				if((argument[0] < 2) || (argument[0] > 6) 
				  || (argument[1] > 255) || (argument[2] > 255)){
					fail = true;
				}
				if(fail == true){
					printf("Invalid command arguments\n\n");
					continue;
				}else if(started == true || loaded == true){
					printf("Invalid Command\n\n");
					continue;
				}else if(argument[1] * argument[2] < argument[0]){
					printf("Cannot Start Game\n\n");
					continue;
				}
				start(argument[0], argument[1], argument[2]);
				started = true;
			}
		}else if(strcmp(first, "DISPLAY") == 0){
			if(count!= 1 || loaded == true){
				printf("Invalid Command\n\n");
				continue;
			}else if(started == false){
				printf("Game Not In Progress\n\n");
				continue;
			}else{
				display();
			}
		}else if(strcmp(first, "STAT") == 0){
			if(count!= 1 || loaded == true){
				printf("Invalid Command\n\n");
				continue;
			}else if(started == false){
				printf("Game Not In Progress\n\n");
				continue;
			}else{
				display_stat();
			}
		}else if(strcmp(first, "PLACE") == 0){
			if(count!= 3){
				printf("Invalid Coordinates\n\n");
				continue;
			}else if(loaded == true){
				printf("Invalid Command\n\n");
				continue;
			}else{
				int argument[2];
				char* end;
				bool fail = false; 
				//converting parameters
				for(int i = 0; i < 2; i++){
					argument[i] = (int)strtol(command[1+i], &end, 10);
					if(*end != 0){
						fail = true;
						break;
					}
				}
				//constraints
				if(fail == false){
					for(int i = 0; i < 2; i++){
						if(argument[i] < 0 ){
							fail = true;
						}
					}
				}
				if(argument[0] >= width || argument[1] >= height){
					fail = true;
				}
				if(fail == true){
					printf("Invalid Coordinates\n\n");
					continue;
				}else{
					place(argument[0], argument[1], false);
					printf("%s's Turn\n\n", colTwo[current]);
				}
			}				
		}else if(strcmp(first, "SAVE") == 0){
			if(count!= 2 || loaded == true){
				printf("Invalid Command\n\n");
				continue;
			}
			save(command[1]);
			
		}else if(strcmp(first, "LOAD") == 0){
			if(count!= 2){
				printf("Invalid Command\n\n");
				continue;
			}else if (started == true){
				printf("Restart Application To Load Save\n\n");
			}else{
				//testing if the file exists
				FILE *file = fopen(command[1], "r");
				if(file == NULL){
					printf("Cannot Load Save\n\n");
					continue;
				}else{
					load(command[1]);
					loaded = true;
				}
			}
		}else if(strcmp(first, "PLAYFROM") == 0){
			if(count!= 2 || loaded == false){
				printf("Invalid Command\n\n");
				continue;
			}else{
				//converting
				char* end;
				int argument = (int)strtol(command[1], &end, 10);
					if(strcmp(command[1], "END") == 0){
						argument = moves;
					}else if(*end != 0){
						printf("Invalid Command\n\n");
						continue;
					}
				
				playfrom(argument);
				started = true;
				loaded = false;//only place to make it false
			}
		}else if(strcmp(first, "UNDO") == 0){
			if(started == false || last_move == NULL){
				printf("Cannot Undo\n\n");
				continue;
			}else{
				if(last_move -> extra != NULL){
				//getting the last move's last auto move
					move_t* last_extra = last_move -> extra;
					while(last_extra -> extra!= NULL){
						last_extra = last_extra -> extra;
					}
					undo_extra(last_extra);
				}
				//getting the last player
				int o = current;
				while(true){
					if(o > 0){
						o--;
					}else{
						o = players_num - 1;
					}
					if((ptr_players + o)-> lost == false){
						break;
					}
				}
				
				//undo the manual move 
				player_t *last_player = ptr_players + o;
				grid_t *this_grid = ptr_grid + (last_move -> y * height + last_move -> x);
				
				if(last_move -> old_owner == NULL){
					//when there's no atom before
					this_grid -> owner = NULL;
					this_grid -> atom_count--;
					last_player -> grids_owned--;
				}else if(last_move  -> old_owner == last_player){
				//when the old owner is the same player(include the situation reached limit) 
					if(this_grid-> atom_count == 0){
						this_grid -> atom_count = this_grid->limit - 1;
						this_grid -> owner = last_player;
						last_player -> grids_owned++;
					}else{
						this_grid -> atom_count--;
					}
				}
				move_t *tmp = last_move -> parent;
				free(last_move);
				last_move = tmp;
				moves--;
				current = o;
				printf("%s's Turn\n\n", colTwo[current]);
			}
		}else{
			printf("Invalid Command\n\n");// all of other in valid input
			continue;
		}
	}
	return 0;
}


