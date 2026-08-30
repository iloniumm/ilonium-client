/*

*************************************************************************

RetroCycles

**************************************************************************

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

***************************************************************************

*/

#ifndef ArmageTron_BACKEND_H
#define ArmageTron_BACKEND_H

// Every backend endpoint the client talks to, gathered in one place. Each host
// is a Cloudflare Workers account subdomain of the form
//   <worker>.<account>.workers.dev
// The worker names on the left of the first dot stay put; only the account
// subdomain in the middle identifies who owns it, so that is the part kept
// neutral here. To move a service, rename the account subdomain in the
// Cloudflare dashboard to match the host below, or change the host and rebuild.

#define RC_BACKEND_API  "retrocycles-api.workers.dev"   // tickets + telemetry
#define RC_BACKEND_NET  "retrocycles-net.workers.dev"   // chat

#define RC_URL_TICKET     "https://ticket-system." RC_BACKEND_API "/"
#define RC_URL_CHAT       "https://client-chat."   RC_BACKEND_NET "/chat"

// Our own machine, which the chat and the tickets have moved to. One host,
// one certificate, and limits that belong to us rather than to somebody's
// free tier. Everything under it is signed with the key the client makes for
// itself on first run - see cIlonium.h.
#define RC_ILONIUM_API    "https://api.ilonium.dev"

// Which build this is. Sent with the hello the client makes every few minutes,
// and compared against what the backend says the current release is - so a
// player running something six weeks old is told so rather than finding out
// from a server that refuses them.
#define RC_CLIENT_VERSION "0.9.8"

#endif
