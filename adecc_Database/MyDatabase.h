#pragma once

//#include <adecc_Scholar/MyForm.h>
#include <adecc_Scholar/MyType_Traits.h>


#include "MyDatabaseFramework.h"

#include "adecc_Tools/MyHelper.h"

#include <utility>
#include <tuple>
#include <type_traits>
#include <concepts>
#include <string>
#include <vector>
#include <regex>
#include <chrono>
#include <set>
#include <optional>
#include <utility>
#include <ranges>
#include <limits>
#include <algorithm>

using namespace std::literals;

template <template <class> class framework_type, class server_type>
   requires my_db_declaration<framework_type, server_type>
class TMyQuery {
private:
   using framework = framework_type<server_type>;
   typename framework::query_type query;
   TMyDatabase<framework_type, server_type> const& database;
   std::vector<std::string> params;
public:
   TMyQuery(void) = delete;
   TMyQuery(TMyDatabase<framework_type, server_type> const& db) : database(db) { query.setForwardOnly(true);  }

   TMyQuery(TMyQuery const& other) = delete;
   TMyQuery(TMyQuery && other) noexcept : query(std::move(other.query)), database(other.database), params(std::move(other.params)) {}

   virtual ~TMyQuery() {}

   operator typename framework::query_conv() { return query; }

   /*
   void Create() { 
      query = database.CreateQueryObject();
      }

   void Create(std::string const& strSQL, std::source_location const& loc = std::source_location::current()) {
      std::swap(query, database.CreateQueryObject());
      SetSQL(strSQL, loc);
      }
   */

   bool SetSQL(std::string_view strSQL, std::source_location const& loc = std::source_location::current()) {
      return SetSQL(std::string { strSQL.begin(), strSQL.end() });
      }

   bool SetSQL(std::string const& strSQL, std::source_location const& loc = std::source_location::current()) {
      params.clear();

      static const std::regex parser("(\\:)(\\w+)(\\s|<|>|=|,|\\(|\\)|$)");
      static const std::string format("$2");
      for (auto it = std::sregex_iterator(strSQL.begin(), strSQL.end(), parser); it != std::sregex_iterator(); ++it) {
         std::string strParam = std::regex_replace(it->str(), parser, format);
         if(std::find(params.begin(), params.end(), strParam) == params.end())
            params.emplace_back(std::forward<std::string>(strParam));
         }
      if (auto [ret, msg] = framework::SetSQL(query, strSQL); !ret) [[unlikely]] {
         throw TMy_Db_Exception("Statement couldn't be set.", msg, database.Status(), strSQL, loc);
         }
      else return ret;
      }

   bool SetSQL(std::wstring_view strSQL, std::source_location const& loc = std::source_location::current()) {
      return SetSQL(std::wstring { strSQL.begin(), strSQL.end() });
      }

   bool SetSQL(std::wstring const& strSQL, std::source_location const& loc = std::source_location::current()) {
      /*
      params.clear();

      static const std::wregex parser(L"(\\:)(\\w+)(\\s|<|>|=|,|\\(|\\)|$)");
      static const std::wstring format(L"$2");
      for (auto it = std::wsregex_iterator(strSQL.begin(), strSQL.end(), parser); it != std::wsregex_iterator(); ++it) {
         params.emplace_back(std::wregex_replace(it->str(), parser, format));
      }
      */
      if (auto [ret, msg] = framework::SetSQL(query, strSQL); !ret) [[unlikely]] {
         throw TMy_Db_Exception("Statement couldn't be set.", msg, database.Status(), "SQL Statement is wstring", loc);
         }
      else return ret;
      }




   std::string GetSQL(void) const {
      return framework::GetSQL(query);
      }

   bool Execute(src_loc const& loc = src_loc::current()) {
      if(auto [ret, msg, sqlparams] = framework::Execute(query); ret) [[likely]] {
         framework::First(query);
         return ret;
         }
      else {
         std::ostringstream query_info;
         query_info << GetSQL();
         if (params.size() > 0) {
            query_info << "\n\nParameter:";
            for (size_t i = 0; i < params.size(); ++i) query_info << std::format("\n{} = {}", params[i], sqlparams[i]);
            }
         throw TMy_Db_Exception("error while executing query", msg, database.Status(), query_info.str(), loc);
         }
      }

