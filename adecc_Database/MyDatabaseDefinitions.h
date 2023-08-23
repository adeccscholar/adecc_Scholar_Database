#pragma once

#include <string>
#include <chrono>
#include <optional>
#include <vector>
#include <tuple>
#include <variant>
#include <type_traits>
#include <concepts>


//using my_date = std::chrono::year_month_day;
//using my_timestamp = std::chrono::system_clock::time_point>;

template <typename... Types>
struct my_type_list {
   using type_list    = std::tuple<Types...>;
   using type_variant = std::variant<Types...>;
};

using my_defined_value_types = my_type_list<int, double, long long, 
                                            std::string, 
                                            std::chrono::year_month_day,
                                            std::chrono::system_clock::time_point>;

using my_db_value  = typename my_defined_value_types::type_variant;
using my_db_param  = std::tuple < std::string, my_db_value, bool>;
using my_db_params = std::vector<my_db_param>;

/*
using my_db_value = std::variant<int, long, long long, bool, double,
                                 std::string,
                                 std::chrono::year_month_day,
                                 std::chrono::system_clock::time_point>;
*/

// -------------------------------------------------------------------------------


template <typename ty, typename... type_list>
struct is_one_of_types {
   static constexpr bool value = (std::is_same_v<ty, type_list> || ...);
};

template <typename ty, typename... types_list>
inline constexpr bool is_one_of_types_v = is_one_of_types<ty, types_list...>::value;

template <typename ty, typename... type_list>
struct is_convertible_to_types {
   static constexpr bool value = (std::is_convertible_v<ty, type_list> || ...);
};

template <typename ty, typename... types_list>
inline constexpr bool is_convertible_to_types_v = is_convertible_to_types<ty, types_list...>::value;

//-----------------

template <typename ty, typename tuple_with_types>
struct is_my_db_value_type_impl;

template <typename ty, typename... types_list>
struct is_my_db_value_type_impl<ty, std::tuple<types_list...>> {
   static constexpr bool value = (std::is_same_v<ty, types_list> || ...);
};

template <typename ty>
struct is_my_db_value_type {
   static constexpr bool value = is_my_db_value_type_impl<ty, typename my_defined_value_types::type_list>::value;
};

template <typename ty>
concept my_db_value_type = is_my_db_value_type<ty>::value;

// Erweiterung der Datenbanktypen um die optionalen Typen für Parameter

// zum Einstieg, später löschen
template <typename ty>
struct my_is_optional : std::false_type {};

template <typename ty>
struct my_is_optional<std::optional<ty>> : std::true_type {};


template <typename ty, typename U>
struct is_my_type_or_optional : std::false_type {};

template <typename ty>
struct is_my_type_or_optional<ty, ty> : std::true_type {};

template <typename ty>
struct is_my_type_or_optional<ty, std::optional<ty>> : std::true_type {};


template <typename ty, typename... types_list>
struct is_my_db_value_type_or_optional_impl;

template <typename ty, typename... types_list>
struct is_my_db_value_type_or_optional_impl<ty, std::tuple<types_list...>> {
   static constexpr bool value = (is_my_type_or_optional<ty, types_list>::value || ...);
};

template <typename ty, typename... types_list>
struct is_my_db_value_type_or_optional_impl<std::optional<ty>, std::tuple<types_list...>> {
   static constexpr bool value = (is_my_type_or_optional<ty, types_list>::value || ...);
};

template <typename ty>
struct is_my_db_value_or_optional_type {
   static constexpr bool value = is_my_db_value_type_or_optional_impl<ty, typename my_defined_value_types::type_list>::value;
};
template <typename ty>
concept my_db_value_or_optional_type = is_my_db_value_or_optional_type<ty>::value;






template <typename ty>
struct is_my_db_additional_param_type {
   static constexpr bool value = is_convertible_to_types_v<ty, std::string_view, const char*, const wchar_t*>;
};

template <typename ty>
struct is_my_db_param_type {
   static constexpr bool value = is_my_db_value_or_optional_type<ty>::value ||
                                 is_my_db_additional_param_type<ty>::value;
};



template <typename ty>
concept my_db_param_type = is_my_db_param_type<ty>::value;

//-----------------------------------------------------------------------------

