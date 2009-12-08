#include <iostream>
#include <string>
#include <queue> 
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

int main(int argc, const char** argv)
{                     
	//The storage for the points we read from stdin
	string input_line; 
	vecQueue inDataPoints;             
	
	//The ultimate results we care about.
	vecQueue outDataPoints;             
	
	//Start the loop that will take stdin.
	while(cin) 
	{
		try
		{
			getline(cin, input_line);
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