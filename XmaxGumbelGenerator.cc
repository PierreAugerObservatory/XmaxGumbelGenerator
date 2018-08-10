// ************************************************************************************
// * License and Disclaimer                                                           *
// *                                                                                  *
// * Copyright 2014 Simone Riggi																			                *
// *																																	                *
// * This file is part of XmaxGumbelGenerator													                *
// * XmaxGumbelGenerator is free software: you can redistribute it and/or modify it   *
// * under the terms of the GNU General Public License as published by                *
// * the Free Software Foundation, either * version 3 of the License,                 *
// * or (at your option) any later version.                                           *
// * XmaxGumbelGenerator is distributed in the hope that it will be useful, but 			*
// * WITHOUT ANY WARRANTY; without even the implied warranty of                       * 
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                             *
// * See the GNU General Public License for more details. You should                  *  
// * have received a copy of the GNU General Public License along with                * 
// * XmaxGumbelGenerator. If not, see http://www.gnu.org/licenses/.                   *
// ************************************************************************************

#include <TColor.h>
#include <TCanvas.h>
#include <TClonesArray.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include <TPaveLabel.h>
#include <TMath.h>
#include <TF1.h>
#include <TF2.h>
#include <TF12.h>
#include <TROOT.h>
#include <TLine.h>
#include <TStyle.h>
#include <TApplication.h>
#include <TMinuit.h>
#include <TMultiDimFit.h>
#include <TCut.h>
#include <TEntryList.h>
#include <TVector3.h>
#include <TPaveStats.h>
#include <TProfile.h>
#include <TApplication.h>
#include <TMatrixD.h>
#include <TRandom.h>
#include <TRandom3.h>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <vector>
#include <getopt.h>

using namespace std;


void Usage(char* exeName){
	cout<<"=========== USAGE ==========="<<endl;
	cout<<"Usage: "<<exeName<<" [options]"<<endl;
	cout<<endl;
	cout<<"Options:"<<endl;
  cout<<"-h, --help \t Show help message and exit"<<endl;
	cout<<"-o, --output \t Output file name (ROOT format) (default=Output.root)"<<endl;
	cout<<"-m, --model \t Hadronic model id (1=QGSJETII, 2=SIBYLL, 3=EPOS, 4=QGSJETII-04, 5=EPOS-LHC) (default=QGSJETII)"<<endl;
	cout<<"-e, --energy \t log10(PrimaryEnergy) (default=19)"<<endl;
	cout<<"-a, --mass \t NuclearMass (e.g. 1=proton, 4=helium, 56=Fe, etc) (default=1)"<<endl;
	cout<<"-n, --nevents \t Number of generated events (default=10000)"<<endl;
	cout<<"-s, --save \t Save to file (default=no)"<<endl;
	cout<<"=============================="<<endl;
}//close Usage()

static const struct option options_tab[] = {
  /* name, has_arg, &flag, val */
  { "help", no_argument, 0, 'h' },
	{ "output", required_argument, 0, 'o' },
	{ "model", required_argument, 0, 'm' },
	{ "energy", required_argument, 0, 'e' },
	{ "mass", required_argument, 0, 'a' },	
	{ "nevents", required_argument, 0, 'n' },	
	{ "save", no_argument, 0, 's' },
  {(char*)0, (int)0, (int*)0, (int)0}
};


//Options
int modelId= 1;
double LgE= 19;
double A= 1;
std::string outputFileName= "Output.root";
bool saveToFile= false;
long int nEvents= 10000;

//Consts & Variables
const double LgERef= 19;
const double Xmax_min= 0;//g/cm^2
const double Xmax_max= 2000;//g/cm^2
double Xmax;
TApplication* app= 0;
TF1* parametrizationFcn= 0;
TFile* outputFile= 0;
TTree* generatedDataTree= 0;
TH1D* generatedDataHisto= 0;

//Enums
/*
enum NuclearMassPrimary { 
	ePhotonA= 0, 
	eProtonA = 1, 
	eHeliumA = 4, 
	eLithiumA = 7, 
	eCarbonA= 12, 
	eNitrogenA= 14, 
	eOxygenA= 16, 
	eNeonA= 20, 
	eSiliconA= 28, 
	eCalciumA= 40, 
	eManganeseA = 55, 
	eIronA= 56
};
*/
enum HadModelType {
	eQGSJETII= 1, 
	eSIBYLL= 2, 
	eEPOS= 3,
	eQGSJETII04= 4,
	eEPOSLHC= 5, 
};



