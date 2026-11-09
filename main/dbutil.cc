/*! \file
 * \brief Generic operations on db file
 */
#include <datalib/codec/binary.h>
#include <datalib/model/arrays.h>


#include <datalib/storage/dbtype.h>
#include "mod_reader.h"
#include <datalib/graph/filenames.h>
#include <datalib/io/serial_xml.h>
#include <datalib/superb/genprop4.h>
#include <datalib/superb/baryon_superb.h>
#include <datalib/superb/meson_superb.h>
#include <datalib/superb/disco_superb.h>
#include <datalib/superb/prop_superb.h>
#include <datalib/storage/filedb.h>
#include <datalib/storage/corr_reader.h>

#include "DBString.h" // for UserData

#include <datalib/legacy/hadron_1pt_corr.h>
#include <datalib/legacy/hadron_2pt_corr.h>
#include <datalib/legacy/hadron_3pt_corr.h>

#include <datalib/superb/baryon_elem_type.h>

#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>
#include <datalib/graph/hadron_timeslice.h>
#include <datalib/superb/meson_elem_type.h>
#include <datalib/superb/prop_elem_type.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/superb/unsmeared_genprop4_elem.h>
#include <datalib/superb/unsmeared_meson_elem.h>
#include <datalib/superb/unsmeared_twoquark_disco.h>
#include <datalib/superb/tetraquark_elem_type.h>
#include <datalib/graph/hadron_class_npart_npt_conn_graph.h>

#include <array>
#include <cmath>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <map>


using namespace FILEDB;
using namespace FF;
using namespace Hadron;

// Four spin components are part of the stored Dirac tensor convention.
constexpr int Ns = 4;

int usage(int argc, char** argv)
{
  std::cerr << "Usage:" << std::endl							 //
	    << "MOD files support meta, keys, keysxml (eigenvectors and distillation props)." << std::endl
	    << "- show meta:" << std::endl						 //
	    << "   " << argv[0] << " <dbasename> meta" << std::endl			 //
	    << "- print keys:" << std::endl						 //
	    << "   " << argv[0] << " <dbasename> keys" << std::endl			 //
	    << "- print keys in xml format:" << std::endl				 //
	    << "   " << argv[0] << " <dbasename> keysxml <output_file_xml>" << std::endl //
	    << "- get the elementals in a simple binary format:" << std::endl		 //
	    << "   " << argv[0] << " <dbasename> get <keysxml_file>" << std::endl	 //
	    << "- compare the elementals from two files:" << std::endl			 //
	    << "   " << argv[0] << " <dbasename0> ... compare <dbasename1> ... [-tol tol [itol]]"
	    << std::endl
	    << "- compare the g5-Hermiticity of props:" << std::endl //
	    << "   " << argv[0] << " <dbasename0> ... g5herm <dbasename1> ... [-tol tol]"
	    << std::endl
	    << "- return pion mass (Frob norm) of props:" << std::endl //
	    << "   " << argv[0] << " <dbasename0> ... pioncheck <dbasename1> ..." << std::endl
	    << "- rewrite the mass label information:" << std::endl //
	    << "   " << argv[0] << " <dbasename> relabel_mass <new_label> <dbasename_out>"
	    << "- check the integrity of the file:" << std::endl		       //
	    << "   " << argv[0] << " <dbasename> check" << std::endl		       //
	    << "- convert to a newer format:" << std::endl			       //
	    << "   " << argv[0] << " <dbasename> convert <dbasename_out>" << std::endl //
	    << std::endl
	    << std::endl;
  return -1;
}

//! Print the metadata
void printMeta(const std::string& dbase)
{
  std::cout << __func__ << ": metadata:\n" << getMetaData(dbase) << std::endl;
}

//! Print all the keys
template <typename K>
void printCorrKeys(const std::string& dbase)
{
  // Open DB
  Datalib::Storage::StorageCorrFunc<K> database;
  database.open_for_reading(dbase);

  for (const auto& key: database.keys())
  {
    std::cout << key << "\n";
  }
}

//! Print all the keys to a xml file
template <typename K>
void printCorrKeysXML(const std::string& xml_file, const std::string& dbase)
{
  // Open DB
  Datalib::Storage::StorageCorrFunc<K> database;
  database.open_for_reading(dbase);

  // Print keys
  Datalib::Serial::XMLFileWriter xml_out(xml_file);
  push(xml_out, "Keys");
  for (const auto& key: database.keys())
  {
    write(xml_out, "elem", key);
  }
  pop(xml_out);

  xml_out.close();
}

//! Get a key/value
template <typename K>
std::vector<std::vector<std::complex<double>>> get_value(const K& key, Datalib::Storage::StorageCorrFunc<K>& database)
{
  std::vector<std::vector<std::complex<double>>> val;
  if (database.get(key, val) != 0)
  {
    std::stringstream ss;
    ss << "key not found: " << key;
    throw std::runtime_error(ss.str());
  }

  return val;
}

//! Print ensemble files
template <typename K>
void printEnsem(const std::string& xml_key_file, const std::string& dbase)
{
  // Open DB
  Datalib::Storage::StorageCorrFunc<K> database;
  database.open_for_reading(dbase);

  Datalib::Serial::XMLReader xml_in(xml_key_file);
  XMLArray::Array<K> keys;
  read(xml_in, "/Keys", keys);

  // Write ensemble file
  for (int i = 0; i < keys.size(); ++i)
  {
    const auto& v = get_value(keys[i], database);
    std::size_t num_configs = v.size();
    std::size_t NtCorr = (num_configs > 0 ? v.at(0).size() : 0);
    std::ofstream f(ensemFileName(keys[i]));
    f << v.size() << " " << NtCorr << " 1 0 1\n";
    f << std::setprecision(16);
    for (const auto row : v)
      for (std::size_t i = 0; i < row.size(); ++i)
	f << i << " " << std::real(row[i]) << " " << std::imag(row[i]) << "\n";
  }
}

//! Convert the database to the new format
template <typename K>
void convertEnsem(const std::string& dbase_in, const std::string& dbase_out)
{
  // Open DB
  Datalib::Storage::StorageCorrFunc<K> db_in;
  db_in.open_for_reading(dbase_in);

  Datalib::Storage::StorageCorrFunc<K> db_out;
  db_out.open_for_writing(dbase_out, db_in.num_configs, db_in.metadata, db_in.NtCorr,
			  true /* use new format */);
  const auto& keys = db_in.keys();
  db_out.insert_keys(keys);
  for (const auto& key : keys)
  {
    db_out.insert(key, get_value(key, db_in));
  }
}

//! Return the Frobenius norm of the given matrix
double fnorm(const std::vector<std::vector<std::complex<double>>>& v)
{
  double n = 0;
  for (const auto& row : v)
    for (const auto& value : row)
      n += std::norm(value);
  return std::sqrt(n);
}

//! Return the difference between the two given matrices
std::vector<std::vector<std::complex<double>>>
diff(const std::vector<std::vector<std::complex<double>>>& v,
     const std::vector<std::vector<std::complex<double>>>& w)
{
  if (v.size() != w.size())
    throw std::runtime_error("incompatible dimensions");
  std::vector<std::vector<std::complex<double>>> r;
  r.reserve(v.size());
  for (std::size_t i = 0; i < v.size(); ++i)
  {
    if (w[i].size() != v[i].size())
      throw std::runtime_error("incompatible dimensions");
    std::vector<std::complex<double>> row;
    row.reserve(v[i].size());
    for (std::size_t j = 0; j < v[i].size(); ++j)
      row.push_back(v[i][j] - w[i][j]);
    r.push_back(std::move(row));
  }
  return r;
}

//! Compare ensemble files
template <typename K>
bool compareEnsem(const std::string& dbase0, const std::string& dbase1, double tol, double itol)
{
  // Open DBs
  Datalib::Storage::StorageCorrFunc<K> database0, database1;
  database0.open_for_reading(dbase0);
  database1.open_for_reading(dbase1);

  bool ret = true;
  for (const auto& key : database0.keys())
  {
    const auto& val0 = get_value(key, database0);
    try
    {
      const auto& val1 = get_value(key, database1);
      double ndiff = fnorm(diff(val0, val1));
      double norm = std::max(fnorm(val0), fnorm(val1));
      if (norm <= itol)
	continue;
      std::cout << "key= " << key << " rel_error= " << ndiff / norm << " Frob-norm= " << norm
		<< std::endl;
      if (norm * tol < ndiff)
	ret = false;
    } catch (...)
    {
      std::cout << "key not found in " << dbase1 << ": " << key << std::endl;
      ret = false;
    }
  }

  return ret;
}