   bool Execute(my_db_params const& params, src_loc const& loc = src_loc::current()) {
      Set(params);
      return Execute(loc);
      }

   bool First(void) { return framework::First(query); }
   bool IsEof(void) { return framework::IsEof(query); }
   bool Next(void) { return framework::Next(query); }

   template <my_db_value_type ret_type>
   std::optional<ret_type> Get(std::string const& field, bool required = false, src_loc const& loc = src_loc::current()) const {
      try {
         return framework::template Get<ret_type>(query, field, required);
         }
      catch (std::exception& ex) {
         throw TMy_Db_Exception(std::format("error for get attribute: {}", field), ex.what(), database.Status(), GetSQL(), loc);
         }
      }

   
   std::vector<std::optional<my_db_value>> Get(std::vector<std::tuple<std::string, my_db_value, bool>>& params, src_loc const& loc = src_loc::current()) const  {
      std::vector<std::optional<my_db_value>> result;

      for (auto& [key, var, required] : params) {
         std::visit([&result, &key, &required, this](auto&& arg) {
            using used_type = std::decay_t<decltype(arg)>;  
            result.emplace_back(Get<used_type>(key, required));
            }, var);
         }
      return result;
      }



   template <my_db_param_type param_type>
   bool Set(std::string const& field_name, std::optional<param_type> const& param, bool required = false, src_loc const& loc = src_loc::current()) {
      if (auto found = framework::template Set<std::optional<param_type>>(query, field_name, param); found) return found;
      else {
         if (!required) [[likely]] return found;
         else
            throw TMy_Db_Exception("error by set parameter for query", 
		                           std::format("required Parameter \"{}\" not found.", field_name), 
								   database.Status(), 
								   GetSQL(), loc);
         }
      }

   template <my_db_param_type param_type>
   bool Set(std::string const& field_name, param_type const& param, bool required = false, src_loc const& loc = src_loc::current()) {
      if (auto found = framework::template Set<param_type>(query, field_name, param); found) return found;
      else {
         if (!required) [[likely]] return found;
         else
            throw TMy_Db_Exception("error by set parameter for query",
               std::format("required Parameter \"{}\" not found.", field_name),
               database.Status(),
               GetSQL(), loc);
         }
      }
    
   template<typename... Ts>
   void CallSet(std::string const& field_name, bool required, std::optional<std::variant<Ts...>> const& opt_variant) {
      if (!opt_variant) {
         (void)(([&]<typename T>() {
            Set<T>(field_name, std::optional<T>{}, required);
            }.template operator()<Ts>()), ...);
         return;
         }

      std::visit([&](auto&& value) {
         using T = std::decay_t<decltype(value)>;
         Set<T>(field_name, std::optional<T>(value), required);
         }, *opt_variant);
      }
   
   template<typename... Ts>
   void CallSet(std::string const& field_name, bool required, std::variant<Ts...> const& variant) {
      ((std::holds_alternative<Ts>(variant) ? (Set(field_name, std::get<Ts>(variant), required), true) : false) || ...);
      }

   bool Set(my_db_params const& params) {
      for(auto const& [field, param, required] : params) {
         CallSet(field, required, param);
         }
      return true;
      }
	  
	  
};

// later a concept
template <typename query_ty>
class QueryIterator {
public:
   using iterator_category = std::input_iterator_tag;
   using value_type = query_ty;
   using difference_type = std::ptrdiff_t;
   using pointer = query_ty*;
   using reference = query_ty&;

   explicit QueryIterator(query_ty& query) : the_query(query), is_active(!query.IsEof()) { }
   explicit QueryIterator(query_ty& query, bool active) : the_query(query), is_active(active) {  }
   QueryIterator(QueryIterator const& other) : the_query(other.query), is_active(other.active) { }

   reference operator*() const { return the_query; }
   pointer operator->() const { return &the_query; }

   QueryIterator& operator++() {
      the_query.Next();
      if (the_query.IsEof()) is_active = false;
      return *this;
      }

   bool operator == (QueryIterator const& other) const {
      return &the_query == &other.the_query && is_active == other.is_active;
      }

   bool operator != (QueryIterator const& other) const {
      return !(*this == other);
      }

private:
   query_ty& the_query;
   bool is_active;
};

template <typename query_ty>
class QueryRange : public std::ranges::view_interface<QueryRange<query_ty>> {
public:
   explicit QueryRange(query_ty&& query) : the_query(std::move(query)) {}

