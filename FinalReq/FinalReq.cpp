#include <chrono>
#include <vector>
#include <iostream>
#include <thread>

#include <Windows.h>

std::wstring tetrisBlocks[7];
int playFieldWidth = 12;
int playFieldHeight = 18;
unsigned char* pField = nullptr;

//default console screen size
int screenWidth;
int screenHeight;


int rotate(int x, int y, int r) {
	switch (abs(r % 4)) {
	case 0:
		return y * 4 + x;				//0 deg
		break;
	case 1:
		return 12 + y - (x * 4);		//90 deg
		break;
	case 2:
		return 15 - (y * 4) - x;		//180 deg
		break;
	case 3:
		return 3 - y + (x * 4);			// 270 deg
		break;
	}
	return 0;
}


bool doesPieceFit(int ntetrisBlock, int rotation, int posX, int posY) {
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			//index of peice
			int i = rotate(x, y, rotation);

			//index of field
			int fi = (posY + y) * playFieldWidth + (posX + x);

			if ((posX + x >= 0 && posX + x <= playFieldWidth) && (posY + y >= 0 && posY + y <= playFieldHeight)) {
				if (tetrisBlocks[ntetrisBlock][i] == L'X' && pField[fi] != 0) {
					return false; //block hit object 
				}
			}
		}
	}
	return true;
}

int pieceCast(int ntetrisBlock, int rotation, int posX, int posY) {
	int rCast = 0;
	while (doesPieceFit(ntetrisBlock, rotation, posX, posY + rCast)) {
		rCast++;
	}
	return rCast - 1;
}

void LockPiece(int ntetrisBlock, int rotation, int posX, int posY) {
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			if (tetrisBlocks[ntetrisBlock][rotate(x, y, rotation)] == L'X') {
				pField[(posY + y) * playFieldWidth + (posX + x)] = ntetrisBlock + 1;
			}
		}
	}
}