//! Compare ensemble files ignoring the phasing information (use for testing)
template <typename K, typename Fun>
bool compareEnsemIgnoring(const std::string& dbase0, const std::string& dbase1, double tol,
			  double itol, Fun& fun)
{
  using V = typename std::vector<std::vector<std::complex<double>>>;
  bool ret = true;
  auto get_keys_and_values = [=, &ret](const std::string& filename) {
    // Read all the key-value pairs
    Datalib::Storage::StorageCorrFunc<K> db;
    db.open_for_reading(filename);
    const auto& [keys, data] = db.keys_and_data();

    // Remove the phasing information of the keys and put the pairs in a map
    auto hash = [](const K& k) { return SB::getHash(k); };
    std::unordered_map<K, V, decltype(hash)> d(keys.size(), hash);
    std::unordered_map<K, K, decltype(hash)> orig_key(keys.size(), hash);
    for (std::size_t i = 0; i < keys.size(); ++i)
    {
      // ki is the current key without phasing
      const auto& ki = fun(keys[i]);
      if (d.count(ki) > 0)
      {
	// if the key without phasing has been found, then report if the values differ
	const auto& val0 = d.at(ki);
	const auto& val1 = data[i];
	double ndiff = fnorm(diff(val0, val1));
	double norm = std::max(fnorm(val0), fnorm(val1));
	if (norm > itol && norm * tol < ndiff)
	{
	  std::cout << "keys in " << filename << ": " << keys[i] << " and " << orig_key.at(ki)
		    << " have different values: rel_error= " << ndiff / norm
		    << " Frob-norm= " << norm << std::endl;
	  ret = false;
	}
	else
	{
	  std::cout << "key= " << keys[i] << " rel_error= " << ndiff / norm
		    << " Frob-norm= " << norm << std::endl;
	}
      }
      else
      {
	d[ki] = data[i];
	orig_key[ki] = keys[i];
      }
    }
    return d;
  };

  // Open DBs
  const auto& m0 = get_keys_and_values(dbase0);
  const auto& m1 = get_keys_and_values(dbase1);

  for (const auto& it0 : m0)
  {
    const auto& key = it0.first;
    if (m1.count(key) == 0)
    {
      std::cout << "key= " << key << " not found in " << dbase1 << std::endl;
      ret = false;
      continue;
    }
    const auto& val0 = it0.second;
    const auto& val1 = m1.at(key);
    double ndiff = fnorm(diff(val0, val1));
    double norm = std::max(fnorm(val0), fnorm(val1));
    if (norm <= itol)
      continue;
    std::cout << "key= " << key << " rel_error= " << ndiff / norm << " Frob-norm= " << norm
	      << std::endl;
    if (norm * tol < ndiff)
      ret = false;
  }

  return ret;
}

//! Check files
template <typename K>
bool checkEnsem(const std::string& dbase)
{
  // Open DB
  Datalib::Storage::StorageCorrFunc<K> database;
  database.open_for_reading(dbase);

  database.string_keys_and_data();

  return true;
}

//! Print all the keys
void printMesonKeys(const std::string& dbase)
{
  // Open DB
  Datalib::TensorStorage::StorageMeson database;
  if (database.open({dbase}) != 0)
  {
    std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
    exit(1);
  }

  std::vector<KeyMesonElementalOperator_t> keys;
  database.keys(keys);

  for (int i = 0; i < keys.size(); ++i)
  {
    std::cout << keys[i] << "\n";
  }
}

//! Print all the keys to a xml file
void printMesonKeysXML(const std::string& xml_file, const std::string& dbase)
{
  // Open DB
  Datalib::TensorStorage::StorageMeson database;
  if (database.open({dbase}) != 0)
  {
    std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
    exit(1);
  }

  std::vector<KeyMesonElementalOperator_t> keys;
  database.keys(keys);

  // Print keys
  Datalib::Serial::XMLFileWriter xml_out(xml_file);
  push(xml_out, "Keys");
  for (int i = 0; i < keys.size(); ++i)
  {
    write(xml_out, "elem", keys[i]);
  }
  pop(xml_out);

  xml_out.close();
}

//! Print meson elemental
void printMeson(const std::string& xml_key_file, const std::string& dbase)
{
  typedef KeyMesonElementalOperator_t K;
  typedef ValMesonElementalOperator_t V;

  try
  {
    // Open DB
    Datalib::TensorStorage::StorageMeson meson_db;
    if (meson_db.open({dbase}) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
      exit(1);
    }

    // Read the desired keys
    Datalib::Serial::XMLReader xml_in(xml_key_file);
    std::vector<K> keys;
    read(xml_in, "/Keys", keys);

    // Write binary file
    for (const auto& key : keys)
    {
      // The key
      // Find the object
      ValMesonElementalOperator_t val;
      if (meson_db.get(key, val) != 0)
      {
	std::cerr << __func__ << ": key not found: key=" << key << std::endl;
	exit(1);
      }

      // Build filename
      std::string file_name;

      if (1)
      {
	std::ostringstream os;
	os << "meson";
	os << ".t" << key.t_slice;
	os << ".d";
	for (int i = 0; i < key.displacement.size(); ++i)
	{
	  os << key.displacement[i];
	}

	os << ".p";
	for (int i = 0; i < key.mom.size(); ++i)
	{
	  os << key.mom[i];
	}

	os << ".dat";

	file_name = os.str();
      }

      // Write the file
      Datalib::Binary::BinaryFileWriter bin(file_name);

      int nrows = val.op.nrows();
      int ncols = val.op.nrows();

      write(bin, nrows);
      write(bin, ncols);

      // Write the size
      for (int cl = 0; cl < val.op.nrows(); ++cl)
      {
	for (int cr = 0; cr < val.op.ncols(); ++cr)
	{
	  write(bin, val.op(cl, cr));
	} // cr
      }	  // cl
    }	  // key
  } catch (const std::string& e)
  {
    std::cerr << __func__ << ": Caught Exception: " << e << std::endl;
    exit(1);
  } catch (std::exception& e)
  {
    std::cerr << __func__ << ": Caught standard library exception: " << e.what() << std::endl;
    exit(1);
  }
}

//! Print prop elemental
void printProp(const std::string& xml_key_file, const std::string& dbase)
{
  typedef KeyProp4ElementalOperator_t K;
  typedef ValProp4ElementalOperator_t V;

  try
  {
    // Open DB
    Datalib::TensorStorage::StorageProp4 prop_db;
    if (prop_db.open(std::vector<std::string>{dbase}) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
      exit(1);
    }

    // Read the desired keys
    Datalib::Serial::XMLReader xml_in(xml_key_file);
    std::vector<K> keys;
    read(xml_in, "/Keys", keys);

    // Write binary file
    for (auto k = keys.begin(); k != keys.end(); ++k)
    {
      // Find the object
      ValProp4ElementalOperator_t val;
      if (prop_db.get(*k, val) != 0)
      {
	std::cerr << __func__ << ": key not found: key=" << *k << std::endl;
	exit(1);
      }

      int n = val.op.size3();

      for (int sl = 0; sl < Ns; ++sl)
      {
	for (int sr = 0; sr < Ns; ++sr)
	{
	  // Build filename
	  std::string file_name;

	  {
	    std::ostringstream os;
	    os << "prop";
	    os << ".m_" << k->mass_label;
	    os << ".t_" << k->t_slice;
	    os << ".t0_" << k->t_source;
	    os << ".sl_" << sl;
	    os << ".sr_" << sr;
	    os << ".dat";

	    file_name = os.str();
	  }

	  // Write the file
	  Datalib::Binary::BinaryFileWriter bin(file_name);

	  // Write the size
	  write(bin, n);
	  write(bin, n);

	  // Write the objects
	  for (int cl = 0; cl < n; ++cl)
	  {
	    for (int cr = 0; cr < n; ++cr)
	    {
	      write(bin, val.op(cl, cr, sl, sr));
	    } // cr
	  }   // cl
	}
      }
    } // key
  } catch (const std::string& e)
  {
    std::cerr << __func__ << ": Caught Exception: " << e << std::endl;
    exit(1);
  } catch (std::exception& e)
  {
    std::cerr << __func__ << ": Caught standard library exception: " << e.what() << std::endl;
    exit(1);
  }
}

