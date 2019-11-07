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

//specific
#include "geometry.h"
#include "analysis.h"
#include "calib.h"
#include "read.h"

float SP = 0.3125;                                                                                                             // ns per bin
float pe = 47.46;                                                                                                              //mV*ns
vector<float> calib_amp = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};    // dummy
vector<float> calib_charge = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; // dummy
vector<float> BL_const = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};     // dummy

double coef = 2.5 / (4096 * 10);

// Run Parameter
Int_t runPosition = -999;
Float_t runEnergy = -999;
Int_t runAngle = -999;
Int_t runNumber = -999;
Int_t runChannelNumberWC = 32;
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
Float_t tSUMp = -999;
Float_t tSUMm = -999;
Int_t nCh = -1;
int womCount = 4;

int nActiveCh = -1;
int numberOfBinaryFiles = 0;
#define btoa(x) ((x) ? "true" : "false")
int defaultErrorLevel = kError;
bool print = true;
int headerSize = 328;
bool newerVersion = false;
// SWITCH dynamic <-> constant baseline
bool switch_BL = false; // true = dyn, false = const
//Integration Window
bool isDC = true;
//IF the calibration values are correct, otherwise use dummies
bool isCalibrated = true;
float integralStart = 100; //Testbeam: 100, 125 charge, 100-150
float integralEnd = 150;
int triggerChannel = 31; //starting from 1 -> Calib: 9, Testbeam: 15
int plotGrid = 5;
int printExtraEvents = 0;
int printedExtraEvents = 0;

//Event Skipping
bool skipThisEvent = false;
int skippedCount = 0;
//Skip events with bad baseline
bool allowBaselineEventSkipping = false;
int skipInChannel = 0;
//Skip events that exceed the WC maximum
bool allowExceedingEventSkipping = true;
int exceedingThreshold = 650;

