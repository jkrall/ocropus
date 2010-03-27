// Copyright 2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// File: logging.cc
// Purpose: logging in HTML file different kind of information
// Responsible: rangoni
// Reviewer: rangoni
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org

#include <stdarg.h>
#include <time.h>
#include "ocropus.h"
#include "narray-io.h"
#include "logger.h"

using namespace colib;
using namespace iulib;
using namespace ocropus;

namespace {
    struct Log {
        narray<iucstring> enabled_logs;
        iucstring dir;
        stdio file;
    };

    // sorry for all that.
    Log *the_log = NULL;
    Log *get_log() {
        if(!the_log)
            the_log = new Log();
        return the_log;
    }

    struct DeleteLog {
        ~DeleteLog() {
            if(the_log)
                delete the_log;
        }
    } delete_log;

    int indent_level = 0;
    int image_counter = 0;
    bool self_logging;

    /// Returns true if the given specification chunk turns on a log with the given name.
    /// The simplest way would be to use !strcmp, but we have an extension:
    /// a spec "X" would turn on a log named "X.Y".
    bool turns_on(const char *spec, const char *name) {
        int spec_len = strlen(spec);
        int name_len = strlen(spec);
        if(spec_len > name_len)
            return false; // then spec is too long to be a prefix of name
        if(strncmp(spec, name, spec_len))
            return false; // then spec can't be a prefix of name
        if(name_len == spec_len)
            return true; // spec == name
        return name[spec_len] == '.';
    }

};


namespace ocropus {
    bool Logger::turned_on(const char *name) {
        for(int i = 0; i < get_log()->enabled_logs.length(); i++)
            if(turns_on(get_log()->enabled_logs[i], name))
                return true;
        return false;
    }

    void Logger::init_logging() {
        char *ocrolog = getenv("ocrolog");
        if(!ocrolog)
            return;
        split_string(get_log()->enabled_logs, ocrolog, ":;");
        self_logging = turned_on("logger");
    }

    void Logger::start_logging() {
        if(!!get_log()->file)
            return;
        const char *ocrologdir = getenv("ocrologdir");
        if(!ocrologdir) {
            ocrologdir = "log.ocropus";
        }
        set_logger_directory(ocrologdir);

        fprintf(get_log()->file, "logging turned on for the following loggers:<BR /><UL>\n");
        for(int i = 0; i < get_log()->enabled_logs.length(); i++)
            fprintf(get_log()->file, "    <LI>%s</LI>\n", get_log()->enabled_logs[i].c_str());
        fprintf(get_log()->file, "</UL>\n");

        time_t rawtime;
        time (&rawtime);
        fprintf(get_log()->file, "Started %s <BR /><BR />\n", ctime (&rawtime));
        fflush(get_log()->file);
    }

    void Logger::putIndent() {
        fprintf(get_log()->file, "[%s] ", name.c_str());
        for(int i = 0; i < indent_level; i++) {
            fprintf(get_log()->file, "&nbsp;&nbsp;");
        }
    }

    stdio Logger::logImage(const char *description, int w, int h) {
        iucstring imageFile = get_log()->dir + "/ocropus-log-" + image_counter + ".png";
        putIndent();
        fprintf(get_log()->file, "%s:<br> <a HREF=\"ocropus-log-%d.png\">"
                "<IMG width=\"%d\" height=\"%d\" border=\"0\" SRC=\"ocropus-log-%d.png\">"
                "</a><BR>\n", description, image_counter, w, h, image_counter);
        fflush(get_log()->file);
        image_counter++;
        return stdio(imageFile, "wb");
    }
    stdio Logger::logImageHtml() {
        iucstring imageFile = get_log()->dir + "/ocropus-log-" + image_counter + ".png";
        //putIndent();
        fprintf(get_log()->file, "<IMG SRC=\"ocropus-log-%d.png\">",image_counter);
        fflush(get_log()->file);
        image_counter++;
        return stdio(imageFile, "wb");
    }
    stdio Logger::logImageHtmlBorder() {
        iucstring imageFile = get_log()->dir + "/ocropus-log-" + image_counter + ".png";
        //putIndent();
        fprintf(get_log()->file, "<IMG hspace=\"4\" vspace=\"4\" style=\"border-color:#8888FF\" SRC=\"ocropus-log-%d.png\" border=\"1\">",image_counter);
        fflush(get_log()->file);
        image_counter++;
        return stdio(imageFile, "wb");
    }

