#include "CLog.hh"
#include "CMSimulation.hh"
#include "CmdLineConfig.hh"
#include "Sources/MultiPointSource.hh"
#include "Sources/PlanarSource.hh"
#include "Sources/PointSource.hh"
#include <TSystem.h>
#include <iostream>
using namespace std;

int main(int argc, char** argv) {

  CmdLineOption opt_output("Output", "-o",
                           "Output file (string), default: simulation.root",
                           "simulation.root");
  CmdLineOption opt_events("Events", "-n",
                           "Number of events, default: 20 (integer)", 1000);
  CmdLineOption opt_source_p("Point", "-point",
                             "Point-type source, default: YES");
  CmdLineOption opt_source_mp("MultiPoint", "-multipoint",
                              "Multi Point-type source, default: NO");
  CmdLineOption opt_source_pl("Planar", "-planar",
                              "Planar-type source, default: NO");
  CmdLineOption opt_detplane_dim(
      "Plane", "-detplane",
      "Detector plane, 6 values required, default: 1:0:0:600:300:300", 0, 0);
  CmdLineOption opt_maskplane_dim(
      "Mask", "-maskplane",
      "Mask plane, 6 values required, default: 1:0:0:500:300:300", 0, 0);

  CmdLineConfig::instance()->ReadCmdLine(argc, argv);

  PositionalArgs args = CmdLineOption::GetPositionalArguments();
  if (args.size() < 2) {
    spdlog::error("{} requires 2 arguments: mask source", argv[0]);
    abort();
  }
  TString mask(argv[2]);
  TString path = TString(gSystem->Getenv("CC6DIR"))+"/share/ComptonCamera6/masks/";
  TFile* maskfile = new TFile(path+"hMURA" + mask + ".root", "READ");
  TH2F* h = 0;
  TString fname = maskfile->GetName();
  if (fname.Contains("2d"))
    h = (TH2F*)maskfile->Get("hMURA2d");
  else
    h = (TH2F*)maskfile->Get("hMURA1d");

  spdlog::set_level(spdlog::level::info);
  TString path =
      TString(gSystem->Getenv("CC6DIR")) + "/share/ComptonCamera6/masks/";
  TString fullname = path + "hMURA" + args[0] + ".root";
  TFile* maskfile = new TFile(fullname, "READ");
  if (maskfile == nullptr) {
    spdlog::error("File with mask: {} does not exist, exit...",
                  fullname.Data());
    abort();
  }
  TH2F* h = TString(maskfile->GetName()).Contains("2d")
                ? (TH2F*)maskfile->Get("hMURA2d")
                : (TH2F*)maskfile->Get("hMURA1d");
  if (!h) {
    spdlog::error("Can't find required mask histograms");
    abort();
  }

  Float_t da = 1., db = 0., dc = 0., dd = 600., de = 300., df = 300.;
  Float_t ma = 1., mb = 0., mc = 0., md = 500., me = 300., mf = 300.;

  if (opt_detplane_dim.GetArraySize() == 6) {
    da = opt_detplane_dim.GetDoubleArrayValue(1);
    db = opt_detplane_dim.GetDoubleArrayValue(2);
    dc = opt_detplane_dim.GetDoubleArrayValue(3);
    dd = opt_detplane_dim.GetDoubleArrayValue(4);
    de = opt_detplane_dim.GetDoubleArrayValue(5);
    df = opt_detplane_dim.GetDoubleArrayValue(6);
  } else if (opt_detplane_dim.GetArraySize() != 0) {
    spdlog::error("Detector plane - 6 parameters required, {} given",
                  opt_detplane_dim.GetArraySize());
    abort();
  }
  if (opt_maskplane_dim.GetArraySize() == 6) {
    ma = opt_maskplane_dim.GetDoubleArrayValue(1);
    mb = opt_maskplane_dim.GetDoubleArrayValue(2);
    mc = opt_maskplane_dim.GetDoubleArrayValue(3);
    md = opt_maskplane_dim.GetDoubleArrayValue(4);
    me = opt_maskplane_dim.GetDoubleArrayValue(5);
    mf = opt_maskplane_dim.GetDoubleArrayValue(6);
  } else if (opt_maskplane_dim.GetArraySize() != 0) {
    spdlog::error("Mask plane - 6 parameters required, {} given",
                  opt_maskplane_dim.GetArraySize());
    abort();
  }
  Int_t source_p = 0, source_mp = 0, source_pl = 0;
  if (opt_source_pl.GetFlagValue()) ++source_pl;
  if (opt_source_mp.GetFlagValue()) ++source_mp;
  if (opt_source_p.GetFlagValue()) ++source_p;

  Int_t source_sum = source_p + source_mp + source_pl;
  if (source_sum > 1) {
    spdlog::error("Source types are mutually exclusive, choose only one");
    abort();
  }
  if (source_sum == 0) ++source_p;

  printf("Detector plane : %g %g %g %g %g %g\n", da, db, dc, dd, de, df);
  printf("Mask plane     : %g %g %g %g %g %g\n", ma, mb, mc, md, me, mf);
  printf("Mask type      : %s\n", args[0].Data());
  printf("Source file    : %s\n", args[1].Data());
  printf("Source type    : [%c] point  [%c] multi-point  [%c] planar\n",
         source_p ? 'x' : ' ', source_mp ? 'x' : ' ', source_pl ? 'x' : ' ');
  printf("No. of events  : %d\n", opt_events.GetIntValue());

  Source* src = nullptr;
  if (source_pl)
    src = new PlanarSource(args[1]);
  else if (source_mp)
    src = new MultiPointSource(args[1]);
  else if (source_p)
    src = new PointSource(args[1]);
  src->Print();

  DetPlane detector(da, db, dc, dd, de, df, "detector");
  Mask mask(ma, mb, mc, md, me, mf, h, "mask");

  CMSimulation sim(src, &mask, &detector);
  sim.RunSimulation(opt_events.GetIntValue());
  sim.Write(opt_output.GetStringValue());

  return 1;
}