//! Compare meson elemental
void compareMeson(const std::string& dbase0, const std::string& dbase1, double tol, double itol)
{
  typedef KeyMesonElementalOperator_t K;
  typedef ValMesonElementalOperator_t V;

  bool fail = false; // whether dbutil to return a failing status
  try
  {
    // Open DBs
    Datalib::TensorStorage::StorageMeson meson0_db;
    if (meson0_db.open({dbase0}) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase0 << std::endl;
      exit(1);
    }

    Datalib::TensorStorage::StorageMeson meson1_db;
    if (meson1_db.open({dbase1}) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase1 << std::endl;
      exit(1);
    }

    std::vector<K> keys0, keys1;
    meson0_db.keys(keys0);
    meson1_db.keys(keys1);

    bool warning_mismatch_size_given = false;

    for (auto key : keys0)
    {
      // Find the object
      V val0, val1;
      if (meson0_db.get(key, val0) != 0)
      {
	std::cerr << __func__ << ": key not found: key=" << key << std::endl;
	exit(1);
      }
      if (meson1_db.get(key, val1) != 0)
      {
	fail = true;
	std::cout << "key not found in dbase= " << dbase1 << ": key=" << key << std::endl;
	continue;
      }

      if (val0.op.nrows() != val1.op.nrows() && !warning_mismatch_size_given)
      {
	std::cout << "mismatch meson size in dbase= " << dbase0
		  << ": nrows=" << val0.op.nrows() << " and in dbase= " << dbase1
		  << ": nrows=" << val1.op.nrows() << std::endl;
	warning_mismatch_size_given = true;
      }

      // If the elemental on val1 is smaller than val0, return failing status
      if (val0.op.nrows() > val1.op.nrows())
	fail = true;

      // If the mesons have difference sizes, compare only the common region
      int n = std::min(val0.op.nrows(), val1.op.nrows());

      // Compute the Frobenius norm of the mesons and its difference
      double fnorm0 = 0;     // F-norm squared of meson0_db's meson
      double fnorm1 = 0;     // F-norm squared of meson1_db's meson
      double fnorm_diff = 0; // F-norm squared of the difference between the mesons
      for (int cl = 0; cl < n; ++cl)
      {
	for (int cr = 0; cr < n; ++cr)
	{
	  fnorm_diff += std::norm(val0.op(cl, cr) - val1.op(cl, cr));
	  fnorm0 += std::norm(val0.op(cl, cr));
	  fnorm1 += std::norm(val1.op(cl, cr));
	}
      }
      double fnorm = std::sqrt(std::max(fnorm0, fnorm1));
      double err = std::sqrt(fnorm_diff);

      // If the error is bigger than tol, return a failing status
      if (err > itol && err > fnorm * tol)
	fail = true;

      std::cout << "key= " << key << " rel_error= " << err / fnorm << " Frob-norm= " << fnorm
		<< std::endl;

    } // key

    for (auto key : keys1)
    {
      V val0;
      if (meson0_db.get(key, val0) != 0)
      {
	std::cout << "key not found in dbase= " << dbase0 << ": key=" << key << std::endl;
      }
    }
  } catch (const std::string& e)
  {
    std::cerr << __func__ << ": Caught Exception: " << e << std::endl;
    exit(1);
  } catch (std::exception& e)
  {
    std::cerr << __func__ << ": Caught standard library exception: " << e.what() << std::endl;
    exit(1);
  }

  if (fail)
    exit(1);
}

//! Relabel the mass on all keys
void relabelMassProp(const std::string& db_filename_in, const std::string& db_filename_out,
		     const std::string new_mass_label)
{
  typedef KeyPropElementalOperator_t K;
  typedef ValPropElementalOperator_t V;

  try
  {
    // Open DB
    ConfDataStoreDB<Datalib::Storage::SerialDBKey<K>, Datalib::Storage::SerialDBData<V>> dbin;
    if (dbin.open(db_filename_in, O_RDONLY, 0400) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << db_filename_in << std::endl;
      exit(1);
    }

    std::string meta_data;
    if (dbin.getUserdata(meta_data) != 0)
    {
      std::cerr << __func__ << ": some error reading meta data out of db=" << db_filename_in
		<< std::endl;
      exit(1);
    }

    // Get all keys
    std::vector<Datalib::Storage::SerialDBKey<K>> keys;
    dbin.keys(keys);

    // Open output DB
    ConfDataStoreDB<Datalib::Storage::SerialDBKey<K>, Datalib::Storage::SerialDBData<V>> dbout;
    dbin.setCacheSize(1024 * 1024 * 1024);
    dbout.setMaxUserInfoLen(meta_data.size());
    if (dbout.open(db_filename_out, O_RDWR | O_CREAT | O_TRUNC, 0644) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << db_filename_out << std::endl;
      exit(1);
    }
    dbout.insertUserdata(meta_data);
    dbout.setNumberBuckets(keys.size());

    // Process all keys
    Datalib::Storage::SerialDBData<V> val;
    for (auto key : keys)
    {
      // Find the object
      if (dbin.get(key, val) != 0)
      {
	std::cerr << __func__ << ": key not found: key=" << key.key() << std::endl;
	exit(1);
      }

      // Change mass label
      key.key().mass_label = new_mass_label;

      // Insert the new key
      dbout.insert(key, val);
    }

    // Close DBs
    dbin.close();
    dbout.close();
  } catch (const std::string& e)
  {
    std::cerr << __func__ << ": Caught Exception: " << e << std::endl;
    exit(1);
  } catch (std::exception& e)
  {
    std::cerr << __func__ << ": Caught standard library exception: " << e.what() << std::endl;
    exit(1);
  }
}

//! Compare baryon elemental
void compareBaryon(const std::vector<std::string>& dbase0, const std::vector<std::string>& dbase1,
		   double tol, double itol)
{
  typedef KeyBaryonElementalOperator_t K;
  typedef ValBaryonElementalOperator_t V;

  std::string dbase0_str, dbase1_str;
  for (std::string db : dbase0)
    dbase0_str += (dbase0_str.size() > 0 ? "," : "") + db;
  for (std::string db : dbase1)
    dbase1_str += (dbase1_str.size() > 0 ? "," : "") + db;

  bool fail = false; // whether dbutil to return a failing status
  try
  {
    // Open DBs
    Datalib::TensorStorage::StorageBaryon bar0_db;
    if (bar0_db.open(dbase0) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase0_str << std::endl;
      exit(1);
    }

    Datalib::TensorStorage::StorageBaryon bar1_db;
    if (bar1_db.open(dbase1) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase1_str << std::endl;
      exit(1);
    }

    std::vector<Datalib::Storage::SerialDBKey<K>> keys0, keys1;
    bar0_db.keys(keys0);
    bar1_db.keys(keys1);

    bool warning_mismatch_size_given = false;

    for (auto key : keys0)
    {
      // Find the object
      Datalib::Storage::SerialDBData<V> val0, val1;
      if (bar0_db.get(key, val0) != 0)
      {
	std::cerr << __func__ << ": key not found: key=" << key.key() << std::endl;
	exit(1);
      }
      if (bar1_db.get(key, val1) != 0)
      {
	fail = true;
	std::cout << "key not found in dbase= " << dbase1_str << ": key=" << key.key() << std::endl;
	continue;
      }

      // Print a warning if the number of colorvecs is different
      if ((val0.data().op.leftSize() != val1.data().op.leftSize() ||
	   val0.data().op.middleSize() != val1.data().op.middleSize() ||
	   val0.data().op.rightSize() != val1.data().op.rightSize()) &&
	  !warning_mismatch_size_given)
      {
	std::cout << "mismatch elem size in dbase= " << dbase0_str
		  << ": #colorvecs=" << val0.data().op.leftSize() << " and in dbase= " << dbase1_str
		  << ": #colorvecs=" << val1.data().op.leftSize() << std::endl;
	warning_mismatch_size_given = true;
      }

      // Return a failing status if the elemental val1 is smaller than val0
      if ((val0.data().op.leftSize() > val1.data().op.leftSize() ||
	   val0.data().op.middleSize() > val1.data().op.middleSize() ||
	   val0.data().op.rightSize() > val1.data().op.rightSize()))
	fail = true;

      // If the props have difference sizes, compare only the common region
      int ni = std::min(val0.data().op.leftSize(), val1.data().op.leftSize());
      int nj = std::min(val0.data().op.middleSize(), val1.data().op.middleSize());
      int nk = std::min(val0.data().op.rightSize(), val1.data().op.rightSize());

      // Compute the Frobenius norm of the props and its difference
      double fnorm0 = 0;     // F-norm squared of db0's elem
      double fnorm1 = 0;     // F-norm squared of db1's elem
      double fnorm_diff = 0; // F-norm squared of the difference between the elems
      for (int i = 0; i < ni; ++i)
      {
	for (int j = 0; j < nj; ++j)
	{
	  for (int k = 0; k < nk; ++k)
	  {
	    fnorm_diff += std::norm(val0.data().op(i, j, k) - val1.data().op(i, j, k));
	    fnorm0 += std::norm(val0.data().op(i, j, k));
	    fnorm1 += std::norm(val1.data().op(i, j, k));
	  }
	}
      }
      double fnorm = std::sqrt(std::max(fnorm0, fnorm1));
      double err = std::sqrt(fnorm_diff);

      // If the error is bigger than tol, return a failing status
      if (err > itol && err > fnorm * tol)
	fail = true;

      std::cout << "key= " << key.key() << " rel_error= " << err / fnorm << " Frob-norm= " << fnorm
		<< std::endl;

    } // key

    for (auto key : keys1)
    {
      Datalib::Storage::SerialDBData<V> val0;
      if (bar0_db.get(key, val0) != 0)
      {
	std::cout << "key not found in dbase= " << dbase0_str << ": key=" << key.key() << std::endl;
      }
    }
  } catch (const std::string& e)
  {
    std::cerr << __func__ << ": Caught Exception: " << e << std::endl;
    exit(1);
  } catch (std::exception& e)
  {
    std::cerr << __func__ << ": Caught standard library exception: " << e.what() << std::endl;
    exit(1);
  }

  if (fail)
    exit(1);
}

