#pragma once

#include <adecc_Scholar/MyForm.h>

#include "MyDatabaseExceptions.h"
#include "MyDatabaseCredentials.h"
#include "MyDatabaseDefinitions.h"

#include "adecc_Tools/MyHelper.h"

#include <utility>
#include <tuple>
#include <type_traits>
#include <concepts>
#include <string>
#include <vector>
#include <regex>
#include <chrono>
#include <optional>
#include <ranges>
#include <limits>

using namespace std::literals;


template <template <class> class framework_type, class server_type>
   requires my_db_declaration<framework_type, server_type>
class TMyQuery {
private:
   using framework = framework_type<server_type>;
   typename framework::query_type query;
   TMyDatabase<framework_type, server_type>& database;
   std::vector<std::string> params;
public:
   TMyQuery(void) = delete;
   TMyQuery(TMyDatabase<framework_type, server_type>& db) : database(db) { }

   operator typename framework::query_conv() { return query; }

   void Create() { 
      query = database.CreateQueryObject();
      }

   void Create(std::string const& strSQL, std::source_location const& loc = std::source_location::current()) {
      query = database.CreateQueryObject();
      SetSQL(strSQL, loc);
      }

   bool SetSQL(std::string_view strSQL, std::source_location const& loc = std::source_location::current()) {
      return SetSQL(std::string { strSQL.begin(), strSQL.end() });
      }

