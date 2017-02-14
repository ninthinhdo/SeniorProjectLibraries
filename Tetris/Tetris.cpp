#include "Arduino.h"
#include "Tetris.h"

#include "Adafruit_NeoPixel.h"
#ifdef __AVR__
#include "avr/power.h"
#endif
#include "avr/pgmspace.h"

#define PIN 44
Adafruit_NeoPixel tetris_strip = Adafruit_NeoPixel(280, PIN, NEO_GRB + NEO_KHZ800);

#include "PixelResource.h"

typedef struct Block_4x4 {
	unsigned line1 : 4;
	unsigned line2 : 4;
	unsigned line3 : 4;
	unsigned line4 : 4;
	unsigned : 0;
};
typedef struct Block_4x4 Block_4x4;

typedef struct Block_3x3 {
	unsigned line1 : 3;
	unsigned line2 : 3;
	unsigned line3 : 3;
	unsigned : 0;
};
typedef struct Block_3x3 Block_3x3;

Block_4x4 IBlock{
	B0000,
	B1111,
	B0000,
	B0000
};

Block_4x4 OBlock{
	B0000,
	B0110,
	B0110,
	B0000
};

Block_3x3 TBlock{
	B000,
	B111,
	B010
};

Block_3x3 SBlock{
	B000,
	B011,
	B110
};

Block_3x3 ZBlock{
	B000,
	B110,
	B011
};

Block_3x3 JBlock{
	B000,
	B100,
	B111
};

Block_3x3 LBlock{
	B000,
	B001,
	B111
};

Block_4x4 *BlockList_4x4[] = { &IBlock, &OBlock };
Block_3x3 *BlockList_3x3[] = { 0, 0, &TBlock, &SBlock, &ZBlock, &JBlock, &LBlock };

#define MAX_BLOCK 7

Tetris::Tetris()
{
	// Play Tetris Theme
	Serial2.print("GAME.TETRIS.");

	// Initialize LED strip
	tetris_strip.begin();
	tetris_strip.setBrightness(32);
	tetris_strip.clear();

	//Setup Random Generator
	randomSeed(analogRead(0));

	// Initialize logicMatrice to zeros;
	clearLogicMatrice();

	// Initialize new block
	generateNewBlock();
}


void Tetris::Update(){
	updateFrame();

	// Disable placement
	allowPlacement = false;

	//1000ms delay
	waitForInput(20, blockSpeed);

	// Enable placement
	allowPlacement = true;

	// Move block down one
	rowPos++;
}


void Tetris::waitForInput(int wait, int blockDelay){

	String command;
	for (int i = 0; i < wait; i++){

		if (Serial1.available() > 0){
			command = Serial1.readStringUntil('.');
			previousCommand = command;

			if (command.equals("JUp"))
				blockSpeed = (blockSpeed > 5) ? blockSpeed - 5 : 10;
			else if (command.equals("JDown"))
				blockSpeed = (blockSpeed < 100) ? blockSpeed + 5 : 100;
			else if (command.equals("BUp")){
				Serial2.print("BLOCK2.");
				rotation = (rotation < 3) ? rotation + 1 : 0;
			}
			else if (command.equals("BRight")){
				Serial2.print("BLOCK3.");
				colPos++;
			}
			else if (command.equals("BDown")){
				Serial2.print("BLOCK3.");
				rowPos++;
				allowPlacement = true;
			}
			else if (command.equals("BLeft")){
				Serial2.print("BLOCK3.");
				colPos--;
			}
			else if (command.equals("BReset")){
				allowPlacement = true;
				while (rowPos != 0){
					rowPos++;
					updateFrame();
				}
				allowPlacement = false;
			}

			updateFrame();
			if (allowPlacement) allowPlacement = false;  // Used by down command
		}

		delay(blockDelay);
	}
}



