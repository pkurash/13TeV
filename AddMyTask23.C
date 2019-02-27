#if !defined (__CINT__) || defined (__CLING__)
#include "AliAnalysisManager.h"
#include "AliAnalysisTaskGammaPHOS13TeV.h"
#include "AliCaloPhoton.h"
#include <TString.h>
#include <TList.h>
#endif



AliAnalysisTaskGammaPHOS13TeV* AddMyTask23(TString name = "name")
{
    // get the manager via the static access member. since it's static, you don't need
    // an instance of the class to call the function
    AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
    if (!mgr) {
        return 0x0;
    }
    // get the input event handler, again via a static method. 
    // this handler is part of the managing system and feeds events
    // to your task
    if (!mgr->GetInputEventHandler()) {
        return 0x0;
    }
    // by default, a file is open for writing. here, we get the filename
  //  TString fileName = AliAnalysisManager::GetCommonFileName();
    TString fileName  = "PtMC.AOD.1.root";
    //fileName += ":MyTask";      // create a subfolder in the file
    // now we create an instance of your task
    AliAnalysisTaskGammaPHOS13TeV* task = new AliAnalysisTaskGammaPHOS13TeV(name.Data());   
    if(!task) return 0x0;
//    TMacro physseladd(gSystem->ExpandPathName(Form("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C(%d)", kTRUE)));
//    AliPhysicsSelectionTask *physseltask = reinterpret_cast<AliPhysicsSelectionTask *>(physseladd.Exec());;

     TMacro physseladd(gSystem->ExpandPathName("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C"));
     AliPhysicsSelectionTask *physseltask = reinterpret_cast<AliPhysicsSelectionTask *>(physseladd.Exec(Form("%d", kTRUE)));
    task->SelectCollisionCandidates(AliVEvent::kINT7);
    // add your task to the manager
    mgr->AddTask(task);
    // your task needs input: here we connect the manager to your task
    mgr->ConnectInput(task,0,mgr->GetCommonInputContainer());
    // same for the output
    mgr->ConnectOutput(task,1,mgr->CreateContainer("Data", TList::Class(), AliAnalysisManager::kOutputContainer, fileName.Data()));
    mgr->ConnectOutput(task,2,mgr->CreateContainer("Data2",   TList::Class(), AliAnalysisManager::kOutputContainer, fileName.Data()));
    // in the end, this macro returns a pointer to your task. this will be convenient later on
    // when you will run your analysis in an analysis train on grid
    return task;
}

