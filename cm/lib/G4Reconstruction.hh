#ifndef __G4Reconstruction_H_
#define __G4Reconstruction_H_ 1

#include "BinnedGeometry.hh"
#include "CLog.hh"
#include "G4SimulationAdapter.hh"
#include <TBranch.h>
#include <TFile.h>
#include <TH2F.h>
#include <TObject.h>
#include <TVector3.h>

class G4Reconstruction : public TObject {
public:
  G4Reconstruction() = default;
  // Generate H matrix based on reconstruction data, TTree data is checked first
  // and if it's not avilable histogram data is used
  G4Reconstruction(CameraGeometry sim, TH2F* detector);
  virtual ~G4Reconstruction() = default;

  void RunReconstruction(int nIter);
  void Write(TString filename) const;
  void GetPSF(TMatrixT<Double_t>  fMatrixH);
  TMatrixT<Double_t> ImageSpaceConvolute(TMatrixT<Double_t> image);
  void ReadFit(TString filename);
  TH2F* SmoothGauss(TH2F* hin, double sigma);

private:
  int SingleIteration();
  Double_t CheckConvergence(TH2F reco);

  // Read simulation data from TTree containg all of the events
  TMatrixT<Double_t> ReadFromTTree(TBranch* branch);
  // Read simualation data from histogram (no info about events, only final sum)
  TMatrixT<Double_t> ReadFromTH2F(TH2F* hist);

  std::vector<TMatrixT<Double_t>> fRecoObject;
  // vectorized column matrix representing detector image
  TMatrixT<Double_t> fImage;

  //Sigmavalue
  Double_t fSigma=100.0;
  Int_t fIter=0;

  //OSEM
  int Ns,T;
  int **nums;
  int *length;
  Double_t sumH, sum2;
  Double_t * product;
  TMatrixT<Double_t> S;
  TMatrixT<Double_t> sx, sy, sigmax, sigmay, histomax;
  // correspond to "b", "c", "sigmax", "sigmay" and "a" in Jan paper

  /** Probability matrix
   *  - row represent i-th detector pixel
   *  - column represent j-th position of point source
   *
   *  H[i, j] represents probability of hitting i-th pixel of detector from j-th
   * segment of source.
   */
  TMatrixT<Double_t> fMatrixH;
  // transposition of H matrix for performance reasons
  TMatrixT<Double_t> fMatrixHTranspose;

  CameraGeometry fParams;

  SiFi::logger log = SiFi::createLogger("G4Reconstruction");

  ClassDef(G4Reconstruction, 0)
};

#endif
