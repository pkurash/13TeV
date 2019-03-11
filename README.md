Will be updated soon (add Tender non-linearity)

Gamma production analysis in pp collitions in Run2

Code for running locally and on grid.


Run as 

aliroot -b -q ' runTaskGammaPHOS13TeV( Bool_t isMC    =  kFALSE,  TString period  = "LHC16g",  TString runmode = "terminate", Bool_t local = kTRUE)'

changing the parameters of macro at your will:
isMC kTRUE/KFALSE, local = kTRUE/kFALSE, period = LHC16g{h, i, j, k, l, o, p} or MC production cycle, runmode = "full"/"terminate"

