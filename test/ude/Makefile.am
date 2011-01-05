bin_PROGRAMS = server client

INCLUDES = -I$(top_srcdir)

server_SOURCES = server.cpp
client_SOURCES = client.cpp

if USE_WIN32API
#server_LDFLAGS = -lws2_32
#server_LDADD =
#client_LDFLAGS = -lws2_32
#client_LDADD = libws2_32.a
endif
