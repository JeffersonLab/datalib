/*! \file
 * \brief Generate graphs
 */

#include <datalib/graph/naming.h>
#include <datalib/graph/operator_name.h>
#include <map>

namespace Hadron
{
  using XMLArray::Array;

  // Utility functions
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

  } // anonymous namespace

  //----------------------------------------------------------------------------------
  //! Get irrep name from operator name
  std::string getIrrep(const std::string& opName)
  {
    std::string opIrrep = "";

    int sub_index1 = opName.rfind("_");
    if (sub_index1 == std::string::npos)
    {
      std::cerr << __func__ << ": ERROR: can't extract irrep from operator name " << opName << std::endl;
      exit(1);
    }
    opIrrep = opName.substr(sub_index1+1,std::string::npos);

    // Remove the helicity label
    opIrrep = removeHelicity(opIrrep);

    // Perform check on irrep name (not necessary?)
    if (getIrrepDim(opIrrep) < 1)
    {
      std::cerr << __func__ << ": ERROR: unknown irrep " << opIrrep << " from operator " << opName << std::endl;
      exit(1);
    }

    return opIrrep;
  }


  //----------------------------------------------------------------------------------
  //! Extract continuum op name from subduced op name
  std::string extractContOpName(const std::string& opName)
  {
    int sub_index = opName.find_last_of("_");
    if (sub_index == std::string::npos)
    {
      std::cerr << __func__ << ": ERROR: can't extract continuum helicity operator name from subduced operator " << opName << std::endl;
      exit(1);
    }

    // Do we want to check that we're actually removing a subduced irrep name here?
    //
    //	  std::string subd_rep = opName.substr(sub_index);
    //    if( (subd_rep != "_A1") && (subd_rep != "_A2") ..... )
    //    {
    //	  std::cerr << __func__ << ": ERROR: can't extract continuum operator name from subduced operator " << opName << std::endl;
    //	  exit(1);
    //	  }

    std::string cont_name = opName.substr(0,sub_index);
    return cont_name;
  }


  //----------------------------------------------------------------------------------
  //! Hidden storage for operator name parsing
  namespace
  {
    // Particle ID stuff
    struct QuantumNum_t
    {
      QuantumNum_t() : had(0), flavor(), N(0), F(), twoI(0), S(0), P(0), G(0), rescale(0.0) {}
      QuantumNum_t(int had_, const std::string& flav_, int N_, const std::string& F_, int I_, int S_, int P_, int G_, double f_) :
	had(had_), flavor(flav_), N(N_), F(F_), twoI(I_), S(S_), P(P_), G(G_), rescale(f_) {}

      int          had;    /*!< 1->meson (no parity in op name), 2->baryon (parity is in op name) */
      std::string  flavor; /*!< Flavor */
      int          N;      /*!< The N in SU(N) */
      std::string  F;      /*!< Flavor irrep */
      int          twoI;   /*!< 2*Isospin */
      int          S;      /*!< Strangeness (s quark has strangeness -1) */
      int          P;      /*!< Rest-frame parity */
      int          G;      /*!< G-parity (if it exists) */

      double       rescale;   /*!< Rescaling factor to bring old redstar wave-func norm. into new norm. No wick factors here. */
    };

    // Initialized
    bool initP(false);

    // Flavor -> isospin, strangeness, parity, charge-conj.
    typedef std::map<std::string, QuantumNum_t> Class_t;
    Class_t classnames;

    Class_t proj_op_classes; // keep a copy, and rescale if proj op and isospin=1

    //---------------------------------------------------------------------------
    // Initialize classnames
    void initProjOpRescale()
    {
      for(Class_t::const_iterator p = classnames.begin(); p != classnames.end(); ++p)
      {
	// Copy the previous class
	QuantumNum_t q = p->second;

	// Will rescale all projected ops by sqrt(2) - except isospin=1
	if (p->second.N == 2 && p->second.F != "3")
	{
	  q.rescale = sqrt(2.0);
	}
	else
	{
	  q.rescale = 1.0;
	}

	// It will be for the proj op
	proj_op_classes.insert(std::make_pair(p->first + "proj", q));
      }
    }

    //---------------------------------------------------------------------------
    // Initialize classnames
    void initQuantumNum()
    {
      if (initP) {return;}

      // Flavor -> quantum-numbers
      classnames.insert(std::make_pair("eta_",QuantumNum_t(1,"eta",2,"1",0,0,-1,+1,1.0)));
      classnames.insert(std::make_pair("f_",QuantumNum_t(1,"f",2,"1",0,0,+1,+1,1.0)));
      classnames.insert(std::make_pair("h_",QuantumNum_t(1,"h",2,"1",0,0,+1,-1,1.0)));
      classnames.insert(std::make_pair("omega_",QuantumNum_t(1,"omega",2,"1",0,0,-1,-1,1.0)));
      classnames.insert(std::make_pair("phi_",QuantumNum_t(1,"phi",2,"1",0,0,-1,-1,1.0)));

      classnames.insert(std::make_pair("eta1_",QuantumNum_t(1,"eta",3,"1",0,0,-1,+1,-1.0)));
      classnames.insert(std::make_pair("f1_",QuantumNum_t(1,"f",3,"1",0,0,-1,+1,-1.0)));
      classnames.insert(std::make_pair("h1_",QuantumNum_t(1,"h",3,"1",0,0,-1,+1,-1.0)));
      classnames.insert(std::make_pair("omega1_",QuantumNum_t(1,"omega",3,"1",0,0,-1,+1,-1.0)));
      classnames.insert(std::make_pair("phi1_",QuantumNum_t(1,"phi",3,"1",0,0,-1,+1,-1.0)));

      classnames.insert(std::make_pair("eta8_",QuantumNum_t(1,"eta",3,"8",0,0,-1,+1,+1.0)));
      classnames.insert(std::make_pair("f8_",QuantumNum_t(1,"f",3,"8",0,0,-1,+1,+1.0)));
      classnames.insert(std::make_pair("h8_",QuantumNum_t(1,"h",3,"8",0,0,-1,+1,+1.0)));
      classnames.insert(std::make_pair("omega8_",QuantumNum_t(1,"omega",3,"8",0,0,-1,+1,+1.0)));
      classnames.insert(std::make_pair("phi8_",QuantumNum_t(1,"phi",3,"8",0,0,-1,+1,+1.0)));

      classnames.insert(std::make_pair("fl_",QuantumNum_t(1,"f",2,"1",0,0,+1,+1,1.0)));
      classnames.insert(std::make_pair("fs_",QuantumNum_t(1,"f",2,"1",0,0,+1,+1,1.0)));
      classnames.insert(std::make_pair("chic_",QuantumNum_t(1,"chi",2,"1",0,0,+1,+1,1.0)));
      classnames.insert(std::make_pair("chice_",QuantumNum_t(1,"chi",2,"1",0,0,+1,+1,1.0)));
      classnames.insert(std::make_pair("hl_",QuantumNum_t(1,"h",2,"1",0,0,+1,-1,1.0)));
      classnames.insert(std::make_pair("hs_",QuantumNum_t(1,"h",2,"1",0,0,+1,-1,1.0)));
      classnames.insert(std::make_pair("hc_",QuantumNum_t(1,"h",2,"1",0,0,+1,-1,1.0)));
      classnames.insert(std::make_pair("hce_",QuantumNum_t(1,"h",2,"1",0,0,+1,-1,1.0)));
      classnames.insert(std::make_pair("etal_",QuantumNum_t(1,"eta",2,"1",0,0,-1,+1,1.0)));
      classnames.insert(std::make_pair("etas_",QuantumNum_t(1,"eta",2,"1",0,0,-1,+1,1.0)));
      classnames.insert(std::make_pair("etac_",QuantumNum_t(1,"eta",2,"1",0,0,-1,+1,1.0)));
      classnames.insert(std::make_pair("etace_",QuantumNum_t(1,"eta",2,"1",0,0,-1,+1,1.0)));
      classnames.insert(std::make_pair("omegal_",QuantumNum_t(1,"omega",2,"1",0,0,-1,-1,1.0)));
      classnames.insert(std::make_pair("omegas_",QuantumNum_t(1,"omega",2,"1",0,0,-1,-1,1.0)));
      classnames.insert(std::make_pair("psi_",QuantumNum_t(1,"omega",2,"1",0,0,-1,-1,1.0)));
      classnames.insert(std::make_pair("psice_",QuantumNum_t(1,"omega",2,"1",0,0,-1,-1,1.0)));

      classnames.insert(std::make_pair("ubaruneg_",QuantumNum_t(1,"ubaru",2,"0",0,0,-1,-1,0.0)));
      classnames.insert(std::make_pair("dbardneg_",QuantumNum_t(1,"dbard",2,"0",0,0,-1,-1,0.0)));
      classnames.insert(std::make_pair("ubarupos_",QuantumNum_t(1,"ubaru",2,"0",0,0,+1,-1,0.0)));
      classnames.insert(std::make_pair("dbardpos_",QuantumNum_t(1,"dbard",2,"0",0,0,+1,-1,0.0)));

      classnames.insert(std::make_pair("Kpos_",QuantumNum_t(1,"Kpos",2,"2",1,+1,+1,0,-1.0)));
      classnames.insert(std::make_pair("Kneg_",QuantumNum_t(1,"Kneg",2,"2",1,+1,-1,0,-1.0)));
      classnames.insert(std::make_pair("Kbarpos_",QuantumNum_t(1,"Kbarpos",2,"2",1,-1,+1,0,1.0)));
      classnames.insert(std::make_pair("Kbarneg_",QuantumNum_t(1,"Kbarneg",2,"2",1,-1,-1,0,1.0)));

      classnames.insert(std::make_pair("Dpos_",QuantumNum_t(1,"Dpos",2,"2",1,0,+1,0,1.0)));
      classnames.insert(std::make_pair("Dneg_",QuantumNum_t(1,"Dneg",2,"2",1,0,-1,0,1.0)));
      classnames.insert(std::make_pair("Dbarpos_",QuantumNum_t(1,"Dbarpos",2,"2",1,0,+1,0,1.0)));
      classnames.insert(std::make_pair("Dbarneg_",QuantumNum_t(1,"Dbarneg",2,"2",1,0,-1,0,1.0)));

      classnames.insert(std::make_pair("Dspos_",QuantumNum_t(1,"Dspos",2,"1",0,+1,+1,0,-1.0)));
      classnames.insert(std::make_pair("Dsneg_",QuantumNum_t(1,"Dsneg",2,"1",0,+1,-1,0,-1.0)));
      classnames.insert(std::make_pair("Dsbarpos_",QuantumNum_t(1,"Dsbarpos",2,"1",0,-1,+1,0,1.0)));
      classnames.insert(std::make_pair("Dsbarneg_",QuantumNum_t(1,"Dsbarneg",2,"1",0,-1,-1,0,1.0)));

      classnames.insert(std::make_pair("Epos_",QuantumNum_t(1,"Epos",2,"2",0,0,+1,0,1.0)));
      classnames.insert(std::make_pair("Eneg_",QuantumNum_t(1,"Eneg",2,"2",0,0,-1,0,1.0)));
      classnames.insert(std::make_pair("Ebarpos_",QuantumNum_t(1,"Ebarpos",2,"2",0,0,+1,0,1.0)));
      classnames.insert(std::make_pair("Ebarneg_",QuantumNum_t(1,"Ebarneg",2,"2",0,0,-1,0,1.0)));

      classnames.insert(std::make_pair("Espos_",QuantumNum_t(1,"Espos",2,"1",0,+1,+1,0,-1.0)));
      classnames.insert(std::make_pair("Esneg_",QuantumNum_t(1,"Esneg",2,"1",0,+1,-1,0,-1.0)));
      classnames.insert(std::make_pair("Esbarpos_",QuantumNum_t(1,"Esbarpos",2,"1",0,-1,+1,0,1.0)));
      classnames.insert(std::make_pair("Esbarneg_",QuantumNum_t(1,"Esbarneg",2,"1",0,-1,-1,0,1.0)));

      classnames.insert(std::make_pair("a_",QuantumNum_t(1,"a",2,"3",2,0,+1,-1,sqrt(2.0))));
      classnames.insert(std::make_pair("b_",QuantumNum_t(1,"b",2,"3",2,0,+1,+1,sqrt(2.0))));
      classnames.insert(std::make_pair("pion_",QuantumNum_t(1,"pion",2,"3",2,0,-1,-1,sqrt(2.0))));
      classnames.insert(std::make_pair("rho_",QuantumNum_t(1,"rho",2,"3",2,0,-1,+1,sqrt(2.0))));

      classnames.insert(std::make_pair("gluepP_",QuantumNum_t(3,"gluepP",3,"1",0,0,+1,+1,1.0)));
      classnames.insert(std::make_pair("gluepM_",QuantumNum_t(3,"gluepM",3,"1",0,0,+1,-1,1.0)));
      classnames.insert(std::make_pair("gluemP_",QuantumNum_t(3,"gluemP",3,"1",0,0,-1,+1,1.0)));
      classnames.insert(std::make_pair("gluemM_",QuantumNum_t(3,"gluemM",3,"3",0,0,-1,-1,1.0)));

      classnames.insert(std::make_pair("Nucleon",QuantumNum_t(2,"Nucleon",2,"2",1,0,+1,0,1.0)));
      classnames.insert(std::make_pair("Delta",QuantumNum_t(2,"Delta",2,"4",3,0,+1,0,1.0)));
      classnames.insert(std::make_pair("Lambda1",QuantumNum_t(2,"Lambda",2,"1",0,-1,+1,0,1.0)));
      classnames.insert(std::make_pair("Lambda8",QuantumNum_t(2,"Lambda",2,"1",0,-1,+1,0,1.0)));
      classnames.insert(std::make_pair("Sigma8",QuantumNum_t(2,"Sigma",2,"3",2,-1,+1,0,1.0)));
      classnames.insert(std::make_pair("Sigma10",QuantumNum_t(2,"Sigma",2,"3",2,-1,+1,0,1.0)));
      classnames.insert(std::make_pair("Xi8",QuantumNum_t(2,"Xi",2,"2",1,-2,+1,0,1.0)));
      classnames.insert(std::make_pair("Xi10",QuantumNum_t(2,"Xi",2,"2",1,-2,+1,0,1.0)));
      classnames.insert(std::make_pair("Omega",QuantumNum_t(2,"Omega",2,"1",0,0,+1,0,1.0)));

      classnames.insert(std::make_pair("LambdacA",QuantumNum_t(2,"LambdacA",2,"1",0,0,+1,0,1.0)));
      classnames.insert(std::make_pair("LambdacM",QuantumNum_t(2,"LambdacM",2,"1",0,0,+1,0,1.0)));
      classnames.insert(std::make_pair("SigmacM",QuantumNum_t(2,"SigmacM",2,"3",2,0,+1,0,1.0)));
      classnames.insert(std::make_pair("SigmacS",QuantumNum_t(2,"SigmacS",2,"3",2,0,+1,0,1.0)));
      classnames.insert(std::make_pair("XicS",QuantumNum_t(2,"XicS",2,"2",1,-1,+1,0,1.0)));
      classnames.insert(std::make_pair("XipcM",QuantumNum_t(2,"XipcM",2,"2",1,-1,+1,0,1.0)));
      classnames.insert(std::make_pair("XicM",QuantumNum_t(2,"XicM",2,"2",1,-1,+1,0,1.0)));

      classnames.insert(std::make_pair("XiccS",QuantumNum_t(2,"XiccS",2,"2",1,0,+1,0,1.0)));
      classnames.insert(std::make_pair("XiccM",QuantumNum_t(2,"XiccM",2,"2",1,0,+1,0,1.0)));

      classnames.insert(std::make_pair("OmegacS",QuantumNum_t(2,"OmegacS",2,"1",0,0,+1,0,1.0)));
      classnames.insert(std::make_pair("OmegaccS",QuantumNum_t(2,"OmegaccS",2,"1",0,0,+1,0,1.0)));
      classnames.insert(std::make_pair("OmegaccS",QuantumNum_t(2,"OmegaccS",2,"1",0,0,+1,0,1.0)));
      classnames.insert(std::make_pair("OmegacM",QuantumNum_t(2,"OmegacM",2,"1",0,0,+1,0,1.0)));
      classnames.insert(std::make_pair("OmegaccM",QuantumNum_t(2,"OmegaccM",2,"1",0,0,+1,0,1.0)));
      classnames.insert(std::make_pair("OmegaccM",QuantumNum_t(2,"OmegaccM",2,"1",0,0,+1,0,1.0)));

      // SU(2)_F tetraquarks
      if (1)
      {
	for(int twoI = 0; twoI <= 4; ++twoI)
	  for(int S=-2; S <= +2; ++S)
	    for(int P=-1; P <= +1; P+=2)
	      for(int G=-1; G <= +1; ++G)
	      {
		std::string F;
		if (1)
		{
		  std::ostringstream os;
		  os << (twoI+1);
		  F = os.str();
		}

		std::ostringstream os;
		os << "tetra2I" << twoI << "S";

		if (S < 0)
		  os << "m" << -S;
		else if (S == 0)
		  os << S;
		else
		  os << "p" << S;

		if (P < 0)
		  os << "m";
		else
		  os << "p";

		if (G < 0)
		  os << "M";
		else if (G > 0)
		  os << "P";

		classnames.insert(std::make_pair(os.str() + "_",QuantumNum_t(4,os.str(),2,F,twoI,S,P,G,1.0)));
	      }
      }


      // SU(3)_F tetraquarks
      if (1)
      {
	std::list<std::string> t3;
	t3.push_back("1"); t3.push_back("8"), t3.push_back("10"), t3.push_back("27");

	for(auto tt = t3.begin(); tt != t3.end(); ++tt)
	  for(int P=-1; P <= +1; P+=2)
	    for(int G=-1; G <= +1; ++G)
	    {
	      std::ostringstream os;
	      os << "tetraF" << *tt;

	      if (P < 0)
		os << "m";
	      else
		os << "p";

	      if (G < 0)
		os << "M";
	      else if (G > 0)
		os << "P";

	      classnames.insert(std::make_pair(os.str() + "_",QuantumNum_t(4,os.str(),3,*tt,0,0,P,G,1.0)));
	    }
      }

      initP = true;

      // Initialize projected op rescaling
      initProjOpRescale();
    }


    //----------------------------------------------------------------------------------
    // Get flavor of particle id
    Class_t::const_iterator getFlavorIndex(const std::string& opName)
    {
      initQuantumNum();

      for(Class_t::const_iterator mm = classnames.begin();
	  mm != classnames.end();
	  ++mm)
      {
	if (opName.compare(0,mm->first.size(),mm->first) == 0)
	{
	  return mm;
	}
      }

      std::cerr << __func__ << ": ERROR: cannot extract flavor from operator name " << opName << std::endl;
      exit(1);

      return classnames.end();
    }


  } // anonymous namespace


  //----------------------------------------------------------------------------------
  // Get the N in SU(N)
  int getSUNF(const std::string& opName)
  {
    Class_t::const_iterator mm = getFlavorIndex(opName);
    if (mm == classnames.end())
    {
      std::cerr << __func__ << ": no known SU(N): opName= " << opName << "\n";
      exit(1);
    }

    return mm->second.N;
  }


  //----------------------------------------------------------------------------------
  // Get flavor irrep
  std::string getFlavorIrrep(const std::string& opName)
  {
    Class_t::const_iterator mm = getFlavorIndex(opName);
    if (mm == classnames.end())
    {
      std::cerr << __func__ << ": no known flavor: opName= " << opName << "\n";
      exit(1);
    }

    return mm->second.F;
  }


  //----------------------------------------------------------------------------------
  // Get flavor of particle id
  std::string getFlavor(const std::string& opName)
  {
    Class_t::const_iterator mm = getFlavorIndex(opName);
    if (mm == classnames.end())
    {
      std::cerr << __func__ << ": no known flavor: opName= " << opName << "\n";
      exit(1);
    }

    return mm->second.flavor;
  }


  //----------------------------------------------------------------------------------
  // Get number of quarks
  int getNumQuarks(const std::string& opName)
  {
    Class_t::const_iterator mm = getFlavorIndex(opName);
    if (mm == classnames.end())
    {
      std::cerr << __func__ << ": no known flavor: opName= " << opName << "\n";
      exit(1);
    }

    int had = mm->second.had;
    int N = -1;

    if (had == 1)
      return 2;
    else if (had == 2)
      return 3;
    else if (had == 3)
      return 2;
    else
    {
      std::cout << __func__ << ": unknown hadron type= " << had << std::endl;
      exit(1);
    }

    return -1;
  }


  //----------------------------------------------------------------------------------
  // Get flavor of particle id
  int getTwoIsospin(const std::string& opName)
  {
    Class_t::const_iterator mm = getFlavorIndex(opName);
    if (mm == classnames.end())
    {
      std::cerr << __func__ << ": no known flavor: opName= " << opName << "\n";
      exit(1);
    }

    return mm->second.twoI;
  }


  //----------------------------------------------------------------------------------
  // Get threeY of id
  int getThreeY(const std::string& opName)
  {
    Class_t::const_iterator mm = getFlavorIndex(opName);
    if (mm == classnames.end())
    {
      std::cerr << __func__ << ": no known flavor: opName= " << opName << "\n";
      exit(1);
    }

    // Y = S + B
    int threeY = mm->second.S;
    if (mm->second.had == 2)
      ++threeY; // baryon

    return 3*threeY;
  }


  //----------------------------------------------------------------------------------
  // Get strangeness of particle id
  int getStrangeness(const std::string& opName)
  {
    Class_t::const_iterator mm = getFlavorIndex(opName);
    if (mm == classnames.end())
    {
      std::cerr << __func__ << ": no known flavor: opName= " << opName << "\n";
      exit(1);
    }

    return mm->second.S;
  }


  //----------------------------------------------------------------------------------
  // Get rescaling from old normalization
  double getRescalingFactor(const std::string& opName)
  {
    Class_t::const_iterator mm = getFlavorIndex(opName);
    if (mm == classnames.end())
    {
      std::cerr << __func__ << ": no known flavor: opName= " << opName << "\n";
      exit(1);
    }

    double f = mm->second.rescale;

    if (f == 0.0)
    {
      std::cout << __func__ << ": no rescaling exists for this op = " << opName << std::endl;
      exit(1);
    }

    return f;
  }


  //----------------------------------------------------------------------------------
  // Get rescaling for projected ops
  double getProjOpRescalingFactor(const std::string& opName)
  {
    if (getFlavorIndex(opName) == classnames.end())
    {
      std::cerr << __func__ << ": no known flavor: opName= " << opName << "\n";
      exit(1);
    }

    double rescale = 1.0;

    for(Class_t::const_iterator mm = proj_op_classes.begin();
	mm != proj_op_classes.end();
	++mm)
    {
      if (opName.compare(0,mm->first.size(),mm->first) == 0)
      {
	rescale = mm->second.rescale;
      }
    }

    return rescale;
  }


  //----------------------------------------------------------------------------------
  // Get irrep with parity
  std::string getIrrepNoParity(const std::string& opName)
  {
    return getCubicRepNoParity(getIrrep(opName));
  }

  //----------------------------------------------------------------------------------
  // Get irrep with parity
  std::string getIrrepWithParity(const std::string& opName)
  {
    std::string irrep = getCubicRepNoParity(getIrrep(opName));

    Class_t::const_iterator mm = getFlavorIndex(opName);
    if (mm == classnames.end())
    {
      std::cerr << __func__ << ": no known SU(N): opName= " << opName << "\n";
      exit(1);
    }

    return buildIrrepWithParity(irrep, mm->second.P);
  }

} // namespace Hadron