//Functions
int ParseOptions(int argc, char *argv[]);
int GenerateData();
int Save();
int Init();
void SetStyle();
void Draw();
double GenGumbelFcn(double* x, double* par);
double GetParametrizationMean(double lgE,double lnA,int modelId);
double GetParametrizationSigma(double lgE,double lnA,int modelId);
double GetParametrizationLambda(double lgE,double lnA,int modelId);

int main(int argc, char **argv)
{
	//================================
	//== Parse command line options
	//================================
	if(ParseOptions(argc,argv)<0){
		cerr<<"ERROR: Failed to parse command line options!"<<endl;
		return -1;
	}
		
	//================================
	//== Initialize data
	//================================	
	//- Create ROOT app
	if(!saveToFile){
		cout<<"INFO: Creating ROOT app..."<<endl;
		app= new TApplication("App",&argc,argv);
	}

	if(Init()<0){
		cerr<<"ERROR: Failed to initialize data!"<<endl;
		return -1;
	}

	//================================
	//== Generate data
	//================================	
	if(GenerateData()<0){
		cerr<<"ERROR: Failed to generate random Gumbel data!"<<endl;
		return -1;
	}

	//================================
	//== Draw plots
	//================================	
	Draw();

	//================================
	//== Save
	//================================	
	if(saveToFile && Save()<0){
		cerr<<"ERROR: Failed to save data!"<<endl;
		return -1;
	}

	cout<<"INFO: End application run"<<endl;
	if(app) app->Run();

	return 0; 

}//close main


int Init()
{
	//Set draw & graphics style
	SetStyle();
	
	//Open output file
	if(saveToFile){
		cout<<"INFO: Creating output file "<<outputFileName<<" ..."<<endl;
		outputFile= new TFile(outputFileName.c_str(),"RECREATE");
	}

	//Create generated data histo
	generatedDataHisto= new TH1D("generatedDataHisto","generatedDataHisto",100,Xmax_min,Xmax_max);
	generatedDataHisto->Sumw2();

	//Create generated data tree
	generatedDataTree= new TTree("data","data");
	generatedDataTree->Branch("Xmax",&Xmax,"Xmax/D");
	
	return 0;

}//close Init()

int GenerateData()
{
	//Get model parameters given energy/mass/model
	double lnA= log(A);
	double paramMean= GetParametrizationMean(LgE,lnA,modelId);
	double paramSigma= GetParametrizationSigma(LgE,lnA,modelId);
	double paramLambda= GetParametrizationLambda(LgE,lnA,modelId);
	cout<<"INFO: Gumbel pars {"<<paramMean<<", "<<paramSigma<<", "<<paramLambda<<"}"<<endl;	

	//Create model function for the generator
	const int nPars= 4;
	const double fcnNorm= 1.;
	parametrizationFcn= new TF1("parametrizationFcn",GenGumbelFcn,Xmax_min,Xmax_max,nPars);
	parametrizationFcn->SetNpx(1000);
	parametrizationFcn->SetParameters(fcnNorm,paramMean,paramSigma,paramLambda);
	parametrizationFcn->FixParameter(0,fcnNorm);
	
	//Generate random data distributed according to Gumbel model
	for(long int i=0;i<nEvents;i++){
		Xmax= parametrizationFcn->GetRandom();
		generatedDataTree->Fill();
		generatedDataHisto->Fill(Xmax);
	}//end loop entries

	return 0;

}//close GenerateData()

int ParseOptions(int argc, char *argv[])
{
	//## Check args given
	/*
	if(argc<2){
		cerr<<"ERROR: Invalid number of arguments...see macro usage!"<<endl;
		Usage(argv[0]);
		exit(1);
	}
	*/

	//## Get command args
	int c = 0;
  int option_index = 0;
	
	while((c = getopt_long(argc, argv, "ho::a::m::e::n::s",options_tab, &option_index)) != -1) {
    
    switch (c) {
			case 0 : 
			{
				break;
			}
			case 'h':
			{
      	Usage(argv[0]);	
				exit(0);
			}
			case 's':
			{
				saveToFile= true;
      	break;
			}
    	case 'o':	
			{
				outputFileName= std::string(optarg);	
				break;	
			}
			case 'e':	
			{
				LgE= atof(optarg);	
				break;	
			}
			case 'a':	
			{
				A= atof(optarg);	
				break;	
			}
			case 'n':	
			{
				nEvents= atol(optarg);	
				break;	
			}
			case 'm':	
			{
				modelId= atoi(optarg);	
				break;	
			}
    	default:
			{
      	Usage(argv[0]);	
				exit(0);
			}
    }//close switch
	}//close while

	
	return 0;
	
}//close ParseOptions()


