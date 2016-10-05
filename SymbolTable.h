#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include <unordered_map>
#include <string>
using namespace std;

class SymbolTable
{
private:
	unordered_map<string, int> table;

public:
	SymbolTable();
	void insert(string, int);
	int search(string);
};

#endif