   QueryIterator<query_ty> begin() {
      return QueryIterator<query_ty>(the_query);
      }

   QueryIterator<query_ty> end() {
      return QueryIterator<query_ty>(the_query, false);
      }

 
   auto begin() const {
      return QueryIterator<query_ty>(const_cast<query_ty&>(the_query));
   }
   auto end() const {
      return QueryIterator<query_ty>(const_cast<query_ty&>(the_query), false);
   }

   auto operator | (auto&& fn) {
      return std::move(fn)(the_query);
      }

private:
   query_ty the_query;
};

namespace std::ranges {
   template<typename query_ty>
   inline constexpr bool enable_borrowed_range<QueryRange<query_ty>> = true;
}




template <template <class> class framework_type, class server_type>
     requires my_db_declaration<framework_type, server_type>
class TMyDatabase : public framework_type<server_type> {
   private:
      using framework   = framework_type<server_type>;
      
   public:
      using type_server = server_type;

      TMyDatabase() : framework_type<server_type>() { }
      TMyDatabase(framework_type<server_type> const& ref) : framework_type<server_type>(ref) { }

      virtual ~TMyDatabase(void) {
         Close();
         }

      TMyDatabase& operator = (server_type const& ref) {
         static_cast<framework&>(*this) = ref;
         return *this;
         }
 
      operator typename framework::database_conv () { return framework::Database();  }

      bool Open(src_loc const& loc = src_loc::current()) {
         if(auto [ret, msg] = framework::Open(); ret ) {
            return ret;
            }
         else {
            static std::regex pwd_pattern(R"((?:Pwd|pwd|Password|password)=[^\s]+)");
            msg = std::regex_replace(msg, pwd_pattern, "Pwd=***");
            throw TMy_Db_Exception("database couldn't be open.", msg, framework::GetInformations(), { }, loc);
            }
         }

      void Close(void) { framework::Close(); }

      
      TMyQuery<framework_type, server_type> CreateQuery(void) const {
         TMyQuery<framework_type, server_type> tmp(*this);
         return tmp;
         }

      TMyQuery<framework_type, server_type> CreateQuery(std::string const& strSQL) const {
         TMyQuery<framework_type, server_type> tmp(*this);
         tmp.SetSQL(strSQL);
         return tmp;
         }

      TMyQuery<framework_type, server_type> Execute(std::string const& strSQL, my_db_params const& params, src_loc const& loc = src_loc::current()) {
         TMyQuery<framework_type, server_type> tmp(*this); 
         tmp.SetSQL(strSQL);
         tmp.Execute(params, loc);
         return std::move(tmp);
         }
 
      std::string Status(void) const {
         return std::format("{} {}", framework::Connected() ? "connected to" : "disconnected from", framework::GetInformations());
         }


      // ----------------------------------------------------------------------------------------------------------------------------

      std::set<std::string> GetTableNames(std::string const& schema) const {
         auto query = CreateQuery();
         std::set<std::string> setTableNames;
         if constexpr (std::is_same<server_type, TMyMSSQL>::value) {
            std::string strQry = "SELECT name AS TableName, SCHEMA_NAME(schema_id) AS SchemaName\n"s +
                                 "FROM sys.tables";
            if (schema.length() > 0) {
               strQry += "\nWHERE SCHEMA_NAME(schema_id) = :schema";
               query.SetSQL(strQry);
               query.Set("schema", schema);
               }
            else query.SetSQL(strQry);
            }
         else if constexpr (std::is_same<server_type, TMyMySQL>::value) {
            std::string strQry = "SELECT table_name AS TableName, table_schema AS SchemaName\n"s +
                                 "FROM information_schema.tables\n"s +
                                 "WHERE table_type = 'BASE TABLE' AND table_schema = :schema"s;
            query.SetSQL(strQry);
            if (schema.length() > 0) query.Set("schema", schema);
            else query.Set("schema", server_type::Database());
            }
         else if constexpr (std::is_same<server_type, TMyOracle>::value) {
            std::string strQry = "SELECT TABLE_NAME AS TableName, OWNER AS SchemaName\n"s +
                                 "FROM ALL_TABLES"s;
            if (schema.length() > 0) {
               strQry += "\nWHERE OWNER = :schema"s;
               query.SetSQL(strQry);
               query.Set("schema", schema);
               }
            else {
              // strQry += "\nWHERE OWNER NOT LIKE 'SYS%'"s;
               query.SetSQL(strQry);
               }
            }
         else if constexpr (std::is_same<server_type, TMyInterbase>::value) {
            std::string strQry = "SELECT RDB$RELATION_NAME AS TableName, RDB$OWNER_NAME AS SchemaName\n"s +
                                 "FROM RDB$RELATIONS\n"s +
                                 "WHERE RDB$VIEW_BLR IS NULL"s;
            if (schema.length() > 0) {
               strQry += "\n      AND RDB$RELATION_SCHEMA = :schema"s;
               query.SetSQL(strQry);
               query.Set("schema", schema);
               }
            else query.SetSQL(strQry);
            }
         else if constexpr (std::is_same<server_type, TMySQLite>::value) {
            std::string strQry = "SELECT name AS TableName, 'main' AS SchemaName\n"s +
                                 "FROM sqlite_master\n"s +
                                 "WHERE type = 'table'"s;
            query.SetSQL(strQry);
            }

         for(query.Execute(), query.First(); !query.IsEof(); query.Next()) {
            if constexpr (std::is_same<server_type, TMyInterbase>::value) {
               auto table = query.template Get<std::string>("TableName");
               if (table)
                  setTableNames.insert(trim(*table));
               }
            else {
               setTableNames.insert(query.template Get<std::string>("TableName").value_or("leer"));
               }
            }
         return setTableNames;
         }