template <typename db_type, typename qry_type, typename ret_type>
concept is_return_type_for_get = requires(qry_type qry, std::string const& field) {
   { db_type::template Get<ret_type>(qry, field, std::declval<bool>()) } -> std::same_as<std::optional<ret_type>>;
};

template <typename db_type, typename qry_type, typename... ret_types>
concept my_return_type_for_get = requires(qry_type qry, std::string const& field) {
   (is_return_type_for_get<db_type, qry_type, ret_types> && ...);
};


template <typename db_type, typename qry_type>
concept my_return_type_for_get_with_def = my_return_type_for_get<db_type, qry_type, typename my_defined_value_types::type_list>;



template <typename db_type, typename qry_type, typename param_type>
concept is_param_type_for_set = requires(db_type db, qry_type qry, param_type param) {
   { db_type::template Set(qry, std::declval<std::string const&>(), param) } -> std::same_as<void>;
};

template <typename db_type, typename qry_type, typename... ret_types>
concept my_param_type_for_set = requires(qry_type qry, std::string const& field) {
   (is_param_type_for_set<db_type, qry_type, ret_types> && ...);
};

template <typename db_type, typename qry_type>
concept my_param_type_for_set_with_def = my_param_type_for_set<db_type, qry_type, typename my_defined_value_types::type_list>;


template <typename db_type, typename qry_type, typename... ret_param_types>
concept my_types_for_get_and_set =
(my_return_type_for_get<db_type, qry_type, ret_param_types> && ...) &&
(my_param_type_for_set<db_type, qry_type, ret_param_types> && ...);

template <typename db_type, typename qry_type>
concept my_types_for_get_and_set_with_def =
my_return_type_for_get_with_def<db_type, qry_type> && my_param_type_for_set_with_def<db_type, qry_type>;


//template <template <typename> typename framework_type, typename server_type, typename... ret_types>
template <template <typename> typename framework_type, typename server_type>
concept my_db_frame_work = my_db_credentials<server_type> &&
   requires(framework_type<server_type> t) {
   typename framework_type<server_type>::database_type;
   typename framework_type<server_type>::database_conv;
   typename framework_type<server_type>::database_const_conv;
   typename framework_type<server_type>::database_para;
   typename framework_type<server_type>::database_const_para;
   typename framework_type<server_type>::query_type;
   typename framework_type<server_type>::query_conv;
   typename framework_type<server_type>::query_const_conv;
   typename framework_type<server_type>::query_para;
   typename framework_type<server_type>::query_const_para;
   { t.Database() } -> std::convertible_to<typename framework_type<server_type>::database_conv>;
   { t.Database() } -> std::convertible_to<typename framework_type<server_type>::database_const_conv>;
   { t.Open() } -> std::convertible_to<std::pair<bool, std::string>>;
   { t.Connected() } -> std::convertible_to<bool>;
   { t.Close() } -> std::convertible_to<void>;
   { t.CreateQueryObject() } -> std::convertible_to<typename framework_type<server_type>::query_type>;
      requires requires(typename framework_type<server_type>::query_para p, std::string const& s) {
         { framework_type<server_type>::SetSQL(p, s) } -> std::convertible_to<std::pair<bool, std::string>>;
         { framework_type<server_type>::GetSQL(p) } -> std::convertible_to<std::string>;
         { framework_type<server_type>::Execute(p) } -> std::convertible_to<std::tuple<bool, std::string, std::vector<std::string>>>;
         { framework_type<server_type>::First(p) } -> std::convertible_to<bool>;
         { framework_type<server_type>::IsEof(p) } -> std::convertible_to<bool>;
         { framework_type<server_type>::Next(p) } -> std::convertible_to<bool>;
   } && my_types_for_get_and_set<framework_type<server_type>, typename framework_type<server_type>::query_type>;
};

// Forward Declaration for TMyDatabase and concept to control

template <template <class> typename framework_type, typename server_type>
concept my_db_declaration = my_db_frame_work<framework_type, server_type>;

template <template <class> class framework_type, class server_type>
   requires my_db_declaration<framework_type, server_type>
class TMyDatabase;

// ---------------------------------------------------------------
template <template <class> class framework_type, class server_type>
   requires my_db_declaration<framework_type, server_type>
class TMyQuery;
// ---------------------------------------------------------------
