#include "blockchain/CChain.h"
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <signal.h>

using namespace std;
using namespace blockchain;

CChain* gChain;

void interruptCallback(int sig)
{
	gChain->stop();
}

int main(int argc, char **argv)
{
	cout << "Start.\n";

	CChain chain(false, "127.0.0.1", 1, storage::EST_LOCAL);
	gChain = &chain;

	cout << "Chain intialized!\n";
	cout << "Current block count: " << chain.getBlockCount() << "\n";

	if(chain.isValid())
		cout << "Chain is valid!\n";
	else
	{
		cout << "INVALID CHAIN\n";
		return 1;
	}

	CBlock* current = chain.getCurrentBlock();

	uint8_t* garbage = new uint8_t[32];
	for(uint32_t n = 0; n < 32; n++)
		garbage[n] = clock() % 255;

	cout << "Garbage generated.\n";
	
	chain.appendToCurrentBlock(garbage, 32);
	delete[] garbage;

	cout << "Garbage appended to current block.\n";

	chain.nextBlock();
	
	cout << "Next block mined.\n";

	cout << "Current Hash: " << chain.getCurrentBlock()->getPrevBlock()->getHashStr() << "\nNonce: " << chain.getCurrentBlock()->getNonce() << "\n";

	garbage = new uint8_t[32];
	for(uint32_t n = 0; n < 32; n++)
		garbage[n] = clock() % 255;

	cout << "Garbage generated.\n";
	
	chain.appendToCurrentBlock(garbage, 32);
	delete[] garbage;

	cout << "Garbage appended to current block.\n";	

	chain.nextBlock();
	
	cout << "Next block mined.\n";

	cout << "Previous Hash: " << chain.getCurrentBlock()->getPrevBlock()->getHashStr() << "\nNonce: " << chain.getCurrentBlock()->getNonce() << "\n";

	cout << "Current block count: " << chain.getBlockCount() << "\n";

	cout << "\n" << "## BLOCK LIST (Descending)" << "\n";

	CBlock* cur = chain.getCurrentBlock();
	while(cur = cur->getPrevBlock())
	{
		time_t ts = cur->getCreatedTS();
		string tstr(ctime(&ts));
		tstr.resize(tstr.size()-1);

		cout << "Block " << cur->getHashStr() << "\tTimeStamp " << tstr << "\tData Size " << cur->getDataSize() << "\n";
	}

	// Interrupt Signal
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = interruptCallback;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	sigaction(SIGQUIT, &sigIntHandler, NULL);

	while(chain.isRunning())
		usleep(5000);

	cout << "\nExit.\n";
	
	return 0;
}
