#include <iostream>
#include <string>
#include <queue>
#include <math.h>
#include <assert.h> 
#include <sstream>
#include <cstdlib>
#include <vector>
#include <exception>

using namespace std; 
//Takes a csv list and builds a vector of doubles out of it.
std::vector<double> parseStringToVector(std::string);

//An exception to throw if parsing goes wrong.
class badValue : public exception
{ 
public:  
	virtual const char* what() const throw() {return "Bad values encountered while parsing input.";}
}; 

//Converts strings to type toT
template<typename toT> toT stringConvert(std::string);

//Just to make the code a little nicer...
typedef std::queue<std::vector<double> > vecQueue;

//Random number from 0 to 1
double uniformUnit() { return rand()/static_cast<double>(RAND_MAX); }

//Functor which calculates the magnitude of a vector (measured from 0)
struct magVector
{
	double operator()(std::vector<double> target) 
	{
		double result(0);
		for (std::size_t i=0; i < target.size(); i++) result += target[i]*target[i];
		return sqrt(result);		
	}
} radius;

//Functor which can add std::vector<double>s using std::accumulate
struct addVectors
{
	std::vector<double> operator()(std::vector<double> a, std::vector<double> b)
	{
		assert(a.size() == b.size());
		std::vector<double> result(a.size(),0);
		for(std::size_t i=0; i < a.size(); i++) result[i] = a[i] + b[i]; 
		return result;
	}
} vectorsum;

//Functor that uses rand() to get a random vector lying on the surface of a 
//sphere of a given radius.
struct sphereRand
{
	std::vector<double> operator()(double radius)
	{
		std::vector<double> result(3,0);
		double cos_phi = 1.0-2.0*uniformUnit();
		double theta = 2.0*M_PI*uniformUnit();
		result[0] = sqrt(1.0-cos_phi*cos_phi)*cos(theta);
		result[1] = sqrt(1.0-cos_phi*cos_phi)*sin(theta);
		result[2] = cos_phi;
		return result;
	}
} uniformOnSphere;

//Functor that calculates the appropriate light cone radius given a time in
//nanoseconds.
struct lightCone
{
	double operator()(double nstime) { return 299.792458*nstime; }
} radiusFromTime;

int main(int argc, const char** argv)
{                     
	//The storage for the points we read from stdin
	string input_line; 
	vecQueue inDataPoints;             
	
	//The ultimate results we care about.
	vecQueue outDataPoints;
	
	//The maximum hit time
	double maxT(0);
	
	//Start the loop that will take stdin.
	while(cin) 
	{
		try
		{
			getline(cin, input_line);
			//We need to capture what the maximum hit time is, so here
			//we store the parsed string as a temporary before pushing it
			//onto the queue.
			std::vector<double> tempParsedVector = parseStringToVector(input_line);
			if ( tempParsedVector[3] > maxT ) maxT = tempParsedVector[3];
			inDataPoints.push(parseStringToVector(input_line));
		}
		catch (badValue &bV)
		{
			cout << bV.what() << endl;
			exit(5);
		}
		catch (...)
		{
			cout << "An exception occurred during stream processing." << endl;
			exit(6);
		}
	}
	
	//Assuming we've had no issues, now we can start producing random points
	//centered around the PMTs.
	//The algorithm is straightforward:
	//	1) grab the PMT x,y,z,t vector
	//	2) generate random points centered at the origin with the appropriate
	//	   radius determined by the time the pmt was hit
	//	3) add random point to PMT position, accept if final radius is
	//     less than 6m (i.e. in the AV)
	//  4) push to output queue
	std::vector<double> tempSpatialVector(inDataPoints.front().size() - 1,0);
	double tempTime; 
	double tempLightCone;
	while ( inDataPoints.empty() == false )
	{ 
		//Grab the spacetime coordinates out of the front of the data queue.
		for(std::size_t i=0; i < inDataPoints.front().size() - 1; i++) tempSpatialVector[i] = inDataPoints.front()[i];
		tempTime = maxT-inDataPoints.front()[3]; //Hack the time to make 0 time work?
		if (tempTime <= 300) //Maximum transit time for non-reflected photons.
		{
			tempLightCone = radiusFromTime(tempTime);   
			//Now throw 100 random numbers inside the AV and push them onto the
			//output queue.
			for(std::size_t j=0; j < 101; j++)
			{  
				bool pointGood = false;
				std::vector<double> tempVec(3,0);
				while( pointGood == false )
				{      
					tempVec = uniformOnSphere(tempLightCone);
					tempVec = vectorsum(tempVec, tempSpatialVector);
					if ( radius(tempVec) <= 10000 ) pointGood = true;
				}
				outDataPoints.push(tempVec);
			}                     
		}
		inDataPoints.pop();
	}
	
	//Now print every point in the output queue.
	while ( outDataPoints.empty() == false )
	{
		cout << outDataPoints.front()[0]
			<< ","
			<< outDataPoints.front()[1]
			<< ","
			<< outDataPoints.front()[2]
			<< endl;
		outDataPoints.pop();
	}
	
	return 0;
}

std::vector<double> parseStringToVector(std::string theString)
{                                 
	std::vector<double> theResult(4,0);
	//Basically, search for commas starting from the end, and at each one
	//create a substring, parse it to a double, and push it onto the front
	//of the vector.
	std::size_t lastComma = theString.find_last_of(',');
	
	//Keep track of how many numbers we have actually pulled out of the stream.
	std::size_t numIndex(0);
	while (lastComma != string::npos)
	{    
		//If we've already pulled out 4 numbers, something is wrong.
		if (numIndex > 3) throw badValue();                       
		//Grab the substring formed by the comma and npos, and convert it to a double.
		theResult[3-numIndex] = stringConvert<double>(theString.substr(lastComma + 1));
		//Increment the counter and chop the string up.
		numIndex++;
		theString.resize(lastComma);
		//Find the next comma.
		lastComma = theString.find_last_of(',');		
	}
	if ( theString.size() != 0 && lastComma == string::npos )
	{   
		theResult[3-numIndex] = stringConvert<double>(theString);
	}
	//Return.
	return theResult;
}

template<typename toT> toT stringConvert(std::string target)
{
	toT result;
	std::istringstream sstreamBuf(target);
	sstreamBuf >> result; 
	return result;
}