    stdio Logger::logText(const char *description) {
        iucstring textFile = get_log()->dir + "/ocropus-log-" + image_counter + ".txt";
        putIndent();
        fprintf(get_log()->file, "<A HREF=\"ocropus-log-%d.txt\">%s</A><BR>\n",
                image_counter, description);
        fflush(get_log()->file);
        image_counter++;
        return stdio(textFile, "w");
    }


    Logger::Logger(const char *name) {
        this->name = name;

        if(!get_log()->enabled_logs.length()) {
            init_logging();
        }

        enabled = false;
        for(int i = 0; i < get_log()->enabled_logs.length(); i++) {
            if(turns_on(get_log()->enabled_logs[i], name)) {
                enabled = true;
                break;
            }
        }

        if(enabled || self_logging)
            start_logging();

        // trying to handle gracefully the situation when the log file couldn't be opened
        if(!get_log()->file) {
            enabled = false;
            return;
        }

        if(self_logging)
            fprintf(get_log()->file, "[logger] `%s': %s<BR />\n", name, enabled ? "enabled": "disabled");
    }

    void Logger::format(const char *format, ...) {
        if(!enabled) return;
        va_list va;
        va_start(va, format);
        putIndent();
        vfprintf(get_log()->file, format, va);
        fprintf(get_log()->file, "<BR />\n");
        va_end(va);
        fflush(get_log()->file);
    }

    void Logger::operator()(const char *s) {
        if(!enabled) return;
        putIndent();
        fprintf(get_log()->file, "%s<BR>\n", s);
        fflush(get_log()->file);
    }
    void Logger::operator()(const char *message, bool val) {
        if(!enabled) return;
        putIndent();
        fprintf(get_log()->file, "%s: %s<BR>\n", message, val ? "true" : "false");
        fflush(get_log()->file);
    }
    void Logger::operator()(const char *message, int val) {
        if(!enabled) return;
        putIndent();
        fprintf(get_log()->file, "%s: %d<BR>\n", message, val);
        fflush(get_log()->file);
    }
    void Logger::operator()(const char *message, double val) {
        if(!enabled) return;
        putIndent();
        fprintf(get_log()->file, "%s: %lf<BR>\n", message, val);
        fflush(get_log()->file);
    }
    void Logger::operator()(const char *message, const char *val) {
        if(!enabled) return;
        putIndent();
        fprintf(get_log()->file, "%s: \"%s\"<BR>\n", message, val);
        fflush(get_log()->file);
    }
    void Logger::operator()(const char *message, nuchar val) {
        if(!enabled) return;
        putIndent();
        fprintf(get_log()->file, "%s: \'%lc\' (hex %x, dec %x)<BR>\n",
                message, val.ord(), val.ord(), val.ord());
        fflush(get_log()->file);
    }
    void Logger::operator()(const char *description, intarray &a, float zoom) {
        if(!enabled) return;
        if(a.rank() == 2) {
            int w = (zoom==100.) ? a.dim(0) : int(a.dim(0)*zoom/100.+.5);
            int h = (zoom==100.) ? a.dim(1) : int(a.dim(1)*zoom/100.+.5);
            stdio f = logImage(description, w, h);
            write_image_packed(f,a,"png");
        } else {
            stdio f = logText(description);
            text_write(f, a);
        }
    }

    void Logger::recolor(const char *description, intarray &a, float zoom) {
        if(!enabled) return;
        if(a.rank() == 2) {
            int w = (zoom==100.) ? a.dim(0) : int(a.dim(0)*zoom/100.+.5);
            int h = (zoom==100.) ? a.dim(1) : int(a.dim(1)*zoom/100.+.5);
            stdio f = logImage(description, w, h);
            intarray tmp;
            copy(tmp, a);
            simple_recolor(tmp);
            write_image_packed(f,tmp,"png");
        } else {
            stdio f = logText(description);
            text_write(f, a);
        }
    }
    void Logger::operator()(const char *description, bytearray &a, float zoom) {
        if(!enabled) return;
        if(a.rank() == 2) {
            int w = (zoom==100.) ? a.dim(0) : int(a.dim(0)*zoom/100.+.5);
            int h = (zoom==100.) ? a.dim(1) : int(a.dim(1)*zoom/100.+.5);
            stdio f = logImage(description, w, h);
            write_png(f, a);
        } else {
            stdio f = logText(description);
            text_write(f, a);
        }
    }
    void Logger::html(bytearray &a) {
        if(!enabled) return;
        if(a.rank() == 2) {
            stdio f = logImageHtml();
            write_png(f, a);
        } else {
            stdio f = logText("error");
            text_write(f, a);
        }
    }
    void Logger::html(const char* s) {
        if(!enabled) return;
        fprintf(get_log()->file, "%s\n", s);
    }
    void Logger::html_border(bytearray &img) {
        if(!enabled) return;
        if(img.rank() == 2) {
            stdio f = logImageHtmlBorder();
            write_png(f, img);
        } else {
            stdio f = logText("error");
            text_write(f, img);
        }
    }

