/* -*- c++ -*-
 * $Id: Cookie.h,v 1.20 2006/05/16 20:06:34 brook Exp $
 */

/*
 * ClearSilver++ Software License.
 *
 * Copyright (c) 2005,2006 Brook Milligan <brook@nmsu.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef Cookie_h_
#define Cookie_h_ 1

#include <string>
#include <time.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace ClearSilver
{

class Cookie
{

public:

    class Authority
    {
    public:
        Authority ();
        explicit Authority (const char*);
        explicit Authority (const std::string&);
        Authority (const Authority&);
        ~Authority () throw();

        Authority& operator = (const Authority&);

        void swap (Authority&) throw();

        bool empty () const;

        std::string operator () () const;

    private:
        std::string authority_;
    };

public:

    class Path
    {
    private:
        std::string path_;

    public:
        Path ();
        explicit Path (const char*);
        explicit Path (const std::string&);
        Path (const Path&);
        ~Path () throw();

        Path& operator = (const Path&);

        void swap (Path&) throw();

        bool empty () const;

        std::string operator () () const;
    };

public:

    class Expires
    {
    private:
        std::string expires_;

    public:
        Expires ();
        explicit Expires (const char*);
        explicit Expires (const std::string&);
        explicit Expires (time_t);
        explicit Expires (struct tm);
        explicit Expires (const boost::posix_time::ptime&);
        Expires (const Expires&);
        ~Expires () throw();

        Expires& operator = (const Expires&);

        void swap (Expires&) throw();
        operator bool () const;
        bool empty () const;

        std::string operator () () const;
    };

public:

    class Credentials
    {
    private:
        Authority authority_;
        Path path_;
        Expires expires_;
        bool persist_;
        bool secure_;

    public:
        Credentials ();
        explicit Credentials (const Authority&);
        explicit Credentials (const Path&);
        explicit Credentials (const Expires&);
        explicit Credentials (const Authority&, const Path&);
        explicit Credentials (const Authority&, const Path&, const Expires&);
        Credentials (const Credentials&);
        ~Credentials () throw();

        Credentials& operator = (const Credentials&);

        void swap (Credentials&) throw();
        Authority  authority () const;
        Authority& authority ();
        Path  path () const;
        Path& path ();
        Expires  expires () const;
        Expires& expires ();
        bool persist () const;
        bool  secure () const;
        bool& secure ();

        std::string to_string () const;
    };

private:
    std::string name_;
    std::string value_;
    Credentials credentials_;

public:
    // Default constructor.
    Cookie ();
    explicit Cookie (const char* name);
    explicit Cookie (const char* name, const Credentials&);
    explicit Cookie (const char* name, const char* value);
    explicit Cookie (const char* name, const char* value, const Credentials&);
    explicit Cookie (const std::string& name);
    explicit Cookie (const std::string& name, const Credentials&);
    explicit Cookie (const std::string& name, const std::string& value);
    explicit Cookie (const std::string& name, const std::string& value,
                     const Credentials&);
    Cookie (const Cookie&);
    ~Cookie () throw();

    Cookie& operator = (const Cookie&);

    void swap (Cookie&) throw();
    std::string name () const;
    std::string value () const;
    Credentials  credentials () const;
    Credentials& credentials ();

    std::string to_string () const;
};

};                              // namespace ClearSilver

#endif
