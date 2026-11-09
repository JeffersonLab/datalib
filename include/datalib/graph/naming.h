#pragma once
#include <datalib/graph/particle_class_op.h>
#include <list>
namespace Hadron {
XMLArray::Array<int> Mom3d(int,int,int);
XMLArray::Array<int> canonicalOrder(const XMLArray::Array<int>&);
std::string generateLittleGroup(const XMLArray::Array<int>&);
XMLArray::Array<int> referenceMom(const XMLArray::Array<int>&);
  struct RotateVec_t
  {
    RotateVec_t(int a, int b, int c) : p1(a), p2(b), p3(c) {}

    int  p1;
    int  p2;
    int  p3;
  };


  std::string getIrrepLG(const std::string& irrep);
  int getIrrepDim(const std::string& irrep);
  std::string buildIrrepWithParity(const std::string& irrep, int P);
  std::string buildIrrepWithPG(const std::string& irrep, int P, int G);
  std::string removeHelicity(const std::string& irrep);
  int getIrrepParity(const std::string& irrep);
  std::string removeIrrepParity(const std::string& irrep);
  std::string removeIrrepGParity(const std::string& irrep);
  std::string removeIrrepEmbed(const std::string& irrep);
  int getIrrepEmbed(const std::string& irrep);
  int getIrrepGParity(const std::string& irrep);
  std::string getCubicRepNoParity(const std::string& irrep);
  std::string removeIrrepLG(const std::string& irrep);
  std::string getSingleHadronOpIrrep(const std::string& opname);
  std::string getSingleHadronOpSpinIrrep(const std::string& opname);
  std::string conjOpName(const std::string& opname);
std::list< XMLArray::Array<int> > generateLittleGroupMom(const std::string& littleGroup0, const XMLArray::Array<int>& canon_mom0);
std::list< std::vector<int> > generateLittleGroupMom(const std::string& littleGroup0, const std::vector<int>& canon_mom0);
bool allowedMomentaRotation(const XMLArray::Array<int>& mom1, const XMLArray::Array<int>& mom2, const XMLArray::Array<int>& momt,
			      const XMLArray::Array<int>& mom1p, const XMLArray::Array<int>& mom2p, const XMLArray::Array<int>& momtp);
int allowedMomentaCombin(const std::string& group1, const std::string& group2,
			   const XMLArray::Array<int>& orig_mom1, const XMLArray::Array<int>& orig_mom2, const XMLArray::Array<int>& orig_momt,
			   const XMLArray::Array<int>& momt, std::list< XMLArray::Array<int> >& allowed_mom1, std::list< XMLArray::Array<int> >& allowed_mom2);
int refMomCombin(const XMLArray::Array<int>& mom1, const XMLArray::Array<int>& mom2, const XMLArray::Array<int>& momt,
		   XMLArray::Array<int>& ref_mom1, XMLArray::Array<int>& ref_mom2, XMLArray::Array<int>& ref_momt);
int refCanonMomCombin(const XMLArray::Array<int>& mom1, const XMLArray::Array<int>& mom2, const XMLArray::Array<int>& momt,
			XMLArray::Array<int>& ref_mom1, XMLArray::Array<int>& ref_mom2, XMLArray::Array<int>& canon_momt);
int refArbitraryMomCombin(const XMLArray::Array<int>& mom1, const XMLArray::Array<int>& mom2, const XMLArray::Array<int>& momt, const XMLArray::Array<int>& new_momt,
			    XMLArray::Array<int>& ref_mom1, XMLArray::Array<int>& ref_mom2);
std::vector<RotateVec_t> generateLGRotation(const std::string& rep);
int isign(int x);
XMLArray::Array<int> rotateMom(const XMLArray::Array<int> mom, const RotateVec_t& rot);
RotateVec_t findRotateVec(const XMLArray::Array<int> mom1, const XMLArray::Array<int> mom2);
}
