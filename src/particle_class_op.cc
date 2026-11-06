/*! \file
 * \brief Hadron-like particle operators
 */

#include "datalib/graph/particle_class_op.h"


namespace Hadron
{
  namespace
  {
    //! Error output
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

    //! Error output
    std::ostream& operator<<(std::ostream& os, const std::vector<int>& d)
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
    std::ostream& operator<<(std::ostream& os, const std::vector<Array<int>>& d)
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


  //----------------------------------------------------------------------------------
  //! Enforce referenceMom order
  KeyParticleClassOp_t referenceOrder(const KeyParticleClassOp_t& d0)
  {
    KeyParticleClassOp_t d(d0);
    d.mom_class = referenceMom(d0.mom_class);

    return d;
  }


  //----------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeyParticleClassOp_t& param)
  {
    os << " name= " << param.name;
    if (param.smear.size() > 0)
      os << " smear= " << param.smear;
    os << " momclass= " << param.mom_class;

    if (param.disp_list.size() > 0)
      os << " disp= " << param.disp_list;

    if (param.phasings.size() > 0)
      os << " phasings= " << param.phasings;

    return os;
  }

  //----------------------------------------------------------------------------
  //! Used for output
  std::ostream& operator<<(std::ostream& os, const Array1dO<KeyParticleClassOp_t>& param)
  {
    for(int i=1; i <= param.size(); ++i)
    {
      os << " Op" << i << ":" << param[i];
    }

    return os;
  }

  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyParticleClassOp_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "name", param.name);
    if (paramtop.count("smear") > 0)
      read(paramtop, "smear", param.smear);
    Array<int> mom_class;
    read(paramtop, "mom_class", mom_class);
    
    param.mom_class = mom_class;

    if (paramtop.count("disp_list") > 0)
      read(paramtop, "disp_list", param.disp_list);

    if (paramtop.count("phasings") > 0)
      read(paramtop, "phasings", param.phasings);
  }

  // Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyParticleClassOp_t& param)
  {
    push(xml, path);

    write(xml, "name", param.name);
    if (param.smear.size() > 0)
      write(xml, "smear", param.smear);
    write(xml, "mom_class", param.mom_class);

    if (param.disp_list.size() > 0)
      write(xml, "disp_list", param.disp_list);

    if (param.phasings.size() > 0)
      write(xml, "phasings", param.phasings);

    pop(xml);
  }

} // namespace Hadron


