#include "tools.C"

struct moep {
  TString first;
  TString second;
};

void cut_flow(TString source)
{
  std::vector<moep> cut_map {
    {"all events", "1"},
    {"4 and more jets", "n_jets>=4"},
      {"Z not leptonic\t", tools::notleptonic},
	{"visible mass > 180", "vismass>=180"},
	  {"successfull rescaling", "chi2>=0"},
	    {"no full hadronic ZZ/WW", tools::deltas},
	      {"2 or more bjets", "(hadjet1_b + hadjet2_b + hadjet3_b + hadjet4_b)>= 2"},
		{"higgs made\t", tools::tags_req + " && " + tools::candidates_base}
  };
  TFile data(source);
  TTree *tree;
  data.GetObject("events", tree);
  std::cout << tree->GetEntries();

  TString cuts = "1";

  for (const auto& it : cut_map) {
    std::cout << it.first << " \t";
    cuts = cuts + " && "  + it.second;
    TH1D temphist = tools::makeHisto("temp", tree, tree->GetEntries()/tools::LUMI, 0, cuts, "higgsmass", false);
    std::cout << temphist.Integral() << std::endl;
  }
}
