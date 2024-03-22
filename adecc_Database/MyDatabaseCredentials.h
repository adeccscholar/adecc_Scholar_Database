#pragma once
#include <string>
#include <format>

using namespace std::literals;

class TMyCredential {
   friend void swap(TMyCredential& lhs, TMyCredential& rhs) noexcept { lhs.swap(rhs);  }
private:
   std::tuple<std::string, std::string, bool> data;
   //std::string strUser;       ///< user name for server credentials
   //std::string strPassword;   ///< password for server credentials
public:
   constexpr TMyCredential(void) : TMyCredential(""s, ""s, false) { }
   constexpr TMyCredential(std::string const& usr, std::string const& pwd, bool isec = false) : data { usr, pwd, isec } { }
   TMyCredential(TMyCredential const& ref) : data(ref.data) { }
   TMyCredential(TMyCredential&& ref) noexcept { swap(ref); }
   virtual ~TMyCredential(void) = default;

   TMyCredential& operator = (TMyCredential const& ref) {
      data = ref.data;
      return *this;
      }

   TMyCredential& operator = (TMyCredential&& ref) noexcept {
      swap(ref);
      return *this;
      }

   auto operator <=> (TMyCredential const& ref) const = default;

   void swap(TMyCredential& ref) noexcept {
      using std::swap;
      swap(data, ref.data);
      }

   std::string const& User(void) const { return std::get<0>(data); }
   std::string const& Password(void) const { return std::get<1>(data); }
   bool               Integrated(void) const { return std::get<2>(data); }

   std::string GetCredential(bool boSecure = true) const {
      if (Integrated()) return "connected with integrated security"s;
      else return std::format("User: {} Password: {}", User(), boSecure ? "*****" : Password());
      }

};


/// class to manage the Credentials for MS SQL Server Connections
class TMyMSSQL : public TMyCredential {
   friend void swap(TMyMSSQL& lhs, TMyMSSQL& rhs) noexcept { lhs.swap(rhs); }
private:
   std::string strServer;     ///< Database server, optional with instance name
   std::string strDatabase;   ///< Database name
public:
   constexpr TMyMSSQL(std::string const& s, std::string const& d, bool b, std::string const& u, std::string const& p) :
      TMyCredential(u, p, b), strServer(s), strDatabase(d) { }

   constexpr TMyMSSQL(void) : TMyMSSQL(""s, ""s, false, ""s, ""s) { }
   explicit constexpr TMyMSSQL(std::string const& data) : TMyMSSQL("(local)", data, true, ""s, ""s) { }
   constexpr TMyMSSQL(std::string const& srv, std::string const& data) : TMyMSSQL(srv, data, true, ""s, ""s) { }

   TMyMSSQL(TMyMSSQL const& ref) : TMyCredential(ref) , strServer(ref.strServer), strDatabase(ref.strDatabase) { }
   TMyMSSQL(TMyMSSQL&& ref) noexcept { swap(ref); }
   virtual ~TMyMSSQL(void) { }

   TMyMSSQL& operator = (TMyMSSQL const& ref) {
      strServer = ref.strServer;
      strDatabase = ref.strDatabase;
      static_cast<TMyCredential&>(*this).operator = (static_cast<TMyCredential const&>(ref));
      return *this;
      }

   TMyMSSQL& operator = (TMyMSSQL&& ref) noexcept {
      swap(ref);
      return *this;
      }

   TMyMSSQL& operator += (TMyCredential const& ref) {
      static_cast<TMyCredential&>(*this).operator = (static_cast<TMyCredential const&>(ref));
      return *this;
      }

   TMyMSSQL& operator += (TMyCredential&& ref) noexcept {
      static_cast<TMyCredential&>(*this).swap(ref);
      return *this;
      }

   void swap(TMyMSSQL& ref) noexcept {
      using std::swap;
      static_cast<TMyCredential&>(*this).swap(static_cast<TMyCredential&>(ref));
      swap(strServer, ref.strServer);
      swap(strDatabase, ref.strDatabase);
   }

   std::string const& Server(void) const { return strServer; }
   std::string const& Database(void) const { return strDatabase; }

   static constexpr std::string ServerType() { return "MS SQL"s; }

   std::string GetDatabase(void) const {
      return std::format("{}/{}", Server(), Database());
      }

   std::string GetServer(void) const {
      return std::format("{} {}", ServerType(), GetDatabase());
      }

