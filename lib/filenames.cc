/*! \file
 * \brief Ensemble file names
 */


#include <datalib/graph/filenames.h>
#include <datalib/legacy/hadron_1pt_corr.h>
#include <datalib/legacy/hadron_2pt_corr.h>
#include <datalib/legacy/hadron_3pt_corr.h>

#include <datalib/graph/hadron_class_npart_npt_conn_graph.h>
#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>


#include <datalib/graph/naming.h>
#include <datalib/graph/operator_name.h>

#include <iostream>
#include <sstream>

namespace Hadron
{


  namespace
  {
    //! Error output
    std::ostream& operator<<(std::ostream& os, const XMLArray::Array<int>& d)
    {
      if (d.size() > 0)
      {
	os << d[0];

	for(int i=1; i < d.size(); ++i)
	  os << " " << d[i];
      }

      return os;
    }

    //! Error output
    template<typename T>
    std::ostream& operator<<(std::ostream& os, const std::vector<T>& d)
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


//----------------------------------------------------------------------------
  //! Momentum as a string
  std::string momName(const XMLArray::Array<int>& sink_mom)
  {
    std::ostringstream s;

    s << "px" << sink_mom[0] << "_py" << sink_mom[1] << "_pz" << sink_mom[2];

    return s.str();
  }

  //! Momentum as a string
  std::string momName(const XMLArray::Array<short>& sink_mom)
  {
    std::ostringstream s;

    s << "px" << sink_mom[0] << "_py" << sink_mom[1] << "_pz" << sink_mom[2];

    return s.str();
  }


  //! Momentum as a string
  std::string sourceMomName(const XMLArray::Array<int>& mom)
  {
    std::ostringstream s;

    s << "pix" << mom[0] << "_piy" << mom[1] << "_piz" << mom[2];

    return s.str();
  }


  //! Momentum as a string
  std::string sinkMomName(const XMLArray::Array<int>& mom)
  {
    std::ostringstream s;

    s << "pfx" << mom[0] << "_pfy" << mom[1] << "_pfz" << mom[2];

    return s.str();
  }


  //! Print links
  std::string linkName(const XMLArray::Array<int>& links)
  {
    std::ostringstream s;

    s << "l" << links.size();

    for(int i=0; i < links.size(); ++i)
    {
//    s << "_" << ((links[i] < 0) ? '-' : '+') << links[i];
      s << "_" << links[i];
    }

    return s.str();
  }

  //! Output operator info
  std::string operatorName(const std::string& name, const std::string& smear, const XMLArray::Array<int>& lorentz, int spin_comp)
  {
    std::ostringstream os;

    os << name;

    if (smear != "")
      os << "-" << smear;

    if (spin_comp >= 0)
      os << "," << spin_comp;

    for(int i=0; i < lorentz.size(); ++i)
      os << "," << lorentz[i];

    return os.str();
  }

//----------------------------------------------------------------------------
  //! Disp as a string
  std::string dispName(const std::vector<int>& d)
  {
    std::ostringstream os;

    if (d.size() > 0)
    {
      os << ",";

      for(int i=0; i < d.size(); ++i)
	os << d[i];
    }

    return os.str();
  }


//----------------------------------------------------------------------------
  //! Smear as a string
  std::string smearName(const std::string& d)
  {
    std::ostringstream os;

    if (! d.empty())
    {
      os << "," << d;
    }

    return os.str();
  }


//----------------------------------------------------------------------------
  //! Output other info
  std::string numVecs(int num_vecs)
  {
    std::ostringstream os;

    // Other stuff. In this case, the number of vectors
    os << "n" << num_vecs << ".dat";

    return os.str();
  }

  //! The delimiter
  std::string delimiter()
  {
    std::string str = ".";

    return str;
  }

  // Build a npt corr name
  std::string shortMom(const XMLArray::Array<int>& key)
  {
    std::ostringstream os;

    os << key[0] << key[1] << key[2];

    return os.str();
  }