void read(TString _inFileList, TString _inDataFolder, TString _outFile, string runName, string _headerSize, string isDC_, string dynamicBL_, string useConstCalibValues_, string runParameter)
{
  gErrorIgnoreLevel = defaultErrorLevel;
  std::vector<std::string> runParams;
  std::string token;
  std::istringstream tokenStream(runParameter);
  char split_char = ',';
  while (std::getline(tokenStream, token, split_char))
  {
    runParams.push_back(token);
  }

  // Run Parameter
  try
  {
    runNumber = stoi(runParams[0]);
    runPosition = stoi(runParams[1]);
    runAngle = stoi(runParams[2]);
    runEnergy = stoi(runParams[3]);
    runChannelNumberWC = stoi(runParams[4]);
  }
  catch (const std::exception &e)
  {
    // std::cerr << e.what() << '\n';
  }
  plotGrid = ceil(sqrt(runChannelNumberWC));

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
    //printf("Current working dir: %s\n", cwd);
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

  /*Create root-file and root-tree for data*/
  TFile *rootFile = new TFile(_outFile, "RECREATE");
  if (rootFile->IsZombie())
  {
    if (numberOfBinaryFiles > 1)
    {
      cout << "PROBLEM with the initialization of the output ROOT ntuple "
           << _outFile << ": check that the path is correct!!!"
           << endl;
    }
    exit(-1);
  }
  TTree *tree = new TTree("T", "USBWC Data Tree");
  TTree::SetBranchStyle(0);
  gStyle->SetLineScalePS(1); // export high resolution plots

  Int_t ChannelNr[runChannelNumberWC];
  int WOMID[runChannelNumberWC]; //1=A, 2=B, 3=C, 4=D

  float chPE[runChannelNumberWC];     // single channel amplitude at sum signal
  float chPE_int[runChannelNumberWC]; // single channel integral

  std::vector<float> amp(runChannelNumberWC, -999);
  std::vector<float> amp_inRange(runChannelNumberWC, -999);
  std::vector<float> max(runChannelNumberWC, -999);
  std::vector<float> min(runChannelNumberWC, -999);
  Float_t t[runChannelNumberWC];
  Float_t tSiPM[runChannelNumberWC];

  float Integral_0_300[runChannelNumberWC];   //array used to store Integral of signal from 0 to 300ns
  float Integral_inRange[runChannelNumberWC]; // calculate integral in given range
  float Integral[runChannelNumberWC];
  float IntegralSum[runChannelNumberWC];

  float Integral_mVns[runChannelNumberWC];

  float BL_output[4];                        //array used for output getBL-function
  Float_t BL_lower[runChannelNumberWC];      //store baseline for runChannelNumberWC channels for 0-75ns range
  Float_t BL_RMS_lower[runChannelNumberWC];  //store rms of baseline for runChannelNumberWC channels for 0-75ns range
  Float_t BL_Chi2_lower[runChannelNumberWC]; //store chi2/dof of baseline-fit for runChannelNumberWC channels for 0-75ns range
  Float_t BL_pValue_lower[runChannelNumberWC];
  Float_t BL_upper[runChannelNumberWC];      //store baseline for runChannelNumberWC channels for 220-320ns range
  Float_t BL_RMS_upper[runChannelNumberWC];  //store rms of baseline for runChannelNumberWC channels for 220-320ns range
  Float_t BL_Chi2_upper[runChannelNumberWC]; //store chi2/dof of baseline-fit for runChannelNumberWC channels for 220-320ns range
  Float_t BL_pValue_upper[runChannelNumberWC];

  Float_t BL_used[runChannelNumberWC];
  Float_t BL_Chi2_used[runChannelNumberWC];
  Float_t BL_pValue_used[runChannelNumberWC];
  float noiseLevel[runChannelNumberWC];

  int nPeaks = 4; // maximum number of peaks to be stored by peakfinder; has to be set also when creating branch
  Double_t peakX[runChannelNumberWC][nPeaks];
  Double_t peakY[runChannelNumberWC][nPeaks];

  int NumberOfBins;
  Int_t EventIDsamIndex[runChannelNumberWC];
  Int_t FirstCellToPlotsamIndex[runChannelNumberWC];

  TH1F hCh("hCh", "dummy;ns;Amplitude, mV", 1024, -0.5 * SP, 1023.5 * SP);
  std::vector<TH1F *> hChSum;
  std::vector<TH1F> hChtemp;
  std::vector<TH1F *> hChShift;

  Float_t amplitudeChannelSumWOM[womCount];
  Float_t chargeChannelSumWOM[womCount];
  std::vector<TH1F *> histChannelSumWOM;

  Short_t amplValues[runChannelNumberWC][1024];
  if (print)
  {

    for (int i = 0; i < runChannelNumberWC; i++)
    {
      TString name("");
      name.Form("hChSum_%d", i);
      TH1F *h = new TH1F("h", ";ns;Amplitude, mV", 1024, -0.5 * SP, 1023.5 * SP);
      h->SetName(name);
      hChSum.push_back(h);
    }

    for (int i = 0; i < runChannelNumberWC; i++)
    {
      TString name("");
      name.Form("hChShift_%d", i);
      TH1F *h = new TH1F("h", ";ns;Amplitude, mV", 1024, -0.5 * SP, 1023.5 * SP);
      h->SetName(name);
      hChShift.push_back(h);
    }

    for (int i = 0; i < womCount; i++)
    {
      TString name("");
      name.Form("histChannelSumWOM%d", i);
      TH1F *h = new TH1F("h", ";ns;Amplitude, mV", 1024, -0.5 * SP, 1023.5 * SP);
      h->SetName(name);
      histChannelSumWOM.push_back(h);
    }
  }

  for (int i = 0; i < runChannelNumberWC; i++)
  {
    TString name("");
    name.Form("hChtemp_%d", i);
    TH1F h("h", ";ns;Amplitude, mV", 1024, -0.5 * SP, 1023.5 * SP);
    h.SetName(name);
    hChtemp.push_back(h);
  }

  TString plotSaveFolder = _outFile;
  plotSaveFolder.ReplaceAll((runName + ".root"), "");
  plotSaveFolder.ReplaceAll(("out.root"), "");

  TCanvas cWaves("cWaves", "cWaves", 1000, 1000);
  cWaves.Divide(plotGrid, plotGrid);
  TCanvas womCanvas("womCanvas", "womCanvas", 1000, 1000);
  womCanvas.Divide(2, 2);

  //TCanvas cCh0("cCh0", "cCh0", 1500, 900);
  //cCh0.Divide(2, 2);
  //TCanvas cTrig("cTrig", "cTrig", 1500, 900);
  //cTrig.Divide(2, 2);
  // TCanvas cSignal("cSignal", "cSignal", 1500, 900);
  //cSignal.Divide(2, 2);
  TCanvas cChSum("cChSum", "cChSum", 1500, 900);
  cChSum.Divide(plotGrid, plotGrid);
  //TCanvas sum_total("sum_total", "sum_total", 1500, 900);
  //sum_total.Divide(2);
  // clibrated sum
  //TCanvas C_amp_array("C_amp_array", "C_amp_array", 1000, 700);
  //C_amp_array.Divide(plotGrid, plotGrid);

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

  //RUN PARAMETER
  tree->Branch("runPosition", &runPosition, "runPosition/I");
  tree->Branch("runEnergy", &runEnergy, "runEnergy/F");
  tree->Branch("runAngle", &runAngle, "runAngle/I");
  tree->Branch("runNumber", &runNumber, "runNumber/I");
  tree->Branch("runChannelNumberWC", &runChannelNumberWC, "runChannelNumberWC/I");

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
  tree->Branch("chargeChannelSumWOM", chargeChannelSumWOM, "chargeChannelSumWOM[4]/F");
  tree->Branch("amplitudeChannelSumWOM", amplitudeChannelSumWOM, "amplitudeChannelSumWOM[4]/F");

  //  tree->Branch("chPE_int", channelSumWOM_amp, "chPE_int[nCh]/F");

  tree->Branch("EventIDsamIndex", EventIDsamIndex, "EventIDsamIndex[nCh]/I");
  tree->Branch("FirstCellToPlotsamIndex", FirstCellToPlotsamIndex, "FirstCellToPlotsamIndex[nCh]/I");

  struct rusage r_usage;

  /*Start reading the raw data from .bin files.*/
  int nitem = 1;
  ifstream inList;
  TString fileName;
  inList.open(_inFileList);
  assert(inList.is_open());

  std::ifstream countStream(_inFileList);
  numberOfBinaryFiles = count(std::istreambuf_iterator<char>(countStream),
                              std::istreambuf_iterator<char>(), '\n');
  //cout << "Number of Binary Files: " << numberOfBinaryFiles << endl;

  if (numberOfBinaryFiles > 1)
  {
    cout << ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << endl;
    cout << ":::::::::::::::::::RUN PARAMETER:::::::::::::::::::::::::::::::::::::::" << endl;
    cout << "RunNr: " << runNumber << endl;
    cout << "Position: " << runPosition << endl;
    cout << "Angle: " << runAngle << endl;
    cout << "Energy: " << runEnergy << endl;
    cout << "Channel/WC: " << runChannelNumberWC << endl;
    cout << ":::::::::::::::::::FILE PARAMETER::::::::::::::::::::::::::::::::::::::" << endl;
    cout << "RunName: " << runName << endl;
    cout << "headerSize Input: " << _headerSize << endl;
    cout << "InDataFolder: " << _inDataFolder.Data() << endl;
    cout << "OutFile: " << _outFile << endl;
    cout << ":::::::::::::::::::CALIBRATION:::::::::::::::::::::::::::::::::::::::::" << endl;
    cout << "Baselines(Constant): " << vectorToString(BL_const) << endl;
    cout << "Amplitude Calibration: " << vectorToString(calib_amp) << endl;
    cout << "Charge Calibration: " << vectorToString(calib_charge) << endl;
    cout << "Is DarkCount: " << btoa(isDC) << " Dynamic Baseline: " << btoa(switch_BL) << " Is Calibrated: " << btoa(isCalibrated) << endl;
    cout << ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << endl;
  }

  int fileCounter = 0;
  int currentPrint = -1;
  while (inList >> fileName)
  {

    fileName = _inDataFolder + fileName;
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
      if (numberOfBinaryFiles > 1)
        cout << "VERSION: " << softwareVersion << " HEADERSIZE: " << headerSize << "  Newer Version: " << newerVersion << endl;
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
      char dummy;
      nitem = fread(&dummy, 1, 1, pFILE);
    }

    int whileCounter = 0;
    /*Loop over events. Events are processed and analysed one by one in order.*/
    while (nitem > 0)
    { //event loop
      skipThisEvent = false;
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

      float MeasuredBaseline[runChannelNumberWC];
      float AmplitudeValue[runChannelNumberWC];
      float ComputedCharge[runChannelNumberWC];
      float RiseTimeInstant[runChannelNumberWC];
      float FallTimeInstant[runChannelNumberWC];
      float RawTriggerRate[runChannelNumberWC];
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
          WOMID[i] = 3;
        }
        else if (i <= 16)
        {
          WOMID[i] = 2;
        }
        else if (i <= 24)
        {
          WOMID[i] = 0;
        }
        else if (i < 31)
        {
          WOMID[i] = 1;
        }
        else
        {
          WOMID[i] = -1;
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
          hCh.SetBinError(j, 0.5);
        }

        /*Analysis if the event/signal starts.*/
        max[i] = hCh.GetMaximum();
        min[i] = hCh.GetMinimum();

        /*Saving the histogram of that event into a temporary histogram hChtemp. These histograms are available outside of the channel-loop. If analysis using the signals/events of multiple channels needs to be done, this can be accomplished by using hChtemp after the channel-loop.*/
        hChtemp.at(i) = hCh;

        //Exceeding Events Skipping
        if (allowExceedingEventSkipping && !skipThisEvent)
        {
          if (max[i] > exceedingThreshold)
          {
            // cout << "EXCEEDING: " << max[i]  <<" CHANNEL: " <<i << endl;
            skipThisEvent = true;
          }
          else
          {
            skipThisEvent = false;
          }
        }
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

        if (i == skipInChannel && allowBaselineEventSkipping)
        {
          if (BL_Chi2_used[i] > 1)
          {
            skipThisEvent = true;
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

        gErrorIgnoreLevel = defaultErrorLevel; // return to normal terminal output

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

        float t_amp = t_max_inRange(&hCh, integralStart, integralEnd);
        float integralStartShifted = t_amp - 10;
        float integralEndShifted = t_amp + 15;

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
          Integral_inRange[i] = integral(&hCh, integralStartShifted, integralEndShifted, BL_shift) / calib_charge.at(i);

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
          if ((currentPrint != fileCounter) || (printedExtraEvents < printExtraEvents))
          {
            cWaves.cd(i + 1);
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
        if (WOMID[i] >= 0)
          histChannelSumWOM[WOMID[i]]->Add(&hCh);
      }

      /*
      __ Number of P.E. _____________________________________________________
      Calculate & save the number of p.e. for an entire WOM.
      Note: for the 1st WOM of each WC one channel was not recorded.
      Thus, there are only 7 values from 8. The result for the WOM is therefore
      scaled up by 8/7 to make the numbers comparable.
      */
      for (int i = 0; i < 4; i++)
      {
        womCanvas.cd(i + 1);
        histChannelSumWOM[i]->DrawCopy();

        float t_amp = t_max_inRange(histChannelSumWOM[i], integralStart, integralEnd);
        float integralStartShifted = t_amp - 10;
        float integralEndShifted = t_amp + 15;

        IntegralSum[i] = integral(histChannelSumWOM[i], integralStartShifted, integralEndShifted, 0) / calib_charge.at(i);

        amplitudeChannelSumWOM[i] = amp_atTime(histChannelSumWOM[i], t_amp);

        float minY = histChannelSumWOM[i]->GetMinimum();
        float maxY = histChannelSumWOM[i]->GetMaximum();

        TLine *ln4 = new TLine(integralStartShifted, minY, integralStartShifted, maxY);
        TLine *ln5 = new TLine(integralEndShifted, minY, integralEndShifted, maxY);

        ln4->SetLineColor(2);
        ln5->SetLineColor(2);
        ln4->Draw("same");
        ln5->Draw("same");

        histChannelSumWOM[i]->Scale(calib_amp.at(i));
        chargeChannelSumWOM[i] = IntegralSum[i];
        // cout << "chargeChannelSum:  " << chargeChannelSumWOM[i]  << endl;
      }

      // PE_WOM1 = 8 / 7 * (amp[0] + amp[1] + amp[2] + amp[3] + amp[4] + amp[5] + amp[6]);
      //PE_WOM2 = (amp[7] + amp[8] + amp[9] + amp[10] + amp[11] + amp[12] + amp[13] + amp[14]);

      /*
      __ TIMING _____
      */
      trigT = t[triggerChannel];
      for (int i = 0; i < runChannelNumberWC; i++)
      {
        tSiPM[i] = t[i] - trigT;
      }

      // add-up all events channel-wise, not calibrated

      if (!skipThisEvent && print)
      {
        for (int i = 0; i < runChannelNumberWC; i++)
        {
          hChSum.at(i)->Add(&hChtemp.at(i), 1);
        }
      }

      if (!skipThisEvent && print)
      {
        /*Saving the plotted signals/events to a new page in the .pdf file.*/
        if ((currentPrint != fileCounter) || (printedExtraEvents < printExtraEvents))
        {
          printedExtraEvents++;
          currentPrint = fileCounter;

          if (fileCounter == 0 && fileCounter != (numberOfBinaryFiles - 1))
          {
            cWaves.Print((TString)(plotSaveFolder + "/waves.pdf("), "pdf");
            womCanvas.Print((TString)(plotSaveFolder + "/womSum.pdf("), "pdf");
            //  C_amp_array.Print((TString)(plotSaveFolder + "/amp_array.pdf("), "pdf");
            //cSignal.Print((TString)(plotSaveFolder + "/signal.pdf("), "pdf");
            //cTrig.Print((TString)(plotSaveFolder + "/trig.pdf("), "pdf");
          }
          else
          {
            cWaves.Print((TString)(plotSaveFolder + "/waves.pdf"), "pdf");
            womCanvas.Print((TString)(plotSaveFolder + "/womSum.pdf"), "pdf");

            // C_amp_array.Print((TString)(plotSaveFolder + "/amp_array.pdf"), "pdf");
            //cSignal.Print((TString)(plotSaveFolder + "/signal.pdf"), "pdf");
            //cTrig.Print((TString)(plotSaveFolder + "/trig.pdf"), "pdf");
          }
        }
      }

      /*Writing the data for that event to the tree.*/
      if (!skipThisEvent)
      {
        tree->Fill();
      }
      else
      {
        skippedCount = skippedCount + 1;
      }
    }
    auto nevent = tree->GetEntries();

    cout << "Events in Tree:  " << nevent << " Skipped:  " << skippedCount << endl;
    fclose(pFILE);
    fileCounter++;
  }

  if (print)
  {
    /*Clearing objects and saving files.*/
    inList.close();

    if (numberOfBinaryFiles != 1)
    {
      cWaves.Print((TString)(plotSaveFolder + "/waves.pdf)"), "pdf");
      womCanvas.Print((TString)(plotSaveFolder + "/womSum.pdf)"), "pdf");
    }
    cWaves.Clear();
    womCanvas.Clear();
    //C_amp_array.Print((TString)(plotSaveFolder + "/amp_array.pdf)"), "pdf");
    //cSignal.Print((TString)(plotSaveFolder + "/signal.pdf)"), "pdf");
    // cTrig.Print((TString)(plotSaveFolder + "/trig.pdf)"), "pdf");

    for (int i = 0; i < runChannelNumberWC; i++)
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