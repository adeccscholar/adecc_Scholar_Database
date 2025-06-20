#pragma once

#include "MyDatabaseExceptions.h"
#include "MyDatabaseCredentials.h"
#include "MyDatabaseDefinitions.h"

#include <adecc_Scholar/MyType_Traits.h>

#include <type_traits>
#include <string>
using namespace std::string_literals;

#if defined BUILD_WITH_QT

#include <QtSql>
#include <QString>
#include <QSqlDatabase>

template <my_db_credentials base_type>
class TMyQtDb : public base_type {
public:
   using database_type = QSqlDatabase;
   using database_conv = QSqlDatabase&;
   using database_const_conv = QSqlDatabase const&;
   using database_para = QSqlDatabase&;
   using database_const_para = QSqlDatabase const&;

   using query_type = QSqlQuery;
   using query_conv = QSqlQuery&;
   using query_const_conv = QSqlQuery const&;
   using query_para = QSqlQuery&;
   using query_const_para = QSqlQuery const&;

private:
   database_type database;
public:
   TMyQtDb(void) : base_type() {
      //*
      if constexpr (std::is_same<base_type, TMyMSSQL>::value) {
         database = database_type::addDatabase("QODBC");
      }
      else if constexpr (std::is_same<base_type, TMyOracle>::value) {
         database = database_type::addDatabase("QOCI");
      }
      else if constexpr (std::is_same<base_type, TMyMySQL>::value) {
         database = database_type::addDatabase("QMYSQL");
      }
      else if constexpr (std::is_same<base_type, TMyInterbase>::value) {
         database = database_type::addDatabase("QIBASE");
      }
      else if constexpr (std::is_same<base_type, TMySQLite>::value) {
         database = database_type::addDatabase("QSQLITE");
      }
      else static_assert_no_supported();
      //*/
   }

   TMyQtDb(base_type const& ref) : base_type(ref) {
      /*
      if constexpr (std::is_same<base_type, TMyMSSQL>::value) {
         database = database_type::addDatabase("QODBC");
         }
      else if constexpr (std::is_same<base_type, TMyOracle>::value) {
         database = database_type::addDatabase("QOCI");
         }
      else if constexpr (std::is_same<base_type, TMyMySQL>::value) {
         database = database_type::addDatabase("QMYSQL");
         }
      else if constexpr (std::is_same<base_type, TMyInterbase>::value) {
         database = database_type::addDatabase("QIBASE");
         }
      else if constexpr (std::is_same<base_type, TMySQLite>::value) {
         database = database_type::addDatabase("QSQLITE");
         }
      else static_assert_no_supported();
      */
   }

   ~TMyQtDb() {
      //if(database.isValid()) database_type::removeDatabase(database.connectionName());
   }