  //----------------------------------------------------------------------------
  //! Build a file name
  std::string ensemFileName(const FF::KeyHadron1PtCorr_t& key)
  {
    // Build some filename
    std::ostringstream os;

    // Operator
    os << operatorName(key.name, key.smear, key.lorentz, key.spin) << delimiter();

    // Momentum
    os << momName(key.mom) << delimiter();

    // Mass
    os << key.mass << delimiter();

    // Other stuff. In this case, the number of vectors
    os << numVecs(key.num_vecs);

    return os.str();
  }


//----------------------------------------------------------------------------
  //! Build a file name
  std::string ensemFileName(const FF::KeyHadron2PtCorr_t& key)
  {
    // Build some filename
    std::ostringstream os;

    // Source
    os << operatorName(key.src_name, key.src_smear, key.src_lorentz, key.src_spin) << delimiter();

    // Sink
    os << operatorName(key.snk_name, key.snk_smear, key.snk_lorentz, key.snk_spin) << delimiter();

    // Momentum
    os << momName(key.mom) << delimiter();

    // Mass
    os << key.mass << delimiter();

    // Other stuff. In this case, the number of vectors
    os << numVecs(key.num_vecs);

    return os.str();
  }


//----------------------------------------------------------------------------
  //! Build a file name
  std::string ensemFileName(const FF::KeyHadron3PtCorr_t& key)
  {
    // Build some filename
    std::ostringstream os;

    // Source
    os << operatorName(key.src_name, key.src_smear, key.src_lorentz, key.src_spin) << delimiter();

    // Sink
    os << operatorName(key.snk_name, key.snk_smear, key.snk_lorentz, key.snk_spin) << delimiter();

    // Quark line
    os << "q" << key.quark << delimiter();

    // Gamma
    os << "g" << key.gamma << delimiter();

    // Links
    os << linkName(key.links) << delimiter();

    // Momentum
    os << sourceMomName(key.pi_pf.p_i) << delimiter()
       << sinkMomName(key.pi_pf.p_f) << delimiter();

    // Mass
    os << key.mass << delimiter();

    // Other stuff. In this case, the number of vectors
    os << numVecs(key.num_vecs);

    return os.str();
  }


  //----------------------------------------------------------------------------
  namespace
  {
    //! Change all colons to underscores
    std::string convertColons(const std::string& input)
    {
      const char c[] = ":";
      const char u[] = "_";
      std::string graph = input;

      std::size_t found = graph.find(c);
      while (found != std::string::npos)
      {
	//	graph.replace(found, 1, u);
	graph.erase(found, 1);
	found = graph.find(c);
      }

      return graph;
    }

    //! Change all colons to underscores
    std::string makeMomStr(const XMLArray::Array<int>& d)
    {
      std::ostringstream os;

      if (d.size() > 0)
      {
	os << d[0];

	for(int i=1; i < d.size(); ++i)
	{
	  os << d[i];
	}
      }

      return os.str();
    }
  }


  //----------------------------------------------------------------------------
  //! Build a graph file name
  std::string ensemFileName(const KeyHadronClassNPartNPtConnGraph_t& key)
  {
    // Build filename
    std::ostringstream os;
    os << key.graph;

    os << ".t";
    for(auto t = key.diag_time_slices.t_slices.ref().begin(); t != key.diag_time_slices.t_slices.ref().end(); ++t)
    {
      os << *t;
    }

#if 1
    for(auto v = key.adj_map.begin(); v != key.adj_map.end(); ++v)
    {
      os << ".v" << v->first;

      for(auto s = v->second.begin(); s != v->second.end(); ++s)
      {
	os << "s" << s->first << "t" << s->second;
      }
    }
#else
    os << "." << key.adj_map;
#endif


    for(auto v = key.vertices.begin(); v != key.vertices.end(); ++v)
    {
      os << ".V" << v->first << "_" << v->second.first.type << v->second.first.piece_num << ((v->second.first.creation_op) ? 'C' : 'c')
	 << makeMomStr(v->second.first.mom) << "," << v->second.first.name << "," << v->second.second;
    }

    os << ".dat";

    return convertColons(os.str());
  }

  //----------------------------------------------------------------------------
  // Build a npt corr name
  std::string ensemFileName(const Hadron::KeyHadronNPartIrrep_t& key)
  {
    std::ostringstream os;

    // Sanity check
    if (key.op.ops.size() == 0)
    {
      std::cerr << __func__ << ": Irrep - have no ops\n";
      exit(1);
    }

    if (key.op.CGs.size() == 0 && key.op.ops.size() != 1)
    {
      std::cerr << __func__ << ": Irrep - have no CGs but more than 1 op\n";
      exit(1);
    }

    // Isospin
    if (key.op.ops.size() == 1)
    {
      os << "I" << Hadron::getTwoIsospin(key.op.ops[1].name);
    }
    else
    {
      os << "I" << key.op.CGs[key.op.CGs.size()].target.twoI;
    }

    os << ",iz" << key.twoI_z;

    // Irrep
    // NOTE: CG-pairing not in filename. Use last CG for irrep.
    if (key.op.CGs.size() == 0)
    {
      os << "," << Hadron::getIrrepWithParity(key.op.ops[1].name);
    }
    else
    {
      os << "," << key.op.CGs[key.op.CGs.size()].target.irrep;
    }

    os << ",r" << key.row;
    os << "," << shortMom(key.mom);

    // Operator name + momentum-type
    for(int i=1; i <= key.op.ops.size(); ++i)
    {
      os << "," << key.op.ops[i].name << "," << shortMom(key.op.ops[i].mom_type) << smearName(key.op.ops[i].smear);
    }

    return os.str();
  }

