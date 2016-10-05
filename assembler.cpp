#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include "SymbolTable.h"
using namespace std;

enum COMMANDS
{
	A_COMMAND = 0,
	C_COMMAND,
	L_COMMAND,
	COMMENT
};

int commandType(string); // return type of command
string symbol(string, int); // extracts symbol from A_COMMAND or L_COMMAND
void comp(string, char*, int); // computed the bits of the instruction related to computation
void dest(string, char*, int); // computed the bits of the instruction related to destination
void jump(string, char*, int); // computed the bits of the instruction related to jump

SymbolTable myTable; // object to hold all labels and variables

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		cerr << "Incorrect number of arguments to program" << endl << "Exiting" << endl;
		return 0;
	}

	/*
	Example file names:

	asmFile: testfiles/Add.asm
	asmCleanFile: testfiles/Add_clean.asm
	hackFile: outputs/Add.hack

	*/

	// ATTENTION : input file must be terminated with a newline

	ifstream asmFile; // input file
	fstream asmCleanFile; // clean file generated from input file
	ofstream hackFile; // output binary file
	
	string hackFileName(argv[1]);
	string asmCleanFileName(argv[1]);

	// manipulation to get name of hackFile and asmCleanFile

	hackFileName.erase(hackFileName.begin(), hackFileName.begin() + hackFileName.find('/'));
	hackFileName.erase(hackFileName.end() - 3, hackFileName.end());
	hackFileName.append("hack");
	hackFileName.insert(0, "outputs");

	asmCleanFileName.insert(asmCleanFileName.rfind('.'), "_clean");

	asmFile.open(argv[1], ios::in);	
	asmCleanFile.open(asmCleanFileName.c_str(), ios::trunc | ios::out);

	asmFile.seekg(0);
	asmCleanFile.seekp(0);

	string instruction; // stores instruction

	int instructionNumber = 0; // stores number of each instruction starting from 0

	// First Pass: Store lables in symbol table

	while(true)
	{
		getline(asmFile, instruction, '\n');

		instruction = regex_replace(instruction, regex("\\s+"), string("")); // remove all whitespaces

		if(asmFile.eof()) // break if eof has been read
		{
			break;
		}

		if(instruction.size() == 0) // null instruction
		{
			continue;
		}

		int command = commandType(instruction); // type of instruction

		if(command == L_COMMAND) // add label to symbol table, if not present already
		{
			string label = symbol(instruction, L_COMMAND);
			if(myTable.search(label) == -1)
			{
				myTable.insert(label, instructionNumber);
			}
		}
		else if(command == A_COMMAND || command == C_COMMAND) // remove comments if any and output to asmCleanFile
		{			
			int commentPosition = instruction.find("//");
			if(commentPosition != -1)
			{
				instruction.erase(instruction.begin() + commentPosition, instruction.end());
			}

			asmCleanFile << instruction << endl;

			instructionNumber++;
		}
	}

	asmFile.close();
	asmCleanFile.close();

	asmCleanFile.open(asmCleanFileName, ios::in); // re-open as input file
	hackFile.open(hackFileName.c_str(), ios::trunc | ios::out);

	asmCleanFile.seekg(0);
	hackFile.seekp(0);

	int lineNumber = 0;
	int varAddress = 16; // address to allocate to new variables

	while(true)
	{
		getline(asmCleanFile, instruction, '\n');

		instruction = regex_replace(instruction, regex("\\s+"), string("")); //remove all whitespaces
		
		if(asmCleanFile.eof()) // break if eof has been read
		{
			break;
		}

		if(instruction.size() == 0)
		{
			continue;
		}

		lineNumber++;

		int command = commandType(instruction);

		if(command == L_COMMAND || command == COMMENT)
		{
			continue;
		}
		if(command == A_COMMAND)
		{
			string label = symbol(instruction, A_COMMAND);
			int value;

			int i;
			for(i = 0; i < label.size(); i++)
			{
				if(label[i] >= '0' && label[i] <= '9')
				{
					continue;
				}
				else
				{
					break;
				}
			}

			if(i == label.size()) // if number following "@" is a number, store it in value
			{
				value = atoi(label.c_str());
			}
			else // else search for it in symbol table
			{
				value = myTable.search(label);

				if(value == -1) // if not found in symbol table, add it to symbol table as a new variable
				{
					myTable.insert(label, varAddress);
					value = varAddress;
					varAddress++;
				}
			}

			char binaryValue[15]; // stores binary value of "value"
			memset(binaryValue, '0', sizeof(binaryValue)); // set all elements to 0

			int counter = 0;

			while(value > 0) // convert "value" to binary
			{
				if(value % 2 == 1)
				{
					binaryValue[counter] = '1';
				}
				counter++;
				value /= 2;
			}

			hackFile << "0"; // print 0 to denote A-instruction

			for(int i = 14; i >= 0; i--) // print to hackFile in reverse
			{
				hackFile << binaryValue[i];
			}
			hackFile << endl;
		}
		else
		{
			char i_comp[8];
			char i_dest[4];
			char i_jump[4];

			comp(instruction, i_comp, lineNumber);
			dest(instruction, i_dest, lineNumber);
			jump(instruction, i_jump, lineNumber);

			hackFile << "111" << i_comp << i_dest << i_jump << endl; // output "111" to hackFile to denote C-instruction followed by respective bits for computation, destination and jump
		}
	}

	asmCleanFile.close();
	hackFile.close();

	return 0;
}