   std::string GetInformations(void) const {
      return std::format("{} ({})", GetServer(), GetCredential(true));
      }

   static bool HasIntegratedSecurity() { return true; }
   static bool HasCredentials() { return true; }
};



class TMyOracle : public TMyCredential {
   friend void swap(TMyOracle& lhs, TMyOracle& rhs) { lhs.swap(rhs);  }
private:
   bool        boUseTNS    = true;
   std::string strService  = "XE";
   std::string strHost     = "localhost";
   int         iPort       = 1521;
   std::string strDatabase = "XE";
public:
   constexpr TMyOracle(bool tns, std::string const& hst, int prt, std::string const& sv, std::string const& db, std::string const& usr, std::string const& pwd) :
      TMyCredential(usr, pwd), boUseTNS(tns), strHost(hst), iPort(prt), strService(sv), strDatabase(db) { }

   TMyOracle(std::string const& hst, int prt, std::string const& db, std::string const& usr, std::string const& pwd) :
      TMyOracle(false, hst, prt, ""s, db, usr, pwd) { }

   TMyOracle() : TMyOracle(false, "localhost"s, 1521, ""s, ""s, ""s, ""s) { }
   TMyOracle(std::string const& db, std::string const& usr, std::string const& pwd) :
      TMyOracle(false, "localhost"s, 1521, ""s, db, usr, pwd) { }
   TMyOracle(bool usetns, std::string const& d, std::string const& usr, std::string const& pwd) :
      TMyOracle(usetns, "localhost", 1521, d, d, usr, pwd) { }

   TMyOracle(TMyOracle const& ref) : TMyCredential(ref), boUseTNS(ref.boUseTNS), strService(ref.strService),
      strHost(ref.strHost), iPort(ref.iPort), strDatabase(ref.strDatabase) {  }
 
   TMyOracle(TMyOracle&& ref) noexcept { swap(ref); }

   TMyOracle& operator = (TMyOracle const& ref) {
      boUseTNS = ref.boUseTNS;
      strService = ref.strService;
      strHost = ref.strHost;
      iPort = ref.iPort;
      strDatabase = ref.strDatabase;
      static_cast<TMyCredential&>(*this).operator = (static_cast<TMyCredential const&>(ref));
      return *this;
      }

   TMyOracle& operator = (TMyOracle&& ref) noexcept {
      swap(ref);
      return *this;
      }

   TMyOracle& operator += (TMyCredential const& ref) {
      static_cast<TMyCredential&>(*this).operator = (static_cast<TMyCredential const&>(ref));
      return *this;
      }

   TMyOracle& operator += (TMyCredential&& ref) noexcept {
      static_cast<TMyCredential&>(*this).swap(ref);
      return *this;
      }


   void swap(TMyOracle& ref) noexcept {
      using std::swap;
      static_cast<TMyCredential&>(*this).swap(static_cast<TMyCredential&>(ref));
      swap(boUseTNS, ref.boUseTNS);
      swap(strService, ref.strService);
      swap(strHost, ref.strHost);
      swap(iPort, ref.iPort);
      swap(strDatabase, ref.strDatabase);
      }

   bool               UseTNS(void) const { return boUseTNS; }
   std::string const& Service(void) const { return strService; }
   std::string const& Host(void) const { return strHost; }
   int                Port(void) const { return iPort; }
   std::string const& Database(void) const { return strDatabase; }

   static constexpr std::string ServerType() { return "Oracle"s; }

   std::string GetDatabase(void) const {
      if (boUseTNS)
         return std::format("Service={}", strService);
      else
         return std::format("{}:{}/{}", strHost, iPort, strDatabase);
   }

   std::string GetServer(void) const {
      return std::format("{} {}", ServerType(), GetDatabase());
      }

   std::string GetInformations(void) const {
      return std::format("{} ({})", GetServer(), GetCredential(true));
      }

   static bool HasIntegratedSecurity() { return false; }
   static bool HasCredentials() { return true; }
};



class TMyMySQL : public TMyCredential {
   friend void swap(TMyMySQL& lhs, TMyMySQL& rhs) { lhs.swap(rhs); }
private:
   std::string strHost;
   int         iPort;
   std::string strDatabase;
public:
   constexpr TMyMySQL(std::string const& hst, int prt, std::string const& db, std::string const& usr, std::string const& pwd) :
       TMyCredential(usr, pwd), strHost(hst), iPort(prt), strDatabase(db) { }
   TMyMySQL() : TMyMySQL("localhost"s, 3306, ""s, ""s, ""s) { }
   TMyMySQL(std::string const& d, std::string const& usr, std::string const& pwd) :
      TMyMySQL("localhost", 3306, d, usr, pwd) { }

