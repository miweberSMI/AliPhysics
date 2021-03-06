void runProtonAnalysisQA(const char* esdAnalysisType = "Hybrid",
			 const char* pidMode = "Ratio",
			 Bool_t kUseOnlineTrigger = kFALSE,
			 Bool_t kUseOfflineTrigger = kFALSE) {
  //Macro to run the proton QA analysis tested for local, proof & GRID.
  //Local: Takes four arguments, the analysis mode, the type of the ESD 
  //       analysis, the PID mode and the path where the tag and ESD or 
  //       AOD files reside.
  //Interactive: Takes four arguments, the analysis mode, the type of the ESD 
  //             analysis, the PID mode and the name of the collection of tag 
  //             files.
  //Batch: Takes four arguments, the analysis mode, the type of the ESD 
  //       analysis, the PID mode and the name of the collection file with 
  //       the event list for each file.
  //Proof: Takes five arguments, the analysis level, the analysis mode in 
  //       case of ESD, the PID mode, the number of events and the dataset 
  //       name and .  
  //Analysis mode can be: "MC", "ESD", "AOD"
  //ESD analysis type can be one of the three: "TPC", "Hybrid", "Global"
  //PID mode can be one of the four: "Bayesian" (standard Bayesian approach) 
  //   "Ratio" (ratio of measured over expected/theoretical dE/dx a la STAR) 
  //   "Sigma1" (N-sigma area around the fitted dE/dx vs P band)
  //   "Sigma2" (same as previous but taking into account the No of TPC points)
  TStopwatch timer;
  timer.Start();
  
  //runLocal("ESD",esdAnalysisType,pidMode,kUseOnlineTrigger,kUseOfflineTrigger,"/home/pchrist/ALICE/Baryons/QA/Local");
  //runProof("ESD",esdAnalysisType,pidMode,kUseOnlineTrigger,kUseOfflineTrigger,100000,"/COMMON/COMMON/LHC10a12_run10482X#esdTree");
  runProof("ESD",esdAnalysisType,pidMode,kUseOnlineTrigger,kUseOfflineTrigger,100000,0,"/COMMON/COMMON/LHC09d10_run10482X#esdTree");
  //runInteractive("ESD",esdAnalysisType,pidMode,kUseOnlineTrigger,kUseOfflineTrigger,"tag104825.xml");
  //runBatch("ESD",esdAnalysisType,pidMode,kUseOnlineTrigger,kUseOfflineTrigger,"wn.xml");

  timer.Stop();
  timer.Print();
}