void Tetris::updateFrame(){

	// Clear previous block
	clearStrip(rowPos, 5);

	bool posConflict = false;

	if (currentBlock < 2){
		Block_4x4 tempBlock = *BlockList_4x4[currentBlock];
		Block_4x4 rotatedBlock = tempBlock;

		// Block Structure
		/*        3 2 1 0 (bit)
		*  line1
		*  line2
		*  line3
		*  line4
		*  (variable)
		*/

		// Rotate block
		for (int i = 0; i < rotation; i++){
			bitWrite(rotatedBlock.line1, 3, bitRead(tempBlock.line4, 3));
			bitWrite(rotatedBlock.line1, 2, bitRead(tempBlock.line3, 3));
			bitWrite(rotatedBlock.line1, 1, bitRead(tempBlock.line2, 3));
			bitWrite(rotatedBlock.line1, 0, bitRead(tempBlock.line1, 3));

			bitWrite(rotatedBlock.line1, 0, bitRead(tempBlock.line1, 3));
			bitWrite(rotatedBlock.line2, 0, bitRead(tempBlock.line1, 2));
			bitWrite(rotatedBlock.line3, 0, bitRead(tempBlock.line1, 1));
			bitWrite(rotatedBlock.line4, 0, bitRead(tempBlock.line1, 0));

			bitWrite(rotatedBlock.line4, 0, bitRead(tempBlock.line1, 0));
			bitWrite(rotatedBlock.line4, 1, bitRead(tempBlock.line2, 0));
			bitWrite(rotatedBlock.line4, 2, bitRead(tempBlock.line3, 0));
			bitWrite(rotatedBlock.line4, 3, bitRead(tempBlock.line4, 0));

			bitWrite(rotatedBlock.line4, 3, bitRead(tempBlock.line4, 0));
			bitWrite(rotatedBlock.line3, 3, bitRead(tempBlock.line4, 1));
			bitWrite(rotatedBlock.line2, 3, bitRead(tempBlock.line4, 2));
			bitWrite(rotatedBlock.line1, 3, bitRead(tempBlock.line4, 3));

			bitWrite(rotatedBlock.line2, 2, bitRead(tempBlock.line3, 2));
			bitWrite(rotatedBlock.line2, 1, bitRead(tempBlock.line2, 2));

			bitWrite(rotatedBlock.line2, 1, bitRead(tempBlock.line2, 2));
			bitWrite(rotatedBlock.line3, 1, bitRead(tempBlock.line2, 1));

			bitWrite(rotatedBlock.line3, 1, bitRead(tempBlock.line2, 1));
			bitWrite(rotatedBlock.line3, 2, bitRead(tempBlock.line3, 1));

			bitWrite(rotatedBlock.line3, 2, bitRead(tempBlock.line3, 1));
			bitWrite(rotatedBlock.line2, 2, bitRead(tempBlock.line3, 2));

			tempBlock = rotatedBlock;
		}

		// Force boundaries
		for (int c = 0; c < 3; c++){

			// Left boundary
			if (bitRead(rotatedBlock.line2, 3 - c) && (colPos + c < 0)){
				colPos++;

				updateFrame();
			}
			if (bitRead(rotatedBlock.line3, 3 - c) && (colPos + c < 0)){
				colPos++;
				updateFrame();
			}

			// Right boundary
			if (bitRead(rotatedBlock.line2, 0 + c) && (colPos + 3 - c > MAX_COL)){
				colPos--;
				updateFrame();
			}
			if (bitRead(rotatedBlock.line3, 0 + c) && (colPos + 3 - c > MAX_COL)){
				colPos--;
				updateFrame();
			}

		}

		// Reverse-rotate block to prevent looping due to left and right collisions
		if (blockShiftCounter > 2){
			rotation = (rotation == 0) ? 3 : rotation - 1;
			blockShiftCounter = 0;
		}

		for (int c = 0; c < 2; c++){
			// Collision with other pieces
			if (!allowPlacement){

				// Collision left
				if (bitRead(rotatedBlock.line1, 3 - c) && logicMatrice[rowPos][colPos + c]){
					colPos++;
					blockShiftCounter++;
					updateFrame();
				}
				if (bitRead(rotatedBlock.line2, 3 - c) && logicMatrice[rowPos + 1][colPos + c]){
					colPos++;
					blockShiftCounter++;
					updateFrame();
				}
				if (bitRead(rotatedBlock.line3, 3 - c) && logicMatrice[rowPos + 2][colPos + c]){
					colPos++;
					blockShiftCounter++;
					updateFrame();
				}
				if (bitRead(rotatedBlock.line4, 3 - c) && logicMatrice[rowPos + 3][colPos + c]){
					colPos++;
					blockShiftCounter++;
					updateFrame();
				}

				// Collision right
				if (bitRead(rotatedBlock.line1, 0 + c) && logicMatrice[rowPos][colPos + 3 - c]){
					colPos--;
					blockShiftCounter++;
					updateFrame();
				}
				if (bitRead(rotatedBlock.line2, 0 + c) && logicMatrice[rowPos + 1][colPos + 3 - c]){
					colPos--;
					blockShiftCounter++;
					updateFrame();
				}
				if (bitRead(rotatedBlock.line3, 0 + c) && logicMatrice[rowPos + 2][colPos + 3 - c]){
					colPos--;
					blockShiftCounter++;
					updateFrame();
				}
				if (bitRead(rotatedBlock.line4, 0 + c) && logicMatrice[rowPos + 3][colPos + 3 - c]){
					colPos--;
					blockShiftCounter++;
					updateFrame();
				}
				blockShiftCounter = 0;
			}
		}

		// Check if next position is conflicting from (1) overlapping a placed block or (2) reached the bottom row
		for (int c = 0; c < 4; c++){
			if (bitRead(rotatedBlock.line1, c) && (logicMatrice[rowPos][colPos + (3 - c)] || (rowPos > MAX_ROW))) posConflict = true;
			if (bitRead(rotatedBlock.line2, c) && (logicMatrice[rowPos + 1][colPos + (3 - c)] || (rowPos + 1 > MAX_ROW))) posConflict = true;
			if (bitRead(rotatedBlock.line3, c) && (logicMatrice[rowPos + 2][colPos + (3 - c)] || (rowPos + 2 > MAX_ROW))) posConflict = true;
			if (bitRead(rotatedBlock.line4, c) && (logicMatrice[rowPos + 3][colPos + (3 - c)] || (rowPos + 3 > MAX_ROW))) posConflict = true;
		}

		// Rever to previous row position if next position conflicted
		if (posConflict)
			rowPos--;

		// Display block
		for (int c = 0; c < 4; c++){
			if ((rowPos >= 0) && (rowPos <= MAX_ROW)){
				if (bitRead(rotatedBlock.line1, c)){
					tetris_strip.setPixelColor(PixelMatrice[rowPos][colPos + (3 - c)], BlockListColor[currentBlock]);
					// Set logicMatrice if conflicting
					if (posConflict) logicMatrice[rowPos][colPos + (3 - c)] = 1;
				}
			}
			if ((rowPos + 1 >= 0) && (rowPos + 1 <= MAX_ROW)){
				if (bitRead(rotatedBlock.line2, c)){
					tetris_strip.setPixelColor(PixelMatrice[rowPos + 1][colPos + (3 - c)], BlockListColor[currentBlock]);
					if (posConflict) logicMatrice[rowPos + 1][colPos + (3 - c)] = 1;
				}
			}
			if ((rowPos + 2 >= 0) && (rowPos + 2 <= MAX_ROW)){
				if (bitRead(rotatedBlock.line3, c)){
					tetris_strip.setPixelColor(PixelMatrice[rowPos + 2][colPos + (3 - c)], BlockListColor[currentBlock]);
					if (posConflict) logicMatrice[rowPos + 2][colPos + (3 - c)] = 1;
				}
			}
			if ((rowPos + 3 >= 0) && (rowPos + 3 <= MAX_ROW)){
				if (bitRead(rotatedBlock.line4, c)){
					tetris_strip.setPixelColor(PixelMatrice[rowPos + 3][colPos + (3 - c)], BlockListColor[currentBlock]);
					if (posConflict) logicMatrice[rowPos + 3][colPos + (3 - c)] = 1;
				}
			}
		}
	}
	else if (currentBlock >= 2){
		Block_3x3 tempBlock = *BlockList_3x3[currentBlock];
		Block_3x3 rotatedBlock = tempBlock;

		// Block Structure
		/*        2 1 0 (bit)
		*  line1
		*  line2
		*  line3
		*  (variable)
		*/

		// Rotate block
		for (int i = 0; i < rotation; i++){
			bitWrite(rotatedBlock.line1, 2, bitRead(tempBlock.line3, 2));
			bitWrite(rotatedBlock.line1, 1, bitRead(tempBlock.line2, 2));
			bitWrite(rotatedBlock.line1, 0, bitRead(tempBlock.line1, 2));

			bitWrite(rotatedBlock.line1, 0, bitRead(tempBlock.line1, 2));
			bitWrite(rotatedBlock.line2, 0, bitRead(tempBlock.line1, 1));
			bitWrite(rotatedBlock.line3, 0, bitRead(tempBlock.line1, 0));

			bitWrite(rotatedBlock.line3, 0, bitRead(tempBlock.line1, 0));
			bitWrite(rotatedBlock.line3, 1, bitRead(tempBlock.line2, 0));
			bitWrite(rotatedBlock.line3, 2, bitRead(tempBlock.line3, 0));

			bitWrite(rotatedBlock.line3, 2, bitRead(tempBlock.line3, 0));
			bitWrite(rotatedBlock.line2, 2, bitRead(tempBlock.line3, 1));
			bitWrite(rotatedBlock.line1, 2, bitRead(tempBlock.line3, 2));

			tempBlock = rotatedBlock;
		}

		// Force Boundary and prevent collision with other pieces
		if (!allowPlacement){
			byte hasCollision[] = { 0, 0, 0 };

			for (int c = 0; c < 3; c++){

				if (bitRead(rotatedBlock.line1, 2 - c) && (logicMatrice[rowPos][colPos + c] || ((colPos + c < 0) || (colPos + c > MAX_COL))))
					hasCollision[c] = 1;
				if (bitRead(rotatedBlock.line2, 2 - c) && (logicMatrice[rowPos + 1][colPos + c] || ((colPos + c < 0) || (colPos + c > MAX_COL))))
					hasCollision[c] = 1;
				if (bitRead(rotatedBlock.line3, 2 - c) && (logicMatrice[rowPos + 2][colPos + c] || ((colPos + c < 0) || (colPos + c > MAX_COL))))
					hasCollision[c] = 1;
			}

			// Revert colPos based on previous command
			if (hasCollision[0] || (hasCollision[1] || hasCollision[2])){
				if ((previousCommand == "BRight") || (previousCommand == "BLeft")){
					if (previousCommand == "BRight")
						colPos--;
					else if (previousCommand == "BLeft")
						colPos++;

					updateFrame();
				}
				else if (previousCommand == "BUp"){
					// Prevent endless loop by reverting rotation if block does not fit within +-1 column range
					if (hasCollision[0] && (blockShiftCounter < 1)){
						blockShiftCounter++;
						colPos++;
						updateFrame();
					}
					else if (hasCollision[2] && (blockShiftCounter < 3)){
						blockShiftCounter++;
						colPos--;
						updateFrame();
					}
					else if (blockShiftCounter >= 3){
						colPos++;
						rotation--;
						blockShiftCounter = 0;
						updateFrame();
					}
				}
			}
			blockShiftCounter = 0;

		}

		// Check if next position is conflicting from (1) overlapping a placed block or (2) reached the bottom row
		for (int c = 0; c < 3; c++){
			if (bitRead(rotatedBlock.line1, c) && (logicMatrice[rowPos][colPos + (2 - c)] || (rowPos > MAX_ROW))) posConflict = true;
			if (bitRead(rotatedBlock.line2, c) && (logicMatrice[rowPos + 1][colPos + (2 - c)] || (rowPos + 1 > MAX_ROW))) posConflict = true;
			if (bitRead(rotatedBlock.line3, c) && (logicMatrice[rowPos + 2][colPos + (2 - c)] || (rowPos + 2 > MAX_ROW))) posConflict = true;
		}

		// Revert to previous row position if next position conflicted
		if (posConflict)
			rowPos--;

		for (int c = 0; c < 3; c++){
			if (((colPos + (2 - c)) <= MAX_COL) && ((colPos + (2 - c)) >= 0)){
				if ((rowPos >= 0) && (rowPos <= MAX_ROW)){
					if (bitRead(rotatedBlock.line1, c)){
						tetris_strip.setPixelColor(PixelMatrice[rowPos][colPos + (2 - c)], BlockListColor[currentBlock]);
						if (posConflict) logicMatrice[rowPos][colPos + (2 - c)] = 1;
					}
				}
				if ((rowPos + 1 >= 0) && (rowPos + 1 <= MAX_ROW)){
					if (bitRead(rotatedBlock.line2, c)){
						tetris_strip.setPixelColor(PixelMatrice[rowPos + 1][colPos + (2 - c)], BlockListColor[currentBlock]);
						if (posConflict) logicMatrice[rowPos + 1][colPos + (2 - c)] = 1;
					}
				}
				if ((rowPos + 2 >= 0) && (rowPos + 2 <= MAX_ROW)){
					if (bitRead(rotatedBlock.line3, c)){
						tetris_strip.setPixelColor(PixelMatrice[rowPos + 2][colPos + (2 - c)], BlockListColor[currentBlock]);
						if (posConflict) logicMatrice[rowPos + 2][colPos + (2 - c)] = 1;
					}
				}
			}
		}

	}
	tetris_strip.show();

	if (posConflict) {
		Serial2.print("BLOCK1.");
		clearLines();

		if (logicMatrice[2][5] || (logicMatrice[2][6] || logicMatrice[2][7])){
			while (true);
		}
		else
			generateNewBlock();

	}
}