//! Compare prop elemental
void compareProp(const std::vector<std::string>& dbase0, const std::vector<std::string>& dbase1,
		 double tol)
{
  std::string dbase0_str, dbase1_str;
  for (std::string db : dbase0)
    dbase0_str += (dbase0_str.size() > 0 ? "," : "") + db;
  for (std::string db : dbase1)
    dbase1_str += (dbase1_str.size() > 0 ? "," : "") + db;

  bool fail = false; // whether dbutil to return a failing status
  try
  {
    // Open DBs
    Datalib::TensorStorage::StorageProp4 prop0_db;
    if (prop0_db.open(dbase0) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase0_str << std::endl;
      exit(1);
    }

    Datalib::TensorStorage::StorageProp4 prop1_db;
    if (prop1_db.open(dbase1) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase1_str << std::endl;
      exit(1);
    }

    std::vector<KeyProp4ElementalOperator_t> keys0, keys1;
    prop0_db.keys(keys0);
    prop1_db.keys(keys1);

    bool warning_mismatch_size_given = false;

    for (auto key : keys0)
    {
      // Find the object
      ValProp4ElementalOperator_t val0, val1;
      if (prop0_db.get(key, val0) != 0)
      {
	std::cerr << __func__ << ": key not found: key=" << key << std::endl;
	exit(1);
      }
      if (prop1_db.get(key, val1) != 0)
      {
	fail = true;
	std::cout << "key not found in dbase= " << dbase1_str << ": key=" << key << std::endl;
	continue;
      }

      if (val0.op.size3() != val1.op.size3() && !warning_mismatch_size_given)
      {
	std::cout << "mismatch prop size in dbase= " << dbase0_str
		  << ": nrows=" << val0.op.size3() << " and in dbase= " << dbase1_str
		  << ": nrows=" << val1.op.size3() << std::endl;
	warning_mismatch_size_given = true;
      }

      // If the elemental on val1 is smaller than val0, return failing status
      if (val0.op.size3() > val1.op.size3())
	fail = true;

      // If the props have difference sizes, compare only the common region
      int n = std::min(val0.op.size3(), val1.op.size3());

      // Compute the Frobenius norm of the props and its difference
      double fnorm0 = 0;     // F-norm squared of prop0_db's prop
      double fnorm1 = 0;     // F-norm squared of prop1_db's prop
      double fnorm_diff = 0; // F-norm squared of the difference between the props
      for (int cl = 0; cl < n; ++cl)
      {
	for (int cr = 0; cr < n; ++cr)
	{
	  for (int sl = 0; sl < Ns; ++sl)
	  {
	    for (int sr = 0; sr < Ns; ++sr)
	    {
	      fnorm_diff +=
		std::norm(val0.op(cl, cr, sl, sr) - val1.op(cl, cr, sl, sr));
	      fnorm0 += std::norm(val0.op(cl, cr, sl, sr));
	      fnorm1 += std::norm(val1.op(cl, cr, sl, sr));
	    }
	  }
	}
      }
      double fnorm = std::sqrt(std::max(fnorm0, fnorm1));
      double err = std::sqrt(fnorm_diff) / fnorm;

      // If the error is bigger than tol, return a failing status
      if (err > tol)
	fail = true;

      std::cout << "key= " << key << " rel_error= " << err
                << " Frob-norm= " << fnorm << std::endl;

    } // key

    for (auto key : keys1)
    {
      ValProp4ElementalOperator_t val0;
      if (prop0_db.get(key, val0) != 0)
      {
	std::cout << "key not found in dbase= " << dbase0_str << ": key=" << key << std::endl;
      }
    }
  } catch (const std::string& e)
  {
    std::cerr << __func__ << ": Caught Exception: " << e << std::endl;
    exit(1);
  } catch (std::exception& e)
  {
    std::cerr << __func__ << ": Caught standard library exception: " << e.what() << std::endl;
    exit(1);
  }

  if (fail)
    exit(1);
}

//! Print unsmeared meson elemental
void printUnsmearedMeson(const std::string& xml_key_file, const std::string& dbase)
{
  typedef KeyUnsmearedMesonElementalOperator_t K;
  typedef ValUnsmearedMesonElementalOperator_t V;

  try
  {
    // Open DB
    ConfDataStoreDB<Datalib::Storage::SerialDBKey<K>, Datalib::Storage::SerialDBData<V>> db;
    if (db.open(dbase, O_RDONLY, 0400) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
      exit(1);
    }

    std::string meta_data;
    if (db.getUserdata(meta_data) != 0)
    {
      std::cerr << __func__ << ": some error reading meta data out of db=" << dbase << std::endl;
      exit(1);
    }

    // Read the desired keys
    Datalib::Serial::XMLReader xml_in(xml_key_file);
    std::vector<K> keys;
    read(xml_in, "/Keys", keys);

    // Write binary file
    for (auto key : keys)
    {
      // Find the object
      Datalib::Storage::SerialDBData<V> val;
      if (db.get(key, val) != 0)
      {
	std::cerr << __func__ << ": key not found: key=" << key << std::endl;
	exit(1);
      }

      // Build filename
      std::string file_name;

      if (1)
      {
	std::ostringstream os;
	os << "unsmearedMeson";
	os << ".m_" << key.mass;
	os << ".t_" << key.t_slice;
	os << ".t0_" << key.t_source;
	os << ".t1_" << key.t_sink;
	os << ".n_" << key.colorvec_src;
	os << ".g_" << key.gamma;
	os << ".d";
	for (int d : key.displacement)
	  os << "_" << d;
	os << ".m";
	for (unsigned int i = 0; i < key.mom.size(); ++i)
	  os << "_" << key.mom(i);
	os << ".dat";

	file_name = os.str();
      }

      // Write the file
      Datalib::Binary::BinaryFileWriter bin(file_name);

      int ni = val.data().op.leftSize();
      int nj = val.data().op.middleSize();
      int nk = val.data().op.rightSize();

      // Write the size
      write(bin, ni);
      write(bin, nj);
      write(bin, nk);

      // Write the objects
      for (int i = 0; i < ni; ++i)
      {
	for (int j = 0; j < nj; ++j)
	{
	  for (int k = 0; k < nk; ++k)
	  {
	    write(bin, val.data().op(i, j, k));
	  }
	}
      }
    }
  } catch (const std::string& e)
  {
    std::cerr << __func__ << ": Caught Exception: " << e << std::endl;
    exit(1);
  } catch (std::exception& e)
  {
    std::cerr << __func__ << ": Caught standard library exception: " << e.what() << std::endl;
    exit(1);
  }
}

//! Print genprop4 meson elemental
void printGenprop4(const std::string& xml_key_file, const std::string& dbase)
{
  typedef KeyGenProp4ElementalOperator_t K;
  typedef ValGenProp4ElementalOperator_t V;

  try
  {
    // Open DB
    Datalib::TensorStorage::StorageGenprop4 db;
    if (db.open(std::vector<std::string>{dbase}) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
      exit(1);
    }

    // Read the desired keys
    Datalib::Serial::XMLReader xml_in(xml_key_file);
    std::vector<K> keys;
    read(xml_in, "/Keys", keys);

    // Write binary file
    for (auto key : keys)
    {
      // Find the object
      Datalib::Storage::SerialDBData<V> val;
      if (db.get(key, val) != 0)
      {
	std::cerr << __func__ << ": key not found: key=" << key << std::endl;
	exit(1);
      }

      // Build filename
      std::string file_name;

      if (1)
      {
	std::ostringstream os;
	os << "genprop4";
	os << ".m_" << key.mass;
	os << ".t_" << key.t_slice;
	os << ".t0_" << key.t_source;
	os << ".t1_" << key.t_sink;
	os << ".g_" << key.g;
	os << ".d";
	for (int d : key.displacement)
	  os << "_" << d;
	os << ".m";
	for (unsigned int i = 0; i < key.mom.size(); ++i)
	  os << "_" << key.mom(i);
	os << ".dat";

	file_name = os.str();
      }

      // Write the file
      Datalib::Binary::BinaryFileWriter bin(file_name);

      int ni = val.data().op.size1(); // colorvec sink?
      int nj = val.data().op.size2(); // colorvec source?
      int nk = val.data().op.size3(); // spin sink
      int nl = val.data().op.size4(); // spin source

      // Write the size
      write(bin, ni);
      write(bin, nj);
      write(bin, nk);
      write(bin, nl);

      // Write the objects
      for (int l = 0; l < nl; ++l)
	for (int k = 0; k < nk; ++k)
	  for (int j = 0; j < nj; ++j)
	    for (int i = 0; i < ni; ++i)
	      write(bin, val.data().op(i, j, k, l));
    }
  } catch (const std::string& e)
  {
    std::cerr << __func__ << ": Caught Exception: " << e << std::endl;
    exit(1);
  } catch (std::exception& e)
  {
    std::cerr << __func__ << ": Caught standard library exception: " << e.what() << std::endl;
    exit(1);
  }
}

