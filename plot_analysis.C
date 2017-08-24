/*
 * reads the results of an analysis and plots stacked signal and background histograms.
 * it also fits and draws the total function.
 */
#include "tools.C"

// Histograms and other stuff that needs to live when the main function goes out of scope
TCanvas cv("cv", "Four Jet Channel", 1000, 707); 

TLatex field(0.62, .86, "-_-");

// region for the fit
int FITLOW = tools::LOWER;
int FITHIGH = 149;

// functions for the fits. 
TF1 func_background("func_background", "pol3", FITLOW, FITHIGH);
TF1 func_signal("func", "[0]*TMath::BreitWigner(x, [1], [2]) + [3]*TMath::Gaus(x, [4], [5])", FITLOW, FITHIGH);
TF1 func_whole("func", "[0]*TMath::BreitWigner(x, [1], [2]) +[3]*TMath::Gaus(x, [4], [5]) + pol3(6)", FITLOW, FITHIGH);



void plot_analysis(TString indir="tagger80/", std::string name="CMS")
{
  gStyle->SetOptStat(0);
  gStyle->SetLegendBorderSize(0);

  //make histograms
  THStack histotal("histotal", "");//, NBINS, LOWER, UPPER);
  TLegend leg(0.15, 0.7, 0.35, 0.9);

  TH1D signal = tools::makeHisto("signal", tools::basepath+indir+"/Higgsstrahlung/", 193, 0, tools::cutstring(2));
  TH1D bgZZ = tools::makeHisto("bgZZ", tools::basepath+indir+"/ZZ_any/", 1300, 0, tools::cutstring(2));
  TH1D bgWW = tools::makeHisto("bgWW", tools::basepath+indir+"/WW_any/", 16000, 0, tools::cutstring(2));
  TH1D bgqqbar = tools::makeHisto("bgqqbar", tools::basepath+indir+"/Zgm_qq/", 50000, 0, tools::cutstring(2));

  cv.Clear();
  TH1D temphist("temphist", "", tools::NBINS, tools::LOWER, tools::UPPER); //for some reasons, functions can't be fitted to a THStack

  histotal.Add(&bgZZ);
  histotal.Add(&bgqqbar);
  histotal.Add(&bgWW);
  histotal.Add(&signal);

  temphist.Add(&bgZZ);
  temphist.Add(&bgWW);
  temphist.Add(&bgqqbar);

  //fitting
  auto fitres = tools::fit_background(&temphist, func_background);
  temphist.Add(&signal);

  TF1 tempfunc;
  TFitResultPtr tempfit;
  std::tie(tempfit, tempfunc) = tools::fit_all(&temphist, fitres, func_whole);
  //colours
  tools::graphstyle(&signal, 2, 3, 1);
  tools::graphstyle(&bgqqbar, 3, 3, 1);
  tools::graphstyle(&bgZZ, 4, 3, 1);
  tools::graphstyle(&bgWW, 6, 3, 1);

  //legend
  leg.SetFillStyle(0);
  leg.AddEntry(&signal, "Higgsstrahlung", "l");
  leg.AddEntry(&bgZZ, "ZZ", "l");
  leg.AddEntry(&bgqqbar, "q#bar{q}", "l");
  leg.AddEntry(&bgWW, "WW", "l");
  // Descriptionfield
  char buffer[1024];
  std::snprintf(&buffer[0], 1024, "#scale[.67]{%s, 500 fb^{-1}, #sqrt{s} = 240 GeV}", name.c_str());
  field.SetTitle(buffer);
  field.SetNDC();
  field.SetTextFont(leg.GetTextFont());
  //draw
  histotal.Draw("hist");
  field.Draw();
  std::cout << signal.Integral() << std::endl << bgZZ.Integral() << std::endl << bgWW.Integral() << std::endl << bgqqbar.Integral() << std::endl;
  leg.Draw("same");

  histotal.GetXaxis()->SetTitle("Higgs boson mass [GeV]");
  histotal.GetYaxis()->SetTitle("events / GeV");
  histotal.GetYaxis()->SetTitleOffset(1.15);
  tempfunc.DrawCopy("lsame");
  leg.SetFillStyle(0);
  leg.SetBorderSize(0);

  histotal.DrawClone("hist");
  field.DrawClone();
  leg.DrawClone();
  // leg.SetHeader(legtitle.c_str());
  // TLegendEntry *header = (TLegendEntry*)leg.GetListOfPrimitives().First();
  // header.SetTextAlign(22);
  cv.Modified();
  cv.Update();
  std::cout << std::endl << tools::get_fwhm(&tempfunc);
  //  std::cout << leg.GetTextFont() << std::endl << leg.GetTextSize();
  //cv.Print("signalAndBkg_more_statistics_qqbar.svg");
  
}
