// CODYlib		-*- mode:c++ -*-
// Copyright (C) 2020 Nathan Sidwell, nathan@acm.org
// License: Apache v2.0

// Cody
#include "internal.hh"
// C++
#include <tuple>
// C
#include <cerrno>
#include <cstdlib>
#include <cstring>

// Server code

namespace Cody {

// These do not need to be members
static Resolver *ConnectRequest (Server *, Resolver *,
				 std::vector<std::string> &words);
static int ModuleRepoRequest (Server *, Resolver *,
			      std::vector<std::string> &words);
static int ModuleExportRequest (Server *, Resolver *,
				std::vector<std::string> &words);
static int ModuleImportRequest (Server *, Resolver *,
				std::vector<std::string> &words);
static int ModuleCompiledRequest (Server *, Resolver *,
				  std::vector<std::string> &words);
static int IncludeTranslateRequest (Server *, Resolver *,
				     std::vector<std::string> &words);

namespace {
using RequestFn = int (Server *, Resolver *, std::vector<std::string> &);
using RequestPair = std::tuple<char const *, RequestFn *>;
static RequestPair
  const requestTable[Detail::RC_HWM] =
  {
    // Same order as enum RequestCode
    RequestPair {"HELLO", nullptr},
    RequestPair {"MODULE-REPO", ModuleRepoRequest},
    RequestPair {"MODULE-EXPORT", ModuleExportRequest},
    RequestPair {"MODULE-IMPORT", ModuleImportRequest},
    RequestPair {"MODULE-COMPILED", ModuleCompiledRequest},
    RequestPair {"INCLUDE-TRANSLATE", IncludeTranslateRequest},
  };
}

Server::Server (Resolver *r)
  : resolver (r), direction (READING)
{
  PrepareToRead ();
}

Server::Server (Server &&src)
  : write (std::move (src.write)),
    read (std::move (src.read)),
    resolver (src.resolver),
    is_connected (src.is_connected),
    direction (src.direction)
{
  fd.from = src.fd.from;
  fd.to = src.fd.to;
}

Server::~Server ()
{
}

Server &Server::operator= (Server &&src)
{
  write = std::move (src.write);
  read = std::move (src.read);
  resolver = src.resolver;
  is_connected = src.is_connected;
  direction = src.direction;
  fd.from = src.fd.from;
  fd.to = src.fd.to;

  return *this;
}

void Server::DirectProcess (Detail::MessageBuffer &from,
			    Detail::MessageBuffer &to)
{
  read.PrepareToRead ();
  std::swap (read, from);
  ProcessRequests ();
  resolver->WaitUntilReady (this);
  write.PrepareToWrite ();
  std::swap (to, write);
}

void Server::ProcessRequests (void)
{
  std::vector<std::string> words;

  direction = PROCESSING;
  while (!read.IsAtEnd ())
    {
      int err = 0;
      unsigned ix = Detail::RC_HWM;
      if (!read.Lex (words))
	{
	  Assert (!words.empty ());
	  while (ix--)
	    {
	      if (words[0] != std::get<0> (requestTable[ix]))
		continue; // not this one

	      if (ix == Detail::RC_CONNECT)
		{
		  // CONNECT
		  if (IsConnected ())
		    err = -1;
		  else if (auto *r = ConnectRequest (this, resolver, words))
		    resolver = r;
		  else
		    err = -1;
		}
	      else
		{
		  if (!IsConnected ())
		    err = -1;
		  else if (int res = (std::get<1> (requestTable[ix])
				      (this, resolver, words)))
		    err = res;
		}
	      break;
	    }
	}

      if (err || ix >= Detail::RC_HWM)
	{
	  // Some kind of error
	  std::string msg;

	  if (err > 0)
	    msg = "error processing '";
	  else if (ix >= Detail::RC_HWM)
	    msg = "unrecognized '";
	  else if (IsConnected () && ix == Detail::RC_CONNECT)
	    msg = "already connected '";
	  else if (!IsConnected () && ix != Detail::RC_CONNECT)
	    msg = "not connected '";
	  else
	    msg = "malformed '";

	  read.LexedLine (msg);
	  msg.append ("'");
	  if (err > 0)
	    {
	      msg.append (" ");
	      msg.append (strerror (err));
	    }
	  resolver->ErrorResponse (this, std::move (msg));
	}
    }
}

// Return numeric value of STR as an unsigned.  Returns ~0u on error
// (so that value is not representable).
static unsigned ParseUnsigned (std::string &str)
{
  char *eptr;
  unsigned long val = strtoul (str.c_str (), &eptr, 10);
  if (*eptr || unsigned (val) != val)
    return ~0u;

  return unsigned (val);
}

Resolver *ConnectRequest (Server *s, Resolver *r,
			  std::vector<std::string> &words)
{
  if (words.size () < 3 || words.size () > 4)
    return nullptr;

  if (words.size () == 3)
    words.emplace_back ("");
  unsigned version = ParseUnsigned (words[1]);
  if (version == ~0u)
    return nullptr;

  return r->ConnectRequest (s, version, words[2], words[3]);
}

int ModuleRepoRequest (Server *s, Resolver *r,std::vector<std::string> &words)
{
  if (words.size () != 1)
    return -1;

  return r->ModuleRepoRequest (s);
}

int ModuleExportRequest (Server *s, Resolver *r, std::vector<std::string> &words)
{
  if (words.size () < 2 || words.size () > 3 || words[1].empty ())
    return -1;

  Flags flags = Flags::None;
  if (words.size () == 3)
    {
      unsigned val = ParseUnsigned (words[2]);
      if (val == ~0u)
	return -1;
      flags = Flags (val);
    }

  return r->ModuleExportRequest (s, flags, words[1]);
}

int ModuleImportRequest (Server *s, Resolver *r, std::vector<std::string> &words)
{
  if (words.size () < 2 || words.size () > 3 || words[1].empty ())
    return -1;

  Flags flags = Flags::None;
  if (words.size () == 3)
    {
      unsigned val = ParseUnsigned (words[2]);
      if (val == ~0u)
	return -1;
      flags = Flags (val);
    }

  return r->ModuleImportRequest (s, flags, words[1]);
}

int ModuleCompiledRequest (Server *s, Resolver *r,
			   std::vector<std::string> &words)
{
  if (words.size () < 2 || words.size () > 3 || words[1].empty ())
    return -1;

  Flags flags = Flags::None;
  if (words.size () == 3)
    {
      unsigned val = ParseUnsigned (words[2]);
      if (val == ~0u)
	return -1;
      flags = Flags (val);
    }

  return r->ModuleCompiledRequest (s, flags, words[1]);
}

int IncludeTranslateRequest (Server *s, Resolver *r,
			     std::vector<std::string> &words)
{
  if (words.size () < 2 || words.size () > 3 || words[1].empty ())
    return -1;

  Flags flags = Flags::None;
  if (words.size () == 3)
    {
      unsigned val = ParseUnsigned (words[2]);
      if (val == ~0u)
	return -1;
      flags = Flags (val);
    }

  return r->IncludeTranslateRequest (s, flags, words[1]);
}

void Server::ErrorResponse (char const *error, size_t elen)
{
  write.BeginLine ();
  write.AppendWord ("ERROR");
  write.AppendWord (error, true, elen);
  write.EndLine ();
}

void Server::OKResponse ()
{
  write.BeginLine ();
  write.AppendWord ("OK");
  write.EndLine ();
}

void Server::ConnectResponse (char const *agent, size_t alen)
{
  is_connected = true;

  write.BeginLine ();
  write.AppendWord ("HELLO");
  write.AppendInteger (Version);
  write.AppendWord (agent, true, alen);
  write.EndLine ();
}

void Server::PathnameResponse (char const *cmi, size_t clen)
{
  write.BeginLine ();
  write.AppendWord ("PATHNAME");
  write.AppendWord (cmi, true, clen);
  write.EndLine ();
}

void Server::BoolResponse (bool truthiness)
{
  write.BeginLine ();
  write.AppendWord ("BOOL");
  write.AppendWord (truthiness ? "TRUE" : "FALSE");
  write.EndLine ();
}

}
