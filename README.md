Gamma production analysis in pp collitions in Run2

Code for running locally and on grid.

Use '13' and '23' macros for analysis of real and MC data, respectively.

Run as 

aliroot -b -q  'runAnalysis13.C(Bool_t local = kTRUE, char* period = "LHC16g", char* runmode = "terminate")'

changing the parameters of macro at your will
(local = kTRUE/kFALSE, period = "LHC16{g}/(h, i, j, k, l, o, p)", runmode "full"/"terminate")

In the case
