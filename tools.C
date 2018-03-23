//#include "tools.h"
//Globale Variablen
//Alles in Klasse gebuendelt, um namensraeume frei zu halten.
#ifndef TOOLS_C
#define TOOLS_C
namespace tools{

  // Directory holding the analyses
  std::string basepath = "/afs/cern.ch/user/j/jneundor/work/public/FCCSamples/";

  // Global vars for the histograms and the rescaling
  int NBINS = 50;
  float LOWER = 100.;    
  float UPPER = 150.;
  double LUMI = 5000.; //in fb

  /*
   * Returns the string with the appropriate cuts
   * @param recomode: Mode for the higgs reconstructions. Choices are
   *  0: any reconstructed higgs
   *  1: dijet masses have to be in certain limits. included in 2 and 4
   *  2: Higgs reconstructed from two b-tagged jets.
   *  3: Weighted with the product of the b-tag efficiencies
   *  4: B-tagged, but no cuts on dijet masses
   *  5: weighted, but no cuts on dijet masses
   *  Throws an error when other options are given.
   */
  const TString notleptonic = "hadjet1_n_constituents >= 5 && hadjet1_n_charged_hadrons>0 && hadjet2_n_constituents >= 5 && hadjet2_n_charged_hadrons>0 && hadjet3_n_constituents >= 5 && hadjet3_n_charged_hadrons>0 && hadjet4_n_constituents >= 5 && hadjet4_n_charged_hadrons>0";
  const TString deltas = "deltaWW>10 && deltaZZ > 10";
  const TString candidates_base = "mHJet > 100 && mZedJet > 80 && mZedJet < 110";
  const TString tags_req = " (hadjet1_higgsmaker*hadjet1_b + hadjet2_higgsmaker*hadjet2_b + hadjet3_higgsmaker*hadjet3_b + hadjet4_higgsmaker*hadjet4_b)==2";
  const TString kinematics = "n_jets>=4 && vismass>=180 && chi2>=0";
  TString cutstring(int recomode)
  {
    //  TString b_not_neg = "hadjet1_tageff>=0 && hadjet2_tageff >=0 && hadjet3_tageff>=0 && hadjet4_tageff>=0"; //for legacy reasons. not required with newer analyses
  

    TString retstring = "n_iso_leptons<=2 && " + notleptonic+"&&"+kinematics+"   && "+deltas;// + " && hadjet1_e>0" ;//+ b_not_neg;

    switch (recomode) {
    case 0: break;
    case 1: 
      retstring = retstring+" && "+candidates_base;
      break;
    case 2:
      retstring = retstring + " && "+tags_req+" && "+candidates_base;

      break;

    case 4:
      retstring = retstring + " && " + tags_req;
      break;

    default:
      std::cout << "Unsupportet Mode! " << recomode << std::endl;
      throw -1;
    }
    return retstring;
  }
  
  //Set style of Histogramm *p
  void graphstyle(TH1D *p, int color, int width, int style, int fill = 0)
  {
    p->SetLineColor(color);
    p->SetLineWidth(width);
    p->SetLineStyle(style);
    p->SetFillColor(fill);
  }

  /*
   * Draw a histogram, scale it and return the pointer to it
   * Requires the global vars NBINS, LOWER, UPPER, LUMI
   * @param name Name of histo
   * @param tree: a pointer to the TTree with the data.
   * @param xsBR Cross Section (in fb) times branching ratio
   * @param nEvents Number of analysed events. If 0, it is set to tree->GetEntries()
   * @param attrib leaf of the TTree that shall be plotted, default is higgsmass
   * @param isPositive Draw only positive values, default true
   */
  TH1D makeHisto(TString name, TTree *tree, double xsBR, unsigned long nEvents, TString cuts = "1", TString attrib="higgsmass", bool only_positive = true)
  {
    if (only_positive) {
      cuts = "(" + cuts + ") *(" + attrib + ">0)";
    }
    if (nEvents==0) {
      nEvents = tree->GetEntries();
    }
    TH1D histo(name, "", NBINS, LOWER, UPPER);
    tree->Draw(attrib+">>+"+name, cuts);
    histo.Scale(xsBR * LUMI/nEvents);
    histo.SetDirectory(0);
    return histo;
  }
  /*
   * as above, but needs the path to the rootfile as second argument.
   */
  TH1D makeHisto(TString name, TString datapath, double xsBR, unsigned long nEvents, TString cuts = "1", TString attrib="higgsmass", bool only_positive = true)
  {
    TString treepath="heppy.analyzers.examples.zh_fourjet.TreeProducer.TreeProducer_1/tree.root";
    TString totpath=datapath+treepath;
  
    TFile data(totpath);
    TTree *tree;
    data.GetObject("events", tree);
    //  tree->Print();
  
    return makeHisto(name, tree, xsBR, nEvents, cuts, attrib, only_positive);
  }

  double get_fwhm(const TF1 *function) {
    double max = function->GetMaximum();
    double xmax = function->GetMaximumX();
    double lower = function->GetX(max/2, LOWER, xmax);
    double upper = function->GetX(max/2, xmax, UPPER);
    return upper - lower;
  }

  /*
   * Plottet den wert des tag der ersten njets jets in ein Histogramm und gibt einen Pointer auf dieses  zurueck.
   * Histogramm wird auf 1 normiert.
   */
  TH1D plot_jettag(std::string path, TString cuts, TString injet="hadjet", TString tag="dr", int njets=4, int bins=100, float unten=0., float oben=.5, bool normed=true)
  {
    TString treepath="heppy.analyzers.examples.zh_fourjet.TreeProducer.TreeProducer_1/tree.root";
    TFile data(path+treepath);
    TTree *tree;
    data.GetObject("events", tree);
    TH1D histo("histo", "", bins, unten, oben);
    for (int i=1; i<=njets; i++) {
      char buffer[2048];
      std::snprintf(&buffer[0], 2048, injet+"%i_"+tag+">>+tempo", i);
      TH1D tempo("tempo", "", bins, unten, oben);
      tree->Draw(buffer, cuts);
      histo.Add(&tempo);
    }
    histo.SetDirectory(0); //Damit histo nicht geloescht wird, wenn data geschlossen wird.
    if (normed) {
      histo.Scale(1/histo.Integral());
    }
    return histo;
  }


  /*
   * fit 
   * @param func to
   * @param hist the histogram containing the backgrounds
   * @return fit result
   */
  TFitResultPtr fit_background(TH1D* hist, TF1 func)
  {
    return hist->Fit(&func, "RS");
  }

  /*
   * Fits the whole (i.e. signal + fixed background function to 
   * @param totalhist the total histogram.
   * @param bgresult contains the results of the background fit.
   * @return The Pointer to the fit result and the whole fitted function.
   */
  std::pair<TFitResultPtr, TF1> fit_all(TH1D* totalhist, TFitResultPtr bgresult, TF1 func, bool setParams=true)
  {
    //double parameterlist[6] {280, 125, 6, 120, 125, 10};
    // if (setParams) {
    //   func.SetParameters(1, 280, 125, 6, 120, 125, 10, 1, 10, 10, 10);
    // }
    auto constants = bgresult->GetParams();
    for (int i=0; i < 4; i++) {
      func.FixParameter(i+8, constants[i]);
    }
    TFitResultPtr ret = totalhist->Fit(&func, "RSNQ");//, FITLOW, FITHIGH);
    //  ret->Print();
    return {ret, func};
  }
};
#endif
