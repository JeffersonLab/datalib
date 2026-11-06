#include "datalib/graph/particle_class_op.h"

namespace Hadron
{
namespace
{
    std::ostream& operator<<(std::ostream& os, const Array<int>& d)
    {
      if (d.size() > 0)
      {
	os << d[0];

	for(int i=1; i < d.size(); ++i)
	  os << " " << d[i];
      }

      return os;
    }
}

  Array<int> Mom3d(int p1, int p2, int p3)
  {
    Array<int> d(3);
    
    d[0] = p1;
    d[1] = p2;
    d[2] = p3;
    
    return d;
  }

  Array<int> canonicalOrder(const Array<int>& mom)
  {
    // first step: make all the components positive
    Array<int> mom_tmp = mom;
    for (int mu=0; mu < mom_tmp.size(); ++mu)
      if (mom_tmp[mu] < 0) mom_tmp[mu] = -mom_tmp[mu];

    // Initially, the first item is considered sorted.  mu divides mom
    // into a sorted region (<mu) and an unsorted one (>=mu)
    for (int mu=1; mu < mom_tmp.size(); ++mu) 
    {
      // Select the item at the beginning of the unsorted region
      int v = mom_tmp[mu];
      // Work backwards, finding where v should go
      int nu = mu;
      // If this element is less than v, move it up one
      while (mom_tmp[nu-1] < v) {
	mom_tmp[nu] = mom_tmp[nu-1];
	--nu;
	if (nu < 1) break;
      }
      // Stopped when mom_tmp[nu-1] >= v, so put v at postion nu
      mom_tmp[nu] = v;
    }

    return mom_tmp;
  }

  std::string generateLittleGroup(const Array<int>& mom)
  {
    Array<int> momCan = canonicalOrder(mom);

    std::string littleGroup = "";
    if (momCan[2] == 0)
    {
      if (momCan[1] == 0)
      {
	if (momCan[0] == 0)   // 0 0 0
	  littleGroup = "Oh";
	else                  // n 0 0
	  littleGroup = "D4";
      }
      else
      {
	if (momCan[0] == momCan[1])   // n n 0
	  littleGroup = "D2";
	else
	  littleGroup = "C4nm0";      // n m 0
      }
    }
    else
    {
      if ( (momCan[0] == momCan[1]) && (momCan[0] == momCan[2]) )   // n n n
	littleGroup = "D3";
      else if ( (momCan[0] == momCan[1]) || (momCan[0] == momCan[2]) || (momCan[1] == momCan[2]) )   // n n m
	littleGroup = "C4nnm";
      else   // n m p
	littleGroup = "C2";
    }

    return littleGroup;
  }

  Array<int> referenceMom(const Array<int>& mom)
  {
    std::string group = generateLittleGroup(mom);
    Array<int> momCan = canonicalOrder(mom);
    Array<int> momRef = Mom3d(0,0,0);

    if (group == "Oh")
      momRef = mom;
    else if (group == "D4")   // n00 -> 00n
    {
      int n = momCan[0];
      momRef = Mom3d(0,0,n);
    }
    else if (group == "D2")   // nn0 -> 0nn
    {
      int n = momCan[0];
      momRef = Mom3d(0,n,n);
    }
    else if (group == "D3")   // nnn -> nnn
    {
      momRef = momCan;
    }
    else if(group == "C4nm0")   // nm0 -> 0mn (where n > m)
    {
      int n = momCan[0];
      int m = momCan[1];
      momRef = Mom3d(0,m,n);
    }
    else if(group == "C4nnm")   // nnm/mnn -> nnm
    {
      int n = 0;
      int m = 0;
      if (momCan[0] == momCan[1])   // nnm
      {
	n = momCan[0];
	m = momCan[2];
      }
      else   // mnn
      {
	n = momCan[1];
	m = momCan[0];
      }
      momRef = Mom3d(n,n,m);
    }
    else if(group == "C2")   // nmp
    {
      momRef = momCan;
    }
    else
    {
      std::cerr << __func__ << ": ERROR: unknown group= " << group << " for mom= " << mom << std::endl;
      exit(1);
    }
    return momRef;
  }
}
