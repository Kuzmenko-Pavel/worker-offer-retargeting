#ifndef DB_H
#define DB_H

#include "../config.h"

#include <mongo/client/connpool.h>
#include "Log.h"
#include <string>
#include <utility>

namespace mongo
{

/** \brief Класс-обёртка над MongoDB.

    Этот класс нужен для упрощения доступа к базам данных MongoDB. Во-первых,
    вводится понятие "текущей базы данных", как это сделано, например, в
    python или javascript драйверах mongo. Во-вторых, все подключения
    сосредотачиваются в одном месте.

    Все подключения регистрируются с помощью статического метода addDatabase.
    Существует одно безымянное подключение "по умолчанию" и произвольное
    количество именованных подключений. Получить именованный экземпляр можно
    передав в конструктор имя базы данных.

    Например:

    // В начале программы регистрируем и инициализируем необходимые подключения
    DB::addDatabase("localhost", "getmyad_db");   // безымянное подключение
    DB::addDatabase("db2", "localhost", "data2"); // подключение с именем "db2"

    // Теперь в любом месте программы можем использовать подключения:
    DB db;		    // Подключение по умолчанию
    DB db2("db2");	    // Именованное подключение
    default.findOne("collection", QUERY());
    db2.remove("trash", QUERY());


    Возможно также подключаться к кластеру баз данных (Replica Set). Для этого
    нужно использовать версии методов addDatabase, принимающие в качестве
    параметра класс ReplicaSetConnection.

 */


class DB
{
public:
    double socketTimeout;
    class ConnectionOptions
    {
        public:
        ConnectionOptions() : slave_ok(false) {}
        std::string database;
        std::string server_host;
        std::string replica_set;
        bool slave_ok;
    };

       /** Описывает строку подключения к MongoDB Replica Set. */
    class ReplicaSetConnection
    {
        std::string replica_set_;
        std::string connection_string_;
    public:
        /** Создаёт строку подключения к MongoDB Replica Set.

            \a replica_set -- название replica_set
            \a connection_string -- строка подключения вида
        			    "server1[:port1],server2[:port2],..." */
        ReplicaSetConnection(const std::string &replica_set,
                             const std::string &connection_string)
            : replica_set_(replica_set),
              connection_string_(connection_string) { }

        std::string replica_set() const
        {
            return replica_set_;
        }
        std::string connection_string() const
        {
            return connection_string_;
        }
    };
public:
    DB(const std::string &name = std::string());
    ~DB();


    /** Регистрирует подключение \a name к базе данных.

    Строка подключения передаётся в параметре \a server_host и имеет вид
    "server[:port]".

    После регистрации можно	получать подключение с этими параметрами через
    конструктор (см. описание класса). */
    static void addDatabase(const std::string &name,
                            const std::string &server_host,
                            const std::string &database,
                            bool slave_ok);

    /** Регистрирует подключение \a name к набору реплик баз данных
    (Replica Set). */
    static void addDatabase(const std::string &name,
                            const ReplicaSetConnection &connection_string,
                            const std::string &database,
                            bool slave_ok);

    /** Регистрирует базу данных "по умолчанию". */
    static void addDatabase(const std::string &server_host,
                            const std::string &database,
                            bool slave_ok);

    /** Регистрирует базу данных "по умолчанию", подключение осуществляется
    к набору реплик (Replica Set). */
    static void addDatabase(const ReplicaSetConnection &connection_string,
                            const std::string &database, bool slave_ok);

    /** Возвращает полное название коллекции */
    std::string collection(const std::string &collection);

    bool createCollection(const string &coll, long long size = 0,
                          bool capped = false, int max = 0, BSONObj *info = 0) {
        return (*db_)->createCollection(this->collection(coll), size,
                                     capped, max, info);
    }



    /** Название используемой базы данных */
    std::string &database();

    /** Адрес сервера MongoDB */
    std::string &server_host();

    /** Название набора реплик (replica set) */
    std::string &replica_set();

    /** Возвращает true, если допускается read-only подключение к slave серверу в кластере */
    bool slave_ok();

    /** Возвращает соединение к базе данных.
    Может использоваться для операций, не предусмотренных обёрткой.
     */
    ScopedDbConnection &db();

    static bool ConnectLogDatabase();

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

    void insert( const string &coll, BSONObj obj, bool safe = false );

    /** Исключение, возникающее при попытке обратиться к незарегистированной
    базе данных */
    class NotRegistered : public std::exception
    {
    public:
        NotRegistered(const std::string &str) : error_message(str) { }
        NotRegistered(const char *str) : error_message(str) { }
        virtual ~NotRegistered() throw() { }
        virtual const char *what() const throw()
        {
            return error_message.c_str();
        }
    private:
        std::string error_message;
    };

private:
    ScopedDbConnection *db_;
    static bool fConnectedToLogDatabase;


protected:
    typedef std::map<std::string, ConnectionOptions*> ConnectionOptionsMap;
    static ConnectionOptionsMap connection_options_map_;
    ConnectionOptions *options_;

    /** Возвращает настройки для базы данных с именем \a name.
    Если база данных с таким именем не была добавлена, вернёт 0. */
    static ConnectionOptions *options_by_name(const std::string &name);

    /** Добавляет настройки подключения */
    static void _addDatabase(const std::string &name,
                             const std::string &server_host,
                             const std::string &database,
                             const std::string &replica_set,
                             bool slave_ok);
};




}  // end namespace mongo
#endif // DB_H
