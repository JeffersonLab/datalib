#include <datalib/graph/naming.h>
#include <map>
#include <sstream>
#include <iostream>
#include <cstdlib>
namespace Hadron {
namespace {template<class T> XMLArray::Array<T> convertArray(const std::vector<T>& a){XMLArray::Array<T> b(a.size());for(int i=0;i<b.size();++i)b[i]=a[i];return b;} template<class T> std::vector<T> convertArray(const XMLArray::Array<T>& a){std::vector<T> b(a.size());for(int i=0;i<a.size();++i)b[i]=a[i];return b;} std::ostream& operator<<(std::ostream& o,const XMLArray::Array<int>& a){for(int i=0;i<a.size();++i){if(i)o<<" ";o<<a[i];}return o;} }
  //----------------------------------------------------------------------------------
  //! Irrep names
  namespace
  {
    // Irrep names
    struct IrrepNames_t
    {
      IrrepNames_t() : ferm(false), lg(false), dim(0), G(0) {}
      IrrepNames_t(const std::string& no_par_, bool ferm_, bool lg_, int dim_, int g_) :
	with_par(no_par_), no_par(no_par_), ferm(ferm_), lg(lg_), dim(dim_), G(g_) {}
      IrrepNames_t(const std::string& with_par_, const std::string& no_par_, bool ferm_, bool lg_, int dim_, int g_) :
	with_par(with_par_), no_par(no_par_), ferm(ferm_), lg(lg_), dim(dim_), G(g_) {}

      std::string  with_par;  /*!< Irrep with parity */
      std::string  no_par;    /*!< Irrep no parity */
      bool         ferm;      /*!< Is this a double cover? */
      bool         lg;        /*!< LG? */
      int          dim;       /*!< dimension */
      int          G;         /*!< G-parity */
    };

    // Initialized
    bool initIR(false);

    // Irrep names
    typedef std::map<std::string, IrrepNames_t> IRNames_t;

    IRNames_t irrep_names_no_par;
    IRNames_t irrep_names_with_par;
    IRNames_t irrep_names_with_pg;

