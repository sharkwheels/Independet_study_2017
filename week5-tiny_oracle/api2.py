########################
# Flask-restful api example
# city feelings, just a hacky example, but its start
# http://flask-restful.readthedocs.io/en/0.3.5/reqparse.html
# https://blog.miguelgrinberg.com/post/designing-a-restful-api-using-flask-restful

# do some error codes
# do some basic auth
########################

from flask import Flask, request, Response
from flask_restful import Api, Resource,reqparse, abort
from functools import wraps
import random
from random import choice

app = Flask(__name__)
api = Api(app)

@app.route('/')
def index():
	return "Hello, World!"

@app.route('/about')
def about():
	return "An an about page"

class CityFeels(Resource):
	def getFeels(self):
		""" Construct how your city feels This is just a test for now to fuck w/ this protocol"""
		randomFeels = [
			{'feeling': "angry",'color':'red','message':'Today I am angry'},
			{'feeling': "happy",'color':'green','message':'Today I am happy'},
			{'feeling': "sad",'color':'blue','message':'Today I am sad'}
		]
		return random.choice(randomFeels)

	def get(self):
		boop = self.getFeels()
		#print(boop)
		return boop

api.add_resource(CityFeels, '/cityfeels')

if __name__ == '__main__':
	app.run(host='0.0.0.0')
	#app.run(debug=True)