   bool SetSQL(std::string const& strSQL, std::source_location const& loc = std::source_location::current()) {
      params.clear();

      static const std::regex parser("(\\:)(\\w+)(\\s|<|>|=|,|\\(|\\)|$)");
      static const std::string format("$2");
      for (auto it = std::sregex_iterator(strSQL.begin(), strSQL.end(), parser); it != std::sregex_iterator(); ++it) {
         params.emplace_back(std::regex_replace(it->str(), parser, format));
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




   std::string GetSQL(void) {
      return framework::GetSQL(query);
   }

   bool Execute(src_loc const& loc = src_loc::current()) {
      auto [ret, msg, sqlparams] = framework::Execute(query);
      if (!ret) {
         std::ostringstream query_info;
         query_info << GetSQL();
         if (params.size() > 0) {
            query_info << "\n\nParameter:";
            for (size_t i = 0; i < params.size(); ++i) query_info << std::format("\n{} = {}", params[i], sqlparams[i]);
         }
         throw TMy_Db_Exception("error while executing query", msg, database.Status(), query_info.str(), loc);
      }
      return ret;
   }

   bool Execute(my_db_params const& params, src_loc const& loc = src_loc::current()) {
      Set(params);
      return Execute(loc);
      }

   bool First(void) { return framework::First(query); }
   bool IsEof(void) { return framework::IsEof(query); }
   bool Next(void) { return framework::Next(query); }

   template <my_db_value_type ret_type>
   std::optional<ret_type> Get(std::string const& field, bool required = false, src_loc const& loc = src_loc::current()) {
      try {
         return framework::Get<ret_type>(query, field, required);
         }
      catch (std::exception& ex) {
         throw TMy_Db_Exception(std::format("error for get attribute: {}", field), ex.what(), database.Status(), GetSQL(), loc);
         }
      }

   template <my_db_param_type param_type>
   bool Set(std::string const& field_name, param_type const& param, bool required = false, src_loc const& loc = src_loc::current()) {
      if (auto found = framework::Set<param_type>(query, field_name, param); found) return found;
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




#if defined BUILD_WITH_QT
#include <QtSQL>
#include <qsqldatabase.h>

template <my_db_credentials base_type>
class TMyQtDb : public base_type {
public:
   using database_type       = QSqlDatabase;
   using database_conv       = QSqlDatabase&;
   using database_const_conv = QSqlDatabase const&;
   using database_para       = QSqlDatabase&;
   using database_const_para = QSqlDatabase const&;

   using query_type       = QSqlQuery;
   using query_conv       = QSqlQuery&;
   using query_const_conv = QSqlQuery const&;
   using query_para       = QSqlQuery&;
   using query_const_para = QSqlQuery const&;

private:
   database_type database;
public:
   TMyQtDb(void) : base_type() { }
   TMyQtDb(base_type const& ref) : base_type(ref) { }

   std::string GetInformations() const { return base_type::GetInformations();  }
   operator base_type& () { return static_cast<base_type&>(*this); }

   TMyQtDb& operator = (base_type const& ref) {
      static_cast<base_type&>(*this) = ref;
      return *this;
      }

   TMyQtDb& operator = (base_type&& ref) {
      static_cast<base_type&>(*this).swap(ref);
      return *this;
      }

   database_conv Database() { return database; }
   database_const_conv Database() const { return database; }

   
   //TMyQuery<TMyQtDb, base_type> CreateQuery(void) const {
   //   TMyQuery<TMyQtDb, base_type> query;
    query_type CreateQueryObject(void) const {
       QSqlQuery query(database);
      return query;
      }

   void swap(TMyQtDb& ref) {
      using std::swap;
      swap(database, ref.database);
      static_cast<base_type&>(*this).swap(static_cast<base_type&>(ref));
      }

   std::pair<bool, std::string>Open(void) {
      if (database.isOpen()) database.close();
      std::string param;
      base_type& srv = static_cast<base_type&>(*this);
      if constexpr (std::is_same<base_type, TMyMSSQL>::value) {

         static constexpr auto driver = []() {
            if constexpr (std::is_same<base_type, TMyMSSQL>::value)
               return "SQL Server Native Client 11.0"s;
            else if constexpr (std::is_same<base_type, TMyOracle>::value || std::is_same<base_type, TMyMySQL>::value)
               return ""s;
            else static_assert_no_supported();
            };

         database = database_type::addDatabase("QODBC");

         param = "DRIVER={"s + driver() + "};"s +
                 "SERVER="s + srv.Server() + ";"s +
                 "DATABASE="s + srv.Database() + ";"s;
         if (srv.Integrated()) param += "Trusted_Connection=yes;"s;   // Integrated Security=SSPI;
         else {
            param += "Uid="s + srv.User() + ";"s +
                     "Pwd="s + srv.Password()  + ";"s;
            }
         database.setDatabaseName(QString::fromStdString(param));
         }
      else if constexpr (std::is_same<base_type, TMyOracle>::value) {
         database = database_type::addDatabase("QOCI");
         if(srv.UseTNS()) {
            param += "Service="s + srv.Service();
            database.setHostName("");
            database.setDatabaseName(QString::fromStdString(srv.Service()));
            //database.setConnectOptions(QString::fromStdString("SERVICE_NAME="s + srv.Service()));
         }
         else {
            param += srv.Host() + ":"s + std::to_string(srv.Port()) + "/"s + srv.Database();
            database.setHostName(QString::fromStdString(srv.Host()));
            database.setPort(srv.Port());
            database.setDatabaseName(QString::fromStdString(srv.Database()));
            }
         database.setUserName(QString::fromStdString(srv.User()));
         database.setPassword(QString::fromStdString(srv.Password()));
         }
      else if constexpr (std::is_same<base_type, TMyMySQL>::value) {
         database = database_type::addDatabase("QMYSQL");
         param += srv.Host() + ":"s + std::to_string(srv.Port()) + "/"s + srv.Database();
         database.setHostName(QString::fromStdString(srv.Host()));
         database.setPort(srv.Port());
         database.setDatabaseName(QString::fromStdString(srv.Database()));
         database.setUserName(QString::fromStdString(srv.User()));
         database.setPassword(QString::fromStdString(srv.Password()));
         }
      else if constexpr (std::is_same<base_type, TMyInterbase>::value) {
         database = database_type::addDatabase("QIBASE");
         param += srv.HostName() + ":"s + std::to_string(srv.Port()) + "/"s + srv.DatabaseName();
         database.setHostName(QString::fromStdString(srv.HostName()));
         database.setPort(srv.Port());
         database.setDatabaseName(QString::fromStdString(srv.DatabaseName()));
         database.setUserName(QString::fromStdString(srv.User()));
         database.setPassword(QString::fromStdString(srv.Password()));
         database.setConnectOptions("CHARSET=WIN1254");
         //database.setConnectOptions("isc_tpb_read_committed,isc_tpb_no_rec_version");
         //database.setConnectOptions("WireCrypt=TLS");
         //database.setConnectOptions("ISC_DPB_LC_CTYPE=Latin1");
         }
      else if constexpr (std::is_same<base_type, TMySQLite>::value) {
         database = database_type::addDatabase("QSQLITE");
         database.setDatabaseName(QString::fromStdString(srv.DatabaseName()));
         }
      else static_assert_no_supported();

      if (auto ret = database.open(); ret == true) return { ret, ""s };
      else {
         return { ret, database.lastError().text().toStdString() + "\n" + param };
         }
      }

   bool Connected() {
      return database.isOpen();
      }

   void Close() {
      database.close();
      }

   bool HasLastID(void) const {
      return database.driver()->hasFeature(QSqlDriver::LastInsertId);
      }

   bool StartTransaction(void) {
      if (!database.driver()->hasFeature(QSqlDriver::Transactions)) return true; // exception for a save enviroment
      else return database.transaction();
      }

   bool Commit(void) {
      if (!database.driver()->hasFeature(QSqlDriver::Transactions)) return true;
      return database.commit();
      }

   bool Rollback(void) {
      if (!database.driver()->hasFeature(QSqlDriver::Transactions)) return true;
      return database.rollback();
      } 


   // --------------------------------------------------------------------------------------------------
   // Hilfsmethoden für Querys

   static std::pair<bool, std::string> SetSQL(query_para query, std::string const& strSQL) {
      if (auto ret = query.prepare(QString::fromLatin1(QByteArray::fromStdString(strSQL))); ret) return { ret, ""s };
      else { 
         return { ret, query.lastError().text().toStdString() };
         }
      }

   static std::pair<bool, std::string> SetSQL(query_para query, std::wstring const& strSQL) {
      if (auto ret = query.prepare(QString::fromStdWString(strSQL)); ret) return { ret, ""s };
      else {
         return { ret, query.lastError().text().toStdString() };
      }
   }


   static std::string GetSQL(query_para query) { return query.lastQuery().toStdString(); }

   static std::tuple<bool, std::string, std::vector<std::string>> Execute(query_para query) {
      if (auto ret = query.exec(); ret) return { ret, ""s, { } };
      else {
         std::vector<std::string> params;
         for (auto const& para : query.boundValues()) params.emplace_back(para.toString().toStdString());
         return { ret, query.lastError().text().toStdString(), params };
         }
      }

   static bool First(query_para query) { return query.first(); }
   static bool IsEof(query_para query) { return !query.isValid(); }
   static bool Next(query_para query) { return query.next();  }

   template <my_db_value_type ret_type>
   static std::optional<ret_type> Get(query_para query, std::string const& field, bool required) {
      if (!query.isActive() || !query.isValid()) throw std::runtime_error("Query isn't active or valid.");
      QVariant attribut = query.value(QString::fromStdString(field));
      if (!attribut.isValid()) {
         if (required) throw std::runtime_error(std::format("attribute {} is required.", field));
         else return { };
         }
      else {
         if (attribut.isNull()) return { };
         if constexpr (std::is_same<ret_type, std::string>::value) return std::make_optional(attribut.toString().toStdString());
         else if constexpr (std::is_same<ret_type, char>::value) return std::make_optional(attribut.toChar());
         else if constexpr (std::is_same<ret_type, bool>::value) return std::make_optional(attribut.toBool());
         else if constexpr (std::is_same<ret_type, int>::value) return std::make_optional(attribut.toInt());
         else if constexpr (std::is_same<ret_type, unsigned int>::value) return std::make_optional(attribut.toUInt());
         else if constexpr (std::is_same<ret_type, long long>::value) return std::make_optional(attribut.toLongLong());
         else if constexpr (std::is_same<ret_type, unsigned long long>::value) return std::make_optional(attribut.toULongLong());
         else if constexpr (std::is_same<ret_type, short>::value) {
            bool ret = true;
            if (auto val = attribut.toInt(&ret); ret && val >= std::numeric_limits<short>::min() &&
                    val <= std::numeric_limits<short>::max()) return std::make_optional(static_cast<short>(val));
            else throw std::runtime_error(std::format("{} can't converted to a short value.", val));
            }
         else if constexpr (std::is_same<ret_type, float>::value) return std::make_optional(attribut.toFloat());
         else if constexpr (std::is_same<ret_type, double>::value) return std::make_optional(attribut.toDouble());
         else if constexpr(std::is_same<ret_type, std::chrono::year_month_day>::value) {
            if constexpr (std::is_same<base_type, TMySQLite>::value) {
               QDate   date = QDate::fromString(attribut.toString(), "yyyy-MM-dd");
               return std::make_optional(std::chrono::year_month_day { std::chrono::year(date.year()),
                                         std::chrono::month(date.month()),
                                         std::chrono::day(date.day()) });
               }
            else {
               QDate date = attribut.toDateTime().date();
               return std::make_optional(std::chrono::year_month_day { std::chrono::year(date.year()),
                                                                       std::chrono::month(date.month()),
                                                                       std::chrono::day(date.day()) });
               }
            }
         else if constexpr (std::is_same<ret_type, std::chrono::system_clock::time_point>::value) {
            if constexpr (std::is_same<base_type, TMySQLite>::value) {
               QDateTime dateTime = QDateTime::fromString(attribut.toString(), Qt::ISODate);
               std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::from_time_t(dateTime.toSecsSinceEpoch());
               return std::make_optional(timePoint);
               }
            else {
               std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::from_time_t(attribut.toDateTime().toSecsSinceEpoch());
               return std::make_optional(timePoint);
               }
            }
         else static_assert_no_supported();
         }
      }

   template <my_db_param_type param_type>
   static bool Set(query_para query, std::string const& field_name, param_type const& param) {
      QString field = QString::fromStdString(":"s + field_name);
      if constexpr (is_optional<param_type>::value) {
         using used_type = typename std::remove_const<typename std::remove_reference<typename std::decay<param_type>::type::value_type>::type>::type;
         if(!param) {
            if constexpr (std::is_same<std::remove_cvref_t<used_type>, std::chrono::year_month_day>::value) {
               QVariant null_value = QVariant::fromValue<QDate>(QDate());
               query.bindValue(field, null_value);
               }
            else if constexpr (std::is_same<std::remove_cvref_t<used_type>, std::chrono::system_clock::time_point>::value) {
               QVariant null_value = QVariant::fromValue<QDateTime>(QDateTime());
               query.bindValue(field, null_value);
               }
            else {
               auto null_value = QVariant();
               query.bindValue(field, null_value);
               }
            return query.boundValue(field).isValid();
            }
         else {
            return Set(query, field_name, *param);
            }
         }
      else {
         if constexpr (std::is_same<std::remove_cvref_t<param_type>, std::string_view>::value) {
            query.bindValue(field, QString::fromLatin1(QByteArray::fromStdString(std::string { param.begin(), param.end() })));
         }
         else if constexpr (std::is_same<std::remove_cvref_t<param_type>, std::string>::value) {
            query.bindValue(field, QString::fromLatin1(QByteArray::fromStdString(param)));
            }
         else if constexpr (std::is_same<std::remove_cvref_t<param_type>, std::wstring>::value) {
            query.bindValue(field, QString::fromWStdString(param));
            }
         else if constexpr (std::is_same<std::remove_cvref_t<param_type>, std::chrono::year_month_day>::value) {
            QDate date(static_cast<int>(param.year()), static_cast<unsigned>(param.month()), static_cast<unsigned>(param.day()));
            query.bindValue(field, date);
            }
         else if constexpr (std::is_same<param_type, std::chrono::system_clock::time_point>::value) {
            QDateTime dateTime = QDateTime::fromSecsSinceEpoch(std::chrono::system_clock::to_time_t(param));
            query.bindValue(field, dateTime);
            }
         else {
            query.bindValue(field, param);
            }
         return query.boundValue(field).isValid();
         }
      }
};



class TMyFDAC { };

#elif defined BUILD_WITH_VCL || defined BUILD_WITH_FMX
#error noch nicht implementiert

#else
#error Nicht definiertes Framework
#endif




template <template <class> class framework_type, class server_type>
  // requires my_db_declaration<framework_type, server_type, int, bool, double, std::string>
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
            throw TMy_Db_Exception("database couldn't be open.", msg, framework::GetInformations(), { }, loc);
            }
         }

      void Close(void) { framework::Close(); }

      
      TMyQuery<framework_type, server_type> CreateQuery(void) {
         TMyQuery<framework_type, server_type> tmp(*this);
         tmp.Create();
         return tmp;
         }

      TMyQuery<framework_type, server_type> CreateQuery(std::string const& strSQL) {
         TMyQuery<framework_type, server_type> tmp(*this);
         tmp.Create(strSQL);
         return tmp;
         }

      std::string Status(void) {
         return std::format("{} {}", framework::Connected() ? "connected to" : "disconnected from", framework::GetInformations());
         }

      std::set<std::string> GetTableNames(std::string const& schema) {
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
               auto table = query.Get<std::string>("TableName");
               if (table)
                  setTableNames.insert(trim(*table));
               }
            else {
               setTableNames.insert(query.Get<std::string>("TableName").value_or("leer"));
               }
            }
         return setTableNames;
         }

      std::set<std::string> GetViewNames(std::string const& schema) {
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
               auto view = query.Get<std::string>("ViewName");
               if (view)
                  setViewNames.insert(trim(*view));
               }
            else {
               setViewNames.insert(query.Get<std::string>("ViewName").value_or("leer"));
               }
            }
         return setViewNames;
      }

   };
