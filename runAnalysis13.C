#if !defined (__CINT__) || defined (__CLING__)
#include <string>
#include "AliAnalysisAlien.h"
#include "AliAnalysisManager.h"
#include "AliAODInputHandler.h"
#include "AliAnalysisTaskGammaPHOS13TeV.h"
#include "AliCaloPhoton.h"
#include "AliTender.h"
#endif
void LoadEnv();
void runAnalysis13(Bool_t local = kTRUE, TString period = "LHC16g", TString runmode = "terminate")
{
    // set if you want to run the analysis locally (kTRUE), or on grid (kFALSE)
    //Bool_t local = kTRUE;
    // if you run on grid, specify test mode (kTRUE) or full grid model (kFALSE)
    Bool_t gridTest = kFALSE;
    // specify period for GRID
    //char per[3]="p"; // g, h, i, j, k, l, o, p

    Int_t Nrun[500];
    Int_t nn = 0;

/*--------------------------*/
   Int_t  recoPass = 1;
   Bool_t isMC = kFALSE; // kTRUE in case of MC
  //Bool_t isMC=kTRUE;
    
    LoadEnv();

    // since we will compile a class, tell root where to look for headers  
#if !defined (__CINT__) || defined (__CLING__)
    gInterpreter->ProcessLine(".include $ROOTSYS/include");
    gInterpreter->ProcessLine(".include $ALICE_ROOT/include");
    gInterpreter->ProcessLine(".include $ALICE_PHYSICS/include");
    gInterpreter->ProcessLine(".include $ALICE_ROOT/PHOS");
#else
    gROOT->ProcessLine(".include $ROOTSYS/include");
    gROOT->ProcessLine(".include $ALICE_ROOT/include");
    gROOT->ProcessLine(".include $ALICE_PHYSICS/include");
    gROOT->ProcessLine(".include $ALICE_ROOT/PHOS");
#endif
     
    // create the analysis manager
    AliAnalysisManager *mgr = new AliAnalysisManager("GammaAnalysis");
    AliAODInputHandler *aodH = new AliAODInputHandler();
    mgr->SetInputEventHandler(aodH);



/*-----------------------------------*/

 TGrid::Connect("alien://");

/*----------------------------------*/

  TString tenderOption = isMC ?"Run2Default" : "";
  AliPHOSTenderTask *tenderPHOS = reinterpret_cast<AliPHOSTenderTask *>(gInterpreter->ExecuteMacro(Form("$ALICE_PHYSICS/PWGGA/PHOSTasks/PHOS_PbPb/AddAODPHOSTender.C(\"%s\", \"%s\", \"%s\", %d, %d)", "PHOSTenderTask","PHOStender", isMC ?"Run2Default" : "", 1, isMC)));
/*----*/
   AliPHOSTenderSupply * PHOSSupply = tenderPHOS->GetPHOSTenderSupply();
   PHOSSupply->ForceUsingBadMap("alien:///alice/cern.ch/user/p/pkurash/BadMap_LHC16-updated.root");

if(isMC)
{
    // Use custom Zero Suppression threshold if needed
    Double_t zs_threshold = 0.020;
    PHOSSupply->ApplyZeroSuppression(zs_threshold); 
}

/*--------------------------*/

 TMacro addresp(gSystem->ExpandPathName("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C"));
 addresp.Exec(Form("%d", isMC));
 addresp.Exec(Form("%d", recoPass));

/*----------------------------------*/


    // compile the class and load the add task macro
    // here we have to differentiate between using the just-in-time compiler
    // from root6, or the interpreter of root5
#if !defined (__CINT__) || defined (__CLING__)
    gInterpreter->LoadMacro("./AliCaloPhoton.cxx++g");
    gInterpreter->LoadMacro("./AliAnalysisTaskGammaPHOS13TeV.cxx++g");
    AliAnalysisTaskGammaPHOS13TeV *task = reinterpret_cast<AliAnalysisTaskGammaPHOS13TeV*>(gInterpreter->ExecuteMacro("AddMyTask13.C"));
#else
    gROOT->LoadMacro("./AliCaloPhoton.cxx++g");
    gROOT->LoadMacro("./AliAnalysisTaskGammaPHOS13TeV.cxx++g");
    gROOT->LoadMacro("AddMyTask13.C");
    AliAnalysisTaskGammaPHOS13TeV *task = AddMyTask13();
#endif

  // TMacro physseladd(gSystem->ExpandPathName("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C"));
  // AliPhysicsSelectionTask *physseltask = reinterpret_cast<AliPhysicsSelectionTask *>(physseladd.Exec());
  // taskpt->SelectCollisionCandidates(AliVEvent::kMB);

 // task->SetRecalib(1,0.9822696);
 // task->SetRecalib(2,0.9861288);
 // task->SetRecalib(3,1.0072);


/*----------------------------------*/


    if(!mgr->InitAnalysis()) return;
    mgr->SetDebugLevel(2);
    mgr->PrintStatus();
    mgr->SetUseProgressBar(1, 25);

    if(local) 
    {
        // if you want to run locally, we need to define some input
        TChain* chain = new TChain("aodTree");
        // add a few files to the chain (change this so that your local files are added)
//        chain->Add("alien:///alice/data/2010/LHC10d/000126285/pass4/AOD172/0001/AliAOD.root");
        chain->Add("alien:///alice/data/2016/LHC16g/000254128/pass1/AOD/031/AliAOD.root");
        //chain->Add("AliAOD.root");
        // start the analysis locally, reading the events from the tchain
        mgr->StartAnalysis("local", chain);
    } else {
        // if we want to run on grid, we create and configure the plugin
        AliAnalysisAlien *alienHandler = new AliAnalysisAlien();
        // also specify the include (header) paths on grid
        alienHandler->AddIncludePath("-I. -I$ROOTSYS/include -I$ALICE_ROOT -I$ALICE_ROOT/include -I$ALICE_PHYSICS/include");
        // make sure your source files get copied to grid
        //alienHandler->SetAdditionalLibs("AliAnalysisTaskMyTask.cxx AliAnalysisTaskMyTask.h");
        alienHandler->SetAdditionalLibs("AliAnalysisTaskGammaPHOS13TeV.h AliAnalysisTaskGammaPHOS13TeV.cxx AliCaloPhoton.h AliCaloPhoton.cxx  libTender.so libTenderSupplies.so libPWGGAPHOSTasks.so");
        alienHandler->SetAnalysisSource("AliAnalysisTaskGammaPHOS13TeV.cxx");
        // select the aliphysics version. all other packages
        // are LOADED AUTOMATICALLY!
        alienHandler->SetAliPhysicsVersion("vAN-20181217_ROOT6-1");
        // set the Alien API version
        alienHandler->SetAPIVersion("V1.1x");
        // select the input data
        alienHandler->SetGridDataDir(Form("/alice/data/2016/%s", period.Data()));
        alienHandler->SetDataPattern("/pass1/AOD/*/AliAOD.root");
        // MC has no prefix, data has prefix 000
        alienHandler->SetRunPrefix("000");

   /*-------------------------*/
        // runnumber
        //alienHandler->AddRunNumber(167813);
        Int_t evN;
        ifstream ff;
        ff.open(Form("datasets/%s-pass1.txt", period.Data()));
 
        //Add runs
        while( !ff.eof() )
        {
         ff>>Nrun[nn];  
         nn = nn + 1;
       //  alienHandler->AddRunNumber(evN);
        }
        ff.close();

       for(Int_t i = 0; i < nn; i++)
       {
         alienHandler->AddRunNumber(Nrun[i]);
       }
   /*-------------------------*/

        // number of files per subjob
        alienHandler->SetSplitMaxInputFileNumber(40);
        alienHandler->SetExecutable("myTask.sh");
        // specify how many seconds your job may take
        alienHandler->SetTTL(18000);
        alienHandler->SetJDLName("myTask.jdl");

        alienHandler->SetOutputToRunNo(kTRUE);
        alienHandler->SetKeepLogs(kTRUE);
        // merging: run with kTRUE to merge on grid
        // after re-running the jobs in SetRunMode("terminate") 
        // (see below) mode, set SetMergeViaJDL(kFALSE) 
        // to collect final results
        alienHandler->SetMaxMergeStages(1);
        alienHandler->SetMergeViaJDL(kTRUE);

        // define the output folders
        alienHandler->SetGridWorkingDir(Form("pp_Analysis/%s", period.Data()));
        alienHandler->SetGridOutputDir("pass1");

        // connect the alien plugin to the manager
        mgr->SetGridHandler(alienHandler);
        if(gridTest) 
        {
            // speficy on how many files you want to run
            alienHandler->SetNtestFiles(1);
            // and launch the analysis
            alienHandler->SetRunMode("test");
            mgr->StartAnalysis("grid");
         } 
         else 
         {
            // else launch the full grid analysis
            alienHandler->SetRunMode(runmode.Data());
            mgr->StartAnalysis("grid");
         }
    }
}
void LoadEnv()
{

  gSystem->Load("libTree.so");
  gSystem->Load("libGeom.so");
  gSystem->Load("libVMC.so");
  gSystem->Load("libPhysics.so");
  
  //load analysis framework
  gSystem->Load("libANALYSIS");
  gSystem->Load("libANALYSISalice");
  gSystem->Load("libPHOSUtils.so"); 
  gSystem->Load("libPWGGAUtils.so");

   //PHOS Tender

  gSystem->Load("libTender.so");
  gSystem->Load("libTenderSupplies.so");
  gSystem->Load("libPWGGAPHOSTasks.so"); 


}
