#include "tools.C"

TCanvas cv("cv", "", 1000, 707);

void spread_comparison(TString component = "Higgsstrahlung", double xsBR = 500)
{
  cv.Clear();
  cv.Divide(2, 2);

  TString cuts = tools::cutstring(2);

  // Read in files

  TFile dat_smearscale(tools::basepath+"/CMS_smearscale/"+component+"/heppy.analyzers.examples.zh_fourjet.TreeProducer.TreeProducer_1/tree.root");
  TTree *tree_smearscale;
  dat_smearscale.GetObject("events", tree_smearscale);

  TFile dat_pure(tools::basepath+"/CMS_pure/"+component+"/heppy.analyzers.examples.zh_fourjet.TreeProducer.TreeProducer_1/tree.root");
  TTree *tree_pure;
  dat_pure.GetObject("events", tree_pure);

  TFile dat_gensmear(tools::basepath+"/CMS_gensmear/"+component+"/heppy.analyzers.examples.zh_fourjet.TreeProducer.TreeProducer_1/tree.root");
  TTree *tree_gensmear;
  dat_pure.GetObject("events", tree_gensmear);
  
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
    ratio.Fit("[0]");
    ratio.DrawClone();

    cv.cd(3);

    ratio = smearscale;
    ratio.Divide(&pure);
    ratio.SetTitle("heppy BES over no BES");
    ratio.Fit("pol(0)");
    ratio.DrawClone();

  }

  // vismass
  {
    cv.cd(4);

    TH1D smearscale2 = tools::makeHisto("smearscale", tree_smearscale, xsBR, 0, cuts, "vismass");
    TH1D pure2 = tools::makeHisto("pure", tree_pure, xsBR, 0, cuts, "vismass");
    TH1D gensmear2 = tools::makeHisto("gensmear", tree_gensmear, xsBR, 0, cuts, "vismass");

    // set colors
    pure2.SetLineColor(kBlue);
    gensmear2.SetLineColor(kGreen);
    smearscale2.SetLineColor(kRed);
  
    pure2.DrawClone();
    gensmear2.DrawClone("same");
    smearscale2.DrawClone("same");

    // legend
    TLegend leg(.15, .7, .35, .9);
    leg.AddEntry(&pure2, "no BES", "l");
    leg.AddEntry(&smearscale2, "heppy BES", "l");
    leg.AddEntry(&gensmear2, "PYTHIA BES", "l");

    leg.DrawClone();


  }

}


  
  

  
  
