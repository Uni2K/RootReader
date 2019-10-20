#include <stdio.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <iostream>

void removeSubstring(char *s, const char *toremove)
{
	while (s = strstr(s, toremove))
		memmove(s, s + strlen(toremove), 1 + strlen(s + strlen(toremove)));
}

int main(int argc, char const *argv[])
{
	//Argument: ./runs/Fast//22_muon6_pos4/0/out.root/T||./runs/Fast//22_muon6_pos4/1/out.root/T||./runs/Fast//22_muon6_pos4/2/out.root/T||
	//std::cout<<argv[0]<<"  ::::::::::: "<<argv[1]<<"::::::::"<<argc <<std::endl;
	TString inFileList;
	TString inDataFolder;
	TString outFile;
	TChain *chain = new TChain("T");
	std::string s = argv[1];
	TString outputFolder = argv[2];
	TString runName = argv[3];
	std::string delimiter = "||";

	std::cout << "MERGER STARTED--------------------------------" << std::endl;

	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos)
	{
		token = s.substr(0, pos);

		TString tok = token;
		if (tok.Length() > 0)
		{
			std::cout << "MERGER ADDED: " << token << std::endl;
			chain->Add(tok);
		}

		s.erase(0, pos + delimiter.length());
	}
	TString tok2 = s;
	std::cout << "MERGER ADDED: " << s << std::endl;
	chain->Add(tok2);

	TString outputFile = outputFolder + "/" + runName + ".root";

	chain->Merge(outputFile);
	std::cout<<"MERGER FINISHED-> Number of Events in the final File: "<< chain->GetEntries()<<std::endl;



	return 0;
}