int commandType(string input)
{
	if(input[0] == '@')
	{
		return A_COMMAND;
	}
	if(input[0] == '(')
	{
		return L_COMMAND;
	}
	if(input[0] == '/' && input[1] == '/')
	{
		return COMMENT;
	}
	return C_COMMAND;
}

string symbol(string input, int command)
{
	if(command == L_COMMAND) // symbol is obtained by removing parentheses
	{
		string temp(input.begin() + 1, input.end() - 1);
		return temp;
	}
	else if(command == A_COMMAND) // symbol is obtained by removing "@" symbol
	{
		string temp(input.begin() + 1, input.end());
		return temp;
	}
}

void comp(string input, char* i_comp, int lineNumber)
{
	int found1 = input.find('='); // store position of "="
	int found2 = input.find(';'); // store position of ';'

	i_comp[7] = '\0';
	i_comp[0] = i_comp[1] = i_comp[2] = i_comp[3] = i_comp[4] = i_comp[5] = i_comp[6] = '0'; // set all to 0 by dsefault


	if(found1 == string::npos)
	{
		found1 = -1;
	}
	if(found2 == string::npos)
	{
		found2 = input.size();
	}

	string comp(input.begin() + found1 + 1, input.begin() + found2); // contains the string between '=' and ';' i.e part of instruction that denotes computation
	
	// compare with all possible mnemonics

	if(!comp.compare("0"))
	{
		i_comp[1] = i_comp[3] = i_comp[5] = '1';
	}
	else if(!comp.compare("1"))
	{
		i_comp[1] = i_comp[2] = i_comp[3] = i_comp[4] = i_comp[5] = i_comp[6] = '1';
	}
	else if(!comp.compare("-1"))
	{
		i_comp[1] = i_comp[2] = i_comp[3] = i_comp[5] = '1';
	}
	else if(!comp.compare("D"))
	{
		i_comp[3] = i_comp[4] = '1';
	}
	else if(!comp.compare("A"))
	{
		i_comp[1] = i_comp[2] = '1';
	}
	else if(!comp.compare("!D"))
	{
		i_comp[3] = i_comp[4] = i_comp[6] = '1';
	}
	else if(!comp.compare("!A"))
	{
		i_comp[1] = i_comp[2] = i_comp[6] = '1';
	}
	else if(!comp.compare("-D"))
	{
		i_comp[3] = i_comp[4] = i_comp[5] = i_comp[6] = '1';
	}
	else if(!comp.compare("-A"))
	{
		i_comp[1] = i_comp[2] = i_comp[5] = i_comp[6] = '1';
	}
	else if(!comp.compare("D+1"))
	{
		i_comp[2] = i_comp[3] = i_comp[4] = i_comp[5] = i_comp[6] = '1';
	}
	else if(!comp.compare("A+1"))
	{
		i_comp[1] = i_comp[2] = i_comp[4] = i_comp[5] = i_comp[6] = '1';
	}
	else if(!comp.compare("D-1"))
	{
		i_comp[3] = i_comp[4] = i_comp[5] = '1';
	}
	else if(!comp.compare("A-1"))
	{
		i_comp[1] = i_comp[2] = i_comp[5] = '1';
	}
	else if(!comp.compare("D+A"))
	{
		i_comp[5] = '1';
	}
	else if(!comp.compare("D-A"))
	{
		i_comp[2] = i_comp[5] = i_comp[6] = '1';
	}
	else if(!comp.compare("A-D"))
	{
		i_comp[4] = i_comp[5] = i_comp[6] = '1';
	}
	else if(!comp.compare("D&A"))
	{
		;
	}
	else if(!comp.compare("D|A"))
	{
		i_comp[2] = i_comp[4] = i_comp[6] = '1';
	}
	else if(!comp.compare("M"))
	{
		i_comp[0] = i_comp[1] = i_comp[2] = '1';
	}
	else if(!comp.compare("!M"))
	{
		i_comp[0] = i_comp[1] = i_comp[2] = i_comp[6] = '1';
	}
	else if(!comp.compare("-M"))
	{
		i_comp[0] = i_comp[1] = i_comp[2] = i_comp[5] = i_comp[6] = '1';
	}
	else if(!comp.compare("M+1"))
	{
		i_comp[0] = i_comp[1] = i_comp[2] = i_comp[4] = i_comp[5] = i_comp[6] = '1';
	}
	else if(!comp.compare("M-1"))
	{
		i_comp[0] = i_comp[1] = i_comp[2] = i_comp[5] = '1';
	}
	else if(!comp.compare("D+M"))
	{
		i_comp[0] = i_comp[5] = '1';
	}
	else if(!comp.compare("D-M"))
	{
		i_comp[0] = i_comp[2] = i_comp[5] = i_comp[6] = '1';
	}
	else if(!comp.compare("M-D"))
	{
		i_comp[0] = i_comp[4] = i_comp[5] = i_comp[6] = '1';
	}
	else if(!comp.compare("D&M"))
	{
		i_comp[0] = '1';
	}
	else if(!comp.compare("D|M"))
	{
		i_comp[0] = i_comp[2] = i_comp[4] = i_comp[6] = '1';
	}
	else
	{
		cerr << "Error in computation field at line " << lineNumber << endl;
		exit(0);
	}
}

