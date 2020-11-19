#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import cherrypy 
import json
from TEMP import convert_Temperature

class TemperatureConverter(object): 
	exposed = True 

	def GET (self, *uri):
        #handling uri as array in local variables, here as
        #comment just to show what they represent
           """
            value = uri[0]
            targetUnit = uri[1] 
            originalUnit = uri[2]
           """
           if (len(uri)!=3):
                raise cherrypy.HTTPError(404, "Error parameters must have following format: value, original dim, destination dim")
           finalUnit = convert_Temperature(uri[0], uri[1], uri[2])

           dict_json = {
                    'Original numerical value':uri[0], 
                    'Original unit of measurement':uri[1],
                    'Numerical value after conversion':finalUnit,
                    'Final unit of measurement':uri[2]
            }
            #return ("Converted temperature: %.3f" % (self.final_T))
           return json.dumps(dict_json)

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