void Draw()
{
	//Draw histo & model
	TCanvas* Plot= new TCanvas("Plot","Plot");
	Plot->cd();

	generatedDataHisto->SetXTitle("X_{max} (g/cm^{2})");
	generatedDataHisto->SetYTitle("entries");
	generatedDataHisto->SetMarkerStyle(8);
	generatedDataHisto->SetMarkerSize(1.1);
	generatedDataHisto->SetMarkerColor(kBlack);
	generatedDataHisto->SetLineColor(kBlack);
	generatedDataHisto->Draw("ep0");

	double fcnNorm = generatedDataHisto->GetEntries() * generatedDataHisto->GetXaxis()->GetBinWidth(1);
	parametrizationFcn->SetParameter(0,fcnNorm);
	parametrizationFcn->SetLineColor(kBlack);
	parametrizationFcn->Draw("l same");

}//close Draw()

int Save()
{
	//Saving data
	if(outputFile && outputFile->IsOpen()){
		cout<<"INFO: Saving data to file "<<outputFileName<<"..."<<endl;
		outputFile->cd();
		if(parametrizationFcn) parametrizationFcn->Write();
		if(generatedDataTree) generatedDataTree->Write();
		if(generatedDataHisto) generatedDataHisto->Write();
		outputFile->Close();
	}

	return 0;

}//close Save()

double GenGumbelFcn(double* x, double* par)
{
	double Xmax= x[0];
	double norm= par[0];
	double mean= par[1];
	double sigma= par[2];
	double lambda= par[3];
	double z= (Xmax-mean)/sigma;

	double gumbelNorm= 1./sigma*pow(lambda,lambda)/TMath::Gamma(lambda);
	double gumbelFcn= gumbelNorm* pow( TMath::Exp(-z-TMath::Exp(-z)) ,lambda);
	double fcnValue= norm*gumbelFcn;

	return fcnValue;

}//close GenGumbelFcn()

double GetParametrizationMean(double lgE,double lnA,int modelId)
{
	//OLD PARAMETRIZATION
	//double p0_qgsjetII[]= {783.030, -19.982, -0.176};//old
	//double p0_sibyll[]= {796.370, -23.416, -0.278};//old
	//double p0_epos[]= {812.500, -25.720, -0.101};//old
	//double p1_qgsjetII[]= {46.607, 1.415, 0.070};//old
	//double p1_sibyll[]= {57.588, -0.884, 0.209};//old
	//double p1_epos[]= {61.016, -0.038, 0.122};//old

	//NEW PARAMETRIZATION
	double p0_qgsjetII[]= {756.599,-10.3221,-1.34685};	//new
	double p0_sibyll[]= {768.88,-15.0258,-1.12532};//new
	double p0_epos[]= {779.737,-11.4095,-1.96729};//new
	double p0_qgsjetII04[]= {761.167,-11.6411,-1.43675};
	double p0_eposlhc[]= {774.987,-7.38343,-2.35973};

	double p1_qgsjetII[]= {50.998,-0.238915,0.246563};//new	
	double p1_sibyll[]= {59.9104,-0.979926,0.144922};//new
	double p1_epos[]= {62.3138,-0.29405,0.139223};//new
	double p1_qgsjetII04[]= {57.7168,-2.09743,0.472897};
	double p1_eposlhc[]= {58.3911,-0.815205,0.328926};

	double* p0= 0;
	double* p1= 0;

	if(modelId==eQGSJETII) {
		p0= p0_qgsjetII;
		p1= p1_qgsjetII;
	}
	else if(modelId==eQGSJETII04) {
		p0= p0_qgsjetII04;
		p1= p1_qgsjetII04;
	}
	else if(modelId==eSIBYLL){
		p0= p0_sibyll;
		p1= p1_sibyll;
	}
	else if(modelId==eEPOS){
		p0= p0_epos;
		p1= p1_epos;
	}
	else if(modelId==eEPOSLHC){
		p0= p0_eposlhc;
		p1= p1_eposlhc;
	}
	else{
		cerr<<"ERROR: Invalid modelId...exit!"<<endl;
		exit(1);
	}

	double P0_A= p0[0]+p0[1]*lnA+p0[2]*lnA*lnA;
	double P1_A= p1[0]+p1[1]*lnA+p1[2]*lnA*lnA;
	double mean= P0_A + P1_A*(lgE-LgERef);

	return mean;

}//close GetParametrizationMean()

