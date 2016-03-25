#include "DB.h"
#include "Config.h"

mongo::DB::ConnectionOptionsMap mongo::DB::connection_options_map_;

mongo::DB::DB(const std::string &name) :
    socketTimeout(0.0),
    db_( 0 )
{
    options_ = options_by_name(name);
    if (!options_)
        throw NotRegistered("Database " +
                            (name.empty()? "(default)" : name) +
                            " is not registered! Use addDatabase() first.");
#ifdef MONGO_2_0
    if (options_->replica_set.empty())
        db_ = new ScopedDbConnection(options_->server_host, socketTimeout);
    else
    {
        db_ = new ScopedDbConnection(
            mongo::ConnectionString(mongo::ConnectionString::SET,
                                    options_->server_host,
                                    options_->replica_set), socketTimeout);
    }
#else
    if (options_->replica_set.empty())
        db_ = ScopedDbConnection::getScopedDbConnection(options_->server_host, socketTimeout);
    else
    {
        db_ = ScopedDbConnection::getScopedDbConnection(
                  mongo::ConnectionString(mongo::ConnectionString::SET,
                                          options_->server_host,
                                          options_->replica_set), socketTimeout);
    }
#endif // MONGO_2
}

mongo::DB::~DB()
{
    db_->done();
    delete db_;
}

void mongo::DB::addDatabase(const std::string &name,
                            const std::string &server_host,
                            const std::string &database,
                            bool slave_ok)
{
    _addDatabase(name, server_host, database, "", slave_ok);
}

/** Регистрирует подключение \a name к набору реплик баз данных
(Replica Set). */
void mongo::DB::addDatabase(const std::string &name,
                            const ReplicaSetConnection &connection_string,
                            const std::string &database,
                            bool slave_ok)
{
    _addDatabase(name, connection_string.connection_string(), database,
                 connection_string.replica_set(), slave_ok);
}

/** Регистрирует базу данных "по умолчанию". */
void mongo::DB::addDatabase(const std::string &server_host,
                            const std::string &database,
                            bool slave_ok)
{
    _addDatabase("", server_host, database, "", slave_ok);
}

/** Регистрирует базу данных "по умолчанию", подключение осуществляется
к набору реплик (Replica Set). */
void mongo::DB::addDatabase(const ReplicaSetConnection &connection_string,
                            const std::string &database, bool slave_ok)
{
    _addDatabase("", connection_string.connection_string(), database,
                 connection_string.replica_set(), slave_ok);
}

/** Возвращает полное название коллекции */
std::string mongo::DB::collection(const std::string &collection)
{
    return database() +  "." + collection;
}

/** Название используемой базы данных */
std::string &mongo::DB::database()
{
    return options_->database;
}

/** Адрес сервера MongoDB */
std::string &mongo::DB::server_host()
{
    return options_->server_host;
}

/** Название набора реплик (replica set) */
std::string &mongo::DB::replica_set()
{
    return options_->replica_set;
}

/** Возвращает true, если допускается read-only подключение к slave серверу в кластере */
bool mongo::DB::slave_ok()
{
    return options_->slave_ok;
}

/** Возвращает соединение к базе данных.
Может использоваться для операций, не предусмотренных обёрткой.
 */
mongo::ScopedDbConnection &mongo::DB::db()
{
    return *db_;
}

// Все нижеследующие методы являются просто обёртками над методами
// DBClientConnection, принимающие вместо параметра ns (namespace)
// параметр coll (collection) и автоматически добавляющие к названию
// коллекции имя базы данных.
//
// Некоторые методы (insert, update, remove) принимают также
// дополнительный параметр safe, который, если равен true, заставляет
// ждать ответа от базы данных, таким образом гарантируя выполнение
// операции (особенность mongodb в асинхронном выполнении команд,
// т.е. по умолчанию метод завершится сразу, а реальное изменение
// данных может произойти позже)
//
// Если при создании подключения был указан параметр slave_ok = true,
// то функции чтения (query, findOne, count) всегда будут устанавливать
// бит QueryOption_SlaveOk.

void mongo::DB::insert( const std::string &coll, BSONObj obj, bool safe )
{
    (*db_)->insert(this->collection(coll), obj);
    if (safe)
        (*db_)->getLastError();
}

mongo::DB::ConnectionOptions *mongo::DB::options_by_name(const std::string &name)
{
    ConnectionOptionsMap::const_iterator it =
        connection_options_map_.find(name);
    if (it != connection_options_map_.end())
        return it->second;
    else
        return 0;
}

/** Добавляет настройки подключения */
void mongo::DB::_addDatabase(const std::string &name,
                             const std::string &server_host,
                             const std::string &database,
                             const std::string &replica_set,
                             bool slave_ok)
{
    ConnectionOptions *options = options_by_name(name);
    if (!options)
        options = new ConnectionOptions;
    else
    {
        Log::warn("Database %s is already registered. Old connection will be overwritten.",
                  (name.empty()? "(default)" : name).c_str());
    };

    options->database = database;
    options->server_host = server_host;
    options->replica_set = replica_set;
    options->slave_ok = slave_ok;
    connection_options_map_[name] = options;
}