//_________________________________________________//
void runLocal(const char* mode = "ESD",
	      const char* analysisType = 0x0,
	      const char* pidMode = 0x0,
	      Bool_t kUseOnlineTrigger = kTRUE,
	      Bool_t kUseOfflineTrigger = kTRUE,
	      const char* path = 0x0) {
  TString outputFilename1 = "Protons.QA."; outputFilename1 += analysisType;
  outputFilename1 += "."; outputFilename1 += pidMode; 
  outputFilename1 += ".root"; //main QA file
  TString outputFilename2 = "Protons.MC.QA."; outputFilename2 += analysisType;
  outputFilename2 += "."; outputFilename2 += pidMode; 
  outputFilename2 += ".root"; //MC process QA
  TString outputFilename3 = "Protons.QA.Histograms."; 
  outputFilename3 += analysisType;
  outputFilename3 += "."; outputFilename3 += pidMode; 
  outputFilename3 += ".root"; //Accepted cut distributions
  TString outputFilename4 = "Protons.Efficiency."; 
  outputFilename4 += analysisType;
  outputFilename4 += "."; outputFilename4 += pidMode; 
  outputFilename4 += ".root"; //Reco and PID efficiency
  TString outputFilename5 = "Vertex.QA.root"; //vertex QA
  TString eventStatsFilename = "eventStats.root";//event stats

  gSystem->Load("libProofPlayer");

  //Setup the par files
  setupPar("STEERBase");
  gSystem->Load("libSTEERBase");
  setupPar("ESD");
  gSystem->Load("libESD");
  setupPar("AOD");
  gSystem->Load("libAOD");
  setupPar("ANALYSIS");
  gSystem->Load("libANALYSIS");
  setupPar("ANALYSISalice");
  gSystem->Load("libANALYSISalice");
  setupPar("CORRFW");
  gSystem->Load("libCORRFW");
  setupPar("PWG2spectra");
  gSystem->Load("libPWG2spectra");

   //____________________________________________//
  AliTagAnalysis *tagAnalysis = new AliTagAnalysis("ESD"); 
  tagAnalysis->ChainLocalTags(path);

  AliRunTagCuts *runCuts = new AliRunTagCuts();
  AliLHCTagCuts *lhcCuts = new AliLHCTagCuts();
  AliDetectorTagCuts *detCuts = new AliDetectorTagCuts();
  AliEventTagCuts *evCuts = new AliEventTagCuts();
  
  TChain* chain = 0x0;
  chain = tagAnalysis->QueryTags(runCuts,lhcCuts,detCuts,evCuts);
  chain->SetBranchStatus("*Calo*",0);

  //____________________________________________//
  gROOT->LoadMacro("configProtonAnalysis.C");
  AliProtonQAAnalysis *analysis = GetProtonQAAnalysisObject(mode,
							    analysisType,
							    pidMode,
							    kUseOnlineTrigger,
							    kUseOfflineTrigger);
  //____________________________________________//
  // Make the analysis manager
  AliAnalysisManager *mgr = new AliAnalysisManager("protonAnalysisQAManager");
  AliVEventHandler* esdH = new AliESDInputHandler;
  mgr->SetInputEventHandler(esdH);
  AliMCEventHandler *mc = new AliMCEventHandler();
  mgr->SetMCtruthEventHandler(mc);
  
  //____________________________________________//
  // 1st Proton task
  AliAnalysisTaskProtonsQA *taskProtonsQA = new AliAnalysisTaskProtonsQA("TaskProtonsQA");
  taskProtonsQA->SetAnalysisObject(analysis);
  mgr->AddTask(taskProtonsQA);

  // Create containers for input/output
  /*AliAnalysisDataContainer *cinput1 = mgr->CreateContainer("dataChain",
							   TChain::Class(),
							   AliAnalysisManager::kInputContainer);*/
  AliAnalysisDataContainer *cinput1  = mgr->GetCommonInputContainer();
  AliAnalysisDataContainer *coutput1 = mgr->CreateContainer("globalQAList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename1.Data());
  AliAnalysisDataContainer *coutput2 = mgr->CreateContainer("pdgCodeList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename2.Data());
  AliAnalysisDataContainer *coutput3 = mgr->CreateContainer("mcProcessList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename2.Data());
  AliAnalysisDataContainer *coutput4 = mgr->CreateContainer("acceptedCutList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename3.Data());
  AliAnalysisDataContainer *coutput5 = mgr->CreateContainer("rejectedCutList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename3.Data());
  AliAnalysisDataContainer *coutput6 = mgr->CreateContainer("acceptedDCAList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename3.Data());
  AliAnalysisDataContainer *coutput7 = mgr->CreateContainer("efficiencyList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename4.Data());
  AliAnalysisDataContainer *coutput8 = mgr->CreateContainer("vertexList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename5.Data());
  AliAnalysisDataContainer *coutput9 = mgr->CreateContainer("cutEfficiencyList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename4.Data());
  AliAnalysisDataContainer *coutput10 = mgr->CreateContainer("fHistEventStats", 
							     TH1::Class(),
							     AliAnalysisManager::kOutputContainer,
							     eventStatsFilename.Data());
  
  //____________________________________________//
  mgr->ConnectInput(taskProtonsQA,0,cinput1);
  mgr->ConnectOutput(taskProtonsQA,0,coutput1);
  mgr->ConnectOutput(taskProtonsQA,1,coutput2);
  mgr->ConnectOutput(taskProtonsQA,2,coutput3);
  mgr->ConnectOutput(taskProtonsQA,3,coutput4);
  mgr->ConnectOutput(taskProtonsQA,4,coutput5);
  mgr->ConnectOutput(taskProtonsQA,5,coutput6);
  mgr->ConnectOutput(taskProtonsQA,6,coutput7);
  mgr->ConnectOutput(taskProtonsQA,7,coutput8);
  mgr->ConnectOutput(taskProtonsQA,8,coutput9);
  mgr->ConnectOutput(taskProtonsQA,9,coutput10);

  if (!mgr->InitAnalysis()) return;
  mgr->PrintStatus();
  mgr->StartAnalysis("local",chain);
}

//_________________________________________________//
void runInteractive(const char* mode = "ESD",
		    const char* analysisType = 0x0,
		    const char* pidMode = 0x0,
		    Bool_t kUseOnlineTrigger = kTRUE,
		    Bool_t kUseOfflineTrigger = kTRUE,
		    const char* collectionName = "tag.xml") {
  TString outputFilename1 = "Protons.QA."; outputFilename1 += analysisType;
  outputFilename1 += "."; outputFilename1 += pidMode; 
  outputFilename1 += ".root"; //main QA file
  TString outputFilename2 = "Protons.MC.QA."; outputFilename2 += analysisType;
  outputFilename2 += "."; outputFilename2 += pidMode; 
  outputFilename2 += ".root"; //MC process QA
  TString outputFilename3 = "Protons.QA.Histograms."; 
  outputFilename3 += analysisType;
  outputFilename3 += "."; outputFilename3 += pidMode; 
  outputFilename3 += ".root"; //Accepted cut distributions
  TString outputFilename4 = "Protons.Efficiency."; 
  outputFilename4 += analysisType;
  outputFilename4 += "."; outputFilename4 += pidMode; 
  outputFilename4 += ".root"; //Reco and PID efficiency
  TString outputFilename5 = "Vertex.QA.root"; //vertex QA
  TString eventStatsFilename = "eventStats.root";//event stats

  TGrid::Connect("alien://");

  //Setup the par files
  setupPar("STEERBase");
  gSystem->Load("libSTEERBase");
  setupPar("ESD");
  gSystem->Load("libESD");
  setupPar("AOD");
  gSystem->Load("libAOD");
  setupPar("ANALYSIS");
  gSystem->Load("libANALYSIS");
  setupPar("ANALYSISalice");
  gSystem->Load("libANALYSISalice");
  setupPar("CORRFW");
  gSystem->Load("libCORRFW");
  setupPar("PWG2spectra");
  gSystem->Load("libPWG2spectra");

  //____________________________________________//
  AliTagAnalysis *tagAnalysis = new AliTagAnalysis("ESD");
 
  AliRunTagCuts *runCuts = new AliRunTagCuts();
  AliLHCTagCuts *lhcCuts = new AliLHCTagCuts();
  AliDetectorTagCuts *detCuts = new AliDetectorTagCuts();
  AliEventTagCuts *evCuts = new AliEventTagCuts();
 
  //grid tags
  TGridCollection* coll = gGrid->OpenCollection(collectionName);
  TGridResult* TagResult = coll->GetGridResult("",0,0);
  tagAnalysis->ChainGridTags(TagResult);
  TChain* chain = 0x0;
  chain = tagAnalysis->QueryTags(runCuts,lhcCuts,detCuts,evCuts);
  chain->SetBranchStatus("*Calo*",0);

  //____________________________________________//
  gROOT->LoadMacro("configProtonAnalysis.C");
  AliProtonQAAnalysis *analysis = GetProtonQAAnalysisObject(mode,
							    analysisType,
							    pidMode,
							    kUseOnlineTrigger,
							    kUseOfflineTrigger);
  //____________________________________________//
  // Make the analysis manager
  AliAnalysisManager *mgr = new AliAnalysisManager("protonAnalysisQAManager");
  AliVEventHandler* esdH = new AliESDInputHandler;
  mgr->SetInputEventHandler(esdH);
  AliMCEventHandler *mc = new AliMCEventHandler();
  mgr->SetMCtruthEventHandler(mc);
  
  //____________________________________________//
  // 1st Proton task
  AliAnalysisTaskProtonsQA *taskProtonsQA = new AliAnalysisTaskProtonsQA("TaskProtonsQA");
  taskProtonsQA->SetAnalysisObject(analysis);
  mgr->AddTask(taskProtonsQA);

  // Create containers for input/output
  /*AliAnalysisDataContainer *cinput1 = mgr->CreateContainer("dataChain",
                                                           TChain::Class(),
                                                           AliAnalysisManager::kInputContainer);*/
  AliAnalysisDataContainer *cinput1  = mgr->GetCommonInputContainer();
  AliAnalysisDataContainer *coutput1 = mgr->CreateContainer("globalQAList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename1.Data());
  AliAnalysisDataContainer *coutput2 = mgr->CreateContainer("pdgCodeList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename2.Data());
  AliAnalysisDataContainer *coutput3 = mgr->CreateContainer("mcProcessList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename2.Data());
  AliAnalysisDataContainer *coutput4 = mgr->CreateContainer("acceptedCutList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename3.Data());
  AliAnalysisDataContainer *coutput5 = mgr->CreateContainer("rejectedCutList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename3.Data());
  AliAnalysisDataContainer *coutput6 = mgr->CreateContainer("acceptedDCAList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename3.Data());
  AliAnalysisDataContainer *coutput7 = mgr->CreateContainer("efficiencyList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename4.Data());
  AliAnalysisDataContainer *coutput8 = mgr->CreateContainer("vertexList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename5.Data());
  AliAnalysisDataContainer *coutput9 = mgr->CreateContainer("cutEfficiencyList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename4.Data());
  AliAnalysisDataContainer *coutput10 = mgr->CreateContainer("fHistEventStats", 
							     TH1::Class(),
							     AliAnalysisManager::kOutputContainer,
							     eventStatsFilename.Data());

  //____________________________________________//
  mgr->ConnectInput(taskProtonsQA,0,cinput1);
  mgr->ConnectOutput(taskProtonsQA,0,coutput1);
  mgr->ConnectOutput(taskProtonsQA,1,coutput2);
  mgr->ConnectOutput(taskProtonsQA,2,coutput3);
  mgr->ConnectOutput(taskProtonsQA,3,coutput4);
  mgr->ConnectOutput(taskProtonsQA,4,coutput5);
  mgr->ConnectOutput(taskProtonsQA,5,coutput6);
  mgr->ConnectOutput(taskProtonsQA,6,coutput7);
  mgr->ConnectOutput(taskProtonsQA,7,coutput8);
  mgr->ConnectOutput(taskProtonsQA,8,coutput9);
  mgr->ConnectOutput(taskProtonsQA,9,coutput10);

  if (!mgr->InitAnalysis()) return;
  mgr->PrintStatus();
  mgr->StartAnalysis("local",chain);
}

//_________________________________________________//
void runBatch(const char* mode = "ESD",
	      const char* analysisType = 0x0,
	      const char* pidMode = 0x0,
	      Bool_t kUseOnlineTrigger = kTRUE,
	      Bool_t kUseOfflineTrigger = kTRUE,
	      const char *collectionfile = "wn.xml") {
  TString outputFilename1 = "Protons.QA."; outputFilename1 += analysisType;
  outputFilename1 += "."; outputFilename1 += pidMode; 
  outputFilename1 += ".root"; //main QA file
  TString outputFilename2 = "Protons.MC.QA."; outputFilename2 += analysisType;
  outputFilename2 += "."; outputFilename2 += pidMode; 
  outputFilename2 += ".root"; //MC process QA
  TString outputFilename3 = "Protons.QA.Histograms."; 
  outputFilename3 += analysisType;
  outputFilename3 += "."; outputFilename3 += pidMode; 
  outputFilename3 += ".root"; //Accepted cut distributions
  TString outputFilename4 = "Protons.Efficiency."; 
  outputFilename4 += analysisType;
  outputFilename4 += "."; outputFilename4 += pidMode; 
  outputFilename4 += ".root"; //Reco and PID efficiency
  TString outputFilename5 = "Vertex.QA.root"; //vertex QA
  TString eventStatsFilename = "eventStats.root";//event stats

  TGrid::Connect("alien://");
  gSystem->Load("libProofPlayer");

  //Setup the par files
  setupPar("STEERBase");
  gSystem->Load("libSTEERBase");
  setupPar("ESD");
  gSystem->Load("libESD");
  setupPar("AOD");
  gSystem->Load("libAOD");
  setupPar("ANALYSIS");
  gSystem->Load("libANALYSIS");
  setupPar("ANALYSISalice");
  gSystem->Load("libANALYSISalice");
  setupPar("CORRFW");
  gSystem->Load("libCORRFW");
  setupPar("PWG2spectra");
  gSystem->Load("libPWG2spectra");

  //____________________________________________//
  //Usage of event tags
  AliTagAnalysis *tagAnalysis = new AliTagAnalysis();
  TChain *chain = 0x0;
  chain = tagAnalysis->GetChainFromCollection(collectionfile,"esdTree");
  chain->SetBranchStatus("*Calo*",0);
  
  //____________________________________________//
  gROOT->LoadMacro("configProtonAnalysis.C");
  AliProtonQAAnalysis *analysis = GetProtonQAAnalysisObject(mode,
							    analysisType,
							    pidMode,
							    kUseOnlineTrigger,
							    kUseOfflineTrigger);
  //____________________________________________//
  // Make the analysis manager
  AliAnalysisManager *mgr = new AliAnalysisManager("protonAnalysisQAManager");
  AliVEventHandler* esdH = new AliESDInputHandler;
  mgr->SetInputEventHandler(esdH);
  AliMCEventHandler *mc = new AliMCEventHandler();
  mgr->SetMCtruthEventHandler(mc);
  
  //____________________________________________//
  // 1st Proton task
  AliAnalysisTaskProtonsQA *taskProtonsQA = new AliAnalysisTaskProtonsQA("TaskProtonsQA");
  taskProtonsQA->SetAnalysisObject(analysis);
  mgr->AddTask(taskProtonsQA);

  // Create containers for input/output
  /*AliAnalysisDataContainer *cinput1 = mgr->CreateContainer("dataChain",
							   TChain::Class(),
							   AliAnalysisManager::kInputContainer);*/
  AliAnalysisDataContainer *cinput1  = mgr->GetCommonInputContainer();
  AliAnalysisDataContainer *coutput1 = mgr->CreateContainer("globalQAList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename1.Data());
  AliAnalysisDataContainer *coutput2 = mgr->CreateContainer("pdgCodeList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename2.Data());
  AliAnalysisDataContainer *coutput3 = mgr->CreateContainer("mcProcessList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename2.Data());
  AliAnalysisDataContainer *coutput4 = mgr->CreateContainer("acceptedCutList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename3.Data());
  AliAnalysisDataContainer *coutput5 = mgr->CreateContainer("rejectedCutList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename3.Data());
  AliAnalysisDataContainer *coutput6 = mgr->CreateContainer("acceptedDCAList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename3.Data());
  AliAnalysisDataContainer *coutput7 = mgr->CreateContainer("efficiencyList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename4.Data());
  AliAnalysisDataContainer *coutput8 = mgr->CreateContainer("vertexList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename5.Data());
  AliAnalysisDataContainer *coutput9 = mgr->CreateContainer("cutEfficiencyList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename4.Data());
  AliAnalysisDataContainer *coutput10 = mgr->CreateContainer("fHistEventStats", 
							     TH1::Class(),
							     AliAnalysisManager::kOutputContainer,
							     eventStatsFilename.Data());

  //____________________________________________//
  mgr->ConnectInput(taskProtonsQA,0,cinput1);
  mgr->ConnectOutput(taskProtonsQA,0,coutput1);
  mgr->ConnectOutput(taskProtonsQA,1,coutput2);
  mgr->ConnectOutput(taskProtonsQA,2,coutput3);
  mgr->ConnectOutput(taskProtonsQA,3,coutput4);
  mgr->ConnectOutput(taskProtonsQA,4,coutput5);
  mgr->ConnectOutput(taskProtonsQA,5,coutput6);
  mgr->ConnectOutput(taskProtonsQA,6,coutput7);
  mgr->ConnectOutput(taskProtonsQA,7,coutput8);
  mgr->ConnectOutput(taskProtonsQA,8,coutput9);
  mgr->ConnectOutput(taskProtonsQA,9,coutput10);

  if (!mgr->InitAnalysis()) return;
  mgr->PrintStatus();
  mgr->StartAnalysis("local",chain);
}
 
//_________________________________________________//
void runProof(const char* mode = "ESD",
	      const char* analysisType = 0x0,
	      const char* pidMode = 0x0,
	      Bool_t kUseOnlineTrigger = kTRUE,
	      Bool_t kUseOfflineTrigger = kTRUE,
	      Int_t stats = 0, Int_t startingPoint = 0,
	      const char* dataset = 0x0) {
  TString outputFilename1 = "Protons.QA."; outputFilename1 += analysisType;
  outputFilename1 += "."; outputFilename1 += pidMode; 
  outputFilename1 += ".root"; //main QA file
  TString outputFilename2 = "Protons.MC.QA."; outputFilename2 += analysisType;
  outputFilename2 += "."; outputFilename2 += pidMode; 
  outputFilename2 += ".root"; //MC process QA
  TString outputFilename3 = "Protons.QA.Histograms."; 
  outputFilename3 += analysisType;
  outputFilename3 += "."; outputFilename3 += pidMode; 
  outputFilename3 += ".root"; //Accepted cut distributions
  TString outputFilename4 = "Protons.Efficiency."; 
  outputFilename4 += analysisType;
  outputFilename4 += "."; outputFilename4 += pidMode; 
  outputFilename4 += ".root"; //Reco and PID efficiency
  TString outputFilename5 = "Vertex.QA.root"; //vertex Q
  TString eventStatsFilename = "eventStats.root";//event stats

  gEnv->SetValue("XSec.GSI.DelegProxy","2");
  printf("****** Connect to PROOF *******\n");
  TProof::Open("alicecaf.cern.ch"); 
  gProof->SetParallel();

  // Enable the Analysis Package
  gProof->UploadPackage("STEERBase.par");
  gProof->EnablePackage("STEERBase");
  gProof->UploadPackage("ESD.par");
  gProof->EnablePackage("ESD");
  gProof->UploadPackage("AOD.par");
  gProof->EnablePackage("AOD");
  gProof->UploadPackage("ANALYSIS.par");
  gProof->EnablePackage("ANALYSIS");
  gProof->UploadPackage("ANALYSISalice.par");
  gProof->EnablePackage("ANALYSISalice");
  gProof->UploadPackage("CORRFW.par");
  gProof->EnablePackage("CORRFW");
  gProof->UploadPackage("PWG2spectra.par");
  gProof->EnablePackage("PWG2spectra");
  
  //____________________________________________//
  //gProof->Load("configProtonAnalysis.C");
  gROOT->LoadMacro("configProtonAnalysis.C");
  AliProtonQAAnalysis *analysis = GetProtonQAAnalysisObject(mode,
							    analysisType,
							    pidMode,
							    kUseOnlineTrigger,
							    kUseOfflineTrigger);
  //____________________________________________//
  // Make the analysis manager
  AliAnalysisManager *mgr = new AliAnalysisManager("protonAnalysisQAManager");
  AliVEventHandler* esdH = new AliESDInputHandler;
  mgr->SetInputEventHandler(esdH);
  AliMCEventHandler *mc = new AliMCEventHandler();
  mgr->SetMCtruthEventHandler(mc);
  
  //____________________________________________//
  // 1st Proton task
  AliAnalysisTaskProtonsQA *taskProtonsQA = new AliAnalysisTaskProtonsQA("TaskProtonsQA");
  taskProtonsQA->SetAnalysisObject(analysis);
  mgr->AddTask(taskProtonsQA);

  // Create containers for input/output
  /*AliAnalysisDataContainer *cinput1 = mgr->CreateContainer("dataChain",
                                                           TChain::Class(),
                                                           AliAnalysisManager::kInputContainer);*/
  AliAnalysisDataContainer *cinput1  = mgr->GetCommonInputContainer();
  AliAnalysisDataContainer *coutput1 = mgr->CreateContainer("globalQAList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename1.Data());
  AliAnalysisDataContainer *coutput2 = mgr->CreateContainer("pdgCodeList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename2.Data());
  AliAnalysisDataContainer *coutput3 = mgr->CreateContainer("mcProcessList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename2.Data());
  AliAnalysisDataContainer *coutput4 = mgr->CreateContainer("acceptedCutList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename3.Data());
  AliAnalysisDataContainer *coutput5 = mgr->CreateContainer("rejectedCutList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename3.Data());
  AliAnalysisDataContainer *coutput6 = mgr->CreateContainer("acceptedDCAList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename3.Data());
  AliAnalysisDataContainer *coutput7 = mgr->CreateContainer("efficiencyList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename4.Data());
  AliAnalysisDataContainer *coutput8 = mgr->CreateContainer("vertexList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename5.Data());
  AliAnalysisDataContainer *coutput9 = mgr->CreateContainer("cutEfficiencyList", 
							    TList::Class(),
							    AliAnalysisManager::kOutputContainer,
							    outputFilename4.Data());
  AliAnalysisDataContainer *coutput10 = mgr->CreateContainer("fHistEventStats", 
							     TH1::Class(),
							     AliAnalysisManager::kOutputContainer,
							     eventStatsFilename.Data());

  //____________________________________________//
  mgr->ConnectInput(taskProtonsQA,0,cinput1);
  mgr->ConnectOutput(taskProtonsQA,0,coutput1);
  mgr->ConnectOutput(taskProtonsQA,1,coutput2);
  mgr->ConnectOutput(taskProtonsQA,2,coutput3);
  mgr->ConnectOutput(taskProtonsQA,3,coutput4);
  mgr->ConnectOutput(taskProtonsQA,4,coutput5);
  mgr->ConnectOutput(taskProtonsQA,5,coutput6);
  mgr->ConnectOutput(taskProtonsQA,6,coutput7);
  mgr->ConnectOutput(taskProtonsQA,7,coutput8);
  mgr->ConnectOutput(taskProtonsQA,8,coutput9);
  mgr->ConnectOutput(taskProtonsQA,9,coutput10);

  if (!mgr->InitAnalysis()) return;
  mgr->PrintStatus();

  if(dataset)
    mgr->StartAnalysis("proof",dataset,stats,startingPoint);
  else {
    // You should get this macro and the txt file from:
    // http://aliceinfo.cern.ch/Offline/Analysis/CAF/
    gROOT->LoadMacro("CreateESDChain.C");
    TChain* chain = 0x0;
    chain = CreateESDChain("ESD82XX_30K.txt",stats);
    chain->SetBranchStatus("*Calo*",0);

    mgr->StartAnalysis("proof",chain);
    //mgr->StartAnalysis("local",chain);
  }
}

//_________________________________________________//
Int_t setupPar(const char* pararchivename) {
  ///////////////////
  // Setup PAR File//
  ///////////////////
  if (pararchivename) {
    char processline[1024];
    sprintf(processline,".! tar xvzf %s.par",pararchivename);
    gROOT->ProcessLine(processline);
    const char* ocwd = gSystem->WorkingDirectory();
    gSystem->ChangeDirectory(pararchivename);
    
    // check for BUILD.sh and execute
    if (!gSystem->AccessPathName("PROOF-INF/BUILD.sh")) {
      printf("*******************************\n");
      printf("*** Building PAR archive    ***\n");
      printf("*******************************\n");
      
      if (gSystem->Exec("PROOF-INF/BUILD.sh")) {
        Error("runAnalysis","Cannot Build the PAR Archive! - Abort!");
        return -1;
      }
    }
    // check for SETUP.C and execute
    if (!gSystem->AccessPathName("PROOF-INF/SETUP.C")) {
      printf("*******************************\n");
      printf("*** Setup PAR archive       ***\n");
      printf("*******************************\n");
      gROOT->Macro("PROOF-INF/SETUP.C");
    }
    
    gSystem->ChangeDirectory("../");
  } 
  return 1;
}
