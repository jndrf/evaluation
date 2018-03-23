/*
 * Reads the results of an analysis and then does the statistical evaluation.
 * Which is in principle: 
 * smear the histogram, fit the signal function, save interesting values, repeat.
 * 
 * General remarks:
 * The fit and background functions are hardcoded in several functions in this file.
 * be aware that it has to be changed in 
 *  - fit_background
 *  - fit_all
 *  - get_sigyield
 *  - get_fwhm
 *  - propagation_of_error
 */
#include "tools.C"


//fitting range
int FITLOW = tools::LOWER;
int FITHIGH = 149;

// functions for the fits. 
TF1 func_background("func_background", "pol3", FITLOW, FITHIGH);
TF1 func_signal("func", "[0]*([1]*TMath::BreitWigner(x, [2], [3]) + [4]*TMath::Gaus(x, [5], [6]))", FITLOW, FITHIGH);
TF1 func_whole("func", "[0]*([1]*TMath::BreitWigner(x, [2], [3]) +[4]*TMath::Gaus(x, [5], [6])) +[7]*( pol3(8))", FITLOW, FITHIGH);

/*
 * create a histogram containing the sum of the backgrounds. 
 *Files are loaded from the path??/heppy.ana..../tree.root
 */
TH1D make_background_histo(TString pathWW, TString pathZZ, TString pathqq)
{
  TH1D ret("background", "", tools::NBINS, tools::LOWER, tools::UPPER);
  auto temp1 = tools::makeHisto("bgWW", pathWW, 16000, 0, tools::cutstring(2));
  ret.Add(&temp1);

  auto temp2 = tools::makeHisto("bgZZ", pathZZ, 1300, 0, tools::cutstring(2));
  ret.Add(&temp2);

  auto temp3 = tools::makeHisto("bgqq", pathqq, 50000, 0, tools::cutstring(2));
  ret.Add(&temp3);

  return ret;
}


/*
 * Fills a histogram with a random value following a poisson distribution with mean
 * @param fitfunc 's value shifted to the left by half a binwidth.
 * @return a new Histogram.
 */
TH1D smear_hist(TF1 fitfunc, int k)
{
  char buffer [32];
  std::snprintf(buffer, 32, "retro_%i", k);
  TH1D retro(buffer, "", tools::NBINS, tools::LOWER, tools::UPPER);
  float binwidth = (tools::UPPER - tools::LOWER)/tools::NBINS;
  TRandom3 random(0);//gives unique seed
  for (int i=1; i<=tools::NBINS; i++) {
    double val = random.Poisson(fitfunc.Eval(retro.GetXaxis()->GetBinCenter(i)));
    retro.SetBinContent(i, val);
    retro.SetBinError(i, TMath::Sqrt(val));
  }
  return retro;
}


/*
 * calculates the error on the signal yield by propagating the errors of the fitted parameters.
 * @param totalfunc function fitted to signal and background
 * @param totalresult fit result ptr of totalfunc
 */
double propagation_of_error(TF1 func, TFitResultPtr totalresult)           
{                                                                               
  auto submat = totalresult->GetCovarianceMatrix().GetSub(0, 6, 0, 6, "");
  submat.Print();
  return func.IntegralError(tools::LOWER, tools::UPPER, nullptr, submat.GetMatrixArray(), .1);
}

