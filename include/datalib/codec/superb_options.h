// -*- C++ -*-
/*! \file                                                                    
 * \brief Alternative self-contained XML parser and processing
 */

#ifndef DATALIB_INCLUDE_SUPERB_OPTIONS
#define DATALIB_INCLUDE_SUPERB_OPTIONS

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace SB
{

  namespace detail
  {
    /// Return the previous lines up this one
    /// \param file: content
    /// \param char_num: character index of the last line to print
    /// \param num_prev_lines: maximum number of previous lines to print
    /// \param prefix: string to print previous to each line

    inline std::string get_prev_lines(const std::string& file, std::size_t char_num,
				      unsigned int num_prev_lines = 0,
				      const std::string prefix = "")
    {
      if (char_num > file.size())
	throw std::runtime_error("Erroneous character number");
      std::size_t line_num = std::count(file.begin(), file.begin() + char_num, '\n') + 1;
      auto p = file.begin();
      std::string r;
      for (unsigned int l = 1; l <= line_num && p != file.end(); ++l)
      {
	auto p1 = std::find(p, file.end(), '\n');
	if (l + num_prev_lines >= line_num)
	  r += std::string(p, p1) + std::string("\n");
	p = p1 + (p1 != file.end() ? 1 : 0);
      }
      return r;
    }

    /// Return the given string with lowercase characters
    /// \param s: input string

    inline std::string to_lower(const std::string& s)
    {

      std::string valueLower = s;
      std::transform(valueLower.begin(), valueLower.end(), valueLower.begin(),
		     [](unsigned char c) { return std::tolower(c); });
      return valueLower;
    }
  }

  /// Auxiliary class for initialize Maybe<T> with no value
  struct None {
  };

  template <typename T, bool B = std::is_reference<T>::value>
  struct Maybe;

  /// is_maybe<T>::value returns whether T is of type Maybe or None
  template <typename T>
  struct is_maybe {
    static constexpr bool value = false;
  };

  template <>
  struct is_maybe<None> {
    static constexpr bool value = true;
  };

  template <typename T>
  struct is_maybe<Maybe<T>> {
    static constexpr bool value = true;
  };

  /// Class for optional values
  template <typename T>
  struct Maybe<T, false> {
    /// Whether the value is set
    bool has_value;

    /// The value
    T value;

    /// Constructor without a value
    Maybe() : has_value{false}, value{}
    {
    }

    /// Constructor without a value
    Maybe(None) : Maybe()
    {
    }

    /// Constructor with a value
    template <typename Q,
	      typename std::enable_if<(!is_maybe<Q>::value && std::is_convertible<Q, T>::value),
				      bool>::type = true>
    Maybe(const Q& t) : has_value{true}, value{T(t)}
    {
    }

    /// Return whether it has been initialized with a value
    bool hasSome() const
    {
      return has_value;
    }

    /// Return whether it has been initialized with a value
    explicit operator bool() const noexcept
    {
      return has_value;
    }

    /// Return the value if it has been initialized with some
    T getSome() const
    {
      if (has_value)
	return value;
      throw std::runtime_error("Maybe::getSome: value isn't set");
    }

    /// Return the value if it has been initialized with some; otherwise return `def`
    T getSome(T def) const
    {
      if (has_value)
	return getSome();
      else
	return def;
    }
  };

  /// Class for optional values
  template <typename T>
  struct Maybe<T, true> {
    /// Whether the value is set
    bool has_value;

    /// The value type: change references by pointers
    using Tvalue = typename std::remove_reference<T>::type*;

    /// The value
    Tvalue value;

    /// Constructor without a value
    Maybe() : has_value{false}, value{}
    {
    }

    /// Constructor without a value
    Maybe(None) : Maybe()
    {
    }

    /// Constructor with a value
    Maybe(const T& t) : has_value{true}, value{&t}
    {
    }

    /// Return whether it has been initialized with a value
    bool hasSome() const
    {
      return has_value;
    }

    /// Return whether it has been initialized with a value
    explicit operator bool() const noexcept
    {
      return has_value;
    }

    /// Return the value if it has been initialized with some
    T getSome() const
    {
      if (has_value)
	return *value;
      throw std::runtime_error("Maybe::getSome: value isn't set");
    }

    /// Return the value if it has been initialized with some; otherwise return `def`
    T getSome(T def) const
    {
      if (has_value)
	return getSome();
      else
	return def;
    }
  };

  /// Initialize Maybe<T> without value
  constexpr None none = None{};

  /// Class for storing options
  struct Option {
    /// Get track of the path of this option on a set of options
    std::string prefix;
    /// Content of the file where the option comes from
    std::shared_ptr<std::string> file;
    /// First line number in `file` associated to the option
    std::size_t char_num;
    /// Whether this option has been checked
    mutable bool visited;

    Option() : char_num(0), visited(false)
    {
    }

  protected:
    Option(std::shared_ptr<std::string> file, std::size_t char_num)
      : file(file), char_num(char_num), visited(false)
    {
    }

    /// Copy `prefix`, `file` and `char_num` from a given option
    /// \param op: option to copy the info

    void copyFileInfo(const Option& op)
    {
      prefix = op.prefix;
      file = op.file;
      char_num = op.char_num;
    }

  public:
    /// Throw an error
    /// \param s: error message

    void throw_error(const std::string& err_msg) const
    {
      if (file)
      {
	std::size_t line_num = std::count(file->begin(), file->begin() + char_num, '\n') + 1;
	throw std::runtime_error(std::string("Error at prefix `") + prefix +	    //
				 "'(l. " + std::to_string(line_num) + "): " +	    //
				 err_msg + "\n" +				    //
				 detail::get_prev_lines(*file, char_num, 5, "| ")); //
      }
      else
      {
	throw std::runtime_error("Error at prefix `" + prefix + ": " + err_msg);
      }
    }

    /// Type of the option
    enum Type { None, String, Double, Vector, Dictionary };

    /// Return the type of this option
    virtual Type getType() const
    {
      throw_error("getType: invalid object, it's abstract");
      throw std::exception{}; // silent no return warning
    }

    /// Return if this options isn't None
    explicit operator bool() const noexcept
    {
      return getType() != None;
    }

    /// Return the string content of the option
    virtual std::string getString() const
    {
      throw_error("expected the value to be a string");
      throw std::exception{}; // silent no return warning
    }

    /// Return the double content of the option
    virtual double getDouble() const
    {
      throw_error("expected the value to be a double");
      throw std::exception{}; // silent no return warning
    }

    /// Return the integer content of the option
    virtual int getInt() const
    {
      throw_error("expected the value to be an integer");
      throw std::exception{}; // silent no return warning
    }

    /// Return the unsigned integer content of the option
    virtual unsigned int getUInt() const
    {
      throw_error("expected the value to be an unsigned integer");
      throw std::exception{}; // silent no return warning
    }

    /// Return the unsigned integer content of the option
    virtual bool getBool() const
    {
      throw_error("expected the value to be a boolean");
      throw std::exception{}; // silent no return warning
    }

    /// Return the vector content of the vector
    virtual std::vector<std::shared_ptr<Option>> getVector() const
    {
      throw_error("expected the value to be a vector");
      throw std::exception{}; // silent no return warning
    }

    /// Return the vector content of the vector
    virtual std::vector<std::shared_ptr<Option>>& getVector()
    {
      throw_error("expected the value to be a vector");
      throw std::exception{}; // silent no return warning
    }

    /// Return the map content of the vector
    virtual const std::map<std::string, std::shared_ptr<Option>>& getDictionary() const
    {
      throw_error("expected the value to be a dictionary");
      throw std::exception{}; // silent no return warning
    }

    /// Return the map content of the vector
    virtual std::map<std::string, std::shared_ptr<Option>>& getDictionary()
    {
      throw_error("expected the value to be a dictionary");
      throw std::exception{}; // silent no return warning
    }

    /// Return the option content on a path
    const Option& getValue(const std::string& path, Maybe<const Option&> defaultValue = none,
			   Maybe<Type> expectedType = none, Maybe<const Option&> fromOption = none,
			   Maybe<std::string> originalPath = none) const
    {
      // If fromOption is none, set this node
      Maybe<const Option&> fromOption_{fromOption.getSome(*this)};

      // If originalPath is none, set the given path
      Maybe<std::string> originalPath_{originalPath.getSome(path)};

      // Construct a nice error message
      const std::string errorHeader = "Error in searching for option `" + originalPath_.getSome() +
				      "' from option at `" + fromOption_.getSome().prefix + "': ";

      // If the path is empty or ask for the root node, just return this node
      if (path.size() == 0 || path == std::string("/"))
      {
	if (expectedType && getType() != expectedType.getSome())
	  throw std::runtime_error(errorHeader + "Expected another type");
	return *this;
      }

      // If that path has a tag but the current option isn't a dictionary, either return
      // the default value or throw an error
      if (getType() != Dictionary)
      {
	if (defaultValue)
	  return defaultValue.getSome();
	throw std::runtime_error(errorHeader + "the element `" + path + "' is not a dictionary");
      }

      // If path starts with `/`, consume it and continue
      if (path[0] == '/')
	return getValue(std::string(path.begin() + 1, path.end()), defaultValue, expectedType,
			fromOption_, originalPath_);

      // Find the name of the tag
      auto p = std::find(path.begin(), path.end(), '/');
      std::string fieldName = std::string(path.begin(), p);

      // If the tag isn't under the current node, either return the default value or throw an error
      auto m = getDictionary();
      if (m.count(fieldName) == 0)
      {
	if (defaultValue.hasSome())
	  return defaultValue.getSome();
	throw std::runtime_error(errorHeader + "the tag `" + fieldName + "' was not found");
      }

      // Otherwise, consume the tag name, and continue
      return m[fieldName]->getValue(std::string(p, path.end()), defaultValue, expectedType,
				    fromOption_, originalPath_);
    }

    /// Return the option content on a path
    const Option& getValue(const std::string& path, Type expectedType) const
    {
      return getValue(path, none, expectedType);
    }

    /// Return the option content on a path
    Maybe<const Option&> getValueMaybe(const std::string& path,
				       Maybe<Type> expectedType = none) const
    {
      struct AuxNone : Option {
	Type getType() const override
	{
	  return None;
	}
      } defaultOp;
      const Option& op = getValue(path, Maybe<const Option&>{defaultOp}, expectedType);
      if (&op == &defaultOp)
	return none;
      return op;
    }

    void setPrefix(const std::string& thisPrefix = "")
    { // Set the prefix of this option
      prefix = thisPrefix;

      switch (getType())
      {
      case None:
      case String:
      case Double:
      {
	// Do nothing
	break;
      }

      case Vector:
      {
	unsigned int i = 0;
	for (auto& it : getVector())
	  it->setPrefix(thisPrefix + "[" + std::to_string(i++) + "]/");
	break;
      }

      case Dictionary:
      {
	for (auto& it : getDictionary())
	  it.second->setPrefix(thisPrefix + it.first + "/");
	break;
      }
      }
    }
  };

  /// Storing a string as the value of an option
  struct NoneOption : public Option {
    NoneOption()
    {
    }
    NoneOption(std::shared_ptr<std::string> file, std::size_t char_num) : Option{file, char_num}
    {
    }
    Type getType() const override
    {
      return None;
    }
  };

  /// Storing a string as the value of an option
  struct StringOption : public Option {
    std::string value;
    StringOption(const std::string& s, std::shared_ptr<std::string> file, std::size_t char_num)
      : Option{file, char_num}, value(s)
    {
    }
    StringOption(const std::string& s, Maybe<const Option&> op = none) : value(s)
    {
      if (op)
	copyFileInfo(op.getSome());
    }

    Type getType() const override
    {
      return String;
    }
    std::string getString() const override
    {
      visited = true;
      return value;
    }
    int getInt() const override
    {
      visited = true;
      try
      {
	return std::stoi(value);
      } catch (...)
      {
	throw_error("expected the value to be an integer");
      }
      throw std::exception{}; // silent no return warning
    }
    unsigned int getUInt() const override
    {
      visited = true;
      try
      {
	return std::stoul(value);
      } catch (...)
      {
	throw_error("expected the value to be an unsigned integer");
      }
      throw std::exception{}; // silent no return warning
    }
    double getDouble() const override
    {
      visited = true;
      try
      {
	return std::stod(value);
      } catch (...)
      {
	throw_error("expected the value to be a double");
      }
      throw std::exception{}; // silent no return warning
    }
    bool getBool() const override
    {
      visited = true;
      std::string valueLower = detail::to_lower(value);
      if (valueLower == std::string("true"))
	return true;
      if (valueLower == std::string("false"))
	return false;
      throw_error("expected the value to be boolean, either `true' or `false'");
      throw std::exception{}; // silent no return warning
    }
    std::vector<std::shared_ptr<Option>> getVector() const override
    {
      visited = true;
      std::vector<std::shared_ptr<Option>> v;
      for (auto i = value.begin(), w = value.begin(); i != value.end(); ++i)
      {
	if (std::isspace(static_cast<unsigned char>(*i)) || i + 1 == value.end())
	{
	  if (!std::isspace(static_cast<unsigned char>(*w)))
	    v.push_back(std::make_shared<StringOption>(
	      std::string(w, i + 1 == value.end() ? i + 1 : i), *this));
	  w = i + 1;
	}
      }
      return v;
    }
  };

  /// Storing a double as the value of an option
  struct DoubleOption : public Option {
    double value;
    DoubleOption(double d, std::shared_ptr<std::string> file, std::size_t char_num)
      : Option{file, char_num}, value(d)
    {
    }
    DoubleOption(double d, Maybe<const Option&> op = none) : value(d)
    {
      if (op)
	copyFileInfo(op.getSome());
    }

    Type getType() const override
    {
      return Double;
    }
    double getDouble() const override
    {
      visited = true;
      return value;
    }
    int getInt() const override
    {
      visited = true;
      return (int)std::round(value);
    }
    unsigned int getUInt() const override
    {
      visited = true;
      int r = getInt();
      if (r < 0)
	throw std::runtime_error("Error at prefix `" + prefix +
				 "': expected the value to be an unsigned integer");
      return (unsigned int)r;
    }
    bool getBool() const override
    {
      visited = true;
      return std::fabs(value) != 0;
    }
  };

  /// Storing a vector as the value of an option
  struct VectorOption : public Option {
    std::vector<std::shared_ptr<Option>> value;
    VectorOption(const Option& op)
    {
      copyFileInfo(op);
    }

    Type getType() const override
    {
      return Vector;
    }
    std::vector<std::shared_ptr<Option>> getVector() const override
    {
      visited = true;
      return value;
    }
    std::vector<std::shared_ptr<Option>>& getVector() override
    {
      visited = true;
      return value;
    }
  };

  /// Storing a vector as the value of an option
  struct DictionaryOption : public Option {
    std::map<std::string, std::shared_ptr<Option>> value;
    DictionaryOption(const Option& op)
    {
      copyFileInfo(op);
    }

    Type getType() const override
    {
      return Dictionary;
    }
    const std::map<std::string, std::shared_ptr<Option>>& getDictionary() const override
    {
      visited = true;
      return value;
    }
    std::map<std::string, std::shared_ptr<Option>>& getDictionary() override
    {
      visited = true;
      return value;
    }
  };

/// Check using C++ 20, otherwise it gets weird errors
#if __cplusplus <= 201703L
#  define STR_HELPER(x) #x
#  define STR(x) STR_HELPER(x)
#  pragma message "current version of C++: " STR(__cplusplus)
#  error Use c++20
#endif

  /// Auxiliary class to represent a string value as a template argument

  template <size_t N>
  struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N])
    {
      std::copy_n(str, N, value);
    }

    char value[N];
  };

  /// Literal for strings
  /// Example:
  ///   template<StringLiteral> void print() {}
  ///   print<"hi"_sl>();

  template <StringLiteral s>
  constexpr auto operator""_sl()
  {
    return s;
  }

  /// Auxiliary class to introspect a class member pointer, &A::member.
  /// member_pointer<&A::member>::class_t returns the class, A, and
  /// member_pointer<&A::member>::type returns the type of A::member.

  template <auto>
  struct member_pointer {
  };

  template <typename T, typename Class, T Class::* value>
  struct member_pointer<value> {
    using class_t = Class;
    using type = T;
  };

  /// Auxiliary value used by `struct_member'
  /// NOTE: it shouldn't parse as XML

  constexpr auto NoDefault = ">>no~default<<"_sl;

  /// Class to annotate the serialization/deserialization of a class
  /// \tparam value: pointer to class member
  /// \tparam name: associated XML tag name
  /// \tparam treat_type: (optional) either None or an alternative type
  /// \tparam default_value: (optional) either NoDefault or an XML string to parse
  ///         when the tag is missing
  /// \tparam active: (optional) whether the member is read/written
  ///
  /// Example:
  ///   struct S {
  ///     int a;
  ///     my_vector<int> b;
  ///     std::array<int, 2> c;
  ///
  ///     using schema = std::tuple<
  ///       struct_member<&S::a, "a"_sl>,
  ///       struct_member<&S::b, "b"_sl, std::vector<int>>,
  ///       struct_member<&S::c, "c"_sl, None, "0 0 1"_sl>
  ///       >;
  ///   };

  template <auto value, StringLiteral name, typename treat_type = None,
	    StringLiteral default_value = NoDefault, bool active = true>
  struct struct_member {
    using class_t = member_pointer<value>::class_t;
    using actual_type = member_pointer<value>::type;
    using type = std::conditional<!std::is_same<treat_type, None>::value, treat_type,
				  typename member_pointer<value>::type>::type;
    static constexpr auto member_value = value;
    static constexpr auto member_label = name;
    static constexpr auto member_default_value = default_value;
    static constexpr auto member_active = active;
  };

  template <typename T>
  T getOption(const Option& ops, const std::string& path, Maybe<T> defaultValue = none);

  template <typename T>
  T getOptionFast(const std::string& s, const std::string& tag);

  template <typename T>
  std::string getXML(const T& t, const std::string& tag);

  /// Return whether T has a `schema' type
  template <typename T, typename = void>
  struct has_schema : std::false_type {
  };

  template <typename T>
  struct has_schema<T, std::void_t<typename T::schema>> : std::true_type {
  };

  namespace detail
  {
    /// is_complex<T>::value return whether T is std::complex

    template <typename T>
    struct is_complex : std::false_type {
    };

    template <typename T>
    struct is_complex<std::complex<T>> : std::true_type {
    };

    /// is_tuple<T>::value return whether T is std::tuple

    template <typename T>
    struct is_tuple : std::false_type {
    };

    template <typename... T>
    struct is_tuple<std::tuple<T...>> : std::true_type {
    };

    template <typename T, typename Q>
    struct is_tuple<std::pair<T, Q>> : std::true_type {
    };

    /// is_vector<T>::value return whether T is std::vector

    template <typename T>
    struct is_vector : std::false_type {
    };

    template <typename T>
    struct is_vector<std::vector<T>> : std::true_type {
    };

    /// is_array<T>::value return whether T is std::array

    template <typename T>
    struct is_array : std::false_type {
    };

    template <typename T, std::size_t N>
    struct is_array<std::array<T, N>> : std::true_type {
    };

    /// is_vector_or_array<T>::value return whether T is std::vector or std::array

    template <typename T>
    struct is_vector_or_array {
      static constexpr bool value = is_vector<T>::value | is_array<T>::value;
    };

    template <typename T>
    struct is_set : std::false_type {};

    template <typename Value, typename Compare, typename Allocator>
    struct is_set<std::set<Value, Compare, Allocator>> : std::true_type {};

    /// is_map<T>::value return whether T is std::map

    template <typename T>
    struct is_map : std::false_type {
    };

    template <typename A, typename B, typename C, typename D>
    struct is_map<std::map<A, B, C, D>> : std::true_type {
    };

    template <typename A, typename B, typename C, typename D, typename E>
    struct is_map<std::unordered_map<A, B, C, D, E>> : std::true_type {
    };

    /// is_std_hashable<T>::value returns whether std::hash can be use with T

    template <typename T, typename = std::void_t<>>
    struct is_std_hashable : std::false_type {
    };

    template <typename T>
    struct is_std_hashable<T,
			   std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>>
      : std::true_type {
    };

    // Convert to another type considering std:vector

    template <typename T, typename Q,
	      typename std::enable_if<std::is_convertible<Q, T>::value, bool>::type = true>
    T static_cast_extra(const Q& q)
    {
      return static_cast<T>(q);
    }

    template <typename T, typename Q,
	      typename std::enable_if<is_vector<T>::value &&
					std::is_convertible<Q, typename T::value_type>::value,
				      bool>::type = true>
    T static_cast_extra(const std::vector<Q>& q)
    {
      T t;
      t.reserve(q.size());
      for (const auto& it : q)
	t.push_back(it);
      return t;
    }

    /// Auxiliary class to fill the members of a struct

    template <typename T>
    struct GetExtraOption {

      /// Return a struct filled with the values of tags, given a path
      /// \param ops: options into look for
      /// \param path: option path
      /// \param defaultValue: return value if the options isn't specified

      static T getOption(const Option& ops, const std::string& path, Maybe<T> defaultValue = none)
      {
	NoneOption defaultOp{};
	const Option& valueOp =
	  ops.getValue(path, defaultValue ? Maybe<const Option&>{defaultOp} : none);
	if (!valueOp)
	  return defaultValue.getSome();
	const auto& d = valueOp.getDictionary();
	T r;
	update_option<0>(r, d, ops);
	return r;
      }

    private:
      template <std::size_t item,
		typename std::enable_if<(item >= std::tuple_size<typename T::schema>::value),
					bool>::type = true>
      static void update_option(T&, const std::map<std::string, std::shared_ptr<Option>>&,
				const Option&)
      {
      }

      template <std::size_t item,
		typename std::enable_if<(item < std::tuple_size<typename T::schema>::value),
					bool>::type = true>
      static void update_option(T& r, const std::map<std::string, std::shared_ptr<Option>>& dict,
				const Option& ops)
      {
	using member = std::tuple_element<item, typename T::schema>::type;
	if constexpr (member::member_active)
	{
	  const auto& label =
	    std::string(member::member_label.value, sizeof(member::member_label.value) - 1);
	  constexpr auto default_value = std::string_view(
	    member::member_default_value.value, sizeof(member::member_default_value.value) - 1);
	  constexpr auto no_default_value =
	    std::string_view(NoDefault.value, sizeof(NoDefault.value) - 1);
	  if (dict.count(label) == 1)
	  {
	    r.*member::member_value = static_cast_extra<typename member::actual_type>(
	      SB::getOption<typename member::type>(*dict.at(label), ""));
	  }
	  else if constexpr (default_value != no_default_value)
	  {
	    const auto& value = SB::getOptionFast<typename member::type>(
	      getXML(std::string(default_value), label), label);
	    r.*member::member_value = static_cast_extra<typename member::actual_type>(value);
	  }
	  else
	  {
	    ops.throw_error(std::string("missing option: ") + label);
	  }
	}
	update_option<item + 1>(r, dict, ops);
      }
    };
  }

  /// \param ops: options into look for
  /// \param path: option path
  /// \param defaultValue: return value if the options isn't specified

  template <typename T>
  T getOption(const Option& ops, const std::string& path, Maybe<T> defaultValue)
  {
    if constexpr (has_schema<T>::value)
    {
      return detail::GetExtraOption<T>::getOption(ops, path, defaultValue);
    }
    else if constexpr (std::is_same<T, std::string>::value)
    {
      StringOption defaultOp{defaultValue.getSome("")};
      return ops.getValue(path, defaultValue ? Maybe<const Option&>{defaultOp} : none).getString();
    }
    else if constexpr (std::is_same<T, double>::value)
    {
      DoubleOption defaultOp{defaultValue.getSome(0.0)};
      return ops.getValue(path, defaultValue ? Maybe<const Option&>{defaultOp} : none).getDouble();
    }
    else if constexpr (std::is_same<T, int>::value)
    {
      DoubleOption defaultOp{(double)defaultValue.getSome(0)};
      return ops.getValue(path, defaultValue ? Maybe<const Option&>{defaultOp} : none).getInt();
    }
    else if constexpr (std::is_same<T, unsigned int>::value)
    {
      DoubleOption defaultOp{(double)defaultValue.getSome(0)};
      return ops.getValue(path, defaultValue ? Maybe<const Option&>{defaultOp} : none).getUInt();
    }
    else if constexpr (std::is_same<T, bool>::value)
    {
      StringOption defaultOp{defaultValue.getSome(false) ? "true" : "false"};
      return ops.getValue(path, defaultValue ? Maybe<const Option&>{defaultOp} : none).getBool();
    }
    else if constexpr (detail::is_vector<T>::value)
    {
      NoneOption defaultOp{};
      const Option& valueOp =
	ops.getValue(path, defaultValue ? Maybe<const Option&>{defaultOp} : none);
      if (!valueOp)
	return defaultValue.getSome();
      const auto& v = valueOp.getVector();
      T r;
      r.reserve(v.size());
      for (const auto& op : v)
	r.push_back(SB::getOption<typename T::value_type>(*op, ""));
      return r;
    }
    else if constexpr (detail::is_array<T>::value)
    {
      NoneOption defaultOp{};
      const Option& valueOp =
	ops.getValue(path, defaultValue ? Maybe<const Option&>{defaultOp} : none);
      if (!valueOp)
	return defaultValue.getSome();
      const auto& v = valueOp.getVector();
      const auto N = std::tuple_size<T>::value;
      if (v.size() != N)
	valueOp.throw_error("invalid number of elements: expected " + std::to_string(N));
      T r = {};
      unsigned int i = 0;
      for (const auto& op : v)
	r[i++] = SB::getOption<typename T::value_type>(*op, "");
      return r;
    }
    else if constexpr (detail::is_map<T>::value)
    {
      NoneOption defaultOp{};
      const Option& valueOp =
	ops.getValue(path, defaultValue ? Maybe<const Option&>{defaultOp} : none);
      if (!valueOp)
	return defaultValue.getSome();
      const auto& v = valueOp.getVector();
      T r;
      if constexpr (requires(const T& t) { t.reserve(0); })
	r.reserve(v.size());
      using Key = typename T::key_type;
      using Value = typename T::mapped_type;
      for (const auto& it : v)
      {
	const auto& d = it->getDictionary();
	r.insert({SB::getOption<Key>(*d.at("Key"), ""), SB::getOption<Value>(*d.at("Val"), "")});
      }
      return r;
    }
    else
    {
      static_assert(sizeof(T) == 0, "unsupported type");
    }
  }

  /// Return an enum option given a path
  /// \param ops: options into look for
  /// \param path: option path
  /// \param defaultValue: return value if the options isn't specified

  template <typename Enum>
  Enum getOption(const Option& ops, const std::string& path, const std::map<std::string, Enum>& m,
		 Maybe<Enum> defaultValue = none)
  {
    // Transform the map entries to lowercase
    std::map<std::string, Enum> m0;
    for (const auto& it : m)
    {
      std::string k = detail::to_lower(it.first);
      if (m0.count(k) == 1)
	throw std::runtime_error("getOption: invalid map, case sensitive keys");
      m0[k] = it.second;
    }

    // Get value from the options
    const std::string defaultStr = "default";
    StringOption defaultOp{defaultStr};
    std::string value = detail::to_lower(
      ops.getValue(path, defaultValue ? Maybe<const Option&>{defaultOp} : none).getString());

    // If no option was given or it's the default, return the default value if a default value is given
    if (defaultValue && value == defaultStr)
      return defaultValue.getSome();

    // Check that the value is in the map
    if (m0.count(value) == 0)
    {
      std::string availableOption = defaultValue ? defaultStr : std::string{};
      for (const auto& it : m0)
	availableOption += std::string(" ") + it.first;
      ops.getValue(path).throw_error("unsupported value `" + value +
				     "'; supported values: " + availableOption);
    }

    // Return the enum associated to the value
    return m0.at(value);
  }

  /// Return a dictionary option
  /// \param ops: options into look for
  /// \param path: option path

  inline const Option& getOptions(const Option& ops, const std::string& path)
  {
    return ops.getValue(path, Option::Dictionary);
  }

  /// Return a dictionary option
  /// \param ops: options into look for
  /// \param path: option path

  template <typename T>
  inline Maybe<T> getOptionMaybe(const Option& ops, const std::string& path)
  {
    if (!ops.getValueMaybe(path))
      return none;
    return getOption<T>(ops, path);
  }

  using Options = Option;

  /// Returns options from XML
  /// \param s: text

  inline std::shared_ptr<Option> getOptionsFromXML(const std::string& s)
  {

    auto throw_error = [&](std::string::const_iterator i, std::string err_msg) {
      std::size_t line_number = std::count(s.begin(), i, '\n') + 1;
      throw std::runtime_error("Error parsing XML at line " + std::to_string(line_number) + ": " +
			       err_msg);
    };

    auto istag = [](unsigned char c) {
      return std::isalnum(c) || c == '_' || c == '-' || c == '.';
    };

    std::vector<std::shared_ptr<Option>> ops; // options of the open tags
    std::vector<std::string> tags;	      // names of the open tags
    std::vector<Option::Type> types;	      // type of the open tags

    // Put the root element
    tags.push_back("/");
    types.push_back(Option::None);
    std::shared_ptr<std::string> file = std::make_shared<std::string>(s);
    ops.push_back(std::make_shared<NoneOption>(file, 0));

    // Parse the string
    for (auto i = s.begin(); i != s.end(); ++i)
    {
      // Ignore blanks
      if (std::isspace(static_cast<unsigned char>(*i)))
	continue;

      // Detect a token
      if (*i == '<')
      {
	if (i + 1 == s.end())
	{
	  throw_error(i, "Broken tag: `<' without closing `>'");
	}
	else if (*(i + 1) == '?')
	{
	  // Ignore <? ... ?>
	  const std::string e("?>");
	  auto p = std::search(i + 2, s.end(), e.begin(), e.end());
	  if (p == s.end())
	    throw_error(i, "Broken tag: `<?' without closing `?>'");
	  i = p + (e.size() - 1);
	}
	else if (*(i + 1) == '!' && i + 2 != s.end() && i + 3 != s.end() && *(i + 2) == '-' &&
		 *(i + 3) == '-')
	{
	  // Ignore <!-- ... -->
	  const std::string e("-->");
	  auto p = std::search(i, s.end(), e.begin(), e.end());
	  if (p == s.end())
	    throw_error(i, "Broken tag: `<!--' without closing `-->'");
	  i = p + (e.size() - 1);
	}
	else if (*(i + 1) != '/')
	{
	  // This is an open tag
	  auto i0 = i + 1;
	  for (; i0 != s.end() && istag(*i0); ++i0)
	    ;
	  std::string tag(i + 1, i0);
	  if (tag.size() == 0)
	    throw_error(i, "Error: empty tag");
	  auto i1 = std::find(i0, s.end(), '>');
	  if (i1 == s.end())
	    throw_error(i, "Broken tag: `<' without `>'");
	  std::string e("/>");
	  bool closing_tag = std::search(i0, i1, e.begin(), e.end()) != i1;

	  // Set the type of the current option
	  bool is_vector = tag == std::string("elem");
	  bool is_comment = ((tag == std::string("comment")) | (tag == std::string("annotation")));
	  if (!is_comment)
	  {
	    if (is_vector && types.back() != Option::None && types.back() != Option::Vector)
	      throw_error(i, "Mixing <elem> tags with other tags is not supported");
	    if (!is_vector && types.back() != Option::None && types.back() != Option::Dictionary)
	      throw_error(i, "Mixing <elem> tags with other tags is not supported");
	    if (types.back() == Option::None)
	    {
	      if (is_vector)
		ops.back() = std::make_shared<VectorOption>(*ops.back());
	      else
		ops.back() = std::make_shared<DictionaryOption>(*ops.back());
	      types.back() = (is_vector ? Option::Vector : Option::Dictionary);
	    }
	    else if (!is_vector && ops.back()->getDictionary().count(tag) != 0)
	    {
	      throw_error(i,
			  "Unsupported more than one tag with the same name under the same node");
	    }
	  }

	  std::shared_ptr<Option> new_op = std::make_shared<NoneOption>(
	    file, (std::size_t)(i - s.begin())); // new option to be filled
	  if (!closing_tag)
	  {
	    // Open tag
	    tags.push_back(tag);
	    types.push_back(Option::None);
	    ops.push_back(new_op);
	  }
	  else if (!is_comment)
	  {
	    // Open and close tag
	    std::shared_ptr<Option> empty_op = std::make_shared<StringOption>("", *new_op);
	    if (is_vector)
	      ops.back()->getVector().push_back(empty_op);
	    else
	      ops.back()->getDictionary().insert({tag, empty_op});
	  }
	  i = i1;
	}
	else if (*(i + 1) == '/')
	{
	  // This is a close tag
	  auto i0 = i + 2;
	  for (; i0 != s.end() && istag(*i0); ++i0)
	    ;
	  std::string tag(i + 2, i0);
	  if (tag.size() == 0)
	    throw_error(i, "Error: empty tag");
	  auto i1 = std::find(i0, s.end(), '>');
	  if (i1 == s.end())
	    throw_error(i, "Broken tag: `<' without `>'");
	  std::string e("/>");
	  bool closing_tag = std::search(i0, i1, e.begin(), e.end()) != i1;
	  if (closing_tag)
	    throw_error(i, "Malformed tag `</' ... `/>'");
	  if (tag != tags.back())
	    throw_error(i, "Unmatched closing tag </" + tag + ">");

	  // Consume the option on top
	  types.pop_back();
	  std::shared_ptr<Option> op = ops.back();
	  ops.pop_back();
	  tags.pop_back();
	  bool is_comment = ((tag == std::string("comment")) | (tag == std::string("annotation")));
	  if (!is_comment)
	  {
	    if (op->getType() == Option::None)
	      op = std::make_shared<StringOption>("", *op);
	    if (types.back() == Option::Vector)
	      ops.back()->getVector().push_back(op);
	    else if (types.back() == Option::Dictionary)
	      ops.back()->getDictionary().insert({tag, op});
	  }
	  i = i1;
	}
      }
      else
      {
	// Detect content
	if (types.back() != Option::None)
	  throw_error(i, "Not supported text between tags");

	auto p = std::find(i, s.end(), '<'); // first non-content character
	auto i1 = p - 1;
	for (; std::isspace(static_cast<unsigned char>(*i1)); --i1)
	  ;
	std::string content(i, i1 + 1);

	// Set content into the top option
	ops.back() = std::make_shared<StringOption>(content, *ops.back());
	i = p - 1;
      }
    }

    if (tags.size() != 1)
      throw_error(s.end(), "there are unclosed tags");

    if (ops.size() == 0)
      return std::make_shared<NoneOption>();

    return ops.front();
  }

  namespace detail
  {
    /// is_integral vector_or_array<T>::value return whether T is std::vector<Q>
    /// or std::array<Q, N> for Q being integral

    template <typename T>
    struct is_integral_vector_or_array : std::false_type {
    };

    template <typename T>
    struct is_integral_vector_or_array<std::vector<T>> {
      static constexpr bool value = std::is_integral<T>::value;
    };

    template <typename T, std::size_t N>
    struct is_integral_vector_or_array<std::array<T, N>> {
      static constexpr bool value = std::is_integral<T>::value;
    };

    /// is_integral vector_or_array<T>::value return whether T is std::vector<Q>
    /// or std::array<Q, N> for Q being a struct or class

    template <typename T>
    struct is_class_vector_or_array : std::false_type {
    };

    template <typename T>
    struct is_class_vector_or_array<std::vector<T>> {
      static constexpr bool value = std::is_class<T>::value;
    };

    template <typename T, std::size_t N>
    struct is_class_vector_or_array<std::array<T, N>> {
      static constexpr bool value = std::is_class<T>::value;
    };

    template <typename T>
    inline std::size_t& get_typical_size()
    {
      static std::size_t size = 0;
      return size;
    }

    template <typename>
    struct GetXMLExtra;

    template <typename Tuple, std::size_t Idx>
    void GetXMLForTuple(const Tuple& t, std::string& r);
  }

  /// Write the xml associated to an option
  /// \param t: the object to serialize

  template <typename T>
  std::string getXML(const T& t)
  {
    if constexpr (has_schema<T>::value)
    {
      return detail::GetXMLExtra<T>::getXML(t);
    }
    else if constexpr (std::is_same<T, std::string>::value)
    {
      return t;
    }
    else if constexpr (std::is_same<T, bool>::value)
    {
      if (t)
	return "true";
      else
	return "false";
    }
    else if constexpr (std::is_integral<T>::value)
    {
      return std::to_string(t);
    }
    else if constexpr (detail::is_tuple<T>::value)
    {
      std::string r;
      detail::GetXMLForTuple<T, 0>(t, r);
      return r;
    }
    else if constexpr (detail::is_integral_vector_or_array<T>::value)
    {
      std::string r;
      r.reserve(t.size() * 4);
      bool is_first = true;
      for (const auto& it : t)
      {
	if (!is_first)
	  r += " ";
	r += std::to_string(it);
	is_first = false;
      }
      return r;
    }
    else if constexpr (detail::is_class_vector_or_array<T>::value)
    {
      std::string r;
      r.reserve(detail::get_typical_size<typename T::value_type>() * t.size());
      for (const auto& it : t)
      {
	r += getXML(it, "elem");
      }
      return r;
    }
    else if constexpr (detail::is_map<T>::value)
    {
      std::string r;
      for (const auto& it : t)
      {
	r += "<elem>";
	r += getXML(it.first, "Key");
	r += getXML(it.second, "Val");
	r += "</elem>";
      }
      return r;
    }
    else
    {
      static_assert(sizeof(T) == 0, "unsupported type");
    }
  }

  namespace detail
  {
    /// Auxiliary function for tuples

    template <typename Tuple, std::size_t Idx>
    void GetXMLForTuple(const Tuple& t, std::string& r)
    {
      if constexpr (Idx < std::tuple_size<Tuple>::value)
      {
	r += getXML(std::get<Idx>(t), std::string("tag") + std::to_string(Idx));
	GetXMLForTuple<Tuple, Idx + 1>(t, r);
      }
    }

    /// Auxiliary struct to print structs

    template <typename T>
    struct GetXMLExtra {
      static std::string getXML(const T& t)
      {
	std::string r;
	r.reserve(get_typical_size<T>());
	print_member<0>(t, r);
	get_typical_size<T>() = std::max(get_typical_size<T>(), r.size());
	return r;
      }

    private:
      template <std::size_t item,
		typename std::enable_if<(item >= std::tuple_size<typename T::schema>::value),
					bool>::type = true>
      static void print_member(const T&, std::string&)
      {
      }

      template <std::size_t item,
		typename std::enable_if<(item < std::tuple_size<typename T::schema>::value),
					bool>::type = true>
      static void print_member(const T& t, std::string& r)
      {
	using member = std::tuple_element<item, typename T::schema>::type;
	if constexpr (member::member_active)
	{
	  const char* label = &member::member_label.value[0];
	  r += SB::getXML(static_cast_extra<typename member::type>(t.*member::member_value), label);
	}
	print_member<item + 1>(t, r);
      }
    };
  }

  /// Return the XML representation of the given value surrounded by the given tag
  /// \param t: value to dump as XML
  /// \param tag: XML tag
  /// Example:
  ///   getXML(5, "value") -> "<value>5</value>"

  template <typename T>
  std::string getXML(const T& t, const std::string& tag)
  {
    const std::string& xml_t = getXML(t);
    std::string r;
    r.reserve(tag.size() * 2 + xml_t.size() + 5u);
    r.append("<");
    r += tag;
    r.append(">");
    r += xml_t;
    r.append("</");
    r += tag;
    r.append(">");
    return r;
  }

  namespace detail
  {
    /// Return the pointer to first element after consuming the string '<' tag '>'
    /// \param begin: first element of the string to parse
    /// \param end: after the last element of the string to parse
    /// \param tag: XML tag to parse

    inline const char* consume_open_tag(const char* begin, const char* end, const std::string& tag)
    {
      std::size_t n = end - begin;
      if (n < 2u + tag.size() || begin[0] != '<' ||
	  !std::equal(tag.begin(), tag.end(), begin + 1) || begin[1u + tag.size()] != '>')
	throw std::runtime_error("unexpected element");
      return begin + 2u + tag.size();
    }

    /// Return the pointer to first element after consuming the string '<' tag '>'
    /// \param begin: first element of the string to parse
    /// \param end: after the last element of the string to parse
    /// \param tag: XML tag to parse

    inline bool can_consume_open_tag(const char* begin, const char* end, const std::string& tag)
    {
      std::size_t n = end - begin;
      if (n < 2u + tag.size() || begin[0] != '<' ||
	  !std::equal(tag.begin(), tag.end(), begin + 1) || begin[1u + tag.size()] != '>')
	return false;
      return true;
    }

    inline const char* consume_close_tag(const char* begin, const char* end, const std::string& tag)
    {
      std::size_t n = end - begin;
      if (n < 3u + tag.size() || begin[0] != '<' || begin[1] != '/' ||
	  !std::equal(tag.begin(), tag.end(), begin + 2) || begin[2u + tag.size()] != '>')
	throw std::runtime_error("unexpected element");
      return begin + 3u + tag.size();
    }

    inline bool can_consume_close_tag(const char* begin, const char* end, const std::string& tag)
    {
      std::size_t n = end - begin;
      if (n < 3u + tag.size() || begin[0] != '<' || begin[1] != '/' ||
	  !std::equal(tag.begin(), tag.end(), begin + 2) || begin[2u + tag.size()] != '>')
	return false;
      return true;
    }

    template <typename>
    struct GetExtraOptionFast;

    template <std::size_t Idx, typename T>
    const char* getOptionFastForTupleElement(const char* begin, const char* end, T& r);

    /// Return the value of a given type next in the string, and return the first string element after
    /// \param begin: first element of the string to parse
    /// \param end: after the last element of the string to parse
    /// \param tag: XML tag to parse

    template <typename T>
    std::tuple<T, const char*> getOptionFast(const char* begin, const char* end,
					     const std::string& tag)
    {
      if constexpr (std::is_class<T>::value && has_schema<T>::value)
      {
	return GetExtraOptionFast<T>::getOptionFast(begin, end, tag);
      }
      else if constexpr (std::is_same<T, bool>::value)
      {
	begin = consume_open_tag(begin, end, tag);
	std::size_t n = end - begin;
	const std::string true_s("true");
	const std::string false_s("false");
	if (n >= true_s.size() && std::equal(true_s.begin(), true_s.end(), begin))
	{
	  return {true, consume_close_tag(begin + true_s.size(), end, tag)};
	}
	else if (n >= false_s.size() && std::equal(false_s.begin(), false_s.end(), begin))
	{
	  return {false, consume_close_tag(begin + false_s.size(), end, tag)};
	}
	throw std::runtime_error("unexpected element");
      }
      else if constexpr (std::is_integral<T>::value && !std::is_same<T, bool>::value)
      {
	begin = consume_open_tag(begin, end, tag);
	const char* i = begin;
	while (i != end && *i != '<')
	  i++;
	T value = std::stoi(std::string(begin, i));
	return {value, consume_close_tag(i, end, tag)};
      }
      else if constexpr (std::is_same<T, std::string>::value)
      {
	begin = consume_open_tag(begin, end, tag);
	const char* i = begin;
	while (i != end && *i != '<')
	  i++;
	return {std::string(begin, i), consume_close_tag(i, end, tag)};
      }
      else if constexpr (is_integral_vector_or_array<T>::value && is_vector<T>::value)
      {
	begin = consume_open_tag(begin, end, tag);
	const char* i = begin;
	T r;
	while (end != i && *i != '<')
	{
	  begin = i;
	  while (i != end && *i != '<' && *i != ' ')
	    i++;
	  if (i != begin)
	    r.push_back(std::stoi(std::string(begin, i)));
	  if (i != end && *i == ' ')
	    i++;
	}
	return {r, consume_close_tag(i, end, tag)};
      }
      else if constexpr (is_integral_vector_or_array<T>::value && is_array<T>::value)
      {
	begin = consume_open_tag(begin, end, tag);
	const char* i = begin;
	T r{{}};
	std::size_t ri = 0;
	while (end != i && *i != '<')
	{
	  begin = i;
	  while (i != end && *i != '<' && *i != ' ')
	    i++;
	  if (i != begin)
	  {
	    if (ri < r.size())
	      r[ri++] = std::stoi(std::string(begin, i));
	    else
	      throw std::runtime_error("parsing an array with an invalid number of arguments");
	  }
	  if (i != end && *i == ' ')
	    i++;
	}
	if (ri != r.size())
	  throw std::runtime_error("parsing an array with an invalid number of arguments");
	return {r, consume_close_tag(i, end, tag)};
      }
      else if constexpr (is_class_vector_or_array<T>::value && is_vector<T>::value)
      {
	begin = consume_open_tag(begin, end, tag);
	T r;
	while (!can_consume_close_tag(begin, end, tag))
	{
	  auto [value, new_begin] = getOptionFast<typename T::value_type>(begin, end, "elem");
	  r.push_back(value);
	  begin = new_begin;
	}
	return {r, consume_close_tag(begin, end, tag)};
      }
      else if constexpr (is_map<T>::value)
      {
	begin = consume_open_tag(begin, end, tag);
	T r;
	while (!can_consume_close_tag(begin, end, tag))
	{
	  begin = consume_open_tag(begin, end, "elem");
	  auto [key, begin_after_key] = getOptionFast<typename T::key_type>(begin, end, "Key");
	  auto [value, begin_after_value] =
	    getOptionFast<typename T::mapped_type>(begin_after_key, end, "Val");
	  r.insert({key, value});
	  begin = consume_close_tag(begin_after_value, end, "elem");
	}
	return {r, consume_close_tag(begin, end, tag)};
      }
      else if constexpr (is_tuple<T>::value)
      {
	begin = consume_open_tag(begin, end, tag);
	T r;
	begin = getOptionFastForTupleElement<0>(begin, end, r);
	return {r, consume_close_tag(begin, end, tag)};
      }
      else
      {
	static_assert(sizeof(T) == 0, "unsupported type");
      }
    }

    template <std::size_t Idx, typename T>
    const char* getOptionFastForTupleElement(const char* begin, const char* end, T& r)
    {
      if constexpr (Idx < std::tuple_size<T>::value)
      {
	const auto tag = std::string("tag") + std::to_string(Idx);
	const auto [ri, new_begin] =
	  getOptionFast<typename std::tuple_element<Idx, T>::type>(begin, end, tag);
	std::get<Idx>(r) = ri;
	return getOptionFastForTupleElement<Idx + 1>(new_begin, end, r);
      }
      else
      {
	return begin;
      }
    }

    template <typename T>
    struct GetExtraOptionFast {
      static std::tuple<T, const char*> getOptionFast(const char* begin, const char* end,
						      const std::string& tag)
      {
	T r;
	const char* new_begin = update_option<0>(r, consume_open_tag(begin, end, tag), end);
	return {r, consume_close_tag(new_begin, end, tag)};
      }

      template <std::size_t item,
		typename std::enable_if<(item >= std::tuple_size<typename T::schema>::value),
					bool>::type = true>
      static const char* update_option(T&, const char* begin, const char*)
      {
	return begin;
      }

      template <std::size_t item,
		typename std::enable_if<(item < std::tuple_size<typename T::schema>::value),
					bool>::type = true>
      static const char* update_option(T& r, const char* begin, const char* end)
      {
	using member = std::tuple_element<item, typename T::schema>::type;
	if constexpr (member::member_active)
	{
	  const auto& label =
	    std::string(member::member_label.value, sizeof(member::member_label.value) - 1);
	  constexpr auto default_value = std::string_view(
	    member::member_default_value.value, sizeof(member::member_default_value.value) - 1);
	  constexpr auto no_default_value =
	    std::string_view(NoDefault.value, sizeof(NoDefault.value) - 1);
	  if (can_consume_open_tag(begin, end, label))
	  {
	    const auto& [value, new_begin] =
	      SB::detail::getOptionFast<typename member::type>(begin, end, label);
	    r.*member::member_value = static_cast_extra<typename member::actual_type>(value);
	    return update_option<item + 1>(r, new_begin, end);
	  }
	  else if constexpr (default_value != no_default_value)
	  {
	    const auto& value = SB::getOptionFast<typename member::type>(
	      SB::getXML(std::string(default_value), label), label);
	    r.*member::member_value = static_cast_extra<typename member::actual_type>(value);
	    return update_option<item + 1>(r, begin, end);
	  }
	  else
	  {
	    throw std::runtime_error("missing tag");
	  }
	}
	else
	{
	  return update_option<item + 1>(r, begin, end);
	}
      }
    };
  }

  /// Return the value of a serialized object by `getXML`
  /// \param s: string returned by `getXML`
  /// \param tag: XML tag surrounded the serialization
  /// NOTE: It may NOT work with arbitrary XMLs not generated with `getXML`!
  ///       Use `getOptionsFromXML' and  `getOption' for arbitrary XMLs.
  ///
  /// Example:
  ///   std::string s = getXML(5, "value"); /// returns "<value>5</value>"
  ///   getOptionFast<int>(s, "value"); /// returns 5

  template <typename T>
  T getOptionFast(const std::string& s, const std::string& tag)
  {
    const auto& [value, new_begin] = detail::getOptionFast<T>(s.data(), s.data() + s.size(), tag);
    if (new_begin != s.data() + s.size())
      throw std::runtime_error("getOptionFast: not all string was consume!");
    return value;
  }

  namespace detail
  {
    inline std::size_t hash_combine(std::size_t a, std::size_t b)
    {
      return a ^ (b << 1);
    }

    template <typename>
    struct GetHashExtra;

    template <typename Tuple, std::size_t Idx>
    std::size_t GetHashForTuple(const Tuple& t);
  }

  /// Get the hash associated to an object
  /// \param t: the object

  template <typename T>
  std::size_t getHash(const T& t)
  {
    if constexpr (has_schema<T>::value)
    {
      return detail::GetHashExtra<T>::getHash(t);
    }
    else if constexpr (detail::is_std_hashable<T>::value)
    {
      return std::hash<T>{}(t);
    }
    else if constexpr (detail::is_complex<T>::value)
    {
      return detail::hash_combine(std::hash<typename T::value_type>{}(std::real(t)),
				  std::hash<typename T::value_type>{}(std::imag(t)));
    }
    else if constexpr (detail::is_tuple<T>::value)
    {
      return detail::GetHashForTuple<T, 0>(t);
    }
    else if constexpr (detail::is_vector_or_array<T>::value || detail::is_map<T>::value ||
                       detail::is_set<T>::value)
    {
      std::size_t r = 12345;
      for (const auto& i : t)
	r = detail::hash_combine(r, getHash(i));
      return r;
    }
    else
    {
      static_assert(sizeof(T) == 0, "unsupported type");
      return 0;
    }
  }

  namespace detail
  {
    /// Auxiliary function to hash tuples

    template <typename Tuple, std::size_t Idx>
    std::size_t GetHashForTuple(const Tuple& t)
    {
      if constexpr (Idx >= std::tuple_size<Tuple>::value)
      {
	return 12345;
      }
      else
      {
	return hash_combine(getHash(std::get<Idx>(t)), GetHashForTuple<Tuple, Idx + 1>(t));
      }
    }

    /// Auxiliary struct to hash structs

    template <typename T>
    struct GetHashExtra {
      static std::size_t getHash(const T& t)
      {
	return hash_member<0>(t, 12345);
      }

    private:
      template <std::size_t item,
		typename std::enable_if<(item >= std::tuple_size<typename T::schema>::value),
					bool>::type = true>
      static std::size_t hash_member(const T&, const std::size_t& r)
      {
	return r;
      }

      template <std::size_t item,
		typename std::enable_if<(item < std::tuple_size<typename T::schema>::value),
					bool>::type = true>
      static std::size_t hash_member(const T& t, std::size_t r)
      {
	using member = std::tuple_element<item, typename T::schema>::type;
	if constexpr (member::member_active)
	{
	  r = detail::hash_combine(
	    r, SB::getHash(static_cast_extra<typename member::type>(t.*member::member_value)));
	}
	return hash_member<item + 1>(t, r);
      }
    };
  }

  namespace example
  {
    template <typename T>
    struct my_vector {
      std::vector<T> v;

      my_vector()
      {
      }

      my_vector(const std::vector<T>& v) : v(v)
      {
      }

      operator std::vector<T>() const
      {
	return v;
      }
    };

    inline void simple_test()
    {
      /// Example:
      struct A {
	int a;
	my_vector<int> b;
	std::array<int, 2> c;

	using schema = std::tuple<			  //
	  struct_member<&A::a, "a"_sl>,			  //
	  struct_member<&A::b, "b"_sl, std::vector<int>>, //
	  struct_member<&A::c, "c"_sl, None, "0 0 1"_sl>  //
	  >;
      };

      {
	const std::string xml = "<A><a>4</a><b>0 1 2</b></A>";
	const auto& ops = SB::getOptionsFromXML(xml);
	A a = SB::getOption<A>(*ops, "A");
	if (a.a != 4)
	  throw std::runtime_error("wtf!");
	if (a.b.v != std::vector<int>{0, 1, 2})
	  throw std::runtime_error("wft!");
	A a0 = SB::getOptionFast<A>(xml, "A");
	if (a0.a != a.a || a0.b.v != a.b.v)
	  throw std::runtime_error("wft!");
	if (SB::getXML(a, "A") != xml)
	  throw std::runtime_error("wft!");
      }
      {
	const std::string xml = "<A><elem><Key>4</Key><Val>0 1 2</Val></elem></A>";
	const auto& ops = SB::getOptionsFromXML(xml);

	auto a = SB::getOption<std::map<int, std::vector<int>>>(*ops, "A");
	if (a.at(4) != std::vector<int>{0, 1, 2})
	  throw std::runtime_error("wft!");
	if (SB::getXML(a, "A") != xml)
	  throw std::runtime_error("wft!");
      }
      {
	const std::string xml = "<A><a>4</a><b>0 1 2</b></A>";
	const auto& ops = SB::getOptionsFromXML(xml);
	A a = SB::getOption<A>(*ops, "A");
	SB::getHash(std::make_tuple(1, std::string("hola")));
	auto hash_a = SB::getHash(a);
	auto hash_b = SB::getHash(std::map<int, std::string>{{0, "hola"}});
	auto hash_c = SB::getHash(std::map<int, A>{{0, a}});
      }
      {
	const auto& a = std::make_tuple(3, std::vector<int>{0, 1});
	const auto& xml = SB::getXML(a, "A");
	const auto& b = SB::getOptionFast<std::remove_cvref_t<decltype(a)>>(xml, "A");
	if (a != b)
	  throw std::runtime_error("wft!");
      }
    }
  }
}

#endif // __INCLUDE_SUPERB_OPTIONS__