void Tetris::clearLines(){
	bool clearLineFlag = false;

	byte rowCleared[] = { 1, 1, 1, 1 };
	for (int r = 0; r < 4; r++){
		if (r + rowPos <= MAX_ROW){
			for (int c = 0; c <= MAX_COL; c++){
				rowCleared[r] = rowCleared[r] & logicMatrice[r + rowPos][c];
			}
		}
		else
			rowCleared[r] = 0;
	}

	// Set cleared lines to white
	for (int r = 0; r < 4; r++){
		if (rowCleared[r]){
			clearLineFlag = true;
			for (int c = 0; c <= MAX_COL; c++){
				tetris_strip.setPixelColor(PixelMatrice[r + rowPos][c], white);
			}
		}
	}

	if (clearLineFlag){
		Serial2.print("BLOCK4.");
		tetris_strip.show();
		delay(1000);
	}

	if (clearLineFlag){

		// Clear cleared lines
		for (int r = 0; r < 4; r++){
			if (rowCleared[r]){
				// Set row to white
				for (int c = 0; c <= MAX_COL; c++){
					tetris_strip.setPixelColor(PixelMatrice[r + rowPos][c], 0);
				}
			}
		}
		tetris_strip.show();
		delay(100);

		for (int i = 3; i >= 0; i--){
			if (rowCleared[i]){
				// Start shifting ever row down 1
				//Serial.print("Row: ");
				//Serial.println(rowPos);
				//Serial.println((rowPos + i));
				for (int r = rowPos + i; r > 1; r--){
					for (int c = 0; c <= MAX_COL; c++){
						logicMatrice[r][c] = logicMatrice[r - 1][c];
						tetris_strip.setPixelColor(PixelMatrice[r][c], tetris_strip.getPixelColor(PixelMatrice[r - 1][c]));
					}
					tetris_strip.show();
				}
				Serial2.print("BLOCK5.");
				delay(100);
				rowPos++; // increase starting position by 1
			}
		}

	}
}




void Tetris::generateNewBlock(){

	// Get Random Color
	int varRandom;
	for (int i = 0; i < (analogRead(0) % 10); i++) varRandom = random(0, sizeof(ColorPick) / 4);
	randomBlockColor = ColorPick[varRandom];

	// Get Random Block
	currentBlock = random(0, MAX_BLOCK);
	//currentBlock = random(1,2);
	//Serial.println(currentBlock);

	// Starting position
	rowPos = 0;
	colPos = 5;
	rotation = 0;

	updateFrame();
}



void Tetris::clearStrip(int initialRow, int length){
	if (initialRow > 0) initialRow--;

	for (int c = 0; c < 14; c++){
		for (int r = 0; r < length; r++){
			if (initialRow + r <= MAX_ROW){
				if (!logicMatrice[initialRow + r][c]) tetris_strip.setPixelColor(PixelMatrice[initialRow + r][c], 0);
			}
		}
	}
}



void Tetris::clearLogicMatrice(){
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 14; j++) logicMatrice[i][j] = 0;
	}
}