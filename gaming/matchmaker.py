#!/usr/bin/env python2
from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer

PORT_NUMBER = 8080

games = {}
class myHandler(BaseHTTPRequestHandler):
    #Handler for the GET requests
    def do_GET(self):

        # Send the html message
        action, params = self.path.split('?')
        params = {k:v for k,v in (p.split('=') for p in params.split('&'))}
        if 'host' in action:
            self.send_response(200)
            self.end_headers()
            games[params['g']] = params['ip']
        else:
            if params['g'] in games:
                self.send_response(200)
                self.end_headers()
                self.wfile.write(games[params['g']])
            else:
                self.send_response(404)
                self.end_headers()
        return

server = HTTPServer(('0.0.0.0', PORT_NUMBER), myHandler)
print 'Started httpserver on port ' , PORT_NUMBER
try:
    server.serve_forever()
except KeyboardInterrupt:
    print '^C received, shutting down the web server'
    server.socket.close()