double GetParametrizationSigma(double lgE,double lnA,int modelId)
{
	//OLD PARAMETRIZATION
	//double p0_qgsjetII[]= {55.819, -11.819, 0.806};//old
	//double p0_sibyll[]= {52.788, -10.564, 0.667};//old
	//double p0_epos[]= {58.967, -16.869, 1.655};//old	
	//double p1_qgsjetII[]= {-3.823, 0.900, -0.050};//old
	//double p1_sibyll[]= {-4.090, -0.033, 0.156};//old
	//double p1_epos[]= {-1.540, -0.401, 0.121};//old

	//NEW PARAMETRIZATION
	double p0_qgsjetII[]= {39.0328,7.45191,-2.17575};//new
	double p0_sibyll[]= {31.717,1.33539,-0.601422};//new
	double p0_epos[]= {28.8526,8.10448,-1.92435};//new
	double p0_qgsjetII04[]= {35.2208,12.3353,-2.88955};
	double p0_eposlhc[]= {29.4034,13.5529,-3.15434};

	double p1_qgsjetII[]= {4.39232,-1.68808,0.170133};//new
	double p1_sibyll[]= {-1.91177,0.00725973,0.0864353};//new
	double p1_epos[]= {-0.0827813,-0.960622,0.214701};//new
	double p1_qgsjetII04[]= {0.307042,-1.14723,0.270959};
	double p1_eposlhc[]= {0.095833,-0.960718,0.149841};

	double* p0= 0;
	double* p1= 0;

	if(modelId==eQGSJETII) {
		p0= p0_qgsjetII;
		p1= p1_qgsjetII;	
	}
	else if(modelId==eQGSJETII04) {
		p0= p0_qgsjetII04;
		p1= p1_qgsjetII04;
	}
	else if(modelId==eSIBYLL){
		p0= p0_sibyll;
		p1= p1_sibyll;
	}
	else if(modelId==eEPOS){
		p0= p0_epos;
		p1= p1_epos;
	}
	else if(modelId==eEPOSLHC){
		p0= p0_eposlhc;
		p1= p1_eposlhc;
	}
	else{
		cerr<<"ERROR: Invalid modelId...exit!"<<endl;
		exit(1);
	}

	double P0_A= p0[0]+p0[1]*lnA+p0[2]*lnA*lnA;
	double P1_A= p1[0]+p1[1]*lnA+p1[2]*lnA*lnA;
	double sigma= P0_A + P1_A*(lgE-LgERef);

	return sigma;

}//close GetParametrizationSigma()


double GetParametrizationLambda(double lgE,double lnA,int modelId)
{
	//OLD PARAMETRIZATION
	//double p0_qgsjetII[]= {0.873, 0.701, -0.067};//old
	//double p0_sibyll[]= {0.754, 0.269, -0.007};//old
	//double p0_epos[]= {0.578, 0.577, -0.013};//old
	//double p1_qgsjetII[]= {0.169, 0.050, -0.011};//old
	//double p1_sibyll[]= {0.015, 0.024, -0.002};//old
	//double p1_epos[]= {-0.008, 0.043, -0.004};//old

	//NEW PARAMETRIZATION
	double p0_qgsjetII[]= {0.856737,0.68618,-0.0399485};//new
	double p0_sibyll[]= {0.682908,0.277991,0.011785};//new
	double p0_epos[]= {0.537829,0.52411,0.0468013};//new
	double p0_qgsjetII04[]= {0.673553,0.69388,-0.00756067};
	double p0_eposlhc[]= {0.563286,0.710902,0.0584517};
	
	double p1_qgsjetII[]= {0.178788,0.0758032,-0.013004};//new	
	double p1_sibyll[]= {0.00795706,0.0514181,-0.00303268};//new 
	double p1_epos[]= {0.00941438,0.0233692,0.00999622};//new
	double p1_qgsjetII04[]= {0.0596604,-0.0193051,0.0171836};
	double p1_eposlhc[]= {0.038608,0.0673679,-0.00417924};

	double* p0= 0;
	double* p1= 0;

	if(modelId==eQGSJETII) {
		p0= p0_qgsjetII;
		p1= p1_qgsjetII;
	}
	else if(modelId==eQGSJETII04) {
		p0= p0_qgsjetII04;
		p1= p1_qgsjetII04;
	}
	else if(modelId==eSIBYLL){
		p0= p0_sibyll;
		p1= p1_sibyll;
	}
	else if(modelId==eEPOS){
		p0= p0_epos;
		p1= p1_epos;
	}
	else if(modelId==eEPOSLHC){
		p0= p0_eposlhc;
		p1= p1_eposlhc;
	}
	else{
		cerr<<"ERROR: Invalid modelId...exit!"<<endl;
		exit(1);
	}

	double P0_A= p0[0]+p0[1]*lnA+p0[2]*lnA*lnA;
	double P1_A= p1[0]+p1[1]*lnA+p1[2]*lnA*lnA;
	double lambda= P0_A + P1_A*(lgE-LgERef);

	return lambda;

}//close GetParametrizationLambda()


