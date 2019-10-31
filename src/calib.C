//root
#include <TH1F.h>
#include <TF1.h>
#include <TMath.h>
#include <TString.h>
#include <TSpectrum.h>
#include <TPolyMarker.h>
#include <TGraphErrors.h>

//C, C++
#include <math.h>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <sstream>
//specific
#include "calib.h"

using namespace std;
string extractValues(string str)
{
  unsigned first = str.find("{");
  unsigned last = str.find("}");
  string strNew = str.substr(first + 1, last - first - 1);
  return strNew;
}

double stringToDouble(string text)
{
  return stod(text);
}

string vectorToString(vector<float> vec){
 std::ostringstream vts; 
  
  if (!vec.empty()) 
  { 
    // Convert all but the last element to avoid a trailing "," 
    std::copy(vec.begin(), vec.end()-1, 
        std::ostream_iterator<int>(vts, ", ")); 
  
    // Now add the last element with no delimiter 
    vts << vec.back(); 
  } 
  return vts.str();

}




/******** FUNCTIONS ********/
vector<float> readCalib(string calib_path, string _runName, double initValue)
{
  FILE *file = fopen(calib_path.c_str(), "r");
  vector<float> calib_amp;

  char line[256];

  while (fgets(line, sizeof(line), file))
  {
    if ((strstr(line, _runName.c_str()) != NULL))
    {
      calib_amp.clear();
      string s = extractValues(line);
      string delimiter = ",";

      size_t pos = 0;
      std::string token;
      string calibValue;
      while ((pos = s.find(delimiter)) != std::string::npos)
      {
        token = s.substr(0, pos);

        calibValue = string(token);
        calib_amp.push_back(stringToDouble(calibValue));

        s.erase(0, pos + delimiter.length());
      }
      calibValue = string(s);

      calib_amp.push_back(stringToDouble(calibValue));
      break;
    }
  }
  fclose(file);

  for (std::size_t i = calib_amp.size(); i<32; i++)
  {
    calib_amp.push_back(initValue);
  }
  return calib_amp;
}