    //---------------------------------------------------------------------------
    // Initialize irrep names
    void initIrrepNames()
    {
      if (initIR) {return;}

      // LG
      irrep_names_no_par.insert(std::make_pair("D4A1",IrrepNames_t("D4A1",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("D4A2",IrrepNames_t("D4A2",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("D4E1",IrrepNames_t("D4E1",true,true,2,0)));
      irrep_names_no_par.insert(std::make_pair("D4E2",IrrepNames_t("D4E2",false,true,2,0)));
      irrep_names_no_par.insert(std::make_pair("D4E3",IrrepNames_t("D4E3",true,true,2,0)));
      irrep_names_no_par.insert(std::make_pair("D4B1",IrrepNames_t("D4B1",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("D4B2",IrrepNames_t("D4B2",false,true,1,0)));

      irrep_names_no_par.insert(std::make_pair("D3A1",IrrepNames_t("D3A1",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("D3A2",IrrepNames_t("D3A2",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("D3E1",IrrepNames_t("D3E1",true,true,2,0)));
      irrep_names_no_par.insert(std::make_pair("D3E2",IrrepNames_t("D3E2",false,true,2,0)));
      irrep_names_no_par.insert(std::make_pair("D3B1",IrrepNames_t("D3B1",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("D3B2",IrrepNames_t("D3B2",false,true,1,0)));

      irrep_names_no_par.insert(std::make_pair("D2A1",IrrepNames_t("D2A1",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("D2A2",IrrepNames_t("D2A2",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("D2E",IrrepNames_t("D2E",true,true,2,0)));
      irrep_names_no_par.insert(std::make_pair("D2B1",IrrepNames_t("D2B1",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("D2B2",IrrepNames_t("D2B2",false,true,1,0)));

      irrep_names_no_par.insert(std::make_pair("C4nm0A1",IrrepNames_t("C4nm0A1",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("C4nm0A2",IrrepNames_t("C4nm0A2",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("C4nm0E",IrrepNames_t("C4nm0E",true,true,2,0)));

      irrep_names_no_par.insert(std::make_pair("C4nnmA1",IrrepNames_t("C4nnmA1",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("C4nnmA2",IrrepNames_t("C4nnmA2",false,true,1,0)));
      irrep_names_no_par.insert(std::make_pair("C4nnmE",IrrepNames_t("C4nnmE",true,true,2,0)));

      // Oh
      irrep_names_no_par.insert(std::make_pair("A1",IrrepNames_t("A1",false,false,1,0)));
      irrep_names_no_par.insert(std::make_pair("A2",IrrepNames_t("A2",false,false,1,0)));
      irrep_names_no_par.insert(std::make_pair("T1",IrrepNames_t("T1",false,false,3,0)));
      irrep_names_no_par.insert(std::make_pair("T2",IrrepNames_t("T2",false,false,3,0)));
      irrep_names_no_par.insert(std::make_pair("E",IrrepNames_t("E",false,false,2,0)));

      irrep_names_no_par.insert(std::make_pair("G1",IrrepNames_t("G1",true,false,2,0)));
      irrep_names_no_par.insert(std::make_pair("G2",IrrepNames_t("G2",true,false,2,0)));
      irrep_names_no_par.insert(std::make_pair("H",IrrepNames_t("H",true,false,4,0)));


      for(IRNames_t::const_iterator ir = irrep_names_no_par.begin(); ir != irrep_names_no_par.end(); ++ir)
      {
	if (! ir->second.lg)
	{
	  const IrrepNames_t& o = ir->second;

	  if (ir->second.ferm)
	  {
	    irrep_names_with_par.insert(std::make_pair(ir->first + "g",IrrepNames_t(ir->first + "g",o.no_par,o.ferm,o.lg,o.dim,o.G)));
	    irrep_names_with_par.insert(std::make_pair(ir->first + "u",IrrepNames_t(ir->first + "u",o.no_par,o.ferm,o.lg,o.dim,o.G)));
	  }
	  else
	  {
	    irrep_names_with_par.insert(std::make_pair(ir->first + "p",IrrepNames_t(ir->first + "p",o.no_par,o.ferm,o.lg,o.dim,o.G)));
	    irrep_names_with_par.insert(std::make_pair(ir->first + "m",IrrepNames_t(ir->first + "m",o.no_par,o.ferm,o.lg,o.dim,o.G)));
	  }
	}
	else
	{
	  irrep_names_with_par.insert(std::make_pair(ir->first,ir->second));
	}
      }


      // Loop over G parity of (-1, 0, +1)
      for(IRNames_t::const_iterator ir = irrep_names_with_par.begin(); ir != irrep_names_with_par.end(); ++ir)
      {
	const IrrepNames_t& o = ir->second;

	if (! ir->second.ferm)
	{
	  for (int ig1=-1; ig1<=1; ig1++)
	  {
	    std::string g1 = (ig1 == -1) ? "M" : ((ig1 == 1) ? "P" : "");

	    irrep_names_with_pg.insert(std::make_pair(ir->first + g1,IrrepNames_t(ir->first + g1,o.no_par,o.ferm,o.lg,o.dim,ig1)));
	  }
	}
      }

      initIR = true;
    }


    //----------------------------------------------------------------------------------
    // Get irrep
    IrrepNames_t getIrrepIndex(const std::string& irrep)
    {
      initIrrepNames();

      // First check the ones with parity and G-parity
      for(IRNames_t::const_iterator mm = irrep_names_with_pg.begin();
	  mm != irrep_names_with_pg.end();
	  ++mm)
      {
	if ( (irrep.size() == mm->first.size()) && (irrep.compare(0,mm->first.size(),mm->first) == 0) )
	{
	  return mm->second;
	}
      }

      // First check the ones with parity
      for(IRNames_t::const_iterator mm = irrep_names_with_par.begin();
	  mm != irrep_names_with_par.end();
	  ++mm)
      {
	if ( (irrep.size() == mm->first.size()) && (irrep.compare(0,mm->first.size(),mm->first) == 0) )
	{
	  return mm->second;
	}
      }

      // Next try the non-parity versions
      for(IRNames_t::const_iterator mm = irrep_names_no_par.begin();
	  mm != irrep_names_no_par.end();
	  ++mm)
      {
	if ( (irrep.size() == mm->first.size()) && (irrep.compare(0,mm->first.size(),mm->first) == 0) )
	{
	  return mm->second;
	}
      }

      std::cerr << __func__ << ": ERROR: cannot extract irrep " << irrep << std::endl;
      exit(1);
    }
  } // anonymous namespace


  //----------------------------------------------------------------------------------
  //! Remove helicity from an irrep name
  std::string removeHelicity(const std::string& irrep)
  {
    // Remove the helicity label
    if ( (irrep.substr(0,3) == "H0D") || (irrep.substr(0,3) == "H1D") || (irrep.substr(0,3) == "H2D") ||
	 (irrep.substr(0,3) == "H3D") || (irrep.substr(0,3) == "H4D") || (irrep.substr(0,3) == "H0C") ||
	 (irrep.substr(0,3) == "H1C") || (irrep.substr(0,3) == "H2C") || (irrep.substr(0,3) == "H3C") ||
	 (irrep.substr(0,3) == "H4C")
      )
    {
      return irrep.substr(2,std::string::npos);
    }

    if ( (irrep.substr(0,5) == "H1o2D") || (irrep.substr(0,5) == "H3o2D") || (irrep.substr(0,5) == "H5o2D") || (irrep.substr(0,5) == "H7o2D") ||
	 (irrep.substr(0,5) == "H1o2C") || (irrep.substr(0,5) == "H3o2C") || (irrep.substr(0,5) == "H5o2C") || (irrep.substr(0,5) == "H7o2C")
      )
    {
      return irrep.substr(4,std::string::npos);
    }

    return irrep;
  }


  //----------------------------------------------------------------------------------
  //! Remove LG from irrep name
  std::string removeIrrepLG(const std::string& irrep)
  {
    std::string rep = irrep;

    // Remove the helicity label
    if ( (rep.substr(0,2) == "D4") || (rep.substr(0,2) == "D2") || (rep.substr(0,2) == "D3") || (rep.substr(0,2) == "C2") || (rep.substr(0,2) == "C4"))
    {
      return rep.substr(2,std::string::npos);
    }
    else if ( (rep.substr(0,3) == "nnm") || (rep.substr(0,3) == "nm0") )
    {
      return rep.substr(3,std::string::npos);
    }


    return rep;
  }


  //----------------------------------------------------------------------------------
  //! Get irrep name from single-hadron operator name
  std::string getSingleHadronOpIrrep(const std::string& opname)
  {
    std::string irrepname;
    std::size_t pos = opname.rfind("_");
    if (pos != std::string::npos)
    {
      irrepname = opname.substr(pos+1);
    }
    else
    {
      std::cerr << __func__ << ": can not determine irrep of op= " << opname << std::endl;
      exit(1);
    }
    return irrepname;
  }


  //----------------------------------------------------------------------------------
  //! Get continuum spin and irrep name from single-hadron operator name
  std::string getSingleHadronOpSpinIrrep(const std::string& opname)
  {
    std::string spinirrepname;
    std::size_t pos = opname.rfind("_");
    if (pos != std::string::npos)
    {
      std::size_t pos2 = opname.rfind("_",pos-1);
      if (pos2 != std::string::npos)
      {
	if (opname.substr(pos2+1,1) == "J")
	  spinirrepname = opname.substr(pos2+1);
	else
	{
	  std::string opnamestart = opname.substr(0,opname.find("_"));
	  if (opnamestart == "pion")
	  {
	    spinirrepname = "J0_" + getSingleHadronOpIrrep(opname);
	  }
	  else if (opnamestart == "eta")
	  {
	    spinirrepname = "J0_" + getSingleHadronOpIrrep(opname);
	  }
	  else if (opnamestart == "Nucleon")
	  {
	    spinirrepname = "J1o2_" + getSingleHadronOpIrrep(opname);
	  }
	  else
	  {
	    std::cerr << __func__ << ": can not determine spin and irrep of op= " << opname << std::endl;
	    exit(1);
	  }
	}
      }
      else
      {
	std::cerr << __func__ << ": can not determine spin and irrep of op= " << opname << std::endl;
	exit(1);
      }
    }
    else
    {
      std::cerr << __func__ << ": can not determine spin and irrep of op= " << opname << std::endl;
      exit(1);
    }
    return spinirrepname;
  }


  //----------------------------------------------------------------------------------
  //! Conjugate an operator name
  std::string conjOpName(const std::string& opname)
  {
    std::string conj_op_name = opname;
    std::string test_string = opname.substr(0,4);
    if ( (test_string == "Kneg") || (test_string == "Kpos")
	 || (test_string == "Dneg") || (test_string == "Dpos")
	 || (test_string == "Eneg") || (test_string == "Epos") )
    {
      conj_op_name.insert(1,"bar");
    }
    else if ( (test_string == "Dsne") || (test_string == "Dspo")
	      || (test_string == "Esne") || (test_string == "Espo") )
    {
      conj_op_name.insert(2,"bar");
    }
    else if ( (test_string == "Kbar") || (test_string == "Dbar") || (test_string == "Ebar") )
    {
      conj_op_name.erase(1,3);
    }
    else if ( (test_string == "Dsba") || (test_string == "Esba") )
    {
      conj_op_name.erase(2,3);
    }
    return conj_op_name;
  }


  //----------------------------------------------------------------------------------
  //! Get parity from an irrep name
  int getIrrepParity(const std::string& irrep)
  {
    std::string irrep_noG = removeIrrepGParity(irrep);
    std::string test_string = irrep_noG.substr(irrep_noG.length()-1);
    if ( (test_string == "p") || (test_string == "g") )
      return 1;
    else if ( (test_string == "m") || (test_string == "u") )
      return -1;
    else
      return 0;
  }


  //----------------------------------------------------------------------------------
  //! Remove parity from an irrep name
  std::string removeIrrepParity(const std::string& irrep)
  {
    std::string test_string = irrep.substr(irrep.length()-1);
    if ((test_string == "p") || (test_string == "m"))
      return irrep.substr(0,irrep.length()-1);
    else if ((test_string == "g") || (test_string == "u"))
      return irrep.substr(0,irrep.length()-1);
    else
      return irrep;
  }

  //----------------------------------------------------------------------------------
  //! Remove G-parity from an irrep name
  std::string removeIrrepGParity(const std::string& irrep)
  {
    std::string test_string = irrep.substr(irrep.length()-1);
    if ((test_string == "P") || (test_string == "M"))
      return irrep.substr(0,irrep.length()-1);
    else
      return irrep;
  }

  //----------------------------------------------------------------------------------
  //! Remove embedding from an irrep name
  std::string removeIrrepEmbed(const std::string& irrep)
  {
    std::size_t pos = irrep.rfind(",");
    if (pos != std::string::npos)
      return irrep.substr(0,pos);
    else
      return irrep;
  }

  //----------------------------------------------------------------------------------
  //! Get embedding from an irrep name
  int getIrrepEmbed(const std::string& irrep)
  {
    std::size_t pos = irrep.rfind(",");
    if (pos != std::string::npos)
      return atoi(irrep.substr(pos+1).c_str());
    else
      return 0;
  }

  //----------------------------------------------------------------------------------
  //! Get G-parity from an irrep name
  int getIrrepGParity(const std::string& irrep)
  {
    std::string test_string = irrep.substr(irrep.length()-1);
    if (test_string == "P")
      return 1;
    else if (test_string == "M")
      return -1;
    else
      return 0;
  }


  //----------------------------------------------------------------------------------
  //! Get little group of irrep
  std::string getIrrepLG(const std::string& long_irrep)
  {
    std::string irrep = removeHelicity(long_irrep);

    // Format of irrep label is D4A1 etc
    std::string irrepLG = "";
    std::string opLG = irrep.substr(0,2);
    std::string opLGlong = irrep.substr(0,5);
    if ( (opLG == "D2") || (opLG == "D3") || (opLG == "D4") || (opLG == "C2") )
      irrepLG = opLG;
    else if ( (opLGlong == "C4nm0") || (opLGlong == "C4nnm") )
      irrepLG = opLGlong;
    else
      irrepLG = "Oh";
    return irrepLG;
  }


  //----------------------------------------------------------------------------------
  //! Get dimension of irrep
  int getIrrepDim(const std::string& irrep)
  {
    IrrepNames_t mm = getIrrepIndex(irrep);

    return mm.dim;
  }


  //----------------------------------------------------------------------------------
  //! Get Oh/LG irrep name with no parity from possible helicity name
  std::string getCubicRepNoParity(const std::string& irrep)
  {
    IrrepNames_t mm = getIrrepIndex(removeHelicity(irrep));

    return mm.no_par;
  }



  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //! Representation including parity if it exists
  std::string buildIrrepWithParity(const std::string& irrep, int P)
  {
    IrrepNames_t mm = getIrrepIndex(irrep);

    if (mm.lg)
    {
      // In flight, so no parity
      return mm.no_par;
    }

    // Must be rest frame. Add parity depending on fermion type
    std::string rep = mm.no_par;

    if (mm.ferm)
    {
      if (P == +1)
      {
	rep += "g";
      }
      else if (P == -1)
      {
	rep += "u";
      }
    }
    else
    {
      if (P == +1)
      {
	rep += "p";
      }
      else if (P == -1)
      {
	rep += "m";
      }
    }

    return rep;
  }

  //! Representation including parity and G-parity if it exists
  std::string buildIrrepWithPG(const std::string& irrep, int P, int G)
  {
    std::string rep = buildIrrepWithParity(irrep, P);
    IrrepNames_t mm = getIrrepIndex(irrep);

    if (! mm.ferm)
    {
      // Only put in G parity if non-zero
      if (G == +1)
      {
	rep += "P";
      }
      else if (G == -1)
      {
      rep += "M";
      }
    }

    return rep;
  }

  //----------------------------------------------------------------------------------



  std::list< XMLArray::Array<int> > generateLittleGroupMom(const std::string& littleGroup0, const XMLArray::Array<int>& canon_mom0)
  {
    std::list< XMLArray::Array<int> > dest;

    std::string littleGroup = getIrrepLG(littleGroup0);
    XMLArray::Array<int> canon_mom = canonicalOrder(canon_mom0);
    int mom2 = norm2(canon_mom);

    if (generateLittleGroup(canon_mom) != littleGroup)
    {
      std::cerr << __func__ << ": little group name= " << littleGroup
		<< " inconsistent with canonical mom= " << canon_mom << "\n";
      exit(1);
    }

    // This is the inefficient, but straigthtforward way. We know the
    // max momentum. Run over all possible values, generate the LG name,
    // and select the desired values.
    for(int px= -mom2; px <= mom2; ++px)
    {
      for(int py= -mom2; py <= mom2; ++py)
      {
	for(int pz= -mom2; pz <= mom2; ++pz)
	{
	  // Check if valid
	  XMLArray::Array<int> mom(3);
	  mom[0] = px; mom[1] = py; mom[2] = pz;

	  if (norm2(mom) != mom2) {continue;}

	  XMLArray::Array<int> momCan = canonicalOrder(mom);

	  if (momCan != canon_mom) {continue;}

	  if (generateLittleGroup(momCan) != littleGroup) {continue;}

	  // Finally, a consistent momentum
	  dest.push_back(mom);
	}
      }
    }

    return dest;
  }
  std::list< std::vector<int> > generateLittleGroupMom(const std::string& littleGroup0, const std::vector<int>& canon_mom0)
  {
    std::list< XMLArray::Array<int> > tmp = generateLittleGroupMom(littleGroup0, convertArray(canon_mom0));

    std::list< std::vector<int> > dest;

    for(auto tt = tmp.begin(); tt != tmp.end(); ++tt)
    {
      dest.push_back(convertArray(*tt));
    }

    return dest;
  }
  bool allowedMomentaRotation(const XMLArray::Array<int>& mom1, const XMLArray::Array<int>& mom2, const XMLArray::Array<int>& momt,
			      const XMLArray::Array<int>& mom1p, const XMLArray::Array<int>& mom2p, const XMLArray::Array<int>& momtp)
  {
    XMLArray::Array<int> mom1_rot = mom1;
    XMLArray::Array<int> mom2_rot = mom2;
    XMLArray::Array<int> momt_rot = momt;

    std::vector<RotateVec_t> rot = generateLGRotation("all");

    for(std::vector<RotateVec_t>::const_iterator rr = rot.begin(); rr != rot.end(); ++rr)
    {
      int i0 = abs((*rr).p1) - 1;
      int s0 = isign((*rr).p1);

      int i1 = abs((*rr).p2) - 1;
      int s1 = isign((*rr).p2);

      int i2 = abs((*rr).p3) - 1;
      int s2 = isign((*rr).p3);

      mom1_rot[0] = s0*mom1[i0];
      mom1_rot[1] = s1*mom1[i1];
      mom1_rot[2] = s2*mom1[i2];

      mom2_rot[0] = s0*mom2[i0];
      mom2_rot[1] = s1*mom2[i1];
      mom2_rot[2] = s2*mom2[i2];

      momt_rot[0] = s0*momt[i0];
      momt_rot[1] = s1*momt[i1];
      momt_rot[2] = s2*momt[i2];

      if ( (mom1_rot == mom1p) && (mom2_rot == mom2p) && (momt_rot == momtp) ) return true;
    }

    return false;
  }
  int allowedMomentaCombin(const std::string& group1, const std::string& group2,
			   const XMLArray::Array<int>& orig_mom1, const XMLArray::Array<int>& orig_mom2, const XMLArray::Array<int>& orig_momt,
			   const XMLArray::Array<int>& momt, std::list< XMLArray::Array<int> >& allowed_mom1, std::list< XMLArray::Array<int> >& allowed_mom2)
  {
    int num = 0;
    allowed_mom1.clear();
    allowed_mom2.clear();

    // Check if momenta add up
    // but for backwards compatibility skip check when orig_mom1 and orig_mom2 are both momentum types
    if ( (orig_mom1 != canonicalOrder(orig_mom1)) || (orig_mom2 != canonicalOrder(orig_mom2)) )
    {
      if (orig_momt != (orig_mom1 + orig_mom2)) return 0;
    }

    std::list< XMLArray::Array<int> > mom_list1 = generateLittleGroupMom(group1,orig_mom1);
    std::list< XMLArray::Array<int> > mom_list2 = generateLittleGroupMom(group2,orig_mom2);
    std::list< XMLArray::Array<int> >::const_iterator iterator1;
    std::list< XMLArray::Array<int> >::const_iterator iterator2;
    for (iterator1 = mom_list1.begin(); iterator1 != mom_list1.end(); ++iterator1)
    {
      for (iterator2 = mom_list2.begin(); iterator2 != mom_list2.end(); ++iterator2)
      {
	if ( momt == ((*iterator1) + (*iterator2)) )
	{
	  // We have to distinguish cases like (0 -1 2)x(0 2 -1)->(0 1 1) and (2 1 0)x(-2 0 1)->(0 1 1)
	  if ( allowedMomentaRotation(orig_mom1, orig_mom2, orig_momt, (*iterator1), (*iterator2), momt) )
	  {
	    num++;
	    allowed_mom1.push_back(*iterator1);
	    allowed_mom2.push_back(*iterator2);
	  }
	  #if 1
	  // Now turned on by default -- backwards compatibility when orig_mom1 and orig_mom2 are momentum types
	  else if ( orig_momt != (orig_mom1 + orig_mom2) )
	  {
	    num++;
	    allowed_mom1.push_back(*iterator1);
	    allowed_mom2.push_back(*iterator2);
	  }
	  #endif
	}
      }
    }
    return num;
  }
  int refMomCombin(const XMLArray::Array<int>& mom1, const XMLArray::Array<int>& mom2, const XMLArray::Array<int>& momt,
		   XMLArray::Array<int>& ref_mom1, XMLArray::Array<int>& ref_mom2, XMLArray::Array<int>& ref_momt)
  {
    std::string group1 = Hadron::generateLittleGroup(mom1);
    std::string group2 = Hadron::generateLittleGroup(mom2);
    std::string groupt = Hadron::generateLittleGroup(momt);
    std::list< XMLArray::Array<int> > allowed_mom1; allowed_mom1.clear();
    std::list< XMLArray::Array<int> > allowed_mom2; allowed_mom2.clear();

    ref_momt = referenceMom(momt);
    int normalisation = allowedMomentaCombin(group1,group2,mom1,mom2,momt,ref_momt,allowed_mom1,allowed_mom2);
    if (normalisation == 0) return normalisation;

    if (groupt == "Oh")
    {
      ref_mom1 = referenceMom(mom1);
      ref_mom2 = - ref_mom1;
    }
    else
    {
      ref_mom1 = allowed_mom1.front();
      ref_mom2 = allowed_mom2.front();
    }
    return normalisation;
  }
  int refCanonMomCombin(const XMLArray::Array<int>& mom1, const XMLArray::Array<int>& mom2, const XMLArray::Array<int>& momt,
			XMLArray::Array<int>& ref_mom1, XMLArray::Array<int>& ref_mom2, XMLArray::Array<int>& canon_momt)
  {
    std::string group1 = Hadron::generateLittleGroup(mom1);
    std::string group2 = Hadron::generateLittleGroup(mom2);
    std::string groupt = Hadron::generateLittleGroup(momt);
    std::list< XMLArray::Array<int> > allowed_mom1; allowed_mom1.clear();
    std::list< XMLArray::Array<int> > allowed_mom2; allowed_mom2.clear();

    canon_momt = canonicalOrder(momt);
    int normalisation = allowedMomentaCombin(group1,group2,mom1,mom2,momt,canon_momt,allowed_mom1,allowed_mom2);
    if (normalisation == 0) return normalisation;

    if (groupt == "Oh")
    {
      ref_mom1 = referenceMom(mom1);
      ref_mom2 = - ref_mom1;
    }
    else
    {
      ref_mom1 = allowed_mom1.front();
      ref_mom2 = allowed_mom2.front();
    }
    return normalisation;
  }
  int refArbitraryMomCombin(const XMLArray::Array<int>& mom1, const XMLArray::Array<int>& mom2, const XMLArray::Array<int>& momt, const XMLArray::Array<int>& new_momt,
			    XMLArray::Array<int>& ref_mom1, XMLArray::Array<int>& ref_mom2)
  {
    std::string group1 = Hadron::generateLittleGroup(mom1);
    std::string group2 = Hadron::generateLittleGroup(mom2);
    std::string groupt = Hadron::generateLittleGroup(momt);
    std::list< XMLArray::Array<int> > allowed_mom1; allowed_mom1.clear();
    std::list< XMLArray::Array<int> > allowed_mom2; allowed_mom2.clear();

    if (canonicalOrder(momt) != canonicalOrder(new_momt))
    {
      std::cerr << __func__ << ": ERROR: momt= " << momt << " is not the same type as new_momt= " << new_momt << std::endl;
      exit(1);
    }

    int normalisation = allowedMomentaCombin(group1,group2,mom1,mom2,momt,new_momt,allowed_mom1,allowed_mom2);
    if (normalisation == 0) return normalisation;

    if (groupt == "Oh")
    {
      ref_mom1 = referenceMom(mom1);
      ref_mom2 = - ref_mom1;
    }
    else
    {
      ref_mom1 = allowed_mom1.front();
      ref_mom2 = allowed_mom2.front();
    }
    return normalisation;
  }
  std::vector<RotateVec_t> generateLGRotation(const std::string& rep)
  {
    std::vector<RotateVec_t> rot;

    if (rep == "D4")
    {
      rot.push_back(RotateVec_t(3,2,-1));
      rot.push_back(RotateVec_t(-2,3,-1));
      rot.push_back(RotateVec_t(1,2,3));
      rot.push_back(RotateVec_t(-3,-2,-1));
      rot.push_back(RotateVec_t(2,-3,-1));
      rot.push_back(RotateVec_t(-1,2,-3));
    }
    else if (rep == "D2")
    {
      rot.push_back(RotateVec_t(3,2,-1));
      rot.push_back(RotateVec_t(1,2,3));
      rot.push_back(RotateVec_t(2,-1,3));
      rot.push_back(RotateVec_t(2,-3,-1));
      rot.push_back(RotateVec_t(-1,2,-3));
      rot.push_back(RotateVec_t(-2,1,3));
      rot.push_back(RotateVec_t(-3,2,1));
      rot.push_back(RotateVec_t(-1,-2,3));
      rot.push_back(RotateVec_t(3,-1,-2));
      rot.push_back(RotateVec_t(-2,-3,1));
      rot.push_back(RotateVec_t(1,-2,-3));
      rot.push_back(RotateVec_t(-3,1,-2));
    }
    else if (rep == "D3")
    {
      rot.push_back(RotateVec_t(1,2,3));
      rot.push_back(RotateVec_t(-2,1,3));
      rot.push_back(RotateVec_t(2,-1,3));
      rot.push_back(RotateVec_t(1,3,-2));
      rot.push_back(RotateVec_t(-1,-2,3));
      rot.push_back(RotateVec_t(1,-2,-3));
      rot.push_back(RotateVec_t(-1,2,-3));
      rot.push_back(RotateVec_t(-2,-1,-3));
    }
    else if (rep == "C4nm0")
    {
      rot.push_back(RotateVec_t(1,2,3));
      rot.push_back(RotateVec_t(2,3,1));
      rot.push_back(RotateVec_t(3,1,2));
      rot.push_back(RotateVec_t(-1,3,2));
      rot.push_back(RotateVec_t(3,2,-1));
      rot.push_back(RotateVec_t(2,-1,3));
      rot.push_back(RotateVec_t(-1,2,-3));
      rot.push_back(RotateVec_t(2,-3,-1));
      rot.push_back(RotateVec_t(-3,-1,2));
      rot.push_back(RotateVec_t(1,-3,2));
      rot.push_back(RotateVec_t(-3,2,1));
      rot.push_back(RotateVec_t(2,1,-3));
      rot.push_back(RotateVec_t(-1,-2,3));
      rot.push_back(RotateVec_t(-2,3,-1));
      rot.push_back(RotateVec_t(3,-1,-2));
      rot.push_back(RotateVec_t(1,3,-2));
      rot.push_back(RotateVec_t(3,-2,1));
      rot.push_back(RotateVec_t(-2,1,3));
      rot.push_back(RotateVec_t(1,-2,-3));
      rot.push_back(RotateVec_t(-2,-3,1));
      rot.push_back(RotateVec_t(-3,1,-2));
      rot.push_back(RotateVec_t(-1,-3,-2));
      rot.push_back(RotateVec_t(-3,-2,-1));
      rot.push_back(RotateVec_t(-2,-1,-3));
    }
    else if (rep == "C4nnm")
    {
      rot.push_back(RotateVec_t(1,2,3));
      rot.push_back(RotateVec_t(2,3,1));
      rot.push_back(RotateVec_t(3,1,2));
      rot.push_back(RotateVec_t(-2,1,3));
      rot.push_back(RotateVec_t(-1,3,2));
      rot.push_back(RotateVec_t(2,-1,3));
      rot.push_back(RotateVec_t(1,3,-2));
      rot.push_back(RotateVec_t(3,-2,1));
      rot.push_back(RotateVec_t(3,2,-1));
      rot.push_back(RotateVec_t(2,1,-3));
      rot.push_back(RotateVec_t(1,-3,2));
      rot.push_back(RotateVec_t(-3,2,1));
      rot.push_back(RotateVec_t(-1,-2,3));
      rot.push_back(RotateVec_t(-2,3,-1));
      rot.push_back(RotateVec_t(3,-1,-2));
      rot.push_back(RotateVec_t(-1,2,-3));
      rot.push_back(RotateVec_t(-2,-3,1));
      rot.push_back(RotateVec_t(1,-2,-3));
      rot.push_back(RotateVec_t(2,-3,-1));
      rot.push_back(RotateVec_t(-3,-1,2));
      rot.push_back(RotateVec_t(-3,1,-2));
      rot.push_back(RotateVec_t(-2,-1,-3));
      rot.push_back(RotateVec_t(-1,-3,-2));
      rot.push_back(RotateVec_t(-3,-2,-1));
    }
    else if (rep == "all")
    {
      rot.push_back(RotateVec_t(1,2,3));
      rot.push_back(RotateVec_t(2,3,1));
      rot.push_back(RotateVec_t(3,1,2));
      rot.push_back(RotateVec_t(3,2,1));
      rot.push_back(RotateVec_t(2,1,3));
      rot.push_back(RotateVec_t(1,3,2));

      rot.push_back(RotateVec_t(-1,2,3));
      rot.push_back(RotateVec_t(-2,3,1));
      rot.push_back(RotateVec_t(-3,1,2));
      rot.push_back(RotateVec_t(-3,2,1));
      rot.push_back(RotateVec_t(-2,1,3));
      rot.push_back(RotateVec_t(-1,3,2));

      rot.push_back(RotateVec_t(1,-2,3));
      rot.push_back(RotateVec_t(2,-3,1));
      rot.push_back(RotateVec_t(3,-1,2));
      rot.push_back(RotateVec_t(3,-2,1));
      rot.push_back(RotateVec_t(2,-1,3));
      rot.push_back(RotateVec_t(1,-3,2));

      rot.push_back(RotateVec_t(1,2,-3));
      rot.push_back(RotateVec_t(2,3,-1));
      rot.push_back(RotateVec_t(3,1,-2));
      rot.push_back(RotateVec_t(3,2,-1));
      rot.push_back(RotateVec_t(2,1,-3));
      rot.push_back(RotateVec_t(1,3,-2));

      rot.push_back(RotateVec_t(-1,-2,3));
      rot.push_back(RotateVec_t(-2,-3,1));
      rot.push_back(RotateVec_t(-3,-1,2));
      rot.push_back(RotateVec_t(-3,-2,1));
      rot.push_back(RotateVec_t(-2,-1,3));
      rot.push_back(RotateVec_t(-1,-3,2));

      rot.push_back(RotateVec_t(-1,2,-3));
      rot.push_back(RotateVec_t(-2,3,-1));
      rot.push_back(RotateVec_t(-3,1,-2));
      rot.push_back(RotateVec_t(-3,2,-1));
      rot.push_back(RotateVec_t(-2,1,-3));
      rot.push_back(RotateVec_t(-1,3,-2));

      rot.push_back(RotateVec_t(1,-2,-3));
      rot.push_back(RotateVec_t(2,-3,-1));
      rot.push_back(RotateVec_t(3,-1,-2));
      rot.push_back(RotateVec_t(3,-2,-1));
      rot.push_back(RotateVec_t(2,-1,-3));
      rot.push_back(RotateVec_t(1,-3,-2));

      rot.push_back(RotateVec_t(-1,-2,-3));
      rot.push_back(RotateVec_t(-2,-3,-1));
      rot.push_back(RotateVec_t(-3,-1,-2));
      rot.push_back(RotateVec_t(-3,-2,-1));
      rot.push_back(RotateVec_t(-2,-1,-3));
      rot.push_back(RotateVec_t(-1,-3,-2));
    }
    else
    {
      std::cerr << __func__ << ": unsupported LG= " << rep << "\n";
      exit(1);
    }

    return rot;
  }
  int isign(int x)
  {
    return (x > 0) ? +1 : -1;
  }
  XMLArray::Array<int> rotateMom(const XMLArray::Array<int> mom, const RotateVec_t& rot)
  {
    XMLArray::Array<int> mom2;
    mom2.resize(3);

    int i0 = abs(rot.p1) - 1;
    int s0 = isign(rot.p1);

    int i1 = abs(rot.p2) - 1;
    int s1 = isign(rot.p2);

    int i2 = abs(rot.p3) - 1;
    int s2 = isign(rot.p3);

    mom2[0] = s0*mom[i0];
    mom2[1] = s1*mom[i1];
    mom2[2] = s2*mom[i2];

    return mom2;
  }
  RotateVec_t findRotateVec(const XMLArray::Array<int> mom1, const XMLArray::Array<int> mom2)
  {
    if (canonicalOrder(mom1) != canonicalOrder(mom2))
    {
      std::cerr << __func__ << ": ERROR: mom1= " << mom1 << " is not the same type as mom2= " << mom2 << std::endl;
      exit(1);
    }

    std::vector<RotateVec_t> rot = generateLGRotation("all");
    for(std::vector<RotateVec_t>::const_iterator rr = rot.begin(); rr != rot.end(); ++rr)
    {
      if (rotateMom(mom1, *rr) == mom2) return *rr;
    }

    std::cerr << __func__ << ": ERROR: can't find lattice rotation that rotates mom1= " << mom1 << " to mom2= " << mom2 << std::endl;
    exit(1);

  }
}