void SetStyle()
{
	TStyle* myStyle= new TStyle("myStyle","myStyle");

	//## CANVAS & PAD
	myStyle->SetCanvasDefH(700); 
  myStyle->SetCanvasDefW(700); 
	myStyle->SetFrameBorderMode(0);
	myStyle->SetCanvasBorderMode(0);
  myStyle->SetPadBorderMode(0);
  myStyle->SetPadColor(0);
  myStyle->SetCanvasColor(0);
	myStyle->SetPadTopMargin(0.1);
  myStyle->SetPadBottomMargin(0.12);
  myStyle->SetPadLeftMargin(0.16);
  myStyle->SetPadRightMargin(0.1);

	//## TITLE
	myStyle->SetOptTitle(0);
	myStyle->SetTitleX(0.1f);
	myStyle->SetTitleW(0.8f);                 
  myStyle->SetTitleXOffset(0.8);
  myStyle->SetTitleYOffset(1.1);
  myStyle->SetTitleFillColor(0);
  myStyle->SetTitleBorderSize(0);//border size of Title PavelLabel
	myStyle->SetTitleSize(0.06,"X");
  myStyle->SetTitleSize(0.06,"Y");
	myStyle->SetTitleSize(0.06,"Z");
	
	//## STAT
	myStyle->SetOptStat("eMR");
	//myStyle->SetOptStat(1);
  myStyle->SetStatColor(0);
	myStyle->SetStatY(0.975);                
  myStyle->SetStatX(0.95);                
  myStyle->SetStatW(0.35);//0.2                
  myStyle->SetStatH(0.10);//0.15
  myStyle->SetStatBorderSize(1);

	myStyle->SetTitleFont(52,"X");
  myStyle->SetTitleFont(52,"Y");
  myStyle->SetTitleFont(52,"Z");
  myStyle->SetLabelFont(42,"X");
  myStyle->SetLabelFont(42,"Y");
  myStyle->SetLabelFont(42,"Z");   
	
	//## OTHER
  myStyle->SetOptFit(1);
	myStyle->SetOptLogx(0);
	myStyle->SetOptLogy(0);
  //myStyle->SetPalette(1,0);
  myStyle->SetMarkerStyle(8);
  myStyle->SetMarkerSize(0.6);
  myStyle->SetFuncWidth(1.); 
  myStyle->SetErrorX(0.);

	myStyle->SetNumberContours(999);
	myStyle->SetPalette(55);

	gROOT->SetStyle("myStyle");
	gStyle= myStyle;
	myStyle->cd();

}//close SetStyle()



/*
int NuclearMassMapper(int particleCode)
{
	switch (particleCode) {
		case(0):
			return ePhotonA; 
		break;
		case(100):
			return eProtonA; 
		break;
		case(400):
			return eHeliumA; 
		break;
		case(700):
			return eLithiumA; 
		break;
		case(1200):
			return eCarbonA; 
		break;
		case(1400):
			return eNitrogenA; 
		break;
		case(1600):
			return eOxygenA; 
		break;
		case(2000):
			return eNeonA; 
		break;
		case(2800):
			return eSiliconA; 
		break;
		case(4000):
			return eCalciumA; 
		break;
		case(5500):
			return eManganeseA; 
		break;		
		case(5600):
			return eIronA; 
		break;
		default:
		{
			cerr<<"ERROR: Cannot map this nucleus code to mass...exit!"<<endl;
			exit(1);
		}
	}//end switch

	return 0;
	
}//close NuclearMassMapper()


int ModelMapper(int modelCode)
{
	switch (modelCode) {
		case(5):
			return eSIBYLL; 
		break;
		case(4):
			return eEPOS; 
		break;
		case(6):
			return eQGSJETII; 
		break;
		default:
		{
			cerr<<"ERROR: Cannot map this model code to model...exit!"<<endl;
			exit(1);
		}
	}//end switch

	return 0;
	
}//close ModelMapper()
*/


