#pragma once

#include "adecc_Tools/MyExceptions.h"

class TMy_Db_Exception : public TMyExceptionInformation, public std::runtime_error {
private:
   std::string                the_details;
   std::optional<std::string> the_database_infos;
   std::optional<std::string> the_query_infos;

   mutable std::string strMessage; ///< Hilfsviable um Speicherdauer der Rückgabe sicherzustellen

public:
   TMy_Db_Exception(std::string const& msg, std::string const& det, std::optional<std::string> const& db = { },
                    std::optional<std::string> const& qry = { },
                    src_loc const& loc = src_loc::current(),
                    time_stamp timepoint = std::chrono::system_clock::now())
              : TMyExceptionInformation(loc, timepoint), std::runtime_error(msg) {
      the_details = det;
      the_database_infos = db;
      the_query_infos = qry;
      }

   std::string                const& details() const { return the_details; }
   std::optional<std::string> const& database_infos() const { return  the_database_infos; }
   std::optional<std::string> const& query_infos() const { return the_query_infos; }

   std::string                       position() const { return TimePosition();  }

   std::string information(void) const {
      std::string ret = std::format("database error: {}\n{}\n\n", std::runtime_error::what(), the_details);
      if (the_database_infos) ret += std::format("database:\n{}\n\n", *the_database_infos);
      if (the_query_infos) ret += std::format("statement:\n{}\n\n", *the_query_infos);
      ret += TimePosition();
      return ret;
      }

   const char* what() const noexcept override {
      strMessage = std::format("{}\n{}", std::runtime_error::what(), TimePosition());
      return strMessage.c_str();
      }
};