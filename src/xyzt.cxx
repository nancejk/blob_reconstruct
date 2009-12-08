#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include "rootStuff.hpp"
#include "ratStuff.hpp"

using std::string;
using std::cout;
using std::endl;

typedef RAT::DS::RunStore runStore;
typedef RAT::DS::Root* ratRootPtr;
typedef RAT::DS::EV* ratEVPtr;
typedef RAT::DS::Run ratRun;
typedef RAT::DSReader ratReader;
typedef RAT::DS::PMTProperties* ratPMTP;
typedef RAT::DS::PMTUnCal* ratPMT;


// Returns true if a file exists.
bool checkFileExists(char*);

int main(int argc, char **argv)
{
	//Input section.  Just takes a single argument.
	char c = 0;
	char* inFile = NULL;
	int eventChosen = 0;
	bool inDef,eventDef;
	inDef = false;
    while((c = getopt(argc, argv, "i:e:")) != -1)
	{
		switch(c)
		{
			case 'i':
				inFile = optarg;
				inDef = true;
				break;
			case 'e':
				eventChosen = atoi(optarg);
				eventDef = true;
				break;
			default:
				abort();
		}
	}
	
	//Die if both input and output haven't been defined
	if ( !inDef )
	{
		cout << "You must define an input file with -i." << endl;
		exit(2);
	}
    
	if (checkFileExists(inFile)) 
	{   
		//Fire up a TChain and get the runT tree from the ROOT file.
		//We need this tree to preload the fetch cache for the RunStore so 
		//that we can get a PMTProperties object from it. 
		TChain ch("runT");
		ch.Add(inFile);
		ch.LoadTree(0);
		TTree* rTr = ch.GetTree();
		
		//Preload the cache so that runstore actually works.
		runStore::PreloadFromTree(rTr);                     

		//Open a DSReader object, grab a ratRoot object from it, and then
		//get the ratRun object associated with it.
		ratReader dsRead(inFile);
		
		//For now just deal with a single event.
		ratRootPtr rDS = NULL;
		ratEVPtr rEV = NULL;
		if(dsRead.GetTotal() >= eventChosen) rDS = dsRead.GetEvent(eventChosen);
		else 
		{   
			//If this is a user defined event we're looking for, let the user know
			//we can't find it.
			if (eventDef)
			{
				cout << "Could not find event " << eventChosen << endl;
			}
			else cout << "No events in file " << inFile << endl;   
			//Die.
			exit(2); 
		}
		//Get the PMT Properties for the run and get the event.  For now just 
		//use the first one.
		ratPMTP thePMTP = runStore::GetRun(rDS)->GetPMTProp(); 
		rEV = rDS->GetEV(0);
		
		//Iterate over every hit PMT and print X,Y,Z,T
		for(int i = 0; i < rEV->GetPMTUnCalCount(); i++) 
		{
			ratPMT tempPMT = rEV->GetPMTUnCal(i);
			cout << tempPMT->GetPos(thePMTP,tempPMT->GetID()).X() 
				<< "," 
				<< tempPMT->GetPos(thePMTP,tempPMT->GetID()).Y()
				<< ","
				<< tempPMT->GetPos(thePMTP,tempPMT->GetID()).Z()
				<< ","
				<< tempPMT->GetTime()
				<< endl;
		}
		
		//Deleting pointers here leads to segmentation faults - probably a 
		//double free.  ROOT must be up to some shenanigans...
	}
	else //The file doesn't exist.  Die semi-gracefully.
	{
		cout << "File " << inFile << " does not exist!" << endl;
		delete inFile;
		exit(2);
	}
	//Exit
	return 0;
}

bool checkFileExists(char* fileName)
{
	struct stat buf;
	int i = stat ( fileName, &buf );
	if (i == 0) return true;
	return false;
}