#if !defined (__CINT__) || defined (__CLING__)
#include "AliAnalysisAlien.h"
#include "AliAnalysisManager.h"
#include "AliAODInputHandler.h"
#include "AliAnalysisTaskGamma_aod.h"
#include "AliCaloPhoton.h"
#include "AliTender.h"
#endif
void LoadEnv(); 
void runAnalysis()
{
    // set if you want to run the analysis locally (kTRUE), or on grid (kFALSE)
    Bool_t local = kTRUE;
    // if you run on grid, specify test mode (kTRUE) or full grid model (kFALSE)
    Bool_t gridTest = kFALSE;
    // specify period for GRID
    char per[3]="e";

    
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

 // TString tenderOption = isMC ?"Run2Default" : "";
  AliPHOSTenderTask *tenderPHOS = reinterpret_cast<AliPHOSTenderTask *>(gInterpreter->ExecuteMacro(Form("$ALICE_PHYSICS/PWGGA/PHOSTasks/PHOS_PbPb/AddAODPHOSTender.C(\"%s\", \"%s\", \"%s\", %d, %d)", "PHOSTenderTask","PHOStender", "", 1, isMC)));
/*----*/
   AliPHOSTenderSupply * PHOSSupply = tenderPHOS->GetPHOSTenderSupply();
   PHOSSupply->ForceUsingBadMap("alien:///alice/cern.ch/user/p/pkurash/BadMap_LHC16-updated.root");

if(isMC)
{
    // Use custom Zero Suppression threshold if needed
    Double_t zs_threshold = 0.020;
    PHOSSupply->ApplyZeroSuppression(zs_threshold); 
}

//  AliPHOSTenderTask *tender = reinterpret_cast<AliPHOSTenderTask *>(gInterpreter->ExecuteMacro(Form("$ALICE_PHYSICS/PWGGA/PHOSTasks/PHOS_PbPb/AddAODPHOSTender.C(\"%s\", \"%s\", \"%s\", %d, %d)", "PHOSTenderTask","PHOStender","", 4, kFALSE)));
/*----*/
  //AliPHOSTenderSupply *supply = tender->GetPHOSTenderSupply();
  //supply->ForceUsingBadMap("alien:///alice/cern.ch/user/p/pkurash/BadMap_LHC10ef_Majority300416.root");
//  mgr->AddTask(tender);

  //gROOT->LoadMacro("CreatePHOSTender.C");
  // gInterpreter->LoadMacro("./CreatePHOSTender.C");

/*--------------------------*/
Int_t recoPass;
Bool_t isMC=kFALSE; // kTRUE in case of MC
//Bool_t isMC=kTRUE;
//===============
//gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C");
//AddTaskPIDResponse(isMC); 
//AddTaskPIDResponse(recoPass=4); 

 TMacro addresp(gSystem->ExpandPathName("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C"));
 addresp.Exec(Form("%d", isMC));
 addresp.Exec("4");

/*----------------------------------*/


    // compile the class and load the add task macro
    // here we have to differentiate between using the just-in-time compiler
    // from root6, or the interpreter of root5
#if !defined (__CINT__) || defined (__CLING__)
    gInterpreter->LoadMacro("./AliCaloPhoton.cxx++g");
    gInterpreter->LoadMacro("./AliAnalysisTaskGamma_aod.cxx++g");
    AliAnalysisTaskGamma_aod *task = reinterpret_cast<AliAnalysisTaskGamma_aod*>(gInterpreter->ExecuteMacro("AddMyTask13.C"));
#else
    gROOT->LoadMacro("./AliCaloPhoton.cxx++g");
    gROOT->LoadMacro("./AliAnalysisTaskGamma_aod.cxx++g");
    gROOT->LoadMacro("AddMyTask.C");
    AliAnalysisTaskGamma_aod *task = AddMyTask13();
#endif

  // TMacro physseladd(gSystem->ExpandPathName("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C"));
  // AliPhysicsSelectionTask *physseltask = reinterpret_cast<AliPhysicsSelectionTask *>(physseladd.Exec());
  // taskpt->SelectCollisionCandidates(AliVEvent::kMB);

  task->SetRecalib(1,0.9822696);
  task->SetRecalib(2,0.9861288);
  task->SetRecalib(3,1.0072);


/*----------------------------------*/


    if(!mgr->InitAnalysis()) return;
    mgr->SetDebugLevel(2);
    mgr->PrintStatus();
    mgr->SetUseProgressBar(1, 25);

    if(local) {
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
        alienHandler->SetAdditionalLibs("AliAnalysisTaskGamma_aod.h AliAnalysisTaskGamma_aod.cxx AliCaloPhoton.h AliCaloPhoton.cxx  libTender.so libTenderSupplies.so libPWGGAPHOSTasks.so");
        alienHandler->SetAnalysisSource("AliAnalysisTaskGamma_aod.cxx");
        // select the aliphysics version. all other packages
        // are LOADED AUTOMATICALLY!
        alienHandler->SetAliPhysicsVersion("vAN-20180429-1");
        // set the Alien API version
        alienHandler->SetAPIVersion("V1.1x");
        // select the input data
        alienHandler->SetGridDataDir(Form("/alice/data/2010/LHC10%s",per));
        alienHandler->SetDataPattern("pass4/AOD172/*/AliAOD.root");
        // MC has no prefix, data has prefix 000
        alienHandler->SetRunPrefix("000");

   /*-------------------------*/
        // runnumber
        //alienHandler->AddRunNumber(167813);
        Int_t evN;
        ifstream ff;
        //  ff.open(Form("runs_LHC10%s_lost.list",per));
         ff.open(Form("runs_LHC10%s.list", per));
        //Add runs
        while( !ff.eof() )
        {
         ff>>evN;  
         alienHandler->AddRunNumber(evN);
        }
        ff.close();
   /*-------------------------*/

        // number of files per subjob
        alienHandler->SetSplitMaxInputFileNumber(40);
        alienHandler->SetExecutable("myTask.sh");
        // specify how many seconds your job may take
        alienHandler->SetTTL(86400);
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
        alienHandler->SetGridWorkingDir(Form("pp_Analysis/LHC10%s",per));
        alienHandler->SetGridOutputDir("pass4");

        // connect the alien plugin to the manager
        mgr->SetGridHandler(alienHandler);
        if(gridTest) {
            // speficy on how many files you want to run
            alienHandler->SetNtestFiles(1);
            // and launch the analysis
            alienHandler->SetRunMode("test");
            mgr->StartAnalysis("grid");
        } else {
            // else launch the full grid analysis
            alienHandler->SetRunMode("terminate");
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