//! Compare genprop4 meson elemental
void compareGenprop4(const std::vector<std::string>& dbase0, const std::vector<std::string>& dbase1,
		     double tol)
{
  typedef KeyGenProp4ElementalOperator_t K;
  typedef ValGenProp4ElementalOperator_t V;

  std::string dbase0_str, dbase1_str;
  for (std::string db : dbase0)
    dbase0_str += (dbase0_str.size() > 0 ? "," : "") + db;
  for (std::string db : dbase1)
    dbase1_str += (dbase1_str.size() > 0 ? "," : "") + db;

  bool fail = false; // whether dbutil to return a failing status
  try
  {
    // Open DBs
    Datalib::TensorStorage::StorageGenprop4 db0;
    if (db0.open(dbase0) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase0_str << std::endl;
      exit(1);
    }

    Datalib::TensorStorage::StorageGenprop4 db1;
    if (db1.open(dbase1) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase1_str << std::endl;
      exit(1);
    }

    std::vector<Datalib::Storage::SerialDBKey<K>> keys0, keys1;
    db0.keys(keys0);
    db1.keys(keys1);

    bool warning_mismatch_size_given = false;

    for (auto key : keys0)
    {
      // Find the object
      Datalib::Storage::SerialDBData<V> val0, val1;
      if (db0.get(key, val0) != 0)
      {
	std::cerr << __func__ << ": key not found: key=" << key.key() << std::endl;
	exit(1);
      }
      if (db1.get(key, val1) != 0)
      {
	fail = true;
	std::cout << "key not found in dbase= " << dbase1_str << ": key=" << key.key() << std::endl;
	continue;
      }

      // Check proper sizes
      if (val0.data().op.size4() != val0.data().op.size3() || val0.data().op.size2() != Ns ||
	  val0.data().op.size1() != Ns)
      {
	std::cout << "Unexpected elemental size in dbase= " << dbase0_str << std::endl;
	exit(1);
      }
      if (val1.data().op.size4() != val1.data().op.size3() || val1.data().op.size2() != Ns ||
	  val1.data().op.size1() != Ns)
      {
	std::cout << "Unexpected elemental size in dbase= " << dbase1_str << std::endl;
	exit(1);
      }

      // Print a warning if the number of colorvecs is different
      if (val0.data().op.size4() != val1.data().op.size4() && !warning_mismatch_size_given)
      {
	std::cout << "mismatch elem size in dbase= " << dbase0_str
		  << ": #colorvecs=" << val0.data().op.size4() << " and in dbase= " << dbase1_str
		  << ": #colorvecs=" << val1.data().op.size4() << std::endl;
	warning_mismatch_size_given = true;
      }

      // Return a failing status if the elemental val1 is smaller than val0
      if (val0.data().op.size4() > val1.data().op.size4())
	fail = true;

      // If the props have difference sizes, compare only the common region
      int nvecs = std::min(val0.data().op.size4(), val1.data().op.size4());

      // Compute the Frobenius norm of the props and its difference
      double fnorm0 = 0;     // F-norm squared of db0's elem
      double fnorm1 = 0;     // F-norm squared of db1's elem
      double fnorm_diff = 0; // F-norm squared of the difference between the elems
      for (int i = 0; i < Ns; ++i)
      {
	for (int j = 0; j < Ns; ++j)
	{
	  for (int k = 0; k < nvecs; ++k)
	  {
	    for (int l = 0; l < nvecs; ++l)
	    {
	      fnorm_diff += std::norm(val0.data().op(l, k, j, i) - val1.data().op(l, k, j, i));
	      fnorm0 += std::norm(val0.data().op(l, k, j, i));
	      fnorm1 += std::norm(val1.data().op(l, k, j, i));
	    }
	  }
	}
      }
      double err = std::sqrt(fnorm_diff / std::max(fnorm0, fnorm1));

      // If the error is bigger than tol, return a failing status
      if (err > tol)
	fail = true;

      std::cout << "key= " << key.key() << " rel_error= " << err << std::endl;

    } // key

    for (auto key : keys1)
    {
      Datalib::Storage::SerialDBData<V> val0;
      if (db0.get(key, val0) != 0)
      {
	std::cout << "key not found in dbase= " << dbase0_str << ": key=" << key.key() << std::endl;
      }
    }
  } catch (const std::string& e)
  {
    std::cerr << __func__ << ": Caught Exception: " << e << std::endl;
    exit(1);
  } catch (std::exception& e)
  {
    std::cerr << __func__ << ": Caught standard library exception: " << e.what() << std::endl;
    exit(1);
  }

  if (fail)
    exit(1);
}

//! Print disco elemental
void printDiscoKeys(const std::string& dbase)
{
  // Open DB
  Datalib::TensorStorage::StorageDisco disco_db;
  if (disco_db.open({dbase}) != 0)
  {
    std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
    exit(1);
  }

  std::vector<Hadron::KeyTwoQuarkDiscoOperator_t> keys;
  disco_db.keys(keys);

  // Write binary file
  for (const auto& key : keys)
  {
    std::cout << "key= " << key << " value= " << std::endl;
  }
}

//! Print disco elemental
void printDiscoKeysXML(const std::string& xml_file, const std::string& dbase)
{
  // Open DB
  Datalib::TensorStorage::StorageDisco disco_db;
  if (disco_db.open({dbase}) != 0)
  {
    std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
    exit(1);
  }

  std::vector<Hadron::KeyTwoQuarkDiscoOperator_t> keys;
  disco_db.keys(keys);

  // Print keys
  Datalib::Serial::XMLFileWriter xml_out(xml_file);
  push(xml_out, "Keys");
  for (const auto& key: keys)
  {
    write(xml_out, "elem", key);
  }
  pop(xml_out);
  xml_out.close();
}

//! Print disco elemental
void printDisco(const std::string& xml_key_file, const std::string& dbase)
{
  // Open DB
  Datalib::TensorStorage::StorageDisco disco_db;
  if (disco_db.open({dbase}) != 0)
  {
    std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
    exit(1);
  }

  // Read the desired keys
  Datalib::Serial::XMLReader xml_in(xml_key_file);
  std::vector<Hadron::KeyTwoQuarkDiscoOperator_t> keys;
  read(xml_in, "/Keys", keys);

  // Write binary file
  for (const auto& key : keys)
  {
    // Find the object
    Hadron::ValTwoQuarkDiscoOperator_t val;
    if (disco_db.get(key, val) != 0)
    {
      std::cerr << __func__ << ": key not found: key=" << key << std::endl;
      exit(1);
    }

    std::cout << "key= " << key << " value= ";

    // Write the objects
    for (int i = 0; i < val.op.size(); ++i)
    {
      std::cout << val.op[i] << " ";
    }
    std::cout << std::endl;
  }
}

//! Compare disco elemental
void compareDisco(const std::string& dbase0, const std::string& dbase1, double tol)
{
  bool fail = false; // whether dbutil to return a failing status

  // Open DBs
  Datalib::TensorStorage::StorageDisco disco0_db;
  if (disco0_db.open({dbase0}) != 0)
  {
    std::cerr << __func__ << ": error opening dbase= " << dbase0 << std::endl;
    exit(1);
  }

  Datalib::TensorStorage::StorageDisco disco1_db;
  if (disco1_db.open({dbase1}) != 0)
  {
    std::cerr << __func__ << ": error opening dbase= " << dbase1 << std::endl;
    exit(1);
  }

  std::vector<Hadron::KeyTwoQuarkDiscoOperator_t> keys0, keys1;
  disco0_db.keys(keys0);
  disco1_db.keys(keys1);

  for (const auto& key : keys0)
  {
    // Find the object
    ValTwoQuarkDiscoOperator_t val0, val1;
    if (disco0_db.get(key, val0) != 0)
    {
      std::cerr << __func__ << ": key not found: key=" << key << std::endl;
      exit(1);
    }
    if (disco1_db.get(key, val1) != 0)
    {
      fail = true;
      std::cout << "key not found in dbase= " << dbase1 << ": key=" << key << std::endl;
      continue;
    }

    const int n = Ns * Ns;
    if (val0.op.size() != n || val1.op.size() != n)
    {
      std::cerr << __func__ << ": value does not have enough content: key=" << key
		<< std::endl;
      exit(1);
    }

    // Compute the Frobenius norm of the discos and its difference
    double fnorm0 = 0;	   // F-norm squared of disco0_db's disco
    double fnorm1 = 0;	   // F-norm squared of disco1_db's disco
    double fnorm_diff = 0; // F-norm squared of the difference between the discos
    for (int g = 0; g < n; ++g)
    {
      fnorm_diff += std::norm(val0.op[g] - val1.op[g]);
      fnorm0 += std::norm(val0.op[g]);
      fnorm1 += std::norm(val1.op[g]);
    }
    double err = std::sqrt(fnorm_diff / std::max(fnorm0, fnorm1));

    // If the error is bigger than tol, return a failing status
    if (err > tol)
      fail = true;

    std::cout << "key= " << key << " rel_error= " << err << std::endl;

  } // key

  for (const auto& key : keys1)
  {
    ValTwoQuarkDiscoOperator_t val0;
    if (disco0_db.get(key, val0) != 0)
    {
      std::cout << "key not found in dbase= " << dbase0 << ": key=" << key << std::endl;
    }
  }

  if (fail)
    exit(1);
}

//! Print all the keys
template <typename K>
void printFilehashKeys(const std::string& dbase)
{
  // Open DB
  AllConfStoreDB<Datalib::Storage::SerialDBKey<K>, UserData> database;
  if (database.open(dbase, O_RDONLY, 0400) != 0)
  {
    std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
    exit(1);
  }

  std::vector<Datalib::Storage::SerialDBKey<K>> keys;
  database.keys(keys);

  for (int i = 0; i < keys.size(); ++i)
  {
    std::cout << keys[i].key() << "\n";
  }
}

//! Print all the keys to a xml file
template <typename K>
void printFilehashKeysXML(const std::string& xml_file, const std::string& dbase)
{
  // Open DB
  AllConfStoreDB<Datalib::Storage::SerialDBKey<K>, UserData> database;
  if (database.open(dbase, O_RDONLY, 0400) != 0)
  {
    std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
    exit(1);
  }

  std::vector<Datalib::Storage::SerialDBKey<K>> keys;
  database.keys(keys);

  // Print keys
  Datalib::Serial::XMLFileWriter xml_out(xml_file);
  push(xml_out, "Keys");
  for (int i = 0; i < keys.size(); ++i)
  {
    write(xml_out, "elem", keys[i].key());
  }
  pop(xml_out);

  xml_out.close();
}

