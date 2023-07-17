We need to add:

* PATCH method functions+parsing+header

* client attribute cwd filled with cwd + root;

* path_type function (is relative or absolute).

* upload req_path, cgi_stat, log_path, CORS, Etag rules in configfile.

* Project struct:

      src{ tools{.cpp}, multiplixers{.cpp}, methods{.cpp}, managers{.cpp}, parsers{.cpp}, handlers{.cpp}, header{.hpp} }

      bin{ program && cgis }

      etc{ config files }


Errors to solve:

* multiplixing Poll : sigfault.

* multiplixer Kqueue : doesnt return to clients whos waiting (CGI and KEEP_ALIVE).

* handlers : doesnt refill res->filepath with correct path.

## IMPORTANT ##

client does not need pointer to server but request does.
