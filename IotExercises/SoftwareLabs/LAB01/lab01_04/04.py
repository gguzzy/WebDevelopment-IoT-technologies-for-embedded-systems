#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Aug  1 11:44:51 2020

@author: Guzzy
"""

import cherrypy
import json

class FreeBoard:
    exposed = True
    
    def GET(self):
        return open('/Users/Guzzy/Desktop/IoT_lab/lab01_os/esercizi/04/freeboard/index.html')
        
    def POST(self):
        my_new_conf = open("dashboard.json", "w")
        #save new dashboard configuration
        str_conf = json.dumps((json.loads(cherrypy.request.body.read())))
        
        my_new_conf.write(str_conf)
        return 
        
#server-client configuration
if __name__ == '__main__':
	app_config = {
		'/': {
			'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
			'tools.sessions.on': True
		}
	}
    
    #application configuration
	cherrypy.tree.mount(FreeBoard(), '/', app_config)

	cherrypy.config.update({'server.socket_host': '0.0.0.0'})
	cherrypy.config.update({'server.socket_port': 8080})
	cherrypy.engine.start()
	cherrypy.engine.block()