//! Check the \gamma-5 Hermiticity of props
void check_g5herm(const std::vector<std::string>& dbase, double tol)
{
  std::string dbase_str;
  for (std::string db : dbase)
    dbase_str += (dbase_str.size() > 0 ? "," : "") + db;

  bool fail = false; // whether dbutil to return a failing status
  try
  {
    // Open DBs
    Datalib::TensorStorage::StorageProp4 prop_db;
    if (prop_db.open(dbase) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase_str << std::endl;
      exit(1);
    }

    std::vector<KeyProp4ElementalOperator_t> keys;
    prop_db.keys(keys);

    for (const auto& key0 : keys)
    {
      // Avoid checking twice
      if (key0.t_slice > key0.t_source)
	continue;

      // Find the object
      ValProp4ElementalOperator_t val0;
      if (prop_db.get(key0, val0) != 0)
      {
	std::cerr << __func__ << ": key not found: key=" << key0 << std::endl;
	exit(1);
      }

      // Get the transposed key (swap sink-source time slices and phasings)
      // NOTE: `phasing_sink` is the phasing of eigenvectors before being conjugated;
      //       so it needs to be negated before and after the exchange with `phasing_source`.
      auto key1 = key0;
      std::swap(key1.t_slice, key1.t_source);
      key1.phasing_sink = -key1.phasing_sink;
      std::swap(key1.phasing_sink, key1.phasing_source);
      key1.phasing_sink = -key1.phasing_sink;
      ValProp4ElementalOperator_t val1;
      if (prop_db.get(key1, val1) != 0)
      {
	continue;
      }

      // If the props have difference sizes, compare only the common region
      int n = std::min(val0.op.size3(), val1.op.size3());

      // Compute the Frobenius norm of the props and its difference
      double fnorm = 0;     // F-norm squared of prop_db's prop
      double fnorm_diff = 0; // F-norm squared of the difference between the prop and its g5-hermitian
      for (int cl = 0; cl < n; ++cl)
      {
	for (int cr = 0; cr < n; ++cr)
	{
	  for (int sl = 0; sl < Ns; ++sl)
	  {
	    for (int sr = 0; sr < Ns; ++sr)
	    {
	      fnorm_diff += std::norm(val0.op(cl, cr, sl, sr) - std::conj(val1.op(cr, cl, sr, sl)) *
								  (sl >= 2 xor sr >= 2 ? -1. : 1.));
	      fnorm +=
		std::max(std::norm(val0.op(cl, cr, sl, sr)), std::norm(val1.op(cr, cl, sr, sl)));
	    }
	  }
	}
      }
      double err = std::sqrt(fnorm_diff / fnorm);

      // If the error is bigger than tol, return a failing status
      if (err > tol)
	fail = true;

      std::cout << "key= " << key0 << " rel_error= " << err
                << " Frob-norm= " << fnorm << std::endl;

    } // key
  } catch (const std::string& e)
  {
    std::cerr << __func__ << ": Caught Exception: " << e << std::endl;
    exit(1);
  } catch (std::exception& e)
  {
    std::cerr << __func__ << ": Caught standard library exception: " << e.what() << std::endl;
    exit(1);
  }

  if (fail)
    exit(1);
}

//! Return the pion mass (Frob norm) of propagators
void check_pion(const std::vector<std::string>& dbase)
{
  // Deal with trivial case: no input files
  if (dbase.size() == 0)
    return;

  // Concatenate all file names into a single string, which will be used for reporting errors
  std::string dbase_str;
  for (std::string db : dbase)
    dbase_str += (dbase_str.size() > 0 ? "," : "") + db;

  // Get time extend
  const int T = getTimeExtent(dbase.front());
  /// Return the canonized coordinate `coor` in a periodic dimension of size `dim`
  const auto norm_coor = [](int coor, int dim) {
    return (dim == 0 ? 0 : (coor + dim * (coor < 0 ? -coor / dim + 1 : 0)) % dim);
  };
  /// Return t1 - t0
  const auto time_diff = [&](int t0, int t1) { return norm_coor(t1 - t0, T); };

  try
  {
    // Open DBs
    Datalib::TensorStorage::StorageProp4 prop_db;
    if (prop_db.open(dbase) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase_str << std::endl;
      exit(1);
    }

    // Get all the keys
    std::vector<KeyProp4ElementalOperator_t> keys;
    prop_db.keys(keys);

    // Structures to store the results
    using mass_sink_source_phasings_tsource_t =
      std::tuple<std::string, std::array<int, 3>, std::array<int, 3>, int>;
    auto hash = [](const mass_sink_source_phasings_tsource_t& k) { return SB::getHash(k); };
    using tdiff_frob_t = std::map<int, double>;
    std::unordered_map<mass_sink_source_phasings_tsource_t, tdiff_frob_t, decltype(hash)> f(
      keys.size(), hash);

    for (const auto& key : keys)
    {
      // Find the object
      ValProp4ElementalOperator_t val;
      if (prop_db.get(key, val) != 0)
      {
	std::cerr << __func__ << ": key not found: key=" << key << std::endl;
	exit(1);
      }

      const auto& mass_label = key.mass_label;
      const auto& phasing_source = Datalib::TensorStorage::detail::toCoor(key.phasing_source);
      const auto& phasing_sink = Datalib::TensorStorage::detail::toCoor(key.phasing_sink);
      const auto t_source = key.t_source;
      const auto t_sink = key.t_slice;

      // Make the prop square
      // The cl/cr indices in val.op(cl, cr, sl, sr) correspond to size3 and size4 (1-based indexing of XMLArray::Array4d)
      int n = std::min(val.op.size3(), val.op.size4());

      // Compute the Frobenius norm of the prop
      double fnorm = 0;
      for (int cl = 0; cl < n; ++cl)
      {
	for (int cr = 0; cr < n; ++cr)
	{
	  for (int sl = 0; sl < Ns; ++sl)
	  {
	    for (int sr = 0; sr < Ns; ++sr)
	    {
	      fnorm += std::norm(val.op(cl, cr, sl, sr));
	    }
	  }
	}
      }

      // Record the frob norm
      f[mass_sink_source_phasings_tsource_t{mass_label, phasing_sink, phasing_source, t_source}]
       [time_diff(t_source, t_sink)] = fnorm;
    } // key

    // Report all frob norms
    for (const auto& it : f)
    {
      const auto& [mass_label, phasing_sink, phasing_source, t_source] = it.first;
      const auto& tdiff_vs_frob = it.second;
      if (tdiff_vs_frob.size() <= 1)
	continue;
      for (const auto& [tdiff, frob_norm] : tdiff_vs_frob)
	std::cout << "mass=" << mass_label << " phasing_sink=" //
		  << phasing_sink.at(0) << "," << phasing_sink.at(1) << "," << phasing_sink.at(2)
		  << " phasing_source=" //
		  << phasing_source.at(0) << "," << phasing_source.at(1) << ","
		  << phasing_source.at(2) //
		  << " corr: t= " << (t_source + tdiff) % T << "    dt= " << tdiff
		  << " t0= " << t_source << "  cor= " << frob_norm << std::endl;
    }
  } catch (const std::string& e)
  {
    std::cerr << __func__ << ": Caught Exception: " << e << std::endl;
    exit(1);
  } catch (std::exception& e)
  {
    std::cerr << __func__ << ": Caught standard library exception: " << e.what() << std::endl;
    exit(1);
  }
}


