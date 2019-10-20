//root
#include <TLine.h>
#include <TString.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TMath.h>
#include <TF1.h>
#include <TStyle.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TEfficiency.h>
#include <TLegend.h>
#include <THStack.h>
#include <THistPainter.h>
#include <TText.h>
#include <TSpectrum.h>   // peakfinder
#include <TPolyMarker.h> // peakfinder
#include <TError.h>      // root verbosity level
#include <TSystem.h>     // root verbosity level

//#include <TStyle.h>
#include <sys/resource.h>
//C, C++
#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <sstream>
//#include <stdlib.h>
//#include <string>
//#include <iomanip>

//specific
#include "geometry.h"
#include "analysis.h"
#include "calib.h"
#include "read.h"

float SP = 0.3125; // ns per bin
float pe = 47.46;  //mV*ns
vector<float> SiPM_shift = {2.679, 2.532, 3.594, 3.855, 3.354, 3.886, 3.865, 4.754}; // dummy
vector<float> calib_amp = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};    // dummy
vector<float> calib_charge = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; // dummy
vector<float> BL_const = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};     // dummy

int ch0PrintRate = 1000000;
int trigPrintRate = 1000000;
int signalPrintRate = 100000;
double coef = 2.5 / (4096 * 10);

extern int runNr;
extern float horizontal;
extern float vertical;
extern float angle;
extern int pdgID;
extern float energy;
extern int isSP;
extern int mp;
extern int safPMT2;
extern int safPMT1;
extern int safSiPM;
extern int trackL;
#define btoa(x) ((x) ? "true" : "false")

int amp_array_printRate = 5000; //Changed down below
int wavesPrintRate = 5000;
bool print = true;
int headerSize = 328;
bool newerVersion = false;
//Skip events with bad baseline
bool allowEventSkipping = false;
bool skipThisEvent = false;
int skippedCount = 0;
int skipInChannel = 0;
// SWITCH dynamic <-> constant baseline
bool switch_BL = false; // true = dyn, false = const
//Integration Window
bool isDC = true;
//IF the calibration values are correct, otherwise use dummies
bool isCalibrated = true;
float integralStart = 150; //Testbeam: 100, 125 charge, 100-150
float integralEnd = 180;
int triggerChannel = 9; //starting from 1 -> Calib: 9, Testbeam: 15
int channelCount = 16;

