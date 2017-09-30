#include "tools.C"

TCanvas cv("cv", "", 1000, 707);

void spread_comparison(TString component = "Higgsstrahlung", double xsBR = 500)
{
  cv.Clear();
  cv.Divide(2, 2);

  // Read in files

  TString cuts = tools::cutstring(2);

  TH1D smearscale = tools::makeHisto("smearscale", tools::basepath+"/CMS_smearscale/"+component, xsBR, 0, cuts);
  TH1D pure = tools::makeHisto("pure", tools::basepath+"/CMS_pure/"+component, xsBR, 0, cuts);
  TH1D gensmear = tools::makeHisto("gensmear", tools::basepath+"/CMS_gensmear/"+component, xsBR, 0, cuts);

  // set colors
  pure.SetLineColor(kBlue);
  gensmear.SetLineColor(kGreen);
  smearscale.SetLineColor(kRed);
  
  // overlay
  cv.cd(1);

  pure.DrawClone();
  gensmear.DrawClone("same");
  smearscale.DrawClone("same");

}

  
  

  
  