//! Handle all the work
template <typename K>
int workHandler(int argc, char** argv)
{
  std::string dbase = argv[1];
  std::string op = argv[2];

  // A simplified version of handler
  if (op == "meta")
  {
    printMeta(dbase);
  }
  else if (op == "keys")
  {
    printCorrKeys<K>(dbase);
  }
  else if (op == "keysxml")
  {
    if (argc != 4)
    {
      return usage(argc, argv);
    }

    printCorrKeysXML<K>(argv[3], dbase);
  }
  else if (op == "get")
  {
    if (argc != 4)
    {
      return usage(argc, argv);
    }

    printEnsem<K>(argv[3], dbase);
  }
  else if (op == "check")
  {
    if (argc != 3)
    {
      return usage(argc, argv);
    }

    checkEnsem<K>(dbase);
  }
  else if (op == "compare")
  {
    std::array<std::vector<std::string>, 2> files{};
    double compare_tol = 1, ignore_tol = 0;
    for (int i = 1, s = 0; i < argc; ++i)
    {
      if (s == 0 && std::strcmp(argv[i], "compare") == 0)
      {
	s = 1;
	continue;
      }
      else if (std::strcmp(argv[i], "-tol") == 0 && i + 1 < argc)
      {
	compare_tol = std::atof(argv[++i]);
	if (i + 1 < argc)
	  ignore_tol = std::atof(argv[++i]);
	continue;
      }
      files[s].push_back(argv[i]);
    }
    if (files[0].size() != 1 || files[1].size() != 1)
    {
      return usage(argc, argv);
    }

    return compareEnsem<K>(files[0][0], files[1][0], compare_tol, ignore_tol) ? 0 : -1;
  }
  else if (op == "compare-no-phasing")
  {
    if constexpr (std::is_same<K, KeyHadronSUNClassNPartNPtCorr_t>::value)
    {
      std::array<std::vector<std::string>, 2> files{};
      double compare_tol = 1, ignore_tol = 0;
      for (int i = 1, s = 0; i < argc; ++i)
      {
	if (s == 0 && std::strcmp(argv[i], "compare-no-phasing") == 0)
	{
	  s = 1;
	  continue;
	}
	else if (std::strcmp(argv[i], "-tol") == 0 && i + 1 < argc)
	{
	  compare_tol = std::atof(argv[++i]);
	  if (i + 1 < argc)
	    ignore_tol = std::atof(argv[++i]);
	  continue;
	}
	files[s].push_back(argv[i]);
      }
      if (files[0].size() != 1 || files[1].size() != 1)
      {
	return usage(argc, argv);
      }

      auto remove_phasing = [=](K key) {
	key.set_phasing(Datalib::Model::zero_mom());
	return key;
      };
      return compareEnsemIgnoring<K>(files[0][0], files[1][0], compare_tol, ignore_tol,
				     remove_phasing)
	       ? 0
	       : -1;
    }
    else
    {
      std::cerr << argv[0] << ": unsupported operation for this file operation= " << op
		<< std::endl;
      return -1;
    }
  }
  else if (op == "compare-no-smearing")
  {
    if constexpr (std::is_same<K, KeyHadronSUNClassNPartNPtCorr_t>::value)
    {
      std::array<std::vector<std::string>, 2> files{};
      double compare_tol = 1, ignore_tol = 0;
      for (int i = 1, s = 0; i < argc; ++i)
      {
	if (s == 0 && std::strcmp(argv[i], "compare-no-smearing") == 0)
	{
	  s = 1;
	  continue;
	}
	else if (std::strcmp(argv[i], "-tol") == 0 && i + 1 < argc)
	{
	  compare_tol = std::atof(argv[++i]);
	  if (i + 1 < argc)
	    ignore_tol = std::atof(argv[++i]);
	  continue;
	}
	files[s].push_back(argv[i]);
      }
      if (files[0].size() != 1 || files[1].size() != 1)
      {
	return usage(argc, argv);
      }

      auto remove_smearing = [=](K key) {
	for (auto& npoint : key.npoint.ref())
	{
	  npoint.irrep.smearedP = false;
	  for (auto& op : npoint.irrep.op.ops.ref())
	    op.phasings.resize(0);
	}
	return key;
      };
      return compareEnsemIgnoring<K>(files[0][0], files[1][0], compare_tol, ignore_tol,
				     remove_smearing)
	       ? 0
	       : -1;
    }
    else
    {
      std::cerr << argv[0] << ": unsupported operation for this file operation= " << op
		<< std::endl;
      return -1;
    }
  }
  else if (op == "convert")
  {
    if (argc != 4)
    {
      return usage(argc, argv);
    }

    convertEnsem<K>(dbase, argv[3]);
  }
  else
  {
    std::cerr << argv[0] << ": unsupported operation= " << op << std::endl;
    return -1;
  }

  return 0;
}

//! Handle all the work
template <typename K, typename V>
int baryonWorkHandler(int argc, char** argv)
{
  std::string dbase = argv[1];
  std::string op = argv[2];

  // Check if some argument is "compare"
  for (int i = 2; i < argc; ++i)
    if (std::strcmp(argv[i], "compare") == 0)
      op = "compare";

  // A simplified version of handler
  if (op == "meta")
  {
    printMeta(dbase);
  }
  else if (op == "keys")
  {
    Datalib::TensorStorage::StorageBaryon database;
    if (database.open(std::vector<std::string>{dbase}) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
      exit(1);
    }

    std::vector<Datalib::Storage::SerialDBKey<K>> keys;
    database.keys(keys);

    for (int i = 0; i < keys.size(); ++i)
      std::cout << keys[i].key() << "\n";
  }
  else if (op == "keysxml")
  {
    if (argc != 4)
    {
      return usage(argc, argv);
    }

    Datalib::TensorStorage::StorageBaryon database;
    if (database.open(std::vector<std::string>{dbase}) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
      exit(1);
    }

    std::vector<Datalib::Storage::SerialDBKey<K>> keys;
    database.keys(keys);

    Datalib::Serial::XMLFileWriter xml_out(argv[3]);
    push(xml_out, "Keys");
    for (int i = 0; i < keys.size(); ++i)
    {
      write(xml_out, "elem", keys[i].key());
    }
    pop(xml_out);
    xml_out.close();
  }
  else if (op == "compare")
  {
    std::array<std::vector<std::string>, 2> files{};
    double compare_tol = 1, ignore_tol = 0;
    for (int i = 1, s = 0; i < argc; ++i)
    {
      if (s == 0 && std::strcmp(argv[i], "compare") == 0)
      {
	s = 1;
	continue;
      }
      else if (std::strcmp(argv[i], "-tol") == 0 && i + 1 < argc)
      {
	++i;
	if (i + 1 < argc)
	  ignore_tol = std::atof(argv[++i]);
	continue;
      }
      files[s].push_back(argv[i]);
    }
    if (files[0].size() == 0 || files[1].size() == 0)
    {
      return usage(argc, argv);
    }

    compareBaryon(files[0], files[1], compare_tol, ignore_tol);
  }
  else
  {
    std::cerr << argv[0] << ": unsupported operation= " << op << std::endl;
    return -1;
  }

  return 0;
}

//! Handle all the work
template <typename K, typename V>
int mesonWorkHandler(int argc, char** argv)
{
  std::string dbase = argv[1];
  std::string op = argv[2];
  double compare_tol = 1, ignore_tol = 0;
  if (argc >= 6 && std::strcmp(argv[4], "-tol") == 0)
  {
    compare_tol = std::atof(argv[5]);
    if (argc >= 7)
      ignore_tol = std::atof(argv[6]);
  }

  // A simplified version of handler
  if (op == "meta")
  {
    printMeta(dbase);
  }
  else if (op == "keys")
  {
    printMesonKeys(dbase);
  }
  else if (op == "keysxml")
  {
    if (argc != 4)
    {
      return usage(argc, argv);
    }

    printMesonKeysXML(argv[3], dbase);
  }
  else if (op == "get")
  {
    if (argc != 4)
    {
      return usage(argc, argv);
    }

    printMeson(argv[3], dbase);
  }
  else if (op == "compare")
  {
    if (argc < 4)
    {
      return usage(argc, argv);
    }

    std::string dbase1 = argv[3];
    compareMeson(dbase, dbase1, compare_tol, ignore_tol);
  }
  else
  {
    std::cerr << argv[0] << ": unsupported operation= " << op << std::endl;
    return -1;
  }

  return 0;
}

//! Handle all the work
template <typename K, typename V>
int propWorkHandler(int argc, char** argv)
{
  std::string dbase = argv[1];
  std::string op = argv[2];

  // Check if some argument is "compare"
  for (int i = 2; i < argc; ++i)
  {
    if (std::strcmp(argv[i], "compare") == 0)
      op = "compare";
    if (std::strcmp(argv[i], "g5herm") == 0)
      op = "g5herm";
    if (std::strcmp(argv[i], "pioncheck") == 0)
      op = "pioncheck";
  }

  // A simplified version of handler
  if (op == "meta")
  {
    printMeta(dbase);
  }
  else if (op == "keys")
  {
    Datalib::TensorStorage::StorageProp4 database;
    if (database.open(std::vector<std::string>{dbase}) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
      exit(1);
    }

    std::vector<K> keys;
    database.keys(keys);

    for (int i = 0; i < keys.size(); ++i)
      std::cout << keys[i] << "\n";
  }
  else if (op == "g5herm")
  {
    double compare_tol = 0.1;
    std::vector<std::string> files;
    for (int i = 1, s = 0; i < argc; ++i)
    {
      if (s == 0 && std::strcmp(argv[i], "g5herm") == 0)
      {
	s = 1;
	continue;
      }
      else if (std::strcmp(argv[i], "-tol") == 0 && i + 1 < argc)
      {
	compare_tol = std ::atof(argv[i + 1]);
	++i;
	continue;
      }
      files.push_back(argv[i]);
    }
    if (files.size() == 0)
    {
      return usage(argc, argv);
    }

    check_g5herm(files, compare_tol);
  }
  else if (op == "pioncheck")
  {
    std::vector<std::string> files;
    for (int i = 1; i < argc; ++i)
    {
      if (std::strcmp(argv[i], "pioncheck") != 0)
      {
	files.push_back(argv[i]);
      }
    }
    if (files.size() == 0)
    {
      return usage(argc, argv);
    }

    check_pion(files);
  }
  else if (op == "keysxml")
  {
    Datalib::TensorStorage::StorageProp4 database;
    if (database.open(std::vector<std::string>{dbase}) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
      exit(1);
    }

    std::vector<K> keys;
    database.keys(keys);

    Datalib::Serial::XMLFileWriter xml_out(argv[3]);
    push(xml_out, "Keys");
    for (int i = 0; i < keys.size(); ++i)
    {
      write(xml_out, "elem", keys[i]);
    }
    pop(xml_out);
    xml_out.close();
  }
  else if (op == "get")
  {
    if (argc != 4)
    {
      return usage(argc, argv);
    }

    printProp(argv[3], dbase);
  }
  else if (op == "compare")
  {
    std::array<std::vector<std::string>, 2> files{};
    double compare_tol = 1;
    for (int i = 1, s = 0; i < argc; ++i)
    {
      if (s == 0 && std::strcmp(argv[i], "compare") == 0)
      {
	s = 1;
	continue;
      }
      else if (std::strcmp(argv[i], "-tol") == 0 && i + 1 < argc)
      {
	compare_tol = std ::atof(argv[i + 1]);
	++i;
	continue;
      }
      files[s].push_back(argv[i]);
    }
    if (files[0].size() == 0 || files[1].size() == 0)
    {
      return usage(argc, argv);
    }

    compareProp(files[0], files[1], compare_tol);
  }
  else if (op == "relabel_mass")
  {
    if (argc < 5)
    {
      return usage(argc, argv);
    }

    std::string new_mass_label = argv[3];
    std::string dbase_out = argv[4];
    relabelMassProp(dbase, dbase_out, new_mass_label);
  }
  else
  {
    std::cerr << argv[0] << ": unsupported operation= " << op << std::endl;
    return -1;
  }

  return 0;
}