void read(TString _inFileList, TString _inDataFolder, TString _outFile, string runName, string _headerSize, string isDC_, string dynamicBL_, string useConstCalibValues_)
{

  if (dynamicBL_ == "0")
  {
    switch_BL = false;
  }
  else
  {
    switch_BL = true;
  }
  if (isDC_ == "0")
  {
    isDC = false;
  }
  else
  {
    isDC = true;
  }
  if (useConstCalibValues_ == "0")
  {
    isCalibrated = false;
  }
  else
  {
    isCalibrated = true;
  }

  string workingDir;
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL)
  {
    printf("Current working dir: %s\n", cwd);
    workingDir = cwd;
  }
  else
  {
    perror("getcwd() error");
  }
  string amp_file = "/src/calibration_amp.txt";
  string charge_file = "/src/calibration_charge.txt";
  string baseline_file = "/src/Baselines.txt";
  string calib_path_amp = workingDir + amp_file;
  string calib_path_charge = workingDir + charge_file;
  string calib_path_bl = workingDir + baseline_file;

  if (isCalibrated)
    calib_amp = readCalib(calib_path_amp, runName, 1);
  if (isCalibrated)
    calib_charge = readCalib(calib_path_charge, runName, 1);
  if (!switch_BL)
    BL_const = readCalib(calib_path_bl, runName, 0);

  if (isDC)
  {
    amp_array_printRate = 5000;
    wavesPrintRate = 5000;
  }
  else
  {
    amp_array_printRate = 5000;
    wavesPrintRate = 5000;
  }

  cout << ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << endl;
  cout << "RunName: " << runName << endl;
  cout << "headerSize Input: " << _headerSize << endl;
  cout << "InDataFolder: " << _inDataFolder.Data() << endl;
  cout << "OutFile: " << _outFile << endl;
  if (!switch_BL)
    printf(("USED BL ARRAY: %f,%f,%f,%f,%f,%f,%f,%f FOR: " + runName + "\n").c_str(), BL_const[0], BL_const[1], BL_const[2], BL_const[3], BL_const[4], BL_const[5], BL_const[6], BL_const[7], BL_const[8]);
  printf(("USED AMP CALIB ARRAY: %f,%f,%f,%f,%f,%f,%f,%f FOR: " + runName + "\n").c_str(), calib_amp[0], calib_amp[1], calib_amp[2], calib_amp[3], calib_amp[4], calib_amp[5], calib_amp[6], calib_amp[7], calib_amp[8]);
  printf(("USED CHARGE CALIB ARRAY: %f,%f,%f,%f,%f,%f,%f,%f FOR: " + runName + "\n").c_str(), calib_charge[0], calib_charge[1], calib_charge[2], calib_charge[3], calib_charge[4], calib_charge[5], calib_charge[6], calib_charge[7], calib_charge[8]);
  printf("IS DARKCOUNT: %s | Dynamic Baseline: %s | Is Calibrated: %s ", btoa(isDC), btoa(switch_BL), btoa(isCalibrated));
  cout << ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << endl;

  /*Create root-file and root-tree for data*/
  TFile *rootFile = new TFile(_outFile, "RECREATE");
  if (rootFile->IsZombie())
  {
    cout << "PROBLEM with the initialization of the output ROOT ntuple "
         << _outFile << ": check that the path is correct!!!"
         << endl;
    exit(-1);
  }
  TTree *tree = new TTree("T", "USBWC Data Tree");
  TTree::SetBranchStyle(0);

  gStyle->SetLineScalePS(1); // export high resolution plots

  /*Declare & define the variables that are to be saved in the root-tree or that are used during the analysis.*/
  Int_t EventNumber = -999;
  Int_t LastEventNumber = -999;
  unsigned long long int TDCsamIndex;
  Float_t SamplingPeriod = -999;
  Double_t EpochTime = -999;
  Int_t Year = -999;
  Int_t Month = -999;
  Int_t Day = -999;
  Int_t Hour = -999;
  Int_t Minute = -999;
  Int_t Second = -999;
  Int_t Millisecond = -999;
  Float_t trigT = -999; //t_trig = (t0+t1+t2+t3)/4
  Float_t tPMT1 = -999;
  Float_t tPMT2 = -999;
  Float_t tPMT2i = -999;
  Float_t tSUMp = -999;
  Float_t tSUMm = -999;
  Float_t trigTp = -999; //t_trig' = [(t0+t1)-(t2+t3)]/4
  Float_t t0t1 = -999;   //t0t1 = [(t0-t1)]
  Float_t t2t3 = -999;   //t2t3 = [(t2-t3)]
  Int_t isVeto = -999;   //variable to define veto, 1 if veto, 0 if not, -999 if undefined
  Int_t isTrig = -999;
  Int_t isLastEvt = -999;
  Int_t isGoodSignal_5 = -999;
  Float_t trigGate = -999;
  Int_t nCh = -1;
  int nActiveCh = -1;
  Int_t ChannelNr[channelCount];
  Int_t WOMID[channelCount]; //1=A, 2=B, 3=C, 4=D

  float chPE[channelCount];     // single channel amplitude at sum signal
  float chPE_int[channelCount]; // single channel integral

  std::vector<float> amp(channelCount, -999);
  std::vector<float> amp_inRange(channelCount, -999);
  std::vector<float> max(channelCount, -999);
  std::vector<float> min(channelCount, -999);
  Float_t t[channelCount];
  Float_t tSiPM[channelCount];

  float Integral_0_300[channelCount];   //array used to store Integral of signal from 0 to 300ns
  float Integral_inRange[channelCount]; // calculate integral in given range
  float Integral[channelCount];
  float Integral_mVns[channelCount];

  float BL_output[4];                  //array used for output getBL-function
  Float_t BL_lower[channelCount];      //store baseline for channelCount channels for 0-75ns range
  Float_t BL_RMS_lower[channelCount];  //store rms of baseline for channelCount channels for 0-75ns range
  Float_t BL_Chi2_lower[channelCount]; //store chi2/dof of baseline-fit for channelCount channels for 0-75ns range
  Float_t BL_pValue_lower[channelCount];
  Float_t BL_upper[channelCount];      //store baseline for channelCount channels for 220-320ns range
  Float_t BL_RMS_upper[channelCount];  //store rms of baseline for channelCount channels for 220-320ns range
  Float_t BL_Chi2_upper[channelCount]; //store chi2/dof of baseline-fit for channelCount channels for 220-320ns range
  Float_t BL_pValue_upper[channelCount];

  Float_t BL_used[channelCount];
  Float_t BL_Chi2_used[channelCount];
  Float_t BL_pValue_used[channelCount];
  float noiseLevel[channelCount];

  int nPeaks = 4; // maximum number of peaks to be stored by peakfinder; has to be set also when creating branch
  Double_t peakX[channelCount][nPeaks];
  Double_t peakY[channelCount][nPeaks];

  int NumberOfBins;
  Int_t EventIDsamIndex[channelCount];
  Int_t FirstCellToPlotsamIndex[channelCount];

  TH1F hCh("hCh", "dummy;ns;Amplitude, mV", 1024, -0.5 * SP, 1023.5 * SP);
  std::vector<TH1F *> hChSum;
  std::vector<TH1F> hChtemp;
  std::vector<TH1F *> hChShift;
  std::vector<TH1F> hChShift_temp;
  Short_t amplValues[channelCount][1024];
  if (print)
  {

    for (int i = 0; i < channelCount; i++)
    {
      TString name("");
      name.Form("hChSum_%d", i);
      TH1F *h = new TH1F("h", ";ns;Amplitude, mV", 1024, -0.5 * SP, 1023.5 * SP);
      h->SetName(name);
      hChSum.push_back(h);
    }

    for (int i = 0; i < channelCount; i++)
    {
      TString name("");
      name.Form("hChShift_%d", i);
      TH1F *h = new TH1F("h", ";ns;Amplitude, mV", 1024, -0.5 * SP, 1023.5 * SP);
      h->SetName(name);
      hChShift.push_back(h);
    }

    for (int i = 0; i < channelCount; i++)
    {
      TString name("");
      name.Form("hChShift_temp_%d", i);
      TH1F h("h", ";ns;Amplitude, mV", 1024, -0.5 * SP, 1023.5 * SP);
      h.SetName(name);
      hChShift_temp.push_back(h);
    }
  }
  for (int i = 0; i < channelCount; i++)
  {
    TString name("");
    name.Form("hChtemp_%d", i);
    TH1F h("h", ";ns;Amplitude, mV", 1024, -0.5 * SP, 1023.5 * SP);
    h.SetName(name);
    hChtemp.push_back(h);
  }
  // uncommtent, if .root file name should equal raw data file
  //TString plotSaveFolder  = _inDataFolder;
  //plotSaveFolder.ReplaceAll("data","runs");
  TString plotSaveFolder = _outFile;
  plotSaveFolder.ReplaceAll((runName+".root"), "");

  TCanvas cWaves("cWaves", "cWaves", 1000, 700);
  cWaves.Divide(4, 4);
  TCanvas cCh0("cCh0", "cCh0", 1500, 900);
  cCh0.Divide(2, 2);
  TCanvas cTrig("cTrig", "cTrig", 1500, 900);
  cTrig.Divide(2, 2);
  TCanvas cSignal("cSignal", "cSignal", 1500, 900);
  cSignal.Divide(2, 2);
  TCanvas cChSum("cChSum", "cChSum", 1500, 900);
  cChSum.Divide(4, 4);
  TCanvas sum_total("sum_total", "sum_total", 1500, 900);
  sum_total.Divide(2);
  // clibrated sum
  TCanvas C_amp_array("C_amp_array", "C_amp_array", 1000, 700);
  C_amp_array.Divide(3, 3);

  /*Create branches in the root-tree for the data.*/
  tree->Branch("EventNumber", &EventNumber, "EventNumber/I");
  tree->Branch("SamplingPeriod", &SamplingPeriod, "SamplingPeriod/F");
  tree->Branch("EpochTime", &EpochTime, "EpochTime/D");
  tree->Branch("Year", &Year, "Year/I");
  tree->Branch("Month", &Month, "Month/I");
  tree->Branch("Day", &Day, "Day/I");
  tree->Branch("Hour", &Hour, "Hour/I");
  tree->Branch("Minute", &Minute, "Minute/I");
  tree->Branch("Second", &Second, "Second_/I");
  tree->Branch("Millisecond", &Millisecond, "Millisecond/I");
  tree->Branch("trigT", &trigT, "trigT/F");
  tree->Branch("tSUMp", &tSUMp, "tSUMp/F");
  tree->Branch("tSUMm", &tSUMm, "tSUMm/F");
  tree->Branch("runNr", &runNr, "runNr/I");      //run number in google table
  tree->Branch("horiz", &horizontal, "horiz/F"); //horizontal position of the box units: [cm]
  tree->Branch("vert", &vertical, "vert/F");     //vertical position of the box, units: [cm]
  tree->Branch("angle", &angle, "angle/F");
  tree->Branch("pdgID", &pdgID, "pdgID/I");
  tree->Branch("energy", &energy, "energy/F");
  tree->Branch("isSP", &isSP, "isSP/I");
  tree->Branch("mp", &mp, "mp/I");
  tree->Branch("safSiPM", &safSiPM, "safSiPM/I"); //solid angle factor
  tree->Branch("trackL", &trackL, "trackL/I");    //track length
  tree->Branch("isLastEvt", &isLastEvt, "isLastEvt/I");
  tree->Branch("trigGate", &trigGate, "trigGate/F");
  tree->Branch("trigTp", &trigTp, "trigTp/F");
  tree->Branch("t0t1", &t0t1, "t0t1/F"); //t0t1 = [(t0-t1)]
  tree->Branch("t2t3", &t2t3, "t2t3/F");
  tree->Branch("isVeto", &isVeto, "isVeto/I");
  tree->Branch("isTrig", &isTrig, "isTrig/I");
  tree->Branch("isGoodSignal_5", &isGoodSignal_5, "isGoodSignal_5/I");

  // CHANNEL INFO (but everything that is nCH-dependend below)
  tree->Branch("nCh", &nCh, "nCh/I");
  tree->Branch("WOMID", WOMID, "WOMID[nCh]/I");
  tree->Branch("ch", ChannelNr, "ch[nCh]/I");
  // AMPLITUDE
  tree->Branch("amp", amp.data(), "amp[nCh]/F");                         // calibrated
  tree->Branch("amp_inRange", amp_inRange.data(), "amp_inRange[nCh]/F"); // calibrated
  tree->Branch("max", max.data(), "max[nCh]/F");
  tree->Branch("min", min.data(), "min[nCh]/F");
  // INTEGRAL
  tree->Branch("Integral_0_300", Integral_0_300, "Integral_0_300[nCh]/F");
  tree->Branch("Integral_inRange", Integral_inRange, "Integral_inRange[nCh]/F");
  tree->Branch("Integral", Integral, "Integral[nCh]/F");                // calibrated
  tree->Branch("Integral_mVns", Integral_mVns, "Integral_mVns[nCh]/F"); // calibrated
  // TIMING
  tree->Branch("t", t, "t[nCh]/F");
  tree->Branch("tSiPM", tSiPM, "tSiPM[nCh]/F");
  // BASELINE
  tree->Branch("BL_lower", BL_lower, "BL_lower[nCh]/F");
  tree->Branch("BL_RMS_lower", BL_RMS_lower, "BL_RMS_lower[nCh]/F");
  tree->Branch("BL_Chi2_lower", BL_Chi2_lower, "BL_Chi2_lower[nCh]/F");
  tree->Branch("BL_pValue_lower", BL_pValue_lower, "BL_pValue_lower[nCh]/F");
  tree->Branch("BL_upper", BL_upper, "BL_upper[nCh]/F");
  tree->Branch("BL_RMS_upper", BL_RMS_upper, "BL_RMS_upper[nCh]/F");
  tree->Branch("BL_Chi2_upper", BL_Chi2_upper, "BL_Chi2_upper[nCh]/F");
  tree->Branch("BL_pValue_upper", BL_pValue_upper, "BL_pValue_upper[nCh]/F");
  tree->Branch("BL_used", BL_used, "BL_used[nCh]/F");
  tree->Branch("BL_Chi2_used", BL_Chi2_used, "BL_Chi2_used[nCh]/F");
  tree->Branch("BL_pValue_used", BL_pValue_used, "BL_pValue_used[nCh]/F");
  // PEAKFINDER
  tree->Branch("noiseLevel", noiseLevel, "noiseLevel[nCh]/F");
  tree->Branch("peakX", peakX, "peakX[nCh][4]/D");
  tree->Branch("peakY", peakY, "peakY[nCh][4]/D");
  // CALIBRATED SUM
  tree->Branch("chPE", chPE, "chPE[nCh]/F");
  tree->Branch("chPE_int", chPE_int, "chPE_int[nCh]/F");
  tree->Branch("EventIDsamIndex", EventIDsamIndex, "EventIDsamIndex[nCh]/I");
  tree->Branch("FirstCellToPlotsamIndex", FirstCellToPlotsamIndex, "FirstCellToPlotsamIndex[nCh]/I");
  struct rusage r_usage;

  /*Start reading the raw data from .bin files.*/
  int nitem = 1;
  ifstream inList;
  TString fileName;
  inList.open(_inFileList);
  assert(inList.is_open());

  int amp_array_PrintStatus = -1;
  int wavePrintStatus = -1;
  int ch0PrintStatus = -1;
  int trigPrintStatus = -1;
  int signalPrintStatus = -1;

  while (inList >> fileName)
  {

    fileName = _inDataFolder + fileName;
    cout << endl;
    cout << fileName << endl;
    FILE *pFILE = fopen(fileName.Data(), "rb");
    if (pFILE == NULL)
    {
      fputs("File error", stderr);
      assert(0);
    }
    fseek(pFILE, 0, SEEK_END);
    float totFileSizeByte = ftell(pFILE);
    rewind(pFILE);

    if (_headerSize == "a")
    {
      char versionBuffer[700];
      fread(versionBuffer, 1, 700, pFILE);

      string softwareVersion;
      softwareVersion += versionBuffer[44]; //i = 44   2
      softwareVersion += versionBuffer[45]; //i = 45   .
      softwareVersion += versionBuffer[46]; //i = 46   9
      softwareVersion += versionBuffer[47]; //i = 47   .
      softwareVersion += versionBuffer[48]; //i = 48   1
      softwareVersion += versionBuffer[49]; //i = 49   0

      std::size_t found = string(versionBuffer).find("Correction"); //Determine the position of "INL Correction: 0" inside the header
      for (int o = 1; o <= 500; o++)
      { //test 500 more bytes after this position, if they are NOT PRINTABLE-> this is were the content starts
        if (isprint(versionBuffer[found + o]) == 0)
        {
          headerSize = found + o;
          break;
        }
      }
        if (headerSize > 328)
    {
      newerVersion = true;
    }
      cout << "VERSION: " << softwareVersion << " HEADERSIZE: " << headerSize <<"  Newer Version: "<<newerVersion <<endl;
    }

    if (headerSize > 328)
    {
      newerVersion = true;
    }

    fseek(pFILE, 0, SEEK_SET);
    char header[headerSize];
    nitem = fread(header, 1, headerSize, pFILE);
    //cout << "Header:\n"
    //<< header << endl;

    char *word;
    word = strtok(header, " \n");
    while (word != NULL)
    {
      if (strcmp("ACQUIRED:", word) == 0)
      {
        word = strtok(NULL, " \n");
        nActiveCh = atoi(word);
        break;
      }
      word = strtok(NULL, " \n");
    }

    if (nActiveCh > 9 || newerVersion)
    {
      cout << endl;
      char dummy;
      nitem = fread(&dummy, 1, 1, pFILE);
    }

    int whileCounter = 0;
    /*Loop over events. Events are processed and analysed one by one in order.*/

    while (nitem > 0)
    { //event loop

      std::vector<TObject *> eventTrash;
      whileCounter++;
      nitem = fread(&EventNumber, sizeof(int), 1, pFILE);
      nitem = fread(&EpochTime, sizeof(double), 1, pFILE);
      nitem = fread(&Year, sizeof(unsigned int), 1, pFILE);
      nitem = fread(&Month, sizeof(unsigned int), 1, pFILE);
      nitem = fread(&Day, sizeof(unsigned int), 1, pFILE);
      nitem = fread(&Hour, sizeof(unsigned int), 1, pFILE);
      nitem = fread(&Minute, sizeof(unsigned int), 1, pFILE);
      nitem = fread(&Second, sizeof(unsigned int), 1, pFILE);
      nitem = fread(&Millisecond, sizeof(unsigned int), 1, pFILE);
      if (newerVersion)
        nitem = fread(&TDCsamIndex, 1, 8, pFILE); //New in 2.9.10

      nitem = fread(&nCh, sizeof(unsigned int), 1, pFILE); // since V2.8.14 the number of stored channels is written for each event

      if (EventNumber % 100 == 0)
      {
        printf("Percentage: %lf, EventNr: %d, nCh: %d+   \n", ftell(pFILE) / totFileSizeByte, EventNumber, nCh);
      }

      float MeasuredBaseline[channelCount];
      float AmplitudeValue[channelCount];
      float ComputedCharge[channelCount];
      float RiseTimeInstant[channelCount];
      float FallTimeInstant[channelCount];
      float RawTriggerRate[channelCount];
      float floatR = -1;

      /*Loop over individual channels. For each event the data from every channel is 
      processed and analysed one by one in order*/
      for (int i = 0; i < nCh; i++)
      {
        nitem = fread(&ChannelNr[i], sizeof(int), 1, pFILE);
        nitem = fread(&EventIDsamIndex[i], sizeof(int), 1, pFILE);
        nitem = fread(&FirstCellToPlotsamIndex[i], sizeof(int), 1, pFILE);
        nitem = fread(&floatR, 1, 4, pFILE);
        MeasuredBaseline[i] = floatR;
        nitem = fread(&floatR, 1, 4, pFILE);
        AmplitudeValue[i] = floatR;
        //  cout<<"AMP: "<<floatR<<endl;

        nitem = fread(&floatR, 1, 4, pFILE);
        ComputedCharge[i] = floatR;
        nitem = fread(&floatR, 1, 4, pFILE);
        RiseTimeInstant[i] = floatR;
        nitem = fread(&floatR, 1, 4, pFILE);
        FallTimeInstant[i] = floatR;
        nitem = fread(&floatR, 1, 4, pFILE);
        RawTriggerRate[i] = floatR;
        ChannelNr[i] = i;

        /*
        __ Set WOMID _________________________________________________________
        The labeling of the WOMs in the box was done using the letters A,B,C,D. For convinience these letters are here replaced by the numbers 1-4 which is stored in the root-tree for every channel and every event.
        */

        if (i <= 7)
        {
          WOMID[i] = 1;
        }
        else if (i <= 16)
        {
          WOMID[i] = 2;
        }
        else if (i <= 24)
        {
          WOMID[i] = 3;
        }
        else if (i <= 32)
        {
          WOMID[i] = 4;
        }

        TString title("");
        title.Form("ch %d, ev %d", i, EventNumber);
        hCh.Reset();
        hCh.SetTitle(title);

        /*
        __ Waveform Histogram _______________________________________________
        Writing the signal amplitude values into the root-histogram hCh.
        */

        for (int j = 0; j < 1024; j++)
        {
          nitem = fread(&amplValues[i][j], sizeof(short), 1, pFILE);
          hCh.SetBinContent(j + 1, (amplValues[i][j] * coef * 1000));
        }

        /*The error of each value in each bin is set to 0.5 mV.*/
        for (int j = 1; j <= hCh.GetXaxis()->GetNbins(); j++)
        {
          hCh.SetBinError(j, 3);
        }

        /*Analysis if the event/signal starts.*/
        max[i] = hCh.GetMaximum();
        min[i] = hCh.GetMinimum();

        /*Saving the histogram of that event into a temporary histogram hChtemp. These histograms are available outside of the channel-loop. If analysis using the signals/events of multiple channels needs to be done, this can be accomplished by using hChtemp after the channel-loop.*/
        hChtemp.at(i) = hCh;

        /*
        __ Baseline Fit _______________________________________________________
        Calculate baseline values infront and after the triggered signal
        Triggered signal is expected in the range fromm 100 to 150 ns
        */
        // BL_fit(&hChtemp.at(i), BL_output, 0.0, 75.0);
        BL_fit(&hChtemp.at(i), BL_output, 0.0, 30.0);
        BL_lower[i] = BL_output[0];
        BL_RMS_lower[i] = BL_output[1];
        BL_Chi2_lower[i] = BL_output[2];
        BL_pValue_lower[i] = BL_output[3];
        // BL_fit(&hChtemp.at(i), BL_output, 220.0, 320.0);

        BL_fit(&hChtemp.at(i), BL_output, 290.0, 320.0);
        BL_upper[i] = BL_output[0];
        BL_RMS_upper[i] = BL_output[1];
        BL_Chi2_upper[i] = BL_output[2];
        BL_pValue_upper[i] = BL_output[3];

        // determine "best" baseline
        if (BL_Chi2_upper[i] <= BL_Chi2_lower[i])
        {
          BL_used[i] = BL_upper[i];
          BL_Chi2_used[i] = BL_Chi2_upper[i];
          BL_pValue_used[i] = BL_pValue_upper[i];
        }
        else
        {
          BL_used[i] = BL_lower[i];
          BL_Chi2_used[i] = BL_Chi2_lower[i];
          BL_pValue_used[i] = BL_pValue_lower[i];
        }

        if (i == skipInChannel && allowEventSkipping)
        {
          if (BL_Chi2_used[i] > 1)
          {
            skipThisEvent = true;
            skippedCount = skippedCount + 1;
          }
          else
          {
            skipThisEvent = false;
          }
        }

        // SWITCH dynamic <-> constant baseline
        float BL_shift;
        if (switch_BL)
        {
          BL_shift = BL_used[i];
        }
        else
        {
          BL_shift = BL_const[i];
        }

        /*
        __ Peakfinder _________________________________________________________
        Implemented to search double-muon-event candiates
        Set maximum number of peaks stored in beginning of script -> nPeaks
        peakX/Yarray[nCh][nPeaks] stores peak coordinates as branches in tree
        Switch on/off with pfON
        -> when off:  set peakX/Yarray[nCh][nPeaks] to zero
        */
        gErrorIgnoreLevel = kError; // suppress root terminal output

        bool pfON = false;
        if (i < 15)
        {
          pfON = false;
        }                     // switch on/off peakfinder
        int sigma = 10;       // sigma of searched peaks
        Double_t thrPF = 0.1; // peakfinder threshold
        TPolyMarker pm;       // store polymarker showing peak position, print later
        peakfinder(&hCh, 0, 130, nPeaks, sigma, thrPF, peakX[i], peakY[i], &pm, pfON);

        gErrorIgnoreLevel = kUnset; // return to normal terminal output

        // baseline-correct Y-values and convert to units of p.e.
        if (pfON)
        {
          for (int j = 0; j < nPeaks; ++j)
          {
            peakY[i][j] = amp2pe(peakY[i][j], calib_amp[i], BL_shift);
          }
        }

        // printf("X: %d %f %f %f %f \n",i,peakX[i][0],peakX[i][1],peakX[i][2],peakX[i][3]);
        // printf("Y: %d %f %f %f %f \n",i,peakY[i][0],peakY[i][1],peakY[i][2],peakY[i][3]);

        /*
        __ CFD _____________________________________________________________
        Setting the signal time by using a constant fraction disriminator method.
        The SiPM and the trigger sinals are handled differently using different thresholds.
        */
        if (i == triggerChannel)
        { //trigger
          t[i] = CDF(&hCh, 0.5);
        }
        else
        { //SiPMs
          t[i] = CFD2(&hCh, 0.35);
          if (t[i] < 95)
          {
            t[i] = CFDinvert2(&hCh, 0.35);
          }
        }

        /*
        __ Integral & Amplitude ________________________________________
        There are several definitions of the integral of a signal used here. Those are:
        - Integral_0_300: Integration over the entire time window (~320ns)
        - Integral: Integration over a smaller time window (~50ns) relative to the trigger
        Additionally the number of p.e. is now calculated using the amplitude
        and the calibration factors in the calib_amp-vactor. The function 'PE' calculates the amplitude of the signal, subtracts the better BL value and divides by the calibration factor.
        */

        Integral[i] = Integrate_50ns(&hCh, BL_shift) / calib_charge.at(i); // difined 50 ns window

        //TESTBEAM
        //Integral_inRange[i] = integral(&hCh, 100, 125, BL_used[i]) / calib_int.at(i); // variable window
        // calibrated, BL-shifted amplitude at maximum in window
        //amp[i] = PE(&hCh, calib_amp.at(i), BL_used[i], 100.0, 150.0);
        //maximum amplitude in range before expected signal (100-130 ns)
        //amp_inRange[i] = PE(&hCh, calib_amp.at(i), BL_used[i], 0.0, 50.0);

        // calibrated, BL-shifted charge
        if (isDC)
          Integral_inRange[i] = integral(&hCh, 50, 75, BL_shift) / calib_charge.at(i); // for DC runs
        else
          Integral_inRange[i] = integral(&hCh, integralStart, integralEnd, BL_shift) / calib_charge.at(i);

        // calibrated, BL-shifted amplitude at maximum in window
        if (isDC)
          amp[i] = PE(&hCh, calib_amp.at(i), BL_shift, 50.0, 100.0); // for DC runs
        else
          amp[i] = PE(&hCh, calib_amp.at(i), BL_shift, integralStart, integralEnd);
        // reduced window
        if (isDC)
          amp_inRange[i] = PE(&hCh, calib_amp.at(i), BL_shift, 50.0, 75.0); // for DC runs
        else
          amp_inRange[i] = PE(&hCh, calib_amp.at(i), BL_shift, integralStart, integralEnd);

        /*
        __ Printing Wafevorms ____________________________________________
        The signals for events can be printed to a .pdf file called waves.pdf. The rate at which the events are drawn to waves.pdf is set via the variable wavesPrintRate. Additional requirements can be set in the if-statement to look at specific events only.
        The entire if-statement so far also plots lines at the found signal maximum, the corresponding integration limit, as well as the BL values to each of the histograms.
        */
        if (print)
        {

          if (EventNumber % wavesPrintRate == 0)
          {
            cWaves.cd(1 + 4 * (i % 4) + (i) / 4);
            hCh.DrawCopy();
            hCh.GetXaxis()->SetRange((t[i] - 20) / SP, (t[i] + 30) / SP);
            int max_bin = hCh.GetMaximumBin();
            int lower_bin = max_bin - 20.0 / SP;
            int upper_bin = max_bin + 30.0 / SP;
            // double x = h->GetXaxis()->GetBinCenter(binmax);
            float max_time = hCh.GetXaxis()->GetBinCenter(max_bin);
            float lower_time = hCh.GetXaxis()->GetBinCenter(lower_bin);
            float upper_time = hCh.GetXaxis()->GetBinCenter(upper_bin);
            hCh.GetXaxis()->SetRange(0, 1024);
            TLine *ln4 = new TLine(0, BL_lower[i], 75, BL_lower[i]);
            TLine *ln5 = new TLine(220, BL_upper[i], 320, BL_upper[i]);
            TText *text = new TText(.5, .5, Form("%f %f", BL_lower[i], BL_upper[i]));
            ln4->SetLineColor(2);
            ln5->SetLineColor(2);
            ln4->Draw("same");
            ln5->Draw("same");
            text->Draw("same");
            if (pfON)
            {
              pm.Draw();
            } // print peakfinders polymarker
          }
          hCh.GetXaxis()->SetRange(1, 30 / SP);
          noiseLevel[i] = hCh.GetMaximum() - hCh.GetMinimum();
          hCh.GetXaxis()->SetRange(1, 1024);
          // End of loop over inividual channels
        }
      }

      /*
      __ Number of P.E. _____________________________________________________
      Calculate & save the number of p.e. for an entire WOM.
      Note: for the 1st WOM of each WC one channel was not recorded.
      Thus, there are only 7 values from 8. The result for the WOM is therefore
      scaled up by 8/7 to make the numbers comparable.
      */

      // PE_WOM1 = 8 / 7 * (amp[0] + amp[1] + amp[2] + amp[3] + amp[4] + amp[5] + amp[6]);
      //PE_WOM2 = (amp[7] + amp[8] + amp[9] + amp[10] + amp[11] + amp[12] + amp[13] + amp[14]);

      /*
      __ TIMING _____
      */
      trigT = t[triggerChannel];
      for (int i = 0; i <= 14; i++)
      {
        tSiPM[i] = t[i] - trigT;
      }

      // add-up all events channel-wise, not calibrated

      if (print)
      {
        for (int i = 0; i <= 15; i++)
        {
          hChSum.at(i)->Add(&hChtemp.at(i), 1);
        }
      }


      if (print)
      {
        /*Saving the plotted signals/events to a new page in the .pdf file.*/
        if (EventNumber % wavesPrintRate == 0)
        {
          if (wavePrintStatus < 0)
          {
            cWaves.Print((TString)(plotSaveFolder + "/waves.pdf("), "pdf");
            wavePrintStatus = 0;
          }
          else
            cWaves.Print((TString)(plotSaveFolder + "/waves.pdf"), "pdf");
        }

        if (EventNumber % trigPrintRate == 0)
        {
          if (trigPrintStatus < 0)
          {
            cTrig.Print((TString)(plotSaveFolder + "/trig.pdf("), "pdf");
            trigPrintStatus = 0;
          }
          else
            cTrig.Print((TString)(plotSaveFolder + "/trig.pdf"), "pdf");
        }
        if (EventNumber % signalPrintRate == 0)
        {
          if (signalPrintStatus < 0)
          {
            cSignal.Print((TString)(plotSaveFolder + "/signal.pdf("), "pdf");
            signalPrintStatus = 0;
          }
          else
            cSignal.Print((TString)(plotSaveFolder + "/signal.pdf"), "pdf");
        }
      }
      /*Writing the data for that event to the tree.*/
      if (EventNumber % amp_array_printRate == 0)
      {
        if (amp_array_PrintStatus < 0)
        {
          C_amp_array.Print((TString)(plotSaveFolder + "/amp_array.pdf("), "pdf");
          amp_array_PrintStatus = 0;
        }
        else
          C_amp_array.Print((TString)(plotSaveFolder + "/amp_array.pdf"), "pdf");
      }

      /*Writing the data for that event to the tree.*/
      if (!skipThisEvent)
      {
        tree->Fill();
      }
      // cout<<"BASELINE: "<<skipThisEvent<<"     "<< skippedCount<<endl;

      // cout<<"SIZE OF: "<<sizeof(amp_array)<< endl;
      //tree->Print();
    }
    auto nevent = tree->GetEntries();

    cout << "EVENTS:  " << nevent << endl;
    cout << "SKIPPED EVENTS:  " << skippedCount << endl;
    fclose(pFILE);
  }

  if (print)
  {

    /*Clearing objects and saving files.*/
    inList.close();
    cWaves.Clear();
    cWaves.Print((TString)(plotSaveFolder + "/waves.pdf)"), "pdf");
    cCh0.Print((TString)(plotSaveFolder + "/ch0.pdf)"), "pdf");
    cTrig.Print((TString)(plotSaveFolder + "/trig.pdf)"), "pdf");
    cSignal.Print((TString)(plotSaveFolder + "/signal.pdf)"), "pdf");
    for (int i = 0; i <= 15; i++)
    {
      cChSum.cd(i + 1);
      hChSum.at(i)->Draw();
    }
    cChSum.Print((TString)(plotSaveFolder + "/ChSum.pdf"), "pdf");
  }

  rootFile = tree->GetCurrentFile();
  rootFile->Write();
  rootFile->Close();
}