      std::set<std::string> GetViewNames(std::string const& schema) const {
         auto query = CreateQuery();
         std::set<std::string> setViewNames;
         if constexpr (std::is_same<server_type, TMyMSSQL>::value) {
            std::string strQry = "SELECT name AS ViewName, SCHEMA_NAME(schema_id) AS SchemaName\n"s +
               "FROM sys.views";
            if (schema.length() > 0) {
               strQry += "\nWHERE SCHEMA_NAME(schema_id) = :schema";
               query.SetSQL(strQry);
               query.Set("schema", schema);
               }
            else query.SetSQL(strQry);
            }
         else if constexpr (std::is_same<server_type, TMyMySQL>::value) {
            std::string strQry = "SELECT table_name AS ViewName, table_schema AS SchemaName\n"s +
               "FROM information_schema.views\n"s +
               "WHERE table_schema = :schema"s;
            query.SetSQL(strQry);
            if (schema.length() > 0) query.Set("schema", schema);
            else query.Set("schema", server_type::Database());
            }
         else if constexpr (std::is_same<server_type, TMyOracle>::value) {
            std::string strQry = "SELECT VIEW_NAME AS ViewName, OWNER AS SchemaName\n"s +
                                 "FROM ALL_VIEWS"s;
            if (schema.length() > 0) {
               strQry += "\nWHERE OWNER = :schema"s;
               query.SetSQL(strQry);
               query.Set("schema", schema);
               }
            else {
              // strQry += "\nWHERE OWNER NOT LIKE 'SYS%'"s;
               query.SetSQL(strQry);
               }
            }
         else if constexpr (std::is_same<server_type, TMyInterbase>::value) {
            std::string strQry = "SELECT RDB$RELATION_NAME AS ViewName, RDB$OWNER_NAME AS SchemaName\n"s +
                                 "FROM RDB$RELATIONS\n"s +
                                 "WHERE RDB$VIEW_BLR IS NOT NULL"s;
            if (schema.length() > 0) {
               strQry += "\n      AND RDB$RELATION_SCHEMA = :schema"s;
               query.SetSQL(strQry);
               query.Set("schema", schema);
               }
            else query.SetSQL(strQry);
            }
         else if constexpr (std::is_same<server_type, TMySQLite>::value) {
            std::string strQry = "SELECT name AS ViewName, 'main' AS SchemaName\n"s +
                                 "FROM sqlite_master\n"s + 
                                 "WHERE type = 'view'"s;
            query.SetSQL(strQry);
            }

         for (query.Execute(), query.First(); !query.IsEof(); query.Next()) {
            if constexpr (std::is_same<server_type, TMyInterbase>::value) {
               auto view = query.template Get<std::string>("ViewName");
               if (view)
                  setViewNames.insert(trim(*view));
               }
            else {
               setViewNames.insert(query.template Get<std::string>("ViewName").value_or("leer"));
               }
            }
         return setViewNames;
      }

   };