   TMyMySQL(TMyMySQL const& ref) : TMyCredential(ref), strHost(ref.strHost), iPort(ref.iPort), strDatabase(ref.strDatabase) { }

   TMyMySQL(TMyMySQL&& ref) noexcept { swap(ref); }


   TMyMySQL& operator = (TMyMySQL const& ref)  {
      strHost = ref.strHost;
      iPort = ref.iPort;
      strDatabase = ref.strDatabase;
      static_cast<TMyCredential&>(*this).operator = (static_cast<TMyCredential const&>(ref));
      return *this;
      }

   TMyMySQL& operator = (TMyMySQL&& ref) noexcept {
      swap(ref);
      return *this;
      }

   TMyMySQL& operator += (TMyCredential const& ref) {
      static_cast<TMyCredential&>(*this).operator = (static_cast<TMyCredential const&>(ref));
      return *this;
      }

   TMyMySQL& operator += (TMyCredential&& ref) noexcept {
      static_cast<TMyCredential&>(*this).swap(ref);
      return *this;
      }

   void swap(TMyMySQL& ref) noexcept {
      using std::swap;
      swap(strHost, ref.strHost);
      swap(iPort, ref.iPort);
      swap(strDatabase, ref.strDatabase);
      }

   std::string const& Host(void) const { return strHost; }
   int                Port(void) const { return iPort; }
   std::string const& Database(void) const { return strDatabase; }

   static constexpr std::string ServerType() { return "MySQL"s; }

   std::string GetDatabase(void) const {
      return std::format("{} at {}:{}", strDatabase, strHost, iPort);
      }

   std::string GetServer(void) const { 
      return std::format("{1:} at {0:}", ServerType(), GetDatabase()); 
      }

   std::string GetInformations(void) const {
      return std::format("{} @ {})", GetServer(), User());
      }

   static bool HasIntegratedSecurity() { return false; }
   static bool HasCredentials() { return true; }
};


class TMyInterbase : public TMyCredential {
   friend void swap(TMyInterbase& lhs, TMyInterbase& rhs) { lhs.swap(rhs); }
private:
   std::string strHostName;
   int         iPort;
   std::string strDatabaseName;
public:
   constexpr TMyInterbase(std::string const& hst, int prt, std::string const& db, std::string const& usr, std::string const& pwd) :
      TMyCredential(usr, pwd), strHostName(hst), iPort(prt), strDatabaseName(db) { }
   TMyInterbase() : TMyInterbase("localhost"s, 3050, ""s, ""s, ""s) { }
   TMyInterbase(std::string const& d, std::string const& usr, std::string const& pwd) :
      TMyInterbase("localhost", 3050, d, usr, pwd) { }

   TMyInterbase(TMyInterbase const& ref) : TMyCredential(ref), strHostName(ref.strHostName), iPort(ref.iPort), strDatabaseName(ref.strDatabaseName) { }

   TMyInterbase(TMyInterbase&& ref) noexcept { swap(ref); }


   TMyInterbase& operator = (TMyInterbase const& ref) {
      strHostName = ref.strHostName;
      iPort = ref.iPort;
      strDatabaseName = ref.strDatabaseName;
      static_cast<TMyCredential&>(*this).operator = (static_cast<TMyCredential const&>(ref));
      return *this;
   }

   TMyInterbase& operator = (TMyInterbase&& ref) noexcept {
      swap(ref);
      return *this;
   }

   TMyInterbase& operator += (TMyCredential const& ref) {
      static_cast<TMyCredential&>(*this).operator = (static_cast<TMyCredential const&>(ref));
      return *this;
   }

   TMyInterbase& operator += (TMyCredential&& ref) noexcept {
      static_cast<TMyCredential&>(*this).swap(ref);
      return *this;
   }

   void swap(TMyInterbase& ref) noexcept {
      using std::swap;
      swap(strHostName, ref.strHostName);
      swap(iPort, ref.iPort);
      swap(strDatabaseName, ref.strDatabaseName);
   }

   std::string const& HostName(void) const { return strHostName; }
   int                Port(void) const { return iPort; }
   std::string const& DatabaseName(void) const { return strDatabaseName; }

