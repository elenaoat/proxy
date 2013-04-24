Host to test the http-dns proxy:

A query:
wget --post-data="Name=huuto.net&Type=A" -O - http://localhost:3001/dns-query

AAAA query:
wget --post-data="Name=ipv6.l.google.com&Type=AAAA" -O - http://localhost:3001/dns-query
