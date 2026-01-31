// Copyright (c) 2013-2025  Made to Order Software Corp.  All Rights Reserved
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
#pragma once

/** \file
 * \brief Test extensions.
 *
 * We use Catch2 to write our tests. This file adds a function one can use
 * to verify that the service filenames match the name defined in the
 * messenger of the service.
 *
 * For example, the prinbee system has two services: the prinbee daemon
 * called "prinbeed" and the proxy daemon called "pb_proxy"; both of
 * these are defined in the conf/... folder as "prinbeed.service" and
 * "pb_proxy.service".
 */

// C++
//
#include    <string>

namespace communicator
{



bool verify_service_name(
      std::string const & source_path
    , std::string messenger_path = std::string()
    , std::string const & library_path = std::string());



} // namespace communicator
// vim: ts=4 sw=4 et
