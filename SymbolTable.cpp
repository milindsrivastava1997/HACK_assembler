#include "SymbolTable.h"
#include <string>
#include <unordered_map>
using namespace std;

SymbolTable::SymbolTable() // mnemonics defined by HACK language
{
	insert("SP", 0);
	insert("LCL", 1);
	insert("ARG", 2);
	insert("THIS", 3);
	insert("THAT", 4);
	insert("R0", 0);
	insert("R1", 1);
	insert("R2", 2);
	insert("R3", 3);
	insert("R4", 4);
	insert("R5", 5);
	insert("R6", 6);
	insert("R7", 7);
	insert("R8", 8);
	insert("R9", 9);
	insert("R10", 10);
	insert("R11", 11);
	insert("R12", 12);
	insert("R13", 13);
	insert("R14", 14);
	insert("R15", 15);
	insert("SCREEN", 16384);
	insert("KBD", 24576);
}

void SymbolTable::insert(string key, int value)
{
	table.insert(make_pair(key, value));
}

int SymbolTable::search(string key)
{
	unordered_map<string, int>::iterator it = table.find(key);
	if(it == table.end())
	{
		return -1;
	}
	return it->second;
}