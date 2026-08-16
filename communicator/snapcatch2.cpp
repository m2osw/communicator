// Copyright (c) 2013-2026  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/snaplogger
// contact@m2osw.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

/** \file
 * \brief Test extensions helper function implementation.
 *
 * Implementation of various helper functions.
 */

// self
//
#include    <communicator/snapcatch2.hpp>


// snaplogger
//
#include    <snaplogger/message.h>


// snapdev
//
#include    <snapdev/file_contents.h>
#include    <snapdev/pathinfo.h>



namespace communicator
{



/** \brief Verify the name of a service.
 *
 * This function can be used by a service test to verify that a service
 * is properly named. The communicator expects a file named:
 *
 * \code
 *     <service-name>.service
 * \endcode
 *
 * in its `/usr/share/communicator/services` folder.
 *
 * \param[in] source_path  The path to the source directory.
 * \param[in] messenger_path  The path to the messenger file to verify;
 *                            if empty use "daemon/messenger.cpp".
 * \param[in] library_path  Optionally, the path to the library since the
 * messenger may be using a name defined in the names.an file.
 *
 * \return true if the required service is properly named.
 */
bool verify_service_name(
      std::string const & source_path
    , std::string messenger_path
    , std::string const & library_path)
{
    if(source_path.empty())
    {
        SNAP_LOG_ERROR
            << "source_path cannot be empty."
            << SNAP_LOG_SEND;
        return false;
    }

    if(messenger_path.empty())
    {
        messenger_path = "daemon/messenger.cpp";
    }
    if(snapdev::pathinfo::is_relative(messenger_path))
    {
        messenger_path = snapdev::pathinfo::canonicalize(source_path, messenger_path);
    }

    snapdev::file_contents messenger_cpp(messenger_path);
    if(!messenger_cpp.exists())
    {
        SNAP_LOG_ERROR
            << "messenger file \""
            << messenger_path
            << "\" not found."
            << SNAP_LOG_SEND;
        return false;
    }

    if(!messenger_cpp.read_all())
    {
        SNAP_LOG_ERROR
            << "error reading messenger file \""
            << messenger_path
            << "\"."
            << SNAP_LOG_SEND;
        return false;
    }

    std::string const & messenger_data(messenger_cpp.contents());

    for(char const * m(messenger_data.c_str()); *m != '\0'; ++m)
    {
        if(*m == '\n')
        {
            for(++m; isspace(*m); ++m);
            if(*m == ':' || *m == ',')
            {
                for(++m; isspace(*m); ++m);

                // the name must be "fluid_settings_connection"
                // or for a few "communicator_connection"
                //
                bool found(false);
                if(strncmp(m, "fluid_settings_connection(", 26) == 0)
                {
                    m += 26;
                    found = true;
                }
                else if(strncmp(m, "communicator_connection(", 24) == 0)
                {
                    m += 24;
                    found = true;
                }
                if(found)
                {
                    // look for the next comma
                    //
                    for(; *m != ',' && *m != '\0'; ++m);
                    if(*m != ',')
                    {
                        SNAP_LOG_ERROR
                            << "no comma found after the connection constructor name."
                            << SNAP_LOG_SEND;
                        return false;
                    }
                    for(++m; isspace(*m); ++m);
                    std::string service_name;
                    if(*m == '"')
                    {
                        // the name is defined in a string, read the string
                        //
                        for(++m; *m != '"'; ++m)
                        {
                            // no support for backslashes (escaped characters)
                            //
                            if(*m == '\\')
                            {
                                SNAP_LOG_ERROR
                                    << "service name string cannot include a backslash."
                                    << SNAP_LOG_SEND;
                                return false;
                            }
                            service_name += *m;
                        }

                        if(service_name.empty())
                        {
                            SNAP_LOG_ERROR
                                << "the service name string cannot be empty in constructor."
                                << SNAP_LOG_SEND;
                            return false;
                        }
                    }
                    else
                    {
                        auto is_identifier_character = [](char c)
                        {
                            return (c >= '0' && c <= '9')
                                || (c >= 'a' && c <= 'z')
                                || (c >= 'A' && c <= 'Z')
                                || c == '_';
                        };
                        std::string name_space;
                        std::string variable_name;
                        for(; is_identifier_character(*m) || *m == ':'; ++m)
                        {
                            if(m[0] == ':' && m[1] == ':')
                            {
                                if(!name_space.empty())
                                {
                                    SNAP_LOG_ERROR
                                        << "the test is limited to one namespace."
                                        << SNAP_LOG_SEND;
                                    return false;
                                }
                                name_space = variable_name;
                                variable_name.clear();
                                ++m; // skip one of the ':', the for() skips the other
                            }
                            else
                            {
                                variable_name += *m;
                            }
                        }
                        if(variable_name.empty())
                        {
                            SNAP_LOG_ERROR
                                << "variable name cannot be empty."
                                << SNAP_LOG_SEND;
                            return false;
                        }

                        std::string project_name(library_path);
                        if(project_name.empty())
                        {
                            project_name = snapdev::pathinfo::basename(source_path);
                        }

                        if(!name_space.empty())
                        {
                            // verify this is this project's name
                            //
                            if(project_name != name_space)
                            {
                                SNAP_LOG_ERROR
                                    << "the project name \""
                                    << project_name
                                    << "\" does not match the namespace \""
                                    << name_space
                                    << "\"."
                                    << SNAP_LOG_SEND;
                                return false;
                            }
                        }

                        // load the names.an
                        //
                        std::string full_library_path(snapdev::pathinfo::canonicalize(source_path, project_name));
                        std::string names_an_filename(snapdev::pathinfo::canonicalize(full_library_path, "names.an"));

                        snapdev::file_contents names_an(names_an_filename);

                        if(!names_an.exists())
                        {
                            SNAP_LOG_ERROR
                                << "cannot find the names.an file \""
                                << names_an_filename
                                << "\"."
                                << SNAP_LOG_SEND;
                            return false;
                        }

                        if(!names_an.read_all())
                        {
                            SNAP_LOG_ERROR
                                << "could not load names_an \""
                                << names_an_filename
                                << "\"."
                                << SNAP_LOG_SEND;
                            return false;
                        }

                        std::string const & names_an_data(names_an.contents());

                        // prinbee::g_name_prinbee_service_proxy    -- name in .cpp file
                        // service_proxy=pb_proxy                   -- name in .an file
                        //
                        std::string introducer("g_name_" + project_name + "_");
                        if(!variable_name.starts_with(introducer))
                        {
                            SNAP_LOG_ERROR
                                << "variable \""
                                << variable_name
                                << "\" does not start with introducer \""
                                << introducer
                                << "\"."
                                << SNAP_LOG_SEND;
                            return false;
                        }

                        char const * name_search(variable_name.c_str() + introducer.length());
                        std::size_t const name_length(variable_name.length() - introducer.length());

                        char const * n(names_an_data.c_str());
                        for(; *n != '\0'; ++n)
                        {
                            if(*n == '\n')
                            {
                                ++n;
                                if(strncmp(n, name_search, name_length) == 0)
                                {
                                    n += name_length;
                                    for(; isspace(*n); ++n);
                                    if(*n != '=')
                                    {
                                        SNAP_LOG_ERROR
                                            << "variable \""
                                            << name_search
                                            << "\" in \""
                                            << names_an_filename
                                            << "\" is not followed by an equal sign."
                                            << SNAP_LOG_SEND;
                                        return false;
                                    }
                                    for(++n; isspace(*n); ++n);
                                    for(; *n != '\n' && *n != '\0' && !isspace(*n); ++n)
                                    {
                                        service_name += *n;
                                    }
                                    break;
                                }
                            }
                        }

                        if(service_name.empty())
                        {
                            SNAP_LOG_ERROR
                                << "could not find \""
                                << name_search
                                << "\" in \""
                                << names_an_filename
                                << "\" found."
                                << SNAP_LOG_SEND;
                            return false;
                        }
                    }

                    // check that the name represents the name of the
                    // service file under the conf/... folder
                    //
                    std::string const conf_path(snapdev::pathinfo::canonicalize(source_path, "conf"));
                    std::string const service_filename(snapdev::pathinfo::canonicalize(conf_path, service_name + ".service"));
                    snapdev::file_contents service_file(service_filename);
                    if(!service_file.exists())
                    {
                        SNAP_LOG_ERROR
                            << "service not found under conf/... folder \""
                            << service_filename
                            << "\", please verify that the name in the messenger matches one of the names in the conf/... folder."
                            << SNAP_LOG_SEND;
                        return false;
                    }

                    // success
                    //
                    return true;
                }
            }
        }
    }

    SNAP_LOG_ERROR
        << "fluid_settings_connection or communicator_connection not found in messenger file."
        << SNAP_LOG_SEND;
    return false;
}



} // namespace communicator
// vim: ts=4 sw=4 et