  // Build a npt corr name
  std::string ensemFileName(const Hadron::KeyHadronNPartNPtCorr_t::NPoint_t& key)
  {
    std::ostringstream os;

    if (key.t_slice < 0)
    {
      os << "tm" << -key.t_slice;
    }
    else
    {
      os << "t" << key.t_slice;
    }

    os << "," << ensemFileName(key.irrep);

    return os.str();
  }

  // Build a npt corr name
  std::string ensemFileName(const Hadron::KeyHadronNPartNPtCorr_t& key)
  {
    std::ostringstream os;

    for(int i=1; i <= key.npoint.size(); ++i)
    {
      os << ensemFileName(key.npoint[i]) << delimiter();
    }

    os << "dat";

    return os.str();
  }

  std::string phasingEnsemFileName(const std::vector<XMLArray::Array<int>>& phasing)
  {
    // If no phasing information is provided, then print nothing
    bool no_phasing = true;

    const XMLArray::Array<int> default_phasing(std::vector<int>{0, 0, 0});
    for (const auto& ph : phasing)
      if (ph != default_phasing)
	no_phasing = false;
    if (no_phasing)
      return {};

    // Check if all quarks have the same phasing
    bool same_phasing = true;
    for (const auto& p : phasing)
    {
      if (p != phasing.at(0))
      {
	same_phasing = false;
	break;
      }
    }
    if (same_phasing)
    {
      return std::string("ph")+ shortMom(phasing.at(0));
    }

    // Otherwise, print all phases
    std::string s = "ph";
    for (std::size_t i = 0; i < phasing.size(); ++i)
    {
      s += shortMom(phasing.at(i));
      if (i + 1 < phasing.size())
	s += "~";
    }
    return s;
  }


  //----------------------------------------------------------------------------
  std::string ensemFileName(const KeyParticleClassOp_t& op, const XMLArray::Array<int>& mom)
  {
    std::ostringstream os;

    os << op.name << "__" << shortMom(mom) << phasingEnsemFileName(op.phasings)
       << smearName(op.smear) << dispName(op.disp_list);

    return os.str();
  }

  std::string ensemFileName(const KeyParticleClassOp_t& op)
  {
    return ensemFileName(op, op.mom_class);
  }

