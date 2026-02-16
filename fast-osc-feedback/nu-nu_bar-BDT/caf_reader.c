#include <thread>
#include <memory>
#include <math.h>

#include "duneanaobj/StandardRecord/StandardRecord.h"

void caf_reader()
{   
    gSystem->Load("./duneanasel/build/libduneanaobj_StandardRecord.so");

    const float pion_proton_cut = 10.0;

    // set up reading of input CAF file
    std::string caf_file_path="/storage/1/st15719/caf_new_sum.2.6M_weighted.root";
    std::unique_ptr<TFile> caf_file( TFile::Open(caf_file_path.c_str(), "read") );
    std::unique_ptr<TTree> caf_tree( (TTree*)caf_file->Get("cafTree") );
    TTreeReader reader(caf_tree.get());
    TTreeReaderValue<caf::StandardRecord> sr_reader(reader, "rec");
    std::cout << "caf n entries: " << caf_tree->GetEntries() << std::endl;

    // set up output file and tree
    std::unique_ptr<TFile> out_file( TFile::Open("reco_pfp_info.root", "recreate") );
    auto out_tree = std::make_unique<TTree>("reco_pfp_tree", "reco_pfp_tree");
    bool reco_ok;
    int n_reco_pfps;
    int n_reco_tracks;
    int n_reco_showers;
    float single_hits_energy;
    int n_reco_muons_pions;
    int n_reco_protons;

    out_tree->Branch("reco_ok", &reco_ok);
    out_tree->Branch("n_reco_pfps", &n_reco_pfps);
    out_tree->Branch("n_reco_tracks", &n_reco_tracks);
    out_tree->Branch("n_reco_showers", &n_reco_showers);
    out_tree->Branch("single_hits_energy", &single_hits_energy);
    out_tree->Branch("n_reco_muons_pions", &n_reco_muons_pions);
    out_tree->Branch("n_reco_protons", &n_reco_protons);

    auto start = std::chrono::high_resolution_clock::now();
    while(reader.Next()) {
        reco_ok = false;
        n_reco_pfps=-1;
        n_reco_tracks=-1;
        n_reco_showers=-1;
        single_hits_energy=-1;
        n_reco_muons_pions=-1;
        n_reco_protons=-1;

        auto n_pandora_nu_events = sr_reader->common.ixn.pandora.size();
        //std::cout << "n pandora nu: " << n_pandora_nu_events << std::endl;

        if (n_pandora_nu_events == 1)
        {
            auto pandora_nu_event = sr_reader->common.ixn.pandora.at(0);

            reco_ok=true;
            n_reco_pfps=pandora_nu_event.part.pandora.size();
            n_reco_tracks=0;
            n_reco_showers=0;
            n_reco_muons_pions=0;
            n_reco_protons=0;

            for (auto reco_pfp : pandora_nu_event.part.pandora)
            {
                //std::cout << "reco pdg " << reco_pfp.pdg << std::endl;
                if (reco_pfp.primary)
                {
                   if (abs(reco_pfp.pdg) == 11)
                    {
                        n_reco_showers++;
                    }
                    else if (abs(reco_pfp.pdg) == 13)
                    {
                        n_reco_tracks++;
                        //std::cout << "track pfp pida score: " << reco_pfp.score << std::endl;
                        if (reco_pfp.score < pion_proton_cut)
                        {
                            n_reco_muons_pions++;
                        }
                        else
                        {
                            n_reco_protons++;
                        }
                    }
                }
                else
                {
                    if (reco_pfp.pdg == 0 && reco_pfp.origRecoObjType == caf::RecoObjType::kHitCollection)
                    {
                        single_hits_energy = reco_pfp.E;
                    }
                }
            }

            if (single_hits_energy < 0)
            {
                std::cout << "[WARNING] single hits energy not set. This should not happen!" << std::endl;
            }
        }
        //else
        //{
        //    std::cout << "[WARNING] unexpected pandora reco size: " << n_pandora_nu_events << std::endl;
        //}
        out_tree->Fill();
    }
    out_tree->Write();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "processing took: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;
}