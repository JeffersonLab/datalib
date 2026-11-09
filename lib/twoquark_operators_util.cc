/*! \file
 * \brief Two quark operators factory
 */


#include "datalib/superb/twoquark_operators_util.h"

namespace Hadron
{
  //----------------------------------------------------------------------------------
  namespace 
  {
    //! Make a continuum momentum from integers
    Array<double> contMom(const Array<int>& nf, const Array<int>& latt_size)
    {
      if (nf.size() != Datalib::TensorStorage::dimension_count)
      {
	std::cerr << __func__ << ": invalid size of momentum array: expect Datalib::TensorStorage::dimension_count\n";
	exit(1);
      }

      Array<double> p_c(nf.size());
      const double pi =  3.14159265359;
      const double twopi(2.0 * pi);
    
      for(int mu=0; mu < nf.size(); ++mu)
      {
	p_c[mu] = twopi * double(nf[mu]) / double(latt_size[mu]);
      }
	
      return p_c;
    }
  
  }


  //----------------------------------------------------------------------------------
  Array<int> constructMom4d(const Array<int>& mom, int decay_dir)
  {
    // The 4d phase
    Array<int> mom4d(Datalib::TensorStorage::dimension_count);
    mom4d = 0;
    for(int mu=0,j=0; mu < Datalib::TensorStorage::dimension_count; ++mu)
    {
      if (mu != decay_dir)
	mom4d[mu] = mom[j++];
    }

    return mom4d;
  }


  //----------------------------------------------------------------------------------
  //! Make a momentum phase from a phase mask
  std::complex<double> momPhase(const Array<int>& nf, const Array<int>& mask, const Array<int>& latt_size)
  {
    if ((nf.size() != Datalib::TensorStorage::dimension_count) || (mask.size() != Datalib::TensorStorage::dimension_count))
    {
      std::cerr << __func__ << ": invalid size of momentum array: expect Datalib::TensorStorage::dimension_count\n";
      exit(1);
    }

    // Extract the integer part of the phase using the mask
    Array<int> mom_mask(nf * mask);

    // The continuum momentum
    Array<double> p_c(contMom(mom_mask,latt_size));

    // Collapse them to a single real number
    double phase = p_c[0];
    for(int mu=1; mu < Datalib::TensorStorage::dimension_count; ++mu)
      phase += p_c[mu];

    return std::complex<double>(cos(phase), sin(phase));
  }


  //----------------------------------------------------------------------------------
  //! Compute integer phase associated with a reverse displacement
  Array<int> dispMomMask(const std::vector<int>& path)
  {
    Array<int> phase(Datalib::TensorStorage::dimension_count);
    for (int i = 0; i < phase.size(); i++)
      phase[i] = 0;

    for (int i = 0; i < path.size(); i++)
    {
      int disp_dir = 0;
      int disp_len = 0;

      if (path[i] > 0)
      {
	disp_dir = path[i] - 1;
	disp_len = 1;
      }
      else if (path[i] < 0)
      {
	disp_dir = -path[i] - 1;
	disp_len = -1;
      }
	
      phase[disp_dir] += disp_len;
    }

    return phase;
  }
}  // end namespace Hadron
