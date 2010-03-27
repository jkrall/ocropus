// -*- C++ -*-

// Copyright 2006 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
// or its licensors, as applicable.
//
// You may not use this file except under the terms of the accompanying license.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You may
// obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Project:
// File: sysutil.h
// Purpose: miscelaneous routines
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de


#ifndef h_sysutil_
#define h_sysutil_


namespace ocropus {

#ifdef WIN32
//WIN32: The struct has been picked up from Winsock2.h. Including Winsock2.h conflicts with header files of colib.
    struct timeval {
          long    tv_sec;         // seconds
          long    tv_usec;        // and microseconds 
    };

inline int gettimeofday(timeval *time, timeval *temp) {
    __time32_t  *seconds=0;
    time->tv_usec=0;//setting the microsecond counter to zero.
    time->tv_sec=_time32(seconds);
    return 0;
}
#endif

    struct Timer {
        double total;
        double start_time;
        Timer();
        void reset();
        void operator+();
        inline void start() {+(*this);}
        void operator-();
        inline void stop() {-(*this);}
        double operator*();
        inline double sum() { return **this; }
    };

    double now();
#ifndef WIN32
    double user_time();
    double system_time();
    double heap_memory();
    double stack_memory();
    double page_faults();
#endif

    void mkdir_if_necessary(const char *path);
}

#endif
