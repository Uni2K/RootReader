//root
#include <TString.h>
#include <TH1F.h>

//C, C++
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <iostream>

//local
#include "geometry.h"
#include "analysis.h"
#include "read.h"

using namespace std;

/* Declarations of global/external variables which are used accross all 
other files (read.C, geometry.C, analysis.C) */
int runNr = -999;
int pdgID = -999;
int isSP = -999;
int mp = -999;
int safPMT2 = -999;  //solid angle factor of pmt 2
int safPMT1 = -999;  //solid angle factor of pmt 1
int safSiPM = -999;  //solid angle factor of SiPM
int trackL = -999;   //track length
float energy = -999; // [GeV]
float horizontal = -999;
float vertical = -999;
float angle = -999;
std::vector<float> pmt2Pos = {410, 410};

int main(int argc, char *argv[])
{
  TString inFileList;
  TString inDataFolder;
  TString outFile;

  //if(argc == 5){

  /* Used i.e. for calibration measurements which do not include angluar or 
    positional information as in testbeam measurements. Only information 
    on the local directories is handet to main() */
  inFileList = argv[1];
  inDataFolder = argv[2];
  outFile = argv[3];
  string runName = argv[4];
  string headerSize = argv[5];

  string dynamicBL_ = argv[6];
  string isDC_ = argv[7];
  string useConstCalibValues_ = argv[8];


  string runParameterJoined=string(argv[9])+ ',' +string(argv[11])+ ',' +string(argv[12])+ ',' +string(argv[13])+ ',' +string(argv[14]);





  // runNr=atoi(argv[4]);
  //WCVersion = "AB";
  //cout << WCVersion << endl;

  read(inFileList, inDataFolder, outFile, runName, headerSize,dynamicBL_,isDC_,useConstCalibValues_,runParameterJoined);
  //}
  /* else if(argc == 10){

     Used for CERNTestBeam2017 data. The data from the testbeam has more 
    parameters which are handed to main() in order to calculate geometries 
    and save measurement positions, angles ect. Used for runNrs 19-87 (angles 0 & 30), as these runs are not measured using x&y coordinates.
    
    inFileList = argv[1];
    inDataFolder = argv[2];
    outFile = argv[3];
    runNr=atoi(argv[4]);
    mp=atoi(argv[5]);
    pdgID=atoi(argv[6]);
    energy=atoi(argv[7]);
    angle=atoi(argv[8]);
    // WCVersion = checkFilename(outFile);
    WCVersion=argv[9];
    cout << WCVersion << endl;


    cout<<"In data file list : "<<inFileList<<endl
      <<"In data path      : "<<inDataFolder<<endl
      <<"Out root file     : "<<outFile<<endl
      <<"Run number         : "<<runNr<<endl;
    printf("hor,ver,ang: %4.2f %4.2f %4.2f\n",horizontal,vertical,angle);
    std::vector<float> starPos = getStartPos(horizontal,vertical,angle);
    //printf("start pos: %4.2f %4.2f %4.2f %4.2f\n",starPos[0],starPos[1],starPos[2],starPos[3]);
    std::vector<float> saf = solidAngleFactor(starPos,pmt2Pos);
    //printf("length: %4.2f %4.2f \n", saf[1],10*calculateDistance(horizontal,angle*TMath::Pi()/180));
    printf("length: %4.2f \n", saf[1]);
    printf("saf: %4.2f \n", saf[0]);
    safPMT2 = saf[0];
    trackL = saf[1];
	
    readFull(inFileList, inDataFolder, outFile);

  }
  else if(argc == 12){

     Used for CERNTestBeam2017 data. The data from the testbeam has more
    parameters which are handed to main() in order to calculate geometries
    and save measurement positions, angles ect. Used for runNrs 88-107 (angle 90), as these runs are measured using x&y coordinates.

    inFileList = argv[1];
    inDataFolder = argv[2];
    outFile = argv[3];
    runNr=atoi(argv[4]);
    mp=atoi(argv[5]);
    pdgID=atoi(argv[6]);
    energy=atoi(argv[7]);
    angle=atoi(argv[8]);
    // WCVersion = checkFilename(outFile);
    WCVersion=argv[9];
    cout << WCVersion << endl;
    horizontal=atof(argv[10]);
    vertical=atof(argv[11]);


    cout<<"In data file list : "<<inFileList<<endl
      <<"In data path      : "<<inDataFolder<<endl
      <<"Out root file     : "<<outFile<<endl
      <<"Run number         : "<<runNr<<endl;
    printf("hor,ver,ang: %4.2f %4.2f %4.2f\n",horizontal,vertical,angle);
    std::vector<float> starPos = getStartPos(horizontal,vertical,angle);
    //printf("start pos: %4.2f %4.2f %4.2f %4.2f\n",starPos[0],starPos[1],starPos[2],starPos[3]);
    std::vector<float> saf = solidAngleFactor(starPos,pmt2Pos);
    //printf("length: %4.2f %4.2f \n", saf[1],10*calculateDistance(horizontal,angle*TMath::Pi()/180));
    printf("length: %4.2f \n", saf[1]);
    printf("saf: %4.2f \n", saf[0]);
    safPMT2 = saf[0];
    trackL = saf[1];

    readFull(inFileList, inDataFolder, outFile);

  }
  else{
    cout<<" ERROR --->  in input arguments "<<endl
      <<"        [1] - in data file list"<<endl
      <<"        [2] - in data path"<<endl
      <<"        [3] - out root file"<<endl;
  }*/

  return 0;
}