   static constexpr std::string ServerType() { return "Interbase"s; }

   std::string GetDatabase(void) const {
      return std::format("{} at {}:{}", strDatabaseName, strHostName, iPort);
   }

   std::string GetServer(void) const {
      return std::format("{1:} at {0:}", ServerType(), GetDatabase());
   }

   std::string GetInformations(void) const {
      return std::format("{} @ {})", GetServer(), User());
   }

   static bool HasIntegratedSecurity() { return false; }
   static bool HasCredentials() { return true; }
};

class TMySQLite : public TMyCredential {
   friend void swap(TMySQLite& lhs, TMySQLite& rhs) { lhs.swap(rhs); }
private:
   std::string strDatabaseName;
public:
   constexpr TMySQLite(std::string const& db, std::string const& usr, std::string const& pwd) :
      TMyCredential(usr, pwd), strDatabaseName(db) { }
   TMySQLite() : TMySQLite(""s, ""s, ""s) { }
   explicit TMySQLite(std::string const& d) : TMySQLite(d, ""s, ""s) { }

   TMySQLite(TMySQLite const& ref) : TMyCredential(ref), strDatabaseName(ref.strDatabaseName) { }

   TMySQLite(TMySQLite&& ref) noexcept { swap(ref); }


   TMySQLite& operator = (TMySQLite const& ref) {
      strDatabaseName = ref.strDatabaseName;
      static_cast<TMyCredential&>(*this).operator = (static_cast<TMyCredential const&>(ref));
      return *this;
      }

   TMySQLite& operator = (TMySQLite&& ref) noexcept {
      swap(ref);
      return *this;
      }

   TMySQLite& operator += (TMyCredential const& ref) {
      static_cast<TMyCredential&>(*this).operator = (static_cast<TMyCredential const&>(ref));
      return *this;
      }

   TMySQLite& operator += (TMyCredential&& ref) noexcept {
      static_cast<TMyCredential&>(*this).swap(ref);
      return *this;
      }

   void swap(TMySQLite& ref) noexcept {
      using std::swap;
      swap(strDatabaseName, ref.strDatabaseName);
      }

   std::string const& DatabaseName(void) const { return strDatabaseName; }

   static constexpr std::string ServerType() { return "SQLite"s; }

   std::string GetDatabase(void) const {
      return std::format("{}", strDatabaseName);
   }

   std::string GetServer(void) const {
      return std::format("{0:} with {1:}", ServerType(), GetDatabase());
   }

   std::string GetInformations(void) const {
      return std::format("{}", GetServer());
   }

   static bool HasIntegratedSecurity() { return true; }
   static bool HasCredentials() { return false; }
};





template <typename ty>
struct is_my_db_credentials {
   static constexpr bool value = std::is_same<std::remove_cvref_t<ty>, TMyMSSQL>::value ||
                                 std::is_same<std::remove_cvref_t<ty>, TMyOracle>::value ||
                                 std::is_same<std::remove_cvref_t<ty>, TMyMySQL>::value ||
                                 std::is_same<std::remove_cvref_t<ty>, TMySQLite>::value ||
                                 std::is_same<std::remove_cvref_t<ty>, TMyInterbase>::value;
};

template <typename ty>
concept my_db_credentials = 
                  is_my_db_credentials<ty>::value &&
                  std::is_base_of<TMyCredential, ty>::value &&
                  std::is_default_constructible<ty>::value &&
                  std::is_copy_constructible<ty>::value &&
                  std::is_move_constructible<ty>::value &&
                  std::has_virtual_destructor<ty>::value &&
                  std::is_assignable<ty&, ty const&>::value &&
                  std::is_assignable<ty&, ty&&>::value &&
           requires (ty t) {
                    { ty::ServerType() } -> std::convertible_to<const std::string>;
                    { t.GetDatabase() } -> std::same_as<std::string>;
                    { t.GetServer() } -> std::same_as<std::string>;
                    { t.GetInformations() } -> std::same_as<std::string>;
                    { ty::HasIntegratedSecurity() } -> std::same_as<bool>;
                    { ty::HasCredentials() } -> std::same_as<bool>;
                 requires requires(TMyCredential const p, TMyCredential && pref) {
                    { t += p } -> std::same_as<ty&>;
                    { t += std::move(pref) } -> std::same_as<ty&>;
                 };
           };