int main()
{
	//init resource to handle cmdline
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

	//activate handle as active
	SetConsoleActiveScreenBuffer(hConsole);

	//init datablock to pass to screen
	DWORD dwByteWritten = 0;

	//init cmdline usable info
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

	//reads hconsole cmdline to write usable info
	GetConsoleScreenBufferInfo(hConsole, &csbiInfo);

	//init COORD to pass to hConsole
	COORD ConsoleWH;

	//pass cmdline info to COORD
	ConsoleWH = csbiInfo.dwSize;

	//default cmdline sizing
	screenWidth = ConsoleWH.X; //defualt for my pc 
	screenHeight = ConsoleWH.Y;

	//create blocks
	tetrisBlocks[0].append(L"..X.");
	tetrisBlocks[0].append(L"..X.");
	tetrisBlocks[0].append(L"..X.");
	tetrisBlocks[0].append(L"..X.");

	tetrisBlocks[1].append(L"..X.");
	tetrisBlocks[1].append(L".XX.");
	tetrisBlocks[1].append(L".X..");
	tetrisBlocks[1].append(L"....");

	tetrisBlocks[2].append(L".X..");
	tetrisBlocks[2].append(L".XX.");
	tetrisBlocks[2].append(L"..X.");
	tetrisBlocks[2].append(L"....");

	tetrisBlocks[3].append(L"....");
	tetrisBlocks[3].append(L".XX.");
	tetrisBlocks[3].append(L".XX.");
	tetrisBlocks[3].append(L"....");

	tetrisBlocks[4].append(L".X..");
	tetrisBlocks[4].append(L".XX.");
	tetrisBlocks[4].append(L".X..");
	tetrisBlocks[4].append(L"....");

	tetrisBlocks[5].append(L"....");
	tetrisBlocks[5].append(L".XX.");
	tetrisBlocks[5].append(L"..X.");
	tetrisBlocks[5].append(L"..X.");

	tetrisBlocks[6].append(L"....");
	tetrisBlocks[6].append(L".XX.");
	tetrisBlocks[6].append(L".X..");
	tetrisBlocks[6].append(L".X..");

	//play feild outline buffer
	pField = new unsigned char[playFieldWidth * playFieldHeight];
	for (int x = 0; x < playFieldWidth; x++) {
		for (int y = 0; y < playFieldHeight; y++) {
			pField[y * playFieldWidth + x] = (x == 0 || x == playFieldWidth - 1 || y == playFieldHeight - 1) ? 9 : 0;
		}
	}


	//create screen buffer
	wchar_t* screen = new wchar_t[screenWidth * screenHeight];
	for (int i = 0; i < screenWidth * screenHeight; i++) {
		screen[i] = L' ';
	}
	int gametiming = 50;

	//basic game logic
	bool bGameOver = false;
	bool blockPlace = true;

	//PieceLogic
	int currentPiece = 0;
	int currentRotation = 0;
	int currentX = (playFieldWidth / 2) - 2;
	int currentY = 0;

	int nextPiece = 0;


	//input bools
	bool key[7];
	bool rotateHold = false;

	//gametick
	int speed = 20;
	int speedCount = 0;
	bool blockDown = false;

	//drophard var
	bool dropHard = false;

	//stashing vars
	int holdPiece = 8;
	bool canstash = true;

	//scoring vars
	int pieceCount = 0;
	int score = 0;

	std::vector<int> compLine;

	srand(rand() ^ time(NULL));
	
	//GAME RUNTIME ********************************************
	while (!bGameOver)
	{
		//GAME TIMING *******************************************
		std::this_thread::sleep_for(std::chrono::milliseconds(gametiming));
		speedCount++;
		gametiming = 50;

		blockDown = (speedCount == speed);

		if (blockPlace) {
			nextPiece = rand() % 7;
			blockPlace = false;
		}

		//raycast for DropHard
		int rProj = pieceCast(currentPiece, currentRotation, currentX, currentY);



		//INPUT ***************************************************
		if (!dropHard) {
			for (int k = 0; k < 7; k++) {							// R   L   D   U  SPACEBAR SHFT C
				key[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x26\x20\x10\x43"[k]))) != 0; //checks if button is pressed
			}
		}


		//GAME LOGIC **********************************************
		if (key[0] && doesPieceFit(currentPiece, currentRotation, currentX + 1, currentY)) {
			++currentX;
		}
		if (key[1] && doesPieceFit(currentPiece, currentRotation, currentX - 1, currentY)) {
			--currentX;
		}

		//DropSoft
		if (key[2] && doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
			++currentY;
		}

		//rotate CCW
		if (key[3] && doesPieceFit(currentPiece, currentRotation + 1, currentX, currentY)) {
			if (!rotateHold) {
				++currentRotation;
				rotateHold = true;
			}
		}

		//DropHard
		if (key[4] && doesPieceFit(currentPiece, currentRotation, currentX, currentY + rProj)) {
			currentY += rProj;
			dropHard = true;
		}

		//Hold Current Piece
		if (key[5] && canstash)
		{
			if (holdPiece > 7) {
				holdPiece = currentPiece;
				currentPiece = nextPiece;
			}
			else {
				std::swap(holdPiece, currentPiece);
			}
			currentY = 0;
			canstash = false;
		}

		//rotate CW
		if (key[6] && doesPieceFit(currentPiece, currentRotation - 1, currentX, currentY)) {
			if (!rotateHold) {
				--currentRotation;
				rotateHold = true;
			}
		}

		//Hold Current Pos
		else rotateHold = false;

		if (blockDown) {
			speedCount = 0;

			//Block animation
			if (doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1) && !dropHard)
			{
				++currentY;
			}

			//Game Logic when block is no longer falling
			else
			{

					LockPiece(currentPiece, currentRotation, currentX, currentY);
					blockPlace = true;

					//Increase Difficulty
					pieceCount++;
					if (pieceCount % 10 == 0 && pieceCount > 10) {
						speed--;
					}


					//check horizontal lines
					for (int y = 0; y < 4; y++) {
						if (currentY + y < playFieldHeight - 1) {
							bool line = true;
							for (int x = 1; x < playFieldWidth - 1; x++) {
								line &= (pField[(currentY + y) * playFieldWidth + x]) != 0;
							}
							if (line) {
								for (int x = 1; x < playFieldWidth - 1; x++) {
									pField[(currentY + y) * playFieldWidth + x] = 8;
								}
								compLine.push_back(currentY + y);
							}
						}
					}
					
					//score
					score += 25;
					if (!compLine.empty()) {
						score += (1 << compLine.size()) * 100; //increase score exponentially for each line completed at a time
					}

					canstash = true;

				//drophard timing compensation
				if (dropHard) {
					gametiming = 50;
					std::this_thread::sleep_for(std::chrono::milliseconds(gametiming));
				}

				//choose next piece
				currentX = ((playFieldWidth / 2) - 2);
				currentY = 0;
				currentRotation = 0;
				currentPiece = nextPiece;

				dropHard = false;

				bGameOver = !doesPieceFit(currentPiece, currentRotation, currentX, currentY);
			}
		}


		//RENDER OUTPUT **********************************************

		//draw playing field
		for (int x = 0; x < playFieldWidth; x++) {
			for (int y = 0; y < playFieldHeight; y++) {
				screen[(y + 2) * screenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * playFieldWidth + x]]; //wchar array 
			}
		}

		//draw piece
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				if (tetrisBlocks[currentPiece][rotate(x, y, currentRotation)] == L'X') {
					screen[(currentY + y + 2) * screenWidth + (currentX + x + 2)] = currentPiece + 65; //for ascii characters
				}
			}
		}

		//draw next piece
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				if (tetrisBlocks[nextPiece][rotate(x, y, 0)] == L'X') {
					screen[(2 + y + 2) * screenWidth + playFieldWidth + 8 + x] = nextPiece + 65; //for ascii characters
				}

				else {screen[(2 + y + 2) * screenWidth + playFieldWidth + 8 + x] = L" "[0]; }
			}
		}

		//draw next piece
		if(holdPiece < 8){
			for (int x = 0; x < 4; x++) {
				for (int y = 0; y < 4; y++) {
					if (tetrisBlocks[holdPiece][rotate(x, y, 0)] == L'X') {
						screen[(10 + y + 2) * screenWidth + playFieldWidth + 8 + x] = holdPiece + 65; //for ascii characters
					}
					else { screen[(10 + y + 2) * screenWidth + playFieldWidth + 8 + x] = L" "[0]; }
					}
				}
			}
		
		
		

		//display score
		swprintf_s(&screen[2 * screenWidth + playFieldWidth + 4], 30, L"Next Piece    Score: %8d", score);
		
		//instructions
		swprintf_s(&screen[4 * screenWidth + playFieldWidth + 18], 26, L"Move Left & Right : < + >");
		swprintf_s(&screen[5 * screenWidth + playFieldWidth + 18], 21, L"Soft Drop : D ArwKey");
		swprintf_s(&screen[6 * screenWidth + playFieldWidth + 18], 21, L"Hard Drop : Spacebar");
		swprintf_s(&screen[7 * screenWidth + playFieldWidth + 18], 19, L"Rotate CCW : C Key");
		swprintf_s(&screen[8 * screenWidth + playFieldWidth + 18], 21, L"Rotate CW : U ArwKey");
		swprintf_s(&screen[9 * screenWidth + playFieldWidth + 18], 14, L"Stash : Shift");

		//display stashed piece
		swprintf_s(&screen[10 * screenWidth + playFieldWidth + 4], 14, L"Stashed Piece");

		

		//make line disappear 
		if (!compLine.empty()) {

			//Display Frame
			WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwByteWritten);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			for (auto& v : compLine) {
				for (int x = 1; x < playFieldWidth - 1; x++) {
					for (int y = v; y > 0; y--) {
						pField[y * playFieldWidth + x] = pField[(y - 1) * playFieldWidth + x];
						pField[x] = 0;
					}
				}

				compLine.clear();
			}
		}

		//Display Output
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwByteWritten);
	}


	//Game Over
	CloseHandle(hConsole);
	std::cout << "\t \t GAME OVER~! ( > ~ < ) \n \t \t Here is your score:" << score << std::endl;
	system("pause");
	return 0;
}
