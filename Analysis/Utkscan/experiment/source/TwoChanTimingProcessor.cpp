/** \file TwoChanTimingProcessor.cpp
 * \brief Analyzes data from a simple Two channel Timing setup
 * \author S. V. Paulauskas
 * \date July 10, 2009
 */
#include <fstream>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "HighResTimingData.hpp"
#include "TwoChanTimingProcessor.hpp"

#include <TFile.h>
#include <TTree.h>
#include <TH1I.h>
#include <TH2I.h>

static HighResTimingData::HrtRoot rstart;
static HighResTimingData::HrtRoot rstop;

TFile *rootfile;
TTree *tree;
TH1I *codes;
TH2I *traces;

enum CODES {
    PROCESS_CALLED,
    WRONG_NUM
};

using namespace std;
using namespace dammIds::experiment;

TwoChanTimingProcessor::TwoChanTimingProcessor() :
        EventProcessor(OFFSET, RANGE, "TwoChanTimingProcessor") {
    associatedTypes.insert("pulser");

    trcfile.open(
            Globals::get()->AppendOutputPath(
                    Globals::get()->GetOutputFileName() + "-trc.dat").c_str());
    rootfile =
            new TFile(Globals::get()->AppendOutputPath(
                    Globals::get()->GetOutputFileName() + ".root").c_str(),
                      "RECREATE");

    tree = new TTree("timing", "");
    tree->Branch("start", &rstart,
                 "qdc/D:time:snr:wtime:phase:abase:sbase:id/b");
    tree->Branch("stop", &rstop, "qdc/D:time:snr:wtime:phase:abase:sbase:id/b");
    codes = new TH1I("codes", "", 40, 0, 40);
    traces = new TH2I("traces","",1000,0,1000,1000,0,1000);
}

TwoChanTimingProcessor::~TwoChanTimingProcessor() {
    codes->Write();
    rootfile->Write();
    rootfile->Close();
}

bool TwoChanTimingProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    //Define a map to hold the information
    TimingMap pulserMap;

    //plot the number of times we called the function
    codes->Fill(PROCESS_CALLED);

    static const vector<ChanEvent *> &pulserEvents =
            event.GetSummary("pulser")->GetList();

    for (vector<ChanEvent *>::const_iterator itPulser = pulserEvents.begin();
         itPulser != pulserEvents.end(); itPulser++) {
        int location = (*itPulser)->GetChanID().GetLocation();
        string subType = (*itPulser)->GetChanID().GetSubtype();

        TimingDefs::TimingIdentifier key(location, subType);
        pulserMap.insert(make_pair(key, HighResTimingData(*(*itPulser))));
    }

    if (pulserMap.empty() || pulserMap.size() % 2 != 0) {
        //If the map is empty or size isn't even we return and increment
        // error code
        codes->Fill(WRONG_NUM);
        EndProcess();
        return false;
    }

    HighResTimingData start =
            (*pulserMap.find(make_pair(0, "start"))).second;
    HighResTimingData stop =
            (*pulserMap.find(make_pair(0, "stop"))).second;

    static int trcCounter = 0;
    for(vector<unsigned int>::const_iterator it = start.GetTrace().begin();
            it != start.GetTrace().end(); it++)
        traces->Fill((int)(it-start.GetTrace().begin()), trcCounter, *it);
    trcCounter++;

    //We only plot and analyze the data if the data is validated
    if (start.GetIsValid() && stop.GetIsValid()) {
            start.FillRootStructure(rstart);
            stop.FillRootStructure(rstop);
            tree->Fill();
            start.ZeroRootStructure(rstart);
            stop.ZeroRootStructure(rstop);
    }
    EndProcess();
    return true;
}