    void Logger::operator()(const char *description, floatarray &a) {
        if(!enabled) return;
        stdio f = logText(description);
        text_write(f, a);
    }
    void Logger::operator()(const char *message, nustring &val) {
        if(!enabled) return;
        char *buf = val.newUtf8Encode();
        putIndent();
        fprintf(get_log()->file, "%s: nustring(\"%s\")<BR>\n", message, buf);
        fflush(get_log()->file);
        delete[] buf;
    }
    void Logger::html(nustring &val) {
        char *buf = val.newUtf8Encode();
        fprintf(get_log()->file, "%s", buf);
        fflush(get_log()->file);
        delete[] buf;
    }
    void Logger::operator()(const char *description, rectangle &val) {
        if(!enabled) return;
        putIndent();
        fprintf(get_log()->file, "%s: rectangle(%d,%d,%d,%d)<BR>\n",
                description, val.x0, val.y0, val.x1, val.y1);
        fflush(get_log()->file);
    }

    void Logger::operator()(const char *message, IGenericFst &val) {
        if(!enabled) return;
        nustring s;
        val.bestpath(s);
        char *buf = s.newUtf8Encode();
        putIndent();
        fprintf(get_log()->file, "%s: ICharLattice(bestpath: \"%s\")<BR>\n", message, buf);
        fflush(get_log()->file);
        delete[] buf;
    }

    void Logger::operator()(const char *description, void *ptr) {
        if(!enabled) return;
        putIndent();
        fprintf(get_log()->file, "%s: pointer(%p)<BR>\n", description, ptr);
        fflush(get_log()->file);
    }

    void Logger::indent() {
        if(enabled)
            indent_level++;
    }
    void Logger::dedent() {
        if(enabled)
            indent_level--;
    }

    const char* get_logger_directory() {
        return get_log()->dir.c_str();
    }

    int get_image_counter() {
        return image_counter;
    }

    void set_image_counter(int v) {
        image_counter = v;
    }

    void set_logger_directory(const char *path) {
        if(!!get_log()->file) {
            fprintf(get_log()->file,
                    "log finished; switching to directory %s\n", path);
        }
        mkdir_if_necessary(path);
        iucstring old_dir;
        if(get_log()->dir)
            old_dir = get_log()->dir;
        get_log()->dir = path;
        iucstring html;
        html = path;
        html += "/index.html";
        get_log()->file = fopen(html,"wt");
        if(!get_log()->file) {
            fprintf(stderr, "unable to open log file `%s' for writing\n",
                    html.c_str());
        }
        fprintf(get_log()->file, "<HTML>\n<HEAD>\n<meta http-equiv=\"content-type\" "
                        "content=\"text/html\"; charset=UTF-8\">\n</HEAD>\n<BODY>\n");
        if(old_dir) {
            fprintf(get_log()->file, "Log continued from %s<P>\n", old_dir.c_str());
        }
    }

#if 0