void dest(string input, char* i_dest, int lineNumber)
{
	i_dest[3] = '\0';
	i_dest[0] = i_dest[1] = i_dest[2] = '0'; // set all to 0 by default

	int found = input.find('=');// store position of '='

	if(found == string::npos) // if '=' is not found, no destination is set
	{
		return;
	}

	string dest(input.begin(), input.begin() + found); // stores part of instruction that denotes destination
	
	if(dest.size() == 0)
	{
		cerr << "Error in destination field at line " << lineNumber << endl;
		exit(0);
	}
	for(int i = 0; i < dest.size(); i++)
	{
		switch(dest[i]) // set appropriate bits to 1
		{
			case 'A':
			{
				i_dest[0] = '1';
				break;
			}
			case 'D':
			{
				i_dest[1] = '1';
				break;
			}
			case 'M':
			{
				i_dest[2] = '1';
				break;
			}
			default:
			{
				cerr << "Error in destination field at line " << lineNumber << endl;
				exit(0);
			}
		}
	}
}

void jump(string input, char* i_jump, int lineNumber)
{
	i_jump[3] = '\0';
	i_jump[0] = i_jump[1] = i_jump[2] = '0'; // set all bits to 0 by default

	int found = input.find(';'); // stores position of ';'

	if(found == string::npos) // if not found, instruction doesn't contain jump instruction
	{
		return;
	}

	string jump(input.begin() + found + 2, input.begin() + found + 4); // stores part of instruction that denotes jump excluding the leading 'J' character
	
	// compare with all possible mnemonics

	if(!jump.compare("MP"))
	{
		i_jump[0] = i_jump[1] = i_jump[2] = '1';
	}
	else if(!jump.compare("GT"))
	{
		i_jump[2] = '1';
	}
	else if(!jump.compare("EQ"))
	{
		i_jump[1] = '1';
	}
	else if(!jump.compare("GE"))
	{
		i_jump[1] = i_jump[2] = '1';
	}
	else if(!jump.compare("LT"))
	{
		i_jump[0] = '1';
	}
	else if(!jump.compare("NE"))
	{
		i_jump[0] = i_jump[2] = '1';
	}
	else if(!jump.compare("LE"))
	{
		i_jump[0] = i_jump[1] = '1';
	}
	else
	{
		cerr << "Error in jump instruction at line " << lineNumber << endl;
		exit(0);
	}
}