  //----------------------------------------------------------------------------
  std::string ensemFileName(const KeyHadronSUNClassNPartIrrepOp_t& op)
  {
    std::ostringstream os;

    // Sanity check
    if (op.ops.size() == 0)
    {
      std::cerr << __func__ << ": Irrep - have no ops\n";
      exit(1);
    }

    if (op.CGs.size() == 0 && op.ops.size() != 1)
    {
      std::cerr << __func__ << ": Irrep - have no CGs but more than 1 op\n";
      exit(1);
    }

    if (op.CGs.size() != (op.ops.size()-1))
    {
      std::cerr << __func__ << ": Number of CGs not one less than number of ops\n";
      exit(1);
    }

    // Single op
    if (op.ops.size() == 1)
    {
      os << ensemFileName(op.ops[1]);
    }

    // Two ops
    if (op.ops.size() == 2)
    {
      // Reference momentum combination
      XMLArray::Array<int> ref_mom1, ref_mom2, ref_momt;
      int num_allowed_mom = refMomCombin(op.ops[1].mom_class, op.ops[2].mom_class, op.CGs[1].target.mom_class,
					 ref_mom1, ref_mom2, ref_momt);
      if (num_allowed_mom < 1)
      {
	std::cerr << __func__ << ": ERROR: problem generating reference momentum combination for "
		  << "(" << op.ops[1].mom_class << ") x (" << op.ops[2].mom_class
		  << ") -> (" << op.CGs[1].target.mom_class << ")" << std::endl;
	exit(1);
      }

      // Operator name + momentum-type
      os << ensemFileName(op.ops[1], ref_mom1);
      os << "xx" << ensemFileName(op.ops[2], ref_mom2);
      os << "__F" << op.CGs[1].target.F;
      os << "_" << op.CGs[1].target.irrep;
      os << "__" << shortMom(ref_momt);
    }

    // Three ops
    if (op.ops.size() == 3)
    {
      // Find reference momentum combination for 12 x 3 -> target
      XMLArray::Array<int> ref_mom12, ref_mom3, ref_momt;
      int num_allowed_mom = refMomCombin(op.CGs[1].target.mom_class, op.ops[3].mom_class, op.CGs[2].target.mom_class,
					 ref_mom12, ref_mom3, ref_momt);
      if (num_allowed_mom < 1)
      {
	std::cerr << __func__ << ": ERROR: problem generating reference momentum combination for "
		  << "(" << op.CGs[1].target.mom_class << ") x (" << op.ops[3].mom_class
		  << ") -> (" << op.CGs[2].target.mom_class << ")" << std::endl;
	exit(1);
      }

      // Find reference momentum combination for 1 x 2 -> 12
      XMLArray::Array<int> ref_mom1, ref_mom2;
      num_allowed_mom = refArbitraryMomCombin(op.ops[1].mom_class, op.ops[2].mom_class, op.CGs[1].target.mom_class, ref_mom12,
					      ref_mom1, ref_mom2);
      if (num_allowed_mom < 1)
      {
	std::cerr << __func__ << ": ERROR: problem generating reference momentum combination for "
		  << "(" << op.ops[1].mom_class << ") x (" << op.ops[2].mom_class
		  << ") -> (" << op.CGs[1].target.mom_class << ")" << std::endl;
	exit(1);
      }

      // Operator name + momentum-type
      os << "XX";
      os << ensemFileName(op.ops[1], ref_mom1);
      os << "xx" << ensemFileName(op.ops[2], ref_mom2);

      os << "__F" << op.CGs[1].target.F;
      os << "_" << op.CGs[1].target.irrep;
      os << "__" << shortMom(ref_mom12);
      os << "XX";

      os << ensemFileName(op.ops[3], ref_mom3);

      os << "__F" << op.CGs[2].target.F;
      os << "_" << op.CGs[2].target.irrep;
      os << "__" << shortMom(ref_momt);
    }

    // At four ops we start to have choices, so switch to a more generic mode
    if (op.ops.size() > 3)
    {
      // Need to update this to use reference momentum combinations
      std::cerr << __func__ << ": op.ops.size()= " << op.ops.size() << " but not implemented for more than 3 ops" << std::endl;
      exit(1);

      for(int i=1; i <= op.ops.size(); ++i)
	os << "K" << i << "V" << ensemFileName(op.ops[i]);

      os << "XX";
      for(auto cg = op.CGs.ref().begin(); cg != op.CGs.ref().end(); ++cg)
      {
	os << "L" << cg->left << "R" << cg->right << "T" << cg->target.slot
	   << "__" << cg->target.F << "_" << cg->target.irrep
	   << "__" << shortMom(cg->target.mom_class);
      }
      os << "XX";
    }

    return os.str();
  }


  //----------------------------------------------------------------------------
  std::string ensemFileName(const KeyHadronSUNClassNPartIrrep_t& key)
  {
    std::ostringstream os;

    os << "f" << key.flavor;
    os << ",r" << key.irrep_mom.row;
    os << "," << shortMom(key.irrep_mom.mom);
    os << "," << ensemFileName(key.op);

    return os.str();
  }


  //----------------------------------------------------------------------------
  // Build a npt corr name
  std::string ensemFileName(const Hadron::KeyHadronSUNClassNPartNPtCorr_t::NPoint_t& key)
  {
    std::ostringstream os;

    if (key.t_slice < 0)
    {
      os << "tm" << -key.t_slice;
    }
    else
    {
      os << "t" << key.t_slice;
    }

    os << "," << ensemFileName(key.irrep);

    return os.str();
  }

  // Build a npt corr name
  std::string ensemFileName(const Hadron::KeyHadronSUNClassNPartNPtCorr_t& key)
  {
    std::ostringstream os;

    for(int i=1; i <= key.npoint.size(); ++i)
    {
      os << ensemFileName(key.npoint[i]) << delimiter();
    }

    os << "dat";

    return os.str();
  }


  // Build a fsq file name

} // namespace ColorVec