    // FIXME/rangoni please get this code out of logger.cc --tmb
    // This sort of specialized code belongs in a separate file.
    void Logger::display_one_line_graph(int d_d_trans,
            int size, int indent, intarray &cuts, floatarray &cost_t,
            intarray &id_t, intarray &prev_t, intarray &trans_t,
            intarray &charcode_t, objlist<bytearray> &subimages) {
        int n = cuts.dim(0);
        int max_trans=max(trans_t);
        char temp[640];
        fprintf(get_log()->file, "<tr>\n");

        for(int i=0; i<indent; i++) {           // empty cells for no transition
            fprintf(get_log()->file, "<td></td>");
        }
        for(int trans=indent; trans<=max_trans-size; trans+=size) {
            bool find = false;
            for(int i=0; i<n; i++) {                // for all the transitions
                if(cuts(i) == 0) {
                    // look for consecutive transition of the same length and start
                    if(((trans_t(i)-prev_t(i)) == size) &&
                        (prev_t(i)%size == indent) && (trans==prev_t(i))) {
                        sprintf(temp,"<td valign=\"top\" colspan=%d><center>",size);
                        fprintf(get_log()->file, temp);
                        if(id_t(i) == 0) {                              // special case: id=0 means white space or \n
                            fprintf(get_log()->file, "<br>White Space");
                        } else {
                            html(subimages(id_t(i)-1));                 // otherwise just put the picture
                        }
                        fprintf(get_log()->file, "<br><table border>");
                        for(int j=0;(d_d_trans==-1)||(j<d_d_trans);j++) {       // do not display all the possible labels
                            fprintf(get_log()->file, "<tr>");                   // make a table
                            nustring char_text;
                            char_text.resize(1);
                            char_text[0] = nuchar(charcode_t(i+j));             // with the label
                            char *buf = char_text.newUtf8Encode();              // and associated cost
                            sprintf(temp, "<td><font size=\"-1\">%s</font></td><td>"
                                "<font size=\"-1\">%.3f</font></td></tr>", buf, cost_t(i+j));
                            delete[] buf;
                            fputs(temp,get_log()->file);
                            if ((i+j+1>=n) || (cuts(i+j+1) == 0)) {
                                break;                  // cannot display more than possible number of transitions
                            }
                        }
                        fprintf(get_log()->file, "</table>");
                        //sprintf(temp, "<font size=\"-2\">%d --> %d</font>", prev_t(i), trans_t(i));
                        //fprintf(file, temp);          // having the 'coordinates' of the transition
                        fprintf(get_log()->file, "</center></td>");
                        find = true;
                    }
                }
            }
            if(find == false) {                             // if the transition is not found
                for(int i=0; i<size; i++) {     // display empty cell in compensation
                    fprintf(get_log()->file, "<td></td>");
                }
            }
        }
        fprintf(get_log()->file, "\n</tr>\n");
        fflush(get_log()->file);
    }

    void Logger::transitions_in_graph(int maxcomp, int debug_display_transitions,
            intarray &cuts, floatarray &cost_t,
            intarray &id_t, intarray &prev_t, intarray &trans_t,
            intarray &charcode_t, objlist<bytearray> &subimages) {
        if(!enabled) return;
        putIndent();
        fprintf(get_log()->file, "<table border>\n");
        for(int i=1; i <= maxcomp; i++) {       // diplay all possible combinations
            for(int j=0; j<i; j++) {
                display_one_line_graph(debug_display_transitions, i, j, cuts, cost_t, id_t, prev_t, trans_t, charcode_t, subimages);
            }
        }
        fprintf(get_log()->file, "</table>\n");
        fflush(get_log()->file);
    }

    void Logger::transitions_in_list(int maxcomp, int n_disp_trans, intarray &cuts, floatarray &cost_t,
                intarray &id_t, intarray &prev_t, intarray &trans_t,
                intarray &charcode_t, objlist<bytearray> &subimages) {
        if(!enabled) return;
        putIndent();
        char temp[1024];
        int n = cuts.dim(0);
        for(int i=0; i<n; i++) {                        // for all the transitions
            if (cuts(i) == 0) {                             // if a new transition is found
                operator()("");
                if (id_t(i) == 0) {                     // special case: id=0 means white space or \n
                    operator()("picture : white space");
                } else {
                    operator()("picture ", subimages(id_t(i)-1));
                }
            }       // display everything or the nbest
            if ((n_disp_trans == -1) or (cuts(i) < n_disp_trans)) {
                nustring char_text;
                char_text.resize(1);
                char_text[0] = nuchar(charcode_t(i));
                char *buf = char_text.newUtf8Encode();
                sprintf(temp,"transition `%s' (%d) id %d from %d to %d costs %f",
                        buf, charcode_t(i), id_t(i), prev_t(i), trans_t(i), fabs(cost_t(i)));
                delete[] buf;
                operator()(temp);
            }
        }
        fflush(get_log()->file);
    }
#endif
}
