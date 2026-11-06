#pragma once

#include "datalib/codec/superb_options.h"

namespace Datalib::Storage::CorrKeyDetail
{
  namespace detail
  {
    namespace detail
    {
      template <std::size_t item, typename T,
		typename std::enable_if<(item >= std::tuple_size<typename T::schema>::value),
					bool>::type = true>
      void get_xml_fragments_aux(const T&, std::vector<std::string>&)
      {
      }

      template <std::size_t item, typename T,
		typename std::enable_if<(item < std::tuple_size<typename T::schema>::value),
					bool>::type = true>
      void get_xml_fragments_aux(const T& t, std::vector<std::string>& r)
      {
	using member = std::tuple_element<item, typename T::schema>::type;
	const char* label = &member::member_label.value[0];
	if constexpr (SB::detail::is_vector_or_array<typename member::type>::value)
	{
	  const auto& values = static_cast<typename member::type>(t.*member::member_value);
	  r.reserve(r.size() + 2 + values.size());
	  r.push_back(std::string{"<"} + label + std::string{">"});
	  r.push_back(std::string{"</"} + label + std::string{">"});
	  for (const auto& it : values)
	    r.push_back(SB::getXML(it, "elem"));
	}
	else
	{
	  r.push_back(
	    SB::getXML(static_cast<typename member::type>(t.*member::member_value), label));
	}
	get_xml_fragments_aux<item + 1>(t, r);
      }
    }

    /// Template declaration for the function that returns pieces of
    /// the XML representation of the key

    template <typename Key>
    std::vector<std::string> get_xml_fragments(const Key& key, const std::string& tag)
    {
      std::vector<std::string> r{std::string{"<"} + tag + std::string{">"},
				 std::string{"</"} + tag + std::string{">"}};
      detail::get_xml_fragments_aux<0>(key, r);
      return r;
    }
  }

}
