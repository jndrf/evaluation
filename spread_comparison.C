#include "tools.C"

TCanvas cv("cv", "", 1000, 707);

void spread_comparison(TString component = "Higgsstrahlung", double xsBR = 500)
{
  cv.Clear();
  cv.Divide(2, 2);

  // Read in files

  TFile dat_smearscale(tools::basepath+"/CMS_smearscale/"+component+"/heppy.analyzers.examples.zh_fourjet.TreeProducer.TreeProducer_1/tree.root");
  TTree *tree_smearscale;
  dat_smearscale.GetObject("events", tree_smearscale);

  TFile dat_pure(tools::basepath+"/CMS_pure/"+component+"/heppy.analyzers.examples.zh_fourjet.TreeProducer.TreeProducer_1/tree.root");
  TTree *tree_pure;
  dat_pure.GetObject("events", tree_pure);

  TFile dat_gensmear(tools::basepath+"/CMS_gensmear/"+component+"/heppy.analyzers.examples.zh_fourjet.TreeProducer.TreeProducer_1/tree.root");
  TTree *tree_gensmear;
  dat_pure.GetObject("events", tree_pure);
  

  TString cuts = tools::cutstring(2);

  // overlay plot and ratio
  {
    cv.cd(1);

    TH1D smearscale = tools::makeHisto("smearscale", tree_smearscale, xsBR, 0, cuts);
    TH1D pure = tools::makeHisto("pure", tree_pure, xsBR, 0, cuts);
    TH1D gensmear = tools::makeHisto("gensmear", tree_gensmear, xsBR, 0, cuts);

    smearscale.SetTitle("Overlay");
    // set colors
    pure.SetLineColor(kBlue);
    gensmear.SetLineColor(kGreen);
    smearscale.SetLineColor(kRed);
  
    pure.DrawClone();
    gensmear.DrawClone("same");
    smearscale.DrawClone("same");

    // legend
    TLegend leg(.15, .7, .35, .9);
    leg.AddEntry(&pure, "no BES", "l");
    leg.AddEntry(&smearscale, "heppy BES", "l");
    leg.AddEntry(&gensmear, "PYTHIA BES", "l");

    leg.DrawClone();

    // ratio

    cv.cd(2);
    
    TH1D ratio = smearscale;
    ratio.Divide(&gensmear);
    ratio.SetTitle("heppy_BES over PYTHIA_BES");
    ratio.DrawClone();
      

  }

  // vismass
  {
    cv.cd(3);

    TH1D smearscale = tools::makeHisto("smearscale", tree_smearscale, xsBR, 0, cuts, "vismass");
    TH1D pure = tools::makeHisto("pure", tree_pure, xsBR, 0, cuts, "vismass");
    TH1D gensmear = tools::makeHisto("gensmear", tree_gensmear, xsBR, 0, cuts, "vismass");

    // set colors
    pure.SetLineColor(kBlue);
    gensmear.SetLineColor(kGreen);
    smearscale.SetLineColor(kRed);
  
    pure.DrawClone();
    gensmear.DrawClone("same");
    smearscale.DrawClone("same");

    // legend
    TLegend leg(.15, .7, .35, .9);
    leg.AddEntry(&pure, "no BES", "l");
    leg.AddEntry(&smearscale, "heppy BES", "l");
    leg.AddEntry(&gensmear, "PYTHIA BES", "l");

    leg.DrawClone();


  }

}


  
  

  
  
