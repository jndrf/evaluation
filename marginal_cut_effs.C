/*
 * prints the marginal cut efficiencies (i.e. percentage of events that pass this cut of those that pass all other cuts) to a nice table
 */
#include <vector>
#include "tools.C"

struct graphdata {
  TString cuts;
  TString descr;
  int color;
};

struct histcollector {
  TH1D HS, WW, Zgm, ZZ;
  TString descr;
};


void marginal_cut_effs(TString indir="lep3tracker/") 
{

  // Styling
  gStyle->SetOptStat(0);
  gStyle->SetLegendBorderSize(0);

  std::vector<graphdata> cutvec {
{tools::cutstring(2), "all cuts", 1},
      {tools::notleptonic+"&&"+tools::deltas+"&&"+tools::candidates_base+"&& vismass>180 && chi2>=0", "jet number cut", 2},
	{tools::deltas+"&&"+tools::candidates_base+"&&"+tools::kinematics, "jet const cut", 3},
	  {tools::notleptonic+"&&"+tools::deltas+"&&"+tools::candidates_base+"&& n_jets>=4 && chi2>=0", "vismass cut", 3},
	    {tools::notleptonic+"&&"+tools::deltas+"&&"+tools::candidates_base+"&& n_jets>=4 && vismass>180", "false rescaling", 4},
	      {tools::notleptonic+"&&"+tools::candidates_base+"&&"+tools::kinematics, "delta cut", 5},
		{tools::notleptonic+"&&"+tools::kinematics+"&&"+tools::deltas, "dijet mass cut", 6}
  };

  // read data
  TString treepath="heppy.analyzers.examples.zh_fourjet.TreeProducer.TreeProducer_1/tree.root";
  TFile datsig(tools::basepath+indir+"/Higgsstrahlung/"+treepath);
  TFile datZZ(tools::basepath+indir+"/ZZ_any/"+treepath);
  TFile datWW(tools::basepath+indir+"/WW_any/"+treepath);
  TFile datqq(tools::basepath+indir+"/Zgm_qq/"+treepath);
  TTree *treeSig;
  datsig.GetObject("events", treeSig);
  TTree *treeZZ;
  datZZ.GetObject("events", treeZZ);
  TTree *treeWW;
  datWW.GetObject("events", treeWW);
  TTree *treeqq;
  datqq.GetObject("events", treeqq);

  std::vector<histcollector> histvec{};

  for (auto it = cutvec.begin(); it != cutvec.end(); it++) {
    histcollector temp {tools::makeHisto("signal"+it->descr, treeSig, 193, 0, it->cuts),
	tools::makeHisto("WW"+it->descr, treeWW, 16000, 0, it->cuts),
	tools::makeHisto("Zgm"+it->descr, treeqq, 50000, 0, it->cuts),
	tools::makeHisto("ZZ"+it->descr, treeZZ, 1300, 0, it->cuts),
	it->descr
	};
    histvec.push_back(temp);
  }

  // print results
  std::cout << "cut\t\t HS\t WW\t Zgm\t ZZ" << std::endl;
  histcollector nocuts = histvec[0];
  for (auto it = histvec.begin(); it!=histvec.end(); it++) {
    cout << std::setprecision(3) << it->descr << "\t " << nocuts.HS.Integral()/it->HS.Integral() << "\t " << nocuts.WW.Integral()/it->WW.Integral() << "\t " << nocuts.Zgm.Integral()/it->Zgm.Integral() << "\t " << nocuts.ZZ.Integral()/it->ZZ.Integral() << std::endl;
  }
}