//! Handle all the work
template <typename K, typename V>
int genprop4WorkHandler(int argc, char** argv)
{
  std::string dbase = argv[1];
  std::string op = argv[2];

  // Check if some argument is "compare"
  for (int i = 2; i < argc; ++i)
    if (std::strcmp(argv[i], "compare") == 0)
      op = "compare";

  // A simplified version of handler
  if (op == "meta")
  {
    printMeta(dbase);
  }
  else if (op == "keys")
  {
    Datalib::TensorStorage::StorageGenprop4 database;
    if (database.open(std::vector<std::string>{dbase}) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
      exit(1);
    }

    std::vector<Datalib::Storage::SerialDBKey<K>> keys;
    database.keys(keys);

    for (int i = 0; i < keys.size(); ++i)
      std::cout << keys[i].key() << "\n";
  }
  else if (op == "keysxml")
  {
    if (argc != 4)
    {
      return usage(argc, argv);
    }

    Datalib::TensorStorage::StorageGenprop4 database;
    if (database.open(std::vector<std::string>{dbase}) != 0)
    {
      std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
      exit(1);
    }

    std::vector<Datalib::Storage::SerialDBKey<K>> keys;
    database.keys(keys);

    Datalib::Serial::XMLFileWriter xml_out(argv[3]);
    push(xml_out, "Keys");
    for (int i = 0; i < keys.size(); ++i)
    {
      write(xml_out, "elem", keys[i].key());
    }
    pop(xml_out);
    xml_out.close();
  }
  else if (op == "get")
  {
    if (argc != 4)
    {
      return usage(argc, argv);
    }

    printGenprop4(argv[3], dbase);
  }
  else if (op == "compare")
  {
    std::array<std::vector<std::string>, 2> files{};
    double compare_tol = 1;
    for (int i = 1, s = 0; i < argc; ++i)
    {
      if (s == 0 && std::strcmp(argv[i], "compare") == 0)
      {
	s = 1;
	continue;
      }
      else if (std::strcmp(argv[i], "-tol") == 0 && i + 1 < argc)
      {
	compare_tol = std ::atof(argv[i + 1]);
	++i;
	continue;
      }
      files[s].push_back(argv[i]);
    }
    if (files[0].size() == 0 || files[1].size() == 0)
    {
      return usage(argc, argv);
    }

    compareGenprop4(files[0], files[1], compare_tol);
  }
  else
  {
    std::cerr << argv[0] << ": unsupported operation= " << op << std::endl;
    return -1;
  }

  return 0;
}

//! Handle all the work
template <typename K, typename V>
int discoWorkHandler(int argc, char** argv)
{
  std::string dbase = argv[1];
  std::string op = argv[2];
  double compare_tol = 1;
  if (argc >= 6 && std::strcmp(argv[4], "-tol") == 0)
    compare_tol = std::atof(argv[5]);

  // A simplified version of handler
  if (op == "meta")
  {
    printMeta(dbase);
  }
  else if (op == "keys")
  {
    printDiscoKeys(dbase);
  }
  else if (op == "keysxml")
  {
    if (argc != 4)
    {
      return usage(argc, argv);
    }

    printDiscoKeysXML(argv[3], dbase);
  }
  else if (op == "get")
  {
    if (argc != 4)
    {
      return usage(argc, argv);
    }

    printDisco(argv[3], dbase);
  }
  else if (op == "compare")
  {
    if (argc < 4)
    {
      return usage(argc, argv);
    }

    std::string dbase1 = argv[3];
    compareDisco(dbase, dbase1, compare_tol);
  }
  else
  {
    std::cerr << argv[0] << ": unsupported operation= " << op << std::endl;
    return -1;
  }

  return 0;
}

//! Handle all the work
template <typename K, typename V>
int filehashWorkHandler(int argc, char** argv)
{
  std::string dbase = argv[1];
  std::string op = argv[2];
  double compare_tol = 1, ignore_tol = 0;
  if (argc >= 6 && std::strcmp(argv[4], "-tol") == 0)
  {
    compare_tol = std::atof(argv[5]);
    if (argc >= 7)
      ignore_tol = std::atof(argv[6]);
  }

  // A simplified version of handler
  if (op == "meta")
  {
    printMeta(dbase);
  }
  else if (op == "keys")
  {
    // NOTE: fix this with the new meson storage
    printFilehashKeys<K>(dbase);
  }
  else if (op == "keysxml")
  {
    if (argc != 4)
    {
      return usage(argc, argv);
    }

    // NOTE: fix this with the new meson storage
    printFilehashKeysXML<K>(argv[3], dbase);
  }
  else
  {
    std::cerr << argv[0] << ": unsupported operation= " << op << std::endl;
    return -1;
  }

  return 0;
}


int main(int argc, char** argv)
{
  // Parse the arguments
  if (argc <= 2)
  {
    return usage(argc, argv);
  }

  // Call the appropriate handler
  int ret;
  try
  {
    if (MODInspect::isMOD(argv[1]))
      return MODInspect::run(argc, argv);

    // Read the dbtype from the metadata of the first file
    std::string dbtype = getDBType(argv[1]);

    std::map<std::string, std::function<int(int, char**)>> m{
      {"hadron1Pt", workHandler<KeyHadron1PtCorr_t>},
      {"hadron2Pt", workHandler<KeyHadron2PtCorr_t>},
      {"hadron3Pt", workHandler<KeyHadron3PtCorr_t>},
      {"hadronNPartNPtCorr", workHandler<KeyHadronNPartNPtCorr_t>},
      {"hadronSUNNPartNPtCorr", workHandler<KeyHadronSUNClassNPartNPtCorr_t>},
      {"hadronSUNClassNPartNPtCorr", workHandler<KeyHadronSUNClassNPartNPtCorr_t>},
      {"baryonElemOp",
       baryonWorkHandler<KeyBaryonElementalOperator_t, ValBaryonElementalOperator_t>},
      {"baryonElemOpSuperb",
       baryonWorkHandler<KeyBaryonElementalOperator_t, ValBaryonElementalOperator_t>},
      {"mesonElemOp", mesonWorkHandler<KeyMesonElementalOperator_t, ValMesonElementalOperator_t>},
      {"propElemOp", propWorkHandler<KeyProp4ElementalOperator_t, ValProp4ElementalOperator_t>},
      {"unsmearedMesonElemOp",
       genprop4WorkHandler<KeyGenProp4ElementalOperator_t, ValGenProp4ElementalOperator_t>},
      {"genprop4ElemOp",
       genprop4WorkHandler<KeyGenProp4ElementalOperator_t, ValGenProp4ElementalOperator_t>},
      {"genprop5ElemOp",
       genprop4WorkHandler<KeyGenProp4ElementalOperator_t, ValGenProp4ElementalOperator_t>},
      {"DiscoBlocks", discoWorkHandler<KeyTwoQuarkDiscoOperator_t, ValTwoQuarkDiscoOperator_t>},
      {"tetraQuarkElemOp",
       filehashWorkHandler<KeyTetraQuarkElementalOperator_t, ValTetraQuarkElementalOperator_t>},
      {"hadronClassNPartNPtConnGraph",
       filehashWorkHandler<KeyHadronClassNPartNPtConnGraph_t, UserData>}};
    if (m.count(dbtype) == 0)
      throw std::runtime_error(std::string("Unsupported database type: ") + dbtype);
    ret = m.at(dbtype)(argc, argv);
  } catch (const std::string& e)
  {
    std::cerr << argv[0] << ": error message: " << e << std::endl;
    exit(1);
  } catch (const char* e)
  {
    std::cerr << argv[0] << ": error message: " << e << std::endl;
    exit(1);
  } catch (const std::exception& e)
  {
    std::cerr << argv[0] << ": error message: " << e.what() << std::endl;
    exit(1);
  } catch (...)
  {
    std::cerr << argv[0] << ": caught generic exception\n";
    exit(1);
  }

  return ret;
}
