/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "version.h"

static void help(const char *msg)
{
  if(msg != NULL)
    fprintf(stderr, "%s:\n\n", msg);
  fprintf(stderr, "Usage: [options] [URL]\n"
          "  -h                    - this help\n"
          " INPUT\n"
          "  --fragment [fragment] - set this fragment\n"
          "  --host [host]         - set this host name\n"
          "  --options [options]   - set these options\n"
          "  --password [secret]   - set this password\n"
          "  --path [path]         - set this path\n"
          "  --port [port]         - set this port number\n"
          "  --query [query]       - set this query\n"
          "  --redirect [URL]      - redirect the base URL to this\n"
          "  --scheme [scheme]     - set this scheme\n"
          "  --url [base URL]      - URL to start with\n"
          "  --user [name]         - set this user\n"
          "  --zoneid [zoneid]     - set this zone id\n"
          " OUTPUT\n"

          "  --only-fragment       - output only the fragment part"
          "  --only-host           - output only the host part"
          "  --only-options        - output only the options part"
          "  --only-password       - output only the password part"
          "  --only-path           - output only the path part"
          "  --only-port           - output only the port part"
          "  --only-query          - output only the query part"
          "  --only-scheme         - output only the scheme part"
          "  --only-user           - output only the user part"
          "  --only-zoneid         - output only the zoneid part"
    );
  exit(1);
}

static void show_version(void)
{
  fputs("urler version " URLER_VERSION_TXT "\n", stdout);
  exit(0);
}

#define OUTPUT_URL      0  /* default */
#define OUTPUT_SCHEME   1
#define OUTPUT_USER     2
#define OUTPUT_PASSWORD 3
#define OUTPUT_OPTIONS  4
#define OUTPUT_HOST     5
#define OUTPUT_PORT     6
#define OUTPUT_PATH     7
#define OUTPUT_QUERY    8
#define OUTPUT_FRAGMENT 9
#define OUTPUT_ZONEID   10


struct option {
  const char *url;
  const char *host;
  const char *scheme;
  const char *port;
  const char *user;
  const char *password;
  const char *options;
  const char *path;
  const char *query;
  const char *fragment;
  const char *zoneid;
  const char *redirect;

  unsigned char output;
};

static int getlongarg(struct option *op,
                      const char *flag,
                      const char *arg)
{
  if(!strcmp("--version", flag))
    show_version();
  if(!strcmp("--url", flag))
    op->url = arg;
  else if(!strcmp("--host", flag))
    op->host = arg;
  else if(!strcmp("--scheme", flag))
    op->scheme = arg;
  else if(!strcmp("--port", flag))
    op->port = arg;
  else if(!strcmp("--user", flag))
    op->user = arg;
  else if(!strcmp("--password", flag))
    op->password = arg;
  else if(!strcmp("--options", flag))
    op->options = arg;
  else if(!strcmp("--path", flag))
    op->path = arg;
  else if(!strcmp("--query", flag))
    op->query = arg;
  else if(!strcmp("--fragment", flag))
    op->fragment = arg;
  else if(!strcmp("--zoneid", flag))
    op->zoneid = arg;
  else if(!strcmp("--redirect", flag))
    op->redirect = arg;

  else if(!strcmp("--only-scheme", flag))
    op->output = OUTPUT_SCHEME;
  else if(!strcmp("--only-user", flag))
    op->output = OUTPUT_USER;
  else if(!strcmp("--only-password", flag))
    op->output = OUTPUT_PASSWORD;
  else if(!strcmp("--only-options", flag))
    op->output = OUTPUT_OPTIONS;
  else if(!strcmp("--only-host", flag))
    op->output = OUTPUT_HOST;
  else if(!strcmp("--only-port", flag))
    op->output = OUTPUT_PORT;
  else if(!strcmp("--only-path", flag))
    op->output = OUTPUT_PATH;
  else if(!strcmp("--only-query", flag))
    op->output = OUTPUT_QUERY;
  else if(!strcmp("--only-fragment", flag))
    op->output = OUTPUT_FRAGMENT;
  else if(!strcmp("--only-zoneid", flag))
    op->output = OUTPUT_ZONEID;
  return 0;
}

static int getshortarg(struct option *op,
                       const char *flag)
{
  (void)op;
  if(!strcmp("-v", flag))
    show_version();
  else if(!strcmp("-h", flag))
    help(NULL);
  return 0;
}


