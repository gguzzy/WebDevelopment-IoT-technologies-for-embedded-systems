#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import cherrypy 
import json
from TEMP import convert_Temperature

class TemperatureConverter(object): 
	exposed = True 

	def GET (self, **params):
            #local variables
            value = params['value']
            targetUnit = params['targetUnit'] 
            originalUnit = params['originalUnit']
            #errors handling
            if (len(params)!=3):
                raise cherrypy.HTTPError(404, "Error parameters must have following format: value, original dim, destination dim")
            #conversion temperature
            finalUnit = convert_Temperature(value, targetUnit, originalUnit)
            #saving in dictionary: still not JSON
            dict_json = {
                    'Original numerical value':value, 
                    'Original unit of measurement':originalUnit,
                    'Numerical value after conversion':finalUnit,
                    'Final unit of measurement':targetUnit
            }
            return json.dumps(dict_json) #returns JSON Response

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