   std::string GetInformations() const { return base_type::GetInformations(); }
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
#ifdef __unix__
               return "ODBC Driver 18 for SQL Server";
#else
               // return "SQL Server Native Client 11.0"s;s
               return "ODBC Driver 18 for SQL Server";
#endif
            else if constexpr (std::is_same<base_type, TMyOracle>::value || std::is_same<base_type, TMyMySQL>::value)
               return ""s;
            else static_assert_no_supported();
            };

         //database = database_type::addDatabase("QODBC");

         param = "DRIVER={"s + driver() + "};"s +
            //"SERVER=.;"s +
            "SERVER="s + srv.Server() + ";"s +
            "DATABASE="s + srv.Database() + ";"s  //;
            "MARS_Connection=Yes;"s +
            "ENCRYPT=No;SSL=No;"s;  // workaround for linux, which inforce a ssl communication, self-signed certificate problem

         if (srv.Integrated()) param += "Trusted_Connection=yes;"s;   // Integrated Security=SSPI;
         else {
            param += "Uid="s + srv.User() + ";"s +
               "Pwd="s + srv.Password() + ";"s;
            }
         database.setDatabaseName(QString::fromStdString(param));
         }
      else if constexpr (std::is_same<base_type, TMyOracle>::value) {
         database = database_type::addDatabase("QOCI");
         if (srv.UseTNS()) {
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

   bool Connected() const {
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
      //query.finish();
      if (auto ret = query.prepare(QString::fromLatin1(QByteArray::fromStdString(strSQL))); ret) return { ret, ""s };
      else {
         return { ret, query.lastError().text().toStdString() };
         }
      }

   static std::pair<bool, std::string> SetSQL(query_para query, std::wstring const& strSQL) {
      //query.finish();
      if (auto ret = query.prepare(QString::fromStdWString(strSQL)); ret) return { ret, ""s };
      else {
         return { ret, query.lastError().text().toStdString() };
         }
      }


   static std::string GetSQL(query_const_para query) { return query.lastQuery().toStdString(); }

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
   static bool Next(query_para query) { return query.next(); }

   template <my_db_value_type ret_type>
   static std::optional<ret_type> Get(query_const_para query, std::string const& field, bool required) {
      if (!query.isActive() || !query.isValid()) throw std::runtime_error("Query isn't active or valid.");
      QVariant attribut = query.value(QString::fromStdString(field));
      if (!attribut.isValid()) {
         if (required) throw std::runtime_error(std::format("attribute {} is required.", field));
         else return { };
         }
      else {
         if (attribut.isNull()) return { };
         if constexpr (std::is_same<ret_type, std::string>::value) return std::make_optional(attribut.toString().toStdString());
         else if constexpr(std::is_same<ret_type, std::wstring>::value) return std::make_optional(attribut.toString().toStdWString());
         else if constexpr (std::is_same<ret_type, char>::value) return std::make_optional(attribut.toChar());
         else if constexpr (std::is_same<ret_type, bool>::value) return std::make_optional(attribut.toBool());
         //else if constexpr (std::is_same<ret_type, short>::value) return std::make_optional(static_cast<short>(attribut.toInt()));
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
         else if constexpr (std::is_same<ret_type, std::chrono::year_month_day>::value) {
            if constexpr (std::is_same<base_type, TMySQLite>::value) {
               QDate   date = QDate::fromString(attribut.toString(), "yyyy-MM-dd");
               return std::make_optional(std::chrono::year_month_day{ std::chrono::year(date.year()),
                                         std::chrono::month(date.month()),
                                         std::chrono::day(date.day()) });
               }
            else {
               QDate date = attribut.toDateTime().date();
               return std::make_optional(std::chrono::year_month_day{ std::chrono::year(date.year()),
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
         else if constexpr(std::is_same<ret_type, std::chrono::hh_mm_ss<std::chrono::seconds>>::value) {
               int totalSeconds = 0;
               if (attribut.canConvert<QTime>()) {
                  QTime timeValue = attribut.toTime();
                  if (timeValue.isValid()) {
                     totalSeconds = timeValue.hour() * 3600 + timeValue.minute() * 60 + timeValue.second();
                     }
                  else 
                     throw std::runtime_error(std::format("attribute {} is convertible to QTime, but isn't a valid value", attribut.toString().toStdString()));
                  }
               else if (attribut.typeId() == QMetaType::QString) {
                  QTime timeValue = QTime::fromString(attribut.toString(), "hh:mm:ss");
                  if (timeValue.isValid()) {
                     totalSeconds = timeValue.hour() * 3600 + timeValue.minute() * 60 + timeValue.second();
                     }
                  else throw std::runtime_error(std::format("attribute {} is a string but can't converted time.", attribut.toString().toStdString()));
                  }
               else if (attribut.canConvert<qint64>()) {
                  totalSeconds = attribut.toLongLong();  // Direkt übernehmen
                  }
               else throw std::runtime_error("attribute can't converted to a hh_mm_ss value.");
               std::chrono::seconds chronoSeconds(totalSeconds);
               std::chrono::hh_mm_ss<std::chrono::seconds> timePoint(chronoSeconds);
               return std::make_optional(timePoint);
            }
         else static_assert_no_supported();
         }
      }

   template <my_db_param_type param_type>
   static bool Set(query_para query, std::string const& field_name, param_type const& param) {
      QString field = QString::fromStdString(":"s + field_name);
      if constexpr (is_optional<param_type>::value) {
         using used_type = typename std::remove_const<typename std::remove_reference<typename std::decay<param_type>::type::value_type>::type>::type;
         if (!param) {
            query.bindValue(field, QVariant());
            return true; // return false ;-) query.boundValue(field).isValid();
            }
         else {
            return Set(query, field_name, *param);
            }
         }
      else {
         if constexpr (std::is_same<std::remove_cvref_t<param_type>, std::string_view>::value) {
            //query.bindValue(field, QString::fromLatin1(QByteArray::fromStdString(std::string{ param.begin(), param.end() })));
            query.bindValue(field, QString::fromStdString(std::string{ param.begin(), param.end() }));
            }
         else if constexpr (std::is_same<std::remove_cvref_t<param_type>, std::string>::value) {
            //query.bindValue(field, QString::fromLatin1(QByteArray::fromStdString(param)));
            query.bindValue(field, QString::fromStdString(param));
            }
         else if constexpr (std::is_same<std::remove_cvref_t<param_type>, std::wstring>::value) {
            query.bindValue(field, QString::fromStdWString(param));
            }
         else if constexpr (std::is_same<std::remove_cvref_t<param_type>, std::chrono::year_month_day>::value) {
            QDate date(static_cast<int>(param.year()), static_cast<unsigned>(param.month()), static_cast<unsigned>(param.day()));
            query.bindValue(field, date);
            }
         else if constexpr (std::is_same<param_type, std::chrono::system_clock::time_point>::value) {
            QDateTime dateTime = QDateTime::fromSecsSinceEpoch(std::chrono::system_clock::to_time_t(param));
            query.bindValue(field, dateTime);
            }
         else if constexpr (std::is_same<param_type, std::chrono::hh_mm_ss<std::chrono::seconds>>::value) {
            QTime timeVal = QTime(param.hours().count(), param.minutes().count(), param.seconds().count());
            query.bindValue(field, timeVal);
            }
         else {
            query.bindValue(field, param);
            }
         return query.boundValue(field).isValid();
         }
   }


};



class TMyFDAC {};

#elif defined BUILD_WITH_VCL || defined BUILD_WITH_FMX
#error noch nicht implementiert

#else
#error Nicht definiertes Framework
#endif

