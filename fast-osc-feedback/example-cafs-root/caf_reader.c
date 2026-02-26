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
    std::unique_ptr<TFile> out_file( TFile::Open("event_daughters_info.root", "recreate") );
    auto out_tree = std::make_unique<TTree>("event_daughters_tree", "event_daughters_tree");
    bool reco_ok;
    int n_reco_pfps;
    int n_reco_tracks;
    int n_reco_showers;
    float single_hits_energy;
    int n_reco_muons_pions;
    int n_reco_protons;

    int n_true_nu; // per event
    int n_primary_daughters;
    int n_primary_electrons;
    int n_primary_muons;
    int n_primary_protons;
    int n_primary_pions;
    int n_primary_photons;
    int n_primary_pi0s;

    int n_primary_tracks;
    int n_primary_showers;

    // reco properties
    out_tree->Branch("reco_ok", &reco_ok);
    out_tree->Branch("n_reco_pfps", &n_reco_pfps);
    out_tree->Branch("n_reco_tracks", &n_reco_tracks);
    out_tree->Branch("n_reco_showers", &n_reco_showers);
    out_tree->Branch("single_hits_energy", &single_hits_energy);
    out_tree->Branch("n_reco_muons_pions", &n_reco_muons_pions);
    out_tree->Branch("n_reco_protons", &n_reco_protons);

    // true properties
    out_tree->Branch("n_true_nu", &n_true_nu);
    out_tree->Branch("n_primary_daughters", &n_primary_daughters);
    out_tree->Branch("n_primary_electrons", &n_primary_electrons);
    out_tree->Branch("n_primary_muons", &n_primary_muons);
    out_tree->Branch("n_primary_protons", &n_primary_protons);
    out_tree->Branch("n_primary_pions", &n_primary_pions);
    out_tree->Branch("n_primary_photons", &n_primary_photons);
    out_tree->Branch("n_primary_pi0s", &n_primary_pi0s);
    out_tree->Branch("n_primary_tracks", &n_primary_tracks);
    out_tree->Branch("n_primary_showers", &n_primary_showers);

    int n_event=0;
    auto start = std::chrono::high_resolution_clock::now();
    while(reader.Next()) {
        reco_ok = false;
        n_reco_pfps=-1;
        n_reco_tracks=-1;
        n_reco_showers=-1;
        single_hits_energy=-1;
        n_reco_muons_pions=-1;
        n_reco_protons=-1;

        n_true_nu=-1;
        n_primary_daughters=-1;
        n_primary_tracks=-1;
        n_primary_showers=-1;

        n_primary_electrons=-1;
        n_primary_muons=-1;
        n_primary_protons=-1;
        n_primary_pions=-1;
        n_primary_photons=-1;
        n_primary_pi0s=-1;

        //true section
        n_true_nu = sr_reader->mc.nu.size();
        //std::cout << "true nu events " << n_true_nu << std::endl;

        // should always be 1, but ...
        if (n_true_nu == 1)
        {
            auto true_nu_event = sr_reader->mc.nu.at(0);

            n_primary_daughters=true_nu_event.prim.size();
            n_primary_electrons=0;
            n_primary_muons=0;
            n_primary_protons=0;
            n_primary_pions=0;
            n_primary_photons=0;
            n_primary_pi0s=0;

            int nprim = true_nu_event.nprim;        ///< Number of primary particles
            int nprefsi = true_nu_event.nprefsi;      ///< How many primary particles there were prior to FSI

            int nproton  = true_nu_event.nproton;  ///< number of (post-FSI) primary protons
            int nneutron = true_nu_event.nneutron; ///< number of (post-FSI) primary neutrons
            int npip     = true_nu_event.npip;     ///< number of (post-FSI) primary pi+
            int npim     = true_nu_event.npim;     ///< number of (post-FSI) primary pi-
            int npi0     = true_nu_event.npi0;     ///< number of (post-FSI) primary pi0

            // loop over primaries vector, and count pdg
            for (auto primary : true_nu_event.prim)
            {
                auto primary_pdg = primary.pdg;
                //std::cout << "primary pdg: " << primary_pdg << std::endl;

                if (abs(primary_pdg) == 11)
                {
                    n_primary_electrons++;
                }
                else if (abs(primary_pdg) == 13)
                {
                    n_primary_muons++;
                }
                else if (abs(primary_pdg) == 211)
                {
                    n_primary_pions++;
                }
                else if (primary_pdg == 2212)
                {
                    n_primary_protons++;
                }
                else if (primary_pdg == 22)
                {
                    n_primary_photons++;
                }
                else if (primary_pdg == 111)
                {
                    n_primary_pi0s++;
                }
            }

            if (nproton != n_primary_protons)
            {
                std::cout << "event " << n_event << " calc/stored n primary protons do not match: " << n_primary_protons << ", " << nproton << std::endl;
            }

            if ((npip+npim) != n_primary_pions)
            {
                std::cout << "calc/stored n primary pions do not match: " << (npip+npim) << ", " << n_primary_pions << std::endl;
            }

            if (npi0 != n_primary_pi0s)
            {
                std::cout << "calc/stored n primary pi0 do not match: " << n_primary_pi0s << ", " << npi0 << std::endl;
            }

            n_primary_tracks=n_primary_protons+n_primary_pions+n_primary_muons;
            n_primary_showers=(2*n_primary_pi0s)+n_primary_electrons;
        }

        // reco section
        auto n_pandora_nu_events = sr_reader->common.ixn.pandora.size();
        //std::cout << "n pandora nu: " << n_pandora_nu_events << std::endl;

        // populate reco tree only if we have a good pandora event
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
        n_event++;
    }
    out_tree->Write();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "processing took: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;
}