void getfitprecision(TString indir="CMS_default") 
{
  //  TH1::SetDefaultSumw2();

  //make histograms
  auto hist_bg = make_background_histo(tools::basepath+indir+"/WW_any/", tools::basepath+indir+"/ZZ_any/", tools::basepath+indir+"/Zgm_qq/");

  auto hist_signal = tools::makeHisto("signal", tools::basepath+indir+"/Higgsstrahlung/", 193, 0, tools::cutstring(2));

  TH1D hist_gesamt("gesamt", "", tools::NBINS, tools::LOWER, tools::UPPER);
  hist_gesamt.Add(&hist_bg);
  hist_gesamt.Add(&hist_signal);
  for (int i=0; i<tools::NBINS; i++) {
    hist_gesamt.SetBinError(i, TMath::Sqrt(hist_gesamt.GetBinContent(i)));
  }

  // copy functions for smear use
  TF1 sf_bg = func_background;
  TF1 sf_total = func_whole;
  
  //initial fit
  auto fit_bg = tools::fit_background(&hist_bg, func_background);
  TFitResultPtr fit_gesamt;
  TF1 func_gesamt;
  // no scaling for initial fit
  func_whole.SetParameters(1, 7000, 1250, 80, -90, 10000, 70000);
  func_whole.FixParameter(0, 1);
  func_whole.FixParameter(7, 1);
  std::tie(fit_gesamt, func_gesamt) = tools::fit_all(&hist_gesamt, fit_bg, func_whole, false);
  //  fit_gesamt->GetCovarianceMatrix().Print();
  double error;
  func_signal.SetParameter(0, 1);
  func_signal.SetParameters(func_gesamt.GetParameters());
  error = propagation_of_error(func_signal, fit_gesamt);

  // now fix everything but the scaling (params 0 and 7)
  for (int i =1; i<sf_total.GetNpar(); i++) {
    if (i!=7) {
      sf_total.FixParameter(i, func_gesamt.GetParameter(i));
      std::cout << i << "\t" << func_gesamt.GetParameter(i) << std::endl;
    }
  }
  sf_total.SetParameter(0, 1);
  sf_total.SetParameter(7, 1);
  // and for the signal
  for (int i {1}; i<func_signal.GetNpar(); i++) {
    func_signal.FixParameter(i, func_gesamt.GetParameter(i));
  }
  
  //Create Histograms to store every interesting variable
  TH1D result_mpv("mpv", "most probable value", 100, 122.5, 127.5);
  TH1D result_fwhm("fwhm", "fwhm", 400,0, 20);
  TH1D result_chi2_dof("chi2_dof", "#chi^{2}",300, 0, 3);
  TH1D result_yield("yield", "signal yield", 80000, 60000, 140000);

  std::cout << "begin smearing" << std::endl;
  std::cout << &fit_bg << std::endl << &sf_total << std::endl << &func_whole << std::endl;
  //smear and fit the histogram often (100k?), store the vars in the histos.
  int n_smears = 0;
  for (int i=0; i<100000; i++) {
    TH1D temphist;
    TFitResultPtr tempfit;
    TF1 tempfunc;
    temphist = smear_hist(sf_total, i);
    std::tie(tempfit, tempfunc) = tools::fit_all(&temphist, fit_bg, sf_total);
    double chi2_dof = tempfunc.GetChisquare()/tempfunc.GetNDF();
    if (chi2_dof < .5 or chi2_dof > 1.5) continue;
    if (tempfit==0) {
      n_smears++;
      func_signal.SetParameters(tempfunc.GetParameters());
      result_mpv.Fill(tempfunc.GetMaximumX());
      result_fwhm.Fill(tools::get_fwhm(&tempfunc));
      result_chi2_dof.Fill(chi2_dof);//tempfunc.GetChisquare()/43);
      result_yield.Fill(func_signal.Integral(tools::LOWER, 149)); // last bin is empty by definition
    }
    if (i%10000==0) {
      std::cout << i << std::endl;
    }
  }


  //Print statistics
  std::cout << "number of smears: " << n_smears << std::endl;
  std::cout << "Most Probable Value\t: " << result_mpv.GetMean() << "\t +- " << result_mpv.GetStdDev() <<  " rel. " << result_mpv.GetStdDev()/result_mpv.GetMean() << std::endl;
  std::cout << "FWHM \t\t\t: " << result_fwhm.GetMean() << "\t +- " << result_fwhm.GetStdDev() << " rel. " << result_fwhm.GetStdDev()/result_fwhm.GetMean() << std::endl;
  std::cout << "Chi^2/dof\t\t: " << result_chi2_dof.GetMean() << "\t +- " << result_chi2_dof.GetStdDev() << " rel. " << result_chi2_dof.GetStdDev()/result_chi2_dof.GetMean() << std::endl;
  std::cout << "Signal Yield\t\t: " << result_yield.GetMean() << "\t +- " << result_yield.GetStdDev() << " rel. " << result_yield.GetStdDev()/result_yield.GetMean() <<  std::endl;
  std::cout << "Error propagation\t \t\t" << error << std::endl;
  std::cout << "error of sigyield error\t" << result_yield.GetStdDevError() << std::endl;
  std::cout << "error of FWHM error\t" << result_fwhm.GetStdDevError() << std::endl;

  //draw the histograms. invoke root with the -b flag is they are not needed.
  hist_gesamt.DrawClone();
  auto c2 = new TCanvas("cv", "", 1000, 707);
  c2->cd();
  result_mpv.DrawCopy("hist");
  auto c3 = new TCanvas("c3", "", 1000, 707);
  c3->cd();
  result_fwhm.DrawCopy();
  auto c4 = new TCanvas("c4", "", 1000, 707);
  c4->cd();
  result_chi2_dof.SetLineWidth(2);
  result_chi2_dof.GetXaxis()->SetTitle("#chi^{2}");
  result_chi2_dof.GetYaxis()->SetTitle("probability");
  result_chi2_dof.DrawCopy("hist");
  auto c5 = new TCanvas("c5", "", 1000, 707);
  c5->cd();
  result_yield.DrawCopy();

  //  fit_gesamt.Print();
  //fit_gesamt.GetCovarianceMatrix().Print();
}
