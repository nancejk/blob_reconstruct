#include <iostream>
#include <iomanip>
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

// This section contains class definitions that are used
// as functors in the main body of the program.  Each one
// of the functors is a method of calculating the residual
// hit times based on the formula
// t_res = t_pmt - t_triggertime + d/c
// Using this method, the only portion of the code that needs
// to be modified if the method of residuals changes or expands
// is this one and the option parsing.
class ResidualBase
{
public:
	virtual double operator()(double) { return (0); };
};

// This functor literally does nothing, and should be the default.
// It does not calculate residual times at all!
class DoNothing: public ResidualBase
{
public:
	double operator()(double rawTime) { return rawTime; };
};

// This functor calculates the residual time based on the assumption
// that the event occurred at the center of the detector.  There are
// some constants in here such as the indices of refraction and 
// thicknesses of the elements of the geometry.  For now, it assumes
// a trigger time of 220ns.
class AssumeCentered: public ResidualBase
{
public:
	double operator()(double);
};

// This is the workhorse method of the AssumeCentered functor.
double AssumeCentered::operator()(double rawTime)
{
	// The trigger delay time.
	double triggerDelay = 220.0;

	// All figures taken from the RATDB geometry and optics files.
	// double avRadius(6.0053); // radius in m
	// double scAvgRI(1.4706); // scintillator average index of refraction
	// double acThickness(0.0551); // acrylic thickness in m
	// double acAvgRI(1.5075); // acrylic average index of refraction
	// double waterThickness(2.3406); // thickness of 'water layer'
	// double waterAvgRI(1.34775); // average index of refraction of h2o
	
	// The transit time, in nanoseconds, for a photon to get through
	// the three sections above.
	// double transitTime = (1.0/0.299792458)*(avRadius*scAvgRI + acThickness*acAvgRI + waterThickness*waterAvgRI);

	// Return the residual time.
	return triggerDelay - rawTime; // + transitTime;
}

int main(int argc, char **argv)
{
	// Set the output flags on cout.
	std::cout << std::fixed << std::setprecision(4) << std::endl;

	//Input section.  Just takes a single argument.
	char c = 0;
	char* inFile = NULL;
	int eventChosen = 0;
	bool inDef,eventDef;
	inDef = false;
	// This is the functor that will calculate the residual times.
	ResidualBase* timeCalc = 0;

    	while((c = getopt(argc, argv, "ci:e:")) != -1)
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
			case 'c':
				timeCalc = new AssumeCentered();
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

	// If the residual time functor hasn't been assigned to anything,
	// it should be a noop.
	if ( !timeCalc ) timeCalc = new DoNothing();
    
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
				<< (*timeCalc)(tempPMT->GetTime())
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
