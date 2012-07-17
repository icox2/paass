/*****************************************
Source File for the Valid Processor, used
to output the MoNA timestamps for the 
VANDLE-MoNA experiment at MSU.
   S.V. Paulauskas 20 June 2012
********************************************/
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include <cmath>

#include "DetectorDriver.hpp"
#include "RawEvent.hpp"
#include "ValidProcessor.hpp"

using namespace std;

stringstream ValidProcessor::fileName;

ValidProcessor::ValidProcessor(): EventProcessor()
{
    name = "Valid";
    associatedTypes.insert("valid"); 

}

bool ValidProcessor::Process(RawEvent &event) 
{
    if (!EventProcessor::Process(event))
	return false;

    OutputData(event);
    EndProcess();
    return true;
}

void ValidProcessor::OutputData(RawEvent &event)
{   
    static const vector<ChanEvent*> & validEvents = 
	event.GetSummary("valid")->GetList();

    
    //Optimize this later!!!! Spends too much time
    //in the open/close operations.
    if(fileName.str() == "") {
	char hisFileName[32];
	GetArgument(1, hisFileName, 32);
	string temp = hisFileName;
	temp = temp.substr(0, temp.find_first_of(" "));
	fileName << "monaTimeStamps-" << temp << ".dat";
    }

    ofstream data;
    data.open(fileName.str().c_str(), fstream::app);      
    
    for(vector<ChanEvent*>::const_iterator itValid = validEvents.begin();
	itValid != validEvents.end(); itValid++) {
	
        string type = (*itValid)->GetChanID().GetType();
	bool hasTag = (*itValid)->GetChanID().GetTag("output");
	unsigned int location = (*itValid)->GetChanID().GetLocation();
	unsigned int long timeLow = (*itValid)->GetQdcValue(0);
	unsigned int long timeHigh = (*itValid)->GetQdcValue(1);
	if(hasTag)
	    data << type << " " << location << " " 
		 << timeLow << " " << timeHigh << endl;
    }
    data.close();
}//void ValidProcessor::OutputData