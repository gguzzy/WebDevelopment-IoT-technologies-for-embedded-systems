#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import cherrypy 
import json
from TEMP import convert_Temperature

class TemperatureConverter(object):
    exposed = True
    
    def PUT(self, *uri, **params):
        list_v = []
        my_string = "POST RESPONSE:"
        my_string = cherrypy.request.body.read()
        #converting dictionary string to dictionary
        dict_json = json.loads(my_string)
        
        values = dict_json['values']
        o = dict_json['originalUnit ']
        f = dict_json['targetUnit ']
        
        for i in values:
            list_v.append(convert_Temperature(i, o, f))
            
        dict_json['Numerical values after conversion'] = list_v

        #converting dictionary to string
        result = json.dumps(dict_json)
        return result
    
if __name__ == '__main__':
	app_config = {
		'/': {
			'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
			'tools.sessions.on': True
		}
	}
    
    #application configuration
	cherrypy.tree.mount(TemperatureConverter(), '/converter', app_config)

	cherrypy.config.update({'server.socket_host': '0.0.0.0'})
	cherrypy.config.update({'server.socket_port': 8080})
	cherrypy.engine.start()
	cherrypy.engine.block()