int main(int argc, const char **argv)
{
  int exit_status = 0;
  char *nurl = NULL;
  struct option o;
  CURLU *uh;
  memset(&o, 0, sizeof(o));
  curl_global_init(CURL_GLOBAL_ALL);

  for(argc--, argv++; argc > 0; argc--, argv++) {
    if(argv[0][0] == '-' && argv[0][1] != '-') {
      /* single-dash prefix */
      if(getshortarg(&o, argv[0]))
        help("unknown option");
    }
    else if(argv[0][0] == '-' && argv[0][1] == '-') {
      /* dash-dash prefixed */
      if(getlongarg(&o, argv[0], argv[1]))
        help("unknown option");

      /* skip the parsed argument */
      argc--;
      argv++;
    }
    else {
      /* this is a URL */
      o.url = argv[0];
    }
  }

  uh = curl_url();
  if(!uh)
    help("out of memory");
  if(o.url) {
    curl_url_set(uh, CURLUPART_URL, o.url,
                 CURLU_GUESS_SCHEME|CURLU_NON_SUPPORT_SCHEME);
    if(o.redirect)
      curl_url_set(uh, CURLUPART_URL, o.redirect,
                   CURLU_GUESS_SCHEME|CURLU_NON_SUPPORT_SCHEME);
  }
  if(o.host)
    curl_url_set(uh, CURLUPART_HOST, o.host, 0);
  if(o.scheme)
    curl_url_set(uh, CURLUPART_SCHEME, o.scheme, CURLU_NON_SUPPORT_SCHEME);
  if(o.port)
    curl_url_set(uh, CURLUPART_PORT, o.port, 0);
  if(o.user)
    curl_url_set(uh, CURLUPART_USER, o.user, 0);
  if(o.password)
    curl_url_set(uh, CURLUPART_PASSWORD, o.password, 0);
  if(o.options)
    curl_url_set(uh, CURLUPART_OPTIONS, o.options, 0);
  if(o.path)
    curl_url_set(uh, CURLUPART_PATH, o.path, 0);
  if(o.query)
    curl_url_set(uh, CURLUPART_QUERY, o.query, 0);
  if(o.fragment)
    curl_url_set(uh, CURLUPART_FRAGMENT, o.fragment, 0);
  if(o.zoneid)
    curl_url_set(uh, CURLUPART_ZONEID, o.zoneid, 0);

  if(o.output) {
    CURLUPart cpart = CURLUPART_HOST;
    const char *name = NULL;

    /* only extract the part we want to show */
    switch(o.output) {
    case OUTPUT_SCHEME:
      cpart = CURLUPART_SCHEME;
      name = "scheme";
      break;
    case OUTPUT_USER:
      cpart = CURLUPART_USER;
      name = "user";
      break;
    case OUTPUT_PASSWORD:
      cpart = CURLUPART_PASSWORD;
      name = "password";
      break;
    case OUTPUT_OPTIONS:
      cpart = CURLUPART_OPTIONS;
      name = "options";
      break;
    case OUTPUT_HOST:
      cpart = CURLUPART_HOST;
      name = "host";
      break;
    case OUTPUT_PORT:
      cpart = CURLUPART_PORT;
      name = "port";
      break;
    case OUTPUT_PATH:
      cpart = CURLUPART_PATH;
      name = "path";
      break;
    case OUTPUT_QUERY:
      cpart = CURLUPART_QUERY;
      name = "query";
      break;
    case OUTPUT_ZONEID:
      cpart = CURLUPART_ZONEID;
      name = "zoneid";
      break;
    }
    if(!curl_url_get(uh, cpart, &nurl, CURLU_DEFAULT_PORT)) {
      printf("%s\n", nurl);
      curl_free(nurl);
    }
    else {
      fprintf(stderr, "not enough input to show %s (urle -h for help)\n",
              name);
      exit_status = 1;
    }
  }
  else {
    /* default output is full URL */
    if(!curl_url_get(uh, CURLUPART_URL, &nurl, 0)) {
      printf("%s\n", nurl);
      curl_free(nurl);
    }
    else {
      fprintf(stderr, "not enough input for a URL (urle -h for help)\n");
      exit_status = 1;
    }
  }
  curl_url_cleanup(uh);
  /* we're done with libcurl, so clean it up */
  curl_global_cleanup();
  return exit_status;
}